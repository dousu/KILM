/*
 * LELA_main.cpp
 *
 *  Created on: 2011/06/29
 *      Author: rindou
 */

/*
 * main.cpp
 *
 *  Created on: 2011/06/07
 *      Author: Rindow
 */

#include "LEILA_main.h"

//this flag for boost ublas library
#define NODEBUG

/********************************
 * SAVE AND LOAD
 ********************************/
template<typename _IFS>
void resume_agents(_IFS& ifs, LELAParameters& param,
		unsigned long long int& icount, unsigned long long int& rcount,
		Dictionary& dic, std::vector<Rule>& meanings, NetWorld& networld) {
	std::string file_name;
	ifs >> boost::serialization::make_nvp("UUF", param.UNIQUE_UTTERANCE);
	ifs >> boost::serialization::make_nvp("RS", param.RANDOM_SEED);
	ifs >> boost::serialization::make_nvp("IRC", icount);
	ifs >> boost::serialization::make_nvp("RRC", rcount);

	ifs >> boost::serialization::make_nvp("MS", meanings);
	ifs >> boost::serialization::make_nvp("DC", dic);

	ifs >> boost::serialization::make_nvp("LA", networld);
}

template<typename _OFS>
void save_agents(_OFS& ofs, LELAParameters& param,
		unsigned long long int& icount, unsigned long long int& rcount,
		Dictionary& dic, std::vector<Rule>& meanings, NetWorld& networld) {
	ofs << boost::serialization::make_nvp("UUF", param.UNIQUE_UTTERANCE);
	ofs << boost::serialization::make_nvp("RS", param.RANDOM_SEED);
	ofs << boost::serialization::make_nvp("IRC", icount);
	ofs << boost::serialization::make_nvp("RRC", rcount);

	ofs << boost::serialization::make_nvp("MS", meanings);
	ofs << boost::serialization::make_nvp("DC", dic);

	ofs << boost::serialization::make_nvp("LA", networld);
}

/********************************
 * CONSTRUCT BASE DATA
 ********************************/
void construct_meanings(std::vector<Rule>& meanings) {
	int VERB_INDEX_BEGIN = 0;
	int VERB_INDEX_END = 4;
	int NOUN_INDEX_BEGIN = 5;
	int NOUN_INDEX_END = 9;

	//construct means
	for (int i = VERB_INDEX_BEGIN; i <= VERB_INDEX_END; i++) {
		Element verb;
		verb.set_ind(i);

		for (int j = NOUN_INDEX_BEGIN; j <= NOUN_INDEX_END; j++) {
			Element ind1;
			ind1.set_ind(j);
			for (int k = NOUN_INDEX_BEGIN; k <= NOUN_INDEX_END; k++) {
				if (j != k) {
					Element ind2;
					std::vector<Element> internal, external;
					Rule mean;

					ind2.set_ind(k);

					internal.push_back(verb);
					internal.push_back(ind1);
					internal.push_back(ind2);

					mean.set_sentence(internal, external);
					meanings.push_back(mean);
				}
			}
		}
	}
}

void construct_individuals(std::vector<Element>& inds, Dictionary &dic) {
	Dictionary::DictionaryType::iterator dic_it;
	dic_it = dic.individual.begin();
	for (; dic_it != dic.individual.end(); dic_it++) {
		Element elem;
		elem.set_ind((*dic_it).first);
		inds.push_back(elem);
	}
}

/********************************
 * ANALYZE RESULT DATA
 ********************************/

std::vector<boost::numeric::ublas::matrix<double> > analyze(
		std::vector<Rule>& meanings, std::vector<Element>& individuals,
		NetWorld& world1, NetWorld& world2) {
    
	boost::numeric::ublas::matrix<double> unit_result(0, 0), inter_result_sent(
			0, 0), inter_result_word(0, 0);
	
        boost::numeric::ublas::matrix<double> effect_sent(0, 0), effect_word(0, 0);
	
        std::vector<boost::numeric::ublas::matrix<double> > result;
	
        int unit_analyze_item_num = 4, vertices_num = 0;

	//行列の初期化
	vertices_num = NetWorld::agent_num;
	effect_sent.resize(vertices_num, vertices_num);
	effect_word.resize(vertices_num, vertices_num);
	unit_result.resize(vertices_num, unit_analyze_item_num);
	inter_result_sent.resize(vertices_num, vertices_num);
	inter_result_word.resize(vertices_num, vertices_num);

	effect_sent.clear();
	effect_word.clear();
	unit_result.clear();
	inter_result_sent.clear();
	inter_result_word.clear();

	//Agent単体の分析
	unit_analyze(unit_result, meanings, world1);

	//Agent相互の分析
	calculate_language_distance(inter_result_sent, inter_result_word, meanings,
			individuals, world1, world2);

	//今のところ三角行列になってるからそれを変えてる
	//inter_result_sent = boost::numeric::ublas::trans(inter_result_sent) + inter_result_sent;
	//inter_result_word = boost::numeric::ublas::trans(inter_result_word) + inter_result_word;

	//多分距離行列×接続行列で前世代との差分を使えばRecurrent Network見たいな重み評価に使える
	//が今は対角だけつかう（近傍変異）
	effect_sent = boost::numeric::ublas::prod(inter_result_sent,
			NetWorld::connected_matrix);
	effect_word = boost::numeric::ublas::prod(inter_result_word,
			NetWorld::connected_matrix);

	//normalize(effect_sent, world);
	//normalize(effect_word, world1);

	//リターンバケットに結果をつめて返す
	result.push_back(unit_result);
	result.push_back(inter_result_sent);
	result.push_back(inter_result_word);
	result.push_back(effect_sent);
	result.push_back(effect_word);

	return result;
}

void unit_analyze(boost::numeric::ublas::matrix<double>& result_matrix,
		std::vector<Rule>& meanings, NetWorld& world) {
	int index = 0, max_index;
	int GEN = 0, EXP = 1, SRN = 2, WRN = 3;

	max_index = world.agents.size();
	for (index = 0; index < max_index; index++) {
		//Item1 :generation number
		result_matrix(index, GEN) = world.get_generation();

		//Item2 :expressiveness
		result_matrix(index, EXP) = expression(meanings, world.agents[index]);

		//Item3 :sentence rule number
		result_matrix(index, SRN) = world.agents[index].kb.sentenceDB.size();

		//Item4 :Word rule number
		result_matrix(index, WRN) = world.agents[index].kb.wordDB.size();
	}
}

int expression(std::vector<Rule>& meanings, KirbyAgent& agent) {
	std::vector<Rule>::iterator mean_it;
	int counter = 0;
	mean_it = meanings.begin();
	while (mean_it != meanings.end()) {
		if (agent.understand(*mean_it))
			counter++;
		mean_it++;
	}

	return counter;
}

void calculate_language_distance(
		boost::numeric::ublas::matrix<double>& lev_sent_matrix,
		boost::numeric::ublas::matrix<double>& lev_word_matrix,
		std::vector<Rule>& meanings, std::vector<Element>& words,
		NetWorld& world1, NetWorld& world2) {
	/*
	 * 1．意味を一つ取り出す（上から順に）
	 * 2．kb1とkb2から意味に対応したグラウンドパターンを取り出す
	 * 3．パターンに含まれる文法規則だけ取り出す
	 * 4．kb1、kb2の適用可能な文法規則の全ての組み合わせについて、レーベンシュタイン距離を計算する
	 * 5．計算したレーベンシュタイン距離を加算する
	 * 6．全ての意味について上記を繰り返す
	 *
	 * 1．kb1とkb2について単語規則を取り出す
	 * 2．同じ内部言語列をもつ単語規則について、全ての組み合わせに対してレーベンシュタイン距離を計算する
	 * 3．加算する
	 *
	 * これを全てのkb組み合わせについて計算する
	 * omoina-
	 * */

	int agent_MAX;
	int agent_index1 = 0;
	int agent_index2 = 0;
	agent_MAX = NetWorld::agent_num;

	for (agent_index1 = 0; agent_index1 < agent_MAX; agent_index1++) {
		for (agent_index2 = 0; agent_index2 < agent_MAX; agent_index2++) {
			/*
			 if (LELAParameters::thread >= 2) {
			 lev_sent_matrix(agent_index1, agent_index2) = cal_SMAD_th(meanings,
			 world1.agents[agent_index1].kb, world2.agents[agent_index2].kb);
			 }

			 else if (LELAParameters::INDEX) {
			 lev_sent_matrix(agent_index1, agent_index2) =
			 calculate_sentence_minimum_average_distance_with_index(meanings,
			 world1.agents[agent_index1].kb, world2.agents[agent_index2].kb);
			 }
			 */
			//else {
//			lev_sent_matrix(agent_index1, agent_index2) =
//					calculate_sentence_minimum_average_distance(meanings,
//							world1.agents[agent_index1].kb,
//							world2.agents[agent_index2].kb);
                    lev_sent_matrix(agent_index1, agent_index2) =
                            calculate_sudo_distance(meanings,
                                world1.agents[agent_index1].kb,
				world2.agents[agent_index2].kb);
			//}
		}
	}

	for (agent_index1 = 0; agent_index1 < agent_MAX; agent_index1++) {
		for (agent_index2 = 0; agent_index2 < agent_MAX; agent_index2++) {
			lev_word_matrix(agent_index1, agent_index2) =// 0;
					calculate_word_distance(words,
							world1.agents[agent_index1].kb,
							world2.agents[agent_index2].kb);
		}
	}
}

double calculate_sentence_distance(std::vector<Rule>& meanings,
		KnowledgeBase& kb1, KnowledgeBase& kb2) {
    
	std::vector<Rule>::iterator mean_it, kb1_pat_it, kb2_pat_it;
	std::vector<Rule> kb1_pat, kb2_pat;

	mean_it = meanings.begin();
	double lev_s = 0;
	int cnt = 0;

	for (; mean_it != meanings.end(); mean_it++) {
		kb1_pat = kb1.groundable_rules(*mean_it);
		kb2_pat = kb2.groundable_rules(*mean_it);

		kb1_pat_it = kb1_pat.begin();
		kb2_pat_it = kb2_pat.begin();

		if (kb1_pat.size() == 0 && kb2_pat.size() != 0) {
			lev_s += 1;
			cnt++;
			continue;
		} else if (kb1_pat.size() != 0 && kb2_pat.size() == 0) {
			lev_s += 1;
			cnt++;
			continue;
		}

		for (; kb1_pat_it != kb1_pat.end(); kb1_pat_it++) {
			for (kb2_pat_it = kb2_pat.begin(); kb2_pat_it != kb2_pat.end();
					kb2_pat_it++) {
				lev_s += Distance::levenstein((*kb1_pat_it).external,
						(*kb2_pat_it).external);
				cnt++;
			}
		}
	}

	if (cnt != 0)
		return lev_s / (double) cnt;
	else
		return lev_s;
}

void tf_cal_smad(std::vector<Rule>& meanings, KnowledgeBase& kb1,
		KnowledgeBase& kb2, boost::shared_ptr<double> ret) {
	std::vector<Rule>::iterator mean_it, kb1_pat_it, kb2_pat_it;
	std::vector<Rule> kb1_pat, kb2_pat;
	//std::cerr << "In Thread" << std::endl;
	mean_it = meanings.begin();
	double lev_s = 0;
	double local_lev = 0;
	double local_lev_sum = 0;

	for (; mean_it != meanings.end(); mean_it++) {
		kb1_pat = kb1.grounded_rules(*mean_it);
		kb2_pat = kb2.grounded_rules(*mean_it);

		kb1_pat_it = kb1_pat.begin();
		kb2_pat_it = kb2_pat.begin();

		local_lev = 1;
		lev_s = 1;

		if (kb1_pat.size() == 0 && kb2_pat.size() != 0) {
			local_lev = 1;
		} else if (kb1_pat.size() != 0 && kb2_pat.size() == 0) {
			local_lev = 1;
		} else if (kb1_pat.size() == 0 && kb2_pat.size() == 0) {
			local_lev = 1;
		} else {
			for (; kb1_pat_it != kb1_pat.end(); kb1_pat_it++) {
				for (kb2_pat_it = kb2_pat.begin(); kb2_pat_it != kb2_pat.end();
						kb2_pat_it++) {
					lev_s = Distance::levenstein((*kb1_pat_it).external,
							(*kb2_pat_it).external);
					if (lev_s < local_lev)
						local_lev = lev_s;

					if (local_lev > 1)
						throw;
				}
			}
		}
		local_lev_sum += local_lev;
	}

	*ret = local_lev_sum;
}

double cal_SMAD_th(std::vector<Rule>& meanings, KnowledgeBase& kb1,
		KnowledgeBase& kb2) {
	std::vector<Rule>::iterator mean_it, kb1_pat_it, kb2_pat_it;
	std::vector<Rule> kb1_pat, kb2_pat;

	mean_it = meanings.begin();
	double lev_s = 0;
	double local_lev = 0;
	double local_lev_sum = 0;

	std::vector<std::vector<Rule> > mean_parts;
	int part_size;
	int mod;
	int current_index = 0;
	int midx = 0;
	part_size = meanings.size() / LELAParameters::thread;
	mod = meanings.size() % LELAParameters::thread;

	//std::cerr << "Cal: " << "PS:" << part_size << " mod:" << mod << std::endl;
	while (midx < LELAParameters::thread) {
		mean_parts.push_back(std::vector<Rule>());

		if (current_index + part_size + mod == meanings.size()) {
			//std::cerr << "In tail" << std::endl;
			for (int j = current_index; j < meanings.size(); j++) {
				mean_parts[midx].push_back(meanings[j]);
			}
		} else {
			for (int j = current_index; j < current_index + part_size; j++) {
				//std::cerr << "push now " << midx << ":" << j << std::endl;
				mean_parts[midx].push_back(meanings[j]);
			}
		}

		current_index += part_size;
		midx++;
	}

	/*
	 std::cerr << "View Means:" << mean_parts.size() << std::endl;
	 for (int i = 0; i < mean_parts.size(); i++) {
	 for (int j = 0; j < mean_parts[i].size(); j++) {
	 std::cerr << "M(" << i << "," << j << "): " << mean_parts[i][j].to_s()
	 << std::endl;
	 }
	 }
	 */

	//std::cerr << "Start Threads" << std::endl;
	std::vector<boost::shared_ptr<double> > rv;
	boost::thread_group threads;
	for (int i = 0; i < LELAParameters::thread; i++) {
		boost::shared_ptr<double> temp_ptr(new double());
		rv.push_back(temp_ptr);
		//std::cerr << "Thread Run" << std::endl;
		threads.create_thread(
				boost::bind(tf_cal_smad, boost::ref(mean_parts[i]),
						boost::ref(kb1), boost::ref(kb2), temp_ptr));
	}
	//std::cerr << "Join Threads" << std::endl;
	threads.join_all();

	//std::cerr << "Aggregate Threads" << std::endl;
	for (int i = 0; i < rv.size(); i++) {
		local_lev_sum += *(rv[i]);
	}

	return local_lev_sum / (static_cast<double>(meanings.size()));
}

double calculate_sentence_minimum_average_distance_with_index(
		std::vector<Rule>& meanings, KnowledgeBase& kb1, KnowledgeBase& kb2) {

	std::vector<Rule>::iterator mean_it, kb1_pat_it, kb2_pat_it;
	std::vector<Rule> kb1_pat, kb2_pat;

	mean_it = meanings.begin();
	double lev_s = 0;
	double local_lev = 0;
	double local_lev_sum = 0;
	if (kb1.indexed && kb2.indexed) {
		for (; mean_it != meanings.end(); mean_it++) {

			std::vector<int> mean_vec;
			for (int j = 0; j < mean_it->internal.size(); j++) {
				mean_vec.push_back(mean_it->internal[j].obj);
			}

			//std::cerr << "A";
			for (int i = 0;
					i
							< (*(kb1.fabricate_index))[mean_vec][KnowledgeBase::ABSOLUTE].size();
					i++) {
				kb1_pat.push_back(
						(*(kb1.fabricate_index))[mean_vec][KnowledgeBase::ABSOLUTE][i]);
			}
			//std::cerr << "B";
			for (int i = 0;
					i
							< (*(kb1.fabricate_index))[mean_vec][KnowledgeBase::COMPLETE].size();
					i++) {
				kb1_pat.push_back(
						(*(kb1.fabricate_index))[mean_vec][KnowledgeBase::COMPLETE][i]);
			}
			//std::cerr << "C";

			for (int i = 0;
					i
							< (*(kb2.fabricate_index))[mean_vec][KnowledgeBase::ABSOLUTE].size();
					i++) {
				kb2_pat.push_back(
						(*(kb2.fabricate_index))[mean_vec][KnowledgeBase::ABSOLUTE][i]);
			}
			for (int i = 0;
					i
							< (*(kb2.fabricate_index))[mean_vec][KnowledgeBase::COMPLETE].size();
					i++) {
				kb2_pat.push_back(
						(*(kb2.fabricate_index))[mean_vec][KnowledgeBase::COMPLETE][i]);
			}

			kb1_pat_it = kb1_pat.begin();
			kb2_pat_it = kb2_pat.begin();

			local_lev = 1;
			lev_s = 1;

			if (kb1_pat.size() == 0 && kb2_pat.size() != 0) {
				local_lev = 1;
			} else if (kb1_pat.size() != 0 && kb2_pat.size() == 0) {
				local_lev = 1;
			} else if (kb1_pat.size() == 0 && kb2_pat.size() == 0) {
				local_lev = 1;
			} else {
				for (; kb1_pat_it != kb1_pat.end(); kb1_pat_it++) {
					for (kb2_pat_it = kb2_pat.begin();
							kb2_pat_it != kb2_pat.end(); kb2_pat_it++) {
						lev_s = Distance::levenstein((*kb1_pat_it).external,
								(*kb2_pat_it).external);
						if (lev_s < local_lev)
							local_lev = lev_s;

						if (local_lev > 1)
							throw;
					}
				}
			}
			local_lev_sum += local_lev;
		}

		return local_lev_sum / (static_cast<double>(meanings.size()));
	} else {
		return calculate_sentence_minimum_average_distance(meanings, kb1, kb2);
	}
}

double calculate_sentence_minimum_average_distance(std::vector<Rule>& meanings,
		KnowledgeBase& kb1, KnowledgeBase& kb2) {
    
	std::vector<Rule>::iterator mean_it, kb1_pat_it, kb2_pat_it;
	std::vector<Rule> kb1_pat, kb2_pat;

	//std::cerr << "In calculate" << std::endl;
 
	
	mean_it = meanings.begin();
	double lev_s = 0;
	double local_lev = 0;
	double local_lev_sum = 0;

	for (int m = 0; m < meanings.size(); m++) {
		kb1_pat = kb1.grounded_rules(meanings[m]);
		kb2_pat = kb2.grounded_rules(meanings[m]);

		local_lev = 1;
		lev_s = 1;

		if (kb1_pat.size() == 0 || kb2_pat.size() == 0) {
			local_lev = 1;
		} else {
			for (int p1 = 0; p1 < kb1_pat.size(); p1++) {
				for (int p2 = 0; p2 < kb2_pat.size(); p2++) {
					lev_s = Distance::levenstein(kb1_pat[p1].external, kb2_pat[p2].external);
					//lev_s = Distance::onp_lv(kb1_pat[p1].external, kb2_pat[p2].external, local_lev);
					if (lev_s < local_lev)
						local_lev = lev_s;
				}
			}
		}

		local_lev_sum += local_lev;
	}

	//std::cerr << "Out calculate" << std::endl;
	return local_lev_sum / (static_cast<double>(meanings.size()));
}

double calculate_sentence_minimum_distance(std::vector<Rule>& meanings,
		KnowledgeBase& kb1, KnowledgeBase& kb2) {
	std::vector<Rule>::iterator mean_it, kb1_pat_it, kb2_pat_it;
	std::vector<Rule> kb1_pat, kb2_pat;

	mean_it = meanings.begin();
	double lev_s = 0;
	double lev_s_temp = 0;
	int cnt = 0;

	//Distance::levensteinの最大値
	lev_s = 1;

	for (; mean_it != meanings.end(); mean_it++) {
		kb1_pat = kb1.groundable_rules(*mean_it);
		kb2_pat = kb2.groundable_rules(*mean_it);

		kb1_pat_it = kb1_pat.begin();
		kb2_pat_it = kb2_pat.begin();

		if (kb1_pat.size() == 0 && kb2_pat.size() != 0) {

			continue;
		} else if (kb1_pat.size() != 0 && kb2_pat.size() == 0) {

			continue;
		}

		for (; kb1_pat_it != kb1_pat.end(); kb1_pat_it++) {
			for (kb2_pat_it = kb2_pat.begin(); kb2_pat_it != kb2_pat.end();
					kb2_pat_it++) {
				lev_s_temp = Distance::levenstein((*kb1_pat_it).external,
						(*kb2_pat_it).external);
				if (lev_s_temp < lev_s)
					lev_s = lev_s_temp;
			}
		}
	}

	return lev_s;
}

double calculate_word_distance(std::vector<Element>& words, KnowledgeBase& kb1,
		KnowledgeBase& kb2) {
	std::vector<Rule> kb1_wd_box, kb2_wd_box;
	std::vector<Rule>::iterator kb1_wd_it, kb2_wd_it;
	std::vector<Element>::iterator word_it;
	std::pair<std::multimap<Element, Rule>::iterator,
			std::multimap<Element, Rule>::iterator> kb1_range, kb2_range;

	double lev_w = 0;
	int cnt = 0;

	word_it = words.begin();
	for (; word_it != words.end(); word_it++) {
		kb1_range = kb1.normal_word_dic.equal_range(*word_it);
		kb2_range = kb2.normal_word_dic.equal_range(*word_it);
		for (; kb1_range.first != kb1_range.second; kb1_range.first++) {
			for (kb2.normal_word_dic.equal_range(*word_it);
					kb2_range.first != kb2_range.second; kb2_range.first++) {
				lev_w += Distance::levenstein(
						(*(kb1_range.first)).second.external,
						(*(kb2_range.first)).second.external);
				cnt++;
			}
		}
	}

	if (cnt != 0)
		return lev_w / (double) cnt;
	else
		return lev_w;
}

void normalize(boost::numeric::ublas::matrix<double>& m, NetWorld& world) {
	std::vector<int> nei;
	boost::numeric::ublas::matrix<double>::iterator1 mr_it;
	boost::numeric::ublas::matrix<double>::iterator2 mc_it;
	for (int index1 = 0; index1 < m.size1(); index1++) {
		for (int index2 = 0; index2 < m.size2(); index2++) {
			nei = world.get_neighbors(index2);
			if (nei.size() != 0)
				m(index1, index2) = m(index1, index2) / (double) nei.size();
		}
	}
}

void tf_result_output(LELAParameters param, std::string file,
		std::vector<boost::numeric::ublas::matrix<double> > res) {
	{
		//besic analyze
		//boost::filesystem::path basic(file.c_str());
		//boost::filesystem::ofstream ofs(param.BASE_PATH / basic, std::ios::out | std::ios::app);
		std::string path;
		path = param.BASE_PATH + file;
		std::ofstream ofs(path.c_str());

		ofs << "#RESULT" << std::endl;

		//unit
		ofs << "BASIC=" << res[0] << std::endl;

		//distance
		ofs << "SDISTM=" << res[1] << std::endl;
		ofs << "WDIST=" << res[2] << std::endl;
		ofs << "CSDISTM=" << res[3] << std::endl;
		ofs << "CWDIST=" << res[4] << std::endl;
		ofs.close();
	}
}

void analyze_and_output(LELAParameters& param, std::vector<Rule> meaning_space,
		std::vector<Element> individuals, NetWorld& world1, NetWorld& world2) {
	std::vector<NetWorld>::iterator n_it;
	std::vector<boost::numeric::ublas::matrix<double> > res;
	std::string index_str, file, file_postfix;
	int index;

	index = world1.get_generation();
	index_str = boost::lexical_cast<std::string>(index);
	file = param.FILE_PREFIX + "_" + param.DATE_STR + "_" + index_str + ".rst";

	res = analyze(meaning_space, individuals, world1, world2);

	if (false) {
		boost::thread th(boost::bind(tf_result_output, param, file, res));
		th.detach();
	} else {
		//besic analyze
		std::ofstream ofs((param.BASE_PATH + file).c_str(),
				std::ios::app | std::ios::out);

		ofs << "#RESULT" << std::endl;

		//unit
		ofs << "BASIC=" << res[0] << std::endl;

		//distance
		ofs << "SDISTM=" << res[1] << std::endl;
		ofs << "WDIST=" << res[2] << std::endl;
		ofs << "CSDISTM=" << res[3] << std::endl;
		ofs << "CWDIST=" << res[4] << std::endl;
	}
}

//method written by sudo
double calculate_sudo_distance(std::vector<Rule>& meanings,
		KnowledgeBase& kb1, KnowledgeBase& kb2) {
    
	std::vector<Rule>::iterator mean_it, kb1_pat_it, kb2_pat_it, kb1_all_it, kb2_all_it, target_it;
	std::vector<Rule> kb1_pat, kb2_pat, kb1_all, kb2_all, target_rules, temp;
        double ham, min_ham=1000, lev, min_lev=2, lev_sum=0;
        
        mean_it=meanings.begin();
        
        for(;mean_it!=meanings.end();mean_it++){
            kb1_pat = kb1.grounded_rules(*mean_it);
	    kb2_pat = kb2.grounded_rules(*mean_it);
            kb1_pat_it=kb1_pat.begin();
            for(;kb1_pat_it!=kb1_pat.end();kb1_pat_it++){
                kb1_all.push_back(*kb1_pat_it);
            }
            kb2_pat_it=kb2_pat.begin();
            for(;kb2_pat_it!=kb2_pat.end();kb2_pat_it++){
                kb2_all.push_back(*kb2_pat_it);
            }
        }
        kb1_all_it=kb1_all.begin();
        for(;kb1_all_it!=kb1_all.end();kb1_all_it++){
            min_ham=1000;
            kb2_all_it=kb2_all.begin();
            for(;kb2_all_it!=kb2_all.end();kb2_all_it++){
                ham=Distance::hamming((*kb1_all_it).internal, (*kb2_all_it).internal);
                if(ham<min_ham){
                    min_ham=ham;
//                    std::cout << target_rules.size() << std::endl;
                    target_rules=temp;
//                    std::cout << target_rules.size() << std::endl;
                    target_rules.push_back(*kb2_all_it);
                }else if(ham==min_ham){
                    target_rules.push_back(*kb2_all_it);
                }
            }
            min_lev=2;
            target_it=target_rules.begin();
            for(;target_it!=target_rules.end();target_it++){
//                std::cout << (*target_it).to_s() << std::endl;
                lev=Distance::levenstein((*kb1_all_it).external,(*target_it).external);
                if(lev<min_lev){
                    min_lev=lev;
                }
            }
//            std::cout << min_lev << std::endl;
            lev_sum+=min_lev;
        }
        
        return lev_sum / (static_cast<double>(kb1_all.size()));

	//std::cerr << "In calculate" << std::endl;
 
	
//	mean_it = meanings.begin();
//	double lev_s = 0;
//	double local_lev = 0;
//	double local_lev_sum = 0;
//
//	for (int m = 0; m < meanings.size(); m++) {
//		kb1_pat = kb1.grounded_rules(meanings[m]);
//		kb2_pat = kb2.grounded_rules(meanings[m]);
//                
//
//		local_lev = 1;
//		lev_s = 1;
//
//		if (kb1_pat.size() == 0 || kb2_pat.size() == 0) {
//			local_lev = 1;
//		} else {
//			for (int p1 = 0; p1 < kb1_pat.size(); p1++) {
//				for (int p2 = 0; p2 < kb2_pat.size(); p2++) {
//					lev_s = Distance::levenstein(kb1_pat[p1].external, kb2_pat[p2].external);
//					//lev_s = Distance::onp_lv(kb1_pat[p1].external, kb2_pat[p2].external, local_lev);
//					if (lev_s < local_lev)
//						local_lev = lev_s;
//				}
//			}
//		}
//
//		local_lev_sum += local_lev;
//	}
//
//	//std::cerr << "Out calculate" << std::endl;
//	return local_lev_sum / (static_cast<double>(meanings.size()));
}



/*********************************************
 * MAIN
 *********************************************/

int main(int argc, char* argv[]) {
//	typedef std::vector<KirbyAgent> Generation;

	std::cout << "hoge" << std::endl;

	/*
	 * ほぼ静的クラスだが、シリアライズのため（Dictionary）と、
	 * デストラクタ実行のため（LogBox）にインスタンスを作成
	 * */
	Dictionary dic;
	LogBox log; //for destructor

	LELAParameters param;
	boost::progress_display *show_progress = 0;

	std::vector<NetWorld> all_generations;
	std::vector<Rule> meaning_space;
	std::vector<Element> individuals;
	NetWorld parent_gen, child_gen;

	int utterance_counter = 0;

	int use_meaning_index = 0;
	int generation_counter = 0;
	int Base_Counter = 0;
	Rule utter, use_meaning;

	std::vector<std::vector<int> > result;
	bool online_analyze = false;

	/**************************************************
	 *
	 * OPTION PROCESSING
	 *
	 **************************************************/
	boost::program_options::options_description opt("option");
	opt.add_options()("help,h", "Description")
	//experiment parameters
	/*ランダムシード*/
	("random-seed,r", boost::program_options::value<uint32_t>(),
			"Random seed (101010)")

	/*実験世代数*/
	("generations,g", boost::program_options::value<int>(),
			"Max generation number (100)")
	/*発話回数*/
	("utterances,u", boost::program_options::value<double>(),
			"Uttering ratio for meaning space (0.7/[0~1])")

	/*接触確率*/
	("contact,c", boost::program_options::value<double>(),
			"Set contact probability from neighbors(0.0/[0~1])")

	/*同世代確率*/
	("neighbor,n", boost::program_options::value<double>(),
			"Set contact probability from neighbors in the same generation(0.0/[0~1])")

	/*エージェント数*/
	("agents", boost::program_options::value<int>(),
			"Set a number of agents in a generation (1)")

	/*グラフファイル指定*/
	("igraph", boost::program_options::value<std::string>(), "Set a graph file")

	/*グラフファイル指定*/
	("linked-matrix", boost::program_options::value<std::string>(),
			"Set a graph file on linked matrix")

	/*規則削除文字列長*/
	("delete-rule-length", boost::program_options::value<int>(),
			"delete rule length (if it not used, then it undefined)")
	/*thread number*/
	("threads", boost::program_options::value<int>(), "use thread (1)")

	/*文字列表記変換*/
	("convert", boost::program_options::value<std::vector<std::string> >(),
			"Convert the state file into human friendly text file (InFile OutFile)")

	/*ロギング*/
	("logging,l", "Logging On")

	/*最短発話*/
	("minimum-utter", "utter minimum length")

	/*インデックス使用発話*/
	("index", "construct index of sentence, and use index")

	/*分析*/
	("analyze,a", "Analyze each agent for expression and a number of rules")

	/*生成文字列最大長*/
	("word-length", boost::program_options::value<int>(),
			"Max length of created word (3)")

	/*世代における解析間隔*/
	("interspace-analysis", boost::program_options::value<int>(),
			"set analysis interspace for the number of generations")

	/*世代におけるロギング間隔*/
	("interspace-logging", boost::program_options::value<int>(),
			"set logging interspace for the number of generations")

	/*辞書ファイル*/
	("dictionary,d", boost::program_options::value<std::string>(),
			"Dictionary file name for meaning space(\"./data.dic\")")

	/*生成規則再利用*/
	("keep-random-rule",
			"Keep created rules with a random word into parent knowledge-base")
	/*規則削除使用*/
	("delete-redundant-rules", "Delete redundant rules")
	/*補完発話*/
	("invention-utterance", "Uttering with invention")
	/*非重複発話*/
	("unique-utterance", "Do not use the same meaning on utterance process")

	/*最大外部言語入力文字列長*/
	("max-listening-length", boost::program_options::value<int>(),
			"Max length of listening external letters. (0) The value of 0 means it is unlimited")
	/*親エージェントとの接触テストを1回にする*/
	("once-parent-test",
			"At default, firstly, a dice is thrown so that a child agent decides which contacts the parent or neighborhood parent agent. Secondly, the dice is thrown again to select for the parent of the agent and the parents of neighborhoods. But, in the case that you use the option \"once-parent-test\", then the times of throwing the dice is once. Namely, if the value of dice is less than contact probability, then a neighborhood parent agent is selected to contact the child agent with the value. If this is not the case, the parent agent of the child agent contacts him/her.")

	/*FILE PREFIX*/
	("prefix", boost::program_options::value<std::string>(),
			"Set file prefix (\"LEKA\")")

	/*BASE PATH*/
	("path", boost::program_options::value<std::string>(),
			"Set folder for output files (\"./\")")
	/*再開*/
	("resume", boost::program_options::value<std::vector<std::string> >(),
			"Resume an experiment from saved file name. Then used path/resume_file")
	/*保存*/
	("last-save", "Save the last state of the experiment into the file")
	/*全保存*/
	("all-save",
			"Save the all state of the experiment into the file with the prefix")
	/*保存形式*/
	("format", boost::program_options::value<std::string>(),
			"Set saved format (bin/[xml])")

	/*プログレスバー*/
	("progress,p", "Show progress bar");



	//process options
	boost::program_options::variables_map vm;
	try {
		boost::program_options::store(
				boost::program_options::parse_command_line(argc, argv, opt),
				vm);
		boost::program_options::notify(vm);
	} catch (boost::program_options::invalid_syntax ee) {
		std::cerr << "invalid syntax:" << ee.tokens() << std::endl;
		std::cerr << std::endl << "SEE HELP" << std::endl << opt << std::endl;
		return 1;
	} catch (boost::program_options::unknown_option ee) {
		std::cerr << "unknown_option:" << ee.what() << std::endl;
		std::cerr << std::endl << "SEE HELP" << std::endl << opt << std::endl;
		return 1;
	} catch (boost::program_options::ambiguous_option ee) {
		std::cerr << "ambiguous_option:" << ee.get_option_name() << std::endl;
		std::cerr << std::endl << "SEE HELP" << std::endl << opt << std::endl;
		return 1;
	} catch (boost::program_options::multiple_values ee) {
		std::cerr << "multiple_values:" << ee.get_option_name() << std::endl;
		std::cerr << std::endl << "SEE HELP" << std::endl << opt << std::endl;
		return 1;
	} catch (boost::program_options::multiple_occurrences ee) {
		std::cerr << "multiple_occurrences:" << ee.get_option_name()
				<< std::endl;
		std::cerr << std::endl << "SEE HELP" << std::endl << opt << std::endl;
		return 1;
	} catch (boost::program_options::validation_error ee) {
		std::cerr << "validation_error:" << ee.get_option_name() << std::endl;
		std::cerr << std::endl << "SEE HELP" << std::endl << opt << std::endl;
		return 1;
	} catch (boost::program_options::too_many_positional_options_error ee) {
		std::cerr << "too_many_positional_options_error:" << ee.what()
				<< std::endl;
		std::cerr << std::endl << "SEE HELP" << std::endl << opt << std::endl;
		return 1;
	} catch (boost::program_options::invalid_command_line_style ee) {
		std::cerr << "invalid_command_line_style:" << ee.what() << std::endl;
		std::cerr << std::endl << "SEE HELP" << std::endl << opt << std::endl;
		return 1;
	} catch (boost::program_options::reading_file ee) {
		std::cerr << "reading_file:" << ee.what() << std::endl;
		std::cerr << std::endl << "SEE HELP" << std::endl << opt << std::endl;
		return 1;
	} catch (boost::program_options::required_option ee) {
		std::cerr << "required_option:" << ee.get_option_name() << std::endl;
		std::cerr << std::endl << "SEE HELP" << std::endl << opt << std::endl;
		return 1;
	}

	//check options
	if (vm.count("help")) {
		std::cerr << opt << std::endl;
		return 1;
	}

	//set parameters
	try {
		param.set_option(vm);
	} catch (std::string e) {
		std::cerr << "option:" << e << std::endl;
		exit(0);
	}

	/************************************************
	 *
	 * Option の処理
	 *
	 ************************************************/

	/*
	 * ファイルコンバート
	 */
	//基本的にResumeと同じ
	if (param.CONVERT_FLAG) {
		NetWorld conv_world;
		try {
			//boost::filesystem::ifstream ifs(param.CONVERTING_PATH);
			std::ifstream ifs(param.CONVERTING_PATH.c_str());

			switch (param.SAVE_FORMAT) {
			case Parameters::BIN: {
				boost::archive::binary_iarchive ia(ifs);
				resume_agents(ia, param, MT19937::icount, MT19937::rcount, dic,
						meaning_space, conv_world);
			}
				break;

			case Parameters::XML: {
				boost::archive::xml_iarchive ia(ifs);
				resume_agents(ia, param, MT19937::icount, MT19937::rcount, dic,
						meaning_space, conv_world);
			}
				break;

			default:
				return 0;
			}

		} catch (...) {
			std::cerr << "State file Error" << std::endl;
			return 0;
		}

		//boost::filesystem::ofstream ofs(param.CONVERTED_PATH);
		std::ofstream ofs(param.CONVERTED_PATH.c_str());
		ofs << conv_world.to_s();
		ofs.close();

		return 0;
	}

	/*
	 * resume処理
	 */
	if (param.RESUME) {
		try {
			//boost::filesystem::ifstream ifs(param.BASE_PATH / param.RESUME_FILE);
			std::ifstream ifs((param.BASE_PATH + param.RESUME_FILE).c_str());

			switch (param.SAVE_FORMAT) {
			case Parameters::BIN: {
				boost::archive::binary_iarchive ia(ifs);
				resume_agents(ia, param, MT19937::icount, MT19937::rcount, dic,
						meaning_space, parent_gen);
			}
				break;

			case Parameters::XML: {
				boost::archive::xml_iarchive ia(ifs);
				resume_agents(ia, param, MT19937::icount, MT19937::rcount, dic,
						meaning_space, parent_gen);
			}
				break;

			default:
				return 0;
			}

			child_gen = parent_gen.make_generation();
			MT19937::set_seed(param.RANDOM_SEED);
			MT19937::waste();

		} catch (...) {
			std::cerr << "State file Error" << std::endl;
			return 0;
		}
	} else {

		/*
		 * initialize
		 */

		//RANDOM SEED
		MT19937::set_seed(param.RANDOM_SEED);

		//Knowledge Base Setting
		KnowledgeBase::set_control(
				KnowledgeBase::ANTECEDE_COMPOSITION | param.CONTROLS);

		//Agent Setting
		if (param.DEL_LONG_RULE) {
			KirbyAgent::DEL_LONG_RULE = true;
			KirbyAgent::DEL_LONG_RULE_LENGTH = param.DEL_LONG_RULE_LENGTH;
		}

		if (param.EX_LIMIT != 0) {
			KirbyAgent::SHORT_MEM_SIZE = param.EX_LIMIT;
		}

		if (param.PARENT_ONCE) {
			NetWorld::PARENT_ONCE = true;
		}

		KirbyAgent::UTTER_MINIMUM = param.UTTER_MINIMUM;
		KirbyAgent::INDEXER_FLAG = param.INDEX;

		//std::cerr << "hoge" << std::endl;

		//Dictionary Setting
		dic.load(param.DICTIONARY_FILE);

		//****NetWorld Setting
		//Agent数設定
		NetWorld::agent_num = param.AGENT_NUM;

		//0世代の構築
		parent_gen.build_world();

		//意味空間の構築
		construct_meanings(meaning_space);

		//対象集合の構築
		construct_individuals(individuals, dic);

	}

	//ログの有無を設定
	if (param.LOGGING)
		NetWorld::logging_on();

	//構築された現世代から次世代の構築
	child_gen = parent_gen.make_generation();
	Base_Counter = parent_gen.get_generation();

	//使用意味空間の出力
	if (param.LOGGING) {
		//LogBox::set_filepath(param.LOG_FILE);
		LogBox::set_filepath(param.LOG_FILE);
		std::vector<Rule>::iterator mean_it;

		LogBox::push_log("USED RANDOM SEED");
		//LogBox::push_log(boost::lexical_cast<std:string>(MT19937::_irand.engine().seed()));

		mean_it = meaning_space.begin();
		LogBox::push_log("USEING MEANINGS");
		for (; mean_it != meaning_space.end(); mean_it++) {
			LogBox::push_log((*mean_it).to_s());
		}
		LogBox::push_log("\n");
	}

	//graphデータの上書き
	//ちなみに次数以外も変更したい場合は、RとかでEdgelist fileを作るべし
	//とりあえずUblasにも対応
	if (param.LINKED_MATRIX) {
		//boost::filesystem::ifstream ifs(param.LINKED_MATRIX_PATH);
		std::ifstream ifs(param.LINKED_MATRIX_PATH.c_str());
		ifs >> NetWorld::connected_matrix;
		if (NetWorld::connected_matrix.size1() != NetWorld::agent_num
				|| NetWorld::connected_matrix.size2() != NetWorld::agent_num) {
			std::cerr << "No matched graph data" << std::endl;
			throw "No matched graph data";
		}
	} else if (param.IGRAPH_FLAG) {
		std::ifstream ifs(param.igraph_file_name.c_str());
		if (!ifs.good()) {
			std::cerr << "Not found graph file" << std::endl;
			throw "file not found";
		}

		FILE *fd;
		igraph_t igraph;
		fd = fopen(param.igraph_file_name.c_str(), "r");
		igraph_read_graph_edgelist(&igraph, fd, 0, 0);
		fclose(fd);

		//convert graph
		int vertex_num, to, vector_end;
		igraph_vector_t neighbors;

		//initialize
		vertex_num = (int) igraph_vcount(&igraph);

		if (vertex_num != NetWorld::agent_num) {
			std::cerr << "No matched graph data" << std::endl;
			throw "No matched graph data";
		}

		NetWorld::connected_matrix.resize(vertex_num, vertex_num);
		NetWorld::connected_matrix.clear();

		for (int from = 0; from < vertex_num; from++) {
			igraph_vector_init(&neighbors, 0);
			igraph_neighbors(&igraph, &neighbors, (igraph_integer_t) from,
					IGRAPH_ALL/*無視される*/);
			igraph_vector_sort(&neighbors);
			vector_end = igraph_vector_size(&neighbors);
			for (int index = 0; index < vector_end; index++) {
				to = VECTOR(neighbors)[index];
				//std::cout << "From:" << from << " " << "to:" << to << std::endl;
				NetWorld::connected_matrix(from, to) = 1;
			}
		}
	}

	//接触確率の設定
	NetWorld::contact_probabirity = param.CONTACT_PROBABIRITY;
	NetWorld::neighbor_probabirity = param.NEIGHBOR_PROBABIRITY;

	//発話回数の設定
	param.UTTERANCES = (int) round(param.PER_UTTERANCES * meaning_space.size());
	if (param.LOGGING) {
		LogBox::push_log(
				"UTTRANCE TIMES = "
						+ boost::lexical_cast<std::string>(param.UTTERANCES));
	}

	// Progress bar表示設定
	if (param.PROGRESS) {
		show_progress = new boost::progress_display(
				param.UTTERANCES * param.MAX_GENERATIONS + 1);
	}

	//Log file Path
	LogBox::set_filepath(param.BASE_PATH + param.LOG_FILE);

	/*************************************************
	 *
	 * メイン処理
	 *
	 *************************************************/

	//Parameter Output
	{
		//boost::filesystem::path param_file("Parameters_" + param.DATE_STR + ".prm");
		//boost::filesystem::ofstream ofs(param.BASE_PATH / param_file);
		std::string param_file("Parameters_" + param.DATE_STR + ".prm");
		std::ofstream ofs((param.BASE_PATH + param_file).c_str());
		ofs << param.to_s() << std::endl;
	}

	if (NetWorld::agent_num * param.MAX_GENERATIONS > 1000)
		online_analyze = true;

	if (param.INTER_ANALYSIS) {
		NetWorld::logging_off();
		param.LOGGING = false;
	}

	try {
		//main loop
		while (generation_counter < param.MAX_GENERATIONS) {
			std::vector<Rule> meanings_copy;
			meanings_copy = meaning_space;

			utterance_counter = 0;
#ifdef DEBUG
			std::cerr << "Start Generation:" << generation_counter << std::endl;
#endif
			if (generation_counter + 1 == param.MAX_GENERATIONS
					&& param.INTER_LOG) {
				NetWorld::logging_on();
				param.LOGGING = true;
			} else if (param.INTER_LOG) {
				if (generation_counter % param.SPACE_LOG == 0) {
					NetWorld::logging_on();
					param.LOGGING = true;
				} else {
					NetWorld::logging_off();
					param.LOGGING = false;
				}
			}

			if (param.LOGGING) {
				LogBox::push_log(
						"\nGENERATION: "
								+ boost::lexical_cast<std::string>(
										generation_counter + Base_Counter));
				LogBox::push_log("BEFORE TALKING");
				LogBox::push_log("\nPARRENT KNOWLEDGE");
				LogBox::push_log(parent_gen.to_s());
				LogBox::push_log("\n-->>EDUCATION");
			}
#ifdef DEBUG
			std::cerr << "Say & learn" << std::endl;
#endif
			while (utterance_counter <= param.UTTERANCES
					&& meanings_copy.size() != 0) {
#ifdef DEBUG
				std::cerr << "Say";
#endif
				parent_gen.educate(meanings_copy, child_gen);
#ifdef DEBUG
				std::cerr << " -> learn" << std::endl;
#endif
				child_gen.learn();

				if (param.PROGRESS)
					++(*show_progress);

				utterance_counter++;
			}

			//事後処理
			if (param.LOGGING) {
				LogBox::push_log("\n<<--EDUCATION");
				LogBox::push_log(
						"\nGENERATION :"
								+ boost::lexical_cast<std::string>(
										generation_counter + Base_Counter));
				LogBox::push_log("AFTER TALKING");
				LogBox::push_log("\nCHILD KNOWLEDGE");
				LogBox::push_log(child_gen.to_s());
			}

#ifdef DEBUG
			std::cerr << "Save State" << std::endl;
#endif

			if (param.SAVE_ALL_STATE) {
				std::string index_str;
				index_str = boost::lexical_cast<std::string>(
						generation_counter + Base_Counter);
				std::string stf = param.FILE_PREFIX + "_Gen_" + index_str
						+ ".st";
				std::ofstream ofs((param.BASE_PATH + stf).c_str());

				switch (param.SAVE_FORMAT) {
				case Parameters::BIN: {
					boost::archive::binary_oarchive oa(ofs);
					save_agents(oa, param, MT19937::icount, MT19937::rcount,
							dic, meaning_space, parent_gen);
				}
					break;

				case Parameters::XML: {
					boost::archive::xml_oarchive oa(ofs);
					save_agents(oa, param, MT19937::icount, MT19937::rcount,
							dic, meaning_space, parent_gen);
				}
					break;

				default:
					std::cerr << "UNKNOWN FORMAT" << std::endl;
					return 0;
				}
			}
#ifdef DEBUG
			std::cerr << "Analyze" << std::endl;
#endif
			if (generation_counter + 1 == param.MAX_GENERATIONS
					&& param.INTER_ANALYSIS) {
				analyze_and_output(param, meaning_space, individuals,
						parent_gen, parent_gen);
				analyze_and_output(param, meaning_space, individuals,
						parent_gen, child_gen);
			} else if (param.INTER_ANALYSIS) {
				if (generation_counter % param.SPACE_ANALYSIS == 0) {
					analyze_and_output(param, meaning_space, individuals,
							parent_gen, parent_gen);
					analyze_and_output(param, meaning_space, individuals,
							parent_gen, child_gen);
				}
			} else if (param.ANALYZE) {
				analyze_and_output(param, meaning_space, individuals,
						parent_gen, parent_gen);
				analyze_and_output(param, meaning_space, individuals,
						parent_gen, child_gen);
			}

			if (param.LOGGING) {
				LogBox::refresh_log();
			}

			//世代交代
#ifdef DEBUG
			std::cerr << "Change Generation" << std::endl;
#endif
			parent_gen = child_gen.grow_up(meaning_space);
			child_gen = parent_gen.make_generation();
			generation_counter++;
		} //while
	} catch (...) {
		if (param.LOGGING) {
			LogBox::refresh_log();
		}
		std::cerr << "throwing Exception" << std::endl;
		exit(0);
	}

	/***********************************************
	 *
	 * 状態保存
	 *
	 ***********************************************/
	//saving proc
	if (param.SAVE_LAST_STATE) {
		//boost::filesystem::ofstream ofs(param.BASE_PATH / param.SAVE_FILE);

		std::ofstream ofs((param.BASE_PATH + param.SAVE_FILE).c_str());

		switch (param.SAVE_FORMAT) {
		case Parameters::BIN: {
			boost::archive::binary_oarchive oa(ofs);
			save_agents(oa, param, MT19937::icount, MT19937::rcount, dic,
					meaning_space, parent_gen);
		}
			break;

		case Parameters::XML: {
			boost::archive::xml_oarchive oa(ofs);
			save_agents(oa, param, MT19937::icount, MT19937::rcount, dic,
					meaning_space, parent_gen);
		}
			break;

		default:
			std::cerr << "UNKNOWN FORMAT" << std::endl;
			return 0;
		}

	}

	//Connect Matrix Output
	{
		//boost::filesystem::path connect_matrix_file("linked_matrix_" + param.DATE_STR + ".mx");
		//boost::filesystem::ofstream ofs(param.BASE_PATH / connect_matrix_file);
		std::string connect_matrix_file(
				"linked_matrix_" + param.DATE_STR + ".mx");
		std::ofstream ofs((param.BASE_PATH + connect_matrix_file).c_str());
		ofs << NetWorld::connected_matrix << std::endl;
	}

	//delete
	delete show_progress;
	if (param.LOGGING)
		log.refresh_log();
	return 0;
}
