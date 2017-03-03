/*
* $Id:
* 0037
*
* $File:
* interface.h
*
* $Proj:
* interface for Statistical Machine Translation
*
* $Func:
* interface
*
* $Version:
* 0.0.1
*
* $Created by:
* Qiang Li
*
* $Email
* liqiangneu@gmail.com
*
* $Last Modified by:
* 2013-04-10,11:02
*/

#ifndef DECODER_ONLINE_SERVICE_H_
#define DECODER_ONLINE_SERVICE_H_

#include <string>
#include <map>
#include <fstream>
#include <cstring>
#include "basic_method.h"
#include "configuration.h"
#include "model.h"
#include "test_set.h"
#include "phrasebased_decoder.h"
#include "translation_memory.h"
#include "multi_thread.h"
#include "recasing.h"
#include "detokenizer.h"

using namespace std;
using namespace basic_method;
using namespace decoder_configuration;
using namespace model;
using namespace decoder_test_set;
using namespace decoder_phrasebased_decoder;
using namespace decoder_translation_memory;
using namespace multi_thread;
using namespace decoder_recasing;
using namespace decoder_detokenizer;

#define WITHOUT_TRANSLATION_MEMORY_
//#define SUPPORT_ONLINE_SERVICE_CE_
//#define SUPPORT_ONLINE_SERVICE_EC_
//#define SUPPORT_ONLINE_SERVICE_ANYLANG2ANYLANG_
#define SUPPORT_OFFLINE_SERVICE_


namespace decoder_online_service{

class OnlineService {

 public:
  OnlineService(){}
  ~OnlineService(){}

 public:
  TranslationMemoryModel translation_memory_model_;

 public:
  PunctuationMap punctuation_map_;

 public:
  Configuration config_smt_;
  Model         model_smt_ ;

#ifdef SUPPORT_ONLINE_SERVICE_CE_
 public:
  Configuration config_recasing_;
  Model         model_recasing_;
#endif

 public:
  ofstream      log_file_;

 public:
  string        system_log_file_name_;
};

}


#ifndef SUPPORT_OFFLINE_SERVICE_
    #ifdef WIN32
        #define DLLEXPORT __declspec(dllexport)
    #else
        #define DLLEXPORT
    #endif

    extern "C" DLLEXPORT void*  __init   ( const char* config );
#ifndef WITHOUT_TRANSLATION_MEMORY_
    extern "C" DLLEXPORT char*  __do_job ( void* class_handle, const char* msg_text );
#else
    extern "C" DLLEXPORT char*  __do_job ( void* class_handle, const char* msg_text, int print_log, const char* log_head );
#endif
    extern "C" DLLEXPORT void   __destroy( void* class_handle );
    extern "C" DLLEXPORT void   __reload ( void* class_handle );
#endif

#endif
