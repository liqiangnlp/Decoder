/*
* $Id:
* 0017
*
* $File:
* me_reordering_table.h
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
* 2012-12-12,14:19
*/

#ifndef DECODER_ME_REORDERING_TABLE_H_
#define DECODER_ME_REORDERING_TABLE_H_

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

namespace decoder_me_reordering_table {

class MeReorderingTable: public BasicMethod {
 public:
  map< string, vector< float > > meTableSrc;
  map< string, vector< float > > meTableTgt;
  // 0=straight, 1=inverted
  // weights.at(0)=0:S(T)LL
  // weights.at(1)=0:S(T)LR
  // weights.at(2)=0:S(T)RL
  // weights.at(3)=0:S(T)RR
  // weights.at(4)=1:S(T)LL
  // weights.at(5)=1:S(T)LR
  // weights.at(6)=1:S(T)RL
  // weights.at(7)=1:S(T)RR

 public:
  MeReorderingTable() {}
  ~MeReorderingTable() {}

 public:
  bool init(Configuration &config);

 private:
  bool initMeReorderingTable(Configuration &config);
  bool loadFeature(vector<string> &meTab, size_t &lineNo);
};

}

#endif

