/*
 * Copyright 2013 YaTrans Inc.
 *
 * $Id:
 * 0006
 *
 * $File:
 * dispatcher.cpp
 *
 * $Proj:
 * Decoder for Statistical Machine Translation
 *
 * $Func:
 * dispatcher method
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
 * 2012-12-04,10:45
 */

#include "dispatcher.h"

namespace decoder_dispatcher
{

/*
 * $Name: ResolveParameters
 * $Function: Resolves parameters from the command line.
 * $Date: 2013-05-07
 */
bool Dispatcher::ResolveParameters (int argc, char * argv[]) {
  string function_icon(argv[1]);
  vector<string> argument;

  for (int i = 2; i != argc; ++i) {
    argument.push_back(argv[i]);
#ifdef WIN32
    string::size_type pos = 0;
    while ((pos = argument.back().find("/")) != string::npos) {
      argument.back().replace(pos, 1, "\\");
    }
#endif
  }

  map<string, string> param;
  for (vector<string>::iterator iter = argument.begin(); iter != argument.end(); ++iter) {
    string temp1(*iter);
    string temp2(*++iter);
    param.insert(make_pair(temp1, temp2));
  }
  return SelectFunction (function_icon, param);
}


/*
 * $Name: SelectFunction
 * $Function: Selects function.
 * $Date: 2013-05-07
 */
bool Dispatcher::SelectFunction (string &function_icon, map<string, string> &parameters) {
  if (function_icon == "--PBD") { 
    AccessPhraseBasedDecoderFunction (parameters);
  } else if (function_icon == "--PBDB") {
    AccessPhraseBasedDecoderBinaryFunction (parameters);
  } else if (function_icon == "--T2T") {
    AccessTree2TreeBasedDecoderFunction (parameters);
  } else if (function_icon == "--REC") {
    AccessRecasingFunction (parameters);
  } else if (function_icon == "--TOKEN") {
    AccessTokenizerFunction(parameters);
  } else if (function_icon == "--DET") {
    AccessDetokenizerFunction (parameters);
  } else if (function_icon == "--SPLITSENT") {
    AccessSplitSentencesFunction(parameters);
  } else if (function_icon == "--CP2B") {
    AccessConvPT2BFunction (parameters);
  } else if (function_icon == "--TM") {
    AccessTranslationMemoryFunction (parameters);
  } else if (function_icon == "--TTMM") {
    AccessTrainingTMModelFunction (parameters);
  } else if (function_icon == "--BLEU") {
    AccessCalculateIbmBleuFunction (parameters);
  } else if (function_icon == "--WDEM") {
    AccessCalculateWdeMetricFunction(parameters);
  } else if (function_icon == "--RESCORING") {
    AccessRescoringFunction(parameters);
  } else if (function_icon == "--LM") {
    AccessLMScoreFunction(parameters);
  } else if (function_icon == "--BUILDINDEX") {
    AccessBuildIndexFunction(parameters);
  } else if (function_icon == "--SEARCH") {
    AccessSearchFunction(parameters);
  } else if (function_icon == "--ASCII") {
//    AccessAsciiFunction(parameters);
  } else if (function_icon == "--ALNDIFF") {
    AccessAlignmentDifferenceFunction(parameters);
  }

  return true;
}


void Dispatcher::AccessPhraseBasedDecoderFunction (map<string, string> &parameters) {
  if (parameters.find("-MERT") == parameters.end()) {
    parameters["-MERT"] = "0";
  }

  ReadConfigFile (parameters);

  bool MERTFlag = parameters["-MERT"] == "0" ? false : true;

  if (!MERTFlag) {
    PhraseBasedDecoder (parameters);
  } else {
    PhraseBasedTraining (parameters);
  }
  return;
}


void Dispatcher::AccessPhraseBasedDecoderBinaryFunction(map< string, string > &parameters) {
  ReadConfigFile( parameters );

  if ( parameters.find( "nthread" ) == parameters.end() ) {
    parameters.insert( make_pair( "nthread", "0" ) );
  }

  int nthread = atoi( parameters[ "nthread" ].c_str() );

  if ( nthread <= 1 ) {
    PhraseBasedDecoderBina(parameters);
  } else {
    cerr<<"Multi-Thread Decoder is not boot!\n"
        <<"Please change the value of \"nthread\" in your configuration file!\n"
        <<flush;
    exit( 1 );
  }
  return;
}


void Dispatcher::AccessTree2TreeBasedDecoderFunction(map< string, string > &parameters) {
  ReadConfigFile(parameters);
  Tree2TreeDecoder(parameters);
  return;
}



void Dispatcher::AccessRecasingFunction(map< string, string > &parameters) {
  ReadConfigFile(parameters);
  Recaser(parameters);
  return;
}


void Dispatcher::AccessTokenizerFunction(map< string, string > &parameters) {
 TokenizerFunction(parameters);
 return;
}


void Dispatcher::AccessDetokenizerFunction(map< string, string > &parameters) {
  DeTokenizer(parameters);
  return;
}


void Dispatcher::AccessSplitSentencesFunction(map< string, string > &parameters) {
 SplitSentencesFunction(parameters);
 return;
}



/*
 * $Name: AccessConvPT2BFunction
 * $Function: Converts text phrase translation table file into binary file.
 * $Date: 2013-05-07
 */
void Dispatcher::AccessConvPT2BFunction(map< string, string > &parameters) {
  ReadConfigFile(parameters);
  ConvertTextPhraseTable2Binary(parameters);
  return;
}


/*
 * $Name: AccessTranslationMemoryFunction
 * $Function: Accesses translation memory function.
 * $Date: 2013-05-07
 */
void Dispatcher::AccessTranslationMemoryFunction(map< string, string > &parameters) {
  ReadConfigFile(parameters);
  TranslationMemoryFunc(parameters);
//  TranslationMemory tm;
//  tm.Init( parameters );
  return;
}


/*
 * $Name: AccessTrainingTMModelFunction
 * $Function:
 * $Date: 2013-05-09
 */
void Dispatcher::AccessTrainingTMModelFunction(map< string, string > &parameters) {
  TrainingTMModel( parameters );
  return;
}


/*
 * $Name: AccessCalculateIbmBleuFunction
 * $Function:
 * $Date: 2014-04-01, in Nanjing
 */
void Dispatcher::AccessCalculateIbmBleuFunction(map< string, string > &parameters) {
  CalculateIbmBleuScore( parameters );
  return;
}


/*
 * $Name: AccessCalculateWdeMetricFunction
 * $Function:
 * $Date: 2015-01-20, in Beijing
 */
void Dispatcher::AccessCalculateWdeMetricFunction(map< string, string > &parameters) {
  CalculateWdeMetricScore( parameters );
  return;
}


/*
 * $Name: AccessRescoringFunction
 * $Function:
 * $Date: 2015-10-28, in USA
 */
void Dispatcher::AccessRescoringFunction(map< string, string > &parameters) {
  RescoringFunction(parameters);
  return;
}


/*
 * $Name: AccessLMScoreFunction
 * $Function:
 * $Date: 11/03/2015, Los Angeles, USA
 */
void Dispatcher::AccessLMScoreFunction(map<string, string> &parameters) {
  LMScoreFunction(parameters);
  return;
}


/*
 * $Name: AccessBuildIndexFunction
 * $Function:
 * $Date: 11/23/2015, 23:17, Los Angeles, USA
 */
void Dispatcher::AccessBuildIndexFunction(map<string, string> &parameters) {
  BuildIndexFunction(parameters);
  return;
}


/*
 * $Name: AccessSearchFunction
 * $Function:
 * $Date: 11/23/2015, 23:17, Los Angeles, USA
 */
void Dispatcher::AccessSearchFunction(map<string, string> &parameters) {
  SearchFunction(parameters);
  return;
}


/*
 * $Name: AccessAlignmentDifferenceFunction
 * $Function:
 * $Date: 02/11/2016, 23:07, Los Angeles, CA, USA
 */
void Dispatcher::AccessAlignmentDifferenceFunction(map<string, string> &parameters) {
  AlignmentDifferenceFunction(parameters);
  return;
}




/*
 * $Name: ReadConfigFile
 * $Function: Reads configuration file.
 * $Date: 2013-05-07
 */
bool Dispatcher::ReadConfigFile(map< string, string > &param) {
  if (param.find( "-config" ) == param.end()) {
    PhraseBasedITGDecoder::PrintPhraseBasedDecoderLogo();
    cerr<<"ERROR: Please add \"-config\" in your command!\n"<<flush;
    exit( 1 );
  }

  ifstream inputConfigFile( param[ "-config" ].c_str() );
  if ( !inputConfigFile ) {
    PhraseBasedITGDecoder::PrintPhraseBasedDecoderLogo();
    cerr<<"ERROR: Config File '"<<param["-config"]<<"' does not exist, exit!\n"<<flush;
    exit( 1 );
  }

  string lineOfConfigFile;
  while (getline(inputConfigFile, lineOfConfigFile)) {
    BasicMethod bm;
    bm.ClearIllegalChar( lineOfConfigFile );
    bm.RmStartSpace    ( lineOfConfigFile );
    bm.RmEndSpace      ( lineOfConfigFile );

    if ( lineOfConfigFile == "" || *lineOfConfigFile.begin() == '#' ) {
      continue;
    } else if (lineOfConfigFile.find("param=\"") == lineOfConfigFile.npos || lineOfConfigFile.find("value=\"") == lineOfConfigFile.npos) {
      continue;
    } else {
      string::size_type pos = lineOfConfigFile.find( "param=\"" );
      pos += 7;
      string key;
      for ( ; lineOfConfigFile[ pos ] != '\"' && pos < lineOfConfigFile.length(); ++pos ) {
        key += lineOfConfigFile[ pos ];
      }
      if ( lineOfConfigFile[ pos ] != '\"' ) {
        continue;
      }

      pos = lineOfConfigFile.find( "value=\"" );
      pos += 7;
      string value;

      for ( ; lineOfConfigFile[ pos ] != '\"' && pos < lineOfConfigFile.length(); ++pos ) {
        value += lineOfConfigFile[ pos ];
      }

      if ( lineOfConfigFile[ pos ] != '\"' ) {
        continue;
      }

      if ( param.find( key ) == param.end() ) {
        param.insert( make_pair( key, value ) );
      } else {
        param[ key ] = value;
      }
    }
  }
  return true;
}


/*
 * $Name: PhraseBasedDecoder
 * $Function: A phrase-based translation engine. The goal of decoding is to search for the best translation
 *            for a given source sentence and trained model.The decoder works in a bottom-up fashion, guaranteeing that
 *            all the sub-cells with in cell[j_1,j_2] are expended before cell[j_1,j_2] is expended.
 * $Date: 2013-05-07
 */
bool Dispatcher::PhraseBasedDecoder(map< string, string > &parameters) {
  PhraseBasedITGDecoder::PrintPhraseBasedDecoderLogo();
  Configuration config;
  config.Init(parameters);
  Model model;
  model.Init(config);
  PhraseBasedITGDecoder pbid;
  pbid.Init(config, model);
  pbid.Decode(config, model);
  return true;
}


/*
 * $Name: PhraseBasedDecoderBina
 * $Function: A phrase-based translation engine. This feature uses the binary phrase translation table file.
 * $Date: 2013-05-07
 */
bool Dispatcher::PhraseBasedDecoderBina(map< string, string > &param) {
  PhraseBasedITGDecoder::PrintPhraseBasedDecoderBinaLogo();
  Configuration config;
  config.phrase_table_binary_flag_ = true;
  config.Init( param );
  Model model;
  model.Init( config );
  PhraseBasedITGDecoder pbid;
  pbid.Init( config, model );
  pbid.Decode( config, model );
  return true;
}



/*
 * $Name: Tree2TreeBasedDecoder
 * $Function: A Tree2Tree-based translation engine.
 * $Date: 2014-10-23
 */
bool Dispatcher::Tree2TreeDecoder(map< string, string > &parameters) {
  Tree2TreeBasedDecoder::PrintTree2TreeBasedDecoderLogo();
  Tree2TreeConfiguration tree2tree_config;
  tree2tree_config.Init(parameters);
  Tree2TreeModel tree2tree_model;
  tree2tree_model.Init(tree2tree_config);
  Tree2TreeTestSet tree2tree_test_set;
  tree2tree_test_set.ReadTestSentences(tree2tree_config.test_file_);
  fprintf(stderr, "Start decoding...\n");
  fflush(stderr);
  for (int i = 0; i < tree2tree_test_set.source_sentences_number_; ++i) {
    Tree2TreeBasedDecoder tree2tree_based_decoder;
    tree2tree_based_decoder.decoding_sentence_.sentence_id_ = i;
    tree2tree_based_decoder.decoding_sentence_.sentence_ = tree2tree_test_set.source_sentences_set_.at(i);
    tree2tree_based_decoder.Init(tree2tree_config, tree2tree_model);
    tree2tree_based_decoder.Decode();
    fprintf(stderr, "    Process %d lines.\n", i);
    fflush(stderr);
  }
  return true;
}






/*
 * $Name: PhraseBasedTraining
 * $Function: To optimize the feature weights, Minimum Error Rate Training (MERT), an optimization algorithm introduced by Och(2003), is
 *            selected as the base learning algorithm in this project.
 * $Date: 2013-05-07
 */
bool Dispatcher::PhraseBasedTraining (map< string, string > &param) {
  PhraseBasedITGDecoder::PrintPhraseBasedTrainingLogo ();

  Configuration config;
  config.Init (param);

  ofstream tmp_mert_file (config.mert_log_file_.c_str());
  tmp_mert_file<<"#### NiuTransServer.mert.log ####\n"<<flush;
  tmp_mert_file.close();

  Model model;
  model.Init (config);

  PhraseBasedITGDecoder pbid;
  pbid.Init (config, model);

  GlobalVar::SetNormalizeText (true);                               // text normalization in MERT
  OurTrainer  * trainer = new OurTrainer ();                        // mert trainer
  TrainingSet * trainst = new TrainingSet ();

  trainst->LoadRefData (config.testFile.c_str(), config.nref_, 4);  // load ref sentences for mert
  cerr<<setiosflags(ios::fixed)<<setprecision( 2 );
  multimap<float, vector<float> > bleuAndScores;
  for (int r = 0; r < config.nround_; r++) {
    cerr<<"Round "<<r+1<<"\n"<<flush;
    pbid.Decode (config, model);
    trainst->LoadTrainingData (config.mert_tmp_file_.c_str(), false);
    trainer->LoadPara (config.mert_config_file_.c_str());
    trainer->OptimzieWeightsWithMERT (config, trainst, 4, (BLEU_TYPE)1);
    vector<float> weights;
    string fn = "niutransserver.mert.result.txt";
    pbid.GetFeatureWeight (config, fn, bleuAndScores);
  }

  pbid.GenerateTranslationResults (config, model, bleuAndScores);

  delete trainst;
  delete trainer;
  return true;
}


/*
 * $Name: Rescoring
 * $Function:
 * $Date: 2015-10-28, in Los Angeles, USA
 */
bool Dispatcher::RescoringFunction(map< string, string > &parameters) {
  Rescoring::PrintRescoringLogo();
  cerr<<setiosflags(ios::fixed)<<setprecision(4);
  Rescoring rescoring;
  rescoring.Initialize(parameters);
  Configuration config;
  config.mert_log_file_ = "niutransserver.mert.log.txt";
  config.mert_tmp_file_ = rescoring.dev_nbest_file_;
  config.mert_config_file_ = rescoring.initial_weights_file_;
  config.features.ParsingFeatures(rescoring.initial_weights_file_);
  rescoring.LoadDevNBest(config);
  rescoring.LoadTestNBest(config);

  GlobalVar::SetNormalizeText(true);                                       // text normalization in MERT
  OurTrainer  * trainer = new OurTrainer ();                               // mert trainer
  TrainingSet * trainst = new TrainingSet();
  trainst->LoadRefData(rescoring.dev_file_.c_str(), rescoring.n_ref_, 4);  // load ref sentences for mert
  BasicMethod basic_method;
  multimap<float, vector<float> > bleuAndScores;
  int r;
  for (r = 0; r < rescoring.n_round_; r++) {
    cerr<<"Round "<<r<<"\n"<<flush;
    rescoring.SortDevNBest(config, r);
    rescoring.SortTestNBest(config, r);

    string dev_nbest_file;
    string initial_weights_file;
    if (0 == r) {
      dev_nbest_file = rescoring.dev_nbest_file_;
      initial_weights_file = rescoring.initial_weights_file_;
    } else {
      string s_nround = basic_method.intToString(r);
      dev_nbest_file = rescoring.dev_nbest_file_ + ".round" + s_nround;
      initial_weights_file = rescoring.initial_weights_file_ + ".round" + s_nround;
    }

    trainst->LoadTrainingData(dev_nbest_file.c_str(), false);
    trainer->LoadPara(initial_weights_file.c_str());
    trainer->OptimzieWeightsWithMERT(config, trainst, 4, (BLEU_TYPE)1);
    string fn = "niutransserver.mert.result.txt";
    rescoring.GetFeatureWeight(config, fn, bleuAndScores);
  }

  cerr<<"Round "<<r<<"\n"<<flush;
  rescoring.SortDevNBest(config, r);
  rescoring.SortTestNBest(config, r);
  delete trainst;
  delete trainer;
  return true;
}


/*
 * $Name: LMScore
 * $Function:
 * $Date: 11/03/2015, Los Angeles, USA
 */
bool Dispatcher::LMScoreFunction(map<string, string> &parameters) {
  LanguageModelScore::PrintLMScoreLogo();
  LanguageModelScore language_model_score;
  language_model_score.Initialize(parameters);
  language_model_score.Process();
  return true;
}


/*
 * $Name: BuildIndexFunction
 * $Function:
 * $Date: 11/23/2015, Los Angeles, USA
 */
bool Dispatcher::BuildIndexFunction(map<string, string> &parameters) {
  Tokenizer tokenizer;
  tokenizer.PrintTokenizerLogo();
  parameters["-input"] = parameters["-corpus"];
  tokenizer.Initialize(parameters);
  if ("en" == tokenizer.language_) {
   tokenizer.StartTokenEng();
  }
  remove(tokenizer.output_file_name_.c_str());

  parameters["-corpus"] = tokenizer.output_lc_file_name_;
  BuildIndex build_index;
  build_index.PrintBuildIndexLogo();
  build_index.Initialize(parameters);
  build_index.BuildIndexSearchEngine();
  remove(tokenizer.output_lc_file_name_.c_str());
  return true;
}


/*
 * $Name: SearchFunction
 * $Function:
 * $Date: 11/23/2015, Los Angeles, USA
 */
bool Dispatcher::SearchFunction(map<string, string> &parameters) {
  SearchEngine search_engine;
  search_engine.PrintSearchEngineLogo();
  search_engine.Initialize(parameters);
  search_engine.Search();
  return true;
}



/*
 * $Name: AlignmentDifferenceFunction
 * $Function:
 * $Date: 02/11/2016, Los Angeles, CA, USA
 */
bool Dispatcher::AlignmentDifferenceFunction(map<string, string> &parameters) {
  AlignmentDifference alignment_difference;
  alignment_difference.PrintAlignmentDifferenceLogo();
  alignment_difference.Initialize(parameters);
  alignment_difference.CalculateDifference();
  return true;
}



/*
 * $Name: TranslationMemoryFunc
 * $Function: A Translation memory, or TM, is a datebase that stores "segments", which can be 
 *            sentences, paragraphs or sentence-like units (headings, titles or elements in a list)
 *            that have previously been translated, in order to aid human translators.
 * $Date: 2013-05-07
 */
bool Dispatcher::TranslationMemoryFunc(map< string, string > &parameters) {
  TranslationMemory::PrintTranslationMemoryLogo();
  if ( parameters.find( "Translation-Memory-Table") == parameters.end() ) {
    cerr<<"ERROR: Please add \"Translation-Memory-Table\" in your configuration file!\n"<<flush;
    exit( 1 );
  }
  if ( parameters.find( "Punct-Dict") == parameters.end() ) {
    cerr<<"ERROR: Please add \"Punct-Dict\" in your configuration file!\n"<<flush;
    exit( 1 );
  }
  if ( parameters.find( "Ending-Punct-Dict") == parameters.end() ) {
    cerr<<"ERROR: Please add \"Ending-Punct-Dict\" in your configuration file!\n"<<flush;
    exit( 1 );
  }


  TranslationMemoryModel translation_memory_model;
  translation_memory_model.LoadTranslationMemoryModel( parameters[ "Translation-Memory-Table"] );
  translation_memory_model.LoadPunctuationDictionary( parameters[ "Punct-Dict" ] );
  translation_memory_model.LoadEndingPunctuationDictionary( parameters[ "Ending-Punct-Dict" ] );
  
  TranslationMemory translation_memory;
  translation_memory.Process( parameters, translation_memory_model );
  return true;
}


/*
 * $Name: TrainingTMModel
 * $Function:
 * $Date: 2013-05-09
 */
bool Dispatcher::TrainingTMModel(map< string, string > &parameters) {
  TranslationMemory tm;
  tm.TrainingTMModel( parameters );
  return true;
}


/*
 * $Name: CalculateIbmBleuScore
 * $Function:
 * $Date: 2014-04-01, in Nanjing
 */
bool Dispatcher::CalculateIbmBleuScore(map< string, string > &parameters) {
  IbmBleuScore ibm_bleu_score;
  ibm_bleu_score.Process( parameters );
  return true;
}


/*
 * $Name: CalculateWdeMetricScore
 * $Function:
 * $Date: 2014-04-01, in Nanjing
 */
bool Dispatcher::CalculateWdeMetricScore(map< string, string > &parameters) {
  WordDeletionErrorMetric word_deletion_error_metric;
  word_deletion_error_metric.Process(parameters);
  return true;
}


/*
 * $Name: Recaser
 * $Function: If we want to present the output to a user, we need to re-case (or re-capitalize) the output
 *            as we train machine translation systems on lowercased data. This project provides a simple 
 *            tool to recase data, which essentially runs Recasing without reordering, using a word-to-word
 *            translation model and a cased language model.
 * $Date: 2013-05-07
 */
bool Dispatcher::Recaser(map< string, string > &param) {
  Recasing::PrintRecasingLogo();
  Configuration config;
  config.recase_flag_ = true;
  config.Init(param);
  Model model;
  model.Init(config);
  Recasing recasing;
  recasing.Init(config, model);
  recasing.Decode(config, model);
  return true;
}


/*
 * $Name: Tokenizer
 * $Function: tokenizer
 * $Date: 11/25/2015
 */
bool Dispatcher::TokenizerFunction(map< string, string > &parameters) {
  Tokenizer tokenizer;
  tokenizer.PrintTokenizerLogo();
  tokenizer.Initialize(parameters);
  if ("en" == tokenizer.language_) {
    tokenizer.StartTokenEng();
  } else {
    cerr<<"NOT HAVE!!! HAHAHAHA!!!\n"<<flush;
    exit (1);
  }
  return true;
}


/*
 * $Name: SplitSentencesFunction
 * $Function: tokenizer
 * $Date: 12/02/2015
 */
bool Dispatcher::SplitSentencesFunction(map<string, string> &parameters) {
  SplitSentences split_sentences;
  split_sentences.PrintSplitSentencesLogo();
  split_sentences.Initialize(parameters);
  if ("en" == split_sentences.language_) {
    split_sentences.StartSplitSentencesEng();
  } else {
    cerr<<"NOT HAVE!!! HAHAHAHA!!!\n"<<flush;
    exit (1);
  }
  return true;
}


/*
 * $Name: DeTokenizer
 * $Function: A detokenizer merges tokens back to their untokenized representation. 
 * $Date: 2013-05-07
 */
bool Dispatcher::DeTokenizer(map< string, string > &param) {
  Detokenizer::PrintDetokenizerLogo();
  Detokenizer detokenizer_handle;
  if( param.find( "-lang" ) == param.end() || ( param[ "-lang" ] != "en" && param[ "-lang" ] != "zh" ) ) {
    detokenizer_handle.language_ = "en";
  } else {
    detokenizer_handle.language_ = param[ "-lang" ];
  }

  if( param.find( "-punct" ) == param.end() ) {
    cerr<<"Error: please set the parameter '-punct', exit!\n";
    exit( 1 );
  }

  PunctuationMap punctuation_map;
  if ( detokenizer_handle.language_ == "zh" ) {
    punctuation_map.LoadPunctuation( param[ "-punct" ] );
    detokenizer_handle.DetokenizerZh( param, punctuation_map );
  } else if ( detokenizer_handle.language_ == "en" ) {
    detokenizer_handle.DetokenizerEn( param );
  }

  return true;
}


/*
 * $Name: ConvertTextPhraseTable2Binary
 * $Function: Converts text phrase translation table file to binary file. 
 * $Date: 2013-05-07
 */
bool Dispatcher::ConvertTextPhraseTable2Binary (map<string, string> &param) {
  Configuration config;
  config.convert_pt_2_binary_flag_ = true;
  config.testFile    = param["-input"];
  config.output_file_  = param["-output"];
  config.Init (param);

  PhraseTable pt;
  pt.ConvertToBinary (config);
  return true;
}

}

