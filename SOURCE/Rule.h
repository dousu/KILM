/*
 * Rule.h
 *
 *  Created on: 2011/05/18
 *      Author: Rindow
 */

#ifndef RULE_H_
#define RULE_H_

#include <vector>
#include <string>

#include <iostream>


#include <boost/range/algorithm.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>

#include "Distance.hpp"
#include "Element.h"
#include "IndexFactory.h"

namespace RULE_TYPE {
  const unsigned int SENTENCE = 0;
  const unsigned int NOUN = 1;
}

class RuleTypeDef {
  public:
    typedef std::vector<Element> InType;
    typedef std::vector<Element> ExType;
};

/*!
 * hoge(foo,bar)->xhkdjeoek
 * のようなルールを表現するクラス
 */
class Rule : public RuleTypeDef {
  public:
    //member
    std::vector<Element> internal; //内部言語列
    std::vector<Element> external; //外部言語列

    int type; //文規則か、単語規則かのデータ
    int cat; //単語規則の場合のカテゴリナンバー

    /*!
     * 文字列表現に変換する際に使用する辞書データ
     */
    static Dictionary dictionary;

    //constructor
    Rule();
    Rule(char* cstr);

    /*!
     * デバッグ用ではあるが、文字列表現のRuleを使ってインスタンスを生成する
     */
    Rule(std::string str);

    //destructor
    virtual
    ~Rule();

    //!operator
    bool
    operator==(const Rule& dst) const;
    bool
    operator!=(Rule& dst) const;
    Rule&
    operator=(const Rule& dst);
    //bool operator<(Rule& dst) const;

    //method
    bool
    is_sentence(void) const;
    bool
    is_noun(void) const;
    int
    composition(void) const;

    std::string
    to_s(void);
    void
    set_noun(Element& dcat, Element& dind, ExType& dex);
    void
    set_noun(int dcat, Element& dind, ExType& dex);
    void
    set_sentence(InType& din, ExType& dex);
    
    std::vector< std::vector<Element> >
    moph(void);
    
  private:
    friend class boost::serialization::access;
    template<class Archive>
      void
      serialize(Archive &ar, const unsigned int /* file_version */) {
        ar & BOOST_SERIALIZATION_NVP(type);
        ar & BOOST_SERIALIZATION_NVP(cat);
        ar & BOOST_SERIALIZATION_NVP(internal);
        ar & BOOST_SERIALIZATION_NVP(external);
      }
};

class RuleSort : public RuleTypeDef {
  public:
    bool
    operator()(const Rule &l, const Rule &r) {

      //metric: l is less than r
      if (l.is_sentence() && r.is_noun()) {
        return false;
      }
      else if (l.is_noun() && r.is_sentence()) {
        return true;
      }

      //guard
      if ((!(l.is_sentence()) && !(l.is_noun()))
          || (!(r.is_sentence()) && !(r.is_noun()))) {
        std::cerr << "illegal rule type" << std::endl;

        throw "illegal rule type";
      }

      //through
      //l.is_sentence() && r.is_sentence()
      //l.is_noun() && r.is_noun()
      if (l.is_noun() && r.is_noun()) {
        if (l.cat < r.cat) {
          return true;
        }
        else if (l.cat > r.cat) {
          return false;
        }
      }

      //l.is_sentence() && r.is_sentence()
      //l.is_noun() && r.is_noun() && l.cat == r.cat
      std::vector<Element>::const_iterator lii, rii, lei, rei;
      lii = l.internal.begin();
      rii = r.internal.begin();
      while (lii != l.internal.end() && rii != r.internal.end()) {
        if ((*lii).is_ind() && (*rii).is_ind()) {
          if ((*lii).obj < ((*rii).obj)) {
            return true;
          }
          else if ((*lii).obj > ((*rii).obj)) {
            return false;
          }
        }

        if ((*lii).is_var() && (*rii).is_var()) {
          if ((*lii).cat < ((*rii).cat)) {
            return true;
          }
          else if ((*lii).cat > ((*rii).cat)) {
            return false;
          }

          if ((*lii).cat == ((*rii).cat)) {
            if ((*lii).obj < ((*rii).obj)) {
              return true;
            }
            else if ((*lii).obj > ((*rii).obj)) {
              return false;
            }
          }
        }

        if ((*lii).is_ind() && (*rii).is_var()) {
          return true;
        }

        if ((*lii).is_var() && (*rii).is_ind()) {
          return false;
        }

        lii++;
        rii++;
      }

      //undecided order by internal
      lei = l.external.begin();
      rei = r.external.begin();
      while (lei != l.internal.end() && rei != r.internal.end()) {
        if ((*lei).is_sym() && (*rei).is_sym()) {
          if ((*lei).obj < ((*rei).obj)) {
            return true;
          }
          else if ((*lei).obj > ((*rei).obj)) {
            return false;
          }
        }

        if ((*lei).is_cat() && (*rei).is_cat()) {
          if ((*lei).cat < ((*rei).cat)) {
            return true;
          }
          else if ((*lei).cat > ((*rei).cat)) {
            return false;
          }

          if ((*lei).cat == ((*rei).cat)) {
            if ((*lei).obj < ((*rei).obj)) {
              return true;
            }
            else if ((*lei).obj > ((*rei).obj)) {
              return false;
            }
          }
        }

        if ((*lei).is_sym() && (*rei).is_cat()) {
          return true;
        }

        if ((*lei).is_cat() && (*rei).is_sym()) {
          return false;
        }

        lei++;
        rei++;
      }

      //safety valve
      //std::cerr << "cannot decide order about rule" << std::endl;
      //throw "cannot decide order about rule";

      return false;
    }
};



#endif /* RULE_H_ */
