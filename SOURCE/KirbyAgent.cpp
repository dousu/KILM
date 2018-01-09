/*
 * KirbyAgent.cpp
 *
 *  Created on: 2012/12/07
 *      Author: Hiroki Sudo
 */

#include "KirbyAgent.h"

bool KirbyAgent::LOGGING_FLAG = false;

KirbyAgent::KirbyAgent() {
    // TODO Auto-generated constructor stub
    generation_index = 1;
}

KirbyAgent::~KirbyAgent() {
    // TODO Auto-generated destructor stub
}

KirbyAgent&
        KirbyAgent::operator=(const KirbyAgent& dst) {
    kb = dst.kb;
    generation_index = dst.generation_index;
    LOGGING_FLAG = dst.LOGGING_FLAG;
    return *this;
}

Rule
KirbyAgent::say(Rule& internal) {
    try {
        //return kb.fabricate2(internal);
        return kb.fabricate(internal);
    } catch (...) {
        LogBox::refresh_log();
        throw;
    }
}

void
KirbyAgent::hear(Rule& term) {
    kb.send_box(term);
}

void
KirbyAgent::learn(void) {
    kb.consolidate();
}

bool
KirbyAgent::utterable(Rule& internal) {
    return kb.acceptable(internal);
}

KirbyAgent
KirbyAgent::make_child(void) {
    KirbyAgent child;
    child.generation_index = generation_index + 1;
    return child;
}

KirbyAgent&
KirbyAgent::grow(std::vector<Rule> meanings) {

    kb.DIC_BLD = false;
    kb.word_dic.clear();
    kb.build_word_index();

    return *this;
}

std::string
KirbyAgent::to_s(void) {
    return kb.to_s();
}

void
KirbyAgent::logging_on(void) {
    LOGGING_FLAG = true;
    KnowledgeBase::logging_on();
}

void
KirbyAgent::logging_off(void) {
    LOGGING_FLAG = false;
    KnowledgeBase::logging_off();
}
