/*
* $Id:
* 0057
*
* $File:
* language_model_score.h
*
* $Proj:
* Decoder for Statistical Machine Translation
*
* $Func:
* Calculate N-gram LM score
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
* 11/03/2015,09:43, Los Angeles, USA
*/


#ifndef DECODER_LANGUAGE_MODEL_SCORE_H_
#define DECODER_LANGUAGE_MODEL_SCORE_H_

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
#include <cmath>
#include "language_model.h"
#include "configuration.h"
#include "basic_method.h"


using namespace std;
using namespace language_model;
using namespace decoder_configuration;
using namespace basic_method;


namespace language_model_score {


 class LanguageModelScore : public BasicMethod{
 public:
  LanguageModel ngram_language_model_;
  map<string, int> m_target_vocab_;

 public:
  string lmbin_file_;
  string vocab_file_;
  int    ngram_;

  string input_file_;
  string output_file_;


 public:
  LanguageModelScore() {
    lmbin_file_ = "";
    vocab_file_ = "";
    ngram_ = 5;
    input_file_ = "";
    output_file_ = "";
  }
  ~LanguageModelScore() {}

 public:
  bool Initialize(map<string, string> &parameters);

 public:
  bool Process();

 private:
  float CalculateLMScore(string &line);

 private:
  bool CheckFiles(map<string, string> &parameters);
  bool CheckFile(map<string, string> &parameters, string &file_key);

 private:
  bool CheckEachParameter(map<string, string> &parameters, string &parameter_key, string &default_value);

 private:
  bool InitTgtVocab(Configuration &config);

 public:
  static bool PrintLMScoreLogo();
};

}

#endif

