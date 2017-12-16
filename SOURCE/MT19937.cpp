/*
 * MT19937E.cpp
 *
 *  Created on: 2011/08/04
 *      Author: Rindow
 */

#include "MT19937.h"

MT19937::MT19937() {
	// TODO Auto-generated constructor stub

}

MT19937::~MT19937() {
	// TODO Auto-generated destructor stub
}

unsigned long long int MT19937::icount=0LL;
unsigned long long int MT19937::rcount=0LL;


boost::mt19937 MT19937::igen(111);
boost::uniform_int<int> MT19937::idist(0, INT_MAX);
boost::variate_generator<boost::mt19937, boost::uniform_int<int> > MT19937::_irand(MT19937::igen, MT19937::idist);

boost::mt19937 MT19937::rgen(111);
boost::uniform_real<double> MT19937::rdist(0, 1);
boost::variate_generator<boost::mt19937, boost::uniform_real<double> > MT19937::_rrand(MT19937::rgen, MT19937::rdist);


int MT19937::irand(void){
	   if(icount < ULONG_LONG_MAX -1)
		   icount++;
	   else
		   throw "cannot use random any more";

	   return _irand();
}

double MT19937::rrand(void){
	   if(rcount < ULONG_LONG_MAX -1)
		   rcount++;
	   else
		   throw "cannot use random any more";
	   return _rrand();
}

void MT19937::waste(void){
	   unsigned long long int i = 0LL;
	   while (i < icount){
		   _irand();
		   i++;
	   }

	   i=0;
	   while(i < rcount){
		   _rrand();
		   i++;
	   }
}

void MT19937::set_seed(uint32_t seed_value){
	   _irand.engine().seed(static_cast<unsigned long>(seed_value));
	   _rrand.engine().seed(static_cast<unsigned long>(seed_value));
}
