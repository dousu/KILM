/* 
 * File:   CM_main.h
 * Author: hiroki
 *
 * Created on August 9, 2014, 6:10 PM
 */

#ifndef CM_MAIN_H
#define	CM_MAIN_H


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

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>

#include "MSILMAgent.h"
#include "Rule.h"
#include "Element.h"
#include "MT19937.h"
#include "Dictionary.h"
#include "LogBox.h"
#include "MSILMParameters.h"
#include "Distance.hpp"

//construct function
void
construct_meanings(std::vector<Rule>& meanings);
void
construct_individuals(std::vector<Element>& inds, Dictionary &dic);

//analyze function
void
unit_analyze(std::vector<double>& result_vector,
    std::vector<Rule>& meanings, MSILMAgent& agent);

double
expression(std::vector<Rule>& meanings, MSILMAgent& agent);

void
calculate_language_distance(
    std::vector<double>& lev_sent_vector,
    std::vector<double>& lev_word_vector,
    std::vector<Rule>& meanings, std::vector<Element>& words, MSILMAgent& agent1,
    MSILMAgent& agent2);

double
calculate_word_distance(std::vector<Element>&, KnowledgeBase&, KnowledgeBase&);

void
analyze_and_output(MSILMParameters& param, std::vector<Rule> meaning_space,
    std::vector<Element> individuals, MSILMAgent& agent1, MSILMAgent& agent2, int index);

//sudo
double
calculate_sudo_distance(std::vector<Rule>& meanings,
    KnowledgeBase& kb1, KnowledgeBase& kb2,double& word_length);

void
calculate_average_word_length(std::vector<Rule>& meanings,KnowledgeBase& kb1,double& word_length);

std::string
tr_vector_double_to_string(std::vector<double> vector);

std::vector<int>
choice_selected_meanings(std::vector<Rule> meanings,MSILMParameters param);

double limit_time;

void 
cognition_init(std::vector<int>& source, MSILMParameters& param);

template<typename _IFS>
void resume_agent(
		_IFS&,
		MSILMParameters&,
		unsigned long long int&,
		unsigned long long int&,
		Dictionary&,
		std::vector<Rule>&,
		int&,
		MSILMAgent&
		);

template<typename _OFS>
void save_agent(
		_OFS&,
		MSILMParameters&,
		unsigned long long int&,
		unsigned long long int&,
		Dictionary&,
		std::vector<Rule>&,
		int&,
		MSILMAgent&
		);


#endif	/* CM_MAIN_H */

