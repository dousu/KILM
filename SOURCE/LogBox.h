/*
 * LogBox.h
 *
 *  Created on: 2011/06/18
 *      Author: Rindow
 */

#ifndef LOGBOX_H_
#define LOGBOX_H_

#include <vector>
#include <list>
#include <string>
#include <iostream>
#include <fstream>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

//#include "LEILAParameters.h"
/*
class LogWriter {
  public:
    typedef std::vector<std::string> LogT;

    LogWriter();
    LogWriter(std::string);
    virtual ~LogWriter();

    void
    operator()();

    void
    stop(void);
    void
    set_log(LogT new_log);

  private:
    boost::condition_variable_any log_cond;
    boost::mutex log_mutex;
    std::string file_name;
    LogT log;
    bool exit_rq;
};
*/

/*!
 * ロギングデータを保持するクラス
 * 特に意識する必要なし。
 */
class LogBox {
  public:
    static std::vector<std::string> log;
    static std::string log_file;
    static int log_size;
    static int threads;
    //static boost::shared_ptr<LogWriter> writer;

    LogBox();
    virtual
    ~LogBox();

    static void
    push_log(std::string);
    static void
    pop_log(int nth = 1);
    static void
    refresh_log(void);

    static void
    set_filepath(std::string);
};


#endif /* LOGBOX_H_ */
