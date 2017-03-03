/*
* $Id:
* 0053
*
* $File:
* context_sensitive_wd.h
*
* $Proj:
* Decoder for Statistical Machine Translation
*
* $Func:
* decoder
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
* 2015-07-04,14:19
*/

#ifndef DECODER_CONTEXT_SENSITIVE_WD_H_
#define DECODER_CONTEXT_SENSITIVE_WD_H_

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <ctime>
#include "basic_method.h"
#include "configuration.h"
#include "debug.h"

using namespace std;
using namespace debug;
using namespace basic_method;
using namespace decoder_configuration;

namespace decoder_context_sensitive_wd {

class ContextSensitiveWd: public BasicMethod {
 public:
  map<string, vector<float> > context_sensitive_wd_src_;
  // 0=unspurious, 1=spurious
  // weights.at(0)  =0:w-2
  // weights.at(1)  =0:w-1
  // weights.at(2)  =0:w
  // weights.at(3)  =0:w+1
  // weights.at(4)  =0:w+2
  // weights.at(5)  =0:p-2
  // weights.at(6)  =0:p-1
  // weights.at(7)  =0:p
  // weights.at(8)  =0:p+1
  // weights.at(9)  =0:p+2
  // weights.at(10) =0:p-2_p-1
  // weights.at(11) =0:p-1_p
  // weights.at(12) =0:p_p+1
  // weights.at(13) =0:p+1_p+2
  // weights.at(14) =1:w-2
  // weights.at(15) =1:w-1
  // weights.at(16) =1:w
  // weights.at(17) =1:w+1
  // weights.at(18) =1:w+2
  // weights.at(19) =1:p-2
  // weights.at(20) =1:p-1
  // weights.at(21) =1:p
  // weights.at(22) =1:p+1
  // weights.at(23) =1:p+2
  // weights.at(24) =1:p-2_p-1
  // weights.at(25) =1:p-1_p
  // weights.at(26) =1:p_p+1
  // weights.at(27) =1:p+1_p+2




 public:
  ContextSensitiveWd () {}
  ~ContextSensitiveWd () {}

 public:
  bool Init (Configuration &config);

 private:
  bool InitContextSensitiveWd (Configuration &config);
  bool LoadFeature (vector<string> &meTab, size_t &lineNo);
};

}

#endif

