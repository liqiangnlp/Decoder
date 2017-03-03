/*
* $Id:
* 0012
*
* $File:
* language_model.cpp
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
* 2012-12-10,14:42
*/

#include "language_model.h"


namespace language_model {

LanguageModel::~LanguageModel() {
  UnloadModel(); 
}


void LanguageModel::LoadModel (Configuration &config) {
  cerr<<"Loading Ngram-LanguageModel-File\n"<<"  >> From File: "<<config.lm_file_<<"\n"<<flush;
  clock_t start, finish;
  start = clock();
  lm_index_ = g_initlm (config.lm_file_.c_str(), config.target_vocab_file_.c_str(), -1, 0);
  finish = clock();
  cerr<<"  Done! [Time:"<<(double)(finish - start)/CLOCKS_PER_SEC<<" s]\n\n"<<flush;
}


void LanguageModel::UnloadModel () {
  g_unloadlm (lm_index_);
}


float LanguageModel::GetProb (vector< int > &wid, int &begin, int &end) {
  int wordId[MAX_WORD_LENGTH];
  int pos = 0;
  for (vector<int>::iterator iter = wid.begin(); iter != wid.end(); ++iter) {
    wordId[pos] = *iter;
    ++pos;
  }
  wordId[pos] = '\0';
  return g_get_ngram_prob2 (lm_index_, wordId, begin, end, -1);
}

}

