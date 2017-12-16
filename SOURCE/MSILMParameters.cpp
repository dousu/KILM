/*
 * LELAParameters.cpp
 *
 *  Created on: 2011/06/29
 *      Author: rindou
 */

#include "MSILMParameters.h"

//int LELAParameters::thread = 1;
bool MSILMParameters::UTTER_MINIMUM = false;
bool MSILMParameters::INDEX = false;

MSILMParameters::MSILMParameters() {
  //CONTACT_PROBABIRITY = 0;
  //NEIGHBOR_PROBABIRITY = 0;
  //CONVERT_FLAG = false;
  //igraph_file_name = "";
  //IGRAPH_FLAG = false;
  //AGENT_NUM = 0;
  DEL_LONG_RULE = false;

  INTER_ANALYSIS = false;
  SPACE_ANALYSIS = 0;
  INTER_LOG = false;
  SPACE_LOG = 0;

  //LINKED_MATRIX = false;
  DEL_LONG_RULE_LENGTH = 10;
  //CONVERTING_PATH = "";
  //CONVERTED_PATH = "./";
  //LINKED_MATRIX_PATH = "";

  //EX_CUT = false;
  EX_LIMIT = 0;
  //PARENT_ONCE = false;
  FILE_PREFIX = "MSILM_";
  
  MULTIPLE_MEANINGS=3;
  PER_TERM=1.0;
  TERMS=0;
  WINDOW=1;
  SYMMETRY=false;
  MUTUAL_EXCLUSIVITY=false;
  EXCEPTION=false;
  OMISSION_A=false;
  OMISSION_B=false;
  OMISSION_C=false;
  OMISSION_D=false;
  ACC_MEA=false;
  SAVE_FILE = (FILE_PREFIX + DATE_STR + STATE_EXT);
  RESULT_FILE = (FILE_PREFIX + DATE_STR + RESULT_EXT);
  RESUME_FILE = (FILE_PREFIX + DATE_STR + STATE_EXT);
  LOG_FILE = (FILE_PREFIX + DATE_STR + LOG_EXT);
  DICTIONARY_FILE = "/home/hiroki/Dropbox/LEILA/leila/LEILA/data.dic"; //どこのカレントディレクトリのコンソールからでも実行したいため絶対パスにした
  PER_UTTERANCES = 0.5; //意味空間の数の半分にしたかった．
  BASE_PATH = "/home/hiroki/Desktop/MSILMresult/TEST/"; //テストフォルダを初期値に設定
}

MSILMParameters::~MSILMParameters() {
  // TODO Auto-generated destructor stub
}

void
MSILMParameters::set_option(boost::program_options::variables_map& vm) {
  Parameters::set_option(vm);

  //if (vm.count("contact")) {
  //  CONTACT_PROBABIRITY = vm["contact"].as<double>();
  //}

  //if (vm.count("neighbor")) {
  //  NEIGHBOR_PROBABIRITY = vm["neighbor"].as<double>();
  //}

  //if (vm.count("convert")) {
  //  std::vector<std::string> buf;
  //  buf = vm["convert"].as<std::vector<std::string> >();
  //  if (buf.size() != 2) {
  //    std::cerr << "less than file names for converting" << std::endl;
  //    throw "less than file names for converting";
  //  }
//
//    CONVERT_FLAG = true;
//    CONVERTING_PATH = buf[0];
//    CONVERTED_PATH = buf[1];
//  }

  //if (vm.count("igraph")) {
  //  igraph_file_name = vm["igraph"].as<std::string>();
  //  IGRAPH_FLAG = true;
  //}

  //if (vm.count("agents")) {
  //  AGENT_NUM = vm["agents"].as<int>();
  //}

  //if (vm.count("linked-matrix")) {
  //  LINKED_MATRIX = true;
  //  LINKED_MATRIX_PATH = vm["linked-matrix"].as<std::string>();
  //}

  if (vm.count("delete-rule-length")) {
    DEL_LONG_RULE_LENGTH = vm["delete-rule-length"].as<int>();
    DEL_LONG_RULE = true;
  }
  //if (vm.count("threads")) {
  //  thread = vm["threads"].as<int>();
  //  LogBox::threads = thread;
  //}

  if (vm.count("minimum-utter")) {
    UTTER_MINIMUM = true;
  }
  if (vm.count("index")) {
    INDEX = true;
  }
  if (vm.count("interspace-analysis")) {
    INTER_ANALYSIS = true;
    SPACE_ANALYSIS = vm["interspace-analysis"].as<int>();
  }
  if (vm.count("interspace-logging")) {
    INTER_LOG = true;
    SPACE_LOG = vm["interspace-logging"].as<int>();
  }


  if (vm.count("max-listening-length")) {
    //EX_CUT = true;
    EX_LIMIT = vm["max-listening-length"].as<int>();
  }
  
  if (vm.count("multiple-meanings")){
    MULTIPLE_MEANINGS=vm["multiple-meanings"].as<int>();
  }
  
  if (vm.count("term")) {
    PER_TERM = vm["term"].as<double>();
  }
  
  if (vm.count("window")) {
    WINDOW = vm["window"].as<int>();
  }
  
  if (vm.count("symmetry")) {
    SYMMETRY = true;
  }
  
  if (vm.count("mutual-exclusivity")) {
    MUTUAL_EXCLUSIVITY = true;
  }
  
  if (vm.count("exception")) {
    EXCEPTION = true;
  }
  
  if (vm.count("omission-A")) {
    OMISSION_A = true;
  }
  
  if (vm.count("omission-B")) {
    OMISSION_B = true;
  }
  
  if (vm.count("omission-C")) {
    OMISSION_C = true;
  }
  
  if (vm.count("omission-D")) {
    OMISSION_D = true;
  }
  
  if (vm.count("accuracy-meaning")) {
    ACC_MEA = true;
  }

  //if (vm.count("once-parent-test")) {
  //  PARENT_ONCE = true;
  //}
      //必ずprefixの変更後に行うこと
    SAVE_FILE = (FILE_PREFIX + DATE_STR + "_" + boost::lexical_cast<std::string>(RANDOM_SEED) + STATE_EXT);
    RESULT_FILE = (FILE_PREFIX + DATE_STR + "_" + boost::lexical_cast<std::string>(RANDOM_SEED) + RESULT_EXT);
    RESUME_FILE = (FILE_PREFIX + DATE_STR + "_" + boost::lexical_cast<std::string>(RANDOM_SEED) + STATE_EXT);
    LOG_FILE = (FILE_PREFIX + DATE_STR + "_" + boost::lexical_cast<std::string>(RANDOM_SEED) + LOG_EXT);
}

std::string
MSILMParameters::to_s(void) {
  std::string param1, param2;
  std::vector<std::string> bag;
  param1 = Parameters::to_s();
  bag.push_back(param1);

  //if (svm.count("contact")) {
  //  bag.push_back("--contact");
  //  bag.push_back(
  //      boost::lexical_cast<std::string>(svm["contact"].as<double>()));
  //}

  //if (svm.count("neighbor")) {
  //  bag.push_back("--contact");
  //  bag.push_back(
  //      boost::lexical_cast<std::string>(svm["neighbor"].as<double>()));
  //}

  //if (svm.count("convert")) {
  //  bag.push_back("--convert");
  //  std::vector<std::string> buf;
  //  buf = svm["convert"].as<std::vector<std::string> >();

  //  bag.push_back(buf[0]);
  //  bag.push_back(buf[1]);
  //}

  //if (svm.count("igraph")) {
  //  bag.push_back("--igraph");
  //  bag.push_back(svm["igraph"].as<std::string>());
  //}

  //if (svm.count("agents")) {
  //  bag.push_back("--agents");
  //  bag.push_back(boost::lexical_cast<std::string>(svm["agents"].as<int>()));
  //}

  //if (svm.count("linked-matrix")) {
  //  bag.push_back("--linked-matrix");
  //  bag.push_back(svm["linked-matrix"].as<std::string>());
  //}
  if (svm.count("delete-rule-length")) {
    bag.push_back("--delete-rule-length");
    bag.push_back(
        boost::lexical_cast<std::string>(svm["delete-rule-length"].as<int>()));
  }

  //if (svm.count("threads")) {
  //  bag.push_back(boost::lexical_cast<std::string>(svm["threads"].as<int>()));
  //}

  if (svm.count("minimum-utter")) {
    bag.push_back("--minimum-utter");
  }
  if (svm.count("index")) {
    bag.push_back("--index");
  }

  if (svm.count("interspace-analysis")) {
    bag.push_back("--interspace-analysis " + boost::lexical_cast<std::string>(svm["interspace-analysis"].as<int>()));
  }
  if (svm.count("interspace-logging")) {
    bag.push_back("--interspace-logging " + boost::lexical_cast<std::string>(svm["interspace-logging"].as<int>()));
  }

  if (svm.count("max-listening-length")) {
    bag.push_back("--max-listening-length " + boost::lexical_cast<std::string>(svm["max-listening-length"].as<int>()));
  }
  
  if (svm.count("multiple-meanings")){
      bag.push_back("--multiple-meanings " + boost::lexical_cast<std::string>(svm["multiple-meanings"].as<int>()));
  }
  
  if (svm.count("term")){
      bag.push_back("--term " + boost::lexical_cast<std::string>(svm["term"].as<double>()));
  }
  
  if (svm.count("window")){
      bag.push_back("--window " + boost::lexical_cast<std::string>(svm["window"].as<int>()));
  }
  
  if (svm.count("symmetry")) {
    bag.push_back("--symmetry ");
  }
  
  if (svm.count("mutual-exclusivity")) {
    bag.push_back("--mutual-exclusivity ");
  }
  
  if (svm.count("exception")){
      bag.push_back("--exception ");
  }
  
  if (svm.count("omission-A")){
      bag.push_back("--omission-A ");
  }
  
  if (svm.count("omission-B")){
      bag.push_back("--omission-B ");
  }
  
  if (svm.count("omission-C")){
      bag.push_back("--omission-C ");
  }
  
  if (svm.count("omission-D")){
      bag.push_back("--omission-D ");
  }
  
  if (svm.count("accuracy-meaning")){
      bag.push_back("--accuracy-meaning ");
  }

  //if (svm.count("once-parent-test")) {
  //  bag.push_back("--once-parent-test");
  //}


  return boost::algorithm::join(bag, " ");
}
