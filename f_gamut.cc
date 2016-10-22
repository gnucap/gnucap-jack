/*                             -*- C++ -*-
 * Copyright (C) 2016 Felix Salfelder
 * Author: same
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
 */

#include <u_parameter.h>
#include <u_function.h>
#include <globals.h>

// -uf compat hack
#ifndef _U_FUNC
typedef std::string fun_t;
#define to_fun_t to_string
#endif


namespace{

class fgamut : public FUNCTION { //
public:
	fun_t eval(CS& Cmd, const CARD_LIST* Scope)const
	{
		PARAMETER<double> n;
		PARAMETER<double> base;
		Cmd >> n;
		if(!Cmd.more()){ untested();
			base = 1.;
		}else{ untested();
			Cmd >> base;
			base.e_val(NOT_INPUT, Scope);
		}
		n.e_val(NOT_INPUT, Scope);
		base.e_val(1, Scope);
		return to_fun_t(gamut(n)*base);
	}
private:
	double gamut(double n) const
	{
		return pow(2., n/12.);
	}
} p_gam;
DISPATCHER<FUNCTION>::INSTALL d_gam(&function_dispatcher, "gam|gamut", &p_gam);
}
