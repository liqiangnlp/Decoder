/*
 * $Id:
 * 0011
 *
 * $File:
 * language_model.h
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
 * 2012-12-10,14:41
 */

#ifndef DECODER_LANGUAGE_MODEL_H_
#define DECODER_LANGUAGE_MODEL_H_

#include "NiuLM.h"
#include <string>
#include <ctime>
#include <iostream>
#include <map>
#include <vector>
#include "configuration.h"
#include "debug.h"

using namespace niu_lm;
using namespace std;
using namespace decoder_configuration;
using namespace debug;


namespace language_model {
#define MAX_WORD_LENGTH 1024

class LanguageModel {
 private:
  int lm_index_;

 public:
  LanguageModel() {};
  ~LanguageModel();

 public:
  void LoadModel (Configuration &config);                        // Load ngram language model
  float GetProb (vector<int> &wid, int &begin, int &end);        // Get n-gram probability, i.e. Pr(w_n|w_1...w_n-1)
  void UnloadModel ();                                           // Unload language model
};

}


#endif


