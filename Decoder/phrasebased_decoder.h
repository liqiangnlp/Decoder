/*
* $Id:
* 0009
*
* $File:
* phrasebased_decoder.h
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
* 2013-04-23,10:26
* 2013-03-15,16:20
* 2013-03-06,15:09
* 2013-03-04,20:34
* 2012-12-04,19:19
*/

#ifndef DECODER_PHRASEBASED_DECODER_H_
#define DECODER_PHRASEBASED_DECODER_H_

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <ctime>
#include <set>
#include <cmath>
#include "basic_method.h"
#include "configuration.h"
#include "model.h"
#include "test_set.h"

using namespace std;
using namespace decoder_configuration;
using namespace basic_method;
using namespace decoder_test_set;
using namespace model;


namespace decoder_phrasebased_decoder {

class TranslationLog {
 public:
  size_t start_position_;
  size_t end_position_;
  string translation_result_;

  TranslationLog() {}
  TranslationLog(const size_t &start_position, const size_t &end_position, const string &translation_result) \
                : start_position_(start_position), end_position_(end_position), translation_result_(translation_result) {}

  ~TranslationLog() {}
};


class Hypothesis {
 public:
  float                    feature_score_except_lm_;
  float                    language_model_score_;
  string                   translation_result_;
  bool                     compose_flag_;
  int                      target_word_count_;
  float                    me_reordering_score_mono_;
  float                    me_reordering_score_swap_;
  vector< string >         target_word_;
  vector< int >            target_word_id_;
  vector< float >          target_n_gram_lm_score_;
  vector< TranslationLog > translation_log_;
  vector< float >          feature_values_;
  vector< float >          me_reorder_scores_;
  /*  0=straight, 1=inverted
      weights.at(0)=0:TLL
      weights.at(1)=0:TLR
      weights.at(2)=0:TRL
      weights.at(3)=0:TRR
      weights.at(4)=1:TLL
      weights.at(5)=1:TLR
      weights.at(6)=1:TRL
      weights.at(7)=1:TRR  */

//  float                    context_sensitive_wd_values_;
  vector<float>            v_context_sensitive_wd_values_;

 public:
  Hypothesis() {
    feature_score_except_lm_ = 0;
    language_model_score_ = 0;
    target_word_count_ = 0;
    compose_flag_ = false;

    v_context_sensitive_wd_values_.resize(2, 0.0f);

//    context_sensitive_wd_values_ = 0;
  }

  Hypothesis(const float &feature_score_except_lm, const float &language_model_score, const string &translation_result, \
             const bool &compose_flag, const int &target_word_count) 
            : feature_score_except_lm_(feature_score_except_lm), language_model_score_(language_model_score), translation_result_(translation_result), \
              compose_flag_(compose_flag), target_word_count_(target_word_count) {
    v_context_sensitive_wd_values_.resize(2, 0.0f);
  }
  
  ~Hypothesis() {}
};


class Cells {
 public:
  vector< vector< vector< Hypothesis > > > cells;

 public:
  Cells() {}
  ~Cells() {}
};


class PosInfoForCubePruning {
 public:
  int front_position_;
  int back_position_;

  PosInfoForCubePruning() {}
  PosInfoForCubePruning(int &front_position, int &back_position)
                       : front_position_(front_position), back_position_(back_position) {}
};


class PosInfoForCubePruningInc {
 public:
  int split_position_;
  int front_position_;
  int back_position_;

  PosInfoForCubePruningInc() {}
  PosInfoForCubePruningInc(int split_position, int &front_position, int &back_position)
                          : split_position_(split_position), front_position_(front_position), back_position_(back_position) {}
};


class PhraseBasedITGDecoder: public BasicMethod {
 public:
  typedef multimap< pair< size_t, size_t >, GeneralizationInfo >::iterator       GENEINFOIT        ;
  typedef multimap< pair< size_t, size_t >, GeneralizationInfo >::const_iterator GENEINFOIT_CONST  ;
  typedef vector< Hypothesis >                                                   CELL              ;
  typedef vector< Hypothesis >::iterator                                         HYPOTHESESIT      ;
  typedef multimap< float, Hypothesis >                                          CELLFORCUBEPRUNING;
  typedef map< string, string >                                                  STRPAIR           ;
  typedef vector< int >::const_iterator                                          CONINTIT          ;
  typedef vector< string >::const_iterator                                       CONSTRIT          ;
  typedef vector< float >::const_iterator                                        CONFLOIT          ;
  typedef vector< TranslationLog >::const_iterator                               CONTRAIT          ;
  typedef vector< vector< vector< Hypothesis > > >::iterator                     VEC3HYPOIT        ;

  PhraseBasedITGDecoder() {}
  ~PhraseBasedITGDecoder() {}

 private:
  TestSet test_set_;

 public:
  bool InterfaceForTranslationMemory(Configuration &config, Model &model, string &translation_memory_output, string &decoder_output);

 public:
  bool Init(Configuration &config, Model &model);

 public:
  bool Decode(Configuration &config, Model &model);

  bool Decode(Configuration &config, Model &model, SourceSentence &source_sentence, string &output);

 private:
  bool Decode(Configuration &config, Model &model, Cells &cells, SourceSentence &source_sentence, ofstream &out_file, ofstream &log_file);

  bool Decode(Configuration &config, Model &model, Cells &cells, SourceSentence &source_sentence, string &output);

 private:
  bool DecodeWithCYKAlgorithm(Configuration &config, Model &model, Cells &cells, SourceSentence &source_sentence);

 public:
  bool GenerateTranslationResults(Configuration &config, Model &model, multimap< float, vector< float> > &bleu_and_scores);

 public:
  bool GetFeatureWeight(Configuration &config, string &file_name, multimap< float, vector< float > > &bleu_and_scores);

 private:
  bool AllocateSpaceForCells(Cells &cells, SourceSentence &source_sentence);

 private:
  bool InitCells(Configuration &config, Model &model, Cells &cells, SourceSentence &source_sentence);

  bool InitCellsNormal(Configuration &config, Model &model, Cells &cells, SourceSentence &source_sentence);

  bool InitCellsBinary(Configuration &config, Model &model, Cells &cells, SourceSentence &source_sentence);

 private:
  bool InitCellsWithGeneralization(Configuration &config, Model &model, Cells &cells, SourceSentence &source_sentence);

 private:
  bool ClearCellsWithForcedType(Configuration &config, Model &model, Cells &cells, SourceSentence &source_sentence);

 private:
  bool SetBeamSize(const Configuration &config, CELLFORCUBEPRUNING &cell_for_cube_pruning, CELL &cell);

 private:
  bool Compose(const Configuration &config, Model &model, CELL &front_cell, CELL &back_cell,  \
               const size_t &start_position, const size_t &split_position, const size_t &end_position, \
               CELLFORCUBEPRUNING &cell_for_cube_pruning, const bool &swap_flag, const SourceSentence &source_sentence);

  bool ComposeNaive(const Configuration &config, Model &model, CELL &front_cell, CELL &back_cell, \
                    const size_t &start_position, const size_t &split_position, const size_t &end_position, \
                    CELLFORCUBEPRUNING &cell_for_cube_pruning, const bool &swap_flag, const SourceSentence &source_sentence);

  bool ComposeWithCubePruning(const Configuration &config, Model &model, CELL &front_cell, CELL &back_cell, \
                              const size_t &start_position, const size_t &split_position, const size_t &end_position, \
                              CELLFORCUBEPRUNING &cell_for_cube_pruning, const bool &swap_flag, const SourceSentence &source_sentence);

  bool ComposeWithCubePruningUpgrade(const Configuration &config, Model &model, const Cells &cells, \
                                     const size_t &start_position, const size_t &end_position, \
                                     CELLFORCUBEPRUNING &cell_for_cube_pruning, const SourceSentence &source_sentence);

 private:
  float ExpandHypotheses(const Configuration &config, Model &model, \
                         const size_t &start_position, const size_t &split_position, const size_t &end_position, \
                         CELLFORCUBEPRUNING &cell_for_cube_pruning, const bool &swap_flag, const SourceSentence &source_sentence, \
                         const Hypothesis &front_hypothesis, const Hypothesis &back_hypothesis, int &count);

 private:
  bool CheckForPunctPruning(const Configuration &config, const size_t &start_position, const size_t &end_position, const SourceSentence &source_sentence);

  bool HavePunct(const int &start_position, const int &end_position, const SourceSentence &source_sentence);

 private:
  bool CanBeReversed( const Configuration &config, const size_t &start_position, const size_t &split_position, const size_t &end_position, const size_t &current_sent_length, const SourceSentence &source_sentence);

 private:
  bool BuildNewHypothesis(const Hypothesis &front_hypothesis, const Hypothesis &back_hypothesis, Hypothesis &hypothesis);

 private:
  bool CalculateLMScore(Configuration &config, Model &model, Hypothesis &hypothesis);

  bool CalculateLMScoreForComposing(const Configuration &config, Model &model, const Hypothesis &front_hypothesis, const Hypothesis &back_hypothesis, Hypothesis &hy);

 private:
  bool CalculateMEReorderScore(const Configuration &config, const size_t &start_position, const size_t &split_position, \
                               const size_t &end_position, const SourceSentence &source_sentence, const Hypothesis &front_hypothesis, \
                               const Hypothesis &back_hypothesis, Hypothesis &hypothesis );

  bool SaveMEReorderValue(const Hypothesis &front_hypothesis, const Hypothesis &back_hypothesis, Hypothesis &hypothesis);

 private:
  bool OutputTranslationResults(const Configuration &config, const Cells &cells, SourceSentence &source_sentence, ofstream &outFile, ofstream &logFile);

 private:
  bool WriteOutputtedFile(const Configuration &config, const Cells &cells, SourceSentence &source_sentence, ofstream &outFile);

  bool WriteLogFile(const Cells &cells, const SourceSentence &source_sentence, ofstream &logFile);

 private:
  bool GenerateTranslationResults(const Configuration &config, const Cells &cells, const SourceSentence &source_sentence, string &output);

 private:
  bool WriteMertConfig(Configuration &config);
  bool WriteMertTmp (const Configuration &config, vector<SourceSentence> &test_set);
  bool WriteMertTmpOld(const Configuration &config, const Hypothesis &hypothesis, string &translation, ofstream &out_file);

 public:
  static bool PrintPhraseBasedDecoderLogo();
  static bool PrintPhraseBasedDecoderBinaLogo();
  static bool PrintPhraseBasedTrainingLogo();
};

}

#endif

