.arch	armv5te
.cpu	arm946e-s
.section    .text
.arm

.global MPUSetSpecial
.type   MPUSetSpecial STT_FUNC
MPUSetSpecial:
	push {r0-r12,lr}
	
	bl setTGDSProjectSpecial0xFFFF0000MPUSettings	//use default BIOS vectors
	
	pop {r0-r12,lr}
	bx	lr
	
.pool
.end