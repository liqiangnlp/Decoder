/*
* $Id:
* 0063
*
* $File:
* split_sentences.h
*
* $Proj:
* Decoder for Statistical Machine Translation
*
* $Func:
* Split Sentences
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
* 12/02/2015, 09:55, Los Angeles, USA
*/


#ifndef DECODER_SPLIT_SENTENCES_H_
#define DECODER_SPLIT_SENTENCES_H_

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <set>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <ctime>
#include <map>
#include "basic_method.h"
using namespace std;
using namespace basic_method;


namespace decoder_split_sentences {

class SplitSentences{

 public:
  string input_file_name_;
  string output_file_name_;
  string log_file_name_;

  string prefix_file_name_;

 public:
  string language_;

 public:
  map<string, int> m_prefix_ref_;


 public:
  SplitSentences() {
    input_file_name_ = "";
    output_file_name_ = "";
    log_file_name_ = "";
    prefix_file_name_ = "";
    language_ = "en";
  }
  ~SplitSentences() {}

 public:
  bool Initialize(map<string, string> &parameters);

 public:
  bool StartSplitSentencesEng();

 private:
  bool LoadPrefixes(ofstream &log_file);

 private:
  bool ConvertTabToSpace(string &line);
  bool ProcessNonPeriodEnd(string &line);
  bool ProcessMultiDots(string &line);
  bool EndPunctInsideQuote(string &line);
  bool EndPunctuation(string &line);
  bool CheckAllRemainPeriods(string &line);
  bool ClearSentences(string &line);

 private:
  bool IsKnownHonorific(string &word);
  bool IsUppercaseAcronym(string &word);
  bool IsNextWordUppercaseOrNumber(string &word);
  bool IsUppercaseOrNumber(char character);
  string GetPrefixGivenWord(string &word);

 private:
  bool IsPi(char character);
  bool IsPf(char character);
  bool IsLower(char character);
  bool IsUpper(char character);
  bool IsNumber(char character);
  bool IsAlphaNumber(char character);
  bool IsPossibleSentenceStarter(string::iterator iter, string &line);
  bool IsSentenceStarter(string::iterator iter, string &line);

 private:
  bool CheckFiles(map<string, string> &parameters);
  bool CheckFile(map<string, string> &parameters, string &file_key);


 public:
  static bool PrintSplitSentencesLogo();


};


}

#endif

