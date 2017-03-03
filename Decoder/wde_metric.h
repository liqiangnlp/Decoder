/*
* $Id:
* 0051
*
* $File:
* wde_metric.h
*
* $Proj:
* Decoder for Statistical Machine Translation
*
* $Func:
* Calculate WDE Metric Score
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
* 2015-01-20,14:05, in Beijing
*/

#ifndef DECODER_WDE_METRIC_H_
#define DECODER_WDE_METRIC_H_

#include <string>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <map>
#include <vector>
#include <cmath>
#include "debug.h"
#include "basic_method.h"

using namespace std;
using namespace basic_method;
using namespace debug;


namespace decoder_wde_metric {

class MatchInformation {
 public:
  string            sentence_id_             ;
  int               current_reference_length_;
  float             wdem_score_;
  vector< size_t >  match_count_             ;
  vector< size_t >  reference_count_         ;
  vector< size_t >  translation_count_       ;
  vector< float >   ibm_bleu_score_;


 public:
  MatchInformation() : current_reference_length_( 0 ) {};
  ~MatchInformation(){};

 public:
  bool Initialize( int &max_ngram, string &sentence_id );
};


class WordDeletionErrorMetric : public BasicMethod {
 public:
  WordDeletionErrorMetric() : total_words_number_(0), max_ngram_(1), sentences_number_(0) {};
  ~WordDeletionErrorMetric(){};

 public:
  int    references_number_            ;
  int    sentences_number_             ;
  string translation_results_file_name_;
  string src_and_ref_file_name_        ;   // file name for source sentences and references
  string output_file_name_             ;
  string stopword_file_name_           ;

 public:
  int    total_words_number_;

 public:
  int    max_ngram_;

 private:
  vector< vector< string > >  references_data_;             // references_data_[ reference_id ][ sentence_id ]
  vector< string >            source_data_;
  vector< string >            translation_results_data_;
  set<string>                 stopword_set_;

 public:
  bool Process( map< string, string > &parameters );

 private:
  bool Initialize( map< string, string > &parameters );

 private:
  bool PrintConfiguration();

 private:
  bool CheckFiles( map< string, string > &parameters );

 private:
  bool CheckFile( map< string, string > &parameters, string &file_key );

 private:
  bool CheckEachParameter( map< string, string > &parameters, string &parameter_key, string &default_value );

 private:
  bool LoadingData();

 private:
  bool LoadingStopword();

 private:
  bool RemoveStopword(string &sentence);

 private:
  bool Tokenization( string &input_string, string &output_string );

 private:
  bool ConvertSgmlTags( string &input_string, string &output_string );

 private:
  bool TokenizePunctuation( string &input_string, string &output_string );
  bool TokenizePeriodAndComma( string &input_string, string &output_string );
  bool TokenizeDash( string &input_string, string &output_string );

 private:
  bool Words2Ngrams( vector< string > &words, map< string, int > &ngrams_count );

 private:
  bool ScoreSystem();
  bool ScoreSegment( size_t &sentence_id, MatchInformation &match_information );
  bool CalculateWdeMetricSmoothing(MatchInformation &match_information, ofstream &output_file);

 public:
  static bool PrintWdeMetricLogo();

};

}


#endif
