/**
 * @ingroup   emu68_type68_devel
 * @file      emu68/type68.h
 * @brief     Type definitions header.
 * @date      1999/03/13
 * @author    Benjamin Gerard <ben@sashipa.com>
 *
 * $Id: type68.h 503 2005-06-24 08:52:56Z loke $
 *
 */

/* Copyright (C) 1998-2003 Benjamin Gerard */

#ifndef _TYPE68_H_
#define _TYPE68_H_

#include "../config_type68.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup  emu68_type68_devel  Type definitions
 *  @ingroup   emu68_devel
 *
 *   Definition of types used by EMU68 and SC68 related projects.
 *   Some of them are probably not neccessary and should be remove to
 *   improve execution time on 64 or more bit platforms.
 *   Setting wrong types may probably produce annoying effects on EMU68.
 *   It is a limitation to EMU68 portability.
 *
 *  @{
 */

/** @name Fixed size integer types.
 *  @{
 */

//typedef TYPE_U8 u8;        /**< Must be an unsigned 8 bit integer. */
//typedef TYPE_S8 s8;        /**< Must be an   signed 8 bit integer. */

//typedef TYPE_U16 u16;      /**< Must be an unsigned 16 bit integer. */
//typedef TYPE_S16 s16;      /**< Must be an   signed 16 bit integer. */

//typedef TYPE_U32 u32;      /**< Must be an unsigned 32 bit integer. */
//typedef TYPE_S32 s32;      /**< Must be an   signed 32 bit integer. */

//typedef TYPE_U64 u64;      /**< Must be an unsigned 64 bit integer. */
//typedef TYPE_S64 s64;      /**< Must be an   signed 64 bit integer. */

/**@}*/

/** Used by cycle counters. */
#ifndef TYPE_CYCLE68
# if SIZEOF_INT >= 4
#  define TYPE_CYCLE68 unsigned int
# elif SIZEOF_LONG >= 4
#  define TYPE_CYCLE68 unsigned long
# elif SIZEOF_LONG_LONG >= 4
#  define TYPE_CYCLE68 unsigned long long
# else
#  error "Could not find a type for cycle68_t"
# endif
#endif

typedef TYPE_CYCLE68 cycle68_t; /**< At least 32 bit integer. */

/**
 *@{
 */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _TYPE68_H_ */

