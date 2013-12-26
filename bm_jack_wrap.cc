#include <globals.h>
#include <e_compon.h>

namespace {
using std::string;

class DEV_MEAS_JACK : public COMPONENT { //
	public:
		string value_name() const { return "dummy"; }
		string port_name(uint_t) const { return "dummy"; }

		CARD* clone()const
		{ untested();
			const CARD* c = device_dispatcher["meas"];
			assert(c);
			CARD* c2 = c->clone();
			COMPONENT* d = prechecked_cast<COMPONENT*>(c2);
			assert(d);
			const COMMON_COMPONENT* b = bm_dispatcher["jack"];
			assert(b);
			COMMON_COMPONENT* bc = b->clone();
			d->attach_common(bc);
			d->set_dev_type("m_jack");
			assert(d->dev_type() == "m_jack");
			return d;
		}
}p1;
DISPATCHER<CARD>::INSTALL d1(&device_dispatcher, "m_jack|meas_jack", &p1);

class DEV_VS_SIN : public COMPONENT { //
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
}p2;
DISPATCHER<CARD>::INSTALL d2(&device_dispatcher, "v_jack", &p2);
}
