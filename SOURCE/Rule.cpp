/*
 * Rule.cpp
 *
 *  Created on: 2011/05/18
 *      Author: Rindow
 */
#include "Rule.h"

Dictionary Rule::dictionary = Dictionary::copy();
/*
 Rule::~Rule() {
 }
 */

Rule::Rule() {
}
Rule::~Rule() {
}

Rule::Rule(char* cstr) {
  Rule(std::string(cstr));
}

Rule::Rule(std::string str) {
  std::vector<std::string> buf, inbuf, exbuf;
  std::vector<std::string>::iterator it;

  boost::algorithm::split(buf, str,
      boost::algorithm::is_any_of(Prefices::ARW.c_str()),
      boost::algorithm::token_compress_on);

  //Left
  buf[0] = boost::algorithm::trim_copy(buf[0]);
  boost::algorithm::split(inbuf, buf[0], boost::algorithm::is_any_of(" "),
      boost::algorithm::token_compress_on);

  //Rule type
  if (inbuf[0] == Prefices::SEN) {
    type = RULE_TYPE::SENTENCE;
    cat = 0;
  }
  else if (inbuf[0].find(Prefices::CLN.c_str()) != std::string::npos) {
    std::vector<std::string> cbuf;
    boost::algorithm::split(cbuf, inbuf[0],
        boost::algorithm::is_any_of(Prefices::CLN.c_str()),
        boost::algorithm::token_compress_on);
    type = RULE_TYPE::NOUN;
    cat = boost::lexical_cast<int>(cbuf[1]);
  }
  else {
    throw "Illegal String";
  }

  /*
   * internal
   */
  it = inbuf.begin() + 1;
  while (it != inbuf.end()) {
    if ((*it).find(Prefices::DEL) != std::string::npos) {
      std::vector<std::string> tbuf, cbuf, vbuf;
      boost::algorithm::split(tbuf, *it,
          boost::algorithm::is_any_of(Prefices::DEL.c_str()),
          boost::algorithm::token_compress_on);
      if (tbuf.size() != 2)
        throw "error";

      boost::algorithm::split(cbuf, tbuf[0],
          boost::algorithm::is_any_of(Prefices::CLN.c_str()),
          boost::algorithm::token_compress_on);
      if (cbuf.size() != 2)
        throw "error";

      boost::algorithm::split(vbuf, tbuf[1],
          boost::algorithm::is_any_of(Prefices::CLN.c_str()),
          boost::algorithm::token_compress_on);
      if (vbuf.size() != 2)
        throw "error";

      Element var;
      unsigned int icat, ivar;
      icat = boost::lexical_cast<int>(cbuf[1].c_str());
      ivar = boost::lexical_cast<int>(vbuf[1].c_str());
      var.set_var(ivar, icat);
      internal.push_back(var);
    }
    else {
      Element ind;
      std::map<std::string, int>::iterator dic_it;
      dic_it = dictionary.conv_individual.find(*it);
      if (dic_it != dictionary.conv_individual.end()) {
        ind.set_ind((*dic_it).second);
        internal.push_back(ind);
      }
      else {
        throw "error";
      }
    }
    it++;
  }

  if (buf.size() == 2) {
    //Right
    buf[1] = boost::algorithm::trim_copy(buf[1]);
    boost::algorithm::split(exbuf, buf[1], boost::algorithm::is_any_of(" "),
        boost::algorithm::token_compress_on);

    it = exbuf.begin();
    while (it != exbuf.end()) {
      if ((*it).find(Prefices::DEL) != std::string::npos) {
        //CAT
        std::vector<std::string> tbuf, cbuf, vbuf;
        boost::algorithm::split(tbuf, *it,
            boost::algorithm::is_any_of(Prefices::DEL.c_str()),
            boost::algorithm::token_compress_on);
        if (tbuf.size() != 2)
          throw "error";

        boost::algorithm::split(cbuf, tbuf[0],
            boost::algorithm::is_any_of(Prefices::CLN.c_str()),
            boost::algorithm::token_compress_on);

        boost::algorithm::split(vbuf, tbuf[1],
            boost::algorithm::is_any_of(Prefices::CLN.c_str()),
            boost::algorithm::token_compress_on);

        Element excat;
        unsigned int icat, ivar;
        icat = boost::lexical_cast<int>(cbuf[1].c_str());
        ivar = boost::lexical_cast<int>(vbuf[1].c_str());
        excat.set_cat(ivar, icat);
        external.push_back(excat);
      }
      else {
        Element sym;
        std::map<std::string, int>::iterator dic_it;
        dic_it = dictionary.conv_symbol.find(*it);
        if (dic_it != dictionary.conv_symbol.end()) {
          sym.set_sym((*dic_it).second);
          external.push_back(sym);
        }
        else {
          throw "error";
        }
      }
      it++;
    }
  }
}

/*
 *
 * Operators
 *
 */

bool
Rule::operator!=(Rule& dst) const {
  return !(*this == dst);
}

bool
Rule::operator==(const Rule& dst) const {
  if (type == dst.type && internal.size() == dst.internal.size()
      && external.size() == dst.external.size()) {
    switch (type) {
      case RULE_TYPE::SENTENCE:
        if (internal == dst.internal && external == dst.external)
          return true;
        break;

      case RULE_TYPE::NOUN:
        if (cat == dst.cat && internal == dst.internal
            && external == dst.external)
          return true;
        break;

      default:
        std::cout << "error type" << std::endl;
    }
  }

  return false;
}

/*
 bool Rule::operator<(Rule& dst) const{
 return ;
 }
 */

Rule&
Rule::operator=(const Rule& dst) {
  type = dst.type;
  cat = dst.cat;
  internal = dst.internal;
  external = dst.external;

  return *this;
}

bool
Rule::is_sentence(void) const {
  if (type == RULE_TYPE::SENTENCE)
    return true;
  return false;
}

bool
Rule::is_noun(void) const {
  if (type == RULE_TYPE::NOUN)
    return true;
  return false;
}

int
Rule::composition(void) const {
  InType::const_iterator it;
  int comp;
  comp = 0;

  for (it = internal.begin(); it != internal.end(); it++) {
    if ((*it).is_var())
      comp++;
  }
  return comp;
}

std::string
Rule::to_s(void) {
  //rule category
  std::string rule_type = "";
  std::string internal_str = "";
  std::string external_str = "";

  switch (type) {
    case RULE_TYPE::SENTENCE:
      rule_type = Prefices::SEN;
      break;

    case RULE_TYPE::NOUN:
      rule_type = Prefices::CAT + Prefices::CLN
          + boost::lexical_cast<std::string>(cat);

      break;
    default:
      std::cerr << "RULE TYPE : " << type << std::endl;
      throw "unknown rule type";
  }

  std::vector<std::string> buffer;

  if (internal.size() > 0) {
    InType::iterator it;
    //internal
    buffer.clear();
    it = internal.begin();
    while (it != internal.end()) {
      buffer.push_back((*it).to_s());
      it++;
    }
    internal_str += boost::algorithm::join(buffer, " ");
  }

  if (external.size() > 0) {
    ExType::iterator it;
    //external
    it = external.begin();
    buffer.clear();
    while (it != external.end()) {
      buffer.push_back((*it).to_s());
      it++;
    }
    external_str = boost::algorithm::join(buffer, " ");
  }

  return rule_type + " " + internal_str + " " + Prefices::ARW + " "
      + external_str;
}

void
Rule::set_noun(Element& dcat, Element& dind, std::vector<Element>& dex) {
  set_noun(dcat.cat, dind, dex);
}

void
Rule::set_noun(int dcat, Element& dind, std::vector<Element>& dex) {
  type = RULE_TYPE::NOUN;
  cat = dcat;
  internal.clear();
  internal.push_back(dind);
  external = dex;
}

void
Rule::set_sentence(std::vector<Element>& din, std::vector<Element>& dex) {
  type = RULE_TYPE::SENTENCE;
  cat = 0; //feature
  internal = din;
  external = dex;
}

std::vector<std::vector<Element> >
Rule::moph(void){
    std::vector<Element>::iterator it; 
    std::vector<std::vector<Element> > moph;
    std::vector<Element> buf;
    
    it = external.begin();
    for(;it != external.end();it++){
        switch((*it).type){
            case ELEM_TYPE::SYM_TYPE:
                buf.push_back(*it);
                break;
            case ELEM_TYPE::VAR_TYPE:
                if(buf.size()!=0)
                    moph.push_back(buf);
                std::vector<Element> new_buf;
                buf=new_buf;
                break;
        }
    }
    if(buf.size()!=0)
        moph.push_back(buf);
    
    return moph;
}

#ifdef DEBUG_RULE
#include <vector>
#include <iostream>

int main(int arg, char** argv) {
  Element::load_dictionary((char*)"data.dic");

  int internal_size =3;

  std::vector<Element> cat,var,sym,ind;
  int type=0;

  while (type <= 3) {
    Element elm;
    switch(type) {
      case ELEM_TYPE::CAT_TYPE :
      for(unsigned int i=0;i<3;i++) {
        elm.set_cat(i, i);
        cat.push_back(elm);
      }
      break;
      case ELEM_TYPE::IND_TYPE :
      for(unsigned int i=0;i<Element::dic.individual.size();i++) {
        elm.set_ind(i);
        ind.push_back(elm);
      }
      break;
      case ELEM_TYPE::SYM_TYPE :
      for(unsigned int i=0;i<Element::dic.symbol.size();i++) {
        elm.set_sym(i);
        sym.push_back(elm);
      }
      break;
      case ELEM_TYPE::VAR_TYPE :
      for(unsigned int i=0;i<internal_size;i++) {
        elm.set_var(i, i);
        var.push_back(elm);
      }
      break;
    }
    type++;
  }

  Rule noun1, noun2, noun3, noun4;
  Rule sent1, sent2, sent3, sent4;
  std::vector<Element> ex1,ex2,ex3,ex4;
  ex1.push_back(sym[0]);
  ex1.push_back(sym[1]);
  ex1.push_back(sym[2]);

  ex2.push_back(sym[0]);
  ex2.push_back(sym[1]);
  ex2.push_back(sym[3]);

  noun1.set_noun(cat[0].cat, ind[0], ex1);
  noun2.set_noun(cat[0].cat, ind[0], ex1);
  noun3.set_noun(cat[0].cat, ind[1], ex1);
  noun4.set_noun(cat[0].cat, ind[0], ex2);

  std::vector<Element> in1,in2,in3,in4;
  in1.push_back(ind[0]);
  in1.push_back(ind[0]);
  in1.push_back(ind[0]);

  in2.push_back(ind[0]);
  in2.push_back(ind[1]);
  in2.push_back(ind[2]);

  in3.push_back(var[0]);
  in3.push_back(ind[1]);
  in3.push_back(ind[2]);

  in4.push_back(ind[0]);
  in4.push_back(ind[0]);
  in4.push_back(ind[0]);

  std::vector<Element> ex5,ex6,ex7,ex8;
  ex5.push_back(sym[0]);
  ex5.push_back(sym[1]);
  ex5.push_back(sym[2]);

  ex6.push_back(sym[0]);
  ex6.push_back(sym[1]);
  ex6.push_back(sym[3]);

  ex7.push_back(sym[0]);
  ex7.push_back(cat[0]);
  ex7.push_back(sym[2]);

  ex8.push_back(sym[0]);
  ex8.push_back(sym[1]);
  ex8.push_back(sym[3]);

  sent1.set_sentence(in1,ex5);
  sent2.set_sentence(in2,ex6);
  sent3.set_sentence(in3,ex7);
  sent4.set_sentence(in4,ex8);

  //	std::cout << noun1.to_s() << std::endl;
  std::cout << "*************************" << std::endl;
  if(noun1 == noun2) {
    std::cout << noun1.to_s() << " = " << noun2.to_s() << std::endl;
  }

  Rule test;

  test = noun1;
  std::cout << "test" << std::endl << sent3.to_s() << std::endl;

  std::vector<Rule> sents1,sents2;
  sents1.push_back(sent1);
  sents1.push_back(sent2);
  sents1.push_back(sent3);
  sents1.push_back(sent4);

  sents1 = sents2;

  Rule temp(sent3.to_s());
  std::cout << std::endl;
  std::cout << std::endl;

  std::cout << "sent3:" << std::endl;
  std::cout << "type:" << sent3.type << std::endl;
  std::cout << "cat:" << sent3.cat << std::endl;

  std::vector<Element>::iterator it2;
  std::cout << "internal: " << std::endl;
  it2 = sent3.internal.begin();
  while(it2 != sent3.internal.end()) {
    std::cout << (*it2).to_s()+"+";
    it2++;
  }
  std::cout << std::endl;

  std::cout << "external: " << std::endl;
  it2 = sent3.external.begin();
  while(it2 != sent3.external.end()) {
    std::cout << (*it2).to_s()+"+";
    it2++;
  }
  std::cout << std::endl;
  std::cout << std::endl;

  std::cout << "temp:" << std::endl;
  std::cout << "type:" << temp.type << std::endl;
  std::cout << "cat:" << temp.cat << std::endl;

  std::cout << "internal: " << std::endl;
  it2 = temp.internal.begin();
  while(it2 != temp.internal.end()) {
    std::cout << (*it2).to_s()+"+";
    it2++;
  }
  std::cout << std::endl;

  std::cout << "external: " << std::endl;
  it2 = temp.external.begin();
  while(it2 != temp.external.end()) {
    std::cout << (*it2).to_s()+"+";
    it2++;
  }
  std::cout << std::endl;

  if(temp == sent3)
  std::cout << "true" << std::endl;
  else
  std::cout << "false" << std::endl;

  std::cout << temp.to_s() << std::endl;
  return 0;
}
#endif
