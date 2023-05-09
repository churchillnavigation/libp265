/*
 * H.265 video codec parser.
 * Copyright (c) 2023 John Willard <john.willard@shotover.com>
 *
 * This file is part of libp265.
 *
 * libp265 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * libp265 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libp265.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "libp265/context.h"

#include <cstring>

BEGIN_NAMESPACE_LIBP265

void error_queue::add_warning(P265_error warning, bool once)
{
  // check if warning was already shown
  bool add=true;
  if (once) {
    for (int i=0;i<nWarningsShown;i++) {
      if (warnings_shown[i] == warning) {
        add=false;
        break;
      }
    }
  }

  if (!add) {
    return;
  }


  // if this is a one-time warning, remember that it was shown

  if (once) {
    if (nWarningsShown < MAX_WARNINGS) {
      warnings_shown[nWarningsShown++] = warning;
    }
  }


  // add warning to output queue

  if (nWarnings == MAX_WARNINGS) {
    warnings[MAX_WARNINGS-1] = P265_WARNING_WARNING_BUFFER_FULL;
    return;
  }

  warnings[nWarnings++] = warning;
}

error_queue::error_queue()
{
  nWarnings = 0;
  nWarningsShown = 0;
}

P265_error error_queue::get_warning()
{
  if (nWarnings==0) {
    return P265_OK;
  }

  P265_error warn = warnings[0];
  nWarnings--;
  memmove(warnings, &warnings[1], nWarnings*sizeof(P265_error));

  return warn;
}

END_NAMESPACE_LIBP265
