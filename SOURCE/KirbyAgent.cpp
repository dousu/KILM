/*
 * KirbyAgent.cpp
 *
 *  Created on: 2011/06/07
 *      Author: Hiroki Sudo
 */

#include "KirbyAgent.h"

bool KirbyAgent::LOGGING_FLAG = false;
IndexFactory KirbyAgent::indexer;

bool KirbyAgent::DEL_LONG_RULE = false;
int KirbyAgent::DEL_LONG_RULE_LENGTH = 0;
bool KirbyAgent::INDEXER_FLAG = false;
bool KirbyAgent::UTTER_MINIMUM = false;
int KirbyAgent::SHORT_MEM_SIZE = 0;

KirbyAgent::KirbyAgent() {
    // TODO Auto-generated constructor stub
    generation_index = 0;
    serial = indexer.generate();
}

KirbyAgent::~KirbyAgent() {
    // TODO Auto-generated destructor stub
}

KirbyAgent&
        KirbyAgent::operator=(const KirbyAgent& dst) {
    kb = dst.kb;
    generation_index = dst.generation_index;
    serial = dst.serial;
    LOGGING_FLAG = dst.LOGGING_FLAG;
    return *this;
}

Rule
KirbyAgent::say(Rule& internal) {
    try {
        //return kb.fabricate(internal);
        //return kb.fabricate2(internal);
        if (INDEXER_FLAG) {
            if (UTTER_MINIMUM) {
                return kb.fabricate_idx_min(internal);
            } else {
                return kb.fabricate_idx(internal);
            }
        } else {
            if (UTTER_MINIMUM)
                return kb.fabricate_min_len(internal);
            else
                return kb.fabricate(internal);
        }
    } catch (...) {
        LogBox::refresh_log();
        throw;
    }
}

void
KirbyAgent::hear(Rule& term) {
    //短期記憶制限処理
    if (SHORT_MEM_SIZE != 0) {
        while (term.external.size() > SHORT_MEM_SIZE) {
            term.external.pop_back();
        }
    }

    kb.send_box(term);
}

void
KirbyAgent::learn(void) {
    kb.consolidate();
}

bool
KirbyAgent::understand(Rule& internal) {
//    kb.DIC_BLD = false;
//    kb.word_dic.clear();
//    kb.build_word_index();
    return kb.acceptable(internal);
}

KirbyAgent
KirbyAgent::make_child(void) {
    KirbyAgent child;
    child.generation_index = generation_index + 1;
    child.serial = KirbyAgent::indexer.generate();
    return child;
}

KirbyAgent&
KirbyAgent::grow(std::vector<Rule> meanings) {

    kb.DIC_BLD = false;
    kb.word_dic.clear();
    kb.build_word_index();
    
    if (INDEXER_FLAG)
        kb.indexer(meanings);


    bool deleted_flag = false;
    if (DEL_LONG_RULE & LOGGING_FLAG) {
        LogBox::push_log("->>DELETE ON");
        LogBox::push_log(
                "->>LENGTH" + boost::lexical_cast<std::string>(DEL_LONG_RULE_LENGTH));
    }


    if (DEL_LONG_RULE) {
        for (int i = 0; i < meanings.size(); i++) {
            std::map<KnowledgeBase::PATTERN_TYPE,
                    std::vector<KnowledgeBase::PatternType> > pats, pats2;
            std::vector<KnowledgeBase::PatternType> abpat, cmpat;
            pats = kb.construct_grounding_patterns(meanings[i]);
            pats2 = kb.natural_construct_grounding_patterns(meanings[i]);
            kb.exception_filter(pats, pats2);
            abpat = pats[KnowledgeBase::ABSOLUTE];
            cmpat = pats[KnowledgeBase::COMPLETE];

            //absolute (horistic)
            for (int j = 0; j < abpat.size(); j++) {
                //if (LOGGING_FLAG) {
                //  LogBox::push_log("->>CHECK(A): " + abpat[j].front().to_s());
                //}

                if (abpat[j].front().external.size() > DEL_LONG_RULE_LENGTH) {
                    std::vector<Rule>::iterator it;
                    it = kb.sentenceDB.begin();
                    while (it != kb.sentenceDB.end()) {
                        if ((*it) == abpat[j].front()) {
                            it = kb.sentenceDB.erase(it);
                        } else {
                            it++;
                        }
                    }
                }
            }

            for (int j = 0; j < cmpat.size(); j++) {
                Rule rule, sent;
                rule.type = RULE_TYPE::SENTENCE;
                rule.internal = meanings[i].internal;
                sent = cmpat[j].front();
                kb.graund_with_pattern(rule, cmpat[j]);
                cmpat[j].push_back(sent);

                //if (LOGGING_FLAG) {
                //  LogBox::push_log("->>CHECK(C): " + rule.to_s());
                //}
                if (rule.external.size() > DEL_LONG_RULE_LENGTH) {
                    //if (LOGGING_FLAG) {
                    //  LogBox::push_log("->>DEL SELECT FOR: " + rule.to_s());
                    //}

                    Rule del;
                    del.type = RULE_TYPE::SENTENCE;
                    //del.external = std::vector<Element>();
                    for (int k = 0; k < cmpat[j].size(); k++) {
                        //if (LOGGING_FLAG) {
                        //  LogBox::push_log("->>CHECK: " + cmpat[j][k].to_s());
                        //}
                        if (del.external.size() < cmpat[j][k].external.size()) {
                            del = cmpat[j][k];
                        }
                    }

                    //if (LOGGING_FLAG) {
                    //  LogBox::push_log("->>SELECTED: " + del.to_s());
                    //}

                    std::vector<Rule>::iterator it;
                    if (del.is_sentence()) {
                        it = kb.sentenceDB.begin();
                        while (it != kb.sentenceDB.end()) {
                            //if (LOGGING_FLAG) {
                            //  LogBox::push_log("->>SEEK: " + (*it).to_s());
                            //}

                            if ((*it) == del) {
                                if (LOGGING_FLAG) {
                                    LogBox::push_log(">>DELETED");
                                    LogBox::push_log("GDP: " + rule.to_s());
                                    LogBox::push_log("DEL: " + del.to_s());
                                    LogBox::push_log("<<DELETED");
                                }
                                deleted_flag = true;
                                it = kb.sentenceDB.erase(it);
                            } else {
                                it++;
                            }
                        }
                    } else if (del.is_noun()) {
                        int l;
                        for (l = 0; l < sent.internal.size(); l++) {
                            if (sent.internal[l] == del.internal.front()) {
                                break;
                            }
                        }
                        if (l >= sent.internal.size()) {
                            std::cerr << "Cannot find var in sent" << std::endl;
                            throw "ERROR";
                        }

                        del.internal[0] = rule.internal[l];

                        //if (LOGGING_FLAG) {
                        //  LogBox::push_log("->>TARG: " + del.to_s());
                        //}

                        it = kb.wordDB.begin();
                        while (it != kb.wordDB.end()) {
                            //if (LOGGING_FLAG) {
                            //  LogBox::push_log("->>SEEK: " + (*it).to_s());
                            //}
                            if ((*it) == del) {
                                if (LOGGING_FLAG) {
                                    LogBox::push_log(">>DELETED");
                                    LogBox::push_log(del.to_s());
                                    LogBox::push_log("<<DELETED");
                                }
                                deleted_flag = true;
                                it = kb.wordDB.erase(it);
                            } else {
                                it++;
                            }
                        }
                    }
                }
            }
            if (deleted_flag) {
                kb.DIC_BLD = false;
                kb.word_dic.clear();
                kb.build_word_index();
            }
        }
    }

    if (DEL_LONG_RULE & LOGGING_FLAG) {
        LogBox::push_log("<<-DELETE");
    }

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
