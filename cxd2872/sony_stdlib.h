/*------------------------------------------------------------------------------
  Copyright 2010-2012 Sony Corporation

  Last Updated  : 2012/03/14
  File Revision : 1.0.0.0
------------------------------------------------------------------------------*/

#ifndef SONY_STDLIB_H
#define SONY_STDLIB_H

#include "sony_common.h"

/*
 <PORTING> Please modify if ANCI C standard library is not available.
*/

#include <string.h>

#define sony_memcpy  memcpy
#define sony_memset  memset

#endif /* SONY_STDLIB_H */
