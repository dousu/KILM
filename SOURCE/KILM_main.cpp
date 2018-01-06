/*
 * main.cpp
 *
 *  Created on: 2012/12/07
 *      Author: Hiroki Sudo
 */
#include "KILM_main.h"



template<typename _OFS>
void save_agent(
		_OFS& ofs,
		Parameters& param,
		unsigned long long int& icount,
		unsigned long long int& rcount,
		Dictionary& dic,
		std::vector<Rule>& meanings,
		int& Generation_Counter,
		KirbyAgent& agent
){
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
		Parameters& param,
		unsigned long long int& icount,
		unsigned long long int& rcount,
		Dictionary& dic,
		std::vector<Rule>& meanings,
		int& Generation_Counter,
		KirbyAgent& agent
){
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

void construct_individuals(std::vector<Element>& inds, Dictionary &dic) {
	Dictionary::DictionaryType::iterator dic_it;
	dic_it = dic.individual.begin();
	for (; dic_it != dic.individual.end(); dic_it++) {
		Element elem;
		elem.set_ind((*dic_it).first);
		inds.push_back(elem);
	}
}

std::vector<std::vector<double> > analyze(std::vector<Rule>& meanings, KirbyAgent& agent1, KirbyAgent& agent2) {
    
	std::vector<double> unit_result, result_dist;
	std::vector<std::vector<double> > result;
	
	unit_result.clear();
	result_dist.clear();

	//Agent単体の分析
	unit_analyze(unit_result, meanings, agent1);

	//Agent相互の分析
	calculate_language_distance(result_dist, meanings, agent1, agent2);

	//リターンバケットに結果をつめて返す
	result.push_back(unit_result);
	result.push_back(result_dist);

	return result;
}

//std::vector<int> agent_to_s(std::vector<Rule>& meanings, KirbyAgent& agent){
//	std::vector<int> result;
//	result.push_back(agent.generation_index);
//	result.push_back(expression(meanings,agent));
//	result.push_back(agent.kb.sentenceDB.size());
//	result.push_back(agent.kb.wordDB.size());
//	return result;
//}

void unit_analyze(std::vector<double>& result_matrix,
		std::vector<Rule>& meanings, KirbyAgent& agent) {
	int index = 0, max_index;
	int GEN = 0, EXP = 1, SRN = 2, WRN = 3;

	//Item1 :generation number
	result_matrix.push_back(agent.generation_index);

	//Item2 :expressiveness
	result_matrix.push_back(expression(meanings, agent));

	//Item3 :sentence rule number
	result_matrix.push_back(agent.kb.sentenceDB.size());

	//Item4 :Word rule number
	result_matrix.push_back(agent.kb.wordDB.size());
}

int expression(std::vector<Rule>& meanings, KirbyAgent& agent){
	std::vector<Rule>::iterator mean_it;
	int counter = 0;
	mean_it = meanings.begin();
	while(mean_it != meanings.end()){
		if(agent.utterable(*mean_it))
			counter++;
		mean_it++;
	}

	return counter;
}

void calculate_language_distance(
		std::vector<double>& dist_matrix,
		std::vector<Rule>& meanings,
		KirbyAgent& agent1, KirbyAgent& agent2) {

        dist_matrix.push_back(
            calculate_distance(meanings,agent1.kb,agent2.kb));
}

void analyze_and_output(Parameters& param, std::vector<Rule> meaning_space,
		KirbyAgent& agent1, KirbyAgent& agent2) {
	std::vector<std::vector<double> > res;
	std::string index_str, file, file_postfix;
	int index;

	index = agent1.generation_index;
	index_str = boost::lexical_cast<std::string>(index);
	file = param.FILE_PREFIX + param.DATE_STR + "_" + index_str + ".rst";

	res = analyze(meaning_space, agent1, agent2);

	//basic analyze
	std::ofstream ofs((param.BASE_PATH + file).c_str(),
			std::ios::app | std::ios::out);

	ofs << "#RESULT" << std::endl;

	//unit
	ofs << "BASIC=" << tr_vector_double_to_string(res[0]) << std::endl;

	//distance(parent and child)
	ofs << "SDISTM=" << tr_vector_double_to_string(res[1]) << std::endl;
}

double calculate_distance(std::vector<Rule>& meanings,
		KnowledgeBase& kb1, KnowledgeBase& kb2) {
    
	std::vector<Rule>::iterator mean_it, kb1_pat_it, kb2_pat_it, kb1_all_it, kb2_all_it, target_it;
	std::vector<Rule> kb1_pat, kb2_pat, kb1_all, kb2_all, target_rules;
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
                    target_rules=std::vector<Rule>();
                    target_rules.push_back(*kb2_all_it);
                }else if(ham==min_ham){
                    target_rules.push_back(*kb2_all_it);
                }
            }
            min_lev=2;
            target_it=target_rules.begin();
            for(;target_it!=target_rules.end();target_it++){
                lev=Distance::levenstein((*kb1_all_it).external,(*target_it).external);
                if(lev<min_lev){
                    min_lev=lev;
                }
            }
            lev_sum+=min_lev;
        }
        
        return lev_sum / (static_cast<double>(kb1_all.size()));
}

std::string
tr_vector_double_to_string(std::vector<double> vector){
    std::string res="(";
    std::stringstream ss;
    std::string work;
    std::vector<double>::iterator double_it=vector.begin();
    ss.clear();
    ss << (*double_it);
    ss >> work;
    res = res + work;
    double_it++;
    for(;double_it!=vector.end();double_it++){
        ss.clear();
        ss << (*double_it);
        ss >> work;
        res = res + "," + work;
    }
    return (res+")");
}


int main(int argc, char* argv[]){
	typedef std::vector<KirbyAgent> Generation;//future work

	/*
	 * ほぼ静的クラスだが、シリアライズのため（Dictionary）と、
	 * デストラクタ実行のため（LogBox）にインスタンスを作成
	 * */
	Dictionary        dic;
	LogBox            log;//for destructor



	Parameters        param;
	boost::progress_display *show_progress = 0;

	Generation        all_generations;
	std::vector<Rule> meaning_space;
        std::vector<Element> individuals;
	KirbyAgent        parent_agent, child_agent;

	int               utterance_counter =0;

	int use_meaning_index = 0;
	int generation_counter = 1;
	int Base_Counter = 0;
	Rule utter, use_meaning;


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
					"Uttering ratio for meaning space (0.5/[0~1])")
			/*ロギング*/
					("logging,l",
					"Logging")
			/*分析*/
					("analyze,a",
					"Analyze each agent for expression and a number of rules")
			/*生成文字列最大長*/
					("word-length,w",
					boost::program_options::value<int>(),
					"Max length of created word (3)")
//                        /*世代における解析間隔*/
//                                        ("interspace-analysis", boost::program_options::value<int>(),
//                                        "set analysis interspace for the number of generations")
//
//                        /*世代におけるロギング間隔*/
//                                        ("interspace-logging", boost::program_options::value<int>(),
//                                        "set logging interspace for the number of generations")
			/*辞書ファイル*/
					("dictionary,d",
					boost::program_options::value<int>(),
					"Dictionary file name for meaning space(\"./data.dic\")")

			/*random生成規則再利用*/
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
			/*FILE PREFIX*/
					("prefix",
					boost::program_options::value<std::string>(),
					"Set file prefix (\"KILM\")")

			/*BASE PATH*/
					("path",
					boost::program_options::value<std::string>(),
					"Set folder for output files (\"../RESULT/\")")
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
	try{
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, opt), vm);
		boost::program_options::notify(vm);
	}
	catch (boost::program_options::invalid_syntax ee){
		std::cerr << "invalid syntax:" << ee.tokens() << std::endl;
		std::cerr << std::endl << "SEE HELP" << std::endl << opt << std::endl;
		return 1;
	}
	catch (boost::program_options::unknown_option ee){
		std::cerr << "unknown_option:" << ee.what()<< std::endl;
		std::cerr << std::endl << "SEE HELP" << std::endl << opt << std::endl;
		return 1;
	}
	catch (boost::program_options::ambiguous_option ee){
		std::cerr << "ambiguous_option:" << ee.get_option_name()<< std::endl;
		std::cerr << std::endl << "SEE HELP" << std::endl << opt << std::endl;
		return 1;
	}
	catch (boost::program_options::multiple_values ee){
		std::cerr << "multiple_values:" << ee.get_option_name()<< std::endl;
		std::cerr << std::endl << "SEE HELP" << std::endl << opt << std::endl;
		return 1;
	}
	catch (boost::program_options::multiple_occurrences ee){
		std::cerr << "multiple_occurrences:" << ee.get_option_name()<< std::endl;
		std::cerr << std::endl << "SEE HELP" << std::endl << opt << std::endl;
		return 1;
	}
	catch (boost::program_options::validation_error ee){
		std::cerr << "validation_error:" << ee.get_option_name()<< std::endl;
		std::cerr << std::endl << "SEE HELP" << std::endl << opt << std::endl;
		return 1;
	}
	catch (boost::program_options::too_many_positional_options_error ee){
		std::cerr << "too_many_positional_options_error:" << ee.what() << std::endl;
		std::cerr << std::endl << "SEE HELP" << std::endl << opt << std::endl;
		return 1;
	}
	catch (boost::program_options::invalid_command_line_style ee){
		std::cerr << "invalid_command_line_style:" << ee.what() << std::endl;
		std::cerr << std::endl << "SEE HELP" << std::endl << opt << std::endl;
		return 1;
	}
	catch (boost::program_options::reading_file ee){
		std::cerr << "reading_file:" << ee.what() << std::endl;
		std::cerr << std::endl << "SEE HELP" << std::endl << opt << std::endl;
		return 1;
	}
	catch (boost::program_options::required_option ee){
		std::cerr << "required_option:" << ee.get_option_name()<< std::endl;
		std::cerr << std::endl << "SEE HELP" << std::endl << opt << std::endl;
		return 1;
	}


	//check options
	if(vm.count("help")){
		std::cerr << opt << std::endl;
		return 1;
	}


	//set parameters
	try{
		param.set_option(vm);
	}
	catch(std::string e){
		std::cerr << "option:" << e << std::endl;
		exit(0);
	}


	//resume
	if(param.RESUME){
		try{
                std::ifstream ifs((param.BASE_PATH + param.RESUME_FILE).c_str());

				switch(param.SAVE_FORMAT){
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
		}catch(...){
			std::cerr << "State file Error" << std::endl;
			return 0;
		}
	}else{
		//initialize
		dic.load(param.DICTIONARY_FILE);
		
		/*
		 * Meaning Space
		 */
		construct_meanings(meaning_space);
		construct_individuals(individuals, dic);
	}

	if(param.LOGGING)
		KirbyAgent::logging_on();

	child_agent = parent_agent.make_child();
	Base_Counter=parent_agent.generation_index;

	//使用意味空間の出力
	if (param.LOGGING) {
		//LogBox::set_filepath(param.LOG_FILE);
		LogBox::set_filepath(param.BASE_PATH + param.LOG_FILE);
		std::vector<Rule>::iterator mean_it;

		LogBox::push_log("USED RANDOM SEED");
		LogBox::push_log(boost::lexical_cast<std::string>(param.RANDOM_SEED));

		mean_it = meaning_space.begin();
		LogBox::push_log("USEING MEANINGS");
		for (; mean_it != meaning_space.end(); mean_it++) {
			LogBox::push_log((*mean_it).to_s());
		}
		LogBox::push_log("\n");
	}

	KnowledgeBase::set_control(KnowledgeBase::ANTECEDE_COMPOSITION | param.CONTROLS);
	MT19937::set_seed(param.RANDOM_SEED);
	MT19937::waste();

	/*
	 * Utterance times
	 */
	param.UTTERANCES = (int)round(param.PER_UTTERANCES * meaning_space.size());
	if(param.LOGGING){
		LogBox::push_log("UTTRANCE TIMES = " + boost::lexical_cast<std::string>(param.UTTERANCES));
	}


	/*
	 * Progress bar construction
	 */
	if(param.PROGRESS){
		show_progress = new boost::progress_display( param.UTTERANCES * param.MAX_GENERATIONS + 1);
	}

	//Log file Path
	LogBox::set_filepath(param.BASE_PATH + param.LOG_FILE);

	//Parameter Output
	{
		std::string param_file("Parameters_" + param.DATE_STR + ".prm");
		std::ofstream ofs((param.BASE_PATH + param_file).c_str());
		ofs << param.to_s() << std::endl;
	}

	//main loop
	while(generation_counter <= param.MAX_GENERATIONS){
		std::vector<Rule> meanings_copy;
		meanings_copy = meaning_space;

		utterance_counter = 0;

#ifdef DEBUG
		std::cerr << "Start Generation:" << generation_counter << std::endl;
#endif

		if(param.LOGGING){
			LogBox::push_log("\nGENERATION: "+ boost::lexical_cast<std::string>(generation_counter + Base_Counter));
			LogBox::push_log("BEFORE TALKING");
			LogBox::push_log("\nPARRENT KNOWLEDGE");
			LogBox::push_log(parent_agent.to_s());
			LogBox::push_log("\n-->>EDUCATION");
		}

#ifdef DEBUG
		std::cerr << "Say & learn" << std::endl;
#endif

		while(utterance_counter <= param.UTTERANCES && meanings_copy.size() != 0){

			use_meaning_index = MT19937::irand() % meanings_copy.size();
			use_meaning = meanings_copy[use_meaning_index];

			if (param.LOGGING) {
				LogBox::push_log(
					"MEANING INDEX: [" + boost::lexical_cast<std::string>(use_meaning_index)+ "]");
			}

#ifdef DEBUG
			std::cerr << "Say";
#endif

			utter = parent_agent.say(use_meaning);
			child_agent.hear(utter);

#ifdef DEBUG
			std::cerr << " -> learn" << std::endl;
#endif

			child_agent.learn();


			if(param.UNIQUE_UTTERANCE)
				meanings_copy.erase(meanings_copy.begin() + use_meaning_index);

			if(param.PROGRESS)
				++(*show_progress);

			utterance_counter++;
		}

		if(param.LOGGING){
			LogBox::push_log("\n<<--EDUCATION");
			LogBox::push_log("\nGENERATION :"+boost::lexical_cast<std::string>(generation_counter + Base_Counter));
			LogBox::push_log("AFTER TALKING");
			LogBox::push_log("\nCHILD KNOWLEDGE");
			LogBox::push_log(child_agent.to_s());
		}

#ifdef DEBUG
		std::cerr << "Save State" << std::endl;
#endif

		if(param.SAVE_ALL_STATE)
			all_generations.push_back(parent_agent);

#ifdef DEBUG
		std::cerr << "Analyze" << std::endl;
#endif

		if(param.ANALYZE){
			analyze_and_output(param, meaning_space,
					parent_agent, parent_agent);
			analyze_and_output(param, meaning_space,
					parent_agent, child_agent);
		}

		if (param.LOGGING) {
			LogBox::refresh_log();
		}

#ifdef DEBUG
		std::cerr << "Change Generation" << std::endl;
#endif

		parent_agent = child_agent.grow(meaning_space);
		child_agent = parent_agent.make_child();
		generation_counter++;
	}

	//saving proc
	if(param.SAVE_LAST_STATE){
        std::ofstream ofs((param.BASE_PATH + param.SAVE_FILE).c_str());
        switch(param.SAVE_FORMAT){
        	case Parameters::BIN:
        	{
        		boost::archive::binary_oarchive oa(ofs);
        		int counter;
        		counter = Base_Counter + param.MAX_GENERATIONS;
        		save_agent<boost::archive::binary_oarchive>(oa, param, MT19937::icount, MT19937::rcount, dic, meaning_space, counter, parent_agent);
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
	if(param.SAVE_ALL_STATE){
		std::vector<KirbyAgent>::iterator a_it;
		int index=0;
		a_it = all_generations.begin();
		while (a_it != all_generations.end()){
			std::string index_str;
			index_str = boost::lexical_cast<std::string>(index+Base_Counter);
            std::string stf((param.FILE_PREFIX+"_Gen_"+index_str+".st").c_str());
            std::ofstream ofs((param.BASE_PATH + stf).c_str());

			switch(param.SAVE_FORMAT){
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

	//destruction
	delete show_progress;
	if (param.LOGGING)
		log.refresh_log();

	return 0;
}
