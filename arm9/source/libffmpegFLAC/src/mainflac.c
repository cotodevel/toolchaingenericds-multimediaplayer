/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id: main.c 7742 2005-11-03 18:48:23Z dave $
 *
 * Copyright (C) 2005 Dave Chapman
 *
 * All files in this archive are subject to the GNU General Public License.
 * See the file COPYING in the source tree root for full license agreement.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/

/* A test program for the Rockbox version of the ffmpeg FLAC decoder.

   Compile using Makefile.test - run it as "./test file.flac" to decode the
   FLAC file to the file "test.wav" in the current directory

   This test program should support 16-bit and 24-bit mono and stereo files.

   The resulting "test.wav" should have the same md5sum as a WAV file created
   by the official FLAC decoder (it produces the same 44-byte canonical WAV 
   header.
*/

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "flac.h"

bool flac_init(FILE *df, FLACContext* fc)
{
    unsigned char buf[255];
    bool found_streaminfo=false;
    int endofmetadata=0;
    int blocklength;
    uint32_t* p;
    uint32_t seekpoint_lo,seekpoint_hi;
    uint32_t offset_lo,offset_hi;
    int n;

    if (fseek(df, 0, SEEK_SET) < 0)
        return false;

    if (fread(buf, 1, 4, df) < 4)
        return false;

    if (memcmp(buf,"fLaC",4) != 0) 
        return false;
	
    fc->metadatalength = 4;

    while (!endofmetadata) {
        if (fread(buf, 1, 4, df) < 4)
            return false;

        endofmetadata=(buf[0]&0x80);
        blocklength = (buf[1] << 16) | (buf[2] << 8) | buf[3];
        fc->metadatalength+=blocklength+4;

        if ((buf[0] & 0x7f) == 0)       /* 0 is the STREAMINFO block */
        {
            /* FIXME: Don't trust the value of blocklength */
            if (fread(buf, 1, blocklength, df) < 0)
                return false;
          
            fc->filesize = flength(df);
            fc->min_blocksize = (buf[0] << 8) | buf[1];
            fc->max_blocksize = (buf[2] << 8) | buf[3];
            fc->min_framesize = (buf[4] << 16) | (buf[5] << 8) | buf[6];
            fc->max_framesize = (buf[7] << 16) | (buf[8] << 8) | buf[9];
            fc->samplerate = (buf[10] << 12) | (buf[11] << 4) 
                             | ((buf[12] & 0xf0) >> 4);
            fc->channels = ((buf[12]&0x0e)>>1) + 1;
            fc->bps = (((buf[12]&0x01) << 4) | ((buf[13]&0xf0)>>4) ) + 1;

            /* totalsamples is a 36-bit field, but we assume <= 32 bits are 
               used */
            fc->totalsamples = (buf[14] << 24) | (buf[15] << 16) 
                               | (buf[16] << 8) | buf[17];

            /* Calculate track length (in ms) and estimate the bitrate 
               (in kbit/s) */
            fc->length = (fc->totalsamples / fc->samplerate) * 1000;

            found_streaminfo=true;
        } else if ((buf[0] & 0x7f) == 3) { /* 3 is the SEEKTABLE block */
            //fprintf(stderr,"Seektable length = %d bytes\n",blocklength);
            while (blocklength >= 18) {
                n=fread(buf,1, 18, df);
                if (n < 18) return false;
                blocklength-=n;

                p=(uint32_t*)buf;
                seekpoint_hi=betoh32(*(p++));
                seekpoint_lo=betoh32(*(p++));
                offset_hi=betoh32(*(p++));
                offset_lo=betoh32(*(p++));
            
                if ((seekpoint_hi != 0xffffffff) && (seekpoint_lo != 0xffffffff)) {
					//fprintf(stderr,"Seekpoint: %u, Offset=%u\n",seekpoint_lo,offset_lo);
                }
            }
            fseek(df, blocklength, SEEK_CUR);
        } else {
            /* Skip to next metadata block */
            if (fseek(df, blocklength, SEEK_CUR) < 0)
                return false;
        }
    }

   if (found_streaminfo) {
       fc->bitrate = ((fc->filesize-fc->metadatalength) * 8) / fc->length;
       return true;
   } else {
       return false;
   }
}
