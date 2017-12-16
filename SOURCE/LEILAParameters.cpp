/*
 * LELAParameters.cpp
 *
 *  Created on: 2011/06/29
 *      Author: rindou
 */

#include "LEILAParameters.h"

int LELAParameters::thread = 1;
bool LELAParameters::UTTER_MINIMUM = false;
bool LELAParameters::INDEX = false;

LELAParameters::LELAParameters() {
  CONTACT_PROBABIRITY = 0;
  NEIGHBOR_PROBABIRITY = 0;
  CONVERT_FLAG = false;
  igraph_file_name = "";
  IGRAPH_FLAG = false;
  AGENT_NUM = 0;
  DEL_LONG_RULE = false;

  INTER_ANALYSIS = false;
  SPACE_ANALYSIS = 0;
  INTER_LOG = false;
  SPACE_ANALYSIS = 0;

  LINKED_MATRIX = false;
  DEL_LONG_RULE_LENGTH = 10;
  CONVERTING_PATH = "";
  CONVERTED_PATH = "./";
  LINKED_MATRIX_PATH = "";

  //EX_CUT = false;
  EX_LIMIT = 0;
  PARENT_ONCE = false;
}

LELAParameters::~LELAParameters() {
  // TODO Auto-generated destructor stub
}

void
LELAParameters::set_option(boost::program_options::variables_map& vm) {
  Parameters::set_option(vm);

  if (vm.count("contact")) {
    CONTACT_PROBABIRITY = vm["contact"].as<double>();
  }

  if (vm.count("neighbor")) {
    NEIGHBOR_PROBABIRITY = vm["neighbor"].as<double>();
  }

  if (vm.count("convert")) {
    std::vector<std::string> buf;
    buf = vm["convert"].as<std::vector<std::string> >();
    if (buf.size() != 2) {
      std::cerr << "less than file names for converting" << std::endl;
      throw "less than file names for converting";
    }

    CONVERT_FLAG = true;
    CONVERTING_PATH = buf[0];
    CONVERTED_PATH = buf[1];
  }

  if (vm.count("igraph")) {
    igraph_file_name = vm["igraph"].as<std::string>();
    IGRAPH_FLAG = true;
  }

  if (vm.count("agents")) {
    AGENT_NUM = vm["agents"].as<int>();
  }

  if (vm.count("linked-matrix")) {
    LINKED_MATRIX = true;
    LINKED_MATRIX_PATH = vm["linked-matrix"].as<std::string>();
  }

  if (vm.count("delete-rule-length")) {
    DEL_LONG_RULE_LENGTH = vm["delete-rule-length"].as<int>();
    DEL_LONG_RULE = true;
  }
  if (vm.count("threads")) {
    thread = vm["threads"].as<int>();
    LogBox::threads = thread;
  }

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
    SPACE_ANALYSIS = vm["interspace-logging"].as<int>();
  }


  if (vm.count("max-listening-length")) {
    //EX_CUT = true;
    EX_LIMIT = vm["max-listening-length"].as<int>();
  }

  if (vm.count("once-parent-test")) {
    PARENT_ONCE = true;
  }
}

std::string
LELAParameters::to_s(void) {
  std::string param1, param2;
  std::vector<std::string> bag;
  param1 = Parameters::to_s();
  bag.push_back(param1);

  if (svm.count("contact")) {
    bag.push_back("--contact");
    bag.push_back(
        boost::lexical_cast<std::string>(svm["contact"].as<double>()));
  }

  if (svm.count("neighbor")) {
    bag.push_back("--contact");
    bag.push_back(
        boost::lexical_cast<std::string>(svm["neighbor"].as<double>()));
  }

  if (svm.count("convert")) {
    bag.push_back("--convert");
    std::vector<std::string> buf;
    buf = svm["convert"].as<std::vector<std::string> >();

    bag.push_back(buf[0]);
    bag.push_back(buf[1]);
  }

  if (svm.count("igraph")) {
    bag.push_back("--igraph");
    bag.push_back(svm["igraph"].as<std::string>());
  }

  if (svm.count("agents")) {
    bag.push_back("--agents");
    bag.push_back(boost::lexical_cast<std::string>(svm["agents"].as<int>()));
  }

  if (svm.count("linked-matrix")) {
    bag.push_back("--linked-matrix");
    bag.push_back(svm["linked-matrix"].as<std::string>());
  }
  if (svm.count("delete-rule-length")) {
    bag.push_back("--delete-rule-length");
    bag.push_back(
        boost::lexical_cast<std::string>(svm["delete-rule-length"].as<int>()));
  }

  if (svm.count("threads")) {
    bag.push_back(boost::lexical_cast<std::string>(svm["threads"].as<int>()));
  }

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

  if (svm.count("once-parent-test")) {
    bag.push_back("--once-parent-test");
  }


  return boost::algorithm::join(bag, " ");
}
