/*	MikMod sound library
	(c) 1998, 1999, 2000 Miodrag Vallat and others - see file AUTHORS for
	complete list.

	This library is free software; you can redistribute it and/or modify
	it under the terms of the GNU Library General Public License as
	published by the Free Software Foundation; either version 2 of
	the License, or (at your option) any later version.
 
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Library General Public License for more details.
 
	You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
	02111-1307, USA.
*/

/*==============================================================================

  $Id: drv_nos.c,v 1.3 2004/01/31 22:39:40 raph Exp $

  Driver for no output

==============================================================================*/

/*

	Written by Jean-Paul Mikkers <mikmak@via.nl>

*/
#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "mikmod_internals.h"
#include "drv_nos.h"

#include <string.h>

static	SBYTE *buf=NULL;

static BOOL NS_IsThere(void)
{
	return 1;
}

static BOOL NS_Init(void)
{	
	return VC_Init();
}

static void NS_Exit(void)
{
	VC_Exit();
}

static void NS_Update(void)
{
	if(buf)
		VC_WriteBytes(buf,ZEROLEN);
}

void setBuffer(s16 *buffer)
{
	buf = (SBYTE *)buffer;
}

MIKMODAPI MDRIVER drv_nos={
	NULL,
	"No Sound",
	"Nosound Driver v3.0",
	0,255,
	"nosound",
	NULL,
	NULL,
	NS_IsThere,
	VC_SampleLoad,
	VC_SampleUnload,
	VC_SampleSpace,
	VC_SampleLength,
	NS_Init,
	NS_Exit,
	NULL,
	VC_SetNumVoices,
	VC_PlayStart,
	VC_PlayStop,
	NS_Update,
	NULL,
	VC_VoiceSetVolume,
	VC_VoiceGetVolume,
	VC_VoiceSetFrequency,
	VC_VoiceGetFrequency,
	VC_VoiceSetPanning,
	VC_VoiceGetPanning,
	VC_VoicePlay,
	VC_VoiceStop,
	VC_VoiceStopped,
	VC_VoiceGetPosition,
	VC_VoiceRealVolume
};


/* ex:set ts=4: */
