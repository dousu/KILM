/*
 * CategoryFactory.cpp
 *
 *  Created on: 2011/05/20
 *      Author: Hiroki Sudo
 */

#include "IndexFactory.h"

//unsigned int IndexFactory::index_counter = 0;

IndexFactory& IndexFactory::operator=(const IndexFactory& dst){
	index_counter = dst.index_counter;
	return *this;
}
int IndexFactory::generate(void){
	if(index_counter+1 < index_counter) throw "range over error";
	index_counter++;
	return index_counter;
}

