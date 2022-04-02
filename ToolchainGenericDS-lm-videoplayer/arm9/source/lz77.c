#include "typedefsTGDS.h"
#include "lz77.h"

__attribute__((section(".itcm")))
int lzssDecompress(u8* source, u8* destination) {
	u32 leng = *(unsigned int *)source >> 8;
	int Offs = 4;
	int dstoffs = 0;
	while (true){
		u8 header = source[Offs++];
		int i = 0; 
		for (i = 0; i < 8; i++){
			if ((header & 0x80) == 0) destination[dstoffs++] = source[Offs++];
			else
			{
				u8 a = source[Offs++];
				u8 b = source[Offs++];
				int offs = (((a & 0xF) << 8) | b) + 1;
				int length = (a >> 4) + 3;
				int j = 0; 
				for (j = 0; j < length; j++){
					destination[dstoffs] = destination[dstoffs - offs];
					dstoffs++;
				}
			}
			if (dstoffs >= (int)leng) return leng;
			header <<= 1;
		}
	}
	return (int)leng;
}
