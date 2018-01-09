/*
 * Parameters.cpp
 *
 *  Created on: 2012/12/24
 *      Author: Hiroki Sudo
 */

#include "Parameters.h"

Parameters::Parameters() {
  time_t now;
  time(&now);
  struct tm *stm = localtime(&now);
  char s[100];
  strftime(s,100,"%Yy%mm%dd%Hh%Mm%Ss",stm);
  std::string date_str = boost::lexical_cast<std::string>(s);

  //initialization with default value
  MAX_GENERATIONS = 100;
  PER_UTTERANCES = 0.5;
  RANDOM_SEED = 101010;
  UNIQUE_UTTERANCE = false;
  SAVE_FORMAT = BIN;
  ANALYZE = false;
  DICTIONARY_FILE = "data.dic";
  buzz_length = 3;

  LOGGING = false;
  PROGRESS = false;
  RESUME = false;
  SAVE_LAST_STATE = false;
  SAVE_ALL_STATE = false;
  OMISSION = false;

  FILE_PREFIX = "KILM_";
  DATE_STR = date_str;
  STATE_EXT = ".st";
  RESULT_EXT = ".rst";
  LOG_EXT = ".log";

  BASE_PATH = "../RESULT/";
  SAVE_FILE = (FILE_PREFIX + DATE_STR + STATE_EXT);
  RESULT_FILE = (FILE_PREFIX + DATE_STR + RESULT_EXT);
  RESUME_FILE = (FILE_PREFIX + DATE_STR + STATE_EXT);
  LOG_FILE = (FILE_PREFIX + DATE_STR + LOG_EXT);

  CONTROLS = 0x0;

  UTTERANCES = 0;
  Generation_Counter = 0;
}

Parameters::~Parameters() {
//	// TODO Auto-generated destructor stub
}

void
Parameters::set_option(boost::program_options::variables_map& vm) {
  svm = vm;

  //Files
  if (vm.count("format")) {
    if (vm["format"].as<std::string>() == "xml")
      SAVE_FORMAT = Parameters::XML;
  }

  if (vm.count("prefix")) {
    FILE_PREFIX = vm["prefix"].as<std::string>();

    SAVE_FILE = (FILE_PREFIX + DATE_STR + STATE_EXT);
    RESULT_FILE = (FILE_PREFIX + DATE_STR + RESULT_EXT);
    RESUME_FILE = (FILE_PREFIX + DATE_STR + STATE_EXT);
    LOG_FILE = (FILE_PREFIX + DATE_STR + LOG_EXT);
  }

  if (vm.count("path")) {
    BASE_PATH = (vm["path"].as<std::string>());
  }

  //Set option values
  if (vm.count("random-seed")) {
    RANDOM_SEED = vm["random-seed"].as<uint32_t>();
  }

  if (vm.count("generations")) {
    MAX_GENERATIONS = vm["generations"].as<int>();
  }

  if (vm.count("utterances")) {
    PER_UTTERANCES = vm["utterances"].as<double>();
  }

  if (vm.count("omission")) {
	  OMISSION = true;
  }

  if (vm.count("analyze")) {
    RESULT_FILE = (FILE_PREFIX + RESULT_EXT);
    ANALYZE = true;
  }

  if (vm.count("unique-utterance")) {
    UNIQUE_UTTERANCE = true;
  }

  if (vm.count("dictionary")) {
    DICTIONARY_FILE = vm["dictionary"].as<std::string>();
  }

  if (vm.count("word-length")) {
    buzz_length = vm["word-length"].as<int>();
  }

  if (vm.count("keep-random-rule")) {
    CONTROLS |= KnowledgeBase::USE_ADDITION_OF_RANDOM_WORD;
  }

  if (vm.count("delete-redundant-rules")) {
    CONTROLS |= KnowledgeBase::USE_OBLITERATION;
  }

  if (vm.count("invention")) {
    CONTROLS |= KnowledgeBase::USE_SEMICOMPLETE_FABRICATION;
  }

  if (vm.count("logging")) {
    LOGGING = true;
  }

  if (vm.count("resume")) {
    std::vector<std::string> args;
    args = vm["resume"].as<std::vector<std::string> >();

    if (args.size() > 0) {
      RESUME_FILE = args.front();
    }

    RESUME = true;
  }

  if (vm.count("last-save")) {
    SAVE_LAST_STATE = true;
  }

  if (vm.count("all-save")) {
    SAVE_ALL_STATE = true;
  }

  if (vm.count("progress")) {
    PROGRESS = true;
  }
}

std::string
Parameters::to_s(void) {
  std::vector<std::string> bag;
  std::string str = "";

  //Files
  if (svm.count("format")) {
    bag.push_back("--format ");
    if (svm["format"].as<std::string>() == "xml")
      bag.push_back("xml");
    else if (svm["format"].as<std::string>() == "bin")
      bag.push_back("bin");
  }

  if (svm.count("prefix")) {
    bag.push_back("--prefix");
    bag.push_back(svm["prefix"].as<std::string>());
  }

  if (svm.count("path")) {
    bag.push_back("--path");
    bag.push_back(svm["path"].as<std::string>());
  }

  //Set option values
  if (svm.count("random-seed")) {
    bag.push_back("--random-seed");
    bag.push_back(
        boost::lexical_cast<std::string>(svm["random-seed"].as<uint32_t>()));
  }

  if (svm.count("generations")) {
    bag.push_back("--generations");
    bag.push_back(
        boost::lexical_cast<std::string>(svm["generations"].as<int>()));
  }

  if (svm.count("utterances")) {
    bag.push_back("--utterances");
    bag.push_back(
        boost::lexical_cast<std::string>(svm["utterances"].as<double>()));
  }

  if (svm.count("analyze")) {
    bag.push_back("--analyze");
  }

  if (svm.count("unique-utterance")) {
    bag.push_back("--unique-utterance");
  }

  if (svm.count("dictionary")) {
    bag.push_back("--dictionary");
    bag.push_back(svm["dictionary"].as<std::string>());
  }

  if (svm.count("word-length")) {
    bag.push_back("--word-length");
    bag.push_back(
        boost::lexical_cast<std::string>(svm["word-length"].as<int>()));
  }

  if (svm.count("keep-random-rule")) {
    bag.push_back("--keep-random-rule");
  }

  if (svm.count("delete-redundant-rules")) {
    bag.push_back("--delete-redundant-rules");
  }

  if (svm.count("invention")) {
    bag.push_back("--invention");
  }

  if (svm.count("logging")) {
    bag.push_back("--logging");
  }

  if (svm.count("resume")) {
    bag.push_back("--resume");
    std::vector<std::string> args;
    args = svm["resume"].as<std::vector<std::string> >();

    if (args.size() > 0) {
      bag.push_back(args.front());
    }
  }

  if (svm.count("last-save")) {
    bag.push_back("--last-save");
  }

  if (svm.count("all-save")) {
    bag.push_back("--all-save");
  }

  if (svm.count("progress")) {
    bag.push_back("--progress");
  }

  return boost::algorithm::join(bag, " ");
}

