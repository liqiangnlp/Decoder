/*
* $Id:
* 0029
*
* $File:
* configuration.h
*
* $Proj:
* Decoder for Statistical Machine Translation
*
* $Func:
* config
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
* 2013-03-14,10:12
*/

#ifndef DECODER_CONFIGURATION_H_
#define DECODER_CONFIGURATION_H_

#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <iomanip>
#include "basic_method.h"

using namespace std;
using namespace basic_method;


namespace decoder_configuration {

class FeatureValue {
 public:
  float weight_;
  float min_value_;
  float max_value_;
  float fixed_;

 public:
  FeatureValue () {};
  FeatureValue (const float &weight, const float &min_value, const float &max_value, const float &fixed)
              : weight_(weight), min_value_(min_value), max_value_(max_value), fixed_(fixed) {};
};


class Features: public BasicMethod {
 public:
  typedef map< string, string > STRPAIR;

 public:
  size_t features_number_;
  vector<FeatureValue> feature_values_;

  size_t cswd_feature_number_;
  vector<FeatureValue>  v_context_sensitive_wd_feature_;

 public:
  Features (): features_number_(0), cswd_feature_number_(0) {}

 public:
  bool ParsingFeatures (STRPAIR &param);
  bool ParsingFeatures(string &in_file_name);
  bool ParsingContextSensitiveFeatures (STRPAIR &parameter);
};


class Configuration {
 public:
  typedef map< string, string > STRPAIR;

 public:
  int      nround_;
  int      ngram_;
  int      beamsize_;
  int      nbest_;
  int      nref_;
  int      max_reordering_distance_;
  int      max_phrase_length_;
  int      nthread_;
  int      free_feature_;

  bool     use_punct_pruning_;
  bool     use_cube_pruning_;
  bool     use_cube_pruninginc_;
  bool     use_me_reorder_;
  bool     use_msd_reorder_;

  bool     use_empty_translation_;
  bool     use_context_sensitive_wd_;

  bool     output_oov_;
  bool     output_empty_translation_;
  bool     label_oov_;
  bool     mert_flag_;
  bool     recase_flag_;
  bool     convert_pt_2_binary_flag_;
  bool     phrase_table_binary_flag_;
  bool     support_service_flag_;

  bool     english_string_;          // for Chinese-2-English translation

  string   recase_method_;

  string   config_file_;
  string   output_file_;
  string   log_file_;

  string   mert_tmp_file_;
  string   mert_log_file_;
  string   mert_config_file_;

  string   lm_file_;
  string   target_vocab_file_;
  string   phrase_table_file_;
  string   source_phrase_file_;
  string   target_phrase_file_;
  string   metabFile        ;
  string   msdtabFile       ;
  string   testFile         ;
  string   context_sensitive_model_;

  Features features         ;

 public:
  Configuration() {
    use_punct_pruning_        = false;
    use_cube_pruning_         = false;
    use_cube_pruninginc_      = false;
    use_me_reorder_           = false;
    use_msd_reorder_          = false;
    use_empty_translation_    = false;
    use_context_sensitive_wd_ = false;
    output_empty_translation_ = false;
    output_oov_               = false;
    label_oov_                = false;
    mert_flag_                = false;
    recase_flag_              = false;
    convert_pt_2_binary_flag_ = false;
    phrase_table_binary_flag_ = false;
    support_service_flag_     = false;
    english_string_           = false;      // for Chinese-2-English translation
  }
  ~Configuration () {}

 public:
  bool Init (STRPAIR &param);
  bool CheckParamsInConf (STRPAIR &param);
  bool CheckEachParamInConf (STRPAIR &param, string &paramKey, string &defValue);
  bool CheckFilesInConf (STRPAIR &param);
  bool checkFileInConf (STRPAIR &param, string &fileKey);
  bool checkFileInComm (STRPAIR &param, string &fileKey);
  bool PrintConfig ();
  bool PrintFeatures ();
};


}

#endif
