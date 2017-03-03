/*
* Copyright 2013 YaTrans Inc.
*
* $Id:
* 0039
*
* $File:
* translation_memory.h
*
* $Proj:
* Decoder
*
* $Func:
* Translation Memory
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
* 2014-01-12,20:31, add InterfaceForDecoder method
* 2013-05-06,14:45
*/

#ifndef DECODER_TRANSLATION_MEMORY_H_
#define DECODER_TRANSLATION_MEMORY_H_

#include <string>
#include <map>
#include <set>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <utility>
#include "basic_method.h"

#ifndef WIN32
#include <sys/time.h>
#endif

using namespace std;
using namespace basic_method;

namespace decoder_translation_memory {

class TranslationMemoryModel : public BasicMethod {
 public:
  set< string > punctuation_dictionary_;
  set< string > ending_punctuation_dictionary_;
  map< string, string > translation_memory_model_;

 public:
  bool LoadPunctuationDictionary(string &punctuation_dictionary_file);
  bool LoadEndingPunctuationDictionary(string &ending_punctuation_dictionary_file);
  bool LoadTranslationMemoryModel(string &translation_memory_model_file);

};

class SentenceSegmentInfo {
 public:
  string source_sentence_;
  string target_sentence_;
  int    translate_flag_;

  SentenceSegmentInfo() {
    translate_flag_ = false;
  }
  SentenceSegmentInfo(string source_sentence, string target_sentence, bool translate_flag)
	                 : source_sentence_(source_sentence), target_sentence_(target_sentence),translate_flag_(translate_flag) {}
};


class GeneralizationInformation {
 public:
  size_t start_position_;
  size_t end_position_;
  string translation_result_;
  string generalization_;
  string raw_information_;

 public:
  GeneralizationInformation() {}
  GeneralizationInformation(size_t &start_position, size_t &end_position, string &translation_result, string &generalization, string &raw_information)
                    :start_position_(start_position), end_position_(end_position), translation_result_(translation_result), \
                     generalization_(generalization), raw_information_(raw_information) {}
  ~GeneralizationInformation(){}
};



class TranslationMemory : public BasicMethod{
 public:
  TranslationMemory() {}
  ~TranslationMemory() {}


 public:
  enum{ UNFOUND, FOUND, CLAUSETRANS };

 public:
  bool Process(map< string, string > &parameters, TranslationMemoryModel &translation_memory_model);

 public:
  bool InterfaceForDecoder( string &sentence, string &output, TranslationMemoryModel &translation_memory_model );

 public:
  int FindSentenceAndClauseTransRes(string &input_sentence, string &output_sentence, vector< SentenceSegmentInfo > &sentence_segment_infos, TranslationMemoryModel &translation_memory_model);

  int FindClauseTranslationResult(string &input_sentence, string &output_sentence, vector< SentenceSegmentInfo > &sentence_segment_infos, TranslationMemoryModel &translation_memory_model);

  int FindSentenceTranslationResult(string &input_sentence, string &output_sentence, TranslationMemoryModel &translation_memory_model);

 public:
  bool TrainingTMModel(map< string, string > &parameters);

 private:
  bool CheckFiles(map< string, string > &parameters);

  bool CheckFile(map< string, string > &parameters, string &file_key);

 public:
  bool CheckGeneralization(string &source_sentence, string &target_sentence);

 public:
  bool RemoveBeginningPunctuation(string &input_sentence, string &beginning_punctuation, TranslationMemoryModel &translation_memory_model);

  bool RemoveEndingPunctuation(string &input_sentence, string &ending_punctuation, TranslationMemoryModel &translation_memory_model);

 private:
  bool ParseGeneralization(vector< string > &word_vector, string &generalization, vector< pair< string, string > > &generalizations);

  bool ParseGeneralizationForClausesTM(string &informations, multimap< size_t, GeneralizationInformation > &generalization_informations);

 private:
  bool SegmentSentence(string &input_sentence, vector< SentenceSegmentInfo > &sentence_segment_infos,TranslationMemoryModel &translation_memory_model);


 public:
  static bool PrintTrainingTMModelLogo();

 public:
  static bool PrintTranslationMemoryLogo();


 private:
  string configuration_file_;
  string inputted_file_;
  string outputted_file_;

};





}

#endif


