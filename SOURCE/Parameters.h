/*
 * Parameters.h
 *
 *  Created on: 2012/12/24
 *      Author: Hiroki Sudo
 */

#ifndef PARAMETERS_H_
#define PARAMETERS_H_

#include <vector>
#include <ctime>
#include <cstdlib>
#include <string>
#include <stdint.h>
#include <climits>
#include <fstream>
#include <iostream>

#include <boost/program_options.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/export.hpp>

#include "KnowledgeBase.h"

class Parameters {
  public:
    //type definition
    enum FORMAT {
      BIN, XML
    };

    //experiment parameters
    int MAX_GENERATIONS;
    double PER_UTTERANCES; //
    boost::uint32_t RANDOM_SEED; //
    bool UNIQUE_UTTERANCE; //

    uint32_t CONTROLS;
    int buzz_length;

    //Execution Values
    int UTTERANCES;
    uint32_t Generation_Counter; //

    //system parameters
    bool LOGGING;
    bool PROGRESS;
    bool RESUME;
    bool SAVE_LAST_STATE;
    bool SAVE_ALL_STATE;
    bool ANALYZE;
    FORMAT SAVE_FORMAT;

    std::string DICTIONARY_FILE;

    //file parameters
    //file prefix
    std::string FILE_PREFIX;
    std::string DATE_STR;

    //file extentions
    std::string STATE_EXT;
    std::string RESULT_EXT;
    std::string LOG_EXT;

    //path
    std::string BASE_PATH;

    //file
    std::string LOG_FILE;
    std::string RESUME_FILE;
    std::string SAVE_FILE;
    std::string RESULT_FILE;

    boost::program_options::variables_map svm;

    Parameters();

    virtual
    ~Parameters();

    std::string
    to_s(void);

    virtual void
    set_option(boost::program_options::variables_map& vm);
};

#endif /* PARAMETERS2_H_ */
