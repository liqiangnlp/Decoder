/*
* $Id:
* 0030
*
* $File:
* configuration.cpp
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
* 2013-03-14,10:13
*/

#include "configuration.h"


namespace decoder_configuration {

bool Configuration::Init (STRPAIR &param) {
  CheckParamsInConf (param);
  if (!convert_pt_2_binary_flag_) {
    CheckFilesInConf (param);
  }
  return true;
}


bool Configuration::CheckParamsInConf (STRPAIR &param) {
  // check for "nround"
  string paramKey = "nround";
  string paramDefValue = "1";
  CheckEachParamInConf (param, paramKey, paramDefValue);
  nround_ = atoi (param[paramKey].c_str());

  // check for "ngram"
  paramKey = "ngram";
  paramDefValue = "3";
  CheckEachParamInConf (param, paramKey, paramDefValue);
  ngram_ = atoi (param[paramKey].c_str());

  // check for "beamsize"
  paramKey = "beamsize";
  paramDefValue = "30";
  CheckEachParamInConf (param, paramKey, paramDefValue);
  beamsize_ = atoi (param[paramKey].c_str());

  // check for "nbest"
  paramKey = "nbest";
  paramDefValue = "30";
  CheckEachParamInConf (param, paramKey, paramDefValue);
  nbest_ = atoi (param[paramKey].c_str() ) > beamsize_ ? beamsize_ : atoi( param[paramKey].c_str());

  // check for "nref"
  paramKey = "nref";
  paramDefValue = "4";
  CheckEachParamInConf (param, paramKey, paramDefValue);
  nref_ = atoi (param[paramKey].c_str());

  // check for "maxreorderdis"
  paramKey = "maxreorderdis";
  paramDefValue = "10";
  CheckEachParamInConf( param, paramKey, paramDefValue );
  max_reordering_distance_ = atoi (param[paramKey].c_str());

  // check for "maxphraselength"
  paramKey = "maxphraselength";
  paramDefValue = "3";
  CheckEachParamInConf (param, paramKey, paramDefValue);
  max_phrase_length_ = atoi (param[paramKey].c_str());

  // check for "nthread"
  paramKey = "nthread";
  paramDefValue = "0";
  CheckEachParamInConf (param, paramKey, paramDefValue);
  nthread_ = atoi (param[paramKey].c_str());

  // check for "freefeature"
  paramKey = "freefeature";
  paramDefValue = "0";
  CheckEachParamInConf (param, paramKey, paramDefValue);
  free_feature_ = atoi (param[paramKey].c_str());

  // check for "usepuncpruning"
  paramKey = "usepunctpruning";
  paramDefValue = "1";
  CheckEachParamInConf (param, paramKey, paramDefValue);
  use_punct_pruning_ = (param[paramKey] == "0" ? false : true);

  // check for "usecubepruning"
  paramKey = "usecubepruning";
  paramDefValue = "1";
  CheckEachParamInConf (param, paramKey, paramDefValue);
  use_cube_pruning_ = (param[paramKey] == "0" ? false : true);

  // check for "usecubepruninginc"
  if (!recase_flag_) {
    paramKey = "usecubepruninginc";
    paramDefValue = "1";
    CheckEachParamInConf (param, paramKey, paramDefValue);
    use_cube_pruninginc_ = (param[paramKey] == "0" ? false : true);
    use_cube_pruning_ = (use_cube_pruninginc_ == true ? false : use_cube_pruning_);
  } else {
    use_cube_pruninginc_ = false;
  }

  // check for "use-me-reorder"
  paramKey = "use-me-reorder";
  paramDefValue = "1";
  CheckEachParamInConf (param, paramKey, paramDefValue);
  use_me_reorder_ = (param[paramKey] == "0" ? false : true);

  // check for "use-msd-reorder"
  paramKey = "use-msd-reorder";
  paramDefValue = "0";
  CheckEachParamInConf (param, paramKey, paramDefValue);
  use_msd_reorder_ = (param[paramKey] == "0" ? false : true);

  // check for "useemptytranslation"
  paramKey = "useemptytranslation";
  paramDefValue = "0";
  CheckEachParamInConf (param, paramKey, paramDefValue);
  use_empty_translation_ = (param[paramKey] == "0" ? false : true);

  // check for "outputemptytranslation"
  paramKey = "outputemptytranslation";
  paramDefValue = "0";
  CheckEachParamInConf (param, paramKey, paramDefValue);
  output_empty_translation_ = (param[paramKey] == "0" ? false : true);

  // check for "use-context-sensitive-wd"
  paramKey = "use-context-sensitive-wd";
  paramDefValue = "0";
  CheckEachParamInConf (param, paramKey, paramDefValue);
  use_context_sensitive_wd_ = (param[paramKey] == "0" ? false : true);

  if (use_context_sensitive_wd_) {
    paramKey = "context-sensitive-wd-weights";
    paramDefValue = "0.5 0.5";
    CheckEachParamInConf(param, paramKey, paramDefValue);

    paramKey = "context-sensitive-wd-ranges";
    paramDefValue = "-3:3 -3:3";
    CheckEachParamInConf(param, paramKey, paramDefValue);

    paramKey = "context-sensitive-wd-fixedfs";
    paramDefValue = "0 0";
    CheckEachParamInConf(param, paramKey, paramDefValue);

    features.ParsingContextSensitiveFeatures(param);
  }

  // check for "outputoov"
  paramKey = "outputoov";
  paramDefValue = "0";
  CheckEachParamInConf (param, paramKey, paramDefValue);
  output_oov_ = (param[paramKey] == "0" ? false : true);

  // check for "labeloov"
  paramKey = "labeloov";
  paramDefValue = "1";
  CheckEachParamInConf (param, paramKey, paramDefValue);
  label_oov_ = (param[paramKey] == "0" ? false : true);

  // check for "englishstring"
  paramKey = "englishstring";
  paramDefValue = "0";
  CheckEachParamInConf (param, paramKey, paramDefValue);
  english_string_ = (param[paramKey] == "0" ? false : true);

  // check for "-MERT"
  paramKey = "-MERT";
  paramDefValue = "0";
  CheckEachParamInConf (param, paramKey, paramDefValue);
  mert_flag_ = (param[paramKey] == "0" ? false : true);

  // check for "recasemethod"
  paramKey = "recasemethod";
  if (param[paramKey] != "l2r" && param[paramKey] != "cyk") {
    param[paramKey] = "l2r";
  }
  paramDefValue = "l2r";
  CheckEachParamInConf (param, paramKey, paramDefValue);
  recase_method_ = param[paramKey];

  // init config file
  config_file_ = param["-config"];

  // check for "log";
  paramKey = "-log";
  paramDefValue = "niutransserver.log.txt";
  CheckEachParamInConf (param, paramKey, paramDefValue);
  log_file_ = param[paramKey];

  // check for "output";
  paramKey = "-output";
  paramDefValue = "niutransserver.output.txt";
  CheckEachParamInConf (param, paramKey, paramDefValue);
  output_file_ = param[paramKey];

  if (mert_flag_) {
    // check for "mert_tmp_file_"
    paramKey = "-mert-tmp-file";
    paramDefValue = "niutransserver.mert.tmp.txt";
    CheckEachParamInConf(param, paramKey, paramDefValue);
    mert_tmp_file_ = param[paramKey];

    // check for "mert_log_file_"
    paramKey = "-mert-log-file";
    paramDefValue = "niutransserver.mert.log.txt";
    CheckEachParamInConf(param, paramKey, paramDefValue);
    mert_log_file_ = param[paramKey];

    // check for "mert_config_file_"
    paramKey = "-mert-config-file";
    paramDefValue = "niutransserver.mert.config.txt";
    CheckEachParamInConf(param, paramKey, paramDefValue);
    mert_config_file_ = param[paramKey];
  }

  // check for "weights"
  paramKey = "weights";
  paramDefValue = "1.000 0.500 0.200 0.200 0.200 0.200 0.500 0.500 -0.100 1.000 0 0 0 0 0 0 0";
  CheckEachParamInConf (param, paramKey, paramDefValue);

  // check for "ranges"
  paramKey = "ranges";
  paramDefValue = "-3:7 -1:3 0:3 0:0.4 0:3 0:0.4 -3:3 -3:3 -3:0 -3:3 0:0 0:0 0:0 0:0 0:0 0:0 0:0";
  CheckEachParamInConf (param, paramKey, paramDefValue);

  // check for "fixedfs"
  paramKey = "fixedfs";
  paramDefValue = "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0";
  CheckEachParamInConf (param, paramKey, paramDefValue);

  features.ParsingFeatures (param);

  return true;
}


bool Configuration::CheckEachParamInConf (STRPAIR &param, string  &paramKey, string  &defValue) {
  if (param.find (paramKey) == param.end())
    param[paramKey] = defValue;
    return true;
}


bool Configuration::CheckFilesInConf (STRPAIR &param) {
  // check for LM file
  string fileKey = "Ngram-LanguageModel-File";
  checkFileInConf (param, fileKey);
  lm_file_ = param["Ngram-LanguageModel-File"];

  // check for Target-side vocabulary
  fileKey = "Target-Vocab-File";
  checkFileInConf (param, fileKey);
  target_vocab_file_ = param["Target-Vocab-File"];

  // check for MaxEnt-based lexicalized reordering model
  if (use_me_reorder_) {
    fileKey = "ME-Reordering-Table";
    checkFileInConf (param, fileKey);
    metabFile = param["ME-Reordering-Table"];
  }

  // check for MSD lexicalized reordering model
  if (use_msd_reorder_) {
    fileKey = "MSD-Reordering-Model";
    checkFileInConf( param, fileKey );
    msdtabFile = param[ "MSD-Reordering-Model" ];
  }

  // check for context sensitive word deletion model
  if (use_context_sensitive_wd_) {
    fileKey = "Context-Sensitive-WD";
    checkFileInConf (param, fileKey);
    context_sensitive_model_ = param[fileKey];
  }

  // check for phrase translation model
  if (!phrase_table_binary_flag_) {
    fileKey = "Phrase-Table";
    checkFileInConf (param, fileKey);
    phrase_table_file_ = param["Phrase-Table"];
  } else {
    fileKey = "Phrase-Table-Binary";
    checkFileInConf (param, fileKey);
    phrase_table_file_ = param["Phrase-Table-Binary"];

    fileKey = "Phrase-Table-Binary-SrcPhr";
    checkFileInConf (param, fileKey);
    source_phrase_file_ = param["Phrase-Table-Binary-SrcPhr"];

    fileKey = "Phrase-Table-Binary-TgtPhr";
    checkFileInConf (param, fileKey);
    target_phrase_file_ = param["Phrase-Table-Binary-TgtPhr"];
  }

  // check for Test file
  if (!support_service_flag_) {
    if (!mert_flag_) {
      fileKey = "-test";
      checkFileInComm (param, fileKey);
      testFile = param["-test"];
    } else {
      fileKey = "-dev";
      checkFileInComm (param, fileKey);
      testFile = param["-dev"];
    }
  }
  return true;
}


    bool
    Configuration::checkFileInConf( 
    STRPAIR &param  , 
    string  &fileKey  )
    {
        if( param.find( fileKey ) != param.end() )
        {
            ifstream inFile( param[ fileKey ].c_str() );
            if( !inFile )
            {
                cerr<<"ERROR: Please check the path of \""<<fileKey<<"\".\n"<<flush;
                exit( 1 );
            }
            inFile.clear();
            inFile.close();
        }
        else
        {
            cerr<<"ERROR: Please add parameter \""<<fileKey<<"\" in your config file.\n"<<flush;
            exit( 1 );
        }
        return true;
    }


    bool 
    Configuration::checkFileInComm( 
    STRPAIR &param  , 
    string  &fileKey  )
    {
        if( param.find( fileKey ) != param.end() )
        {
            ifstream inFile( param[ fileKey ].c_str() );
            if( !inFile )
            {
                cerr<<"ERROR: Please check the path of \""<<fileKey<<"\".\n";
                cerr<<"       file="<<param[fileKey]<<"\n"<<flush;
                exit( 1 );
            }
            inFile.clear();
            inFile.close();
        }
        else
        {
            cerr<<"ERROR: Please add parameter \""<<fileKey<<"\" in your command.\n"<<flush;
            exit( 1 );
        }
        return true;
    }


bool Features::ParsingFeatures (map<string,string> &param) {
  vector< string > weightsVector;
  vector< string > rangesVector ;
  vector< string > fixedfsVector;
  Split (param["weights"], ' ', weightsVector);
  Split (param["ranges"], ' ', rangesVector);
  Split (param["fixedfs"], ' ', fixedfsVector);
  features_number_ = weightsVector.size();

  if (features_number_ != rangesVector.size() || features_number_ != fixedfsVector.size()) {
    cerr<<"ERROR: Values number of 'weights', 'ranges', 'fixedfs' in config file is not the same!\n"<<flush;
    exit (1);
  }

  size_t pos = 0;
  for (vector< string >::iterator iter = weightsVector.begin(); iter != weightsVector.end(); ++iter) {
    float w = (float)atof(iter->c_str());
    float f = (float)atof(fixedfsVector.at(pos).c_str());

    vector<string> range;
    Split (rangesVector.at( pos ), ':', range);
    float min = (float)atof(range.at(0).c_str());
    float max = (float)atof(range.at(1).c_str());

    FeatureValue featureValue(w, min, max, f);
    feature_values_.push_back(featureValue);
    ++pos;
  }
  return true;
}


bool Features::ParsingFeatures(string &in_file_name) {
  cerr<<"ParsingFeatures from "<<in_file_name<<"...\n"<<flush;
  ifstream in_file(in_file_name.c_str());
  if (!in_file) {
    cerr<<"Can not open file "<<in_file_name<<"\n"<<flush;
    exit(1);
  }

  string line;

  getline(in_file, line);
  RmEndSpace(line);
  RmStartSpace(line);
  ClearIllegalChar(line);
  features_number_ = atoi(line.c_str());
  feature_values_.clear();
  
  while (getline(in_file,line)) {
    RmEndSpace(line);
    RmStartSpace(line);
    ClearIllegalChar(line);
    vector<string> v_single_feature;
    Split(line, ' ', v_single_feature);
    if (v_single_feature.size() != 5) {
      cerr<<"Error: Format in config file is incorrect!\n"<<flush;
      exit(1);
    }
    float w = (float)atof(v_single_feature.at(1).c_str());
    float min = (float)atof(v_single_feature.at(2).c_str());
    float max = (float)atof(v_single_feature.at(3).c_str());
    float f = (float)atof(v_single_feature.at(4).c_str());
    FeatureValue feature_value(w, min, max, f);
    feature_values_.push_back(feature_value);
  }
  if (feature_values_.size() != features_number_) {
    cerr<<"Error: Format in config file is incorrect!\n"<<flush;
    exit(1);
  }
  in_file.close();
  return true;
}


bool Features::ParsingContextSensitiveFeatures(map<string, string> &parameter) {
  vector<string> v_weights;
  vector<string> v_ranges;
  vector<string> v_fixedfs;
  Split (parameter["context-sensitive-wd-weights"], ' ', v_weights);
  Split (parameter["context-sensitive-wd-ranges"], ' ', v_ranges);
  Split (parameter["context-sensitive-wd-fixedfs"], ' ', v_fixedfs);
  cswd_feature_number_ = v_weights.size();
  if (cswd_feature_number_ != v_ranges.size() || cswd_feature_number_ != v_fixedfs.size()) {
      cerr<<"Error: Values number of 'context-sensitive-wd-weights', 'context-sensitive-wd-ranges', and 'context-sensitive-wd-fixedfs' is not the same!\n"<<flush;
      exit (1);
  }
  size_t tmp_position = 0;
  for (vector<string>::iterator iter = v_weights.begin(); iter != v_weights.end(); ++iter) {
    float w = (float)atof(iter->c_str());
    float f = (float)atof(v_fixedfs.at(tmp_position).c_str());
    vector<string> v_range;
    Split (v_ranges.at(tmp_position), ':', v_range);
    float min = (float)atof(v_range.at(0).c_str());
    float max = (float)atof(v_range.at(1).c_str());
    FeatureValue feature_value(w, min, max, f);
    v_context_sensitive_wd_feature_.push_back(feature_value);
    ++tmp_position;
  }

  return true;
}



bool Configuration::PrintConfig () {
  cerr<<setfill(' ');
  cerr<<"Configuration:"<<"\n"
      <<"                  nround : "<<nround_         <<"\n"
      <<"                   ngram : "<<ngram_          <<"\n"
      <<"                beamsize : "<<beamsize_       <<"\n"
      <<"                   nbest : "<<nbest_          <<"\n"
      <<"                    nref : "<<nref_           <<"\n"
      <<"           maxreorderdis : "<<max_reordering_distance_<<"\n"
      <<"         maxphraselength : "<<max_phrase_length_<<"\n"
      <<"             freefeature : "<<free_feature_    <<"\n"
      <<"                 nthread : "<<nthread_        <<"\n\n"
      <<"         usepunctpruning : "<<(use_punct_pruning_     == true ? "true" : "false")<<"\n"
      <<"          usecubepruning : "<<(use_cube_pruning_      == true ? "true" : "false")<<"\n"
      <<"       usecubepruninginc : "<<(use_cube_pruninginc_   == true ? "true" : "false")<<"\n"
      <<"          use-me-reorder : "<<(use_me_reorder_        == true ? "true" : "false")<<"\n"
      <<"         use-msd-reorder : "<<(use_msd_reorder_       == true ? "true" : "false")<<"\n"
      <<"     useemptytranslation : "<<(use_empty_translation_ == true ? "true" : "false")<<"\n"
      <<"  outputemptytranslation : "<<(output_empty_translation_ == true ? "true" : "false")<<"\n"
      <<"               outputoov : "<<(output_oov_            == true ? "true" : "false")<<"\n"
      <<"                labeloov : "<<(label_oov_             == true ? "true" : "false")<<"\n"
      <<"           englishstring : "<<(english_string_        == true ? "true" : "false")<<"\n"
      <<"                    MERT : "<<(mert_flag_             == true ? "true" : "false")<<"\n";
  if (recase_flag_) {
    cerr<<"            recasemethod :"<<recase_method_<<"\n\n";
  } else {
    cerr<<"\n";
  }
  cerr<<"  config = "<<config_file_<<"\n"
      <<"    test = "<<testFile  <<"\n"
      <<"  output = "<<output_file_<<"\n";

  if (!recase_flag_) {
    cerr<<"     log = "<<log_file_<<"\n"
        <<"\n"<<flush;
  } else {
    cerr<<"\n"<<flush;
  }
  return true;
}


bool Configuration::PrintFeatures () {
  cerr<<setiosflags( ios::fixed )<<setprecision( 2 );
  cerr<<"Weights of Feature:"<<"\n"
      <<"          N-gram LM 00 :"
      <<" w="  <<features.feature_values_.at(0).weight_
      <<" f="  <<features.feature_values_.at(0).fixed_
      <<" min="<<features.feature_values_.at(0).min_value_
      <<" max="<<features.feature_values_.at(0).max_value_<<"\n"
      <<"     # of Tar-words 01 :"
      <<" w="  <<features.feature_values_.at(1).weight_
      <<" f="  <<features.feature_values_.at(1).fixed_
      <<" min="<<features.feature_values_.at(1).min_value_
      <<" max="<<features.feature_values_.at(1).max_value_<<"\n"
      <<"            Pr(e|f) 02 :"
      <<" w="  <<features.feature_values_.at(2).weight_
      <<" f="  <<features.feature_values_.at(2).fixed_
      <<" min="<<features.feature_values_.at(2).min_value_
      <<" max="<<features.feature_values_.at(2).max_value_<<"\n"
      <<"           Lex(e|f) 03 :"
      <<" w="  <<features.feature_values_.at(3).weight_
      <<" f="  <<features.feature_values_.at(3).fixed_
      <<" min="<<features.feature_values_.at(3).min_value_
      <<" max="<<features.feature_values_.at(3).max_value_<<"\n"
      <<"            Pr(f|e) 04 :"
      <<" w="  <<features.feature_values_.at(4).weight_
      <<" f="  <<features.feature_values_.at(4).fixed_
      <<" min="<<features.feature_values_.at(4).min_value_
      <<" max="<<features.feature_values_.at(4).max_value_<<"\n"
      <<"           Lex(f|e) 05 :"
      <<" w="  <<features.feature_values_.at(5).weight_
      <<" f="  <<features.feature_values_.at(5).fixed_
      <<" min="<<features.feature_values_.at(5).min_value_
      <<" max="<<features.feature_values_.at(5).max_value_<<"\n"
      <<"       # of Phrases 06 :"
      <<" w="  <<features.feature_values_.at(6).weight_
      <<" f="  <<features.feature_values_.at(6).fixed_
      <<" min="<<features.feature_values_.at(6).min_value_
      <<" max="<<features.feature_values_.at(6).max_value_<<"\n";

  if (!recase_flag_) {
    cerr<<"  # of bi-lex links 07 :"
        <<" w="  <<features.feature_values_.at(7).weight_
        <<" f="  <<features.feature_values_.at(7).fixed_
        <<" min="<<features.feature_values_.at(7).min_value_
        <<" max="<<features.feature_values_.at(7).max_value_<<"\n"
        <<"    # of NULL-trans 08 :"
        <<" w="  <<features.feature_values_.at(8).weight_
        <<" f="  <<features.feature_values_.at(8).fixed_
        <<" min="<<features.feature_values_.at(8).min_value_
        <<" max="<<features.feature_values_.at(8).max_value_<<"\n"
        <<"   ME Reorder Model 09 :"
        <<" w="  <<features.feature_values_.at(9).weight_
        <<" f="  <<features.feature_values_.at(9).fixed_
        <<" min="<<features.feature_values_.at(9).min_value_
        <<" max="<<features.feature_values_.at(9).max_value_<<"\n"
        <<"        <UNDEFINED> 10 :"
        <<" w="  <<features.feature_values_.at(10).weight_
        <<" f="  <<features.feature_values_.at(10).fixed_
        <<" min="<<features.feature_values_.at(10).min_value_
        <<" max="<<features.feature_values_.at(10).max_value_<<"\n"
        <<"     MSD Pre & Mono 11 :"
        <<" w="  <<features.feature_values_.at(11).weight_
        <<" f="  <<features.feature_values_.at(11).fixed_
        <<" min="<<features.feature_values_.at(11).min_value_
        <<" max="<<features.feature_values_.at(11).max_value_<<"\n"
        <<"     MSD Pre & Swap 12 :"
        <<" w="  <<features.feature_values_.at(12).weight_
        <<" f="  <<features.feature_values_.at(12).fixed_
        <<" min="<<features.feature_values_.at(12).min_value_
        <<" max="<<features.feature_values_.at(12).max_value_<<"\n"
        <<"     MSD Pre & Disc 13 :"
        <<" w="  <<features.feature_values_.at(13).weight_
        <<" f="  <<features.feature_values_.at(13).fixed_
        <<" min="<<features.feature_values_.at(13).min_value_
        <<" max="<<features.feature_values_.at(13).max_value_<<"\n"
        <<"     MSD Fol & Mono 14 :"
        <<" w="  <<features.feature_values_.at(14).weight_
        <<" f="  <<features.feature_values_.at(14).fixed_
        <<" min="<<features.feature_values_.at(14).min_value_
        <<" max="<<features.feature_values_.at(14).max_value_<<"\n"
        <<"     MSD Fol & Swap 15 :"
        <<" w="  <<features.feature_values_.at(15).weight_
        <<" f="  <<features.feature_values_.at(15).fixed_
        <<" min="<<features.feature_values_.at(15).min_value_
        <<" max="<<features.feature_values_.at(15).max_value_<<"\n"
        <<"     MSD Fol & Disc 16 :"
        <<" w="  <<features.feature_values_.at(16).weight_
        <<" f="  <<features.feature_values_.at(16).fixed_
        <<" min="<<features.feature_values_.at(16).min_value_
        <<" max="<<features.feature_values_.at(16).max_value_<<"\n"
        <<"\n"<<flush;

    if (free_feature_ != 0) {
      if (free_feature_ <= features.feature_values_.size() - 17) {
        for (int i = 0; i < free_feature_; ++i) {
          cerr<<"  Free Feature ["<<i + 1<<"]  :"
              <<" w="  <<features.feature_values_.at( i + 17 ).weight_
              <<" f="  <<features.feature_values_.at( i + 17 ).fixed_
              <<" min="<<features.feature_values_.at( i + 17 ).min_value_
              <<" max="<<features.feature_values_.at( i + 17 ).max_value_<<"\n";
        }
        cerr<<"\n"<<flush;
      } else {
        cerr<<"ERROR: Parameter 'freefeature' in config file do not adapt to the phrasetable!\n";
        exit( 1 );
      }
    }
  } else {
    cerr<<"\n"<<flush;
  }

  if (use_context_sensitive_wd_) {
    cerr<<"         CSWD Align 01 :"
        <<" w="  <<features.v_context_sensitive_wd_feature_.at(0).weight_
        <<" f="  <<features.v_context_sensitive_wd_feature_.at(0).fixed_
        <<" min="<<features.v_context_sensitive_wd_feature_.at(0).min_value_
        <<" max="<<features.v_context_sensitive_wd_feature_.at(0).max_value_<<"\n"
        <<"       CSWD Unalign 02 :"
        <<" w="  <<features.v_context_sensitive_wd_feature_.at(1).weight_
        <<" f="  <<features.v_context_sensitive_wd_feature_.at(1).fixed_
        <<" min="<<features.v_context_sensitive_wd_feature_.at(1).min_value_
        <<" max="<<features.v_context_sensitive_wd_feature_.at(1).max_value_<<"\n"
        <<"\n"<<flush;
  }

  return true;
}

}

