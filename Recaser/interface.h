/*
* $Id:
* 0003
*
* $File:
* interface.h
*
* $Proj:
* RecaserLib for Statistical Machine Translation
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

#ifndef RECASERLIB_INTERFACE_H_
#define RECASERLIB_INTERFACE_H_

#include <iostream>
#include <string>
#include <cstring>
#ifdef _WIN32
#include "../Decoder/recasing.h"
#include "../Decoder/configuration.h"
#include "../Decoder/model.h"
#include "../Decoder/test_set.h"
#else
#include "recasing.h"
#include "configuration.h"
#include "model.h"
#include "test_set.h"
#endif

#ifndef WIN32
#include <sys/time.h>
#endif

using namespace std;
using namespace decoder_recasing;
using namespace decoder_configuration;
using namespace model;
using namespace decoder_test_set;

namespace recaser_interface {

class RecasingInterface {

 public:
  RecasingInterface(){}
  ~RecasingInterface(){}

 public:
  Configuration config_recasing_;
  Model model_recasing_;

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


