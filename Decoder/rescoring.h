/*
* $Id:
* 0007
*
* $File:
* rescoring.h
*
* $Proj:
* Decoder for Statistical Machine Translation
*
* $Func:
* rescoring
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
* 2015-10-28, 20:36, in USA
*/


#ifndef DECODER_RESCORING_H_
#define DECODER_RESCORING_H_

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <set>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <ctime>
#include <map>
#include "basic_method.h"
#include "configuration.h"
#include "ibm_bleu_score.h"
using namespace std;
using namespace basic_method;
using namespace decoder_configuration;
using namespace decoder_ibm_bleu_score;


namespace decoder_rescoring {

class Rescoring : public BasicMethod {
 public:
  string dev_file_;
  string dev_nbest_file_;
  string initial_weights_file_;
  string test_file_;
  string test_nbest_file_;
  int n_ref_;
  int n_round_;

 public:
  vector<vector<pair<string, vector<float> > > > vv_dev_nbest_;
  vector<vector<pair<string, vector<float> > > > vv_test_nbest_;

 public:
  Rescoring() {
    dev_file_ = "";
    dev_nbest_file_ = "";
    initial_weights_file_ = "";
    test_file_ = "";
    test_nbest_file_ = "";
    n_ref_ = 4;
    n_round_ = 2;
  };
  ~Rescoring() {};

 public:
  bool Initialize(map<string, string> &parameters);

 private:
  bool CheckEachParameter(map<string, string> &parameters, string &parameter_key, string &default_value);

 public:
  bool SortDevNBest(Configuration &config, int &round);
  bool SortTestNBest(Configuration &config, int &round);
  bool GetFeatureWeight(Configuration &config, string &fn, multimap<float, vector<float> > &bleuAndScores);
  bool LoadDevNBest(Configuration &config);
  bool LoadTestNBest(Configuration &config);

 private:
  bool CheckFiles(map<string, string> &parameters);
  bool CheckFile(map<string, string> &parameters, string &file_key);

 public:
  static bool PrintRescoringLogo();
};



}

#endif

