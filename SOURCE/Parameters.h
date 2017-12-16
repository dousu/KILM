/*
 * Parameters2.h
 *
 *  Created on: 2011/06/24
 *      Author: rindou
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
//#include <boost/filesystem.hpp>
//#include <boost/filesystem/fstream.hpp>
//#include <boost/filesystem/path.hpp>
//#include <boost/iostreams/operations.hpp>
#include <boost/random.hpp>

//#include "KirbyAgent.h"
//#include "Rule.h"
//#include "Element.h"
//#include "Random.hpp"
//#include "MT19937.h"
//#include "Dictionary.h"
//#include "LogBox.h"
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

    //boost::filesystem::path DICTIONARY_FILE;
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
    //boost::filesystem::path BASE_PATH;
    std::string BASE_PATH;

    //file
    //boost::filesystem::path LOG_FILE;
    //boost::filesystem::path RESUME_FILE;
    //boost::filesystem::path SAVE_FILE;
    //boost::filesystem::path RESULT_FILE;
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
