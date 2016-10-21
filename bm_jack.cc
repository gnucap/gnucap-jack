/*                              -*- C++ -*-
 * Copyright (C) 2013 Felix Salfelder
 * Author: Felix Salfelder
 *
 * This file is part of "gnucap-jack".
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
 * JACK bm plugin
 */

//hack
#define hp(x)
//hack
#define USE(x)

#include "e_elemnt.h"
#include "globals.h"
#include "u_lang.h"
#include "l_denoise.h"
#include "bm.h"

extern "C"{
  #include <jack/jack.h>
  #include <jack/transport.h>
  #include <sys/types.h>
  #include <unistd.h>
}

#include <boost/assign.hpp>

/*--------------------------------------------------------------------------*/
static const unsigned MAXPORTS = 16;
typedef jack_default_audio_sample_t sample_t;
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
using std::map;
using std::string;
/*--------------------------------------------------------------------------*/
const int _default_direction (0);
const int _default_connect (0);
/*--------------------------------------------------------------------------*/
static int srate(jack_nframes_t nframes, void *ctx);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class EVAL_BM_JACK : public EVAL_BM_ACTION_BASE {
private:
  PARAMETER<int> _direction;
  PARAMETER<unsigned> _connect;
  PARAMETER<double> _end;
  void set_param_by_name(string Name, string Value);

  static map<string, PARA_BASE EVAL_BM_JACK::*> _param_dict;
  explicit	EVAL_BM_JACK(const EVAL_BM_JACK& p);
public:
  explicit      EVAL_BM_JACK(int c=0);
		~EVAL_BM_JACK()		{}
private: // override vitrual
  bool		operator==(const COMMON_COMPONENT&)const;
  COMMON_COMPONENT* clone()const	{return new EVAL_BM_JACK(*this);}
  void		print_common_obsolete_callback(OMSTREAM&, LANGUAGE*)const;

  void		precalc_first(const CARD_LIST*);
  void		precalc_last(const CARD_LIST*);
  void		tr_eval(ELEMENT*)const;
  TIME_PAIR	tr_review(COMPONENT*)const;
  void tr_accept(COMPONENT*)const;
  std::string	name()const		{return "jack";}
  bool		ac_too()const		{return true;}
  bool		parse_numlist(CS&);
  bool		parse_params_obsolete_callback(CS&);
  jack_port_t *_jack_port;
  mutable sample_t* _buffer;
  mutable volatile unsigned _pos;
public:
  void expand(const COMPONENT* d);
private:
  static int process_cb(jack_nframes_t nframes, void* ctx);
  static int srate_cb(jack_nframes_t nframes, void* ctx);
  int process(jack_nframes_t nframes);
  friend int srate(jack_nframes_t, void*);
  int srate(jack_nframes_t nframes);
  static EVAL_BM_JACK* _ctx[MAXPORTS];
private: // common in all commons.
  static unsigned long _sr;
  static jack_client_t *_client;
  static unsigned _nframes;
  static unsigned short _samples_per_step;
};
/*--------------------------------------------------------------------------*/
unsigned long EVAL_BM_JACK::_sr;
jack_client_t* EVAL_BM_JACK::_client;
unsigned EVAL_BM_JACK::_nframes;
unsigned short EVAL_BM_JACK::_samples_per_step;
/*--------------------------------------------------------------------------*/
EVAL_BM_JACK* EVAL_BM_JACK::_ctx[MAXPORTS];
/*--------------------------------------------------------------------------*/
EVAL_BM_JACK::EVAL_BM_JACK(int c)
  :EVAL_BM_ACTION_BASE(c),
	_direction(0),
	_connect(0),
	_jack_port(0),
	_buffer(0)
{
}
/*--------------------------------------------------------------------------*/
EVAL_BM_JACK::EVAL_BM_JACK(const EVAL_BM_JACK& p)
  :EVAL_BM_ACTION_BASE(p),
	_direction(p._direction),
	_connect(p._connect),
	_jack_port(p._jack_port),
	_buffer(0)
{
}
/*--------------------------------------------------------------------------*/
bool EVAL_BM_JACK::operator==(const COMMON_COMPONENT& x)const
{
	return false; // buffer in COMMON...
	const EVAL_BM_JACK* p = dynamic_cast<const EVAL_BM_JACK*>(&x);
	bool rv = p
		&& _connect == p->_connect
		&& _direction == p->_direction;
	return rv;
}
/*--------------------------------------------------------------------------*/
void EVAL_BM_JACK::print_common_obsolete_callback(OMSTREAM& o, LANGUAGE* lang)const
{
  assert(lang);
  o << name();
  print_pair(o, lang, "direction",  _direction);
  EVAL_BM_ACTION_BASE::print_common_obsolete_callback(o, lang);
}
/*--------------------------------------------------------------------------*/
void EVAL_BM_JACK::precalc_first(const CARD_LIST* Scope)
{
  assert(Scope);
  EVAL_BM_ACTION_BASE::precalc_first(Scope);
  _direction.e_val(_default_direction, Scope);
  _connect.e_val(_default_connect, Scope);
}
/*--------------------------------------------------------------------------*/
void EVAL_BM_JACK::precalc_last(const CARD_LIST*)
{
	_samples_per_step = (short unsigned) (double(_sr) * CKT_BASE::_sim->_dtmin / 0.9999);
	error(bTRACE, "precalc_last sr " + to_string(unsigned(_sr)) + "\n");
	error(bTRACE, "precalc_last dtmin " + to_string(CKT_BASE::_sim->_dtmin) + "\n");
	error(bTRACE, "precalc_last samples_per_step " + to_string(_samples_per_step) + "\n");
}
/*--------------------------------------------------------------------------*/
int EVAL_BM_JACK::process_cb(jack_nframes_t nframes, void* ctx)
{
	unsigned ret = 0;
	EVAL_BM_JACK** c = (EVAL_BM_JACK**) ctx;
	while(*c) {
		ret += (*c)->process(nframes);
		++c;
	}
	return ret;
}
/*--------------------------------------------------------------------------*/
int EVAL_BM_JACK::process(jack_nframes_t nframes)
{
	assert(_nframes==nframes);

	if (_pos < nframes){
		error(bWARNING, "jack lag %i of %i\n", _pos, nframes);
	} else {
		// cerr << "." << _buffer[13];
	}
	sample_t *buffer = (sample_t *) jack_port_get_buffer (_jack_port, nframes);

	if (jack_port_flags(_jack_port) & JackPortIsOutput) {
		memcpy (buffer, _buffer, sizeof (sample_t) * nframes);
	} else {
		memcpy (_buffer, buffer, sizeof (sample_t) * nframes);
	}

	_pos = 0;
	return 0;
}
/*--------------------------------------------------------------------------*/
int EVAL_BM_JACK::srate_cb(jack_nframes_t nframes, void *ctx)
{
	unsigned ret = 0;
	EVAL_BM_JACK** c = (EVAL_BM_JACK**) ctx;
	while(*c) {
		ret += (*c)->srate(nframes);
		++c;
	}
	return ret;
}
/*--------------------------------------------------------------------------*/
int EVAL_BM_JACK::srate(jack_nframes_t nframes)
{
	error(bDANGER, "srate %i\n", nframes);
	incomplete();
	return 0;
}
/*--------------------------------------------------------------------------*/
void EVAL_BM_JACK::expand(const COMPONENT* d)
{
	// trace3("expand", d->long_label(), hp(this), hp(d));
	pid_t p = getpid();
	string client_name = "gnucap [" + to_string(p) + "]";
	const char* port_name = d->long_label().c_str();

	if (_client) {
	} else { untested();
		if ((_client = jack_client_open (client_name.c_str(), JackNullOption, 0)) == 0) {
			fprintf (stderr, "jack server not running?\n");
			throw Exception("jack server not running?");
		}
		_sr = jack_get_sample_rate(_client);
		_nframes = jack_get_buffer_size(_client);

		error(bTRACE, "nframes: " + to_string(_nframes) + "\n");
		error(bTRACE, "samplerate " + to_string((int)_sr) + "\n");
		error(bTRACE, "dtmin " + to_string(d->_sim->_dtmin) + "\n");

		jack_set_sample_rate_callback(_client, srate_cb, (void*) _ctx);
		jack_set_process_callback (_client, process_cb, (void*) _ctx);
	}

	assert(_nframes);
	if (_buffer) { untested();
	}else{ untested();
		_buffer = new sample_t[_nframes];
	}

	if (jack_activate (_client)) {
		fprintf (stderr, "cannot activate client\n");
		throw Exception("cannot activate");
	}


	if (_jack_port){
	}else{
		string dir;
		if (d->is_source()) { untested();
			_jack_port = jack_port_register (_client, port_name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
			dir = "input";
			memset (_buffer, 0, sizeof (sample_t) * _nframes);
		}else{ untested();
			_jack_port = jack_port_register (_client, port_name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
			dir = "output";
		}
		EVAL_BM_JACK** c = _ctx;
		while (*c) {
			++c;
		}
		error(bTRACE, "registering " + dir + " port " + d->long_label() + " in slot " + to_string(int(c-_ctx)) + "\n");
		assert(c-_ctx<MAXPORTS);
		*c = this;

		if (_connect && _jack_port) {
			error(bTRACE, "connecting " + d->long_label() + " to " + to_string(_connect) + "\n");
			const char ** ports;
			if ((ports = jack_get_ports (_client, NULL, NULL,
							JackPortIsPhysical|JackPortIsInput)) == NULL) {
				fprintf(stderr, "Cannot find any physical playback ports\n");
				throw Exception("cannot connect" + d->long_label());
			}

			unsigned c = 0;
			while (c+1<_connect) { untested();
				if(!ports[c]) break;
				++c;
			}
			if (jack_connect (_client, jack_port_name(_jack_port), ports[c])) {
				error(bDANGER, "cannot connect " + d->long_label() + " to port #" + to_string(c) + "\n");
			} else {
				error(bTRACE, "connected " + d->long_label() + " to " + string(ports[c]) + "\n");
			}
			free (ports);
		}
	}
}
/*--------------------------------------------------------------------------*/
void EVAL_BM_JACK::tr_eval(ELEMENT* d)const
{
	double ev = 0;

	if (d->is_source()) {
		while (_pos>_nframes) {
			// waiting for process to fetch/push
		}
		for (unsigned i=_pos; i<_pos+_samples_per_step; ++i) {
			ev += _buffer[i];
		}
		ev /= (double)_samples_per_step;
	}else{ untested();
	}
	tr_finish_tdv(d, ev);
}
/*--------------------------------------------------------------------------*/
TIME_PAIR EVAL_BM_JACK::tr_review(COMPONENT* d)const
{
  d->q_accept();
#ifdef HAVE_DTIME
  return d->_dt_by;
#else
  return d->_time_by;
#endif
}
/*--------------------------------------------------------------------------*/
void EVAL_BM_JACK::tr_accept(COMPONENT*d)const
{
	ELEMENT* e = prechecked_cast<ELEMENT*>(d);
	assert(e);
	assert(_buffer);

	if (!d->is_source()) {
		while (_pos>_nframes) {
			// waiting for process to fetch/push
		}
		for(unsigned i=0; i<_samples_per_step; ++i){
			_buffer[_pos++] = (sample_t) e->tr_input();
		}
	} else {
		_pos += _samples_per_step;
	}

	if (!_pos) {
		// cerr << ",";
	}
}
/*--------------------------------------------------------------------------*/
bool EVAL_BM_JACK::parse_numlist(CS& cmd)
{
	incomplete();
  unsigned start = cmd.cursor();
  unsigned here = cmd.cursor();
#if 0
  for (PARAMETER<double>* i = &_direction;  i < &_end;  ++i) {
    PARAMETER<double> val(NOT_VALID);
    cmd >> val;
    if (cmd.stuck(&here)) {
      break;
    }else{
      *i = val;
    }
  }
#endif
  return cmd.gotit(start);
}
/*--------------------------------------------------------------------------*/
bool EVAL_BM_JACK::parse_params_obsolete_callback(CS& cmd)
{
  return ONE_OF
    || Get(cmd, "dir{ection}",	&_direction)
    || EVAL_BM_ACTION_BASE::parse_params_obsolete_callback(cmd)
    ;
}
/*--------------------------------------------------------------------------*/
map<string, PARA_BASE EVAL_BM_JACK::*> EVAL_BM_JACK::_param_dict =
  boost::assign::map_list_of
    ("direction", (PARA_BASE EVAL_BM_JACK::*) &EVAL_BM_JACK::_direction)
    ("dir",       (PARA_BASE EVAL_BM_JACK::*) &EVAL_BM_JACK::_direction)
    ("connect",   (PARA_BASE EVAL_BM_JACK::*) &EVAL_BM_JACK::_connect);
/*--------------------------------------------------------------------------*/
void EVAL_BM_JACK::set_param_by_name(std::string Name, std::string Value)
{ untested();
  PARA_BASE EVAL_BM_JACK::* x = (_param_dict[Name]);
  if (x) { untested();
    PARA_BASE* p = &(this->*x);
    *p = Value;
  }else{ untested();
    EVAL_BM_ACTION_BASE::set_param_by_name(Name, Value);
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
EVAL_BM_JACK p1(CC_STATIC);
DISPATCHER<COMMON_COMPONENT>::INSTALL d1(&bm_dispatcher, "jack", &p1);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
