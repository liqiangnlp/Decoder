/*
* $Id:
* 0003
*
* $File:
* interface.h
*
* $Proj:
* DetokenLib for Statistical Machine Translation
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

#ifndef DETOKENLIB_INTERFACE_H_
#define DETOKENLIB_INTERFACE_H_

#include <iostream>
#include <string>
#include <cstring>
#ifdef _WIN32
#include "../Decoder/detokenizer.h"
#else
#include "detokenizer.h"
#endif
using namespace std;
using namespace decoder_detokenizer;

#define SUPPORT_ONLINE_SERVICE_CE_
//#define SUPPORT_ONLINE_SERVICE_EC_


namespace detoken_interface {

class DetokenInterface {

 public:
  DetokenInterface(){}
  ~DetokenInterface(){}

#ifdef SUPPORT_ONLINE_SERVICE_EC_
 public:
  PunctuationMap punctuation_map_;
#endif

 public:
  string punct_mapping_dict_file_name_;
 public:
  string system_log_file_name_;
  ofstream system_log_;
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


