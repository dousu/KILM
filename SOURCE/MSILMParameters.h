/*
 * LELAParameters.h
 *
 *  Created on: 2011/06/29
 *      Author: rindou
 */

#ifndef MSILMPARAMETERS_H_
#define MSILMARAMETERS_H_

#include <vector>
#include <string>
#include <fstream>
#include <boost/program_options.hpp>
//#include <boost/system/config.hpp>
//#include <boost/filesystem.hpp>
//#include <boost/filesystem/path.hpp>
//#include <boost/filesystem/fstream.hpp>

//#include <boost/shared_ptr.hpp>

#include "Parameters.h"
#include "LogBox.h"



/*!
 * 実行時引数を解釈して、保持するクラス
 */
class MSILMParameters: public Parameters {
public:
	//double CONTACT_PROBABIRITY; //! 他エージェントとの接触確率
	//double NEIGHBOR_PROBABIRITY;// 水平隣接エージェントとの接触確率

	//bool   CONVERT_FLAG;  // stファイルを、文字列表現にするフラグ
	//bool   LINKED_MATRIX; // ネットワークのグラフファイルを渡すフラグ
	bool   DEL_LONG_RULE;
	int DEL_LONG_RULE_LENGTH;

	//boost::filesystem::path CONVERTING_PATH; //文字列表現のもとのファイルパス
	//boost::filesystem::path CONVERTED_PATH; //文字列表現ファイルパス
	//boost::filesystem::path LINKED_MATRIX_PATH; //外部グラフファイルパス

	//std::string CONVERTING_PATH; //文字列表現のもとのファイルパス
	//std::string CONVERTED_PATH; //文字列表現ファイルパス
	//std::string LINKED_MATRIX_PATH; //外部グラフファイルパス

	//std::string igraph_file_name; //渡したグラフファイル名
	//int AGENT_NUM; //エージェント数
	//bool IGRAPH_FLAG; //igraphファイルかのフラグ
	//static int thread; //thread数
	static bool UTTER_MINIMUM;
	static bool INDEX;

	bool INTER_ANALYSIS;
	int SPACE_ANALYSIS;

	bool INTER_LOG;
	int SPACE_LOG;

	int EX_LIMIT;
	//bool EX_CUT;
	//bool PARENT_ONCE;
        
        int MULTIPLE_MEANINGS;
        
        double PER_TERM;
        int TERMS;
        int WINDOW;
        bool SYMMETRY;
        bool MUTUAL_EXCLUSIVITY;
        bool EXCEPTION;
        bool OMISSION_A;
        bool OMISSION_B;
        bool OMISSION_C;
        bool OMISSION_D;
        bool ACC_MEA;

	MSILMParameters();
	virtual ~MSILMParameters();

	void set_option(boost::program_options::variables_map& vm);
	std::string to_s(void);
};

#endif /* MSILMPARAMETERS_H_ */
