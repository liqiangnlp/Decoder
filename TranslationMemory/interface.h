/*
* $Id:
* 0003
*
* $File:
* interface.h
*
* $Proj:
* TranslationMemoryLib for Statistical Machine Translation
*
* $Func:
* header file of interface
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
* 2014-01-10,13:14
*/

#ifndef TRANSLATIONMEMORYLIB_INTERFACE_H_
#define TRANSLATIONMEMORYLIB_INTERFACE_H_

#include <iostream>
#include <string>
#include <cstring>
#ifdef _WIN32
#include "../Decoder/translation_memory.h"
#else
#include "translation_memory.h"
#endif
using namespace std;
using namespace decoder_translation_memory;

//#define TRAINING_TRANSLATION_MEMORY_MODEL_

namespace translation_memory_interface {

class TranslationMemoryInterface {

 public:
  TranslationMemoryInterface(){}
  ~TranslationMemoryInterface(){}

 public:
  TranslationMemoryModel translation_memory_model_;

 public:
  ofstream      log_file_;

 public:
  string        system_log_file_name_;
  string        tm_model_file_name_;
  string        punct_dict_file_name_;
  string        ending_punct_dict_file_name_;
  string        src_tm_file_;
  string        tgt_tm_file_;

};

}


#ifdef WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

extern "C" DLLEXPORT void*  __init   ( const char* config );
extern "C" DLLEXPORT char*  __do_job ( void* class_handle, const char* msg_text, int print_log, const char* log_head );
extern "C" DLLEXPORT void   __reload ( void* class_handle );
extern "C" DLLEXPORT void   __destroy( void* class_handle );

#endif


