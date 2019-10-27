#ifndef _FLAC_INCLUDED
#define _FLAC_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "decoderflac.h"

bool flac_init(FILE *df, FLACContext* fc);

#ifdef __cplusplus
}
#endif

#endif
