	.TEXT
	.ARM
	.ALIGN 4

#include "spcdefs.h"

@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Function called with:
@ r0 - Raw brr data (s8*)
@ r1 - Decoded sample data (s16*)
@ r2 - DspChannel *channel
@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Function Data:
@ r4 - sample1
@ r5 - sample2
@ r6,r7 - tmp
@ r8 - shift amount
@ r9 - number of iterations left
@ r10 - 0xf
@ r11 - low clip
@ r12 - high clip

#define PREV0 r2
#define PREV1 r3
#define SAMPLE1 r4
#define SAMPLE2 r5
#define BRR_TAB r8
#define ITER_LEFT r9
#define CONST_F r10

.GLOBAL brrHash
brrHash:
.word 0

.extern apuCacheSamples
@required to remove audio cracks!
.GLOBAL DecodeSampleBlockAsmUncached
DecodeSampleBlockAsmUncached:
    stmfd sp!, {r4-r12,r14}
    @ Save the channel pointer
    mov r14, r2
	
	b finishDecode1

.GLOBAL DecodeSampleBlockAsm
DecodeSampleBlockAsm:
    stmfd sp!, {r4-r12,r14}

    @ Save the channel pointer
    mov r14, r2
	
	@ Hash the block, and skip the decode if we can
    ldmia r0, {r4-r7}
    ldr r8, =0x050C5D1F @ (2166136261 * 16777619)
    ldr r9, =16777619
    eor r8, r8, r4
    mul r8, r9, r8
    eor r8, r8, r5
    mul r8, r9, r8
    eor r8, r8, r6
    @ r8 is the hashed value
    ldr r4, brrHash
    @ Compute the actual brr location minus the apu ram base
    ldr r6, =APU_MEM
    ldr r6, [r6]
    sub r6, r0, r6
    @ Compute the address of the cached samples in brrHash
    add r3, r4, #0x8000
    mov r6, r6, lsr #3
    add r3, r3, r6, lsl #5
    @ Load the previous hash value
    ldr r5, [r4, r6, lsl #2]
    str r8, [r4, r6, lsl #2]
    cmp r5, r8
    bne doDecode
    @ Load the cached samples
    ldmia r3, {r4-r11}
    stmia r1!, {r4-r11}
    ldrsh PREV0, [r1, #-2]
    ldrsh PREV1, [r1, #-4]
    b doneDecodeCached
	
doDecode:
finishDecode1:
    stmfd sp!, {r3}

	ldrh r4, [r14, #54]
	strh r4, [r14, #22]
	
    @ Load prev0 and prev1
    ldrsh PREV0, [r14, #62]
    ldrsh PREV1, [r14, #64]
    
    ldrb r4, [r0], #1
    @ Compute the index into the brrTab to load the bytes from
    mov r9, r4, lsr #4
    ldr r8, =brrTab
    add r8, r8, r9, lsl #5 @ brrTabPtr = brrTab + (r4 * 32)

    mov CONST_F, #0xf << 1
    ldr r11, =0xffff8000
    ldr r12, =0x7fff

    @ 16 samples to decode, but do two at a time
    mov ITER_LEFT, #8
    @ Figure out the type of decode filter
    mov r4, r4, lsr #2
    and r4, r4, #3
    ldr pc, [pc, r4, lsl #2]
    nop
.word case0
.word case1
.word case2
.word case3
case0:
    ldrb r4, [r0], #1
    and r5, CONST_F, r4, lsl #1
    ldrsh SAMPLE2, [BRR_TAB, r5]
    and r4, CONST_F, r4, lsr #3
    ldrsh SAMPLE1, [BRR_TAB, r4]

    mov r4, r4, lsl #1
    mov r5, r5, lsl #1
    strh r4, [r1], #2
    strh r5, [r1], #2

    subs ITER_LEFT, ITER_LEFT, #1
    bne case0

    @ Set up prev0 and prev1
    ldrsh PREV0, [r1, #-2]
    ldrsh PREV1, [r1, #-4]
    
    b doneDecode

case1:
    ldrb r4, [r0], #1
    and r5, CONST_F, r4, lsl #1
    ldrsh SAMPLE2, [BRR_TAB, r5]
    and r4, CONST_F, r4, lsr #3
    ldrsh SAMPLE1, [BRR_TAB, r4]

    @ prev1 = sample1 + (last1 >> 1) - (last1 >> 5)    
    add PREV1, SAMPLE1, PREV0, asr #1
    sub PREV1, PREV1, PREV0, asr #5
    
    cmp PREV1, r12
    movgt PREV1, r12
    cmp PREV1, r11
    movlt PREV1, r11

    mov PREV1, PREV1, lsl #1
    strh PREV1, [r1], #2
    ldrsh PREV1, [r1, #-2]

    @ same for prev0 now
    add PREV0, SAMPLE2, PREV1, asr #1
    sub PREV0, PREV0, PREV1, asr #5

    cmp PREV0, r12
    movgt PREV0, r12
    cmp PREV0, r11
    movlt PREV0, r11

    mov PREV0, PREV0, lsl #1
    strh PREV0, [r1], #2
    ldrsh PREV0, [r1, #-2]

    subs ITER_LEFT, ITER_LEFT, #1
    bne case1

    b doneDecode

case2:
    ldrb r4, [r0], #1
    and r5, CONST_F, r4, lsl #1
    ldrsh SAMPLE2, [BRR_TAB, r5]
    and r4, CONST_F, r4, lsr #3
    ldrsh SAMPLE1, [BRR_TAB, r4]

    @ Sample 1
    mov r6, PREV1, asr #1
    rsb r6, r6, PREV1, asr #5
    mov PREV1, PREV0
    add r7, PREV0, PREV0, asr #1
    rsb r7, r7, #0
    add r6, r6, r7, asr #5
    add r7, SAMPLE1, PREV0
    add PREV0, r6, r7

    cmp PREV0, r12
    movgt PREV0, r12
    cmp PREV0, r11
    movlt PREV0, r11
    mov PREV0, PREV0, lsl #1
    strh PREV0, [r1], #2
    ldrsh PREV0, [r1, #-2]

    @ Sample 2
    mov r6, PREV1, asr #1
    rsb r6, r6, PREV1, asr #5
    mov PREV1, PREV0
    add r7, PREV0, PREV0, asr #1
    rsb r7, r7, #0
    add r6, r6, r7, asr #5
    add r7, SAMPLE2, PREV0
    add PREV0, r6, r7

    cmp PREV0, r12
    movgt PREV0, r12
    cmp PREV0, r11
    movlt PREV0, r11
    mov PREV0, PREV0, lsl #1
    strh PREV0, [r1], #2
    ldrsh PREV0, [r1, #-2]
    
    subs ITER_LEFT, ITER_LEFT, #1
    bne case2

    b doneDecode

case3:
    ldrb r4, [r0], #1
    and r5, CONST_F, r4, lsl #1
    ldrsh SAMPLE2, [BRR_TAB, r5]
    and r4, CONST_F, r4, lsr #3
    ldrsh SAMPLE1, [BRR_TAB, r4]

    @ Sample 1
    add r6, PREV1, PREV1, asr #1
    mov r6, r6, asr #4
    sub r6, r6, PREV1, asr #1
    mov PREV1, PREV0
    add r7, PREV0, PREV0, lsl #2
    add r7, r7, PREV0, lsl #3
    rsb r7, r7, #0
    add r6, r6, r7, asr #7
    add r6, r6, PREV0
    add PREV0, SAMPLE1, r6

    cmp PREV0, r12
    movgt PREV0, r12
    cmp PREV0, r11
    movlt PREV0, r11
    mov PREV0, PREV0, lsl #1
    strh PREV0, [r1], #2
    ldrsh PREV0, [r1, #-2]

    @ Sample 2
    add r6, PREV1, PREV1, asr #1
    mov r6, r6, asr #4
    sub r6, r6, PREV1, asr #1
    mov PREV1, PREV0
    add r7, PREV0, PREV0, lsl #2
    add r7, r7, PREV0, lsl #3
    rsb r7, r7, #0
    add r6, r6, r7, asr #7
    add r6, r6, PREV0
    add PREV0, SAMPLE2, r6

    cmp PREV0, r12
    movgt PREV0, r12
    cmp PREV0, r11
    movlt PREV0, r11
    mov PREV0, PREV0, lsl #1
    strh PREV0, [r1], #2
    ldrsh PREV0, [r1, #-2]

    subs ITER_LEFT, ITER_LEFT, #1
    bne case3

doneDecode:
    sub r1, r1, #32
    ldmia r1, {r4-r11}
    ldmfd sp!, {r1}
    stmia r1, {r4-r11}

doneDecodeCached:
    @ Store prev0 and prev1
    strh PREV0, [r14, #62]
    strh PREV1, [r14, #64]

    ldmfd sp!, {r4-r12,r14}
    bx lr

// Envelope state definitions
#define ENVSTATE_NONE       0
#define ENVSTATE_ATTACK		1
#define ENVSTATE_DECAY		2
#define ENVSTATE_SUSTAIN	3
#define ENVSTATE_RELEASE	4
#define ENVSTATE_DIRECT		5
#define ENVSTATE_INCREASE	6
#define ENVSTATE_BENTLINE	7
#define ENVSTATE_DECREASE	8
#define ENVSTATE_DECEXP		9

@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Function called with:
@ r0 - int Number of samples to mix
@ r1 - u16* mix buffer (left first, right is always MIXBUFSIZE * 4 bytes ahead
@@@@@@@@@@@@@@@@@@@@@@@@@@@@

/*
struct DspChannel {
int sampleSpeed;        0
int samplePos;          4
int envCount;           8
int envSpeed;           12
s16 prevDecode[4];      16
s16 decoded[16];        24
u16 decaySpeed;         56
u16 sustainSpeed;       58
s16 envx;               60
s16 prevSamp1;          62
s16 prevSamp2;          64
u16 blockPos;           66
s16 leftCalcVolume;     68
s16 rightCalcVolume;    70
u8 envState;            72
u8 sustainLevel;        73
s8 leftVolume;          74
s8 rightVolume;         75
s8 keyWait;             76
bool active;            77
u8 brrHeader;           78
bool echoEnabled;       79
};
*/

#define DSPCHANNEL_SIZE 80

#define SAMPLESPEED_OFFSET 0
#define SAMPLEPOS_OFFSET 4
#define ENVCOUNT_OFFSET 8
#define ENVSPEED_OFFSET 12
#define PREVDECODE_OFFSET 16
#define DECODED_OFFSET 24
#define DECAYSPEED_OFFSET 56
#define SUSTAINSPEED_OFFSET 58
#define ENVX_OFFSET 60
#define BLOCKPOS_OFFSET 66
#define LEFTCALCVOL_OFFSET 68
#define RIGHTCALCVOL_OFFSET 70
#define ENVSTATE_OFFSET 72
#define SUSTAINLEVEL_OFFSET 73
#define LEFTVOL_OFFSET 74
#define RIGHTVOL_OFFSET 75
#define KEYWAIT_OFFSET 76
#define ACTIVE_OFFSET 77
#define ECHOENABLED_OFFSET 79

@ r0 - channel structure base
@ r1 - mix buffer
@ r2 - echo buffer ptr
@ r3 - numSamples
@ r4 - sampleSpeed
@ r5 - samplePos
@ r6 - envCount
@ r7 - envSpeed
@ r8 - sampleValue (value of the current sample)
@ r9 - tmp
@ r10 - leftCalcVol
@ r11 - rightCalcVol
@ r12 - tmp
@ r13 - tmp
@ r14 - tmp

#define MIXBUFFER r1
#define ECHOBUFFER r2
#define SAMPLE_SPEED r4
#define SAMPLE_POS r5
#define ENV_COUNT r6
#define ENV_SPEED r7
#define SAMPLE_VALUE r8
#define LEFT_CALC_VOL r10
#define RIGHT_CALC_VOL r11

.GLOBAL DspMixSamplesStereo
.FUNC DspMixSamplesStereo
DspMixSamplesStereo:
    stmfd sp!, {r4-r12, lr}

    mov r3, #0
    strb r3, channelNum
    str r0, numSamples

    @ Store the original mix buffer for use later
    stmfd sp!, {r1}

    @ Clear the left and right mix buffers, saving their initial positions
    ldr r1, =mixBuffer
    ldr r2, =echoBuffer
    mov r3, #0
    mov r4, #0
    mov r5, #0
    mov r6, #0
clearLoop:
    stmia r1!, {r3-r6}
    stmia r1!, {r3-r6}
    stmia r2!, {r3-r6}
    stmia r2!, {r3-r6}
    subs r0, r0, #4
    cmp r0, #0
    bgt clearLoop

    @ Load the initial mix buffer and echo position
    ldr r1, =mixBuffer
    ldr r2, =echoBuffer

    ldr r0, =channels
channelLoopback:
    @ Check if active == 0, then next
    ldrb r3, [r0, #ACTIVE_OFFSET]
    cmps r3, #0
    beq nextChannelNothingDone

    @ Save the start position of the mix buffer & echo buffer
    stmfd sp!, {r1,r2}

    @ Get echo enabled, then replace the opcode there if it's enabled
    ldrb r14, [r0, #ECHOENABLED_OFFSET]
    cmp r14, #1
    ldr r3, =0x01A00000 @ mov r0, r0
    streq r3, branchLocation

    ldrb r3, numSamples
    @ Load the important variables into registers
    ldmia r0, {r4-r7}
    ldrsh LEFT_CALC_VOL, [r0, #LEFTCALCVOL_OFFSET]
    ldrsh RIGHT_CALC_VOL, [r0, #RIGHTCALCVOL_OFFSET]

mixLoopback:

    @ Commence the mixing
    subs ENV_COUNT, ENV_COUNT, ENV_SPEED
    bpl noEnvelopeUpdate

    @ Update envelope
    mov ENV_COUNT, #0x7800

    ldrsh r9, [r0, #ENVX_OFFSET]
    ldrb r12, [r0, #ENVSTATE_OFFSET]

    ldr pc, [pc, r12, lsl #2]
    nop
@ Jump table for envelope handling
.word noEnvelopeUpdate
.word envStateAttack
.word envStateDecay
.word envStateSustain
.word envStateRelease
.word noEnvelopeUpdate      @ Actually direct, but we don't need to do anything
.word envStateIncrease
.word envStateBentline
.word envStateDecrease
.word envStateSustain       @ Actually decrease exponential, but it's the same code

#define ENVX_SHIFT 8
#define ENVX_MAX 0x7f00

envStateAttack:
    add r9, r9, #4 << ENVX_SHIFT

    cmp r9, #ENVX_MAX
    ble storeEnvx
    @ envx = 0x7f, state = decay, speed = decaySpeed
    mov r9, #ENVX_MAX
    mov r12, #ENVSTATE_DECAY
    strb r12, [r0, #ENVSTATE_OFFSET]
    ldrh ENV_SPEED, [r0, #DECAYSPEED_OFFSET]
    b storeEnvx
    
envStateDecay:
    rsb r9, r9, r9, lsl #8
    mov r9, r9, asr #8

    ldrb r12, [r0, #SUSTAINLEVEL_OFFSET]
    cmp r9, r12, lsl #ENVX_SHIFT
    bge storeEnvx
    @ state = sustain, speed = sustainSpeed
    mov r12, #ENVSTATE_SUSTAIN
    strb r12, [r0, #ENVSTATE_OFFSET]
    ldrh ENV_SPEED, [r0, #SUSTAINSPEED_OFFSET]
    
    @ Make sure envx > 0
    cmp r9, #0
    bge storeEnvx
    
    @ If not, end channel, then go to next channel
    stmfd sp!, {r0-r3, r14}
    ldrb r0, channelNum
    bl DspSetEndOfSample
    ldmfd sp!, {r0-r3, r14}
    b nextChannel
    
envStateSustain:
    rsb r9, r9, r9, lsl #8
    mov r9, r9, asr #8

    @ Make sure envx > 0
    cmp r9, #0
    bge storeEnvx

    @ If not, end channel, then go to next channel
    stmfd sp!, {r0-r3,r14}
    ldrb r0, channelNum
    bl DspSetEndOfSample
    ldmfd sp!, {r0-r3,r14}
    b nextChannel

envStateRelease:
    sub r9, r9, #1 << ENVX_SHIFT

    @ Make sure envx > 0
    cmp r9, #0
    bge storeEnvx

    @ If not, end channel, then go to next channel
    stmfd sp!, {r0-r3,r14}
    ldrb r0, channelNum
    bl DspSetEndOfSample
    ldmfd sp!, {r0-r3,r14}
    b nextChannel

envStateIncrease:
    add r9, r9, #4 << ENVX_SHIFT

    cmp r9, #ENVX_MAX
    ble storeEnvx
    @ envx = 0x7f, state = direct, speed = 0
    mov r9, #ENVX_MAX
    mov r12, #ENVSTATE_DIRECT
    strb r12, [r0, #ENVSTATE_OFFSET]
    mov ENV_SPEED, #0
    b storeEnvx

envStateBentline:
    cmp r9, #0x5f << ENVX_SHIFT
    addgt r9, r9, #1 << ENVX_SHIFT
    addle r9, r9, #4 << ENVX_SHIFT

    cmp r9, #ENVX_MAX
    blt storeEnvx
    @ envx = 0x7f, state = direct, speed = 0
    mov r9, #ENVX_MAX
    mov r12, #ENVSTATE_DIRECT
    strb r12, [r0, #ENVSTATE_OFFSET]
    mov ENV_SPEED, #0
    b storeEnvx

envStateDecrease:
    sub r9, r9, #4 << ENVX_SHIFT

    @ Make sure envx > 0
    cmp r9, #0
    bge storeEnvx
    
    @ If not, end channel, then go to next channel
    stmfd sp!, {r0-r3,r14}
    ldrb r0, channelNum
    bl DspSetEndOfSample
    ldmfd sp!, {r0-r3,r14}
    b nextChannel

storeEnvx:
    strh r9, [r0, #ENVX_OFFSET]

    @ Recalculate leftCalcVol and rightCalcVol
    ldrsb LEFT_CALC_VOL, [r0, #LEFTVOL_OFFSET]
    mul LEFT_CALC_VOL, r9, LEFT_CALC_VOL
    mov LEFT_CALC_VOL, LEFT_CALC_VOL, asr #7

    ldrsb RIGHT_CALC_VOL, [r0, #RIGHTVOL_OFFSET]
    mul RIGHT_CALC_VOL, r9, RIGHT_CALC_VOL
    mov RIGHT_CALC_VOL, RIGHT_CALC_VOL, asr #7
    
noEnvelopeUpdate:
    add SAMPLE_POS, SAMPLE_POS, SAMPLE_SPEED
    cmp SAMPLE_POS, #16 << 12
    blo noSampleUpdate
    
    @ Decode next 16 bytes...
    sub SAMPLE_POS, SAMPLE_POS, #16 << 12

    @ Decode the sample block, r0 = DspChannel*
    stmfd sp!, {r0-r3, r14}
    bl DecodeSampleBlock
    cmps r0, #1
    ldmfd sp!, {r0-r3, r14}
    beq nextChannel

noSampleUpdate:
    @ This is really a >> 12 then << 1, but since samplePos bit 0 will never be set, it's safe.
    @ Must ensure that sampleSpeed bit 0 is never set, and samplePos is never set to anything but 0
    @ TODO - The speed up hack doesn't work.  Find out why
    mov r12, SAMPLE_POS, lsr #12
    add r12, r0, r12, lsl #1
    
handleLoPassFilter:
    @ interpolate
	ldrsh r8, [r12, #DECODED_OFFSET]
	ldrsh r14, [r12, #DECODED_OFFSET - 2]
	ldrsh r12, [r12, #DECODED_OFFSET - 4]
	
    and r9, SAMPLE_POS, #0x1
	rsb r9, r9, #0x1
	smlal r14,r8,r14,r12
	mul r14, r8, r9
	mov r8, r14, asr #2

branchLocation:
    @ DSP Echo Mixing
	ldmia r2, {r9,r12}
    mla r9, r8, LEFT_CALC_VOL, r9
    mla r12, r8, RIGHT_CALC_VOL, r12
    stmia r2!, {r9,r12}
    
    ldmia r1, {r9,r12}
    mla r9, r8, LEFT_CALC_VOL, r9
    mla r12, r8, RIGHT_CALC_VOL, r12
	stmia r1!, {r9,r12}
	
endMixEcho:
    subs r3, r3, #1
    bne mixLoopback

nextChannel:

    @ Set ENVX and OUTX
    ldrb r3, channelNum
    ldr r12, =DSP_MEM
    add r12, r12, r3, lsl #4

    @ Set ENVX
    ldrsh r9, [r0, #ENVX_OFFSET]
    mov r9, r9, asr #ENVX_SHIFT
    strb r9, [r12, #0x8]

    @ Set OUTX
    mul r9, r8, r9
    mov r9, r9, asr #15
    strb r9, [r12, #0x9]
    
    strh LEFT_CALC_VOL, [r0, #LEFTCALCVOL_OFFSET]
    strh RIGHT_CALC_VOL, [r0, #RIGHTCALCVOL_OFFSET]

    @ Store changing values
    stmia r0, {r4-r7}

    @ Reload mix&echo buffer position
    ldmfd sp!, {r1,r2}

nextChannelNothingDone:
    @ Move to next channel
    add r0, r0, #DSPCHANNEL_SIZE

    @ Increment channelNum
    ldrb r3, channelNum
    add r3, r3, #1
    strb r3, channelNum
    cmps r3, #8
    blt channelLoopback

@ This is the end of normal mixing

    @ Store the original mix & echo buffers, cause we trash these regs
    stmfd sp!, {r1, r2}

    @ r0 - 
    @ r1 - 
    @ r2 - 
    @ r3 - 
    @ r4 - 
    @ r5 - echo volume (right)
    @ r6 - numSamples
    @ r7 - echo in apu ram (r/w)
    @ r8 - echo mix buffer (r/w)
    @ r9 - end of echo in apu ram
    @ r10 - echo volume (left)
    @ r11 - echo feedback
    @ r12 - FIR coefficients in DSP ram
    @ r13 - FIR table base
    @ r14 - FIR offset


@ Process the echo filter stuff
echoMixSetup:
    mov r8, r2

    ldr r0, =DSP_MEM

    ldrsb r10, [r0, #0x2C] @ Get left echo volume
    mov r10, r10, lsl #7
    ldrsb r5, [r0, #0x3C] @ Get right echo volume
    mov r5, r5, lsl #7

    @ Get echo feedback
    ldrsb r11, [r0, #0x0D]

    @ Check if echo is enabled
    ldrb r1, [r0, #0x6C]
    strb r1, echoEnabled
    @ Get echo base (APU_MEM + DSP_ESA << 8)
    ldr r7, =echoBase
    ldr r7, [r7]
    str r7, echoBufferStart
    @ Set up end of echo delay area in r8
    ldr r0, =echoDelay
    ldrh r0, [r0]
    add r9, r7, r0

    @ Set up current echo cursor location
    ldr r0, =echoCursor
    ldrh r0, [r0]
    add r7, r7, r0

    ldr r14, =firOffset
    ldrb r14, [r14]

    @ Offset firTable to start at FIR #7
    ldr r12, =DSP_MEM
    add r12, r12, #0x7F

    ldr r6, numSamples

echoMixLoopback:
    @ Load the old echo value (l,r)
    ldrsh r0, [r7]
    ldrsh r1, [r7, #2]

    @ r0,r1 contains the filtered samples
    ldr r2, [r8]
    @ Left channel = (feedback * filtered) >> 7
    mla r2, r11, r0, r2
    mov r2, r2, asr #15

    ldr r3, [r8, #4]
    @ Right channel = (feedback * filtered) >> 7
    mla r3, r11, r1, r3
    mov r3, r3, asr #15

    @ Store (filtered * echoFB) + echobuffer into echobuffer
    ldrb r5, echoEnabled
    tst r5, #0x20
    streqh r2, [r7], #2
    streqh r3, [r7], #2
    cmp r7, r9
    ldrge r7, echoBufferStart

    @ Store (filtered * echoVol) into echomix
    mul r2, r10, r0
    str r2, [r8], #4
    mul r2, r5, r1
    str r2, [r8], #4

    subs r6, r6, #1
    bne echoMixLoopback
    
doneEchoMix:

    ldr r3, echoBufferStart
    sub r7, r7, r3
    ldr r0, =echoCursor
    strh r7, [r0]

    @ Reload mix buffer & echo positions
    ldmfd sp!, {r1, r2}
    
clipAndMix:
    @ Put the original output buffer into r3
    ldmfd sp!, {r3}
    
    @ Set up the preamp & overall volume
    ldr r8, =dspPreamp
    ldrh r8, [r8]

    ldr r9, =DSP_MEM
    ldrsb r4, [r9, #0x0C] @ Main left volume
    ldrsb r6, [r9, #0x1C] @ Main right volume
    
    mul r4, r8, r4
    mov r4, r4, asr #7
    mul r6, r8, r6
    mov r6, r6, asr #7

    @ r0 - numSamples
    @ r1 - mix buffer
    @ r2 - echo buffer
    @ r3 - output buffer
    @ r4 - left volume
    @ r5 - TMP (assigned to sample value)
    @ r6 - right volume
    @ r7 - TMP
    @ r8 - preamp
    @ r9 - 
    @ r10 - 
    @ r11 - 
    @ r12 - 
    @ r14 - 

    @ Do volume multiplication, mix in echo buffer and clipping here
    ldr r0, numSamples

mixClipLoop:
    @ Load and scale by volume (LEFT)
    ldr r5, [r1], #4
    mov r5, r5, asr #15
    mul r5, r4, r5
    ldr r7, [r2], #4
    add r5, r5, r7, asr #7
    mov r5, r5, asr #7

    @ Clip and store
    cmp r5, #0x7f00
    movgt r5, #0x7f00
    cmn r5, #0x7f00
    movlt r5, #0x8100
	ldr r7, =(MIXBUFSIZE * 4)
    strh r5, [r3], r7

    @ Load and scale by volume (RIGHT)
    ldr r5, [r1], #4
    mov r5, r5, asr #15
    mul r5, r6, r5
    ldr r7, [r2], #4
    add r5, r5, r7, asr #7
    mov r5, r5, asr #7

    @ Clip and store
    cmp r5, #0x7f00
    movgt r5, #0x7f00
    cmn r5, #0x7f00
    movlt r5, #0x8100
    
	ldr r7, =( 2 - (MIXBUFSIZE * 4))
	strh r5, [r3], r7

    subs r0, r0, #1
    bne mixClipLoop

doneMix:
    ldmfd sp!, {r4-r12, lr}
    bx lr
.ENDFUNC

.GLOBAL channelNum

tmpSp:
.word 0
echoBufferStart:
.word 0
numSamples:
.word 0
channelNum:
.byte 0
echoEnabled:
.byte 0

.align
.pool
