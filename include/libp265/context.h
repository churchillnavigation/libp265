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

#ifndef LIBP265_CONTEXT_H
#define LIBP265_CONTEXT_H

#include "libp265/libp265.h"
#include "libp265/sps.h"
#include "libp265/pps.h"
#include "libp265/vps.h"

#include <memory>

BEGIN_NAMESPACE_LIBP265

#define MAX_WARNINGS 20

#define P265_MAX_VPS_SETS 16   // this is the maximum as defined in the standard
#define P265_MAX_SPS_SETS 16   // this is the maximum as defined in the standard
#define P265_MAX_PPS_SETS 64   // this is the maximum as defined in the standard

class error_queue
{
 public:
    LIBP265_API error_queue();

    LIBP265_API void add_warning(P265_error warning, bool once);
    LIBP265_API P265_error get_warning();

 private:
    P265_error warnings[MAX_WARNINGS];
    int nWarnings;
    P265_error warnings_shown[MAX_WARNINGS]; // warnings that have already occurred
    int nWarningsShown;
};

class parse_context : public error_queue
{
public:

    parse_context() = default;
    virtual ~parse_context() = default;

    virtual bool has_sps(int id) { return (bool)sps[id]; }
    virtual bool has_pps(int id) { return (bool)pps[id]; }

    virtual std::shared_ptr<seq_parameter_set> get_shared_sps(int id) { return sps[id]; }
    virtual std::shared_ptr<pic_parameter_set> get_shared_pps(int id) { return pps[id]; }

    /* */ seq_parameter_set* get_sps(int id)       { return sps[id].get(); }
    const seq_parameter_set* get_sps(int id) const { return sps[id].get(); }
    /* */ pic_parameter_set* get_pps(int id)       { return pps[id].get(); }
    const pic_parameter_set* get_pps(int id) const { return pps[id].get(); }

    virtual void set_sps(int id, std::shared_ptr<seq_parameter_set> sps) { this->sps[id] = sps; }
    virtual void set_pps(int id, std::shared_ptr<pic_parameter_set> pps) { this->pps[id] = pps; }

private:

    std::shared_ptr<video_parameter_set>  vps[ P265_MAX_VPS_SETS ];
    std::shared_ptr<seq_parameter_set>    sps[ P265_MAX_SPS_SETS ];
    std::shared_ptr<pic_parameter_set>    pps[ P265_MAX_PPS_SETS ];
};

END_NAMESPACE_LIBP265

#endif
