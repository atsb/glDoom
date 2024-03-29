// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// DESCRIPTION:
//	Fixed point arithemtics, implementation.
//
//-----------------------------------------------------------------------------

#ifndef __M_FIXED__
#define __M_FIXED__

#ifdef __GNUG__
#pragma interface
#endif

#include <stdint.h>

//
// Fixed point, 32bit as 16.16.
//
#define FRACBITS		16
#define FRACUNIT		(1<<FRACBITS)

typedef int32_t fixed_t;

#define FIXED_MAX   INT32_MAX
#define FIXED_MIN   INT32_MIN

fixed_t FixedMul(fixed_t a, fixed_t b);
fixed_t FixedDiv(fixed_t a, fixed_t b);

#define FIXED_TO_FLOAT_MULT    (1.0f / ((float)FRACUNIT))
#define FIXED_TO_FLOAT(x)      (((float)(x)) * FIXED_TO_FLOAT_MULT)

#define FLOAT_TO_FIXED(x)      ((fixed_t)((x) * (float)(FRACUNIT))

#endif
