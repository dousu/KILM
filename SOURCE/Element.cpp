/*
 * Element.cpp
 *
 *  Created on: 2011/05/19
 *      Author: Rindow
 */

#include "Element.h"


//static initialize
Dictionary Element::dictionary = Dictionary::copy();


Element::Element(){
}

/*
 * Operators
 */
bool Element::operator!=(const Element& dst) const{
	return !(*this == dst);
}


bool Element::operator==(const Element& dst) const{
	//タイプ検査
	if(type != dst.type) return false;

	//同タイプでの比較
	switch(type){
	case ELEM_TYPE::CAT_TYPE:
		if(obj == dst.obj && cat == dst.cat) return true;
		break;
	case ELEM_TYPE::VAR_TYPE:
		if(obj == dst.obj) return true;
		break;
	case ELEM_TYPE::IND_TYPE:
		if(obj == dst.obj) return true;
		break;
	case ELEM_TYPE::SYM_TYPE:
		if(obj == dst.obj) return true;
		break;
	default:
		std::cout << type << ":" << "unknown type" << std::endl;
		throw "unknown type";
		break;
	}

	return false;
}

bool Element::operator<(const Element& dst) const{
	if(type < dst.type)
		return true;
	else if(type > dst.type)
		return false;

	//同タイプでの比較
	switch(type){
	case ELEM_TYPE::CAT_TYPE:
		if(cat < dst.cat)
			return true;
		else if(cat > dst.cat)
			return false;

		if(obj < dst.obj)
			return true;
		else if(obj > dst.obj)
			return false;

		break;
	case ELEM_TYPE::VAR_TYPE:
		if(obj < dst.obj)
			return true;
		else if(obj > dst.obj)
			return false;

		break;
	case ELEM_TYPE::IND_TYPE:
		if(obj < dst.obj)
			return true;
		else if(obj > dst.obj)
			return false;

		break;
	case ELEM_TYPE::SYM_TYPE:
		if(obj < dst.obj)
			return true;
		else if(obj > dst.obj)
			return false;

		break;
	default:
		std::cout << type << ":" << "unknown type" << std::endl;
		throw "unknown type";
		break;
	}

	return false;

}

Element& Element::operator=(const Element& dst){
	type = dst.type;
	obj = dst.obj;
	cat = dst.cat;

	return *this;
}



std::string Element::to_s(void){
	switch(type){

	case ELEM_TYPE::CAT_TYPE :
		return Prefices::CAT + Prefices::CLN +
				boost::lexical_cast<std::string>(cat) +
				Prefices::DEL +
				Prefices::VAR + Prefices::CLN +
				boost::lexical_cast<std::string>(obj);
		break;

	case ELEM_TYPE::IND_TYPE :
		if(dictionary.individual.find(obj) == dictionary.individual.end()){
			return "-";
		}
		return dictionary.individual[obj];
		break;

	case ELEM_TYPE::SYM_TYPE :
		if(dictionary.symbol.find(obj) == dictionary.symbol.end()){
			return "*";
		}
		return dictionary.symbol[obj];
		break;

	case ELEM_TYPE::VAR_TYPE :
		return Prefices::CAT + Prefices::CLN +
				boost::lexical_cast<std::string>(cat) +
				Prefices::DEL +
				Prefices::VAR + Prefices::CLN +
				boost::lexical_cast<std::string>(obj);
		break;

	default:
		std::cerr << "unknown type: Element::to_s()" << std::endl;
		throw "unknown type of Element";
	}
}

Element& Element::set_cat(int var, int cat){
	return set(ELEM_TYPE::CAT_TYPE, var, cat);
}

Element& Element::set_var(int var, int cat){
	return set(ELEM_TYPE::VAR_TYPE, var, cat);
}

Element& Element::set_ind(int id){
	if(dictionary.individual.find(id) == dictionary.individual.end())
		throw "range over for individual";
	return set(ELEM_TYPE::IND_TYPE, id, 0);
}

Element& Element::set_sym(int id){
	if(dictionary.symbol.find(id) == dictionary.symbol.end())
		throw "range over for symbol";
	return set(ELEM_TYPE::SYM_TYPE, id, 0);
}

Element& Element::set(int dtype, int dobj, int dcat){
	type = dtype;
	obj  = dobj;
	cat  = dcat;
	return *this;
}

bool Element::is_var(void) const {
	if(type == ELEM_TYPE::VAR_TYPE) return true;
	else return false;
}
bool Element::is_cat(void) const {
	if(type == ELEM_TYPE::CAT_TYPE) return true;
	else return false;
}
bool Element::is_ind(void) const {
	if(type == ELEM_TYPE::IND_TYPE) return true;
	else return false;
}
bool Element::is_sym(void) const {
	if(type == ELEM_TYPE::SYM_TYPE) return true;
	else return false;
}

#ifdef DEBUG_ELEMENT
#include <vector>
#include <iostream>
int main(int arg, char** argv){
	Element::dictionary.load((char*)"data.dic");

	std::vector<Element> elements, elements2;
	int type=0;

	while (type <= 3){
		Element elm;
		switch(type){
		case ELEM_TYPE::CAT_TYPE:
			for(unsigned int i=0;i<3;i++){
				elm.set_cat(i, i);
				elements.push_back(elm);
			}
			break;
		case ELEM_TYPE::IND_TYPE:
			for(unsigned int i=0;i<Element::dictionary.individual.size();i++){
				elm.set_ind(i);
				elements.push_back(elm);
			}
			break;
		case ELEM_TYPE::SYM_TYPE:
			for(unsigned int i=0;i<Element::dictionary.symbol.size();i++){
				elm.set_sym(i);
				elements.push_back(elm);
			}
			break;
		case ELEM_TYPE::VAR_TYPE:
			for(unsigned int i=0;i<3;i++){
				elm.set_var(i, 0);
				elements.push_back(elm);
			}
			break;
		}
		type++;
	}


	std::vector<Element>::iterator it;
	it = elements.begin();
	int l=0;
	while (it != elements.end()){
		std::cout << l++ <<": "<< (*it).to_s() << std::endl;
		it++;
	}

	std::vector<Element>::iterator it1, it2;
	it1 = elements.begin();
	int k=0;
	int j=0;
	while (it1 != elements.end()){
		it2 = elements.begin();
		k=0;
		while (it2 != elements.end()){
			if(*it1 == *it2){
//				std::cout << "*************************************" <<std::endl;

				//if(j != 28 && k != 28){
					std::cout << j << " : " << k << " ";
					std::cout << (*it1).to_s();
					std::cout << " = " ;
					std::cout << (*it2).to_s() << std::endl;
				//}
				/*
				 * なぜかElement28番のto_sがおかしい……
				 * 比較はできているようだから、とりあえずスルー
				 * */
				/*
				if(elements[28] == *it1){
					std::cout << (*(it1)).to_s() << "::::::::::::" << elements[28].to_s() << "???" << std::endl;
				}
				*/

			}
			k++;
			it2++;
		}
		j++;
		it1++;
	}

	it = elements.begin();
	std::cout << elements[28].to_s() << std::endl;

	elements2 = elements;
	it = elements2.begin();
	while (it != elements2.end()){
		std::cout << (*it).to_s() << std::endl;
		it++;
	}
	return 0;
}
#endif
