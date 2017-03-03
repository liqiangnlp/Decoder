/*
 * Copyright 2013 Qiang Li.
 *
 * $Id:
 * 0046
 *
 * $File:
 * tree2tree_decoder.h
 *
 * $Proj:
 * Decoder
 *
 * $Func:
 * Tree 2 Tree Decoder
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
 * 2014-10-23
 */

#include "tree2tree_decoder.h"

namespace decoder_tree2tree_decoder {


bool Tree2TreeFeatures::ParsingFeatures(map<string, string> &parameters) {
  vector<string> weights_vector;
  vector<string> ranges_vector;
  vector<string> fixedfs_vector;
  Split(parameters[ "weights" ], ' ', weights_vector);
  Split(parameters[ "ranges"  ], ' ', ranges_vector);
  Split(parameters[ "fixedfs" ], ' ', fixedfs_vector);
  features_number_ = weights_vector.size();

  if (features_number_ != ranges_vector.size() || features_number_ != fixedfs_vector.size()) {
    cerr<<"ERROR: Values number of 'weights', 'ranges', 'fixedfs' in config file is not the same!\n"<<flush;
    exit(1);
  }

  size_t pos = 0;
  for (vector<string>::iterator iter = weights_vector.begin(); iter != weights_vector.end(); ++iter) {
    float w = (float)atof(iter->c_str());
    float f = (float)atof(fixedfs_vector.at( pos ).c_str());

    vector<string> range;
    Split(ranges_vector.at( pos ), ':', range);
    float min = (float)atof(range.at( 0 ).c_str());
    float max = (float)atof(range.at( 1 ).c_str());

    Tree2TreeFeatureValue featureValue(w, min, max, f);
    v_feature_values_.push_back(featureValue);
    ++pos;
  }
  return true;
}


//////////////////////////////////////////////////////////////////////////
bool Tree2TreeConfiguration::Init(STRPAIR &parameters) {
  CheckParamsInConf(parameters);
  CheckFilesInConf(parameters);
  return true;
}


bool Tree2TreeConfiguration::CheckParamsInConf(STRPAIR &parameters) {
  // check for "nround"
  string parameter_key = "nround";
  string parameter_default_value = "1";
  CheckEachParamInConf(parameters, parameter_key, parameter_default_value);
  nround_ = atoi(parameters[parameter_key].c_str());

  // check for "ngram"
  parameter_key = "ngram";
  parameter_default_value = "3";
  CheckEachParamInConf(parameters, parameter_key, parameter_default_value);
  ngram_ = atoi(parameters[parameter_key].c_str());

  // check for "beamsize"
  parameter_key = "beamsize";
  parameter_default_value = "30";
  CheckEachParamInConf(parameters, parameter_key, parameter_default_value);
  beamsize_ = atoi(parameters[parameter_key].c_str());

  // check for "nbest"
  parameter_key = "nbest";
  parameter_default_value = "30";
  CheckEachParamInConf(parameters, parameter_key, parameter_default_value);
  nbest_ = atoi(parameters[parameter_key].c_str()) > beamsize_ ? beamsize_ : atoi(parameters[parameter_key].c_str());

  // check for "nref"
  parameter_key = "nref";
  parameter_default_value = "4";
  CheckEachParamInConf(parameters, parameter_key, parameter_default_value);
  nref_ = atoi(parameters[parameter_key].c_str());

  // check for "nthread"
  parameter_key = "nthread";
  parameter_default_value = "0";
  CheckEachParamInConf(parameters, parameter_key, parameter_default_value);
  nthread_ = atoi(parameters[parameter_key].c_str());

  // check for "freefeature"
  parameter_key = "freefeature";
  parameter_default_value = "0";
  CheckEachParamInConf(parameters, parameter_key, parameter_default_value);
  free_feature_ = atoi(parameters[parameter_key].c_str());

  // check for "usepuncpruning"
  parameter_key = "usepunctpruning";
  parameter_default_value = "1";
  CheckEachParamInConf(parameters, parameter_key, parameter_default_value);
  use_punct_pruning_ = (parameters[parameter_key] == "0" ? false : true);

  // check for "usecubepruning"
  parameter_key = "usecubepruning";
  parameter_default_value = "1";
  CheckEachParamInConf(parameters, parameter_key, parameter_default_value);
  use_cube_pruning_ = (parameters[parameter_key] == "0" ? false : true);

  // check for "usenulltrans"
  parameter_key = "usenulltrans";
  parameter_default_value = "1";
  CheckEachParamInConf(parameters, parameter_key, parameter_default_value);
  use_null_translation_ = (parameters[parameter_key] == "0" ? false : true);

  // check for "outputoov"
  parameter_key = "outputoov";
  parameter_default_value = "0";
  CheckEachParamInConf(parameters, parameter_key, parameter_default_value);
  output_oov_ = (parameters[parameter_key] == "0" ? false : true);

  // check for "labeloov"
  parameter_key = "labeloov";
  parameter_default_value = "1";
  CheckEachParamInConf(parameters, parameter_key, parameter_default_value);
  label_oov_ = (parameters[parameter_key] == "0" ? false : true);

  // check for "-MERT"
  parameter_key = "-MERT";
  parameter_default_value = "0";
  CheckEachParamInConf(parameters, parameter_key, parameter_default_value);
  mert_flag_ = (parameters[parameter_key] == "0" ? false : true);

  // check for "treeparsing"
  parameter_key = "treeparsing";
  parameter_default_value = "0";
  CheckEachParamInConf(parameters, parameter_key, parameter_default_value);
  tree_parsing_flag_ = (parameters[parameter_key] == "0" ? false : true);

  // check for "grammar-encoding"
  parameter_key = "grammar-encoding";
  parameter_default_value = "0";
  CheckEachParamInConf(parameters, parameter_key, parameter_default_value);
  grammar_encoding_flag_ = (parameters[parameter_key] == "0" ? false : true);

  // init config file
  config_file_ = parameters["-config"];

  // check for "log";
  parameter_key = "-log";
  parameter_default_value = "niutransserver.log.txt";
  CheckEachParamInConf(parameters, parameter_key, parameter_default_value);
  log_file_ = parameters[parameter_key];

  // check for "output";
  parameter_key = "-output";
  parameter_default_value = "niutransserver.output.txt";
  CheckEachParamInConf(parameters, parameter_key, parameter_default_value);
  output_file_ = parameters[parameter_key];

  // check for "weights"
  parameter_key = "weights";
  parameter_default_value = "1.000 0.500 0.200 0.200 0.200 0.200 0.500 0.500 -0.100 1.000 0 0 0 0 0 0 0";
  CheckEachParamInConf(parameters, parameter_key, parameter_default_value);

  // check for "ranges"
  parameter_key = "ranges";
  parameter_default_value = "-3:7 -1:3 0:3 0:0.4 0:3 0:0.4 -3:3 -3:3 -3:0 -3:3 0:0 0:0 0:0 0:0 0:0 0:0 0:0";
  CheckEachParamInConf(parameters, parameter_key, parameter_default_value);

  // check for "fixedfs"
  parameter_key = "fixedfs";
  parameter_default_value = "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0";
  CheckEachParamInConf(parameters, parameter_key, parameter_default_value);

  tree2tree_features_.ParsingFeatures(parameters);
  return true;
}


bool Tree2TreeConfiguration::CheckEachParamInConf(STRPAIR &parameters, string  &parameter_key, string  &default_value) {
  if (parameters.find(parameter_key) == parameters.end()) {
    parameters[parameter_key] = default_value;
  }
  return true;
}


bool Tree2TreeConfiguration::CheckFilesInConf(STRPAIR &parameters) {
  // check for LM file
  string file_key = "Ngram-LanguageModel-File";
  CheckFileInConf(parameters, file_key);
  lm_file_ = parameters[file_key];

  // check for Target-side vocabulary
  file_key = "Target-Vocab-File";
  CheckFileInConf(parameters, file_key);
  target_vocab_file_ = parameters[file_key];

  // check for scfg rule set
  file_key = "SCFG-Rule-Set";
  CheckFileInConf(parameters, file_key);
  scfg_rule_set_file_ = parameters[file_key];

  // check for test file
  file_key = "-test";
  CheckFileInConf(parameters, file_key);
  test_file_ = parameters[file_key];
  return true;
}


bool Tree2TreeConfiguration::CheckFileInConf(STRPAIR &parameters, string &file_key) {
  if (parameters.find(file_key) != parameters.end()) {
    ifstream in_file(parameters[file_key].c_str());
    if (!in_file) {
      cerr<<"ERROR: Please check the path of \""<<file_key<<"\".\n"<<flush;
      exit( 1 );
    }
    in_file.clear();
    in_file.close();
  } else {
    cerr<<"ERROR: Please add parameter \""<<file_key<<"\" in your config file.\n"<<flush;
    exit( 1 );
  }
  return true;
}


bool Tree2TreeConfiguration::CheckFileInComm(STRPAIR &parameters, string &file_key) {
  if (parameters.find(file_key) != parameters.end()) {
    ifstream in_file(parameters[file_key].c_str());
    if (!in_file) {
      cerr<<"ERROR: Please check the path of \""<<file_key<<"\".\n"<<flush;
      exit(1);
    }
    in_file.clear();
    in_file.close();
  } else {
    cerr<<"ERROR: Please add parameter \""<<file_key<<"\" in your command.\n"<<flush;
    exit(1);
  }
  return true;
}


bool Tree2TreeConfiguration::PrintConfig() {
  cerr<<setfill( ' ' );
  cerr<<"Configuration:"<<"\n"
      <<"  nround            :"<<setw(9)<<nround_<<"\n"
      <<"  ngram             :"<<setw(9)<<ngram_<<"\n"
      <<"  beamsize          :"<<setw(9)<<beamsize_<<"\n"
      <<"  nbest             :"<<setw(9)<<nbest_<<"\n"
      <<"  nref              :"<<setw(9)<<nref_<<"\n"
      <<"  freefeature       :"<<setw(9)<<free_feature_<<"\n"
      <<"  nthread           :"<<setw(9)<<nthread_<<"\n\n"
      <<"  usepunctpruning   :"<<setw(9)<<(use_punct_pruning_ == true ? "true" : "false" )<<"\n"
      <<"  usecubepruning    :"<<setw(9)<<(use_cube_pruning_ == true ? "true" : "false" )<<"\n"
      <<"  usenulltranslation:"<<setw(9)<<(use_null_translation_ == true ? "true" : "false" )<<"\n"
      <<"  outputoov         :"<<setw(9)<<(output_oov_ == true ? "true" : "false" )<<"\n"
      <<"  labeloov          :"<<setw(9)<<(label_oov_ == true ? "true" : "false" )<<"\n"
      <<"  treeparsing       :"<<setw(9)<<(tree_parsing_flag_ == true ? "true" : "false" )<<"\n"
      <<"  nofiltering       :"<<setw(9)<<(no_fltering_flag_ == true ? "true" : "false" )<<"\n"
      <<"  grammarencoding   :"<<setw(9)<<(grammar_encoding_flag_ == true ? "true" : "false" )<<"\n"
      <<"  MERT              :"<<setw(9)<<(mert_flag_ == true ? "true" : "false" )<<"\n\n";
  cerr<<"  config = "<<config_file_<<"\n"
      <<"  output = "<<output_file_<<"\n"
      <<"  log    = "<<log_file_<<"\n"
      <<"  test   = "<<test_file_<<"\n"
      <<flush;
  return true;
}


bool Tree2TreeConfiguration::PrintFeatures() {
  cerr<<setiosflags( ios::fixed )<<setprecision( 2 );
  cerr<<"Weights of Feature:"<<"\n"
      <<"  0  N-gram LM              :"
      <<" w="  <<tree2tree_features_.v_feature_values_.at(0).weight_
      <<" f="  <<tree2tree_features_.v_feature_values_.at(0).fixed_
      <<" min="<<tree2tree_features_.v_feature_values_.at(0).min_value_
      <<" max="<<tree2tree_features_.v_feature_values_.at(0).max_value_<<"\n"
      <<"  1  # of Tar-words         :"
      <<" w="  <<tree2tree_features_.v_feature_values_.at(1).weight_
      <<" f="  <<tree2tree_features_.v_feature_values_.at(1).fixed_
      <<" min="<<tree2tree_features_.v_feature_values_.at(1).min_value_
      <<" max="<<tree2tree_features_.v_feature_values_.at(1).max_value_<<"\n"
      <<"  2  Pr(e|f)                :"
      <<" w="  <<tree2tree_features_.v_feature_values_.at(2).weight_
      <<" f="  <<tree2tree_features_.v_feature_values_.at(2).fixed_
      <<" min="<<tree2tree_features_.v_feature_values_.at(2).min_value_
      <<" max="<<tree2tree_features_.v_feature_values_.at(2).max_value_<<"\n"
      <<"  3  Lex(e|f)               :"
      <<" w="  <<tree2tree_features_.v_feature_values_.at(3).weight_
      <<" f="  <<tree2tree_features_.v_feature_values_.at(3).fixed_
      <<" min="<<tree2tree_features_.v_feature_values_.at(3).min_value_
      <<" max="<<tree2tree_features_.v_feature_values_.at(3).max_value_<<"\n"
      <<"  4  Pr(f|e)                :"
      <<" w="  <<tree2tree_features_.v_feature_values_.at(4).weight_
      <<" f="  <<tree2tree_features_.v_feature_values_.at(4).fixed_
      <<" min="<<tree2tree_features_.v_feature_values_.at(4).min_value_
      <<" max="<<tree2tree_features_.v_feature_values_.at(4).max_value_<<"\n"
      <<"  5  Lex(f|e)               :"
      <<" w="  <<tree2tree_features_.v_feature_values_.at(5).weight_
      <<" f="  <<tree2tree_features_.v_feature_values_.at(5).fixed_
      <<" min="<<tree2tree_features_.v_feature_values_.at(5).min_value_
      <<" max="<<tree2tree_features_.v_feature_values_.at(5).max_value_<<"\n"
      <<"  6  # of rules             :"
      <<" w="  <<tree2tree_features_.v_feature_values_.at(6).weight_
      <<" f="  <<tree2tree_features_.v_feature_values_.at(6).fixed_
      <<" min="<<tree2tree_features_.v_feature_values_.at(6).min_value_
      <<" max="<<tree2tree_features_.v_feature_values_.at(6).max_value_<<"\n"
      <<"  7  # of bi-lex (not fired):"
      <<" w="  <<tree2tree_features_.v_feature_values_.at(7).weight_
      <<" f="  <<tree2tree_features_.v_feature_values_.at(7).fixed_
      <<" min="<<tree2tree_features_.v_feature_values_.at(7).min_value_
      <<" max="<<tree2tree_features_.v_feature_values_.at(7).max_value_<<"\n"
      <<"  8  # of NULL-translation  :"
      <<" w="  <<tree2tree_features_.v_feature_values_.at(8).weight_
      <<" f="  <<tree2tree_features_.v_feature_values_.at(8).fixed_
      <<" min="<<tree2tree_features_.v_feature_values_.at(8).min_value_
      <<" max="<<tree2tree_features_.v_feature_values_.at(8).max_value_<<"\n"
      <<"  9  # of phrasal rules     :"
      <<" w="  <<tree2tree_features_.v_feature_values_.at(9).weight_
      <<" f="  <<tree2tree_features_.v_feature_values_.at(9).fixed_
      <<" min="<<tree2tree_features_.v_feature_values_.at(9).min_value_
      <<" max="<<tree2tree_features_.v_feature_values_.at(9).max_value_<<"\n"
      <<"  10 # of glue rules        :"
      <<" w="  <<tree2tree_features_.v_feature_values_.at(10).weight_
      <<" f="  <<tree2tree_features_.v_feature_values_.at(10).fixed_
      <<" min="<<tree2tree_features_.v_feature_values_.at(10).min_value_
      <<" max="<<tree2tree_features_.v_feature_values_.at(10).max_value_<<"\n"
      <<"  11 Pr(r|root(r))          :"
      <<" w="  <<tree2tree_features_.v_feature_values_.at(11).weight_
      <<" f="  <<tree2tree_features_.v_feature_values_.at(11).fixed_
      <<" min="<<tree2tree_features_.v_feature_values_.at(11).min_value_
      <<" max="<<tree2tree_features_.v_feature_values_.at(11).max_value_<<"\n"
      <<"  12 Pr(s(r)|root(r))       :"
      <<" w="  <<tree2tree_features_.v_feature_values_.at(12).weight_
      <<" f="  <<tree2tree_features_.v_feature_values_.at(12).fixed_
      <<" min="<<tree2tree_features_.v_feature_values_.at(12).min_value_
      <<" max="<<tree2tree_features_.v_feature_values_.at(12).max_value_<<"\n"
      <<"  13 # of lexical rules     :"
      <<" w="  <<tree2tree_features_.v_feature_values_.at(13).weight_
      <<" f="  <<tree2tree_features_.v_feature_values_.at(13).fixed_
      <<" min="<<tree2tree_features_.v_feature_values_.at(13).min_value_
      <<" max="<<tree2tree_features_.v_feature_values_.at(13).max_value_<<"\n"
      <<"  14 # of composed rules    :"
      <<" w="  <<tree2tree_features_.v_feature_values_.at(14).weight_
      <<" f="  <<tree2tree_features_.v_feature_values_.at(14).fixed_
      <<" min="<<tree2tree_features_.v_feature_values_.at(14).min_value_
      <<" max="<<tree2tree_features_.v_feature_values_.at(14).max_value_<<"\n"
      <<"  15 # of low-freq rules    :"
      <<" w="  <<tree2tree_features_.v_feature_values_.at(15).weight_
      <<" f="  <<tree2tree_features_.v_feature_values_.at(15).fixed_
      <<" min="<<tree2tree_features_.v_feature_values_.at(15).min_value_
      <<" max="<<tree2tree_features_.v_feature_values_.at(15).max_value_<<"\n"
      <<"  16 <UNDEFINED>            :"
      <<" w="  <<tree2tree_features_.v_feature_values_.at(16).weight_
      <<" f="  <<tree2tree_features_.v_feature_values_.at(16).fixed_
      <<" min="<<tree2tree_features_.v_feature_values_.at(16).min_value_
      <<" max="<<tree2tree_features_.v_feature_values_.at(16).max_value_<<"\n\n"
      <<flush;
  return true;
}


//////////////////////////////////////////////////////////////////////////
LanguageModel::~LanguageModel() { 
  UnloadModel(); 
}


void LanguageModel::LoadModel(Tree2TreeConfiguration &config) {
  cerr<<"Loading Ngram-LanguageModel-File\n"
      <<"  >> From File: "<<config.lm_file_<<"\n"
      <<flush;
  clock_t start, finish;
  start = clock();
  lm_index_ = g_initlm(config.lm_file_.c_str(), config.target_vocab_file_.c_str(), -1, 0);
  finish = clock();
  cerr<<"  Done! [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]\n\n"<<flush;
}


void LanguageModel::UnloadModel() {
  g_unloadlm(lm_index_);
}


float LanguageModel::GetProbability(vector<int> &wid, int &begin, int &end) {
  int word_id[MAX_WORD_LENGTH];
  int pos = 0;
  for (vector<int>::iterator iter = wid.begin(); iter != wid.end(); ++iter) {
    word_id[pos] = *iter;
    ++pos;
  }
  word_id[pos] = '\0';
  return g_get_ngram_prob2(lm_index_, word_id, begin, end, -1);
}


float LanguageModel::GetProbability(int *wid, int &begin, int &end) {
//  int word_id[MAX_WORD_LENGTH];
//  int pos = 0;
//  for (vector<int>::iterator iter = wid.begin(); iter != wid.end(); ++iter) {
//    word_id[pos] = *iter;
//    ++pos;
//  }
//  word_id[pos] = '\0';
  return g_get_ngram_prob2(lm_index_, wid, begin, end, -1);
}


//////////////////////////////////////////////////////////////////////////
bool UnitRule::InitBasicInformation(ScfgRule &scfg_rule) {
  root_ = scfg_rule.root_;
  root_source_ = scfg_rule.root_source_;
  root_target_ = scfg_rule.root_target_;
  source_ = scfg_rule.source_;
  target_ = scfg_rule.target_;
  alignment = scfg_rule.alignment;

  v_nonterminal_ = scfg_rule.v_nonterminal_;
  nonterminal_count_ = scfg_rule.nonterminal_count_;

  scores_of_translation_option_ = scfg_rule.scores_of_translation_option_;
  v_word_id_ = scfg_rule.v_word_id_;
  target_word_count_ = scfg_rule.target_word_count_;
  key_ = scfg_rule.key_;
  return true;
}


//////////////////////////////////////////////////////////////////////////
bool SynchronousGrammar::InitTargetVocab(Tree2TreeConfiguration &tree2tree_config) {
  ifstream in_file(tree2tree_config.target_vocab_file_.c_str());
  if (!in_file) {
    cerr<<"ERROR: Can not open file \""<<tree2tree_config.target_vocab_file_<<"\".\n"<<flush;
    exit(1);
  } else {
    cerr<<"Loading Target-Vocab-File\n"<<"  >> From File: "<<tree2tree_config.target_vocab_file_<<"\n"<<flush;
  }

  clock_t start, finish;
  start = clock();

  string line_of_in_file;
  int    wid = 0;
  while (getline(in_file, line_of_in_file)) {
    ClearIllegalChar(line_of_in_file);
    RmEndSpace(line_of_in_file);
    RmStartSpace(line_of_in_file);
    m_target_vocab_.insert(make_pair(line_of_in_file, wid));
    finish = clock();
    if (wid % 100000 == 0) {
      cerr<<"\r  Processed "<<wid<<" lines. [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   "<<flush;
    }
    ++wid;
  }
  finish = clock();
  cerr<<"\r  Processed "<<wid<<" lines. [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   \n"<<"  Done!\n\n"<<flush;
  in_file.close();
  return true;
}


bool SynchronousGrammar::InitNamedEntityLabelDict() {
  m_named_entity_label_dict_["$default"] = "NP=NP";
  return true;
}



bool SynchronousGrammar::GetMetaWordId(ScfgRule &scfg_rule) {
  scfg_rule.v_word_id_.push_back(m_target_vocab_[scfg_rule.source_]);
  scfg_rule.target_word_count_ = 1;
  return true;
}


bool SynchronousGrammar::GetWordId(string &translation, vector<int> &v_word_id) {
  vector<string> v_target;
  Split(translation, ' ', v_target);
  v_word_id.resize(v_target.size(), 0);
  for (size_t i = 0; i != v_target.size(); ++i) {
    if ('#' == v_target.at(i).at(0) && v_target.at(i).size() > 1) {
      v_word_id.at(i) = -atoi(v_target.at(i).substr(1).c_str());
    } else if (m_target_vocab_.find(v_target.at(i)) != m_target_vocab_.end()) {
      v_word_id.at(i) = m_target_vocab_[v_target.at(i)];
    } else {
      v_word_id.at(i) = m_target_vocab_["<unk>"];
    }
  }
  return true;
}


bool SynchronousGrammar::GetWordId(ScfgRule &scfg_rule) {
  vector<string> v_target;
  Split(scfg_rule.target_, ' ', v_target);
  scfg_rule.v_word_id_.resize(v_target.size(), 0);
  for (size_t i = 0; i != v_target.size(); ++i) {
    if ('#' == v_target.at(i).at(0) && v_target.at(i).size() > 1) {
      scfg_rule.v_word_id_.at(i) = -atoi(v_target.at(i).substr(1).c_str());
    } else if (m_target_vocab_.find(v_target.at(i)) != m_target_vocab_.end()) {
      scfg_rule.v_word_id_.at(i) = m_target_vocab_[v_target.at(i)];
    } else {
      scfg_rule.v_word_id_.at(i) = m_target_vocab_["<unk>"];
    }
  }
  scfg_rule.target_word_count_ = (int)scfg_rule.v_word_id_.size();
  return true;
}



int SynchronousGrammar::LoadScfg(Tree2TreeConfiguration &tree2tree_config) {
  ifstream in_file(tree2tree_config.scfg_rule_set_file_.c_str());
  if (!in_file) {
    fprintf(stderr, "ERROR: Can not open file '%s'.\n", tree2tree_config.scfg_rule_set_file_.c_str());
    fflush(stderr);
    exit(1);
  } else {
    fprintf(stderr, "Loading SCFG-Rule-Set\n  >> from File: %s\n", tree2tree_config.scfg_rule_set_file_.c_str());
    fflush(stderr);
  }

  string line_of_in_file;
  size_t line_no = 0;
  int total_count = 0;
  size_t invalid_count = 0;
  time(&start_time_);
  time_t now;
  while (getline(in_file, line_of_in_file)) {
    ++line_no;
    ScfgRule scfg_rule;
    l_scfg_rule_base_.push_back(scfg_rule);
    ClearIllegalChar(line_of_in_file);
    RmEndSpace(line_of_in_file);
    RmStartSpace(line_of_in_file);
    vector<string> rule_domains;
    SplitWithStr(line_of_in_file, " ||| ", rule_domains);
    if (!tree2tree_config.no_fltering_flag_ && \
        (!IsValid(tree2tree_config, rule_domains.at(0), rule_domains.at(1)) || !IsValidForScfg(rule_domains.at(0), rule_domains.at(1)))) {
      if (tree2tree_config.grammar_encoding_flag_) {
        while (getline(in_file, line_of_in_file)) {
          ClearIllegalChar(line_of_in_file);
          RmEndSpace(line_of_in_file);
          RmStartSpace(line_of_in_file);
          if ("" == line_of_in_file) {
            break;
          }
        }
      }
      ++invalid_count;
      continue;
    }
//    if (!ParseScfgRule(tree2tree_config, rule_domains, v_scfg_rule_base_.at(v_scfg_rule_base_.size() - 1))) {
    if (!ParseScfgRule(tree2tree_config, rule_domains, l_scfg_rule_base_.back())) {
      if (tree2tree_config.grammar_encoding_flag_) {
        while (getline(in_file, line_of_in_file)) {
          ClearIllegalChar(line_of_in_file);
          RmEndSpace(line_of_in_file);
          RmStartSpace(line_of_in_file);
          if ("" == line_of_in_file) {
            break;
          }
        }
      }
      ++invalid_count;
      continue;
    }
    
    // load unit rules
    if (tree2tree_config.grammar_encoding_flag_) {
//      LoadUnitRules(tree2tree_config, in_file, v_scfg_rule_base_.at(v_scfg_rule_base_.size() - 1));
      LoadUnitRules(tree2tree_config, in_file, l_scfg_rule_base_.back());
    }

//    AddSourceSideForUnitRules(v_scfg_rule_base_.at(v_scfg_rule_base_.size() - 1));
    AddSourceSideForUnitRules(l_scfg_rule_base_.back());
    ++total_count;


    if (line_no % 10000 == 0) {
      time(&now);
      int seconds = (int)difftime(now, start_time_);
      fprintf(stderr, "\r  Processed %-9d lines. total=%-5d invalid=%-5d seconds=%-5d", line_no, total_count, invalid_count, seconds);
      fflush(stderr);
    }
  }
  time(&now);
  int seconds = (int)difftime(now, start_time_);
  fprintf(stderr, "\r  Processed %-9d lines. total=%-5d invalid=%-5d seconds=%-5d\n", line_no, total_count, invalid_count, seconds);
  fflush(stderr);

  AddMetaRules();
  SortRules();
  return total_count;
}


bool SynchronousGrammar::ParseScfgRule(Tree2TreeConfiguration &tree2tree_config, vector<string> &rule_domains, ScfgRule &scfg_rule) {
  string source_domain = rule_domains.at(0);
  string target_domain = rule_domains.at(1);
  string root_domain = rule_domains.at(2);
  string features_domain = rule_domains.at(3);
  string aligment_domain = (rule_domains.size() > 4 ? rule_domains.at(4) : "");
  if (!tree2tree_config.use_null_translation_ && ("<NULL>" == target_domain)) {
    return false;
  }
  ParseRootSymbol(root_domain, scfg_rule);
  bool is_unary_production = IsUnaryProduction(source_domain);
  bool is_unary = is_unary_production && IsUnaryProductionForSourceSide(scfg_rule.root_source_, source_domain);
  if (tree2tree_config.tree_parsing_flag_ && !is_unary) {
    string real_root = tree2tree_config.generate_tree_structure_flag_ ? root_domain : scfg_rule.root_;
    scfg_rule.source_ = "#" + real_root + " ( " + source_domain + " )";
  } else {
    scfg_rule.source_ = source_domain;
  }
  scfg_rule.target_ = target_domain;
  ParseFeatureValues(features_domain, scfg_rule.scores_of_translation_option_);

  /////////////// output <NULL>

  GetWordId(scfg_rule);
  if (tree2tree_config.tree_parsing_flag_ && tree2tree_config.generate_tree_structure_flag_) {
    ;
  } else if (tree2tree_config.tree_parsing_flag_ && !tree2tree_config.generate_tree_structure_flag_) {
    if( !ParseSourceSide(scfg_rule, !is_unary) ) {
      return false;
    }
  }
  scfg_rule.scores_of_translation_option_.number_of_target_words_ = (float)(scfg_rule.target_word_count_ - scfg_rule.nonterminal_count_);
  if (scfg_rule.nonterminal_count_ == 0) {
    scfg_rule.scores_of_translation_option_.number_of_phrasal_rules_ = 1;
  }

  return (IsValidWordIdForScfg(scfg_rule) && InformalCheck(source_domain,target_domain, scfg_rule));
}


bool SynchronousGrammar::LoadUnitRules(Tree2TreeConfiguration &tree2tree_config, ifstream &in_file, ScfgRule &scfg_rule) {
  string line_of_in_file;
  while (getline(in_file, line_of_in_file)) {
    ClearIllegalChar(line_of_in_file);
    RmEndSpace(line_of_in_file);
    RmStartSpace(line_of_in_file);
    if ("" == line_of_in_file) {
      break;
    }
    if ("<null>" == line_of_in_file) {
      break;
    }
    if (tree2tree_config.tree_parsing_flag_) {
      continue;                                 // unit rules are useless for tree-parsing
    }
  }

  if (tree2tree_config.tree_parsing_flag_) {
    if ("<null>" == line_of_in_file) {
      getline(in_file, line_of_in_file);        // skip a empty line
    }
    GenerateUnitRules(tree2tree_config, scfg_rule);
    return true;
  }
  return true;
}


bool SynchronousGrammar::GenerateUnitRules(Tree2TreeConfiguration &tree2tree_config, ScfgRule &scfg_rule) {
  UnitRule unit_rule;
  unit_rule.p_parent_rule_ = &scfg_rule;
  unit_rule.is_lexical_rule_ = !IsNonLexical(scfg_rule.source_);
  unit_rule.is_complete_ = IsComplete(scfg_rule.root_);
  unit_rule.InitBasicInformation(scfg_rule);
  unit_rule.id_ = 0;

  // unit rules
  scfg_rule.rule_count_ = 0;
  scfg_rule.v_unit_rules_.push_back(unit_rule);
  ++scfg_rule.rule_count_;

  if (!tree2tree_config.tree_parsing_flag_) {
    //////////////////////////////////////////////////////////////////////////
    
  } else {
    if ("#" != unit_rule.key_) {
      AddOptionWithRuleSourceSide(unit_rule);
    }
    AddOptionWithSymbol(unit_rule);
  }

  return true;
}


bool SynchronousGrammar::AddSourceSideForUnitRules(ScfgRule &scfg_rule) {
  for (int i = 0; i < scfg_rule.rule_count_; ++i) {
    UnitRule * p_unit_rule = &scfg_rule.v_unit_rules_.at(i);
//    list<UnitRule> * p_unit_rule_list = FindRuleListWithSymbol(p_unit_rule->source_);
    UnitRules * p_unit_rules = FindRuleListWithSymbol(p_unit_rule->source_);
//    if (p_unit_rule_list->size() <=1 ) {
    if (p_unit_rules->l_unit_rules_.size() <=1 ) {
      l_source_side_base_.push_back(scfg_rule.source_);
    }
  }
  return true;
}


bool SynchronousGrammar::SortRules() {
  fprintf(stderr, "Sorting rules ...\n");
  for (list<string>::iterator iter = l_source_side_base_.begin(); iter != l_source_side_base_.end(); ++iter) {
    UnitRules * p_unit_rules = &m_translation_options_symbol_[*iter];
    p_unit_rules->l_unit_rules_.sort(CompareUnitRule);
  }
  return true;
}


bool SynchronousGrammar::ParseRootSymbol(string &root, ScfgRule &scfg_rule) {
  if ("" == root) {
    return false;
  }
  scfg_rule.root_ = root;
  vector<string> tmp_src_tgt_root;
  Split(root, '=', tmp_src_tgt_root);
  if (tmp_src_tgt_root.size() < 2) {
    scfg_rule.root_source_ = scfg_rule.root_;
    scfg_rule.root_target_ = scfg_rule.root_;
    return true;
  } else {
    scfg_rule.root_target_ = tmp_src_tgt_root.at(0);
    scfg_rule.root_source_ = tmp_src_tgt_root.at(1);
    return true;
  }
}


bool SynchronousGrammar::ParseSourceSide(ScfgRule &scfg_rule, bool index_for_tree_parsing) {
  scfg_rule.key_.reserve(scfg_rule.source_.size());
  int position_begin = 0;
  int position_end = 0;
  for (position_begin = 0; position_begin < scfg_rule.source_.size(); ++position_begin) {
    if ('#' != scfg_rule.source_.at(position_begin)) {
      scfg_rule.key_ += scfg_rule.source_.at(position_begin);
      continue;
    } else {
      if ((position_end = (int)scfg_rule.source_.find(' ', position_begin)) != string::npos) {
        if (position_end > position_begin + 1) {
          string tmp_nonterminal = scfg_rule.source_.substr(position_begin + 1, position_end - position_begin - 1);
          vector<string> v_tmp_nonterminal;
          Split(tmp_nonterminal, '=', v_tmp_nonterminal);
          scfg_rule.key_ += '#' + v_tmp_nonterminal.at(1);
          scfg_rule.nonterminal_count_ += 1;
          if (scfg_rule.nonterminal_count_ > 1) {
            Nonterminal nonterminal(tmp_nonterminal, v_tmp_nonterminal.at(1), v_tmp_nonterminal.at(0));
            scfg_rule.v_nonterminal_.push_back(nonterminal);
          }
          position_begin = position_end - 1;
          continue;
        } else {
          return false;
        }
      } else if (0 == position_begin) {
        string tmp_nonterminal = scfg_rule.source_.substr(position_begin + 1);
        vector<string> v_tmp_nonterminal;
        Split(tmp_nonterminal, '=', v_tmp_nonterminal);
        Nonterminal nonterminal(tmp_nonterminal, v_tmp_nonterminal.at(1), v_tmp_nonterminal.at(0));
        scfg_rule.v_nonterminal_.push_back(nonterminal);
        scfg_rule.key_ = "#";
        scfg_rule.nonterminal_count_ = 1;
        break;
      }
    }
  }
  if (0 == scfg_rule.nonterminal_count_) {
    scfg_rule.nonterminal_count_ = 0;
  } else if (index_for_tree_parsing) {
    scfg_rule.nonterminal_count_ -= 1;
  } else {
    ;
  }
  return true;
}



bool SynchronousGrammar::ParseFeatureValues(string &features_domain, ScoresOfTree2TreeTranslateOption &scores_of_translation_option) {
  vector<string> v_feature_value_string;
  Split(features_domain, ' ',v_feature_value_string);

  if (v_feature_value_string.size() < 11) {
    fprintf(stderr, "  Warning: Format Error!\n");
    fflush(stderr);
    return false;
  }

  scores_of_translation_option.pr_tgt_given_src_ = (float)atof(v_feature_value_string.at(0).c_str());
  scores_of_translation_option.lex_tgt_given_src_ = (float)atof(v_feature_value_string.at(1).c_str());
  scores_of_translation_option.pr_src_given_tgt_ = (float)atof(v_feature_value_string.at(2).c_str());
  scores_of_translation_option.lex_src_given_tgt_ = (float)atof(v_feature_value_string.at(3).c_str());
  scores_of_translation_option.number_of_rules_ = (float)atof(v_feature_value_string.at(4).c_str());
  scores_of_translation_option.pr_r_give_rootr_ = (float)atof(v_feature_value_string.at(5).c_str());
  scores_of_translation_option.pr_sr_give_rootr_ = (float)atof(v_feature_value_string.at(6).c_str());
  scores_of_translation_option.is_lexicalized_ = (float)atof(v_feature_value_string.at(7).c_str());
  scores_of_translation_option.is_composed_ = (float)atof(v_feature_value_string.at(8).c_str());
  scores_of_translation_option.is_low_frequency_ = (float)atof(v_feature_value_string.at(9).c_str());
  scores_of_translation_option.bi_lex_links_ = (float)atof(v_feature_value_string.at(10).c_str());
  return true;
}


bool SynchronousGrammar::AddOptionWithRuleSourceSide(UnitRule &unit_rule) {
  if (m_translation_options_.find(unit_rule.key_) != m_translation_options_.end()) {
    if (m_translation_options_symbol_.find(unit_rule.source_) == m_translation_options_symbol_.end()) {
      m_translation_options_[unit_rule.key_].l_unit_rules_.push_back(unit_rule);
    }
  } else {
    m_translation_options_[unit_rule.key_].l_unit_rules_.push_back(unit_rule);
  }
  return true;
}


bool SynchronousGrammar::AddOptionWithSymbol(UnitRule &unit_rule) {
  m_translation_options_symbol_[unit_rule.source_].l_unit_rules_.push_back(unit_rule);
  return true;
}


bool SynchronousGrammar::AddOption(UnitRule &unit_rule) {
  m_translation_options_[unit_rule.key_].l_unit_rules_.push_back(unit_rule);
  return true;
}



bool SynchronousGrammar::AddMetaRules() {
  string symbol = "<s>";
  AddMetaRuleEntry(symbol);
  symbol = "</s>";
  AddMetaRuleEntry(symbol);
  symbol = "<unk>";
  AddMetaRuleEntry(symbol);
  return true;
}


bool SynchronousGrammar::AddMetaRuleEntry(string &symbol) {
  ScfgRule scfg_rule;
  l_scfg_rule_base_.push_back(scfg_rule);
//  ScfgRule * p_scfg_rule = &v_scfg_rule_base_.at(v_scfg_rule_base_.size() - 1);
  ScfgRule * p_scfg_rule = &l_scfg_rule_base_.back();

//  ParseRootSymbol(symbol, v_scfg_rule_base_.at(v_scfg_rule_base_.size() - 1));
  ParseRootSymbol(symbol, l_scfg_rule_base_.back());
  p_scfg_rule->source_ = symbol;
  p_scfg_rule->target_ = "";
//  GetMetaWordId(v_scfg_rule_base_.at(v_scfg_rule_base_.size() - 1));
  GetMetaWordId(l_scfg_rule_base_.back());
  p_scfg_rule->scores_of_translation_option_.number_of_target_words_ = (float)p_scfg_rule->target_word_count_;

  UnitRule unit_rule;
//  unit_rule.InitBasicInformation(v_scfg_rule_base_.at(v_scfg_rule_base_.size() - 1));
  unit_rule.InitBasicInformation(l_scfg_rule_base_.back());

  unit_rule.key_ = p_scfg_rule->source_;
  unit_rule.p_parent_rule_ = p_scfg_rule;
  unit_rule.id_ = 0;
  unit_rule.is_complete_ = true;

  // unit rules
  p_scfg_rule->rule_count_ = 0;
  p_scfg_rule->v_unit_rules_.push_back(unit_rule);
  ++p_scfg_rule->rule_count_;
  if ("<unk>" == symbol) {
      p_scfg_rule->root_ = m_named_entity_label_dict_["$default"];
      p_scfg_rule->v_unit_rules_.at(0).root_ = p_scfg_rule->root_;
  }

  // index the rule
  AddOption(unit_rule);
  AddOptionWithSymbol(unit_rule);
  return true;
}


bool SynchronousGrammar::IsUnaryProduction(string &source) {
  if (source.size() < 2) {
    return false;
  }
  if (source.at(0) != '#' || source.find(' ') != string::npos) {
    return false;
  }
  return true;
}


bool SynchronousGrammar::IsUnaryProductionForSourceSide(string &root_source, string &source) {
  if ("" == source) {
    return false;
  }
  string source_next = source.substr(1);
  vector<string> tmp_src_tgt_symbol;
  Split(source_next, '=', tmp_src_tgt_symbol);

  if (tmp_src_tgt_symbol.size() != 2) {
    return false;
  } else {
    if (root_source == tmp_src_tgt_symbol.at(1)) {
      return true;
    } else {
      return false;
    }
  }
}


bool SynchronousGrammar::IsValidWordIdForScfg(ScfgRule &scfg_rule) {
//  bool lexical_flag = false;
  for (int i = 0; i < scfg_rule.target_word_count_; ++i) {
    if (scfg_rule.v_word_id_.at(i) < 0 && (-scfg_rule.v_word_id_.at(i)) > scfg_rule.nonterminal_count_) {
      return false;
    }
//    if (scfg_rule.v_word_id_.at(i) > 0) {
//      lexical_flag = true;
//    }
  }
  return true;
}


bool SynchronousGrammar::InformalCheck(string &source_domain, string &target_domain, ScfgRule &scfg_rule) {
  if (target_domain.find("$number - $number , $number - $number , $number") != string::npos) {
    return false;
  }
  if (source_domain.find(" ") == string::npos && scfg_rule.target_word_count_ > 8) {
    return false;
  }
  return true;
}


bool SynchronousGrammar::IsNonLexical(string &source) {
  if (source.size() == 0 || source.at(0) != '#') {
    return false;
  }
  string::size_type word_position = source.find(' ');
  while (word_position != string::npos) {
    string word = source.substr(word_position);
    if (word.size() > 1 && word.at(1) != '#') {
      return false;
    }
    if (word_position < source.size() - 2 && word.at(2) == ' ') {
      return false;
    }
    word_position = source.find(' ', word_position + 1);
  }
  return true;
}


bool SynchronousGrammar::IsComplete(string &root) {
  if ("" == root) {
    return true;
  }
  string::size_type virutal_nt_position = root.find(':');
  if (string::npos != virutal_nt_position) {
    return false;
  } else {
    return true;
  }
}


bool SynchronousGrammar::IsValid(Tree2TreeConfiguration &tree2tree_config, string &source, string &target) {
  string type = "$number";
  if (!IsValid(source, target, type)) {
    return false;
  }
  type = "$time";
  if (!IsValid(source, target, type)) {
    return false;
  }
  type = "$date";
  if (!IsValid(source, target, type)) {
    return false;
  }
  type = "(";
  if (!IsValid(source, target, type)) {
    return false;
  }
  type = ")";
  if (!IsValid(source, target, type)) {
    return false;
  }
  if ("<NULL>" == target && !tree2tree_config.use_null_translation_) {
    return false;
  }
  return true;
}


bool SynchronousGrammar::IsValid(string &source, string &target, string &type) {
  string::size_type source_position = source.find(type);
  string::size_type target_position = target.find(type);
  while ((string::npos != source_position) && (string::npos != target_position)) {
    source_position = source.find(type, source_position + 1);
    target_position = target.find(type, target_position + 1);
  }
  return !((string::npos == source_position) ^ (string::npos == target_position));
}


bool SynchronousGrammar::IsValidForScfg(string &source, string &target) {
  if (source.size() > 2) {
    if ('#' == source.at(0) && ' ' == source.at(1)) {
      return false;
    } else if (('#' == source.at(source.size() - 1)) && (' ' == source.at(source.size() - 2))) {
      return false;
    } else if (source.find(" # ") != string::npos) {
      return false;
    }
  }
  return true;
}


bool SynchronousGrammar::IsMetaSymbol(string &phrase) {
  if ("<s>" == phrase || "</s>" == phrase || "<unk>" == phrase) {
    return true;
  } else {
    return false;
  }
}

/*
list<UnitRule> * SynchronousGrammar::FindRuleListWithSymbol(string &source) {
 if (m_translation_options_symbol_.find(source) != m_translation_options_symbol_.end()) {
  return &m_translation_options_symbol_[source].l_unit_rules_;
 } else {
  return NULL;
 }
}
*/


UnitRules * SynchronousGrammar::FindRuleListWithSymbol(string &source) {
  if (m_translation_options_symbol_.find(source) != m_translation_options_symbol_.end()) {
    UnitRules * p_unit_rules = &m_translation_options_symbol_[source];
    return p_unit_rules;
  } else {
    return NULL;
  }
}


UnitRules * SynchronousGrammar::FindRuleList(string &key) {
  if (m_translation_options_.find(key) != m_translation_options_.end()) {
    UnitRules * p_unit_rules = &m_translation_options_[key];
    return p_unit_rules;
  } else {
    return NULL;
  }
}


//////////////////////////////////////////////////////////////////////////
bool SlotInformation::Init(int &slot_number) {
  slot_number_ = slot_number;
  v_slot_hypothesis_pointer_.resize(slot_number_, NULL);
  v_slot_hypothesis_.resize(slot_number_, NULL);
  v_hypothesis_number_.resize(slot_number_, 0);
  v_inverted_nonterminal_index_.resize(slot_number_, 0);
  p_matched_pattern_ = NULL;
  return true;
}


//////////////////////////////////////////////////////////////////////////
bool Tree2TreeHypothesis::Init(Tree2TreeConfiguration &tree2tree_config, Tree2TreeCell &tree2tree_cell) {
  tree2tree_cell_ = &tree2tree_cell;
  model_score_except_lm_ = 0;
  v_feature_values_.resize(tree2tree_config.tree2tree_features_.features_number_, 0.0f);
  return true;
}


bool Tree2TreeHypothesis::InitFeatureScores(UnitRule &unit_rule) {
  if (v_feature_values_.size() < 17) {
    fprintf(stderr, "    ERROR: Features in config file is not right!\n");
    fflush(stderr);
    exit(1);
  } else {
    v_feature_values_.at(1) = unit_rule.scores_of_translation_option_.number_of_target_words_;
    v_feature_values_.at(2) = unit_rule.scores_of_translation_option_.pr_tgt_given_src_;
    v_feature_values_.at(3) = unit_rule.scores_of_translation_option_.lex_tgt_given_src_;
    v_feature_values_.at(4) = unit_rule.scores_of_translation_option_.pr_src_given_tgt_;
    v_feature_values_.at(5) = unit_rule.scores_of_translation_option_.lex_src_given_tgt_;
    v_feature_values_.at(6) = unit_rule.scores_of_translation_option_.number_of_rules_;
    v_feature_values_.at(7) = unit_rule.scores_of_translation_option_.bi_lex_links_;
    v_feature_values_.at(9) = unit_rule.scores_of_translation_option_.number_of_phrasal_rules_;
    v_feature_values_.at(11) = unit_rule.scores_of_translation_option_.pr_r_give_rootr_;
    v_feature_values_.at(12) = unit_rule.scores_of_translation_option_.pr_sr_give_rootr_;
    v_feature_values_.at(13) = unit_rule.scores_of_translation_option_.is_lexicalized_;
    v_feature_values_.at(14) = unit_rule.scores_of_translation_option_.is_composed_;
    v_feature_values_.at(15) = unit_rule.scores_of_translation_option_.is_low_frequency_;
  }
  return true;
}


bool Tree2TreeHypothesis::Create(string &translation, Tree2TreeModel &tree2tree_model) {
  translation_length_ = (int)translation.size();
  translation_result_ = translation;
  model_score_ = 0;
  tree2tree_model.scfg_model_.GetWordId(translation_result_, v_target_word_id_);
  v_feature_values_.at(7) = (float)v_target_word_id_.size();
  return true;
}




bool Tree2TreeHypothesis::ComputeFeatureScores(UnitRule &unit_rule) {
  if (v_feature_values_.size() < 17) {
    fprintf(stderr, "    ERROR: Features in config file is not right!\n");
    fflush(stderr);
    exit(1);
  }
  v_feature_values_.at(2) += unit_rule.scores_of_translation_option_.pr_tgt_given_src_;
  v_feature_values_.at(3) += unit_rule.scores_of_translation_option_.lex_tgt_given_src_;
  v_feature_values_.at(4) += unit_rule.scores_of_translation_option_.pr_src_given_tgt_;
  v_feature_values_.at(5) += unit_rule.scores_of_translation_option_.lex_src_given_tgt_;
  v_feature_values_.at(6) += unit_rule.scores_of_translation_option_.number_of_rules_;
  v_feature_values_.at(7) += unit_rule.scores_of_translation_option_.bi_lex_links_;
  v_feature_values_.at(9) += unit_rule.scores_of_translation_option_.number_of_phrasal_rules_;
  v_feature_values_.at(11) += unit_rule.scores_of_translation_option_.pr_r_give_rootr_;
  v_feature_values_.at(12) += unit_rule.scores_of_translation_option_.pr_sr_give_rootr_;
  v_feature_values_.at(13) += unit_rule.scores_of_translation_option_.is_lexicalized_;
  v_feature_values_.at(14) += unit_rule.scores_of_translation_option_.is_composed_;
  v_feature_values_.at(15) += unit_rule.scores_of_translation_option_.is_low_frequency_;
  return true;
}



bool Tree2TreeHypothesis::Copy(Tree2TreeHypothesis * p_tree2tree_hypothesis) {
  tree2tree_cell_ = p_tree2tree_hypothesis->tree2tree_cell_;
  model_score_ = p_tree2tree_hypothesis->model_score_;
  model_score_except_lm_ = p_tree2tree_hypothesis->model_score_except_lm_;
  language_model_score_ = p_tree2tree_hypothesis->language_model_score_;
  translation_result_ = p_tree2tree_hypothesis->translation_result_;
  translation_length_ = p_tree2tree_hypothesis->translation_length_;
  target_word_count_ = p_tree2tree_hypothesis->target_word_count_;
  v_target_word_.clear();
  v_target_word_.insert(v_target_word_.begin(), p_tree2tree_hypothesis->v_target_word_.begin(), p_tree2tree_hypothesis->v_target_word_.end());
  v_target_word_id_.clear();
  v_target_word_id_.insert(v_target_word_id_.begin(), p_tree2tree_hypothesis->v_target_word_id_.begin(), p_tree2tree_hypothesis->v_target_word_id_.end());
  v_target_n_gram_lm_score_.clear();
  v_target_n_gram_lm_score_.insert(v_target_n_gram_lm_score_.begin(), p_tree2tree_hypothesis->v_target_n_gram_lm_score_.begin(), p_tree2tree_hypothesis->v_target_n_gram_lm_score_.end());
  v_feature_values_.clear();
  v_feature_values_.insert(v_feature_values_.begin(), p_tree2tree_hypothesis->v_feature_values_.begin(), p_tree2tree_hypothesis->v_feature_values_.end());
  root_ = p_tree2tree_hypothesis->root_;
  p_rule_used_ = p_tree2tree_hypothesis->p_rule_used_;

  left_partial_translation_ = p_tree2tree_hypothesis->left_partial_translation_;
  right_partial_translation_ = p_tree2tree_hypothesis->right_partial_translation_;
  p_matched_pattern_ = p_tree2tree_hypothesis->p_matched_pattern_;
  return true;
}


bool Tree2TreeHypothesis::operator ==(Tree2TreeHypothesis &tree2tree_hypothesis) {
  if (tree2tree_cell_ == tree2tree_hypothesis.tree2tree_cell_ && \
      root_ == tree2tree_hypothesis.root_ && \
      model_score_ == tree2tree_hypothesis.model_score_) {
    return true;
  } else {
    return false;
  }
}


//////////////////////////////////////////////////////////////////////////
bool Tree2TreeCell::Init(int &begin, int &end, int &beam_size) {
  begin_ = begin;
  end_ = end;
  v_hypotheses_.reserve(beam_size);
  return true;
}


bool Tree2TreeCell::InitSyntax(int &begin, int &end, int &beam_size) {
  
  return true;
}


bool Tree2TreeCell::AddTree2TreeHypothesis(Tree2TreeHypothesis &tree2tree_hypothesis) {
  if ("" != tree2tree_hypothesis.root_) {
    l_translation_.push_back(tree2tree_hypothesis);
  }
  return true;
}


bool Tree2TreeCell::AddMatchedPattern(MatchedPattern &matched_pattern) {
  v_matched_pattern_.push_back(matched_pattern);
  return true;
}


bool Tree2TreeCell::ReassignCellToHypothesisInBeam(Tree2TreeCell &tree2tree_cell) {
  for (int i = 0; i < n_; ++i) {
    v_nbest_results_.v_tree2tree_hypothesis_.at(i).tree2tree_cell_ = &tree2tree_cell;
  }
  return true;
}




//////////////////////////////////////////////////////////////////////////
bool Tree2TreeModel::Init(Tree2TreeConfiguration &tree2tree_config) {
  ngram_language_model_.LoadModel(tree2tree_config);
  scfg_model_.InitTargetVocab(tree2tree_config);
  scfg_model_.InitNamedEntityLabelDict();
  scfg_model_.LoadScfg(tree2tree_config);
  tree2tree_config.PrintConfig();
  tree2tree_config.PrintFeatures();
  return true;
}


//////////////////////////////////////////////////////////////////////////
bool Tree2TreeTestSet::ReadTestSentences(string &test_file_name) {
  fprintf(stderr, "Start loading test sentences...\n");
  fflush(stderr);
  ifstream in_file(test_file_name.c_str());
  if (!in_file) {
    fprintf(stderr, "    ERROR: Please check the path of '%s'.\n", test_file_name.c_str());
    fflush(stderr);
    exit(1);
  }

  string line;
  int line_no = 0;
  while (getline(in_file, line)) {
    ++line_no;
    ClearIllegalChar(line);
    RmEndSpace(line);
    RmStartSpace(line);
    source_sentences_set_.push_back(line);
    ++source_sentences_number_;
    if (line_no % 100 == 0) {
      fprintf(stderr, "\r    Processed %5d lines.", line_no);
    }
  }
  fprintf(stderr, "\r    Processed %5d lines.\n", line_no);


  in_file.clear();
  in_file.close();
  return true;
}



//////////////////////////////////////////////////////////////////////////
bool CompareUnitRule(const UnitRule &unit_rule_1, const UnitRule &unit_rule_2) {
  return unit_rule_1.scores_of_translation_option_.pr_tgt_given_src_ > unit_rule_2.scores_of_translation_option_.pr_tgt_given_src_;
}


bool CompareTranslationList(const Tree2TreeHypothesis &tree2tree_hypothesis_1, const Tree2TreeHypothesis &tree2tree_hypothesis_2) {
  return tree2tree_hypothesis_1.model_score_ > tree2tree_hypothesis_2.model_score_;
}


//////////////////////////////////////////////////////////////////////////
/*
NodeHeapForSearch::~NodeHeapForSearch() {

}


bool NodeHeapForSearch::Init(int &heap_size, HEAPTYPE type) {
  type_ = type;
  size_ = heap_size;
  v_tree2tree_cell_.resize(size_, 0);
  count_ = 0;
  return true;
}


bool NodeHeapForSearch::Clear() {
  count_ = 0;
  v_tree2tree_cell_.clear();
  v_tree2tree_cell_.resize(size_, 0);
  return true;
}
*/


//////////////////////////////////////////////////////////////////////////
bool Tree2TreeBasedDecoder::Init(Tree2TreeConfiguration &tree2tree_config, Tree2TreeModel &tree2tree_model) {
  p_tree2tree_config_ = &tree2tree_config;
  p_tree2tree_model_ = &tree2tree_model;
  InitSourceTreeParameters();

  /*
  int heap_size = 5 * tree2tree_config.beamsize_ * tree2tree_config.beamsize_;
  if (heap_size < 50) {
    heap_size = 50;
  }
  node_heap_for_search_.Init(heap_size, MAXHEAP);
  */

  explored_.resize(tree2tree_config.beamsize_ + 20);
  for (vector<vector<int> >::iterator iter = explored_.begin(); iter != explored_.end(); ++iter) {
    iter->resize(tree2tree_config.beamsize_ + 20, 0);
  }

  return true;
}


bool Tree2TreeBasedDecoder::InitSourceTreeParameters() {
/*
  source_tree_ = new Tree();
  source_tree_->maxWordNum = 4;
  source_tree_->maxNTNum = 5;
  source_tree_->maxDepth = 3;
  source_tree_->maxEdgeNumPerNode = 1000000;
  source_tree_->maxFragNumPerNode = 1000000;
*/
  return true;
}


//bool Tree2TreeBasedDecoder::AllocateSpaceForCellsAndInit(Tree2TreeConfiguration &tree2tree_config) {
bool Tree2TreeBasedDecoder::AllocateSpaceForCellsAndInit() {
  tree2tree_cells_.cells_.resize(v_source_words_.size() + 1);
  for (vector<vector<Tree2TreeCell> >::iterator iter = tree2tree_cells_.cells_.begin(); iter != tree2tree_cells_.cells_.end(); ++iter) {
    iter->resize(v_source_words_.size() + 1);
  }

  for (int i = 0; i < v_source_words_.size() + 1; ++i) {
    for (int j = 0; j < v_source_words_.size() + 1; ++j) {
      tree2tree_cells_.cells_.at(i).at(j).Init(i,j, p_tree2tree_config_->beamsize_);
    }
  }
  return true;
}


bool Tree2TreeBasedDecoder::Decode() {
  if (!ParseSourceSentence()) {
    fprintf(stderr, "\nFailure: %s\n", decoding_sentence_.sentence_.c_str());
    return false;
  }
  AllocateSpaceForCellsAndInit();
  MatchScfgRules();
  AddBasicRules();                     // Basic rules (to deal with OOV words);

  // Loading pre-matched rules and rules used in Viterbi derivation
  LoadPrematchedRules();
  LoadViterbiRules();
  if (p_tree2tree_config_->tree_parsing_flag_) {
    TreeParsingDecoding();
  } else {
    CkyDecoding();
  }

  if (0 == tree2tree_cells_.cells_.at(0).at(v_source_words_.size()).n_) {
    decoding_failure_ = true;
  }
  
  if (0 == tree2tree_cells_.cells_.at(0).at(v_source_words_.size()).n_ && p_tree2tree_config_->deal_with_decoding_failure_flag_) {
    CreateTranslationUsingTreeSequence();
  }

  return true;
}


bool Tree2TreeBasedDecoder::CreateTranslationUsingTreeSequence() {
  ////////////////////////////////////////////////////////////////////////// NEW
  vector<Tree2TreeCell *> v_best_span_sequence;
  GetBestSpanSequenceForTreeParsing(v_best_span_sequence);
  if (0 == v_best_span_sequence.size()) {
    return false;
  }

  v_best_span_sequence.insert(v_best_span_sequence.begin(), &tree2tree_cells_.cells_.at(0).at(1));
  v_best_span_sequence.push_back(&tree2tree_cells_.cells_.at(v_source_words_.size() - 1).at(v_source_words_.size()));
  GlueMultipleNodeTranslation(tree2tree_cells_.cells_.at(0).at(v_source_words_.size()), v_best_span_sequence);
  return true;
}


bool Tree2TreeBasedDecoder::GetBestSpanSequenceForTreeParsing(vector<Tree2TreeCell *> &v_tree2tree_cell) {
  ////////////////////////////////////////////////////////////////////////// NEW
  MyTreeNode * p_root = (MyTreeNode *)v_cell_nodes_.at(0).v_tree_nodes_.at(0);
  GetBestSpanSequenceForTreeParsing(v_tree2tree_cell, p_root);
  return true;
}


float Tree2TreeBasedDecoder::GetBestSpanSequenceForTreeParsing(vector<Tree2TreeCell *> &v_tree2tree_cell, MyTreeNode * p_root) {
  ////////////////////////////////////////////////////////////////////////// NEW
  Tree2TreeCell * p_tree2tree_cell = &v_cell_nodes_.at(p_root->id_);
  if (p_tree2tree_cell->n_ > 0) {
    v_tree2tree_cell.push_back(p_tree2tree_cell);
    Tree2TreeHypothesis * p_best_translations = &p_tree2tree_cell->v_nbest_results_.v_tree2tree_hypothesis_[0];
    return p_best_translations->model_score_;
  } else if (0 == p_root->v_edges_.size()) {
    fprintf(stderr, "Wrong: float GetBestSpanSequenceForTreeParsing\n");
    return 0;
  }

  float best_score = (float)MODEL_SCORE_MIN;
  vector<Tree2TreeCell *> v_best_sequence;

  // Glue partial translations recursively
  for (int i = 0; i < p_root->v_edges_.size(); ++i) {
    MyTreeEdge * p_edge = p_root->v_edges_.at(i);
    float score = 0;
    vector<Tree2TreeCell *> v_child_sequence;
    for (int j = 0; j < p_edge->v_children_.size(); ++j) {
      MyTreeNode * p_child = p_edge->v_children_.at(j);
      score += GetBestSpanSequenceForTreeParsing(v_child_sequence, p_child);
      score += -1000;               // penatly
    }

    if (score > best_score) {
      best_score = score;
      v_best_sequence.clear();
      for (vector<Tree2TreeCell *>::iterator iter = v_child_sequence.begin(); iter != v_child_sequence.end(); ++iter) {
        v_best_sequence.push_back(*iter);
      }
    }
  }

  v_tree2tree_cell.insert(v_tree2tree_cell.end(), v_best_sequence.begin(), v_best_sequence.end());
  return best_score;
}





bool Tree2TreeBasedDecoder::TreeParsingDecoding() {
  for (int i = (int)v_cell_nodes_.size() - 1; i >= 0; --i) {
    GenerateTranslation(v_cell_nodes_.at(i));
  }

  // glue n-best candidates with <s> and </s>
  for (int i = 0; i < v_cell_nodes_.size(); ++i) {
    Tree2TreeCell * p_tree2tree_cell = &v_cell_nodes_.at(i);
    if (p_tree2tree_cell->begin_ > 1 || p_tree2tree_cell->end_ < v_source_words_.size() - 1) {
      break;
    }

    vector<Tree2TreeCell *> v_glue_items;
    v_glue_items.reserve(3);
    v_glue_items.push_back(&tree2tree_cells_.cells_.at(0).at(1));
    v_glue_items.push_back(p_tree2tree_cell);
    v_glue_items.push_back(&tree2tree_cells_.cells_.at(v_source_words_.size() - 1).at(v_source_words_.size()));

    GlueMultipleNodeTranslation(tree2tree_cells_.cells_.at(0).at(v_source_words_.size()), v_glue_items);

  }

  return true;
}


bool Tree2TreeBasedDecoder::GlueMultipleNodeTranslation(Tree2TreeCell &tree2tree_cell, vector<Tree2TreeCell *> &v_nodes) {
  int begin = tree2tree_cell.begin_;
  int end = tree2tree_cell.end_;

  bool entire_span_flag = ((begin <= 1) && (end >= v_source_words_.size() - 1)) ? true : false;
  int nbest_size = ((0 == begin) && (v_source_words_.size() == end)) ? p_tree2tree_config_->nbest_ : p_tree2tree_config_->beamsize_;
  bool complete_with_nonterminal = !entire_span_flag;

  if (v_nodes.size() < 2) {
    return false;
  }

  SlotInformation slot_information;
  int slot_number = 2;
  slot_information.Init(slot_number);

  Tree2TreeCell tmp_cell;
  tmp_cell.Init(tree2tree_cell.begin_, tree2tree_cell.end_, p_tree2tree_config_->beamsize_);
  tmp_cell.InitSyntax(tree2tree_cell.begin_, tree2tree_cell.end_, p_tree2tree_config_->beamsize_);

  Tree2TreeCell * current_cell_node = v_nodes.at(0);
  Tree2TreeCell * next_cell_node = v_nodes.at(1);

  // glue translation for the first two tree-nodes
  GlueTwoCells(tmp_cell, *current_cell_node, *next_cell_node, slot_information);

  bool check_rules = false;
  CompleteWithBeamPruning(tmp_cell, check_rules);
  tmp_cell.ReassignCellToHypothesisInBeam(tree2tree_cells_.cells_.at(current_cell_node->begin_).at(next_cell_node->end_));
  tmp_cell.l_translation_.clear();
  tmp_cell.begin_ = current_cell_node->begin_;
  tmp_cell.end_ = next_cell_node->end_;

  // then incrementally glue translations for other nodes
  for (int i = 2; i < v_nodes.size(); ++i) {
    Tree2TreeCell * p_target_cell_node = ( i == v_nodes.size() - 1 ) ? &tree2tree_cell : &tmp_cell;
    current_cell_node = v_nodes.at(i);
    GlueTwoCells(*p_target_cell_node, tmp_cell, *current_cell_node, slot_information);

    if (i < v_nodes.size() - 1) {
      bool check_rules = false;
      CompleteWithBeamPruning(*p_target_cell_node, check_rules);
      p_target_cell_node->ReassignCellToHypothesisInBeam(tree2tree_cells_.cells_.at(tmp_cell.begin_).at(current_cell_node->end_));
      p_target_cell_node->l_translation_.clear();
    }
  }
  return true;
}


bool Tree2TreeBasedDecoder::GlueTwoCells(Tree2TreeCell &tree2tree_cell, Tree2TreeCell &sub_cell_1, Tree2TreeCell &sub_cell_2, SlotInformation &slot_information) {
  vector<Tree2TreeCell *> v_p_cell;
  v_p_cell.resize(2);
  v_p_cell.at(0) = &sub_cell_1;
  v_p_cell.at(1) = &sub_cell_2;

  for (int i = 0; i < 2; ++i) {
    Tree2TreeCell * p_child_cell = v_p_cell.at(i);
    if (0 == p_child_cell->n_) {
      return false;
    }

    slot_information.v_slot_hypothesis_.at(i) = &p_child_cell->v_nbest_results_;
    slot_information.v_slot_hypothesis_pointer_.at(i) = &p_child_cell->v_nbest_results_.v_tree2tree_hypothesis_.at(0);
    slot_information.v_hypothesis_number_.at(i) = p_child_cell->n_;
    slot_information.v_inverted_nonterminal_index_.at(i) = i;
    slot_information.p_unit_rules_ = NULL;
  }
  bool have_generalization = false;
  bool for_non_lexical = false;
  HeuristicSearch(tree2tree_cell, NULL, slot_information, have_generalization, for_non_lexical);
  return true;
}


bool Tree2TreeBasedDecoder::GenerateTranslation(Tree2TreeCell &tree2tree_cell) {
  int begin = tree2tree_cell.begin_;
  int end = tree2tree_cell.end_;
  bool last_update = false;
  bool entire_span = (begin <= 1 && end >= v_source_words_.size() - 1) ? true : false;
  int nbest_size = ((0 == begin) && (end == v_source_words_.size())) ? p_tree2tree_config_->nbest_ : p_tree2tree_config_->beamsize_;            //?? in niutrans, nbest = 1
  bool completed_with_nontermianl_symbol = !entire_span;

  default_hypothesis_.Init(*p_tree2tree_config_, tree2tree_cell);
  string translation = "";
  default_hypothesis_.Create(translation, *p_tree2tree_model_);
  default_hypothesis_.model_score_ = (float)FLOAT_MIN;

  if (p_tree2tree_config_->tree_parsing_flag_) {      // tree-parsing
    for (int i = 0; i < tree2tree_cell.v_matched_pattern_.size(); ++i) {
      MatchedPattern * p_matched_pattern = &tree2tree_cell.v_matched_pattern_.at(i);
      UnitRules * p_unit_rules =  p_matched_pattern->p_unit_rules_;

      SlotInformation slot_information;
      bool have_generalization =  HaveGeneralization(begin, end, tree2tree_cell.v_matched_pattern_.at(i), slot_information);

      if (p_unit_rules->l_unit_rules_.size() > 0) {
        ApplyMatchedPattern(tree2tree_cell, tree2tree_cell.v_matched_pattern_.at(i), slot_information, have_generalization);
      }
    }
    bool check_rule = false;
    CompleteWithBeamPruning(tree2tree_cell, check_rule);
  } else {                                            // parsing
    //////////////////////////////////////////////////////////////////////////
  }

  // apply unary rules
  if (p_tree2tree_config_->allow_unary_production_flag_ && tree2tree_cell.end_ - tree2tree_cell.begin_ <= 1) {
    ApplyUnaryRulesInBeam(tree2tree_cell);
    bool check_rule = false;
    CompleteWithBeamPruning(tree2tree_cell, check_rule);
  }

  CompleteSpanForNonterminalSymbols(tree2tree_cell);


  return true;
}


bool Tree2TreeBasedDecoder::CompleteSpanForNonterminalSymbols(Tree2TreeCell &tree2tree_cell) {
  if (0 == tree2tree_cell.n_) {
    return false;
  }

  vector<Tree2TreeHypothesis *> v_p_nbest_results_new(tree2tree_cell.n_);
  for (size_t i = 0; i < tree2tree_cell.n_; ++i) {
    v_p_nbest_results_new.at(i) = &tree2tree_cell.v_nbest_results_.v_tree2tree_hypothesis_.at(i);
  }

  SortWithNonterminalAndModelScore(v_p_nbest_results_new, 0, tree2tree_cell.n_ - 1);

  int begin = 0;
  int end = begin;

  for (end = begin + 1; end < tree2tree_cell.n_; ++end) {
    Tree2TreeHypothesis * hypothesis_end_0 = v_p_nbest_results_new.at(end - 1);
    Tree2TreeHypothesis * hypothesis_end_1 = v_p_nbest_results_new.at(end);
    if (hypothesis_end_0->root_ != hypothesis_end_1->root_) {
      HypothesisList hypothesis_list;
      tree2tree_cell.m_hypothesis_list_with_symbol_[hypothesis_end_0->root_] = hypothesis_list;
      for (int i = 0; i < end - begin; ++i) {
        tree2tree_cell.m_hypothesis_list_with_symbol_[hypothesis_end_0->root_].v_tree2tree_hypothesis_.push_back(*v_p_nbest_results_new.at(begin + i));
      }
      begin = end;
    }
  }

  HypothesisList hypothesis_list;
  tree2tree_cell.m_hypothesis_list_with_symbol_[v_p_nbest_results_new.at(end - 1)->root_] = hypothesis_list;
  for (int i = 0; i < end - begin; ++i) {
    tree2tree_cell.m_hypothesis_list_with_symbol_[v_p_nbest_results_new.at(end - 1)->root_].v_tree2tree_hypothesis_.push_back(*v_p_nbest_results_new.at(begin + i));
  }

  for (int i = 0; i < tree2tree_cell.n_; ++i) {
    if (v_p_nbest_results_new.at(i)->root_ == "") {
      if ((0 == begin) && (1 == end)) {
        continue;
      }
      if ((v_source_words_.size() - 1 == begin) && (v_source_words_.size() == end)) {
        continue;
      }
      fprintf(stderr, "ERROR: No root label for span[%d, %d]!\n", tree2tree_cell.begin_, tree2tree_cell.end_);
      fflush(stderr);
    }
  }

  return true;
}


bool Tree2TreeBasedDecoder::SortWithNonterminalAndModelScore(vector<Tree2TreeHypothesis *> &v_p_nbest_result, int left, int right) {
  int i = left;
  int j = right;

  Tree2TreeHypothesis * tmp = NULL;
  Tree2TreeHypothesis * middle = v_p_nbest_result.at((left + right) / 2);

  while (i <= j) {
    while (i <= j && CompareWithNonterminalAndModelScore(*v_p_nbest_result.at(i), *middle) > 0) {
      ++i;
    }
    while (i <= j && CompareWithNonterminalAndModelScore(*v_p_nbest_result.at(j), *middle) < 0) {
      --j;
    }

    if (i <= j) {
      tmp = v_p_nbest_result.at(i);
      v_p_nbest_result.at(i) = v_p_nbest_result.at(j);
      v_p_nbest_result.at(j) = tmp;
      ++i;
      --j;
    }
  }

  if (left < j) {
    SortWithNonterminalAndModelScore(v_p_nbest_result, left, j);
  }
  if (i < right) {
    SortWithNonterminalAndModelScore(v_p_nbest_result, i, right);
  }
  return true;
}


int Tree2TreeBasedDecoder::CompareWithNonterminalAndModelScore(Tree2TreeHypothesis &tree2tree_hypothesis_1, Tree2TreeHypothesis &tree2tree_hypothesis_2) {
  int compare_value = 0;
  if (tree2tree_hypothesis_1.root_ > tree2tree_hypothesis_2.root_) {
    compare_value = 1;
  } else if (tree2tree_hypothesis_1.root_ < tree2tree_hypothesis_2.root_) {
    compare_value = -1;
  } else {
    compare_value = 0;
  }

  if (0 == compare_value) {
    if (tree2tree_hypothesis_1.model_score_ > tree2tree_hypothesis_2.model_score_) {
      compare_value = 1;
    } else if (tree2tree_hypothesis_1.model_score_ < tree2tree_hypothesis_2.model_score_) {
      compare_value = -1;
    } else {
      compare_value = 0;
    }
  }
  return compare_value;
}


bool Tree2TreeBasedDecoder::ApplyUnaryRulesInBeam(Tree2TreeCell &tree2tree_cell) {
  string root;
  if ((1 == tree2tree_cell.end_ - tree2tree_cell.begin_) && (0 == tree2tree_cell.begin_ || tree2tree_cell.end_ == v_source_words_.size())) {
    return false;
  }

  SlotInformation slot_information;
  slot_information.slot_number_ = 1;
  slot_information.v_slot_hypothesis_pointer_.resize(1);
  slot_information.v_inverted_nonterminal_index_.resize(1);

  for (int i = 0;i < tree2tree_cell.n_; ++i) {
    Tree2TreeHypothesis * p_tree2tree_hypothesis = &tree2tree_cell.v_nbest_results_.v_tree2tree_hypothesis_.at(i);
    slot_information.v_slot_hypothesis_pointer_.at(0) = p_tree2tree_hypothesis;
    slot_information.v_inverted_nonterminal_index_.at(0) = 0;
    root = "#" + p_tree2tree_hypothesis->root_;

    UnitRules *p_unit_rules = p_tree2tree_model_->scfg_model_.FindRuleListWithSymbol(root);
    if (NULL != p_unit_rules) {
      for (list<UnitRule>::iterator iter = p_unit_rules->l_unit_rules_.begin(); iter != p_unit_rules->l_unit_rules_.end(); ++iter) {
        UnitRule * p_unit_rule = &(*iter);
        Tree2TreeHypothesis tree2tree_hypothesis;

        bool have_generalization = false;
        GenerateHypothesisWithRule(tree2tree_cell, p_unit_rule, slot_information, have_generalization, tree2tree_hypothesis);
        tree2tree_cell.AddTree2TreeHypothesis(tree2tree_hypothesis);
      }
    }
  }
  return true;
}


bool Tree2TreeBasedDecoder::HaveGeneralization(int &begin, int &end, MatchedPattern &matched_pattern, SlotInformation &slot_information) {
  bool have_generalization = false;
  int number_slot = 0;
  vector<int> word_index_buf;
  word_index_buf.reserve(10000);
  for (int i = begin; i < end; ++i) {
//    if (number_slot < matched_pattern.matching_number_) {
    if (number_slot < matched_pattern.v_matching_.size()) {
      if (i >= matched_pattern.v_matching_[number_slot].left_ && i <= matched_pattern.v_matching_[number_slot].right_) {
        i = matched_pattern.v_matching_[number_slot].right_;
        ++number_slot;
        continue;
      }
    }
    word_index_buf.push_back(i);
    if (IsGeneralization(v_source_words_.at(i))) {
      have_generalization = true;
    }
  }
  slot_information.v_word_index_.insert(slot_information.v_word_index_.begin(), word_index_buf.begin(), word_index_buf.end());
  return have_generalization;
}


bool Tree2TreeBasedDecoder::ApplyMatchedPattern(Tree2TreeCell &tree2tree_cell, MatchedPattern &matched_pattern, SlotInformation &slot_information, bool &have_generalization) {
  UnitRules * p_unit_rules = matched_pattern.p_unit_rules_;
  if (p_tree2tree_config_->tree_parsing_flag_) {
    for (list<UnitRule>::iterator iter = p_unit_rules->l_unit_rules_.begin(); iter != p_unit_rules->l_unit_rules_.end(); ++iter) {
      UnitRule * p_unit_rule = &(*iter);
      UnitRules * p_unit_rules_with_symbol = p_tree2tree_model_->scfg_model_.FindRuleListWithSymbol(p_unit_rule->source_);
      ApplyTranslationRule(tree2tree_cell, p_unit_rules_with_symbol, matched_pattern, slot_information, have_generalization);
    }
  } else {
//    ApplyTranslationRule();
  }
  return true;
}


bool Tree2TreeBasedDecoder::ApplyTranslationRule(Tree2TreeCell &tree2tree_cell, UnitRules * p_unit_rules, MatchedPattern &matched_pattern, SlotInformation &slot_information, bool &have_generalization) {
  if (!p_tree2tree_config_->fast_decoding_flag_ || 0 == matched_pattern.v_matching_.size()) {
    for (list<UnitRule>::iterator iter = p_unit_rules->l_unit_rules_.begin(); iter != p_unit_rules->l_unit_rules_.end(); ++iter) {
      UnitRule * p_unit_rule = &(*iter);
      if (!InitSlotInformation(matched_pattern, p_unit_rule, slot_information)) {
        continue;
      }
      bool for_non_lexical = false;
      HeuristicSearch(tree2tree_cell, p_unit_rule, slot_information, have_generalization, for_non_lexical);
    }
  } else if (p_unit_rules->l_unit_rules_.size() > 0) {
    UnitRule * first_rule = &p_unit_rules->l_unit_rules_.front();
    if (!InitSlotInformation(matched_pattern, first_rule, slot_information)) {
      return true;
    }
    slot_information.p_unit_rules_ = p_unit_rules;
    bool for_non_lexical = false;
    HeuristicSearch(tree2tree_cell, NULL, slot_information, have_generalization, for_non_lexical);
  }
  return true;
}


bool Tree2TreeBasedDecoder::InitSlotInformation(MatchedPattern &matched_pattern, UnitRule * p_unit_rule, SlotInformation &slot_information) {
  if (NULL == p_unit_rule) {
    return false;
  }
  int slot_number = (int)matched_pattern.v_matching_.size();
  slot_information.slot_number_ = slot_number;
  slot_information.v_slot_hypothesis_.reserve(slot_number);
  slot_information.v_slot_hypothesis_pointer_.reserve(slot_number);
  slot_information.v_hypothesis_number_.reserve(slot_number);
  slot_information.v_inverted_nonterminal_index_.resize(slot_number);
  slot_information.p_unit_rules_ = NULL;
  slot_information.p_matched_pattern_ = &matched_pattern;
  vector<Nonterminal> * p_v_nonterminal = (p_unit_rule == NULL ? NULL : &p_unit_rule->v_nonterminal_);
  for (int i = 0; i < slot_number; ++i) {
    int begin = matched_pattern.v_matching_.at(i).left_;
    int end = matched_pattern.v_matching_.at(i).right_ + 1;
    Tree2TreeCell * child_cell = p_tree2tree_config_->tree_parsing_flag_ ? matched_pattern.v_matching_.at(i).cell_node_ : &tree2tree_cells_.cells_.at(begin).at(end);
    string nonterminal_symbol = p_v_nonterminal->at(i).symbol_;
    HypothesisList * p_hypothesis_list = NULL;
    if (child_cell->m_hypothesis_list_with_symbol_.find(nonterminal_symbol) != child_cell->m_hypothesis_list_with_symbol_.end()) {
      p_hypothesis_list = &child_cell->m_hypothesis_list_with_symbol_[nonterminal_symbol];
    } else {
      return false;
    }
    slot_information.v_slot_hypothesis_.push_back(p_hypothesis_list);
    slot_information.v_slot_hypothesis_pointer_.push_back(&p_hypothesis_list->v_tree2tree_hypothesis_.front());
    slot_information.v_hypothesis_number_.push_back((int)p_hypothesis_list->v_tree2tree_hypothesis_.size());
  }
  return true;
}


bool Tree2TreeBasedDecoder::HeuristicSearch(Tree2TreeCell &tree2tree_cell, UnitRule * p_unit_rule, SlotInformation &slot_information, bool &have_generalization, bool &for_non_lexical) {
  int nonterminal_count = slot_information.slot_number_;
  ExploredNode explored_node;
  explored_node.v_offsets_.resize(nonterminal_count);
  explored_node.p_unit_rules_ = slot_information.p_unit_rules_;

  if (NULL == p_unit_rule && NULL != slot_information.p_unit_rules_) {
    UnitRule * p_current_rule = &slot_information.p_unit_rules_->l_unit_rules_.front();
    GenerateHypothesisWithRule(tree2tree_cell, p_current_rule, slot_information, have_generalization, explored_node.tree2tree_hypothesis_);
    explored_node.last_updated_ = -1;
    explored_node.last_updated_slot_ = 0;
    explored_node.rule_offset_ = 0;
  } else {
    if (for_non_lexical && NULL == p_unit_rule && !HaveValidNullTranslation(slot_information)) {
      explored_node.tree2tree_hypothesis_.root_ = "";
    } else {
      GenerateHypothesisWithRule(tree2tree_cell, p_unit_rule, slot_information, have_generalization, explored_node.tree2tree_hypothesis_);
    }
    explored_node.last_updated_ = 0;
    explored_node.last_updated_slot_ = 0;
    explored_node.rule_offset_ = -1;
  }

  int count = 0;
  int computed_count = 0;
  int max_count = p_tree2tree_config_->beamsize_;
  int max_computed_count = max_count * 10;

  hypothesis_for_search_.clear();
  hypothesis_for_search_.insert(make_pair(explored_node.tree2tree_hypothesis_.model_score_, explored_node));

//  float new_score = explored_node.tree2tree_hypothesis_.model_score_ - 1;
//  hypothesis_for_search_.insert(make_pair(new_score, explored_node));

  if (for_non_lexical) {
    ClearExplored();
    SetExplored(explored_node);
  }

  // heuristics-based search
  while (hypothesis_for_search_.size() > 0) {
    multimap<float, ExploredNode>::iterator iter = --hypothesis_for_search_.end();

    if (IsAcceptable(iter->second.tree2tree_hypothesis_)) {
      tree2tree_cell.AddTree2TreeHypothesis(iter->second.tree2tree_hypothesis_);
      ++count;
    }
    ++computed_count;
    if (count > max_count + 10 || computed_count > max_computed_count) {
      break;
    }

    EnQueue(tree2tree_cell, iter->second, p_unit_rule, slot_information, have_generalization, for_non_lexical);
    hypothesis_for_search_.erase(iter);
  }

  if (p_tree2tree_config_->dump_left_hypothesis_flag_) {
    while(hypothesis_for_search_.size() > 0) {
      multimap<float, ExploredNode>::iterator iter = --hypothesis_for_search_.end();

      if (IsAcceptable(iter->second.tree2tree_hypothesis_)) {
        tree2tree_cell.AddTree2TreeHypothesis(iter->second.tree2tree_hypothesis_);
      }
      hypothesis_for_search_.erase(iter);
    }
  }
  return true;
}


bool Tree2TreeBasedDecoder::EnQueue(Tree2TreeCell &tree2tree_cell, ExploredNode &explored_node, UnitRule * p_unit_rule, SlotInformation &slot_information, bool &have_generalization, bool &for_non_lex) {
  int nonterminal_count = slot_information.slot_number_;
  UnitRules * p_unit_rules = explored_node.p_unit_rules_;
  for (int i = 0; i < nonterminal_count; ++i) {
    int offset = explored_node.v_offsets_.at(i);
    slot_information.v_slot_hypothesis_pointer_.at(i) = &slot_information.v_slot_hypothesis_.at(i)->v_tree2tree_hypothesis_.at(offset);
  }

  if (-1 == explored_node.last_updated_) {
    int new_offset = explored_node.rule_offset_ + 1;
    if (new_offset < p_unit_rules->l_unit_rules_.size()) {
      list<UnitRule>::iterator iter = p_unit_rules->l_unit_rules_.begin();
      advance(iter, new_offset);
      UnitRule * p_new_unit_rule = &(*iter);

      ExploredNode new_explored_node;
      CopyExploredNode(explored_node, nonterminal_count, new_explored_node);
      GenerateHypothesisWithRule(tree2tree_cell, p_new_unit_rule, slot_information, have_generalization, new_explored_node.tree2tree_hypothesis_);
      new_explored_node.rule_offset_ = new_offset;

      if ("" != new_explored_node.tree2tree_hypothesis_.root_) {
        hypothesis_for_search_.insert(make_pair(new_explored_node.tree2tree_hypothesis_.model_score_, new_explored_node));
      }
    }

    if (for_non_lex && new_offset > 1) {
      return true;
    }
  }

  if (!for_non_lex && explored_node.rule_offset_ >= 0) {
    list<UnitRule>::iterator iter =  p_unit_rules->l_unit_rules_.begin();
    advance(iter, explored_node.rule_offset_);
    p_unit_rule = &(*iter);
  }

  int updated_slot = for_non_lex ? explored_node.last_updated_slot_ : explored_node.last_updated_;

  for (int i = updated_slot; i < nonterminal_count; ++i) {
    if (-1 == i) {
      continue;
    }

    bool first_flag = true;
    bool failure_flag = false;

    UnitRules * p_unit_rules_backup = explored_node.p_unit_rules_;
    int old_offset = explored_node.v_offsets_.at(i);
    Tree2TreeHypothesis * hypothesis_backup = slot_information.v_slot_hypothesis_pointer_.at(i);

    for (int new_offset = old_offset + 1; new_offset < slot_information.v_hypothesis_number_.at(i); ++new_offset) {
      explored_node.v_offsets_.at(i) = new_offset;
      slot_information.v_slot_hypothesis_pointer_.at(i) = &slot_information.v_slot_hypothesis_.at(i)->v_tree2tree_hypothesis_.at(new_offset);
      Tree2TreeHypothesis * new_hypothesis = NULL;

      if (for_non_lex) {
        if (CheckExplored(explored_node)) {
          first_flag = false;
          continue;
        }

        failure_flag = false;
        explored_node.p_unit_rules_ = GetUnitRulesForSlotInformation(slot_information);

        if (NULL != explored_node.p_unit_rules_) {
          p_unit_rule = &explored_node.p_unit_rules_->l_unit_rules_.front();
        } else if (HaveValidNullTranslation(slot_information)) {
          p_unit_rule = NULL;
        } else {
          if (first_flag) {
            p_unit_rule = NULL;
            new_hypothesis = &default_hypothesis_;
          } else {
            continue;
          }
          failure_flag = true;
        }
      }

      ExploredNode new_explored_node;
      CopyExploredNode(explored_node, nonterminal_count, new_explored_node);

      if (!failure_flag) {
        GenerateHypothesisWithRule(tree2tree_cell, p_unit_rule, slot_information, have_generalization, new_explored_node.tree2tree_hypothesis_);
      } else {
        new_explored_node.tree2tree_hypothesis_.Copy(new_hypothesis);
      }

      if (!for_non_lex) {
        new_explored_node.last_updated_ = i;
      } else {
        new_explored_node.last_updated_ = (NULL == p_unit_rule ? i : -1);
        new_explored_node.last_updated_slot_ = (for_non_lex ? 0 : i);
        new_explored_node.rule_offset_ = 0;
        SetExplored(explored_node);
      }

      if ("" != new_explored_node.tree2tree_hypothesis_.root_) {
        hypothesis_for_search_.insert(make_pair(new_explored_node.tree2tree_hypothesis_.model_score_, new_explored_node));
        if (!failure_flag) {
          break;
        }
      }

      if (!for_non_lex) {
        break;
      }

      first_flag =false;
    }
    explored_node.p_unit_rules_ = p_unit_rules_backup;
    explored_node.v_offsets_.at(i) = old_offset;
    slot_information.v_slot_hypothesis_pointer_.at(i) = hypothesis_backup;
  }
  return true;
}


UnitRules * Tree2TreeBasedDecoder::GetUnitRulesForSlotInformation(SlotInformation &slot_information) {
  string source_side_buffer = "#" + slot_information.v_slot_hypothesis_pointer_.at(0)->root_ + " #" + slot_information.v_slot_hypothesis_pointer_.at(1)->root_;
  return p_tree2tree_model_->scfg_model_.FindRuleListWithSymbol(source_side_buffer);
}


bool Tree2TreeBasedDecoder::CheckExplored(ExploredNode &explored_node) {
  return (0 != explored_.at(explored_node.v_offsets_.at(0)).at(explored_node.v_offsets_.at(1)));
}


bool Tree2TreeBasedDecoder::CopyExploredNode(ExploredNode &explored_node, int &nonterminal_count, ExploredNode &new_explored_node) {
//  fprintf(stderr, "CopyExploredNode does not boot!\n");
//  fflush(stderr);
  new_explored_node.v_offsets_.insert(new_explored_node.v_offsets_.begin(), explored_node.v_offsets_.begin(), explored_node.v_offsets_.end());
  new_explored_node.last_updated_ = explored_node.last_updated_;
  new_explored_node.last_updated_slot_ = explored_node.last_updated_slot_;
  new_explored_node.rule_offset_ = explored_node.rule_offset_;
  new_explored_node.p_unit_rules_ = explored_node.p_unit_rules_;
  return true;
}


bool Tree2TreeBasedDecoder::IsAcceptable(Tree2TreeHypothesis &tree2tree_hypothesis) {
  if ("" == tree2tree_hypothesis.root_) {
    return false;
  } else if (tree2tree_hypothesis.model_score_ <= p_tree2tree_config_->model_score_min_) {
    return false;
  }

  if (0 == tree2tree_hypothesis.tree2tree_cell_->begin_ && v_source_words_.size() == tree2tree_hypothesis.tree2tree_cell_->end_) {
    if (!p_tree2tree_model_->scfg_model_.IsComplete(tree2tree_hypothesis.root_)) {
      return false;
    }
  }
  return true;
}


bool Tree2TreeBasedDecoder::ClearExplored() {
  for (int i = 0; i < p_tree2tree_config_->beamsize_; ++i) {
    explored_.at(i).resize(p_tree2tree_config_->beamsize_, 0);
  }
  return true;
}


bool Tree2TreeBasedDecoder::SetExplored(ExploredNode &explored_node) {
  explored_.at(explored_node.v_offsets_.at(0)).at(explored_node.v_offsets_.at(1)) = 1;
  return true;
}


bool Tree2TreeBasedDecoder::HaveValidNullTranslation(SlotInformation &slot_information) {
  fprintf(stderr, "HaveValidNullTranslation does not boot!\n");
  fflush(stderr);
  return true;
}


//bool Tree2TreeBasedDecoder::GenerateHypothesisWithRule(Tree2TreeCell &tree2tree_cell, UnitRule * p_unit_rule, SlotInformation &slot_information, bool &have_generalization, ExploredNode &explored_node) {
bool Tree2TreeBasedDecoder::GenerateHypothesisWithRule(Tree2TreeCell &tree2tree_cell, UnitRule * p_unit_rule, SlotInformation &slot_information, bool &have_generalization, Tree2TreeHypothesis &tree2tree_hypothesis) {
  if (NULL == p_unit_rule) {
    if (2 == slot_information.slot_number_) {
      Tree2TreeHypothesis * p_tree2tree_hypothesis_0 = slot_information.v_slot_hypothesis_pointer_.at(0);
      Tree2TreeHypothesis * p_tree2tree_hypothesis_1 = slot_information.v_slot_hypothesis_pointer_.at(1);
      fprintf(stderr, "GenerateHypothesisWithRule does not boot!\n");
      fflush(stderr);
    }
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
  }

  float n_gram_language_model_score = 0;
  int word_count = 0;
  int nonterminal_count = 0;

  // for n-gram language model
  for (int i = 0; i < p_unit_rule->target_word_count_; ++i) {
    if (p_unit_rule->v_word_id_.at(i) < 0) {
      int variable_index = -(p_unit_rule->v_word_id_.at(i) + 1);
      Tree2TreeHypothesis * current_hypothesis = slot_information.v_slot_hypothesis_pointer_.at(variable_index);
      slot_information.v_inverted_nonterminal_index_.at(nonterminal_count) = variable_index;

      if (0 == current_hypothesis->target_word_count_ && !p_tree2tree_config_->allow_null_substitution_flag_) {
        return NULL;
      }

//      word_id_buf_.insert(word_id_buf_.begin() + word_count, current_hypothesis->v_target_word_id_.begin(), current_hypothesis->v_target_word_id_.end());

//      memcpy(word_id_buf_ + word_count, current_hypothesis->v_target_word_id_.begin(), sizeof(int) * current_hypothesis->target_word_count_);

      for (int i = 0; i < current_hypothesis->target_word_count_; ++i) {
        word_id_buf_[word_count + i] = current_hypothesis->v_target_word_id_.at(i);
      }


      for (int k = 0; k < current_hypothesis->target_word_count_; ++k) {
        int end = word_count + 1;
        int begin = (end - p_tree2tree_config_->ngram_ < 0) ? 0 : end - p_tree2tree_config_->ngram_;
        if (k + 1 >= p_tree2tree_config_->ngram_) {
          language_model_score_buf_.at(word_count) = current_hypothesis->v_target_n_gram_lm_score_.at(k);
        } else {
          language_model_score_buf_.at(word_count) = p_tree2tree_model_->ngram_language_model_.GetProbability(word_id_buf_, begin, end);
        }
#ifdef WIN32
        if (language_model_score_buf_.at(word_count) < -20) {
#else
        if (language_model_score_buf_.at(word_count) < -20 || isnan(language_model_score_buf_.at(word_count))) {
#endif
          language_model_score_buf_.at(word_count) = -20;
        }
        n_gram_language_model_score += language_model_score_buf_.at(word_count);
        ++word_count;
      }
      ++nonterminal_count;
    } else {
//      word_id_buf_.at(word_count) = p_unit_rule->v_word_id_.at(i);
//      word_id_buf_.push_back(p_unit_rule->v_word_id_.at(i));
      word_id_buf_[word_count] = p_unit_rule->v_word_id_.at(i);

      int end = word_count + 1;
      int begin = end - p_tree2tree_config_->ngram_ < 0 ? 0 : end - p_tree2tree_config_->ngram_;
      language_model_score_buf_.at(word_count) = p_tree2tree_model_->ngram_language_model_.GetProbability(word_id_buf_, begin, end);
#ifdef WIN32
      if (language_model_score_buf_.at(word_count) < -20) {
#else
      if (language_model_score_buf_.at(word_count) < -20 || isnan(language_model_score_buf_.at(word_count))) {
#endif
        language_model_score_buf_.at(word_count) = -20;
      }
      n_gram_language_model_score += language_model_score_buf_.at(word_count);
      ++word_count;
    }
  }

  tree2tree_hypothesis.Init(*p_tree2tree_config_, tree2tree_cell);
  tree2tree_hypothesis.v_target_word_id_.resize(word_count, 0);
  for (int i = 0; i < word_count; ++i) {
    tree2tree_hypothesis.v_target_word_id_.at(i) = word_id_buf_[i];
  }
  tree2tree_hypothesis.v_target_n_gram_lm_score_.insert(tree2tree_hypothesis.v_target_n_gram_lm_score_.begin(), language_model_score_buf_.begin(), language_model_score_buf_.begin() + word_count);
  tree2tree_hypothesis.target_word_count_ = word_count;

  // n-gram language model
  tree2tree_hypothesis.v_feature_values_.at(0) = n_gram_language_model_score;
  tree2tree_hypothesis.v_feature_values_.at(1) = (float)tree2tree_hypothesis.target_word_count_;

  // translation
  GenerateTranslationWithRule(p_unit_rule, slot_information, tree2tree_hypothesis.translation_result_, tree2tree_hypothesis.translation_length_);

  if (have_generalization && p_tree2tree_config_->replace_generalization_flag_) {
    ReplaceGeneralizationTranslation(slot_information, tree2tree_hypothesis.translation_result_);
    tree2tree_hypothesis.translation_length_ = (int)tree2tree_hypothesis.translation_result_.size();
  }

  // model score
  float overall_score = 0;

  // score of substituted variables
  for (int variable = 0; variable < nonterminal_count; ++variable) {
    int variable_index = slot_information.v_inverted_nonterminal_index_.at(variable);
    Tree2TreeHypothesis * current_hypothesis = slot_information.v_slot_hypothesis_pointer_.at(variable_index);
    for (int f = 0; f < p_tree2tree_config_->tree2tree_features_.features_number_; ++f) {
      if ((0 != f) && (1 != f)) {
        tree2tree_hypothesis.v_feature_values_.at(f) = current_hypothesis->v_feature_values_.at(f);
      }
    }
  }

  // score of the rule
  if (p_unit_rule->is_complete_) {
    tree2tree_hypothesis.ComputeFeatureScores(*p_unit_rule);
  } else {
    ComputeUnCompleteOverallScore(p_unit_rule, *p_tree2tree_config_, overall_score);
  }

  for (int f = 0; f < p_tree2tree_config_->tree2tree_features_.features_number_; ++f) {
    overall_score += tree2tree_hypothesis.v_feature_values_.at(f) * p_tree2tree_config_->tree2tree_features_.v_feature_values_.at(f).weight_;
  }

  tree2tree_hypothesis.model_score_ = overall_score;

  tree2tree_hypothesis.root_ = p_unit_rule->root_;
  tree2tree_hypothesis.left_partial_translation_ = NULL;
  tree2tree_hypothesis.right_partial_translation_ = NULL;

  if (nonterminal_count > 0) {
    if (nonterminal_count <= 2) {
      tree2tree_hypothesis.left_partial_translation_ = slot_information.v_slot_hypothesis_pointer_.at(0);
    }
    if (2 == nonterminal_count) {
      tree2tree_hypothesis.right_partial_translation_ = slot_information.v_slot_hypothesis_pointer_.at(1);
    }
  }

  tree2tree_hypothesis.p_rule_used_ = p_unit_rule;
  tree2tree_hypothesis.p_matched_pattern_ = slot_information.p_matched_pattern_;
  return true;
}


bool Tree2TreeBasedDecoder::ComputeUnCompleteOverallScore(UnitRule * p_unit_rule, Tree2TreeConfiguration &tree2tree_config, float &overall_score) {
  overall_score += p_unit_rule->scores_of_translation_option_.pr_tgt_given_src_ * tree2tree_config.tree2tree_features_.v_feature_values_.at(2).weight_ +
                   p_unit_rule->scores_of_translation_option_.lex_tgt_given_src_ * tree2tree_config.tree2tree_features_.v_feature_values_.at(3).weight_ +
                   p_unit_rule->scores_of_translation_option_.pr_src_given_tgt_ * tree2tree_config.tree2tree_features_.v_feature_values_.at(4).weight_ +
                   p_unit_rule->scores_of_translation_option_.lex_src_given_tgt_ * tree2tree_config.tree2tree_features_.v_feature_values_.at(5).weight_ +
                   p_unit_rule->scores_of_translation_option_.number_of_rules_ * tree2tree_config.tree2tree_features_.v_feature_values_.at(6).weight_ +
                   p_unit_rule->scores_of_translation_option_.bi_lex_links_ * tree2tree_config.tree2tree_features_.v_feature_values_.at(7).weight_ +
                   p_unit_rule->scores_of_translation_option_.number_of_phrasal_rules_ * tree2tree_config.tree2tree_features_.v_feature_values_.at(9).weight_ +
                   p_unit_rule->scores_of_translation_option_.pr_r_give_rootr_ * tree2tree_config.tree2tree_features_.v_feature_values_.at(11).weight_ +
                   p_unit_rule->scores_of_translation_option_.pr_sr_give_rootr_ * tree2tree_config.tree2tree_features_.v_feature_values_.at(12).weight_ +
                   p_unit_rule->scores_of_translation_option_.is_lexicalized_ * tree2tree_config.tree2tree_features_.v_feature_values_.at(13).weight_ +
                   p_unit_rule->scores_of_translation_option_.is_composed_ * tree2tree_config.tree2tree_features_.v_feature_values_.at(14).weight_ +
                   p_unit_rule->scores_of_translation_option_.is_low_frequency_ * tree2tree_config.tree2tree_features_.v_feature_values_.at(15).weight_;

/*
  float number_of_target_words_;  // number of target words, feature '1'
  float number_of_phrasal_rules_; // number of phrasal_rules, feature '9'
  float pr_tgt_given_src_;        // Pr(e|f), f->e translation probability, table '0', feature '2'
  float lex_tgt_given_src_;       // Lex(e|f), f->e lexical weight, table '1', feature '3'
  float pr_src_given_tgt_;        // Pr(f|e), e->f translation probability, table '2', feature '4'
  float lex_src_given_tgt_;       // Lex(f|e), e->f lexical weight, table '3', feature '5'
  float number_of_rules_;         // Natural Logarithm e, number of rules, table '4', feature '6'
  float pr_r_give_rootr_;         // Pr(r|root(r)), table '5', feature '11'
  float pr_sr_give_rootr_;        // Pr(s(r)|root(r)), table '6', feature '12'
  float is_lexicalized_;          // IsLexicalized, table '7', feature '13'
  float is_composed_;             // IsComposed, table '8', feature '14'
  float is_low_frequency_;        // IsLowFrequency, table '9', feature '15'
  float bi_lex_links_;            // number of bi-lex links (not fired in current version, 0), table '10', feature '7'
*/
  return true;
}


bool Tree2TreeBasedDecoder::ReplaceGeneralizationTranslation(SlotInformation &slot_information, string &translation) {
  fprintf(stderr, "ReplaceGeneralizationTranslation does not boot!\n");
  fflush(stderr);
  return true;
}


bool Tree2TreeBasedDecoder::GenerateTranslationWithRule(UnitRule * p_unit_rule, SlotInformation &slot_information, string &translation, int &translation_length) {
  if (p_unit_rule->nonterminal_count_ >= 2 || p_unit_rule->nonterminal_count_ >= 1) {
    fprintf(stderr, "Nonterminal count >= 2\n");
    fflush(stderr);
  }

  string target = p_unit_rule->target_;
  int length = (int)target.size();
  int begin_position = (int)target.find('#');
  int end_position = begin_position + 1;
  string last = target;
  int last_position = 0;
  int target_length = 0;
  int nonterminal_count = 0;
  translation_buf_.clear();

  while (string::npos != begin_position && begin_position > 0 && ' ' != target.at(begin_position - 1)) {
   begin_position = (int)target.find('#', begin_position + 1);
   end_position = begin_position + 1;
  }

  while (string::npos != begin_position) {
    end_position = begin_position + 1;
    while ((target.size() != end_position) && (' ' != target.at(end_position))) {
      ++end_position;
    }
    if (end_position != begin_position + 1) {
      translation_buf_.insert(translation_buf_.begin() + target_length, last.begin(), last.begin() + begin_position - last_position);
      target_length += begin_position - last_position;
      int variable_index = slot_information.v_inverted_nonterminal_index_.at(nonterminal_count);
      Tree2TreeHypothesis * current_hypothesis = slot_information.v_slot_hypothesis_pointer_.at(variable_index);
      int slot_translation_length = (int)current_hypothesis->translation_result_.size();

      translation_buf_.insert(translation_buf_.begin() + target_length, current_hypothesis->translation_result_.begin(), current_hypothesis->translation_result_.end());
      target_length += slot_translation_length;

      if ((target.size() != end_position) && slot_translation_length > 0) {
        translation_buf_.push_back(' ');
        ++target_length;
      }
      if (end_position + 1 < target.size()) {
        last = target.substr(end_position + 1);
      } else {
        last = "";
      }
      last_position = end_position + 1;
      ++nonterminal_count;
    }
    begin_position = (int)target.find('#', end_position);
    while (string::npos != begin_position && begin_position > 0 && ' ' != target.at(begin_position - 1)) {
      begin_position = (int)target.find('#', begin_position + 1);
    }
  }

  begin_position = length;
  if (begin_position - last_position > 0) {
    translation_buf_.insert(translation_buf_.begin() + target_length, last.begin(), last.begin() + begin_position - last_position);
    target_length += begin_position - last_position;
  }

//  if (' ' == translation_buf_.at(target_length - 1)) {
//    --target_length;
//  }

  translation.insert(translation.begin(), translation_buf_.begin(), translation_buf_.begin() + target_length);
  RmEndSpace(translation);
  translation_length = (int)translation.size();
  return true;
}


bool Tree2TreeBasedDecoder::IsGeneralization(string &source) {
  if (("$time" == source) || ("$number" == source) || ("$date" == source)) {
    return true;
  } else {
    return false;
  }
}


bool Tree2TreeBasedDecoder::CkyDecoding() {
  fprintf(stderr, "CkyDecoding is not booted...\n");
  fflush(stderr);
  return true;
}




bool Tree2TreeBasedDecoder::LoadPrematchedRules() {
  if (decoding_sentence_.l_matched_rules_.empty()) {
    return true;
  }
  fprintf(stderr, "LoadPrematchedRules Not Boot...\n");
  fflush(stderr);
  
  /*
  for (list<SimpleRule>::iterator iter = decoding_sentence_.l_matched_rules_.begin(); iter != decoding_sentence_.l_matched_rules_.end(); ++iter) {
    SimpleRule * p_simple_rule = &(*iter);
  }
  */
  return true;
}


bool Tree2TreeBasedDecoder::LoadViterbiRules() {
  if (decoding_sentence_.l_viterbi_rules_.empty()) {
    return true;
  }
  fprintf(stderr, "LoadViterbiRules Not Boot...\n");
  fflush(stderr);
  return true;
}



bool Tree2TreeBasedDecoder::ParseSourceSentence() {
  vector<string> v_decoding_sentence_domains;
  SplitWithStr(decoding_sentence_.sentence_, " |||| ", v_decoding_sentence_domains);
  source_sentence_ = v_decoding_sentence_domains.at(0);
  RmEndSpace(source_sentence_);
  if("" != source_sentence_) {
    source_sentence_ = "<s> " + source_sentence_ + " </s>";
  } else {
    source_sentence_ = "<s> </s>";
  }
  Split(source_sentence_, ' ', v_source_words_);
  v_source_words_information_.resize(v_source_words_.size(), false);
  for (int i = 0; i < v_source_words_.size(); ++i) {
    if (i > 0 && i < v_source_words_.size() - 1) {
      v_source_words_information_[i] = StringUtil::IsLiteral(v_source_words_.at(i).c_str());
    } else {
      v_source_words_information_[i] = false;
    }
  }
  // source tree (for syntax-based decoding)
  if (v_decoding_sentence_domains.size() > 2) {
//    source_tree_->CreateTree(v_decoding_sentence_domains.at(2).c_str());
    my_source_tree_.CreateTree(v_decoding_sentence_domains.at(2));
  }
  return true;
}


//bool Tree2TreeBasedDecoder::MatchScfgRules(Tree2TreeConfiguration &tree2tree_config, Tree2TreeModel &tree2tree_model) {
bool Tree2TreeBasedDecoder::MatchScfgRules() {
  if (p_tree2tree_config_->tree_parsing_flag_) {
    /*
    if (NULL == source_tree_) {
      fprintf(stderr, "ERROR: Source-tree is empty!\n");
      fflush(stderr);
      exit(1);
    }
    */
    if (NULL == my_source_tree_.root_) {
      fprintf(stderr, "ERROR: My source-tree is empty!\n");
      fflush(stderr);
      exit(1);
    }
    MatchRulesForTreeParsing();
    return true;
  }

  return true;
}


//bool Tree2TreeBasedDecoder::AddBasicRules(Tree2TreeConfiguration &tree2tree_config) {
bool Tree2TreeBasedDecoder::AddBasicRules() {
  // Processing nknow words
  string phrase;
  int max_phrase_length = 1;
  for (int begin = 0; begin < v_source_words_.size(); ++begin) {
    bool is_generalization = false;
    bool is_forced = false;
    for (int end = begin + 1; end - begin <= max_phrase_length && end <= v_source_words_.size(); ++end) {
      if (end == begin + 1) {
        phrase = v_source_words_.at(end - 1);
      } else {
        phrase = phrase + " " + v_source_words_.at(end - 1);
      }

      if (tree2tree_cells_.cells_.at(end - 1).at(end).have_forced_) {
        is_forced = true;
      } else if (is_forced) {
        break;
      }

      if (!p_tree2tree_config_->tree_parsing_flag_ || IsSentenceEnd(tree2tree_cells_.cells_.at(begin).at(end))) {
        ProcessUnknownWord(tree2tree_cells_.cells_.at(begin).at(end), phrase);
        bool check_rule = false;
        CompleteWithBeamPruning(tree2tree_cells_.cells_.at(begin).at(end), check_rule);
      }
    }
  }
  return true;
}


bool Tree2TreeBasedDecoder::ProcessUnknownWord(Tree2TreeCell& tree2tree_cell, string &phrase) {
  UnitRules * p_unit_rules = p_tree2tree_model_->scfg_model_.FindRuleList(phrase);
  bool use_unknown_list = false;

  if (NULL == p_unit_rules || 0 == p_unit_rules->l_unit_rules_.size()) {
    ToLower(phrase);
    p_unit_rules = p_tree2tree_model_->scfg_model_.FindRuleList(phrase);
  }

  if (NULL != p_unit_rules && !p_tree2tree_model_->scfg_model_.IsMetaSymbol(phrase)) {
    return true;
  }

  if (NULL == p_unit_rules || 0 == p_unit_rules->l_unit_rules_.size()) {
    if (tree2tree_cell.l_translation_.size() > 0) {
      return true;
    }
    string unknown_word = "<unk>";
    p_unit_rules = p_tree2tree_model_->scfg_model_.FindRuleList(unknown_word);     // unknown words
    use_unknown_list = true;
  }

  for (list<UnitRule>::iterator iter = p_unit_rules->l_unit_rules_.begin(); iter != p_unit_rules->l_unit_rules_.end(); ++iter) {
    string label = "";
    bool output_oov_flag = use_unknown_list && p_tree2tree_config_->output_oov_;
    AddCellTranslation(tree2tree_cell, *iter, output_oov_flag, label);
  }
  return true;
}


unsigned int Tree2TreeBasedDecoder::CompleteWithBeamPruning(Tree2TreeCell& tree2tree_cell, bool &check_rule) {
  unsigned int update_count = 0;
  ++tree2tree_cell.to_l_span_update_count_;
  if (tree2tree_cell.l_translation_.size() > 0) {
    tree2tree_cell.l_translation_.sort(CompareTranslationList);
  }

  int count_tmp = (int)tree2tree_cell.l_translation_.size();
  if (p_tree2tree_config_->beamsize_ > 0 && tree2tree_cell.l_translation_.size() > p_tree2tree_config_->beamsize_) {
    count_tmp = p_tree2tree_config_->beamsize_;
  }

  if (count_tmp > tree2tree_cell.n_) {
    tree2tree_cell.v_nbest_results_.v_tree2tree_hypothesis_.resize(count_tmp);
  }

  tree2tree_cell.n_ = count_tmp;

  int in_complete_hypothesis_number = 0;
  int max_in_complete_hypothesis_number = (int)(tree2tree_cell.l_translation_.size() > tree2tree_cell.n_ ? tree2tree_cell.n_ * p_tree2tree_model_->scfg_model_.in_complete_hypothesis_rate_ : tree2tree_cell.n_);
  int max_symbol_number = max_num_of_symbol_with_same_trans_;

  bool ok_flag = false;

  vector<Tree2TreeHypothesis *> v_n_tmp(tree2tree_cell.n_);
  int count = 0;
  for (list<Tree2TreeHypothesis>::iterator iter = tree2tree_cell.l_translation_.begin(); iter != tree2tree_cell.l_translation_.end(); ++iter) {
    Tree2TreeHypothesis * p_tree2tree_hypothesis = &(*iter);
    int symbol_translation_count = 0;

    if (check_rule && !CheckHypothesisAvailability(p_tree2tree_hypothesis)) {
      continue;
    }

    bool duplicated = false;
    for (int j = 0; j < count; ++j) {
      if (p_tree2tree_hypothesis->translation_result_ == v_n_tmp.at(j)->translation_result_) {
        if (symbol_translation_count >= max_symbol_number) {
          duplicated = true;
          ++symbol_translation_count;
          break;
        }
      }
    }

    if (!duplicated) {
      if (!p_tree2tree_model_->scfg_model_.IsComplete(p_tree2tree_hypothesis->root_)) {
        if (in_complete_hypothesis_number > max_in_complete_hypothesis_number) {
          ++in_complete_hypothesis_number;
          continue;
        }
      }
      v_n_tmp[count] = p_tree2tree_hypothesis;
      ++count;
    }
    if (count >= tree2tree_cell.n_) {
      break;
    }
  }

  for (int i = count - 1; i >= 0; --i) {
/*
    Tree2TreeHypothesis * p_tree2tree_hypothesis = v_n_tmp.at(i);
    tree2tree_cell.v_nbest_results_.at(i).Copy(p_tree2tree_hypothesis);
    ++update_count;
    ++tree2tree_cell.to_l_hypothesis_update_count_;
    */

    Tree2TreeHypothesis * p_tree2tree_hypothesis = v_n_tmp.at(i);
    if ("" == tree2tree_cell.v_nbest_results_.v_tree2tree_hypothesis_.at(i).root_) {
      tree2tree_cell.v_nbest_results_.v_tree2tree_hypothesis_.at(i).Copy(p_tree2tree_hypothesis);
      ++update_count;
      ++tree2tree_cell.to_l_hypothesis_update_count_;
    } else if (tree2tree_cell.v_nbest_results_.v_tree2tree_hypothesis_.at(i) == *p_tree2tree_hypothesis) {
      ;
    } else {
      tree2tree_cell.v_nbest_results_.v_tree2tree_hypothesis_.at(i).Copy(p_tree2tree_hypothesis);
      ++update_count;
      ++tree2tree_cell.to_l_hypothesis_update_count_;
    }
  }

  if (tree2tree_cell.n_ > count) {
    tree2tree_cell.n_ = count;
  }

  tree2tree_cell.l_translation_.clear();
  tree2tree_cell.l_translation_.insert(tree2tree_cell.l_translation_.begin(), tree2tree_cell.v_nbest_results_.v_tree2tree_hypothesis_.begin(), tree2tree_cell.v_nbest_results_.v_tree2tree_hypothesis_.end());

  return update_count;
}


bool Tree2TreeBasedDecoder::CheckHypothesisAvailability(Tree2TreeHypothesis * p_tree2tree_hypothesis) {
  if (NULL == p_tree2tree_hypothesis) {
    return false;
  }
  if (NULL == p_tree2tree_hypothesis->p_rule_used_) {
    return true;
  } else {
    UnitRule * p_unit_rule = p_tree2tree_hypothesis->p_rule_used_;
    string source = p_unit_rule->p_parent_rule_->source_;
    return CheckAvailability(source);
  }
}


bool Tree2TreeBasedDecoder::CheckAvailability(string &source) {
  fprintf(stderr, "CheckAvailability Not Boot.!\n");
  fflush(stderr);
  return false;
}


bool Tree2TreeBasedDecoder::MatchRulesForTreeParsing() {
  int max_node_number = (int)my_source_tree_.v_node_base_.size();
  cell_node_number_ = 0;
  v_cell_nodes_.resize(max_node_number);

  for (int i = 0;i < max_node_number; ++i) {
    MyTreeNode * tree_node = &my_source_tree_.v_node_base_.at(i);
    int end = tree_node->end_ + 1;
    v_cell_nodes_.at(i).Init(tree_node->begin_, end, p_tree2tree_config_->beamsize_);
    v_cell_nodes_.at(i).InitSyntax(tree_node->begin_, end, p_tree2tree_config_->beamsize_);

    v_cell_nodes_.at(i).v_tree_nodes_.push_back(tree_node);
    SetTreeNodeNoRecursion(tree_node);
  }

//  source_tree_->GenerateTreeFragments(tree2tree_config.generate_tree_structure_flag_);
  my_source_tree_.GenerateTreeFragments(p_tree2tree_config_->generate_tree_structure_flag_);  // to generate tree-fragments for each node

  SetUserTranslateForTreeParsing();

  for(int i = 0; i < max_node_number; ++i) {
    MatchRulesForTreeParsing(v_cell_nodes_.at(i));
  }

  return true;
}


bool Tree2TreeBasedDecoder::MatchRulesForTreeParsing(Tree2TreeCell &tree2tree_cell) {
  bool no_matched = true;
  for (int i = 0; i < tree2tree_cell.v_tree_nodes_.size(); ++i) {
    MyTreeNode * p_tree_node = tree2tree_cell.v_tree_nodes_.at(i);
    if (0 == p_tree_node->v_tree_fragments_.size()) {
      continue;
    }
    for (int j = 1; j < p_tree_node->v_tree_fragments_.size(); ++j) {
      if (AddMatchedPatternUsingTreeFragment(tree2tree_cell, p_tree_node->v_tree_fragments_.at(j))) {
        no_matched = false;
      }
    }
  }

  if (no_matched && tree2tree_cell.l_translation_.size() == 0) {
    MyTreeNode * current_tree_node = tree2tree_cell.v_tree_nodes_.at(0);
    if (0 == current_tree_node->v_edges_.size()) {      // unknown word
      string unknown_word = "<unk>";
      UnitRules * p_unit_rules = p_tree2tree_model_->scfg_model_.FindRuleList(unknown_word);
      for (list<UnitRule>::iterator iter = p_unit_rules->l_unit_rules_.begin(); iter != p_unit_rules->l_unit_rules_.end(); ++iter) {
        string label;
        string target_symbol = "NP";
        GetDefaultRootLabel(tree2tree_cell, target_symbol, label);
        AddCellTranslation(tree2tree_cell, *iter, p_tree2tree_config_->output_oov_, label);
      }
    }
  }
  return true;
}


bool Tree2TreeBasedDecoder::AddMatchedPatternUsingTreeFragment(Tree2TreeCell &tree2tree_cell, MyTreeFragment &tree_fragment) {
  string key;
  UnitRules * p_unit_rules;
  if (p_tree2tree_config_->generate_tree_structure_flag_) {
    ;
  } else {
    key = "#" + tree_fragment.root_->label_ + " ( " + tree_fragment.frontier_sequence_ + " )";
    p_unit_rules = p_tree2tree_model_->scfg_model_.FindRuleList(key);
  }

  if (NULL == p_unit_rules) {
    return false;
  }

  MatchedPattern matched_pattern;
  matched_pattern.key_ = tree_fragment.frontier_sequence_;
  matched_pattern.v_matching_.reserve(tree_fragment.non_terminal_number_);
  for (list<MyTreeNode *>::iterator iter = tree_fragment.l_frontier_non_terminals_.begin(); iter != tree_fragment.l_frontier_non_terminals_.end(); ++iter) {
    MyTreeNode * p_child_node = *iter;
    NonTerminalBoundary non_terminal_boundary(p_child_node->begin_, p_child_node->end_, &v_cell_nodes_.at(p_child_node->id_));
    matched_pattern.v_matching_.push_back(non_terminal_boundary);
  }
  matched_pattern.p_unit_rules_ = p_unit_rules;
  tree2tree_cell.AddMatchedPattern(matched_pattern);
  return true;
}


bool Tree2TreeBasedDecoder::AddCellTranslation(Tree2TreeCell &tree2tree_cell, UnitRule &unit_rule, bool &output_oov, string &label) {
  Tree2TreeHypothesis tree2tree_hypothesis;
  tree2tree_cell.l_translation_.push_back(tree2tree_hypothesis);
  Tree2TreeHypothesis * p_tree2tree_hypothesis = &tree2tree_cell.l_translation_.back();

  p_tree2tree_hypothesis->Init(*p_tree2tree_config_, tree2tree_cell);
  p_tree2tree_hypothesis->InitFeatureScores(unit_rule);

  p_tree2tree_hypothesis->translation_result_ = unit_rule.target_;
  p_tree2tree_hypothesis->translation_length_ = (int)p_tree2tree_hypothesis->translation_result_.size();

  if (output_oov) {
    if (p_tree2tree_config_->label_oov_) {
      p_tree2tree_hypothesis->translation_result_ = "<" + v_source_words_.at(tree2tree_cell.begin_) + ">";
    } else {
      p_tree2tree_hypothesis->translation_result_ = v_source_words_.at(tree2tree_cell.begin_);
    }
    p_tree2tree_hypothesis->translation_length_ = (int)p_tree2tree_hypothesis->translation_result_.size();
  }

  p_tree2tree_hypothesis->v_target_word_id_.insert(p_tree2tree_hypothesis->v_target_word_id_.begin(), unit_rule.v_word_id_.begin(), unit_rule.v_word_id_.end());
  p_tree2tree_hypothesis->target_word_count_ = unit_rule.target_word_count_;
  p_tree2tree_hypothesis->root_ = ("" == label ? unit_rule.root_ : label);

  ComputeModelScore(tree2tree_cell.l_translation_.back());
  return true;
}


bool Tree2TreeBasedDecoder::ComputeModelScore(Tree2TreeHypothesis &tree2tree_hypothesis) {
  if (0 == tree2tree_hypothesis.v_target_n_gram_lm_score_.size()) {
    tree2tree_hypothesis.v_target_n_gram_lm_score_.resize(tree2tree_hypothesis.target_word_count_, 0.0f);
  }

  // n-gram language model;
  for (int i = 0; i < tree2tree_hypothesis.target_word_count_; ++i) {
    int end = i + 1;
    int begin = end - p_tree2tree_config_->ngram_ > 0 ? end - p_tree2tree_config_->ngram_ : 0;
    float probability = p_tree2tree_model_->ngram_language_model_.GetProbability(tree2tree_hypothesis.v_target_word_id_, begin, end);
#ifdef WIN32
    if (probability < -20) {
#else
    if (probability < -20 || isnan(probability)) {
#endif
      probability = -20;
    }
    tree2tree_hypothesis.v_target_n_gram_lm_score_.at(i) = probability;
    tree2tree_hypothesis.v_feature_values_.at(0) += probability;
  }
  // model score
  for (int f = 0; f < p_tree2tree_config_->tree2tree_features_.features_number_; ++f) {
    tree2tree_hypothesis.model_score_ += tree2tree_hypothesis.v_feature_values_.at(f) * p_tree2tree_config_->tree2tree_features_.v_feature_values_.at(f).weight_;
    if (0 != f) {
      tree2tree_hypothesis.model_score_except_lm_ += tree2tree_hypothesis.v_feature_values_.at(f) * p_tree2tree_config_->tree2tree_features_.v_feature_values_.at(f).weight_;
    } else {
      tree2tree_hypothesis.language_model_score_ = tree2tree_hypothesis.v_feature_values_.at(f) * p_tree2tree_config_->tree2tree_features_.v_feature_values_.at(f).weight_;
    }
  }
  return true;
}


bool Tree2TreeBasedDecoder::GetDefaultRootLabel(Tree2TreeCell &tree2tree_cell, string &target_symbol, string &label) {
  if ((1 == tree2tree_cell.end_ - tree2tree_cell.begin_) && IsSentenceEnd(tree2tree_cell)) {
    if (0 == tree2tree_cell.begin_) {
      label = "<s>";
    } else {
      label = "</s>";
    }
    return true;
  }

  MyTreeNode * p_tree_node = (0 == tree2tree_cell.v_tree_nodes_.size()) ? NULL : tree2tree_cell.v_tree_nodes_.at(0);
  return GetDefaultRootLabel(p_tree_node, target_symbol, label);
}


bool Tree2TreeBasedDecoder::GetDefaultRootLabel(MyTreeNode * p_tree_node, string &target_symbol, string &label) {
  if (NULL == p_tree_node) {
    label = default_symbol_;
    return true;
  }
  label = target_symbol + "=" + p_tree_node->label_;
  return true;
}


bool Tree2TreeBasedDecoder::IsSentenceEnd(Tree2TreeCell &tree2tree_cell) {
  bool left_end = (0 == tree2tree_cell.begin_);
  bool right_end = (v_source_words_.size() == tree2tree_cell.end_);
  return left_end || right_end;
}





bool Tree2TreeBasedDecoder::SetTreeNodeNoRecursion(MyTreeNode *root) {
  if (NULL == root) {
    return false;
  }
  Tree2TreeCell * cell = &tree2tree_cells_.cells_.at(root->begin_).at(root->end_ + 1);
  bool exist = false;
  for (int i = 0; i < cell->v_tree_nodes_.size(); ++i) {
    MyTreeNode * node = cell->v_tree_nodes_.at(i);
    if (root == node) {
      exist = true;
      break;
    }
  }
  // root has not been calculated
  if (!exist) {
    cell->v_tree_nodes_.push_back(root);
    cell->v_cell_nodes_.push_back(&v_cell_nodes_.at(root->id_));
  }
  return true;
}


bool Tree2TreeBasedDecoder::SetUserTranslateForTreeParsing() {
  return true;
}



bool Tree2TreeBasedDecoder::PrintTree2TreeBasedDecoderLogo() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# Tree2Tree-Decoder for Statistical Machine Translation           #\n"
      <<"#                                            Version 0.0.1        #\n"
      <<"#                                            liqiangneu@gmail.com #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<flush;
  return true;
}


}

