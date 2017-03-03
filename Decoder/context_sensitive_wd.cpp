/*
* $Id:
* 0018
*
* $File:
* me_reordering_table.cpp
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
* 2012-12-28,19:55
* 2012-12-12,14:20
*/

#include "context_sensitive_wd.h"


namespace decoder_context_sensitive_wd {

bool ContextSensitiveWd::Init (Configuration &config) {
  InitContextSensitiveWd (config);
  return true;
}


bool ContextSensitiveWd::InitContextSensitiveWd (Configuration &config) {
  ifstream in_file(config.context_sensitive_model_.c_str());
  if (!in_file) {
    cerr<<"ERROR: Can not open file \""<<config.context_sensitive_model_<<"\".\n"<<flush;
    exit(1);
  } else {
    cerr<<"Loading Context-Sensitive-WD\n"<<"  >> From File: "<<config.context_sensitive_model_<<"\n"<<flush;
  }

  clock_t start, finish;
  start = clock();

  string lineOfInFile;
  size_t lineNo = 0;
  while (getline(in_file, lineOfInFile)) {
    ++lineNo;
    ClearIllegalChar(lineOfInFile);
    RmEndSpace(lineOfInFile);
    RmStartSpace(lineOfInFile);

    vector<string> v_context_sensitive_feature;
    Split(lineOfInFile, '\t', v_context_sensitive_feature);

    if (v_context_sensitive_feature.size() != 2) {
//      cerr<<"WARNING: Format error in line "<<lineNo<<"\n"<<flush;
      continue;
    }

    if (!LoadFeature(v_context_sensitive_feature, lineNo)) {
      continue;
    }

    finish = clock();
    if (lineNo % 50000 == 0) {
      cerr<<"\r  Processed "<<lineNo<<" lines. [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   "<<flush;
    }
  }
  finish = clock();
  cerr<<"\r  Processed "<<lineNo<<" lines. [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   \n"<<"  Done!\n\n"<<flush;
  in_file.close();
  return true;
}


bool ContextSensitiveWd::LoadFeature (vector<string> &v_context_sensitive_feature, size_t &lineNo) {
  if (v_context_sensitive_feature.at(0).size() < 5) {
//  cerr<<"  WARNING: Format error in line "<<lineNo<<"\n"<<flush;
    return false;
  }

  float weight = (float)atof(v_context_sensitive_feature.at(1).c_str());
  vector<string> v_feature_label;
  Split(v_context_sensitive_feature.at(0), '=', v_feature_label);
  if (v_feature_label.size() < 2) {
//  cerr<<"  WARNING: Format error in line "<<lineNo<<"\n"<<flush;
    return false;
  }
  string src_word = v_feature_label.at(1);

  vector<string> v_del_flag_and_label;
  Split(v_feature_label.at(0), ':', v_del_flag_and_label);
  if (v_feature_label.size() < 2) {
//  cerr<<"  WARNING: Format error in line "<<lineNo<<"\n"<<flush;
    return false;
  }
  string position = v_del_flag_and_label.at(1);
  bool spurious_flag = v_del_flag_and_label.at(0) == "1" ? true : false;

  int base_position = 0;
  if (spurious_flag) {
    base_position += 14;
  }
  if (position == "w-2") {
    base_position += 0;
  } else if (position == "w-1") {
    base_position += 1;
  } else if (position == "w") {
    base_position += 2;
  } else if (position == "w+1") {
    base_position += 3;
  } else if (position == "w+2") {
    base_position += 4;
  } else if (position == "p-2") {
    base_position += 5;
  } else if (position == "p-1") {
    base_position += 6;
  } else if (position == "p") {
    base_position += 7;
  } else if (position == "p+1") {
    base_position += 8;
  } else if (position == "p+2") {
    base_position += 9;
  } else if (position == "p-2_p-1") {
      base_position += 10;
  } else if (position == "p-1_p") {
      base_position += 11;
  } else if (position == "p_p+1") {
      base_position += 12;
  } else if (position == "p+1_p+2") {
      base_position += 13;
  }

  if (context_sensitive_wd_src_.find(src_word) == context_sensitive_wd_src_.end()) {
    vector<float> tmp_weights(28, 0.0);
    tmp_weights.at(base_position) = weight;
    context_sensitive_wd_src_.insert(make_pair(src_word, tmp_weights));
  } else {
    context_sensitive_wd_src_[src_word].at(base_position) = weight;
  }
  return true;
}

}


