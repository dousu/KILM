/*
 * NetWorldFactory.h
 *
 *  Created on: 2011/07/21
 *      Author: rindou
 */

#ifndef NETWORLDFACTORY_H_
#define NETWORLDFACTORY_H_

class NetWorldFactory {
public:
	NetWorldFactory();
	virtual ~NetWorldFactory();

	NetWorld create();
};

#endif /* NETWORLDFACTORY_H_ */
