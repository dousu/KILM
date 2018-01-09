/*
 * Dictionary.h
 *
 *  Created on: 2012/11/23
 *      Author: Hiroki Sudo
 */

#ifndef DICTIONARY_H_
#define DICTIONARY_H_
#include <map>
#include <string>
#include <vector>
#include <fstream>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>

/*!
 * Kirbyモデルで使用する単語の辞書を提供します。例えば、内部言語列の単語「like」や「hate」など。
 * また同時に外部言語列の文字も提供します。例えば、「a,b,c,d」など。
 * この辞書を作成しなければ全てが動きません。
 *
 *
 * 辞書ファイルのフォーマットについて
 * デフォルトの辞書は以下のようになっています
 *
 \code
 * IND=admire,detest,hate,like,love,john,mary,pete,heather,gavin
 * SYM=a,b,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,t,u,v,w,y,z
 \endcode
 *
 * INDで内部言語の終端記号をカンマ区切りで並べ、定義します。また、
 * SYMで外部言語の終端記号をカンマ区切りで並べ、定義します。
 *
 * なお、boost/serializationに対応しています
 */
class Dictionary {
public:
	typedef std::map<int, std::string> DictionaryType;

	static std::map<int, std::string> individual;
	static std::map<int, std::string> symbol;

	static std::map<std::string, int> conv_individual;
	static std::map<std::string, int> conv_symbol;

	/*!
	 * 辞書ファイルを読み込みます。引数にはファイルパスを入れます。\n
	 * 例:\n
	 \code
	 * Dictionary::load(boost::filesystem::path("/hoge/foo/dic.dat"));
	 \endcode
	 */
	//static void load(boost::filesystem::path file_path);
	static void load(std::string&);

	static Dictionary copy(void);

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int /* file_version */){
    	ar & BOOST_SERIALIZATION_NVP(individual);
    	ar & BOOST_SERIALIZATION_NVP(symbol);
    	ar & BOOST_SERIALIZATION_NVP(conv_individual);
    	ar & BOOST_SERIALIZATION_NVP(conv_symbol);
    }
};

#endif /* DICTIONARY_H_ */
