/*
 * kirby_main.h
 *
 *  Created on: 2012/12/23
 *      Author: rindou
 */

#ifndef KIRBY_MAIN_H_
#define KIRBY_MAIN_H_

#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <climits>
#include <cfloat>

#include <stdio.h>

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/progress.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/export.hpp>
#include <boost/lexical_cast.hpp>

#include "KirbyAgent.h"
#include "Rule.h"
#include "Element.h"
#include "MT19937.h"
#include "Dictionary.h"
#include "LogBox.h"
#include "Parameters.h"
#include "Distance.hpp"

//construct function
void
construct_meanings(std::vector<Rule>& meanings);
void
construct_individuals(std::vector<Element>& inds, Dictionary &dic);

//analyze function
void
unit_analyze(std::vector<double>& result_vector,
    std::vector<Rule>& meanings, KirbyAgent& agent);

int
expression(std::vector<Rule>& meanings, KirbyAgent& agent);

void
calculate_language_distance(
    std::vector<double>& dist_vector,
    std::vector<Rule>& meanings, KirbyAgent& agent1,KirbyAgent& agent2);

void
analyze_and_output(Parameters& param, std::vector<Rule> meaning_space,
    KirbyAgent& agent1, KirbyAgent& agent2);

//language distance
double
calculate_distance(std::vector<Rule>& meanings,
    KnowledgeBase& kb1, KnowledgeBase& kb2);

std::string
tr_vector_double_to_string(std::vector<double> vector);


template<typename _IFS>
void resume_agent(
		_IFS&,
		Parameters&,
		unsigned long long int&,
		unsigned long long int&,
		Dictionary&,
		std::vector<Rule>&,
		int&,
		KirbyAgent&
		);

template<typename _OFS>
void save_agent(
		_OFS&,
		Parameters&,
		unsigned long long int&,
		unsigned long long int&,
		Dictionary&,
		std::vector<Rule>&,
		int&,
		KirbyAgent&
		);

//std::vector<int> analyze(std::vector<Rule>& meanings, KirbyAgent& agent);
//int expression(std::vector<Rule>& meanings, KirbyAgent& agent);

#endif /* KIRBY_MAIN_H_ */
