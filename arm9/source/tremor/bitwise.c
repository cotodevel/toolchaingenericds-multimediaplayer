/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis 'TREMOR' CODEC SOURCE CODE.   *
 *                                                                  *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis 'TREMOR' SOURCE CODE IS (C) COPYRIGHT 1994-2003    *
 * BY THE Xiph.Org FOUNDATION http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

  function: packing variable sized words into an octet stream

 ********************************************************************/

/* We're 'LSb' endian; if we write a word but read individual bits,
   then we'll read the lsb first */

#include <string.h>
#include <stdlib.h>
#include "misc.h"
#include "ogg.h"

static unsigned long mask[]=
{0x00000000,0x00000001,0x00000003,0x00000007,0x0000000f,
 0x0000001f,0x0000003f,0x0000007f,0x000000ff,0x000001ff,
 0x000003ff,0x000007ff,0x00000fff,0x00001fff,0x00003fff,
 0x00007fff,0x0000ffff,0x0001ffff,0x0003ffff,0x0007ffff,
 0x000fffff,0x001fffff,0x003fffff,0x007fffff,0x00ffffff,
 0x01ffffff,0x03ffffff,0x07ffffff,0x0fffffff,0x1fffffff,
 0x3fffffff,0x7fffffff,0xffffffff };

/* spans forward, skipping as many bytes as headend is negative; if
   headend is zero, simply finds next byte.  If we're up to the end
   of the buffer, leaves headend at zero.  If we've read past the end,
   halt the decode process. */

static void _span(oggpack_buffer *b){
  while(b->headend-(b->headbit>>3)<1){
    b->headend-=b->headbit>>3;
    b->headbit&=0x7;

    if(b->head->next){
      b->count+=b->head->length;
      b->head=b->head->next;

		if(b->headend+b->head->length>0){
			b->headptr=b->head->buffer->data+b->head->begin-b->headend;
		}
	  b->headend+=b->head->length; 
    }else{
      /* we've either met the end of decode, or gone past it. halt
	 only if we're past */
      if((b->headend*8) < b->headbit){
			/* read has fallen off the end */
			b->headend=-1;
      }
	  break;
    }
  }
}

void oggpack_readinit(oggpack_buffer *b,ogg_reference *r){
  memset(b,0,sizeof(*b));

  b->tail=b->head=r;
  b->count=0;
  b->headptr=b->head->buffer->data+b->head->begin;
  b->headend=b->head->length;
  _span(b);
}

#define _lookspan()   while(!end){\
                        head=head->next;\
                        if(!head) return -1;\
                        ptr=head->buffer->data + head->begin;\
                        end=head->length;\
                      }

/* Read in bits without advancing the bitptr; bits <= 32 */
long oggpack_look(oggpack_buffer *b,int bits){
  unsigned long m=mask[bits];
  unsigned long ret = 0;

  bits+=b->headbit;

  if(bits >= b->headend<<3){
    int            end=b->headend;
    unsigned char *ptr=b->headptr;
    ogg_reference *head=b->head;

    if(end<0)return -1;
    
    if(bits){
      _lookspan();
      ret=*ptr++>>b->headbit;
      if(bits>8){
        --end;
        _lookspan();
        ret|=*ptr++<<(8-b->headbit);  
        if(bits>16){
          --end;
          _lookspan();
          ret|=*ptr++<<(16-b->headbit);  
          if(bits>24){
            --end;
            _lookspan();
            ret|=*ptr++<<(24-b->headbit);  
            if(bits>32 && b->headbit){
              --end;
              _lookspan();
              ret|=*ptr<<(32-b->headbit);
            }
          }
        }
      }
    }

  }else{

    /* make this a switch jump-table */
    ret=b->headptr[0]>>b->headbit;
    if(bits>8){
      ret|=b->headptr[1]<<(8-b->headbit);  
      if(bits>16){
        ret|=b->headptr[2]<<(16-b->headbit);  
        if(bits>24){
          ret|=b->headptr[3]<<(24-b->headbit);  
          if(bits>32 && b->headbit)
            ret|=b->headptr[4]<<(32-b->headbit);
        }
      }
    }
  }

  ret&=m;
  return ret;
}

/* limited to 32 at a time */
void oggpack_adv(oggpack_buffer *b,int bits){
  bits+=b->headbit;
  b->headbit=bits&7;
  b->headend-=(bits>>3);
  b->headptr+=(bits>>3);
  if(b->headend<1)_span(b);
}

int oggpack_eop(oggpack_buffer *b){
  if(b->headend<0)return -1;
  return 0;
}

/* bits <= 32 */
long oggpack_read(oggpack_buffer *b,int bits){
  long ret=oggpack_look(b,bits);
  oggpack_adv(b,bits);
  return(ret);
}

long oggpack_bytes(oggpack_buffer *b){
  if(b->headend<0)return b->count+b->head->length;
  return b->count + b->head->length-b->headend + 
    (b->headbit+7)/8;
}

long oggpack_bits(oggpack_buffer *b){
  if(b->headend<0)return (b->count+b->head->length)*8;
  return (b->count + b->head->length-b->headend)*8 + 
    b->headbit;
}

/* Self test of the bitwise routines; everything else is based on
   them, so they damned well better be solid. */

#ifdef _V_BIT_TEST
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "framing.c"

static int ilog(unsigned long v){
  int ret=0;
  while(v){
    ret++;
    v>>=1;
  }
  return(ret);
}
      
oggpack_buffer r;
oggpack_buffer o;
ogg_buffer_state *bs;
ogg_reference *or;
#define TESTWORDS 256

void report(char *in){
  fprintf(stderr,"%s",in);
  exit(1);
}

int getbyte(ogg_reference *or,int position){
  while((or!=NULL) && (position>=or->length)){
    position-=or->length;
    or=or->next;
    if(or==NULL){
      fprintf(stderr,"\n\tERROR: getbyte ran off end of buffer.\n");
      exit(1);
    }
  }

  if((position+or->begin)&1)
    return (or->buffer->data[(position+or->begin)>>1])&0xff;
  else
    return (or->buffer->data[(position+or->begin)>>1]>>8)&0xff;
}

void cliptest(unsigned long *b,int vals,int bits,int *comp,int compsize){
  long i,bitcount=0;
  ogg_reference *or=ogg_buffer_alloc(bs,64);
  for(i=0;i<compsize;i++)
    or->buffer->data[i]= comp[i];
  or->length=i;

  oggpack_readinit(&r,or);
  for(i=0;i<vals;i++){
    unsigned long test;
    int tbit=bits?bits:ilog(b[i]);
    if((test=oggpack_look(&r,tbit))==0xffffffff)
      report("out of data!\n");
    if(test!=(b[i]&mask[tbit])){
      fprintf(stderr,"%ld) %lx %lx\n",i,(b[i]&mask[tbit]),test);
      report("looked at incorrect value!\n");
    }
    if((test=oggpack_read(&r,tbit))==0xffffffff){
      report("premature end of data when reading!\n");
    }
    if(test!=(b[i]&mask[tbit])){
      fprintf(stderr,"%ld) %lx %lx\n",i,(b[i]&mask[tbit]),test);
      report("read incorrect value!\n");
    }
    bitcount+=tbit;

    if(bitcount!=oggpack_bits(&r))
      report("wrong number of bits while reading!\n");
    if((bitcount+7)/8!=oggpack_bytes(&r))
      report("wrong number of bytes while reading!\n");

  }
  if(oggpack_bytes(&r)!=(bitcount+7)/8)report("leftover bytes after read!\n");
  ogg_buffer_release(or);
}

void _end_verify(int count){
  int i;

  /* are the proper number of bits left over? */
  int leftover=count*8-oggpack_bits(&o);
  if(leftover>7)
    report("\nERROR: too many bits reported left over.\n");
  
  /* does reading to exactly byte alignment *not* trip EOF? */
  if(oggpack_read(&o,leftover)==-1)
    report("\nERROR: read to but not past exact end tripped EOF.\n");
  if(oggpack_bits(&o)!=count*8)
    report("\nERROR: read to but not past exact end reported bad bitcount.\n");

  /* does EOF trip properly after a single additional bit? */
  if(oggpack_read(&o,1)!=-1)
    report("\nERROR: read past exact end did not trip EOF.\n");
  if(oggpack_bits(&o)!=count*8)
    report("\nERROR: read past exact end reported bad bitcount.\n");
  
  /* does EOF stay set over additional bit reads? */
  for(i=0;i<=32;i++){
    if(oggpack_read(&o,i)!=-1)
      report("\nERROR: EOF did not stay set on stream.\n");
    if(oggpack_bits(&o)!=count*8)
      report("\nERROR: read past exact end reported bad bitcount.\n");
  }
}       

void _end_verify2(int count){
  int i;

  /* are the proper number of bits left over? */
  int leftover=count*8-oggpack_bits(&o);
  if(leftover>7)
    report("\nERROR: too many bits reported left over.\n");
  
  /* does reading to exactly byte alignment *not* trip EOF? */
  oggpack_adv(&o,leftover);
  if(o.headend!=0)
    report("\nERROR: read to but not past exact end tripped EOF.\n");
  if(oggpack_bits(&o)!=count*8)
    report("\nERROR: read to but not past exact end reported bad bitcount.\n");
  
  /* does EOF trip properly after a single additional bit? */
  oggpack_adv(&o,1);
  if(o.headend>=0)
    report("\nERROR: read past exact end did not trip EOF.\n");
  if(oggpack_bits(&o)!=count*8)
    report("\nERROR: read past exact end reported bad bitcount.\n");
  
  /* does EOF stay set over additional bit reads? */
  for(i=0;i<=32;i++){
    oggpack_adv(&o,i);
    if(o.headend>=0)
      report("\nERROR: EOF did not stay set on stream.\n");
    if(oggpack_bits(&o)!=count*8)
      report("\nERROR: read past exact end reported bad bitcount.\n");
  }
}       

long ogg_buffer_length(ogg_reference *or){
  int count=0;
  while(or){
    count+=or->length;
    or=or->next;
  }
  return count;
}

ogg_reference *ogg_buffer_extend(ogg_reference *or,long bytes){
  if(or){
    while(or->next){
      or=or->next;
    }
    or->next=ogg_buffer_alloc(or->buffer->ptr.owner,bytes);
    return(or->next);
  }
  return 0;
}

void ogg_buffer_posttruncate(ogg_reference *or,long pos){
  /* walk to the point where we want to begin truncate */
  while(or && pos>or->length){
    pos-=or->length;
    or=or->next;
  }
  if(or){
    ogg_buffer_release(or->next);
    or->next=0;
    or->length=pos;
  }
}

#endif
