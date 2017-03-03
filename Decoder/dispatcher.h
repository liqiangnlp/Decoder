/*
 * Copyright 2013 YaTrans Inc.
 *
 * $Id:
 * 0005
 *
 * $File:
 * dispatcher.h
 *
 * $Proj:
 * Decoder for Phrase-Based Statistical Machine Translation
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
 * 2015-10-28, 20:50, in Los Angeles, USA
 * 2013-05-07, 13:48
 */

#ifndef DECODER_DISPATCHER_H_
#define DECODER_DISPATCHER_H_

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <map>
#include <cstdio>
#include "basic_method.h"
#include "phrasebased_decoder.h"
#include "tree2tree_decoder.h"
#include "configuration.h"
#include "OurTrainer.h"
#include "recasing.h"
#include "tokenizer.h"
#include "detokenizer.h"
#include "split_sentences.h"
#include "translation_memory.h"
#include "ibm_bleu_score.h"
#include "wde_metric.h"
#include "multi_thread.h"
#include "rescoring.h"
#include "language_model_score.h"
#include "search_engine.h"
#include "alignment_difference.h"
#include "debug.h"

using namespace std;
using namespace basic_method;
using namespace decoder_phrasebased_decoder;
using namespace decoder_tree2tree_decoder;
using namespace decoder_configuration;
using namespace decoder_our_trainer;
using namespace decoder_recasing;
using namespace decoder_tokenizer;
using namespace decoder_detokenizer;
using namespace decoder_split_sentences;
using namespace decoder_translation_memory;
using namespace decoder_ibm_bleu_score;
using namespace decoder_wde_metric;
using namespace decoder_rescoring;
using namespace language_model_score;
using namespace search_engine;
using namespace alignment_difference;
using namespace multi_thread;
using namespace debug;


namespace decoder_dispatcher {

/*
 * $Class: Dispatcher
 * $Function: A Dispatcher is a class that used to identify different functions.
 * $Date: 2013-05-07
 */
class Dispatcher: public MultiThread {
 public:
  Dispatcher() {};
  ~Dispatcher() {};

 public:
  bool ResolveParameters(int argc, char * argv[]);

 private:
  bool SelectFunction(string &function_icon, map< string, string > &parameters);

 private:
  bool ReadConfigFile(map< string, string > &param);

 private:
  void AccessPhraseBasedDecoderFunction(map< string, string > &parameters);
  void AccessPhraseBasedDecoderBinaryFunction(map< string, string > &parameters);

 private:
  void AccessTree2TreeBasedDecoderFunction(map< string, string > &parameters);

 private:
  void AccessTranslationMemoryFunction(map< string, string > &parameters);
  void AccessTrainingTMModelFunction(map< string, string > &paramters);

 private:
  void AccessCalculateIbmBleuFunction(map< string, string > &parameters);

 private:
  void AccessCalculateWdeMetricFunction(map<string, string> &parameters);

 private:
  void AccessRescoringFunction(map<string, string> &parameters);

 private:
  void AccessLMScoreFunction(map<string, string> &parameters);

 private:
  void AccessBuildIndexFunction(map<string, string> &parameters);

 private:
  void AccessSearchFunction(map<string, string> &parameters);

 private:
  void AccessConvPT2BFunction(map<string, string> &parameters);

 private:
  void AccessRecasingFunction(map<string, string> &parameters);

 private:
  void AccessTokenizerFunction(map<string, string> &parameters);
  void AccessDetokenizerFunction(map<string, string> &parameters);

 private:
  void AccessSplitSentencesFunction(map<string, string> &parameters);

 private:
  void AccessAlignmentDifferenceFunction(map<string, string> &parameters);


 private:
  bool PhraseBasedDecoder(map<string, string> &param);
  bool PhraseBasedTraining(map<string, string> &param);

 private:
  bool Tree2TreeDecoder(map<string, string> &param);

 private:
  bool TranslationMemoryFunc(map<string, string> &parameters);
  bool TrainingTMModel(map<string, string> &parameters);

 private:
  bool CalculateIbmBleuScore(map<string, string> &parameters);

 private:
  bool CalculateWdeMetricScore(map<string, string> &parameters);

 private:
  bool RescoringFunction(map<string, string> &parameters);

 private:
  bool LMScoreFunction(map<string, string> &parameters);

 private:
  bool BuildIndexFunction(map<string, string> &parameters);

 private:
  bool SearchFunction(map<string, string> &parameters);

 private:
  bool AlignmentDifferenceFunction(map<string, string> &parameters);

 private:
  bool PhraseBasedDecoderBina(map< string, string > &parameters);

 private:
  bool Recaser(map< string, string > &parameters);
  
 private:
  bool TokenizerFunction(map<string, string> &parameters);
  bool DeTokenizer(map<string, string> &parameters);

 private:
  bool SplitSentencesFunction(map<string, string> &parameters);

 private:
  bool ConvertTextPhraseTable2Binary(map< string, string > &parameters);
};

}

#endif




