/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 *
 * Copyright (C) 2006-2007 Adam Gashlin (hcs)
 * Copyright (C) 2004-2007 Shay Green (blargg)
 * Copyright (C) 2002 Brad Martin
 *
 * All files in this archive are subject to the GNU General Public License.
 * See the file COPYING in the source tree root for full license agreement.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/

/* lovingly ripped off from Game_Music_Emu 0.5.2. http://www.slack.net/~ant/ */
/* DSP Based on Brad Martin's OpenSPC DSP emulator */
/* tag reading from sexyspc by John Brawn (John_Brawn@yahoo.com) and others */

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "spc.h"
#include "sound.h"
#include "misc.h"

/* rather than comment out asserts, just define NDEBUG */
#define NDEBUG
#include <assert.h>
#undef check
#define check assert

#ifdef CPU_ARM
    #undef  ICODE_ATTR
    #define ICODE_ATTR

    #undef  IDATA_ATTR
    #define IDATA_ATTR
	
	#undef	ICONST_ATTR
	#define ICONST_ATTR
	
	#undef	IBSS_ATTR
	#define IBSS_ATTR
#endif

/* TGB is the only target fast enough for gaussian and realtime BRR decode */
/* echo is almost fast enough but not quite */

/* Cache BRR waves */
#define SPC_BRRCACHE 0

/* Disable gaussian interpolation */
#define SPC_NOINTERP 0

/* Disable echo processing */
#define SPC_NOECHO 0

/* Samples per channel per iteration */
/*
#ifdef CPU_COLDFIRE
#define WAV_CHUNK_SIZE 1024
#else
#define WAV_CHUNK_SIZE 2048
#endif
*/

#define WAV_CHUNK_SIZE (SPC_OUT_SIZE >> 2)

#define THIS struct Spc_Emu* const this

/**************** Little-endian handling ****************/

static inline unsigned get_le16( void const* p )
{
    return  ((unsigned char const*) p) [1] * 0x100u +
            ((unsigned char const*) p) [0];
}

static inline int get_le16s( void const* p )
{
    return  ((signed char const*) p) [1] * 0x100 +
            ((unsigned char const*) p) [0];
}

static inline void set_le16( void* p, unsigned n )
{
    ((unsigned char*) p) [1] = (unsigned char) (n >> 8);
    ((unsigned char*) p) [0] = (unsigned char) n;
}

#define GET_LE16( addr )        get_le16( addr )
#define SET_LE16( addr, data )  set_le16( addr, data )
#define INT16A( addr ) (*(uint16_t*) (addr))
#define INT16SA( addr ) (*(int16_t*) (addr))

#ifdef ROCKBOX_LITTLE_ENDIAN
    #define GET_LE16A( addr )       (*(uint16_t*) (addr))
    #define GET_LE16SA( addr )      (*( int16_t*) (addr))
    #define SET_LE16A( addr, data ) (void) (*(uint16_t*) (addr) = (data))
#else
    #define GET_LE16A( addr )       get_le16 ( addr )
    #define GET_LE16SA( addr )      get_le16s( addr )
    #define SET_LE16A( addr, data ) set_le16 ( addr, data )
#endif

typedef struct
{
    union {
        uint8_t padding1 [0x100];
        uint16_t align;
    } padding1 [1];
    uint8_t ram      [0x10000];
    uint8_t padding2 [0x100];
} SNESram;

SNESram *ram = NULL;

#include "Spc_Dsp.h"

#undef RAM
#define RAM ram[0].ram

/**************** Timers ****************/

enum { timer_count = 3 };

struct Timer
{
    long next_tick;
    int period;
    int count;
    int shift;
    int enabled;
    int counter;
};

static void Timer_run_( struct Timer* t, long time ) ICODE_ATTR;
static void Timer_run_( struct Timer* t, long time )
{
    /* when disabled, next_tick should always be in the future */
    assert( t->enabled ); 
    
    int elapsed = ((time - t->next_tick) >> t->shift) + 1;
    t->next_tick += elapsed << t->shift;
    
    elapsed += t->count;
    if ( elapsed >= t->period ) /* avoid unnecessary division */
    {
        int n = elapsed / t->period;
        elapsed -= n * t->period;
        t->counter = (t->counter + n) & 15;
    }
    t->count = elapsed;
}

static inline void Timer_run( struct Timer* t, long time )
{
    if ( time >= t->next_tick )
        Timer_run_( t, time );
}

/**************** SPC emulator ****************/
/* 1.024 MHz clock / 32000 samples per second */
enum { clocks_per_sample = 32 }; 

enum { extra_clocks = clocks_per_sample / 2 };

/* using this disables timer (since this will always be in the future) */
enum { timer_disabled_time = 127 };

enum { rom_size = 64 };
enum { rom_addr = 0xFFC0 };

struct cpu_regs_t
{
    long pc; /* more than 16 bits to allow overflow detection */
    uint8_t a;
    uint8_t x;
    uint8_t y;
    uint8_t status;
    uint8_t sp;
};


struct Spc_Emu
{
    uint8_t cycle_table [0x100];
    struct cpu_regs_t r;
    
    int32_t* sample_buf;
    long next_dsp;
    int rom_enabled;
    int extra_cycles;
    
    struct Timer timer [timer_count];
    
    /* large objects at end */
    struct Spc_Dsp dsp;
    uint8_t extra_ram [rom_size];
    uint8_t boot_rom  [rom_size];
};

static void SPC_enable_rom( THIS, int enable )
{
    if ( this->rom_enabled != enable )
    {
        this->rom_enabled = enable;
        memcpy( RAM + rom_addr, (enable ? this->boot_rom : this->extra_ram), rom_size );
        /* TODO: ROM can still get overwritten when DSP writes to echo buffer */
    }
}

static void SPC_Init( THIS )
{
    this->timer [0].shift = 4 + 3; /* 8 kHz */
    this->timer [1].shift = 4 + 3; /* 8 kHz */
    this->timer [2].shift = 4; /* 8 kHz */
    
    /* Put STOP instruction around memory to catch PC underflow/overflow. */
    memset( ram[0].padding1, 0xFF, sizeof ram[0].padding1 );
    memset( ram[0].padding2, 0xFF, sizeof ram[0].padding2 );
    
    /* A few tracks read from the last four bytes of IPL ROM */
    this->boot_rom [sizeof this->boot_rom - 2] = 0xC0;
    this->boot_rom [sizeof this->boot_rom - 1] = 0xFF;
    memset( this->boot_rom, 0, sizeof this->boot_rom - 2 );
}

static void SPC_load_state( THIS, struct cpu_regs_t const* cpu_state,
        const void* new_ram, const void* dsp_state )
{
    memcpy(&(this->r),cpu_state,sizeof this->r);
        
    /* ram */
    memcpy( RAM, new_ram, sizeof RAM );
    memcpy( this->extra_ram, RAM + rom_addr, sizeof this->extra_ram );
    
    /* boot rom (have to force enable_rom() to update it) */
    this->rom_enabled = !(RAM [0xF1] & 0x80);
    SPC_enable_rom( this, !this->rom_enabled );
    
    /* dsp */
    /* some SPCs rely on DSP immediately generating one sample */
    this->extra_cycles = 32; 
    DSP_reset( &this->dsp );
    int i;
    for ( i = 0; i < register_count; i++ )
        DSP_write( &this->dsp, i, ((uint8_t const*) dsp_state) [i] );
    
    /* timers */
    for ( i = 0; i < timer_count; i++ )
    {
        struct Timer* t = &this->timer [i];
        
        t->next_tick = -extra_clocks;
        t->enabled = (RAM [0xF1] >> i) & 1;
        if ( !t->enabled )
            t->next_tick = timer_disabled_time;
        t->count = 0;
        t->counter = RAM [0xFD + i] & 15;
        
        int p = RAM [0xFA + i];
        if ( !p )
            p = 0x100;
        t->period = p;
    }
    
    /* Handle registers which already give 0 when read by
       setting RAM and not changing it.
       Put STOP instruction in registers which can be read,
       to catch attempted execution. */
    RAM [0xF0] = 0;
    RAM [0xF1] = 0;
    RAM [0xF3] = 0xFF;
    RAM [0xFA] = 0;
    RAM [0xFB] = 0;
    RAM [0xFC] = 0;
    RAM [0xFD] = 0xFF;
    RAM [0xFE] = 0xFF;
    RAM [0xFF] = 0xFF;
}

static void clear_echo( THIS )
{
    if ( !(DSP_read( &this->dsp, 0x6C ) & 0x20) )
    {
        unsigned addr = 0x100 * DSP_read( &this->dsp, 0x6D );
        size_t   size = 0x800 * DSP_read( &this->dsp, 0x7D );
        size_t max_size = sizeof RAM - addr;
        if ( size > max_size )
            size = sizeof RAM - addr;
        memset( RAM + addr, 0xFF, size );
    }
}

enum { spc_file_size = 0x10180 };

struct spc_file_t
{
    char    signature [27];
    char    unused [10];
    uint8_t pc [2];
    uint8_t a;
    uint8_t x;
    uint8_t y;
    uint8_t status;
    uint8_t sp;
    char    unused2 [212];
    uint8_t ram [0x10000];
    uint8_t dsp [128];
    uint8_t ipl_rom [128];
};

static int SPC_load_spc( THIS, const void* data, long size )
{
    struct spc_file_t const* spc = (struct spc_file_t const*) data;
    struct cpu_regs_t regs;
    
    if ( size < spc_file_size )
        return -1;
    
    if ( memcmp( spc->signature, "SNES-SPC700 Sound File Data", 27 ) != 0 )
        return -1;
    
    regs.pc     = spc->pc [1] * 0x100 + spc->pc [0];
    regs.a      = spc->a;
    regs.x      = spc->x;
    regs.y      = spc->y;
    regs.status = spc->status;
    regs.sp     = spc->sp;
    
    if ( (unsigned long) size >= sizeof *spc )
        memcpy( this->boot_rom, spc->ipl_rom, sizeof this->boot_rom );
    
    SPC_load_state( this, &regs, spc->ram, spc->dsp );
    
    clear_echo(this);
    
    return 0;
}

/**************** DSP interaction ****************/

static void SPC_run_dsp_( THIS, long time ) ICODE_ATTR;
static void SPC_run_dsp_( THIS, long time )
{
    /* divide by clocks_per_sample */
    int count = ((time - this->next_dsp) >> 5) + 1; 
    int32_t* buf = this->sample_buf;
    this->sample_buf = buf + count;
    this->next_dsp += count * clocks_per_sample;
    DSP_run( &this->dsp, count, buf );
}

static inline void SPC_run_dsp( THIS, long time )
{
    if ( time >= this->next_dsp )
        SPC_run_dsp_( this, time );
}

static int SPC_read( THIS, unsigned addr, long const time ) ICODE_ATTR;
static int SPC_read( THIS, unsigned addr, long const time )
{
    int result = RAM [addr];
    
    if ( ((unsigned) (addr - 0xF0)) < 0x10 )
    {
        assert( 0xF0 <= addr && addr <= 0xFF );
        
        /* counters */
        int i = addr - 0xFD;
        if ( i >= 0 )
        {
            struct Timer* t = &this->timer [i];
            Timer_run( t, time );
            result = t->counter;
            t->counter = 0;
        }
        /* dsp */
        else if ( addr == 0xF3 )
        {
            SPC_run_dsp( this, time );
            result = DSP_read( &this->dsp, RAM [0xF2] & 0x7F );
        }
    }
    return result;
}

static void SPC_write( THIS, unsigned addr, int data, long const time )
    ICODE_ATTR;
static void SPC_write( THIS, unsigned addr, int data, long const time )
{
    /* first page is very common */
    if ( addr < 0xF0 )
    {
        RAM [addr] = (uint8_t) data;
    }
    else switch ( addr )
    {
        /* RAM */
        default:
            if ( addr < rom_addr )
            {
                RAM [addr] = (uint8_t) data;
            }
            else
            {
                this->extra_ram [addr - rom_addr] = (uint8_t) data;
                if ( !this->rom_enabled )
                    RAM [addr] = (uint8_t) data;
            }
            break;
        
        /* DSP */
        /*case 0xF2:*/ /* mapped to RAM */
        case 0xF3: {
            SPC_run_dsp( this, time );
            int reg = RAM [0xF2];
            if ( reg < register_count ) {
                DSP_write( &this->dsp, reg, data );
            }
            else {
			
            }
            break;
        }
        
        case 0xF0: /* Test register */
            break;
        
        /* Config */
        case 0xF1:
        {
            int i;
            /* timers */
            for ( i = 0; i < timer_count; i++ )
            {
                struct Timer * t = this->timer+i;
                if ( !(data & (1 << i)) )
                {
                    t->enabled = 0;
                    t->next_tick = timer_disabled_time;
                }
                else if ( !t->enabled )
                {
                    /* just enabled */
                    t->enabled = 1;
                    t->counter = 0;
                    t->count = 0;
                    t->next_tick = time;
                }
            }
            
            /* port clears */
            if ( data & 0x10 )
            {
                RAM [0xF4] = 0;
                RAM [0xF5] = 0;
            }
            if ( data & 0x20 )
            {
                RAM [0xF6] = 0;
                RAM [0xF7] = 0;
            }
            
            SPC_enable_rom( this, (data & 0x80) != 0 );
            break;
        }
        
        /* Ports */
        case 0xF4:
        case 0xF5:
        case 0xF6:
        case 0xF7:
            /* to do: handle output ports */
            break;

        /* verified on SNES that these are read/write (RAM) */
        /*case 0xF8: */
        /*case 0xF9: */
        
        /* Timers */
        case 0xFA:
        case 0xFB:
        case 0xFC: {
            int i = addr - 0xFA;
            struct Timer* t = &this->timer [i];
            if ( (t->period & 0xFF) != data )
            {
                Timer_run( t, time );
                this->timer[i].period = data ? data : 0x100;
            }
            break;
        }
        
        /* Counters (cleared on write) */
        case 0xFD:
        case 0xFE:
        case 0xFF:
            this->timer [addr - 0xFD].counter = 0;
            break;
    }
}

#include "Spc_Cpu.h"

/**************** Sample generation ****************/

static int SPC_play( THIS, long count, int32_t* out ) ICODE_ATTR;
static int SPC_play( THIS, long count, int32_t* out )
{
    int i;
    assert( count % 2 == 0 ); /* output is always in pairs of samples */
    
    long start_time = -(count >> 1) * clocks_per_sample - extra_clocks;
    
    /* DSP output is made on-the-fly when DSP registers are read or written */
    this->sample_buf = out;
    this->next_dsp = start_time + clocks_per_sample;
    
    /* Localize timer next_tick times and run them to the present to prevent
       a running but ignored timer's next_tick from getting too far behind
       and overflowing. */
    for ( i = 0; i < timer_count; i++ )
    {
        struct Timer* t = &this->timer [i];
        if ( t->enabled )
        {
            t->next_tick += start_time + extra_clocks;
            Timer_run( t, start_time );
        }
    }
    
    /* Run from start_time to 0, pre-advancing by extra cycles from last run */
    this->extra_cycles = CPU_run( this, start_time + this->extra_cycles ) +
                         extra_clocks;
    if ( this->extra_cycles < 0 )
    {   
        return -1;
    }
    
    /* Catch DSP up to present */
#if 0
    ENTER_TIMER(cpu);
#endif
    SPC_run_dsp( this, -extra_clocks );
#if 0
    EXIT_TIMER(cpu);
#endif
    assert( this->next_dsp == clocks_per_sample - extra_clocks );
    assert( this->sample_buf - out == count );
    
    return 0;
}

/**************** ID666 parsing ****************/

struct {
    unsigned char isBinary;
    char song[32];
    char game[32];
    char dumper[16];
    char comments[32];
    int day,month,year;
    unsigned long length;
    unsigned long fade;
    char artist[32];
    unsigned char muted;
    unsigned char emulator;
} ID666;

static int LoadID666(unsigned char *buf) {
    unsigned char *ib=buf;
    int isbinary = 1;
    int i;
  
    memcpy(ID666.song,ib,32);
    ID666.song[31]=0;
    ib+=32;

    memcpy(ID666.game,ib,32);
    ID666.game[31]=0;
    ib+=32;

    memcpy(ID666.dumper,ib,16);
    ID666.dumper[15]=0;
    ib+=16;

    memcpy(ID666.comments,ib,32);
    ID666.comments[31]=0;
    ib+=32;

    /* Ok, now comes the fun part. */
   
    /* Date check */
    if(ib[2] == '/'  && ib[5] == '/' )
        isbinary = 0;
  
    /* Reserved bytes check */
    if(ib[0xD2 - 0x2E - 112] >= '0' &&
       ib[0xD2 - 0x2E - 112] <= '9' &&
       ib[0xD3 - 0x2E - 112] == 0x00)
        isbinary = 0;
    
    /* is length & fade only digits? */
    for (i=0;i<8 && ( 
        (ib[0xA9 - 0x2E - 112+i]>='0'&&ib[0xA9 - 0x2E - 112+i]<='9') ||
        ib[0xA9 - 0x2E - 112+i]=='\0');
        i++);
    if (i==8) isbinary=0;
    
    ID666.isBinary = isbinary;
  
    if(isbinary) {
        ID666.year=*ib;
        ib++;
        ID666.year|=*ib<<8;
        ib++;
        ID666.month=*ib;
        ib++;    
        ID666.day=*ib;
        ib++;

        ib+=7;

        ID666.length=*ib;
        ib++;
    
        ID666.length|=*ib<<8;
        ib++;
    
        ID666.length|=*ib<<16;
        ID666.length*=1000;
        ib++;
    
        ID666.fade=*ib;
        ib++;
        ID666.fade|=*ib<<8; 
        ib++;
        ID666.fade|=*ib<<16;
        ib++;
        ID666.fade|=*ib<<24;
        ib++;

        memcpy(ID666.artist,ib,32);
        ID666.artist[31]=0;
        ib+=32;

        ID666.muted=*ib;
        ib++;

        ID666.emulator=*ib;
        ib++;    
    } else {
        unsigned long tmp;
        char buf[64];
        
        memcpy(buf, ib, 2);
        buf[2] = 0; 
        tmp = 0;
        for (i=0;i<2 && buf[i]>='0' && buf[i]<='9';i++) tmp=tmp*10+buf[i]-'0';
        ID666.month = tmp;
        ib+=3;
        
        memcpy(buf, ib, 2);
        buf[2] = 0; 
        tmp = 0;
        for (i=0;i<2 && buf[i]>='0' && buf[i]<='9';i++) tmp=tmp*10+buf[i]-'0';
        ID666.day = tmp;
        ib+=3;
        
        memcpy(buf, ib, 4);
        buf[4] = 0; 
        tmp = 0;
        for (i=0;i<4 && buf[i]>='0' && buf[i]<='9';i++) tmp=tmp*10+buf[i]-'0';
        ID666.year = tmp;
        ib+=5;
    
        memcpy(buf, ib, 3);
        buf[3] = 0; 
        tmp = 0;
        for (i=0;i<3 && buf[i]>='0' && buf[i]<='9';i++) tmp=tmp*10+buf[i]-'0';
        ID666.length = tmp * 1000;
        ib+=3;

        memcpy(buf, ib, 5);
        buf[5] = 0;
        tmp = 0;
        for (i=0;i<5 && buf[i]>='0' && buf[i]<='9';i++) tmp=tmp*10+buf[i]-'0';
        ID666.fade = tmp;
        ib+=5;

        memcpy(ID666.artist,ib,32);
        ID666.artist[31]=0;
        ib+=32;

        /*I have no idea if this is right or not.*/
        ID666.muted=*ib;
        ib++;

        memcpy(buf, ib, 1);
        buf[1] = 0;
        tmp = 0;
        ib++;
    }
    return 1;
}

static struct Spc_Emu spc_emu IDATA_ATTR;
static int32_t *samples = NULL;
static char tmpYear[20];

bool spcInit(uint8_t* buffer, u32 buffersize)
{
	ram = (SNESram *)trackMalloc(sizeof(SNESram), "snes ram");

#if SPC_BRRCACHE
	BRRcache = (int16_t *)trackMalloc((0x20000 + 32) * sizeof(int16_t), "snes brrcache");
#endif

    memcpy(spc_emu.cycle_table, cycle_table, sizeof cycle_table );

	SPC_Init(&spc_emu);
	if(SPC_load_spc(&spc_emu,buffer,buffersize)) 
		return false;
	
	LoadID666(buffer+0x2e);
	
	if(ID666.year != 0)
		sprintf(tmpYear, "%d", ID666.year);
	else
		strcpy(tmpYear, "");
	
	samples = (int32_t *)trackMalloc(WAV_CHUNK_SIZE*2*sizeof(int32_t), "spc render buffer");
	
	return true;
}

bool spcPlay(s16 *lBuf, s16 *rBuf)
{
    if(SPC_play(&spc_emu,WAV_CHUNK_SIZE*2,samples))
		return false;

    //ci->pcmbuf_insert(samples, samples+WAV_CHUNK_SIZE, WAV_CHUNK_SIZE);
	
	int i;
	
	for(i=0;i<WAV_CHUNK_SIZE;++i)
	{
		lBuf[i] = samples[i] >> 8;
		rBuf[i] = samples[i+WAV_CHUNK_SIZE] >> 8;
	}
	
	return true;
}

void spcFree()
{
	if(samples)
		trackFree(samples);
	
	samples = NULL;
	
	if(ram)
		trackFree(ram);
	
	ram = NULL;

#if SPC_BRRCACHE	
	if(BRRcache)
		trackFree(BRRcache);

	BRRcache = NULL;
#endif
}

char *getSPCMeta(int which)
{
	switch(which)
	{
		case 0:
			return ID666.song;
		case 1:
			return ID666.game;
		case 2:
			return ID666.artist;
		case 3:
			return tmpYear;
		case 4:
			return ID666.comments;
	}
	
	return NULL;
}

