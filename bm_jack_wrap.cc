/*                      -*- C++ -*-
 * Copyright (C) 2013-2016 Felix Salfelder
 * Author: Felix Salfelder <felix@salfelder.org>
 *
 * This file is part of "Gnucap", the Gnu Circuit Analysis Package
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *------------------------------------------------------------------
 */

// tmp
#define USE(x)

#include <globals.h>
#include <e_compon.h>
#ifndef HAVE_UINT_T
typedef int uint_t;
#endif

namespace {//
using std::string;

class DEV_MEAS_JACK : public COMPONENT { //
	public:
		string value_name() const { return "dummy"; }
		string port_name(uint_t) const { return "dummy"; }

		CARD* clone()const
		{ untested();
			const CARD* c = device_dispatcher["meter"];
			if(!c){ untested();
				error(bDANGER, "meter is unavailable\n");
			}else{ untested();
			}
			assert(c);
			CARD* c2 = c->clone();
			COMPONENT* d = prechecked_cast<COMPONENT*>(c2);
			assert(d);
			const COMMON_COMPONENT* b = bm_dispatcher["jack"];
			assert(b);
			COMMON_COMPONENT* bc = b->clone();
			d->attach_common(bc);
			d->set_dev_type("m_jack");
			// assert(d->dev_type() == "m_jack"); not yet
			return d;
		}
		bool print_type_in_spice()const {return false;}
}p1;
DISPATCHER<CARD>::INSTALL d1(&device_dispatcher, "to_jack|m_jack|meas_jack", &p1);

class DEV_V_JACK : public COMPONENT { //
	public:
		string value_name() const { return "dummy"; }
		string port_name(uint_t) const { return "dummy"; }

		CARD* clone()const
		{ untested();
			const CARD* c = device_dispatcher["V"];
			assert(c);
			CARD* c2 = c->clone();
			COMPONENT* d = prechecked_cast<COMPONENT*>(c2);
			assert(d);
			const COMMON_COMPONENT* b = bm_dispatcher["jack"];
			assert(b);
			COMMON_COMPONENT* bc = b->clone();
			d->attach_common(bc);
			d->set_dev_type("v_jack");
			assert(d->dev_type() == "v_jack");
			return d;
		}
		bool print_type_in_spice()const {return false;}
}p2;
DISPATCHER<CARD>::INSTALL d2(&device_dispatcher, "v_jack", &p2);
}
