/*$Id: d_meter.cc,v 26.138 2013/04/24 02:44:30 al Exp $ -*- C++ -*-
 * Copyright (C) 2010 Albert Davis
 * Author: Albert Davis <aldavis@gnu.org>
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
 * 2-port "meter" device
 * does nothing to the circuit, but has probes
 */

#include "globals.h"
#include "e_elemnt.h"
#include "u_xprobe.h"
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
class DEV : public ELEMENT {
private:
  explicit DEV(const DEV& p) :ELEMENT(p) {untested();}
public:
  explicit DEV()		:ELEMENT() {untested();}
private: // override virtual
  char	   id_letter()const	{untested();return '\0';}
  std::string value_name()const {untested();return "";}
  std::string dev_type()const	{untested();return "meter";}
  int	   max_nodes()const	{untested();return 2;}
  int	   min_nodes()const	{untested();return 2;}
  int	   matrix_nodes()const	{untested();return 2;}
  int	   net_nodes()const	{untested();return 2;}
  CARD*	   clone()const		{return new DEV(*this);}
  void	   tr_iwant_matrix()	{untested();}
  void	   ac_iwant_matrix()	{untested();}
  void     precalc_last();
  double   tr_involts()const	{untested();return dn_diff(_n[OUT1].v0(), _n[OUT2].v0());}
  double   tr_involts_limited()const {untested();return tr_involts();}
  COMPLEX  ac_involts()const	{untested();return _n[OUT1]->vac() - _n[OUT2]->vac();}
  double   tr_probe_num(const std::string&)const;
  XPROBE   ac_probe_ext(const std::string&)const;

  std::string port_name(int i)const {untested();
    assert(i >= 0);
    assert(i < 4);
    static std::string names[] = {"outp", "outn", "inp", "inn"};
    return names[i];
  }
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void DEV::precalc_last()
{ untested();
  ELEMENT::precalc_last();
  set_constant(true);
  set_converged();
}
/*--------------------------------------------------------------------------*/
double DEV::tr_probe_num(const std::string& x)const
{ untested();
  if (Umatch(x, "gain ")) { untested();
    return tr_outvolts() / tr_involts();
  }else{ untested();
    return ELEMENT::tr_probe_num(x);
  }
}
/*--------------------------------------------------------------------------*/
XPROBE DEV::ac_probe_ext(const std::string& x)const
{ untested();
  if (Umatch(x, "gain ")) { untested();
    return XPROBE(ac_outvolts() / ac_involts());
  }else{ untested();
    return ELEMENT::ac_probe_ext(x);
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
DEV p1;
DISPATCHER<CARD>::INSTALL d1(&device_dispatcher, "meter", &p1);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
