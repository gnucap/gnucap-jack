/*                      -*- C++ -*-
 * Copyright (C) 2013 Felix Salfelder
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
#include "globals.h"
#include "l_dispatcher.h"
#include "u_sim_data.h"
#include "u_status.h"
#include "u_prblst.h"
#include "u_parameter.h"
#include "ap.h"
#include "u_time_pair.h"
#include "declare.h"	/* gen */
#include "u_opt.h"
#include "c_comand.h"
#include "io_.h"
#include "s__.h"
/*--------------------------------------------------------------------------*/
namespace {
#include "s_tr.h"
  TRANSIENT p5;
  DISPATCHER<CMD>::INSTALL      d5(&command_dispatcher, "jack", &p5);
/*--------------------------------------------------------------------------*/
int TRANSIENT::steps_accepted_;
int TRANSIENT::steps_rejected_;
int TRANSIENT::steps_total_;
int TRANSIENT::steps_total_out_; //??

bool skipreview = true; // hmmm
/*--------------------------------------------------------------------------*/
void TRANSIENT::do_it(CS& Cmd, CARD_LIST* Scope)
{
  trace0("JACK::do_it");
  _scope = Scope;
  _sim->set_command_tran();
  ::status.tran.reset().start();
  command_base(Cmd);
  _scope = NULL;
  ::status.tran.stop();
}
/*--------------------------------------------------------------------------*/
std::string TRANSIENT::status()const
{
  return "transient timesteps: accepted=" + to_string(steps_accepted())
    + ", rejected=" + to_string(steps_rejected())
    + ", total=" + to_string(steps_total()) + "\n";  
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
//	void	TRANSIENT::setup(CS&);
//	void	TRANSIENT::options(CS&);
/*--------------------------------------------------------------------------*/
/* tr_setup: transient analysis: parse command string and set options
 * 	(options set by call to tr_options)
 */

PARAMETER<unsigned> _samplerate;

void TRANSIENT::setup(CS& Cmd)
{
  _samplerate.e_val(44600, _scope);
  if (Cmd.match1("'\"({") || Cmd.is_pfloat()) {
    trace1("TRANSIENT::setup parsing args", printlist().size());
    PARAMETER<unsigned> arg1;
    Cmd >> arg1;
    arg1.e_val(44600,_scope);
    
    if (arg1.has_hard_value()) {
      _samplerate = arg1;
    }
  }

  trace0("JACK::setup");
  options(Cmd);

  _sim->_freq = _samplerate;
  _tstep = 1/_sim->_freq;
  _dtmax = _tstep;
  _sim->_dtmin = _dtmax;
  _tstart = 0;
  _sim->_time0 = 0;
  _sim->_dt0 = 0;
}
/*--------------------------------------------------------------------------*/
/* tr_options: set options common to transient and fourier analysis
 */
void TRANSIENT::options(CS& Cmd)
{
  unsigned int sr = 48e3;
  trace0("JACK::options");
  _out = IO::mstdout;
  _out.reset(); //BUG// don't know why this is needed
  _sim->_temp_c = OPT::temp_c;
  bool ploton = IO::plotset  &&  plotlist().size() > 0;
  _sim->_uic = _cold = false;
  _trace = tNONE;
  unsigned here = Cmd.cursor();
  do{
    ONE_OF
      || Get(Cmd, "c{old}",	   &_cold)
      || Get(Cmd, "dte{mp}",	   &_sim->_temp_c,  mOFFSET, OPT::temp_c)
      || Get(Cmd, "dtma{x}",	   &_dtmax_in)
      || Get(Cmd, "dtmi{n}",	   &_dtmin_in)
      || Get(Cmd, "dtr{atio}",	   &_dtratio_in)
      || Get(Cmd, "pl{ot}",	   &ploton)
      || Get(Cmd, "sk{ip}",	   &_skip_in)
      || Get(Cmd, "te{mperature}", &_sim->_temp_c)
      || Get(Cmd, "uic",	   &_sim->_uic)
      || (Cmd.umatch("sample{rate} {=}") &&
	  (ONE_OF
	   || Set(Cmd, "48000",    &sr, 48000u)
	   || Set(Cmd, "48k",      &sr, 48000u)
	   || Set(Cmd, "24000",    &sr, 24000u)
	   || Set(Cmd, "24k",      &sr, 24000u)
	   || Cmd.warn(bWARNING, "need 48k, or 24k")
	   )
	  )
      || (Cmd.umatch("tr{ace} {=}") &&
	  (ONE_OF
	   || Set(Cmd, "n{one}",      &_trace, tNONE)
	   || Set(Cmd, "o{ff}",       &_trace, tNONE)
	   || Set(Cmd, "w{arnings}",  &_trace, tUNDER)
	   || Set(Cmd, "a{lltime}",   &_trace, tALLTIME)
	   || Set(Cmd, "r{ejected}",  &_trace, tREJECTED)
	   || Set(Cmd, "i{terations}",&_trace, tITERATION)
	   || Set(Cmd, "v{erbose}",   &_trace, tVERBOSE)
	   || Cmd.warn(bWARNING, "need none, off, warnings, alltime, "
		       "rejected, iterations, verbose")
	   )
	  )
      || outset(Cmd,&_out)
      ;
  }while (Cmd.more() && !Cmd.stuck(&here));
  Cmd.check(bWARNING, "what's this?");

  IO::plotout = (ploton) ? IO::mstdout : OMSTREAM();
  initio(_out);

  _dtmax_in.e_val(BIGBIG, _scope);
  _dtmin_in.e_val(OPT::dtmin, _scope);
  _dtratio_in.e_val(OPT::dtratio, _scope);
  _skip_in.e_val(1, _scope);

  _samplerate = sr;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
//	void	TRANSIENT::sweep(void);
//	void	TRANSIENT::first(void);
//	bool	TRANSIENT::next(void);
//	void	TRANSIENT::accept(void);
//	void	TRANSIENT::reject(void);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
namespace TR {
  static std::string step_cause[] = {
    "impossible",
    "user requested",
    "event queue",
    "command line \"skip\"",
    "convergence failure, reducing (itl4)",
    "slow convergence, holding (itl3)",
    "truncation error",
    "ambiguous event",
    "limit growth",
    "initial guess"
  };
}
/*--------------------------------------------------------------------------*/
void TRANSIENT::sweep()
{
  trace0("JACK::sweep");
  _sim->_phase = p_INIT_DC;
  head(_tstart, _tstop, "Time");
  _sim->_bypass_ok = false;
  _sim->set_inc_mode_bad();
  
  if (_cont) {  // use the data from last time
    _sim->_phase = p_RESTORE;
    _sim->restore_voltages();
    CARD_LIST::card_list.tr_restore();
  }else{
    _sim->clear_limit();
    CARD_LIST::card_list.tr_begin();
  }
  
  first();
  _sim->_genout = gen();
  
  if (_sim->uic_now()) {
    advance_time();
    _sim->zero_voltages();
    CARD_LIST::card_list.do_tr();    //evaluate_models
    while (!_sim->_late_evalq.empty()) {itested(); //BUG// encapsulation violation
      _sim->_late_evalq.front()->do_tr_last();
      _sim->_late_evalq.pop_front();
    }
    _converged = true;
    _sim->_loadq.clear(); // fake solve, clear the queue
    //BUG// UIC needs further analysis.
  }else{
    _converged = solve_with_homotopy(OPT::DCBIAS,_trace);
    if (!_converged) {
      error(bWARNING, "did not converge\n");
    }else{
    }
  }
  review(); 
  _accepted = true;
  accept();
  
  {
    bool printnow = (_sim->_time0 == _tstart || _trace >= tALLTIME);
    if (printnow) {
      _sim->keep_voltages();
      outdata(_sim->_time0);
    }else{untested();
    }
  }
  
  while (next()) {
    trace2("loop", step_cause(), _sim->_iter[p_TRAN]);
    _sim->count_iterations(p_TRAN); // after accept??
    _sim->_bypass_ok = false;
    _sim->_phase = p_TRAN;
    _sim->_genout = gen();
    _converged = solve(OPT::TRHIGH,_trace);

    _accepted = _converged && review();

    if (_accepted) {
      assert(_converged);
      assert(_sim->_dt0 <= _time_by_user_request);
      accept();
      if (step_cause() == scUSER) {
	assert(up_order(_sim->_time0-_sim->_dtmin, _time_by_user_request, _sim->_time0+_sim->_dtmin));
	++_stepno;
	_time_by_user_request = _tstep;	/* advance user time */
      }else{ untested();
	_time_by_user_request -= _sim->_dt0;
      }
      assert(0 < _time_by_user_request);
    }else{ untested();
      reject();
      assert(time1 < _time_by_user_request);
    }
    {
      bool printnow =
	   (_trace >= tREJECTED)
	|| (_accepted && ((_trace >= tALLTIME) 
			  || (step_cause() == scUSER && _sim->_time0+_sim->_dtmin > _tstart)));
      if (_sim->_iter[p_TRAN] & ((1U << 12) - 1)) {
      } else if (printnow) {
	_sim->keep_voltages();
	outdata(_sim->_time0);
      }else{
      }
    }
    
    if (!_converged && OPT::quitconvfail) {untested();
      outdata(_sim->_time0);
      throw Exception("convergence failure, giving up");
    }else{
    }
  }
}
/*--------------------------------------------------------------------------*/
void TRANSIENT::set_step_cause(STEP_CAUSE C)
{
  switch (C) {
  case scITER_A:untested();
  case scADT:untested();
  case scITER_R:
  case scINITIAL:
  case scSKIP:
  case scTE:
  case scAMBEVENT:
  case scEVENTQ:
  case scUSER:
    ::status.control = C;
    break;
  case scNO_ADVANCE:untested();
  case scZERO:untested();
  case scSMALL:itested();
  case scREJECT:
    ::status.control += C;
    break;
  default:
    unreachable();
  }
}
/*--------------------------------------------------------------------------*/
int TRANSIENT::step_cause()const
{
  return ::status.control;
}
/*--------------------------------------------------------------------------*/
void TRANSIENT::first()
{
  /* usually, _sim->_time0, time1 == 0, from setup */
  assert(_sim->_time0 == time1);
  assert(_sim->_time0 <= _tstart);
  _time_by_user_request = _tstep;
  while (!_sim->_eq.empty()) {
    _sim->_eq.pop();
  }
  _stepno = 0;
  set_step_cause(scUSER);
  ++::status.hidden_steps;
}
/*--------------------------------------------------------------------------*/
#define check_consistency() {						\
    trace4("", __LINE__, newtime, almost_fixed_time, fixed_time);	\
    assert(almost_fixed_time <= fixed_time);				\
    assert(newtime <= fixed_time);					\
    /*assert(newtime == fixed_time || newtime <= fixed_time -_sim->_dtmin);*/	\
    assert(newtime <= almost_fixed_time);				\
    /*assert(newtime == almost_fixed_time || newtime <= almost_fixed_time - _sim->_dtmin);*/ \
    assert(newtime > time1);						\
    assert(newtime > reftime);						\
    assert(new_dt > 0.);						\
    assert(new_dt >= _sim->_dtmin);						\
    assert(newtime <= _time_by_user_request);				\
    /*assert(newtime == _time_by_user_request*/				\
    /*	   || newtime < _time_by_user_request - _sim->_dtmin);	*/	\
  }
#define check_consistency2() {						\
    assert(newtime > time1);						\
    assert(new_dt > 0.);						\
    assert(new_dt >= _sim->_dtmin);						\
    assert(newtime <= _time_by_user_request);				\
    /*assert(newtime == _time_by_user_request	*/			\
    /*	   || newtime < _time_by_user_request - _sim->_dtmin);*/		\
  }
/*--------------------------------------------------------------------------*/
/* next: go to next time step
 * Set _sim->_time0 to the next time step, store the old one in time1.
 * Try several methods.  Take the one that gives the shortest step.
 */
bool TRANSIENT::next()
{
  double _dt_by_user_request = _time_by_user_request;

  double old_dt = _sim->_time0 - time1;
  assert(old_dt >= 0);
  
  double new_dt = _dt_by_user_request;
  double newtime = _sim->_time0 + new_dt;
  STEP_CAUSE new_control = scNO_ADVANCE;

  if (_sim->_dt0 == 0) { untested();
    assert(time1==0);
    new_dt = _dtmax/2;
    newtime = _sim->_time0 + new_dt;
    new_control = scINITIAL;
  }else if (!_converged) { incomplete();
    new_dt = old_dt + _dtmax;
    newtime = _sim->_time0 + new_dt;
    new_control = scITER_R;
    new_control = scUSER;
  }else{
    new_dt = std::min(_dtmax,new_dt);
    newtime = _sim->_time0 + new_dt;
    new_control = scUSER;
  }

  set_step_cause(new_control);

  /* got it, I think */

  _sim->_dt0 = new_dt;
  _sim->_time0 = new_dt;
  ++::status.hidden_steps;
  ++steps_total_;
  return (true);
}
/*--------------------------------------------------------------------------*/
bool TRANSIENT::review()
{
  _sim->count_iterations(iTOTAL);

  if (skipreview){
    return true;
  }
  incomplete();

  TIME_PAIR time_by = CARD_LIST::card_list.tr_review();
  _dt_by_error_estimate = time_by._error_estimate;

  // limit minimum time step
  // 2*_sim->_dtmin because _time[1] + _sim->_dtmin might be == _time[0].
  if (time_by._event < time1 + 2*_sim->_dtmin) {
    _dt_by_ambiguous_event = time1 + 2*_sim->_dtmin;
  }else{
    _dt_by_ambiguous_event = time_by._event;
  }
  // force advance when time too close to previous
  if (std::abs(_dt_by_ambiguous_event - _sim->_time0) < 2*_sim->_dtmin) {
    _dt_by_ambiguous_event = _sim->_time0 + 2*_sim->_dtmin;
  }else{
  }

  if (time_by._error_estimate < time1 + 2*_sim->_dtmin) {
    _dt_by_error_estimate = time1 + 2*_sim->_dtmin;
  }else{
    _dt_by_error_estimate = time_by._error_estimate;
  }
  if (std::abs(_dt_by_error_estimate - _sim->_time0) < 1.1*_sim->_dtmin) {
    _dt_by_error_estimate = _sim->_time0 + 1.1*_sim->_dtmin;
  }else{
  }

  return (_dt_by_error_estimate > _sim->_time0  &&  _dt_by_ambiguous_event > _sim->_time0);
}
/*--------------------------------------------------------------------------*/
void TRANSIENT::accept()
{
  ++_sim->_timestep;
  _sim->set_limit();
  if (OPT::traceload && !skipreview) { itested();
    while (!_sim->_acceptq.empty()) {
      _sim->_acceptq.back()->tr_accept();
      _sim->_acceptq.pop_back();
    }
  }else{ untested();
    _sim->_acceptq.clear();
    CARD_LIST::card_list.tr_accept();
  }
  ++steps_accepted_;
}
/*--------------------------------------------------------------------------*/
void TRANSIENT::reject()
{
  ::status.accept.start();
  _sim->_acceptq.clear();
  ++steps_rejected_;
  ::status.accept.stop();
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
}
