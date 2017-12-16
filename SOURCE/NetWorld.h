/*
 * NetWorld.h
 *
 *  Created on: 2011/06/15
 *      Author: Rindow
 */

#ifndef NETWORLD_H_
#define NETWORLD_H_

//ublas matrix用
#define NODEBUG

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <igraph.h>

#include "KirbyAgent.h"
#include "Rule.h"
#include "MT19937.h"
#include "LogBox.h"
#include "LEILAParameters.h"


/*!
 * 複数のエージェントが、ネットワークで接続された世界を表現するクラス
 */
class NetWorld {
public:
	std::vector<KirbyAgent> agents; //エージェントの配列

	/*!
	 * エージェント数。ただしこれはコンストラクタでのみ使用されるため、
	 * そのときのエージェント数を意味しない
	 */
	static int agent_num;
	static bool LOGGING;
	static double contact_probabirity;
	static double neighbor_probabirity;
	static boost::numeric::ublas::matrix<double> connected_matrix;
	//static bool EX_CUTTER;
	//static int EX_LIMIT;
	static bool PARENT_ONCE;

	NetWorld();
	virtual ~NetWorld();

	NetWorld& operator=(const NetWorld& obj);

	/*!
	 * 代入されたネットワーク、エージェント数に応じて、世界を構築する。
	 * 事実上のコンストラクタ
	 */
	void build_world(void);

	/*!
	 * ネットワークを新規に構築する。
	 */
	void make_graph(void);

	/*!
	 * エージェント数に応じてエージェントを生成する
	 */
	void make_agents(void);

	/*!
	 * 世代を交代する。返すのは自分自身。
	 */
	NetWorld& grow_up(std::vector<Rule>);
	static void
	tf_grow_up(int, int,std::vector<KirbyAgent>&, std::vector<Rule>&);

	/*!
	 * 引数に渡されたNetWorld世代を教育する。
	 * 教育とは、発話してそれを次世代に渡し、次世代はその発話をもとに学習することである。
	 */
	void educate(std::vector<Rule>& meanings, NetWorld& next_gen);

	/*!
	 * エージェントが受け取った発話をつかい学習を行う。
	 */
	void learn(void);
	static void tf_learn(int, int, std::vector<KirbyAgent>&);

	/*!
	 * 次世代を生成する。この意味は、現世代と同じ数のエージェント、同じネットワーク、
	 * そういうNetWorldのインスタンスを生成すること。
	 */
	NetWorld make_generation(void);

	/*!
	 * 自分自身の世代数を返す
	 */
	int get_generation(void);

	/*!
	 * 世代の文字列表現を返す
	 */
	std::string to_s(void);

	/*!
	 * 引数のインデックスを持つエージェントと接続しているエージェントのインデックス集合を返す
	 */
	std::vector<int> get_neighbors(int p);

	std::vector<double> get_nei_prob_map(int p);
	boost::numeric::ublas::matrix<double> create_probabirity_map(int p);
	boost::numeric::ublas::matrix<double> create_triangular_matrix(int i, int j);
	static void logging_on(void);
	static void logging_off(void);

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int /* file_version */){
    	ar & BOOST_SERIALIZATION_NVP(contact_probabirity);
    	ar & BOOST_SERIALIZATION_NVP(neighbor_probabirity);
    	ar & BOOST_SERIALIZATION_NVP(agent_num);
    	ar & BOOST_SERIALIZATION_NVP(agents);
    	ar & BOOST_SERIALIZATION_NVP(connected_matrix);
    }
};

#endif /* NETWORLD_H_ */
