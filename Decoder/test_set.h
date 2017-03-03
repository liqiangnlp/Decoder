/*
* $Id:
* 0023
*
* $File:
* test_set.h
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
* 2014-01-07,21:00, English string recognition and translation.
* 2013-03-15,20:11
* 2012-12-13,14:31
*/


#ifndef DECODER_TEST_SET_H_
#define DECODER_TEST_SET_H_


#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <ctime>
#include <utility>
#include <map>
#include <cmath>
#include "debug.h"
#include "basic_method.h"
#include "me_reordering_table.h"
#include "context_sensitive_wd.h"
#include "configuration.h"


using namespace std;
using namespace debug; 
using namespace basic_method;
using namespace decoder_me_reordering_table ;
using namespace decoder_context_sensitive_wd;
using namespace decoder_configuration;


namespace decoder_test_set{

class GeneralizationInfo {
 public:
  size_t start_position_;
  size_t end_position_;
  string translation_result_;
  string symbol_;
  size_t symbol_type_;       // 0 equal to 'basic', i.e. $number, $date, $time etc; 1 equal to 'forced' type, i.e. $literal; 2 equal to normal, i.e. '$person';

 public:
  GeneralizationInfo() {}
  GeneralizationInfo(size_t &start_position, size_t &end_position, string &translation_result, string &symbol, size_t &symbol_type)
                    :start_position_(start_position), end_position_(end_position), translation_result_(translation_result), \
                     symbol_(symbol), symbol_type_(symbol_type) {}
  ~GeneralizationInfo(){}
};


class SourceSentence: public BasicMethod {
 public:
  typedef multimap< pair< size_t, size_t >, GeneralizationInfo >::iterator GENEINFOIT;

  vector< string >          cell_span_;
  set< int >                punctuation_position_;
  vector< vector< float > > me_reorder_scores_;
  // 0=straight, 1=inverted
  // weights.at(0)=0:SLL
  // weights.at(1)=0:SLR
  // weights.at(2)=0:SRL
  // weights.at(3)=0:SRR
  // weights.at(4)=1:SLL
  // weights.at(5)=1:SLR
  // weights.at(6)=1:SRL
  // weights.at(7)=1:SRR
  vector<pair<float, float> > context_sensitive_wd_scores_;   // 0=unspurious; 1=spurious

  multimap< pair< size_t, size_t >, GeneralizationInfo > generalization_informations_;

  map<string, vector<float> > mert_candidate_;
  map<string, vector<float> > mert_candidate_current_;

  int sentence_id_;

 public:
  SourceSentence () : sentence_id_(0) {}
  ~SourceSentence () {}

 public:
  bool Init (string &source_string);
  bool Init (string &source_string, MeReorderingTable &me_reordering_table);
  bool Init (string &source_string, MeReorderingTable &me_reordering_table, int &sentence_id);
  bool Init (string &source_string, MeReorderingTable &me_reordering_table, ContextSensitiveWd &context_sensitive_model, int &sentence_id);

 private:
  bool InitMeReorderingScores (MeReorderingTable &meReorderingTable);
  bool InitContextSensitiveWdScores (ContextSensitiveWd &context_sensitive_model);

 public:
  bool SetMertCandidate (const Configuration &config, const vector<float> &v_feature_value, string translation);
  bool SetMertCandidate (const Configuration &config, const vector<float> &v_feature_value, const float &context_sentence_wd_value, string &translation);
  bool SetMertCandidateCurrent (const Configuration &config, const vector<float> &v_feature_value, const vector<float> &v_context_sentence_wd_value, string &translation);

 private:
  bool ParseGeneralizationInfo (string &info);
};


class TestSet: public BasicMethod {
 public:
  vector< SourceSentence > test_set_;

 public:
  TestSet () {}
  ~TestSet () {}

 public:
  bool Init (Configuration &config);
  bool Init (Configuration &config, MeReorderingTable &me_reordering_table);
  bool Init (Configuration &config, MeReorderingTable &me_reordering_table, ContextSensitiveWd &context_sensitive_model);

 private:
  bool LoadTestSet (Configuration &config);
  bool LoadTestSet (Configuration &config, MeReorderingTable &me_reordering_table );
  bool LoadTestSet (Configuration &config, MeReorderingTable &me_reordering_table, ContextSensitiveWd &context_sensitive_model);

 public:
  bool RecognizeAndTranslateEnString (string &input_sentence);

 private:
  bool IsEnglishWord (string &input_word);
};

}


#endif

