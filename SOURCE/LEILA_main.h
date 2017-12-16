/*
 * LELA_main.h
 *
 *  Created on: 2011/06/29
 *      Author: rindou
 */

#ifndef LELA_MAIN_H_
#define LELA_MAIN_H_

//standard c++ library
#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <climits>
#include <cfloat>

//standard c library
#include <stdio.h>

//boost library
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
//#include <boost/filesystem.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>

//igraph library
#include <igraph.h>

//headers
#include "KirbyAgent.h"
#include "Rule.h"
#include "Element.h"
//#include "Random.hpp"
#include "MT19937.h"
#include "Dictionary.h"
#include "LogBox.h"
#include "LEILAParameters.h"
#include "NetWorld.h"
#include "Distance.hpp"

//construct function
void
construct_meanings(std::vector<Rule>& meanings);
void
construct_individuals(std::vector<Element>& inds, Dictionary &dic);

/*
 //saving and resuming function
 template<typename _IFS>
 void resume_agents(
 _IFS& ifs,
 LELAParameters& param,
 unsigned long long int& icount,
 unsigned long long int& rcount,
 Dictionary& dic,
 std::vector<Rule>& meanings,
 NetWorld& agents
 );
 template <>
 void resume_agents(
 boost::archive::binary_iarchive& ifs,
 LELAParameters& param,
 unsigned long long int& icount,
 unsigned long long int& rcount,
 Dictionary& dic,
 std::vector<Rule>& meanings,
 NetWorld& agents
 );
 template <>
 void resume_agents(
 boost::archive::xml_iarchive& ifs,
 LELAParameters& param,
 unsigned long long int& icount,
 unsigned long long int& rcount,
 Dictionary& dic,
 std::vector<Rule>& meanings,
 NetWorld& agents
 );


 template<typename _OFS>
 void save_agents(
 _OFS& ofs,
 LELAParameters& param,
 unsigned long long int& icount,
 unsigned long long int& rcount,
 Dictionary& dic,
 std::vector<Rule>& meanings,
 NetWorld& agents
 );
 template<>
 void save_agents(
 boost::archive::binary_oarchive& ofs,
 LELAParameters& param,
 unsigned long long int& icount,
 unsigned long long int& rcount,
 Dictionary& dic,
 std::vector<Rule>& meanings,
 NetWorld& agents
 );
 template<>
 void save_agents(
 boost::archive::xml_oarchive& ofs,
 LELAParameters& param,
 unsigned long long int& icount,
 unsigned long long int& rcount,
 Dictionary& dic,
 std::vector<Rule>& meanings,
 NetWorld& agents
 );
 */
void
save_network(igraph_t* net);

void
load_network(igraph_t* net);

//analyze function
std::vector<boost::numeric::ublas::matrix<double> >
analyze(std::vector<Rule>& meanings, std::vector<Element>& individuals,
    NetWorld& world1, NetWorld& world2);

void
unit_analyze(boost::numeric::ublas::matrix<double>& result_matrix,
    std::vector<Rule>& meanings, NetWorld& world);

int
expression(std::vector<Rule>& meanings, KirbyAgent& agent);

void
calculate_language_distance(
    boost::numeric::ublas::matrix<double>& lev_sent_matrix,
    boost::numeric::ublas::matrix<double>& lev_word_matrix,
    std::vector<Rule>& meanings, std::vector<Element>& words, NetWorld& world1,
    NetWorld& world2);

double
calculate_sentence_distance(std::vector<Rule>& meanings, KnowledgeBase& kb1,
    KnowledgeBase& kb2);

double
calculate_sentence_minimum_distance(std::vector<Rule>& meanings,
    KnowledgeBase& kb1, KnowledgeBase& kb2);

double
calculate_sentence_minimum_average_distance(std::vector<Rule>& meanings,
    KnowledgeBase& kb1, KnowledgeBase& kb2);
double
calculate_word_distance(std::vector<Element>&, KnowledgeBase&, KnowledgeBase&);


double
calculate_sentence_minimum_average_distance_with_index(std::vector<Rule>& meanings,
    KnowledgeBase& kb1, KnowledgeBase& kb2);

void
normalize(boost::numeric::ublas::matrix<double>& m, NetWorld& world);

void
analyze_and_output(LELAParameters& param, std::vector<Rule> meaning_space,
    std::vector<Element> individuals, NetWorld& world1, NetWorld& world2);

double
cal_SMAD_th(std::vector<Rule>& meanings, KnowledgeBase& kb1,
    KnowledgeBase& kb2);

//sudo
double
calculate_sudo_distance(std::vector<Rule>& meanings,
    KnowledgeBase& kb1, KnowledgeBase& kb2);


#endif /* LELA_MAIN_H_ */
