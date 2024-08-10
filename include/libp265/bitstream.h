/*
 * H.265 video codec.
 * Copyright (c) 2013-2014 struktur AG, Dirk Farin <farin@struktur.de>
 *
 * This file is part of libde265.
 *
 * libde265 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * libde265 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libde265.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef P265_BITSTREAM_H
#define P265_BITSTREAM_H

#include "libp265/libp265.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#ifdef HAVE_STDBOOL_H
#include <stdbool.h>
#endif
#include <stdint.h>

BEGIN_NAMESPACE_LIBP265


#define MAX_UVLC_LEADING_ZEROS 20
#define UVLC_ERROR -99999


typedef struct {
  uint8_t* data;
  int bytes_remaining;

  uint64_t nextbits; // left-aligned bits
  int nextbits_cnt;
} bitreader;

LIBP265_API void bitreader_init(bitreader*, unsigned char* buffer, int len);
LIBP265_API void bitreader_refill(bitreader*); // refill to at least 56+1 bits
LIBP265_API int  next_bit(bitreader*);
LIBP265_API int  next_bit_norefill(bitreader*);
LIBP265_API int  get_bits(bitreader*, int n);
LIBP265_API int  get_bits_fast(bitreader*, int n);
LIBP265_API int  peek_bits(bitreader*, int n);
LIBP265_API void skip_bits(bitreader*, int n);
LIBP265_API void skip_bits_fast(bitreader*, int n);
LIBP265_API void skip_to_byte_boundary(bitreader*);
LIBP265_API void prepare_for_CABAC(bitreader*);
LIBP265_API int  get_uvlc(bitreader*);  // may return UVLC_ERROR
LIBP265_API int  get_svlc(bitreader*);  // may return UVLC_ERROR

LIBP265_API bool check_rbsp_trailing_bits(bitreader*); // return true if remaining filler bits are all zero

END_NAMESPACE_LIBP265

#endif
