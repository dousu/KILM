/* 
 * File:   MSILM_main.cpp
 * Author: hiroki
 *
 * Created on October 28, 2013, 6:12 PM
 */

#include "MSILM_main.h"

template<typename _OFS>
void save_agent(
        _OFS& ofs,
        MSILMParameters& param,
        unsigned long long int& icount,
        unsigned long long int& rcount,
        Dictionary& dic,
        std::vector<Rule>& meanings,
        int& Generation_Counter,
        MSILMAgent& agent
        ) {
    ofs << boost::serialization::make_nvp("UUF", param.UNIQUE_UTTERANCE);
    ofs << boost::serialization::make_nvp("RS", param.RANDOM_SEED);
    ofs << boost::serialization::make_nvp("IRC", icount);
    ofs << boost::serialization::make_nvp("RRC", rcount);
    ofs << boost::serialization::make_nvp("GC", Generation_Counter);
    ofs << boost::serialization::make_nvp("MS", meanings);
    ofs << boost::serialization::make_nvp("DC", dic);
    ofs << boost::serialization::make_nvp("KA", agent);
}

template<typename _IFS>
void resume_agent(
        _IFS& ifs,
        MSILMParameters& param,
        unsigned long long int& icount,
        unsigned long long int& rcount,
        Dictionary& dic,
        std::vector<Rule>& meanings,
        int& Generation_Counter,
        MSILMAgent& agent
        ) {
    ifs >> boost::serialization::make_nvp("UUF", param.UNIQUE_UTTERANCE);
    ifs >> boost::serialization::make_nvp("RS", param.RANDOM_SEED);
    ifs >> boost::serialization::make_nvp("IRC", icount);
    ifs >> boost::serialization::make_nvp("RRC", rcount);
    ifs >> boost::serialization::make_nvp("GC", Generation_Counter);
    ifs >> boost::serialization::make_nvp("MS", meanings);
    ifs >> boost::serialization::make_nvp("DC", dic);
    ifs >> boost::serialization::make_nvp("KA", agent);
}

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

//std::vector<Rule> construct_meanings(
//		int VERB_INDEX_BEGIN,
//		int VERB_INDEX_END,
//		int NOUN_INDEX_BEGIN,
//		int NOUN_INDEX_END
//){
//	std::vector<Rule> meanings;
//
//	//construct means
//	for(int i = VERB_INDEX_BEGIN; i <= VERB_INDEX_END; i++){
//		Element verb;
//		verb.set_ind(i);
//
//		for(int j = NOUN_INDEX_BEGIN; j <= NOUN_INDEX_END; j++){
//			Element ind1;
//			ind1.set_ind(j);
//			for(int k = NOUN_INDEX_BEGIN; k <= NOUN_INDEX_END; k++){
//				if(j != k){
//					Element ind2;
//					std::vector<Element> internal,external;
//					Rule mean;
//
//					ind2.set_ind(k);
//
//					internal.push_back(verb);
//					internal.push_back(ind1);
//					internal.push_back(ind2);
//
//					mean.set_sentence(internal, external);
//					meanings.push_back(mean);
//				}
//			}
//		}
//	}
//
//	return meanings;
//}

void construct_individuals(std::vector<Element>& inds, Dictionary &dic) {
    Dictionary::DictionaryType::iterator dic_it;
    dic_it = dic.individual.begin();
    for (; dic_it != dic.individual.end(); dic_it++) {
        Element elem;
        elem.set_ind((*dic_it).first);
        inds.push_back(elem);
    }
}

std::vector<std::vector<double> > analyze(std::vector<Rule>& meanings, std::vector<Element>& individuals,
        MSILMAgent& agent1, MSILMAgent& agent2) {

    std::vector<double> unit_result, dist_result, nu_result;

    //        std::vector<double> effect_sent, effect_word;

    std::vector<std::vector<double> > result;

    //        int unit_analyze_item_num = 4, vertices_num = 0;

    //	//行列の初期化
    //	vertices_num = NetWorld::agent_num;
    //	effect_sent.resize(vertices_num, vertices_num);
    //	effect_word.resize(vertices_num, vertices_num);
    //	unit_result.resize(vertices_num, unit_analyze_item_num);
    //	inter_result_sent.resize(vertices_num, vertices_num);
    //	inter_result_word.resize(vertices_num, vertices_num);

    //	effect_sent.clear();
    //	effect_word.clear();
    unit_result.clear();
    dist_result.clear();
    nu_result.clear();

    //Agent単体の分析
    unit_analyze(unit_result, meanings, agent1);

    //Agent相互の分析
    calculate_language_distance(dist_result, nu_result, meanings,
            individuals, agent1, agent2);

    //今のところ三角行列になってるからそれを変えてる
    //inter_result_sent = boost::numeric::ublas::trans(inter_result_sent) + inter_result_sent;
    //inter_result_word = boost::numeric::ublas::trans(inter_result_word) + inter_result_word;

    //多分距離行列×接続行列で前世代との差分を使えばRecurrent Network見たいな重み評価に使える
    //が今は対角だけつかう（近傍変異）
    //	effect_sent = boost::numeric::ublas::prod(inter_result_sent,
    //			NetWorld::connected_matrix);
    //	effect_word = boost::numeric::ublas::prod(inter_result_word,
    //			NetWorld::connected_matrix);

    //normalize(effect_sent, world);
    //normalize(effect_word, world1);

    //リターンバケットに結果をつめて返す
    result.push_back(unit_result);
    result.push_back(dist_result);
    result.push_back(nu_result);
    //	result.push_back(effect_sent);
    //	result.push_back(effect_word);

    return result;
}

//std::vector<int> analyze(std::vector<Rule>& meanings, KirbyAgent& agent){
//	std::vector<int> result;
//	result.push_back(agent.generation_index);
//	result.push_back(expression(meanings,agent));
//	result.push_back(agent.kb.sentenceDB.size());
//	result.push_back(agent.kb.wordDB.size());
//	return result;
//}

void unit_analyze(std::vector<double>& result_matrix,
        std::vector<Rule>& meanings, MSILMAgent& agent) {
    int index = 0, max_index;
    int GEN = 0, EXP = 1, SRN = 2, WRN = 3;

    //	max_index = world.agents.size();
    //	for (index = 0; index < max_index; index++) {
    //Item1 :generation number
    result_matrix.push_back(agent.generation_index);

    //Item2 :expressiveness
    result_matrix.push_back(expression(meanings, agent));

    //Item3 :sentence rule number
    result_matrix.push_back(agent.kb.sentenceDB.size());

    //Item4 :Word rule number
    result_matrix.push_back(agent.kb.wordDB.size());
    //	}
}

double expression(std::vector<Rule>& meanings, MSILMAgent& agent) {
    std::vector<Rule>::iterator mean_it;
    int counter = 0;
    mean_it = meanings.begin();
    while (mean_it != meanings.end()) {
        if (agent.understand(*mean_it))
            counter++;
        mean_it++;
    }

    return (static_cast<double> (counter)) / (static_cast<double> (meanings.size()))*100.0;
}

void calculate_language_distance(
        std::vector<double>& lev_matrix,
        std::vector<double>& word_matrix,
        std::vector<Rule>& meanings, std::vector<Element>& words,
        MSILMAgent& agent1, MSILMAgent& agent2) {

    double word_length;

    //	int agent_MAX;
    //	int agent_index1 = 0;
    //	int agent_index2 = 0;
    //	agent_MAX = NetWorld::agent_num;

    //	for (agent_index1 = 0; agent_index1 < agent_MAX; agent_index1++) {
    //		for (agent_index2 = 0; agent_index2 < agent_MAX; agent_index2++) {

    //言語間距離を計算
    lev_matrix.push_back(calculate_sudo_distance(meanings, agent1.kb, agent2.kb, word_length));
    word_matrix.push_back(word_length);

    //一発話平均文字数を計算
    //calculate_average_word_length(meanings,agent1.kb,word_length);
    //lev_sent_matrix.push_back(word_length);

    //		}
    //	}

    //	for (agent_index1 = 0; agent_index1 < agent_MAX; agent_index1++) {
    //		for (agent_index2 = 0; agent_index2 < agent_MAX; agent_index2++) {
    //    word_matrix.push_back(// 0;
    //            calculate_word_distance(words,
    //            agent1.kb,
    //            agent2.kb));
    //		}
    //	}
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

void tf_result_output(MSILMParameters param, std::string file,
        std::vector<std::vector<double> > res) {
    {
        //besic analyze
        //boost::filesystem::path basic(file.c_str());
        //boost::filesystem::ofstream ofs(param.BASE_PATH / basic, std::ios::out | std::ios::app);
        std::string path;
        path = param.BASE_PATH + file;
        std::ofstream ofs(path.c_str());

        ofs << "#RESULT" << std::endl;

        //unit
        ofs << "BASIC=" << tr_vector_double_to_string(res[0]) << std::endl;

        //distance
        ofs << "DIST =" << tr_vector_double_to_string(res[1]) << std::endl;
        ofs << "NU   =" << tr_vector_double_to_string(res[2]) << std::endl;
        //		ofs << "CSDISTM=" << tr_vector_double_to_string(res[3]) << std::endl;
        //		ofs << "CWDIST=" << tr_vector_double_to_string(res[4]) << std::endl;
        ofs.close();
    }
}

void analyze_and_output(MSILMParameters& param, std::vector<Rule> meaning_space,
        std::vector<Element> individuals, MSILMAgent& agent1, MSILMAgent& agent2, int index) {
    //	std::vector<NetWorld>::iterator n_it;
    std::vector<std::vector<double> > res;
    std::string index_str, file, file_postfix;
    //int index;

    //index = agent1.generation_index;
    index_str = boost::lexical_cast<std::string>(index);
    file = param.FILE_PREFIX + "_" + param.DATE_STR + "_" + boost::lexical_cast<std::string>(param.RANDOM_SEED) + "_" + index_str + ".rst";

    res = analyze(meaning_space, individuals, agent1, agent2);

    if (false) {
        boost::thread th(boost::bind(tf_result_output, param, file, res));
        th.detach();
    } else {
        //basic analyze
        std::ofstream ofs((param.BASE_PATH + file).c_str(),
                std::ios::app | std::ios::out);

        ofs << "#RESULT" << std::endl;

        //unit
        ofs << "BASIC=" << tr_vector_double_to_string(res[0]) << std::endl;

        //distance
        ofs << "DIST =" << tr_vector_double_to_string(res[1]) << std::endl;
        ofs << "NU   =" << tr_vector_double_to_string(res[2]) << std::endl;
        //		ofs << "CSDISTM=" << tr_vector_double_to_string(res[3]) << std::endl;
        //		ofs << "CWDIST=" << tr_vector_double_to_string(res[4]) << std::endl;
    }
}

void accuracy_meaning_output(MSILMParameters param, std::string file, std::vector<std::vector<int> > data) {
    {
        //besic analyze
        //boost::filesystem::path basic(file.c_str());
        //boost::filesystem::ofstream ofs(param.BASE_PATH / basic, std::ios::out | std::ios::app);
        std::string path;
        path = param.BASE_PATH + file;
        std::ofstream ofs(path.c_str());

        std::vector<std::vector<int> >::iterator data_it;
        std::vector<int>::iterator inside_it;
        data_it = data.begin();
        for (; data_it != data.end(); data_it++) {
            inside_it = (*data_it).begin();
            if (inside_it != (*data_it).end()) {
                ofs << *inside_it;
                inside_it++;
                for (; inside_it != (*data_it).end(); inside_it++) {
                    ofs << " " << *inside_it;
                }
            }
            ofs << std::endl;
        }
        ofs.close();
    }
}

//method written by sudo

double calculate_sudo_distance(std::vector<Rule>& meanings,
        KnowledgeBase& kb1, KnowledgeBase& kb2, double& word_length) {

    //from kb1 to kb2

    std::vector<Rule>::iterator mean_it, kb1_pat_it, kb2_pat_it, kb1_all_it, kb2_all_it, target_it;
    std::vector<Rule> kb1_pat, kb2_pat, kb1_all, kb2_all, target_rules, temp;
    double ham, min_ham = 1000, lev, min_lev = 2, lev_sum = 0, work_lev = 2, length_cnt = 0;
    int ham_zero_cnt = 0;

    word_length = 0;

    mean_it = meanings.begin();

    for (; mean_it != meanings.end(); mean_it++) {
        kb1_pat = kb1.grounded_rules(*mean_it);
        kb2_pat = kb2.grounded_rules(*mean_it);
        if (kb1_pat.size() != 0) {
            kb1_pat_it = kb1_pat.begin();
            for (; kb1_pat_it != kb1_pat.end(); kb1_pat_it++) {
                kb1_all.push_back(*kb1_pat_it);
                word_length += ((*kb1_pat_it).external.size());
                length_cnt++;
            }
        } else {
            kb1_all.push_back(kb1.pseudofabricate(*mean_it));
        }
        if (kb2_pat.size() != 0) {
            kb2_pat_it = kb2_pat.begin();
            for (; kb2_pat_it != kb2_pat.end(); kb2_pat_it++) {
                kb2_all.push_back(*kb2_pat_it);
            }
        } else {
            kb2_all.push_back(kb2.pseudofabricate(*mean_it));
        }
    }
    word_length = (word_length / length_cnt);
    kb1_all_it = kb1_all.begin();
    for (; kb1_all_it != kb1_all.end(); kb1_all_it++) {
        min_ham = 1000;
        kb2_all_it = kb2_all.begin();
        for (; kb2_all_it != kb2_all.end(); kb2_all_it++) {
            ham = Distance::hamming((*kb1_all_it).internal, (*kb2_all_it).internal);
            if (ham < min_ham) {
                min_ham = ham;
                //                    std::cout << target_rules.size() << std::endl;
                target_rules = temp;
                //                    std::cout << target_rules.size() << std::endl;
                target_rules.push_back(*kb2_all_it);
            } else if (ham == min_ham) {
                target_rules.push_back(*kb2_all_it);
            }
        }
        if ((min_ham == 0) || true) {
            if (false) {
                work_lev = 0;
                target_it = target_rules.begin();
                for (; target_it != target_rules.end(); target_it++) {
                    //                std::cout << (*target_it).to_s() << std::endl;
                    lev = Distance::levenstein((*kb1_all_it).external, (*target_it).external);
                    work_lev += lev;
                }
                min_lev = work_lev / (static_cast<double> (target_rules.size()));
            } else {
                min_lev = 1;
                target_it = target_rules.begin();
                for (; target_it != target_rules.end(); target_it++) {
                    //                std::cout << (*target_it).to_s() << std::endl;
                    lev = Distance::levenstein((*kb1_all_it).external, (*target_it).external);
                    if (lev < min_lev) {
                        min_lev = lev;
                    }
                }
            }
            //            std::cout << min_lev << std::endl;
            lev_sum += min_lev;
        } else {
            ham_zero_cnt += 1;
        }
    }

    return lev_sum / (static_cast<double> (kb1_all.size() - ham_zero_cnt));
}

void
calculate_average_word_length(std::vector<Rule>& meanings, KnowledgeBase& kb1, double& word_length) {
    std::vector<Rule>::iterator mean_it, kb1_pat_it;
    std::vector<Rule> kb1_pat;
    double length_cnt = 0;

    word_length = 0;

    mean_it = meanings.begin();

    for (; mean_it != meanings.end(); mean_it++) {
        kb1_pat = kb1.grounded_rules(*mean_it);
        if (kb1_pat.size() != 0) {
            kb1_pat_it = kb1_pat.begin();
            for (; kb1_pat_it != kb1_pat.end(); kb1_pat_it++) {
                word_length += ((*kb1_pat_it).external.size());
                length_cnt++;
            }
        }
    }
    word_length = (word_length / length_cnt);

}

std::string
tr_vector_double_to_string(std::vector<double> vector) {
    std::string res = "(";
    std::stringstream ss;
    std::string work;
    std::vector<double>::iterator double_it = vector.begin();
    ss.clear();
    ss << (*double_it);
    ss >> work;
    res = res + work;
    double_it++;
    for (; double_it != vector.end(); double_it++) {
        ss.clear();
        ss << (*double_it);
        ss >> work;
        res = res + "," + work;
    }
    return (res + ")");
}

bool
include_index_check(std::vector<int> meaning_indexs, int sample_index) {
    bool flag = false;
    std::vector<int>::iterator meaning_indexs_it = meaning_indexs.begin();

    for (; meaning_indexs_it != meaning_indexs.end(); meaning_indexs_it++) {
        flag = (flag || (*meaning_indexs_it) == sample_index);
    }

    return flag;
}

std::vector<int>
choice_selected_meanings(std::vector<Rule> meanings, MSILMParameters param) {
    std::vector<int> collect_meaning_indexs;
    int index;
    bool flag = false;
    for (int i = 0; i < param.MULTIPLE_MEANINGS; i++) {
        if (param.MULTIPLE_MEANINGS == 100) {
            index = i;
        } else {
            index = MT19937::irand() % meanings.size();
        }
        flag = include_index_check(collect_meaning_indexs, index);
        while (flag) {
            index = MT19937::irand() % meanings.size();
            flag = include_index_check(collect_meaning_indexs, index);
        }
        collect_meaning_indexs.push_back(index);
    }
    return collect_meaning_indexs;
}

void
cognition_init(std::vector<int>& source, MSILMParameters& param) {

    source.clear();
    for (int i = 0; i < param.TERMS; i++) {
        source.push_back(1);
    }
    for (int i = 0; i < param.UTTERANCES - param.TERMS; i++) {
        source.push_back(0);
    }
    Random r;
    std::random_shuffle(source.begin(), source.end(), r);

}

int main(int argc, char* argv[]) {
    typedef std::vector<MSILMAgent> Generation; //future work

    /*
     * ほぼ静的クラスだが、シリアライズのため（Dictionary）と、
     * デストラクタ実行のため（LogBox）にインスタンスを作成
     * */
    Dictionary dic;
    LogBox log; //for destructor
    time_t start, now;

    limit_time = 20 * 60; //second

    MSILMParameters param;
    //  boost::progress_display *show_progress = 0;

    Generation all_generations;
    std::vector<Rule> meaning_space;
    std::vector<Element> individuals;
    MSILMAgent parent_agent, child_agent;

    int utterance_counter = 0;

    int use_meaning_index = 0;
    int generation_counter = 0;
    int Base_Counter = 0;
    Rule utter, use_meaning;
    std::vector<int> use_meaning_indexs;
    std::vector<Rule> use_meanings, utters;
    std::vector<std::vector<Rule> > use_meaningss;
    //	std::vector<Rule> meanings_copy;
    //	std::vector<std::vector<int> > result;
    std::vector<int> cognition_flag;

    std::vector<std::vector<int> > correct_meaningss;

    /**************************************************
     *
     * OPTION PROCESSING
     *
     **************************************************/
    boost::program_options::options_description opt("option");
    opt.add_options()
            ("help,h", "Description")
            //experiment parameters
            /*ランダムシード*/
            ("random-seed,r",
            boost::program_options::value<uint32_t>(),
            "Random seed (101010)")

            /*実験世代数*/
            ("generations,g",
            boost::program_options::value<int>(),
            "Max generation number (100)")
            /*発話回数*/
            ("utterances,u",
            boost::program_options::value<double>(),
            "Uttering ratio for meaning space (0.5/[0-1])")
            /*エージェントに渡す意味の数*/
            ("multiple-meanings,m",
            boost::program_options::value<int>(),
            "At once utterance process, number of meanings given agent (3)")
            /*学習期間の設定*/
            ("term",
            boost::program_options::value<double>(),
            "Single meaning ratio for utterances (1.0/[0-1])")
            /*短期記憶期間の設定*/
            ("window",
            boost::program_options::value<int>(),
            "Number of utterance for prediction of meaning")
            /*対称性バイアス*/
            ("symmetry", "When received multiple meanings, use symmetry bias")
            /*相互排他性バイアス*/
            ("mutual-exclusivity", "When received multiple meanings, use mutual exclusivity bias")
            /*知識修正*/
            ("exception", "When child finds own mistake in his knowledge, try to correct his knowledge")
            /*言葉の省略A*/
            ("omission-A", "Omission")
            /*言葉の省略B*/
            ("omission-B", "Omission")
            /*言葉の省略C*/
            ("omission-C", "Omission")
            /*言葉の省略D*/
            ("omission-D", "Omission")
            /*規則削除文字列長*/
            ("delete-rule-length", boost::program_options::value<int>(),
            "delete rule length (if it not used, then it undefined)")
            /*ロギング*/
            ("logging,l",
            "Logging")
            /*最短発話*/
            ("minimum-utter", "utter minimum length")
            /*インデックス使用発話*/
            ("index", "construct index of sentence, and use index")
            /*分析*/
            ("analyze,a",
            "Analyze each agent for expression and a number of rules")
            /*生成文字列最大長*/
            ("word-length,w",
            boost::program_options::value<int>(),
            "Max length of created word (3)")
            /*世代における解析間隔*/
            ("interspace-analysis", boost::program_options::value<int>(),
            "set analysis interspace for the number of generations")
            /*世代におけるロギング間隔*/
            ("interspace-logging", boost::program_options::value<int>(),
            "set logging interspace for the number of generations")
            /*親と子の選択した意味が合っていたかどうかの結果出力*/
            ("accuracy-meaning", "Output logging whether parent and child selected same meaning")
            /*辞書ファイル*/
            ("dictionary,d",
            boost::program_options::value<std::string>(),
            "Dictionary file name for meaning space(\"/home/hiroki/Dropbox/LEILA/leila/LEILA/data.dic\")")

            /*生成規則再利用*/
            ("keep-random-rule",
            "Keep created rules with a random word into parent knowledge-base")
            /*規則削除使用*/
            ("delete-redundant-rules",
            "Delete redundant rules")
            /*補完発話*/
            ("invention-utterance",
            "Uttering with invention")
            /*非重複発話*/
            ("unique-utterance",
            "Do not use the same meaning on utterance process")
            /*最大外部言語入力文字列長*/
            ("max-listening-length", boost::program_options::value<int>(),
            "Max length of listening external letters. (0) The value of 0 means it is unlimited")
            /*FILE PREFIX*/
            ("prefix",
            boost::program_options::value<std::string>(),
            "Set file prefix (\"MSILM\")")

            /*BASE PATH*/
            ("path",
            boost::program_options::value<std::string>(),
            "Set folder for output files (\"/home/hiroki/Desktop/MSILMresult/TEST/\")")
            /*再開*/
            ("resume",
            boost::program_options::value<std::vector<std::string> >(),
            "Resume an experiment from saved file name. Then used path/resume_file")
            /*保存*/
            ("last-save",
            "Save the last state of the experiment into the file")
            /*全保存*/
            ("all-save",
            "Save the all state of the experiment into the file with the prefix")
            /*保存形式*/
            ("format",
            boost::program_options::value<std::string>(),
            "Set saving format (bin/[xml])")

            /*プログレスバー*/
            ("progress,p",
            "Show progress bar");


    //process options
    boost::program_options::variables_map vm;
    try {
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, opt), vm);
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
        std::cerr << "multiple_occurrences:" << ee.get_option_name() << std::endl;
        std::cerr << std::endl << "SEE HELP" << std::endl << opt << std::endl;
        return 1;
    } catch (boost::program_options::validation_error ee) {
        std::cerr << "validation_error:" << ee.get_option_name() << std::endl;
        std::cerr << std::endl << "SEE HELP" << std::endl << opt << std::endl;
        return 1;
    } catch (boost::program_options::too_many_positional_options_error ee) {
        std::cerr << "too_many_positional_options_error:" << ee.what() << std::endl;
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


    //resume
    if (param.RESUME) {
        try {
            //			boost::filesystem::ifstream ifs(param.BASE_PATH / param.RESUME_FILE);
            std::ifstream ifs((param.BASE_PATH + param.RESUME_FILE).c_str());

            switch (param.SAVE_FORMAT) {
                case Parameters::BIN:
                {
                    boost::archive::binary_iarchive ia(ifs);
                    resume_agent(ia, param, MT19937::icount, MT19937::rcount, dic, meaning_space, Base_Counter, parent_agent);
                }
                    break;

                case Parameters::XML:
                {
                    boost::archive::xml_iarchive ia(ifs);
                    resume_agent(ia, param, MT19937::icount, MT19937::rcount, dic, meaning_space, Base_Counter, parent_agent);
                }
                    break;

                default:
                    return 0;
            }

            //child_agent = parent_agent.make_child();
            MT19937::set_seed(param.RANDOM_SEED);
            MT19937::waste();
        } catch (...) {
            std::cerr << "State file Error" << std::endl;
            return 0;
        }
    } else {

        //             std::cerr << "detect!!" << std::endl;
        /*
         * Meaning Space
         */
        MT19937::set_seed(param.RANDOM_SEED);
        //		KnowledgeBase::CONTROLS |= KnowledgeBase::ANTECEDE_COMPOSITION;
        //		KnowledgeBase::CONTROLS |= param.CONTROLS;
        KnowledgeBase::set_control(
                KnowledgeBase::ANTECEDE_COMPOSITION | param.CONTROLS);

        //initialize
        if (param.DEL_LONG_RULE) {
            MSILMAgent::DEL_LONG_RULE = true;
            MSILMAgent::DEL_LONG_RULE_LENGTH = param.DEL_LONG_RULE_LENGTH;
        }

        if (param.EX_LIMIT != 0) {
            MSILMAgent::SHORT_MEM_SIZE = param.EX_LIMIT;
        }

        //		if (param.PARENT_ONCE) {
        //			NetWorld::PARENT_ONCE = true;
        //		}
        MSILMAgent::UTTER_MINIMUM = param.UTTER_MINIMUM;
        MSILMAgent::INDEXER_FLAG = param.INDEX;

        //             std::cerr << param.DICTIONARY_FILE << std::endl;

        //        MSILMAgent::UTTER_MINIMUM = false;
        //        MSILMAgent::INDEXER_FLAG = false;

        dic.load(param.DICTIONARY_FILE);

        //             std::cerr << "detect!!" << std::endl;

        construct_meanings(meaning_space);
        construct_individuals(individuals, dic);



        if (param.LOGGING) {
            std::vector<Rule>::iterator mean_it;
            mean_it = meaning_space.begin();
            LogBox::push_log("USEING MEANINGS");
            for (; mean_it != meaning_space.end(); mean_it++) {
                LogBox::push_log((*mean_it).to_s());
            }
            LogBox::push_log("\n");
        }
    }


    ///*
    // * Utterance times
    // */
    //param.UTTERANCES = (int)round(param.PER_UTTERANCES * meaning_space.size());
    //if(param.LOGGING){
    //	LogBox::push_log("UTTRANCE TIMES = " + boost::lexical_cast<std::string>(param.UTTERANCES));
    //}


    if (param.LOGGING)
        MSILMAgent::logging_on();
    if (param.SYMMETRY)
        MSILMAgent::sym_on();
    if (param.MUTUAL_EXCLUSIVITY)
        MSILMAgent::mut_on();
    if (param.EXCEPTION)
        MSILMAgent::exc_on();
    if (param.OMISSION_A || param.OMISSION_B || param.OMISSION_C || param.OMISSION_D) {
        MSILMAgent::omission_on();
        KnowledgeBase::MEANING_SPACE = meaning_space;
    }
    if (param.OMISSION_A)
        KnowledgeBase::omissionA_on();
    if (param.OMISSION_B)
        KnowledgeBase::omissionB_on();
    if (param.OMISSION_C)
        KnowledgeBase::omissionC_on();
    if (param.OMISSION_D)
        KnowledgeBase::omissionD_on();
    if (false) {
        std::vector<Rule>::iterator mean_it;
        mean_it = KnowledgeBase::MEANING_SPACE.begin();
        std::cout << ("USEING MEANINGS") << std::endl;
        for (; mean_it != KnowledgeBase::MEANING_SPACE.end(); mean_it++) {
            //std::cout << (*mean_it).to_s() << std::endl;
            std::cout << ("NEXT") << std::endl;
        }
        std::cout << ("USEING MEANINGS FIN.") << std::endl;
    }

    child_agent = parent_agent.make_child();
    Base_Counter = parent_agent.generation_index;

    //使用意味空間の出力
    if (param.LOGGING) {
        //LogBox::set_filepath(param.LOG_FILE);
        LogBox::set_filepath(param.BASE_PATH + param.LOG_FILE);
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

    /*
     * Utterance times
     */
    param.UTTERANCES = (int) round(param.PER_UTTERANCES * meaning_space.size());
    param.TERMS = (int) round(param.PER_TERM * param.UTTERANCES);
    if (param.LOGGING) {
        LogBox::push_log("UTTRANCE TIMES = " + boost::lexical_cast<std::string>(param.UTTERANCES));
    }


    /*
     * Progress bar construction
     */
    //    if (param.PROGRESS) {
    boost::progress_display show_progress(param.UTTERANCES * param.MAX_GENERATIONS);
    //    }

    //Log file Path
    LogBox::set_filepath(param.BASE_PATH + param.LOG_FILE);

    //Parameter Output
    {
        //boost::filesystem::path param_file("Parameters_" + param.DATE_STR + ".prm");
        //boost::filesystem::ofstream ofs(param.BASE_PATH / param_file);
        std::string param_file("Parameters_" + param.DATE_STR + "_" + boost::lexical_cast<std::string>(param.RANDOM_SEED) + ".prm");
        std::ofstream ofs((param.BASE_PATH + param_file).c_str());
        ofs << param.to_s() << std::endl;
    }

    //if (NetWorld::agent_num * param.MAX_GENERATIONS > 1000)
    //	online_analyze = true;

    if (param.INTER_ANALYSIS) {
        MSILMAgent::logging_off();
        param.LOGGING = false;
    }


    //main loop
    while (generation_counter < param.MAX_GENERATIONS) {
        std::vector<Rule> meanings_copy;
        meanings_copy = meaning_space;

        utterance_counter = 0;
        //                std::cerr << "new gen" << std::endl;
        time(&start);

#ifdef DEBUG
        std::cerr << "Start Generation:" << generation_counter << std::endl;
#endif

        if (generation_counter + 1 == param.MAX_GENERATIONS
                && param.INTER_LOG) {
            MSILMAgent::logging_on();
            param.LOGGING = true;
        } else if (param.INTER_LOG) {
            if (generation_counter % param.SPACE_LOG == 0) {
                MSILMAgent::logging_on();
                param.LOGGING = true;
            } else {
                MSILMAgent::logging_off();
                param.LOGGING = false;
            }
        }

        if (param.LOGGING) {
            LogBox::push_log("\nGENERATION: " + boost::lexical_cast<std::string>(generation_counter + Base_Counter));
            LogBox::push_log("BEFORE TALKING");
            LogBox::push_log("\nPARRENT KNOWLEDGE");
            LogBox::push_log(parent_agent.to_s());
            LogBox::push_log("\n-->>EDUCATION");
        }
#ifdef DEBUG
        std::cerr << "Say & learn" << std::endl;
#endif

        cognition_init(cognition_flag, param);

        std::vector<int> pseudo_meanings;
        correct_meaningss.push_back(pseudo_meanings);
        while (utterance_counter < param.UTTERANCES && meanings_copy.size() != 0) {
            std::vector<Rule> temp_r;
            Rule parent_meaning;
            time(&now);
            use_meaningss.clear();
            utters.clear();
            for (int i = 0; i < param.WINDOW && utterance_counter < param.UTTERANCES; i++) {

                //                        std::cerr << difftime(now,start) << std::endl;
                if (difftime(now, start) > limit_time) {
                    std::cerr << "TIME OVER : GENERATION " << generation_counter + 1 << std::endl;
                    return 0;
                }

                if (cognition_flag[utterance_counter] == 1) {
                    use_meanings.clear();
                    use_meaning_index = MT19937::irand() % meanings_copy.size();
                    use_meaning = meanings_copy[use_meaning_index];
                    use_meanings.push_back(use_meaning);
                    temp_r = use_meanings;
                    //use_meaningss.push_back(temp_r);   
                } else {
                    //                    std::cerr << " next ";
                    use_meaning_indexs.clear();
                    use_meanings.clear();
                    use_meaning_indexs = choice_selected_meanings(meanings_copy, param);
                    std::vector<int>::iterator use_meaning_indexs_it = use_meaning_indexs.begin();
                    for (; use_meaning_indexs_it != use_meaning_indexs.end(); use_meaning_indexs_it++) {
                        use_meanings.push_back(meanings_copy[(*use_meaning_indexs_it)]);
                    }
                    temp_r = use_meanings;

                    //std::cerr << (*temp_r.begin()).to_s();
                    //use_meaningss.push_back(temp_r);
                }

                //                std::cerr << " 0: " << use_meanings.size();

                if (param.LOGGING) {
                    LogBox::push_log("\n\n\n" + boost::lexical_cast<std::string>(utterance_counter + 1) + " UTTERANCE(S)");
                    if (cognition_flag[utterance_counter] == 1) {
                        LogBox::push_log(
                                "MEANING INDEX: [" + boost::lexical_cast<std::string>(use_meaning_index) + "]");
                    } else {
                        LogBox::push_log("USE MULTIPLE MEANINGS:");
                        std::vector<int>::iterator use_meaning_indexs_it = use_meaning_indexs.begin();
                        for (; use_meaning_indexs_it != use_meaning_indexs.end(); use_meaning_indexs_it++) {
                            LogBox::push_log("INDEX: [" + boost::lexical_cast<std::string>((*use_meaning_indexs_it)) + "]");
                            LogBox::push_log("MEANING: " + (meanings_copy[(*use_meaning_indexs_it)]).to_s());
                        }
                    }
                }

#ifdef DEBUG
                std::cerr << "Say";
#endif
                //                        std::cerr << utterance_counter;
                //                        std::cerr << " : ";
                //                        std::cerr << use_meanings.size() << std::endl;

                if (cognition_flag[utterance_counter] == 1) {
                    //                std::cout << " The baked egg is delicious.3" << std::endl;
                    utter = parent_agent.say(use_meaning);
                    //                std::cout << " The baked egg is delicious.2" << std::endl;
                    utters.push_back(utter);
                    use_meaningss.push_back(temp_r);
                } else {

                    //                    std::cerr << " 1: " << use_meanings.size();
                    utter = parent_agent.dither_say(use_meanings);

                    parent_meaning = parent_agent.return_last_selected_meaning();
                    //                                std::cerr << opt << std::endl;
                    //                return 1;

                    //                std::cerr<< "BBBBBBBBBB" <<std::endl;
                    utters.push_back(utter);
                    use_meaningss.push_back(temp_r);
                }

                if (param.UNIQUE_UTTERANCE) {
                    if (param.MULTIPLE_MEANINGS > 1 && cognition_flag[utterance_counter] != 1) {
                        //                    std::cerr << "CANNOT ERASE USED MEANING BECAUSE USE MULTIPLE MEANINGS" << std::endl;
                        std::vector<int>::iterator use_meaning_indexs_it = use_meaning_indexs.begin();
                        std::vector<Rule> eraselist;
                        std::vector<Rule>::iterator eraselist_it;
                        for (; use_meaning_indexs_it != use_meaning_indexs.end(); use_meaning_indexs_it++) {
                            //                        meanings_copy.erase(meanings_copy.begin() + (*use_meaning_indexs_it));
                            eraselist.push_back(meanings_copy[(*use_meaning_indexs_it)]);
                        }
                        eraselist_it = eraselist.begin();
                        for (; eraselist_it != eraselist.end(); eraselist_it++) {
                            std::vector<Rule>::iterator meanings_copy_it = meanings_copy.begin();
                            for (; meanings_copy_it != meanings_copy.end(); meanings_copy_it++) {
                                if ((*meanings_copy_it) == (*eraselist_it)) {
                                    meanings_copy.erase(meanings_copy_it);
                                    break;
                                }
                            }
                        }
                    } else {
                        meanings_copy.erase(meanings_copy.begin() + use_meaning_index);
                    }
                }

                if (param.PROGRESS)
                    ++show_progress;

                utterance_counter++;
            }
            child_agent.dither_hear(utters, use_meaningss, meaning_space);
            if (parent_meaning.internal == child_agent.return_last_selected_meaning().internal) {
                correct_meaningss[correct_meaningss.size() - 1].push_back(1);
            } else {
                correct_meaningss[correct_meaningss.size() - 1].push_back(0);
            }
            //                        std::cout << " The baked egg is delicious." << std::endl;

#ifdef DEBUG
            std::cerr << " -> learn" << std::endl;
#endif


            child_agent.learn();

            //            std::cout << "CHILD LEARNED" << std::endl;

        }



        if (param.LOGGING) {
            LogBox::push_log("\n<<--EDUCATION");
            LogBox::push_log("\nGENERATION :" + boost::lexical_cast<std::string>(generation_counter + Base_Counter));
            LogBox::push_log("AFTER TALKING");
            LogBox::push_log("\nCHILD KNOWLEDGE");
            LogBox::push_log(child_agent.to_s());
        }

        child_agent = child_agent.grow(meaning_space);


#ifdef DEBUG
        std::cerr << "Save State" << std::endl;
#endif

        if (param.SAVE_ALL_STATE)
            all_generations.push_back(parent_agent);

#ifdef DEBUG
        std::cerr << "Analyze" << std::endl;
#endif
        if (generation_counter + 1 == param.MAX_GENERATIONS
                && param.INTER_ANALYSIS) {

            analyze_and_output(param, meaning_space, individuals,
                    child_agent, parent_agent, generation_counter);
            analyze_and_output(param, meaning_space, individuals,
                    parent_agent, child_agent, generation_counter);


        } else if (param.INTER_ANALYSIS) {

            if (generation_counter % param.SPACE_ANALYSIS == 0) {
                analyze_and_output(param, meaning_space, individuals,
                        child_agent, parent_agent, generation_counter);
                analyze_and_output(param, meaning_space, individuals,
                        parent_agent, child_agent, generation_counter);
            }

        } else if (param.ANALYZE) {
            //result.push_back(analyze(meaning_space, parent_agent));
            analyze_and_output(param, meaning_space, individuals,
                    child_agent, parent_agent, generation_counter);
            analyze_and_output(param, meaning_space, individuals,
                    parent_agent, child_agent, generation_counter);
        }

        if (param.LOGGING) {
            LogBox::refresh_log();
        }

#ifdef DEBUG
        std::cerr << "Change Generation" << std::endl;
#endif

        parent_agent = child_agent;
        child_agent = parent_agent.make_child();
        generation_counter++;
    }
    if (param.ACC_MEA) {
        std::string mea_file;
        mea_file = param.FILE_PREFIX + "_" + param.DATE_STR + "_" + boost::lexical_cast<std::string>(param.RANDOM_SEED) + ".mea.acc";
                accuracy_meaning_output(param, mea_file, correct_meaningss);
    }


    //saving proc
    if (param.SAVE_LAST_STATE) {
        std::ofstream ofs((param.BASE_PATH + param.SAVE_FILE).c_str());
        //        boost::filesystem::ofstream ofs(param.BASE_PATH / param.SAVE_FILE);
        switch (param.SAVE_FORMAT) {
            case Parameters::BIN:
            {
                boost::archive::binary_oarchive oa(ofs);
                int counter;
                counter = Base_Counter + param.MAX_GENERATIONS;
                save_agent<boost::archive::binary_oarchive>
                        (oa, param, MT19937::icount, MT19937::rcount, dic, meaning_space, counter, parent_agent);
            }
                break;

            case Parameters::XML:
            {
                boost::archive::xml_oarchive oa(ofs);
                int counter;
                counter = Base_Counter + param.MAX_GENERATIONS;
                save_agent<boost::archive::xml_oarchive>
                        (oa, param, MT19937::icount, MT19937::rcount, dic, meaning_space, counter, parent_agent);
            }
                break;

            default:
                std::cerr << "UNKNOWN FORMAT" << std::endl;
                return 0;
        }

    }
    if (param.SAVE_ALL_STATE) {
        std::vector<MSILMAgent>::iterator a_it;
        int index = 0;
        a_it = all_generations.begin();
        while (a_it != all_generations.end()) {
            std::string index_str;
            index_str = boost::lexical_cast<std::string>(index + Base_Counter);
            //			boost::filesystem::path stf(param.FILE_PREFIX+"_Gen_"+index_str+".st");
            //			boost::filesystem::ofstream ofs(param.BASE_PATH / stf);
            std::string stf((param.FILE_PREFIX + "_Gen_" + index_str + ".st").c_str());
            std::ofstream ofs((param.BASE_PATH + stf).c_str());

            switch (param.SAVE_FORMAT) {
                case Parameters::BIN:
                {
                    boost::archive::binary_oarchive oa(ofs);
                    int counter;
                    counter = Base_Counter + index;
                    save_agent(oa, param, MT19937::icount, MT19937::rcount, dic, meaning_space, counter, *a_it);
                }
                    break;

                case Parameters::XML:
                {
                    boost::archive::xml_oarchive oa(ofs);
                    int counter;
                    counter = Base_Counter + index;
                    save_agent(oa, param, MT19937::icount, MT19937::rcount, dic, meaning_space, counter, *a_it);
                }
                    break;

                default:
                    std::cerr << "UNKNOWN FORMAT" << std::endl;
                    return 0;
            }

            index++;
            a_it++;
        }
    }

    //if(param.ANALYZE){
    //	boost::filesystem::ofstream ofs(param.BASE_PATH / param.RESULT_FILE);
    //	std::vector<std::vector<int> >::iterator r_it;
    //	std::vector<int>::iterator line_it;

    //	ofs << "#Format = Generation Number, Expression Ratio, Sentence Rule Number, Word Rule Number" << std::endl;

    //	r_it = result.begin();
    //	while(r_it != result.end()){
    //		line_it = (*r_it).begin();
    //		while(line_it != (*r_it).end()){
    //			if(line_it != (*r_it).end()-1)
    //				ofs << *line_it << ", ";
    //			else
    //				ofs << (*line_it) << std::endl;
    //			line_it++;
    //		}
    //		r_it++;
    //	}
    //}

    //	std::cerr << MT19937::icount << std::endl;

    //delete
    //    delete show_progress;
    if (param.LOGGING)
        log.refresh_log();

    return 0;
}
