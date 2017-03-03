/*
* $Id:
* 0031
*
* $File:
* recasing.h
*
* $Proj:
* Recasing for Statistical Machine Translation
*
* $Func:
* recasing
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
* 2013-03-16,09:59
*/

#ifndef DECODER_RECASING_H_
#define DECODER_RECASING_H_

#include <vector>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cmath>
#include "configuration.h"
#include "model.h"
#include "test_set.h"
#include "basic_method.h"

using namespace std;
using namespace decoder_configuration;
using namespace model;
using namespace decoder_test_set;
using namespace basic_method;

namespace decoder_recasing {

class Hypothesis {
 public:
  float  feature_score_except_lm_;
  float  language_model_score_;
  string translation_result_;
  bool   compose_flag_;
  int    target_word_count_;
  vector< string > target_word_;
  vector< int >    target_word_id_;
  vector< float >  target_n_gram_lm_score_;

  Hypothesis() {
    feature_score_except_lm_ = 0;
    language_model_score_ = 0;
    target_word_count_ = 0;
    compose_flag_ = false;
  }

  Hypothesis(const float  &feature_score_except_lm, const float &language_model_score, const string &translation_result, \
             const bool &compose_flag, const int    &target_word_count)
            :feature_score_except_lm_(feature_score_except_lm), language_model_score_(language_model_score),
             translation_result_(translation_result), compose_flag_(compose_flag), target_word_count_(target_word_count) {}

  ~Hypothesis() {}
};


class Cells {
 public:
  vector< vector< vector< Hypothesis > > > cells_;

 public:
  Cells() {}
  ~Cells() {}
};


class PosInfoForCubePruning {
 public:
  int front_position_;
  int back_position_;

 public:
  PosInfoForCubePruning() {}
  PosInfoForCubePruning(int &front_position, int &back_position)
                       :front_position_( front_position ), 
                        back_position_( back_position ) {}
};


class Recasing: public BasicMethod {
 public:
  typedef multimap< float, Hypothesis >                      CELLFORCUBEPRUNING;
  typedef vector< vector< vector< Hypothesis > > >::iterator VEC3HYPOIT        ;
  typedef vector< Hypothesis >                               CELL              ;
  typedef vector< int >::const_iterator                      CONINTIT          ;
  typedef vector< string >::const_iterator                   CONSTRIT          ;
  typedef vector< float >::const_iterator                    CONFLOIT          ;

  TestSet test_set_;

 public:
  bool Init( Configuration &config, Model &model );

 public:
  bool Decode( Configuration &config, Model &model );

  bool Decode( Configuration &config, Model &model, SourceSentence &sourceSentence, string &output );

 private:
  bool Decode( Configuration &config, Model &model, Cells &cells, SourceSentence &sourceSentence, ofstream &outFile, ofstream &logFile );

  bool Decode( Configuration &config, Model &model, Cells &cells, SourceSentence &sourceSentence, string &output );

 private:
  bool DecodeWithCYKAlgorithm( Configuration &config, Model &model, Cells &cells, SourceSentence &sourceSentence );

  bool DecodeWithLeftToRight( Configuration &config, Model &model, Cells &cells, SourceSentence &sourceSentence );

private:
  bool AllocateSpaceForCells( Cells &cells, SourceSentence &sourceSentence );

 private:
  bool InitCells( Configuration &config, Model &model, Cells &cells, SourceSentence &sourceSentence );

 private:
  bool CalculateLMScore( Configuration &config, Model &model, Hypothesis &hy );

  bool CalculateLMScoreForComposing( const Configuration &config, Model &model, const Hypothesis &hyPrev, \
                                     const Hypothesis &hyFoll, Hypothesis &hy );

 private:
  bool SetBeamSize( const Configuration &config, CELLFORCUBEPRUNING &cellForCubePruning, CELL &cell );

 private:
  bool Compose( const Configuration &config, Model &model, CELL &prevCell, CELL &follCell, \
                const size_t &startPos, const size_t &segPos, const size_t &endPos, \
                CELLFORCUBEPRUNING &cellForCubePruning, const SourceSentence &sourceSentence );  

  bool ComposeNaive( const Configuration &config, Model &model, CELL &prevCell, CELL &follCell, \
                     const size_t &startPos, const size_t &segPos, const size_t &endPos, \
                     CELLFORCUBEPRUNING &cellForCubePruning, const SourceSentence &sourceSentence );

  bool ComposeWithCubePruning( const Configuration &config, Model &model, CELL &prevCell, CELL &follCell, \
                               const size_t &startPos, const size_t &segPos, const size_t &endPos, \
                               CELLFORCUBEPRUNING &cellForCubePruning, const SourceSentence &sourceSentence );

 private:
  float ExpandHypotheses( const Configuration &config, Model &model, const size_t &startPos, const size_t &segPos, \
                          const size_t &endPos, CELLFORCUBEPRUNING &cellForCubePruning, const SourceSentence &sourceSentence, \
                          const Hypothesis &prevHypo, const Hypothesis &follHypo , int &count );

 private:
  bool BuildNewHypothesis( const Hypothesis &hyPrev, const Hypothesis &hyFoll, Hypothesis &hy );

 private:
  bool CheckForPunctPruning( const Configuration &config, const size_t &startPos, const size_t &endPos, const SourceSentence &sourceSentence );

  bool HavePunct( const int &startPos, const int &endPos, const SourceSentence &sourceSentence );

 private:
  bool GenerateTranslationResult( const Configuration &config, const Cells &cells, const SourceSentence &sourceSentence, string &output );

  bool OutputTranslationResult( const Configuration &config, const Cells &cells, const SourceSentence &sourceSentence, ofstream &outFile, ofstream &logFile );

  bool WriteOutputFile( const Configuration &config , const Cells &cells, const SourceSentence &sourceSentence, ofstream &outFile );

 public:
  bool UppercaseSentenceBegin( string &str );

 public:
  static bool PrintRecasingLogo();
};

}


#endif


