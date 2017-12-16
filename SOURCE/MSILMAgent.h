/* 
 * File:   MSILMAgent.h
 * Author: hiroki
 *
 * Created on October 25, 2013, 4:08 PM
 */

#ifndef MSILMAGENT_H
#define	MSILMAGENT_H
#include "KirbyAgent.h"

class MSILMAgent : public KirbyAgent{
public:
    
    MSILMAgent make_child(void);
    
    MSILMAgent& grow(std::vector<Rule>);
    
    MSILMAgent& operator=(const MSILMAgent& dst);
    
    Rule say(Rule& internal);
    void hear(std::vector<Rule>& terms, std::vector<Rule> all_meanings);
    Rule dither_say(std::vector<Rule>& internals);
//    void dither_hear(Rule& term,std::vector<Rule>& meanings, std::vector<Rule>& all_meanings);
    void dither_hear(std::vector<Rule>& terms,std::vector<std::vector<Rule> >& meaningss, std::vector<Rule>& all_meanings);
    
    std::vector<Rule> think_meaning(std::vector<Rule>& internals);
    std::vector<std::vector<Rule> > think_meaning(bool& flag,std::vector<Rule>& terms,std::vector<std::vector<Rule> >& meaningss, std::vector<Rule>& all_meanings);
    std::vector<Rule> random_think_meaning(std::vector<Rule>& internals);
    void symmetry_bias_think(std::vector<Rule>& terms,std::vector<std::vector<Rule> >& meaningss,std::vector<Rule>& reference, std::vector<KnowledgeBase::PatternType>& patterns,std::vector<std::vector<Rule> >& return_rules);
    void mutual_exclusivity_bias_think(std::vector<Rule>& terms,std::vector<std::vector<Rule> >& meaningss,std::vector<Rule>& reference, std::vector<KnowledgeBase::PatternType>& patterns,std::vector<std::vector<Rule> >& return_rules);
    void symmetry_bias_think(std::vector<Rule>& terms,std::vector<std::vector<Rule> >& meaningss,std::vector<Rule>& reference, std::vector<KnowledgeBase::PatternType>& patterns,std::vector<std::vector<Rule> >& term_pairs, std::vector<std::vector<std::vector<Rule> > >& meaning_pair_orders,std::vector<std::vector<double> >& meaning_distancess);
    void mutual_exclusivity_bias_think(std::vector<Rule>& terms,std::vector<std::vector<Rule> >& meaningss,std::vector<Rule>& reference, std::vector<KnowledgeBase::PatternType>& patterns,std::vector<std::vector<Rule> >& term_pairs, std::vector<std::vector<std::vector<Rule> > >& meaning_pair_orders,std::vector<std::vector<double> >& meaning_distancess);
    void decide_likelihood(std::vector<Rule>& terms, std::vector<std::vector<Rule> >& term_pairs, std::vector<std::vector<std::vector<Rule> > >& meaning_pair_orders);
    static bool SYM_FLAG;
    static bool MUT_FLAG;
    static bool EXC_FLAG;
    static bool OMISSION_FLAG;
    
    void symmetry_exception_check(Rule term, std::vector<Rule>& reference, std::vector<KnowledgeBase::PatternType>& patterns);
    void mutual_exclusivity_exception_check(Rule term, std::vector<Rule>& reference, std::vector<KnowledgeBase::PatternType>& patterns);
    
    Rule return_last_selected_meaning();
    
    //waste
    std::string tr_vector_Rule_to_string(std::vector<Rule> vector);
    std::string tr_vector_Rule_to_string(std::vector<std::vector<Rule> > vector);
    
    MSILMAgent();
    virtual ~MSILMAgent();
    static void sym_on(void);
    static void mut_on(void);
    static void exc_on(void);
    static void omission_on(void);
    static void omission_off(void);
    
    Rule last_selected_meaning;
    
private:

};

class Random
{
public:
	// コンストラクタ
	Random()
	{
		
	}

	// 関数オブジェクト
	unsigned int operator()(unsigned int max)
	{
		return static_cast<unsigned int>( MT19937::irand() % max );
	}
};

#endif	/* MSILMAGENT_H */

