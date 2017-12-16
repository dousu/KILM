/*
 * NetWorld.cpp
 *
 *  Created on: 2011/06/15
 *      Author: Rindow
 */

#include "NetWorld.h"

double NetWorld::contact_probabirity = 0;
double NetWorld::neighbor_probabirity = 0;
int NetWorld::agent_num = 1;
bool NetWorld::LOGGING = false;
boost::numeric::ublas::matrix<double> NetWorld::connected_matrix(0, 0);
//bool NetWorld::EX_CUTTER = false;
//int NetWorld::EX_LIMIT = 0;
bool NetWorld::PARENT_ONCE = false;

NetWorld::NetWorld() {
  // TODO Auto-generated constructor stub
//	igraph_empty(network, 0, 0);
}

NetWorld::~NetWorld() {
  // TODO Auto-generated destructor stub
}

/*
 * Construct World
 */

void
NetWorld::build_world(void) {
  if (agent_num <= 0) {
    std::cerr << "Unset a number of agents" << std::endl;
    throw "Unset a number of agents";
  }

  //igraphのライブラリで、
  //なぜか知らないが、この関数を抜けるとGraphデータが壊れるw
  //使い物にならないので、正常に使えるmake_graph関数内で
  //接続行列に変換する
  make_graph();
  make_agents();
}

void
NetWorld::make_graph(void) {
  //making graph
  igraph_vector_t v;
  igraph_t network;

  igraph_vector_init(&v, 0);
  igraph_barabasi_game(&network, agent_num, 1, &v, 0, 0);
  igraph_vector_destroy(&v);

  //convert graph
  int vertex_num, to, vector_end;
  igraph_vector_t neighbors;

  //initialize
  vertex_num = (int) igraph_vcount(&network);
  connected_matrix.resize(vertex_num, vertex_num);
  connected_matrix.clear();

  for (int from = 0; from < vertex_num; from++) {
    igraph_vector_init(&neighbors, 0);
    igraph_neighbors(&network, &neighbors, (igraph_integer_t) from,
        IGRAPH_ALL/*無視される*/);
    igraph_vector_sort(&neighbors);
    vector_end = igraph_vector_size(&neighbors);
    for (int index = 0; index < vector_end; index++) {
      to = VECTOR(neighbors)[index];
      connected_matrix(from, to) = 1;
    }
  }

  igraph_vector_destroy(&neighbors);
  igraph_destroy(&network);
}

void
NetWorld::make_agents(void) {
  for (int i = 0; i < agent_num; i++) {
    KirbyAgent agent;
    agents.push_back(agent);
  }
}

NetWorld
NetWorld::make_generation(void) {
  NetWorld next_gen;
  std::vector<KirbyAgent>::iterator a_it;
  a_it = agents.begin();
  for (; a_it != agents.end(); a_it++) {
    next_gen.agents.push_back((*a_it).make_child());
  }

  return next_gen;
}

std::vector<int>
NetWorld::get_neighbors(int p) {
  if (p > connected_matrix.size1()) {
    std::cerr << "over range of pibot" << std::endl;
    throw "over range of pibot";
  }

  std::vector<int> neighbors;
  for (int i = 0; i < connected_matrix.size2(); i++) {
    if (connected_matrix(p, i) != 0)
      neighbors.push_back(i);
  }

  return neighbors;
}
std::vector<double>
NetWorld::get_nei_prob_map(int p) {
  if (p > connected_matrix.size1()) {
    std::cerr << "over range of pibot" << std::endl;
    throw "over range of pibot";
  }

  std::vector<double> neighbors;
  for (int i = 0; i < connected_matrix.size2(); i++) {
    if (connected_matrix(p, i) != 0)
      neighbors.push_back(connected_matrix(p, i));
  }

  return neighbors;
}

int
NetWorld::get_generation(void) {
  if (agents.size() == 0)
    return -1;
  else
    return agents.front().generation_index;
}

void
NetWorld::logging_on(void) {
  KirbyAgent::logging_on();
  LOGGING = true;
}

void
NetWorld::logging_off(void) {
  KirbyAgent::logging_off();
  LOGGING = false;
}

NetWorld&
NetWorld::operator=(const NetWorld& obj) {
  agents = obj.agents;

  return *this;
}

NetWorld&
NetWorld::grow_up(std::vector<Rule> meanings) {
  for (int i = 0; i < agents.size(); i++) {
    agents[i].grow(meanings);
  }

  //if (LELAParameters::thread > 1) {
  if (false) {
    int part_size;
    int mod;
    part_size = agents.size() / LELAParameters::thread;
    mod = agents.size() % LELAParameters::thread;
    std::vector<KirbyAgent> ret;
    int current = 0;


    boost::thread_group ths;
    while (current + part_size + mod < agents.size()){
      if(current + part_size + mod == agents.size()){
        ths.create_thread(boost::bind(tf_grow_up, current, agents.size(), boost::ref(agents), boost::ref(meanings)));
      }
      else{
        ths.create_thread(boost::bind(tf_grow_up, current, current+part_size, boost::ref(agents), boost::ref(meanings)));
      }
      current += part_size;
    }

    ths.join_all();
  }
  else {
    for (int i = 0; i < agents.size(); i++) {
      agents[i].grow(meanings);
    }
  }

  return *this;
}

void
NetWorld::tf_grow_up(int start, int end, std::vector<KirbyAgent>& agents, std::vector<Rule>& meanings){
  for (int i = start; i < end; i++) {
    agents[i].grow(meanings);
  }
}

void
NetWorld::educate(std::vector<Rule>& meanings, NetWorld& next_gen) {
  std::vector<KirbyAgent>::iterator parent_it, child_it;
  std::vector<Rule> meanings_copy;
  std::vector<int> neighbors_vec;
  Rule used_meaning, utter;
  int meaning_index, agent_index;
  double contact_dice;

  parent_it = agents.begin();
  child_it = next_gen.agents.begin();
  agent_index = 0;
  while (child_it != next_gen.agents.end()) {
    meanings_copy = meanings;
    if (LOGGING) {
      std::ostringstream os;
      os << "NOW AGENT " << agent_index;
      LogBox::push_log(os.str());
    }
#ifdef DEBUG
        std::cerr << "Start Educate";
#endif
    //とりあえず、普通の垂直
    //同じ順序のAgentは同じノードに載ってるとする

    //発話意味選択
    meaning_index = MT19937::irand() % meanings_copy.size();
    used_meaning = meanings_copy[meaning_index];

    if (LOGGING) {
      LogBox::push_log(
          "MEANING INDEX: [" + boost::lexical_cast<std::string>(meaning_index)
              + "]");
    }

    //接触選択
    contact_dice = MT19937::rrand();
    if (contact_probabirity > contact_dice) {
      //隣接ノードID抽出
      neighbors_vec = get_neighbors(agent_index);

      if (LOGGING) {
        std::vector<std::string> log_buf;
        for (std::vector<int>::iterator buf_it = neighbors_vec.begin();
            buf_it != neighbors_vec.end(); buf_it++) {
          log_buf.push_back(boost::lexical_cast<std::string>(*buf_it));
        }

        LogBox::push_log(
            "NEIGHBORS: [" + boost::algorithm::join(log_buf, ",") + "]");
      }

      if (neighbors_vec.size() == 0) {
        /*
         * 隣接ノードがない場合、親が発話?
         * BA modelではあり得ないが
         */
        if (LOGGING) {
          LogBox::push_log(
              "P_AGENT:" + boost::lexical_cast<std::string>(agent_index)
                  + " -(UTTER)-> " + "C_AGENT:"
                  + boost::lexical_cast<std::string>(agent_index)
                  + " :ERROR(NO NEIGHBORS)");
        }
        utter = (*parent_it).say(used_meaning);
      }
      else {
        int neighbor_index = -1;
        double nei_prob;
        boost::numeric::ublas::matrix<double> prob;

//				neighbor_index = MT19937::irand() % neighbors_vec.size();
        nei_prob = MT19937::rrand();
        prob = create_probabirity_map(agent_index);

        if (LOGGING) {
          std::ostringstream os;
          os << prob;
          LogBox::push_log("NEI PROB: [" + os.str() + "]");
          LogBox::push_log(
              "USE DICE: " + boost::lexical_cast<std::string>(nei_prob));
        }

        for (int i = 0; i < prob.size2(); i++) {
          if (nei_prob < prob(0, i)) {
            neighbor_index = neighbors_vec[i];
            break;
          }
        }
        if (neighbor_index == -1)
          neighbor_index = neighbors_vec.back();

        //中村さん仕様
        if(!PARENT_ONCE){
          neighbors_vec.push_back(agent_index);
          neighbor_index = neighbors_vec[MT19937::irand() % neighbors_vec.size()];
        }


#ifdef DEBUG
        std::cerr << "-> End Educate" << std::endl;
        std::cerr << "Start Say";
#endif
        //ここはまだ隣接同世代が発話するのか、隣接前世代が発話するのか決まってない
        //とりあえず、前世代
        contact_dice = MT19937::rrand();
        if (neighbor_probabirity > contact_dice) {
          if (LOGGING) {
            LogBox::push_log(
                "C_AGENT:" + boost::lexical_cast<std::string>(neighbor_index)
                    + " -UTTER-> " + "C_AGENT:"
                    + boost::lexical_cast<std::string>(agent_index));
          }
          utter = next_gen.agents[neighbor_index].say(used_meaning);
        }
        else {

          if (LOGGING) {
            LogBox::push_log(
                "P_AGENT:" + boost::lexical_cast<std::string>(neighbor_index)
                    + " -UTTER-> " + "C_AGENT:"
                    + boost::lexical_cast<std::string>(agent_index));
          }

          utter = agents[neighbor_index].say(used_meaning);
        }
#ifdef DEBUG
        std::cerr << " -> End Say" << std::endl;
#endif
      }
    }
    else {
      if (LOGGING) {
        LogBox::push_log(
            "P_AGENT:" + boost::lexical_cast<std::string>(agent_index)
                + " -UTTER-> " + "C_AGENT:"
                + boost::lexical_cast<std::string>(agent_index));
      }
      utter = (*parent_it).say(used_meaning);
    }

    /*
    if(EX_CUTTER){
    	if(EX_LIMIT != 0 && utter.external.size() > EX_LIMIT){
    		for(int i = 0 ; i < utter.external.size() - EX_LIMIT ; i++){
    			utter.external.pop_back();
    		}
    	}
    }
     */

    (*child_it).hear(utter);

    parent_it++;
    child_it++;
    agent_index++;
  }
}

void
NetWorld::learn(void) {
  //if (LELAParameters::thread > 1) {
  if (false) {
    int part_size;
    int mod;
    part_size = agents.size() / LELAParameters::thread;
    mod = agents.size() % LELAParameters::thread;
    std::vector<KirbyAgent> ret;
    int current = 0;


    boost::thread_group ths;
    while (current + part_size + mod < agents.size()){
      if(current + part_size + mod == agents.size()){
        ths.create_thread(boost::bind(tf_learn, current, agents.size(), boost::ref(agents)));
      }
      else{
        ths.create_thread(boost::bind(tf_learn, current, current+part_size, boost::ref(agents)));
      }
      current += part_size;
    }

    ths.join_all();
  }
  else {
    for (int i = 0; i < agents.size(); i++) {
      agents[i].learn();
    }
  }

  //std::cerr << "Learnned";
}

void
NetWorld::tf_learn(int start, int end, std::vector<KirbyAgent>& agents) {
  for (int i = start; i < end; i++) {
    agents[i].learn();
  }
}

boost::numeric::ublas::matrix<double>
NetWorld::create_probabirity_map(int p) {
  std::vector<double> prob;
  std::vector<double>::iterator prob_it;
  boost::numeric::ublas::matrix<double> tri_id, prob_mx;
  double pd = 0;

  prob = get_nei_prob_map(p);
  prob_mx.resize(1, prob.size());

  for (int i = 0; i < prob.size(); i++) {
    prob_mx(0, i) = prob[i];
  }
  for (int i = 0; i < prob.size(); i++) {
    pd += prob_mx(0, i);
  }
  tri_id = create_triangular_matrix(prob.size(), prob.size());
  prob_mx = boost::numeric::ublas::prod(prob_mx, tri_id);

  /*
   if(LOGGING){
   std::ostringstream os;
   os << "tri: " << tri_id << std::endl;
   os << "prob: " << prob_mx << std::endl;
   LogBox::push_log(os.str());
   }
   */
  prob_mx = prob_mx / pd;

  return prob_mx;
}

boost::numeric::ublas::matrix<double>
NetWorld::create_triangular_matrix(int i, int j) {
  boost::numeric::ublas::matrix<double> tri_id(0, 0);
  int k, l;
  tri_id.resize(i, j);
  tri_id.clear();

  for (k = 0; k < i; k++) {
    for (l = k; l < j; l++) {
      tri_id(k, l) = 1;
    }
  }
  return tri_id;
}

std::string
NetWorld::to_s(void) {
  std::string str = "";

  str += "GENERATION ";
  str += boost::lexical_cast<std::string>(get_generation());
  str += "\n";

  for (int index = 0; index < agents.size(); index++) {
    str += "AGENT ";
    str += boost::lexical_cast<std::string>(index);
    str += "\n";
    str += agents[index].to_s();
  }

  return str;
}

