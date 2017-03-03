/*
* $Id:
* 0019
*
* $File:
* msd_reordering_table.h
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
* 2012-12-12,14:21
*/

#ifndef DECODER_MSD_REORDERING_TABLE_H_
#define DECODER_MSD_REORDERING_TABLE_H_

#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <ctime>
#include "basic_method.h"
#include "configuration.h"
#include "debug.h"

using namespace std;
using namespace basic_method;
using namespace decoder_configuration;
using namespace debug;

namespace msd_reordering_table {

class ScoresOfMsdReorderOption {
 public:
  double prevMono;    // MSD reordering model: Previous  & Monotonic
  double prevSwap;    // MSD reordering model: Previous  & Swap
  double prevDisc;    // MSD reordering model: Previous  & Discontinuous
  double follMono;    // MSD reordering model: Following & Monotonic
  double follSwap;    // MSD reordering model: Following & Swap
  double follDisc;    // MSD reordering model: Following & Discontinuous

 public:
  ScoresOfMsdReorderOption(){}
  ScoresOfMsdReorderOption(double &pM, double &pS, double &pD, double &fM, double &fS, double &fD)
                          : prevMono(pM), prevSwap(pS), prevDisc(pD), follMono(fM), follSwap(fS), follDisc(fD) {}
  ~ScoresOfMsdReorderOption(){}
};


class MsdReorderOption {
 public:
  string                   tgtOption               ;
  ScoresOfMsdReorderOption scoresOfMsdReorderOption;

 public:
  MsdReorderOption(){}
  MsdReorderOption(string &tOpt, ScoresOfMsdReorderOption &scOfMsdReOpt) {
    tgtOption                = tOpt        ;
    scoresOfMsdReorderOption = scOfMsdReOpt;
  }
  ~MsdReorderOption(){}
};


class MsdReorderOptions {
 public:
  vector< MsdReorderOption > msdReorderOption;
};


class MsdReorderingTable: public BasicMethod {
 public:
  MsdReorderingTable (){}
  ~MsdReorderingTable(){}

 public:
  bool init( Configuration &config );

 public:
  map< string, MsdReorderOptions > msdReordersOptions;
};

}

#endif


