/*
* $Id:
* 0008
*
* $File:
* rescoring.cpp
*
* $Proj:
* Decoder for Statistical Machine Translation
*
* $Func:
* rescoring
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
* 2015-10-28, 20:38, in USA
*/


#include "rescoring.h"


namespace decoder_rescoring {

bool Rescoring::Initialize(map<string, string> &parameters) {
  CheckFiles(parameters);
  dev_file_ = parameters["-dev"];
  dev_nbest_file_ = parameters["-devnbest"];
  initial_weights_file_ = parameters["-initweights"];
  test_file_ = parameters["-test"];
  test_nbest_file_ = parameters["-testnbest"];

  string parameter_key = "-nref";
  string default_value = "1";
  CheckEachParameter(parameters, parameter_key, default_value);
  n_ref_ = atoi(parameters[parameter_key].c_str());

  parameter_key = "-nround";
  default_value = "2";
  CheckEachParameter(parameters, parameter_key, default_value);
  n_round_ = atoi(parameters[parameter_key].c_str());
  return true;
}


bool Rescoring::CheckEachParameter( map< string, string > &parameters, string &parameter_key, string &default_value ) {
  if( parameters.find( parameter_key ) == parameters.end() ) {
    parameters[ parameter_key ] = default_value;
  }
  return true;
}


/*
 * $Name: SortDevNBest
 * $Function: Rescoring n-best translation results with using MERT
 * $Date: 2015-10-28, in Los Angeles, USA
 */
bool Rescoring::SortDevNBest(Configuration &config, int &round) {
  cerr<<"Sort dev  nbest "<<flush;
  string s_round = intToString(round);
  string out_file_name = config.mert_tmp_file_ + ".round" + s_round;
  string out_file_1best_name = config.mert_tmp_file_ + ".1best.round" + s_round;
  string out_weights_name = config.mert_config_file_ + ".round" + s_round;
  ofstream out_file (out_file_name.c_str());
  if (!out_file) {
    cerr<<"ERROR: Can not open file \""<<out_file_name<<"\".\n"<<flush;
    exit(1);
  }

  ofstream out_file_1best (out_file_1best_name.c_str());
  if (!out_file_1best) {
    cerr<<"ERROR: Can not open file \""<<out_file_1best_name<<"\".\n"<<flush;
    exit(1);
  }

  ofstream out_weights (out_weights_name.c_str());
  if (!out_weights) {
    cerr<<"ERROR: Can not open file \""<<out_weights_name<<"\".\n"<<flush;
    exit(1);
  }

  for(vector<vector<pair<string, vector<float> > > >::iterator iter = vv_dev_nbest_.begin(); iter != vv_dev_nbest_.end(); ++iter) {
    multimap<float, pair<string, vector<float> > > m_translations;
    for(vector<pair<string, vector<float> > >::iterator iter_p = iter->begin(); iter_p != iter->end(); ++iter_p) {
       float score = 0.0f;
       int position = 0;
       for(vector<float>::iterator iter_v = iter_p->second.begin(); iter_v != iter_p->second.end(); ++iter_v) {
         score += config.features.feature_values_.at(position).weight_ * (*iter_v);
         ++ position;
       }
       m_translations.insert(make_pair(score, make_pair(iter_p->first, iter_p->second)));
    }
    int high_score_position = 0;
    for(multimap<float, pair<string, vector<float> > >::reverse_iterator r_iter = m_translations.rbegin(); r_iter != m_translations.rend(); ++r_iter) {
     if (0 == high_score_position) {
       out_file_1best<<r_iter->second.first<<"\n";
     }
     out_file<<r_iter->second.first<<" ||||";
     int position = 1;
     for(vector<float>::iterator iter_v = r_iter->second.second.begin(); iter_v != r_iter->second.second.end(); ++iter_v) {
       out_file<<" "<<position<<":"<<*iter_v;
       ++position;
     }
     out_file<<" |||| "<<r_iter->first<<"\n";
     ++high_score_position;
    }
    out_file<<"===============================\n";
  }

  out_weights<<config.features.features_number_<<"\n";
  int position = 1;
  for (vector<FeatureValue>::iterator iter = config.features.feature_values_.begin(); iter != config.features.feature_values_.end(); ++iter) {
    out_weights<<position<<" "<<iter->weight_<<" "<<iter->min_value_<<" "<<iter->max_value_<<" "<<iter->fixed_<<"\n";
    ++position;
  }


  out_file.close();
  out_file_1best.close();
  out_weights.close();

  map<string, string> parameter_dev;
  parameter_dev["-1best"] = out_file_1best_name;
  parameter_dev["-dev"] = dev_file_;
  parameter_dev["-out"] = out_file_1best_name + ".bleu";
  parameter_dev["-nref"] = intToString(n_ref_);
  parameter_dev["-printinformation"] = "0";
  IbmBleuScore ibm_bleu_score;
  float bleu_score = 0.0f;
  bleu_score = ibm_bleu_score.Process(parameter_dev);
  cerr<<"* BLEU4="<<bleu_score<<" *\n"<<flush;
  return true;
}


/*
 * $Name: SortTestNBest
 * $Function: Rescoring n-best translation results with using MERT
 * $Date: 2015-10-29, in Los Angeles, USA
 */
bool Rescoring::SortTestNBest(Configuration &config, int &round) {
  cerr<<"Sort test nbest "<<flush;
  string s_round = intToString(round);
  string out_file_name = test_nbest_file_ + ".round" + s_round;
  string out_file_1best_name = test_nbest_file_ + ".1best.round" + s_round;
  ofstream out_file (out_file_name.c_str());
  if (!out_file) {
    cerr<<"ERROR: Can not open file \""<<out_file_name<<"\".\n"<<flush;
    exit(1);
  }

  ofstream out_file_1best (out_file_1best_name.c_str());
  if (!out_file_1best) {
    cerr<<"ERROR: Can not open file \""<<out_file_1best_name<<"\".\n"<<flush;
    exit(1);
  }

  for(vector<vector<pair<string, vector<float> > > >::iterator iter = vv_test_nbest_.begin(); iter != vv_test_nbest_.end(); ++iter) {
    multimap<float, pair<string, vector<float> > > m_translations;
    for(vector<pair<string, vector<float> > >::iterator iter_p = iter->begin(); iter_p != iter->end(); ++iter_p) {
       float score = 0.0f;
       int position = 0;
       for(vector<float>::iterator iter_v = iter_p->second.begin(); iter_v != iter_p->second.end(); ++iter_v) {
         score += config.features.feature_values_.at(position).weight_ * (*iter_v);
         ++ position;
       }
       m_translations.insert(make_pair(score, make_pair(iter_p->first, iter_p->second)));
    }
    int high_score_position = 0;
    for(multimap<float, pair<string, vector<float> > >::reverse_iterator r_iter = m_translations.rbegin(); r_iter != m_translations.rend(); ++r_iter) {
     if (0 == high_score_position) {
       out_file_1best<<r_iter->second.first<<"\n";
     }
     out_file<<r_iter->second.first<<" ||||";
     int position = 1;
     for(vector<float>::iterator iter_v = r_iter->second.second.begin(); iter_v != r_iter->second.second.end(); ++iter_v) {
       out_file<<" "<<position<<":"<<*iter_v;
       ++position;
     }
     out_file<<" |||| "<<r_iter->first<<"\n";
     ++high_score_position;
    }
    out_file<<"===============================\n";
  }

  out_file.close();
  out_file_1best.close();

  map<string, string> parameter_test;
  parameter_test["-1best"] = out_file_1best_name;
  parameter_test["-dev"] = test_file_;
  parameter_test["-out"] = out_file_1best_name + ".bleu";
  parameter_test["-nref"] = intToString(n_ref_);
  parameter_test["-printinformation"] = "0";
  IbmBleuScore ibm_bleu_score;
  float bleu_score = 0.0f;
  bleu_score = ibm_bleu_score.Process(parameter_test);
  cerr<<"* BLEU4="<<bleu_score<<" *\n"<<flush;
  return true;
}




/*
 * $Name: CheckFiles
 * $Function: If the files used in training TM model does not exist, exit!
 * $Date: 2015-10-28, in Los Angeles, USA
 */
bool Rescoring::CheckFiles(map<string, string> &parameters) {
  string file_key = "-dev";
  CheckFile(parameters, file_key);

  file_key = "-devnbest";
  CheckFile(parameters, file_key);

  file_key = "-test";
  CheckFile(parameters, file_key);

  file_key = "-testnbest";
  CheckFile(parameters, file_key);

  file_key = "-initweights";
  CheckFile(parameters, file_key);

  if(parameters.find("-out") == parameters.end()) {
    cerr<<"ERROR: Please add parameter \"-out\" in your command line!\n";
  }
  return true;
}


/*
 * $Name: CheckFile
 * $Function: If the file to be checked does not exist, exit!
 * $Date: 2015-10-28, in Los Angeles, USA
 */
bool Rescoring::CheckFile(map<string, string> &parameters, string &file_key) {
  if ( parameters.find( file_key ) != parameters.end() ) {
    ifstream in_file( parameters[ file_key ].c_str() );
    if ( !in_file ) {
      cerr<<"ERROR: Please check the path of \""<<file_key<<"\".\n"<<flush;
      exit( 1 );
    }
    in_file.clear();
    in_file.close();
  } else {
    cerr<<"ERROR: Please add parameter \""<<file_key<<"\" in your command line!\n"<<flush;
    exit( 1 );
  }
  return true;
}


/*
 * $Name: GetFeatureWeight
 * $Function: 
 * $Date: 10/29/2015, Los Angeles, USA
 */
bool Rescoring::GetFeatureWeight(Configuration &config, string &fn, multimap<float, vector<float> > &bleuAndScores) {
  ifstream inFn(fn.c_str());
  string line;
  vector<string> tmpWeights;
  float curBleu = 0;
  vector<float> curWeight;
  while (getline(inFn, line)) {
    if (line.find("Before optimization - BLEU") != string::npos) {
      vector<string> tmpVec;
      Split(line, ' ', tmpVec);
      curBleu = (float)atof(tmpVec.at( 4 ).c_str());
      continue;
    }

    if (line.find("Final Weights") != string::npos) {
      Split(line, ' ', tmpWeights);
      break;
    } else {
      continue;
    }
  }

  if (tmpWeights.size() > 3) {
    for (int i = 0; i < tmpWeights.size() - 3; ++ i) {
      curWeight.push_back(config.features.feature_values_.at( i ).weight_);
      config.features.feature_values_.at(i).weight_ = (float)atof(tmpWeights.at(i + 3).c_str());
    }
  }
  bleuAndScores.insert(make_pair(curBleu, curWeight));
  if (config.features.feature_values_.size() != 0) {
    cerr<<"Inputted  Weights ("<<curWeight.size()<<") : ";
    for (vector<float>::iterator iter = curWeight.begin(); iter != curWeight.end(); ++iter) {
      cerr<<*iter<<" ";
    }
    cerr<<"\n";
    cerr<<"Optimized Weights ("<<config.features.feature_values_.size()<<") : ";
    for (vector<FeatureValue>::iterator iter = config.features.feature_values_.begin(); iter != config.features.feature_values_.end(); ++ iter) {
      cerr<<iter->weight_<<" ";
    }
    cerr<<"\n\n";
  }

  inFn.close();
  return true;
}


bool Rescoring::LoadDevNBest(Configuration &config) {
  ifstream in_file(dev_nbest_file_.c_str());
  string line;
  vector<pair<string, vector<float> > > v_sentence_translations;
  int line_no = 0;
  cerr<<"Start loading nbest (dev) from "<<dev_nbest_file_<<"!\n"<<flush;
  while(getline(in_file, line)) {
    ++ line_no;
    RmEndSpace(line);
    RmStartSpace(line);
    ClearIllegalChar(line);
    vector<string> v_sentence_scores;
    SplitWithStr(line, " |||| ", v_sentence_scores);
    if (v_sentence_scores.size() < 3) {
      if ("===============================" == line || "================" == line) {
        vv_dev_nbest_.push_back(v_sentence_translations);
        v_sentence_translations.clear();
      } else {
        cerr<<"Error: Format error in "<<dev_nbest_file_<<"!\n"<<flush;
        cerr<<"SENTENCE: "<<line<<"\n"
            <<"LINE NUMBER: "<<line_no<<"\n";
        exit(1);
      }
    } else {
      vector<string> v_id_scores;
      vector<float> v_scores;
      Split(v_sentence_scores.at(1), ' ', v_id_scores);
      if (v_id_scores.size() != config.features.features_number_) {
        cerr<<"Error: Format error in "<<dev_nbest_file_<<"!\n"<<flush;
        cerr<<"SENTENCE: "<<line<<"\n"
            <<"LINE NUMBER: "<<line_no<<"\n";
        exit(1);
      } else {
        for (vector<string>::iterator iter = v_id_scores.begin(); iter != v_id_scores.end(); ++iter) {
         vector<string> v_tmp_id_score;
         Split(*iter, ':', v_tmp_id_score);
         v_scores.push_back((float)atof(v_tmp_id_score.at(1).c_str()));
        }
        v_sentence_translations.push_back(make_pair(v_sentence_scores.at(0), v_scores));
      }
    }
    if (line_no % 1000 == 0) {
      cerr<<"\r  Processed "<<line_no<<" lines.";
    }
  }
  cerr<<"\r  Processed "<<line_no<<" lines.\n";
  in_file.close();
  return true;
}


bool Rescoring::LoadTestNBest(Configuration &config) {
  ifstream in_file(test_nbest_file_.c_str());
  string line;
  vector<pair<string, vector<float> > > v_sentence_translations;
  int line_no = 0;
  cerr<<"Start loading nbest (test) from "<<test_nbest_file_<<"!\n"<<flush;
  while(getline(in_file, line)) {
    ++ line_no;
    RmEndSpace(line);
    RmStartSpace(line);
    ClearIllegalChar(line);
    vector<string> v_sentence_scores;
    SplitWithStr(line, " |||| ", v_sentence_scores);
    if (v_sentence_scores.size() < 3) {
      if ("===============================" == line || "================" == line) {
        vv_test_nbest_.push_back(v_sentence_translations);
        v_sentence_translations.clear();
      } else {
        cerr<<"Error: Format error in "<<test_nbest_file_<<"!\n"<<flush;
        exit(1);
      }
    } else {
      vector<string> v_id_scores;
      vector<float> v_scores;
      Split(v_sentence_scores.at(1), ' ', v_id_scores);
      if (v_id_scores.size() != config.features.features_number_) {
        cerr<<"Error: Format error in "<<test_nbest_file_<<"!\n"<<flush;
        exit(1);
      } else {
        for (vector<string>::iterator iter = v_id_scores.begin(); iter != v_id_scores.end(); ++iter) {
         vector<string> v_tmp_id_score;
         Split(*iter, ':', v_tmp_id_score);
         v_scores.push_back((float)atof(v_tmp_id_score.at(1).c_str()));
        }
        v_sentence_translations.push_back(make_pair(v_sentence_scores.at(0), v_scores));
      }
    }
    if (line_no % 1000 == 0) {
      cerr<<"\r  Processed "<<line_no<<" lines.";
    }
  }
  cerr<<"\r  Processed "<<line_no<<" lines.\n";
  in_file.close();
  return true;
}


bool Rescoring::PrintRescoringLogo() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# Rescoring                                                       #\n"
      <<"#                                            Version 0.0.1        #\n"
      <<"#                                            liqiangneu@gmail.com #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<flush;
  return true;
}




}

