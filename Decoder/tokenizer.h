/*
* $Id:
* 0061
*
* $File:
* tokenizer.h
*
* $Proj:
* Decoder for Statistical Machine Translation
*
* $Func:
* tokenizer
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
* 11/25/2015, 09:55, Los Angeles, USA
*/


#ifndef DECODER_TOKENIZER_H_
#define DECODER_TOKENIZER_H_

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


namespace decoder_tokenizer {

class Tokenizer {
 public:
  string input_file_name_;
  string output_file_name_;
  string output_lc_file_name_;
  string output_clean_file_name_;
  string log_file_name_;

  string prefix_file_name_;

 public:
  string language_;

 public:
  map<string, int> m_prefix_ref_;

 public:
  Tokenizer() {
    input_file_name_ = "";
    output_file_name_ = "";
    output_lc_file_name_ = "";
    log_file_name_ = "";
    prefix_file_name_ = "";

    language_ = "en";
  }
  ~Tokenizer() {}

 public:
  bool Initialize(map<string, string> &parameters);

 public:
  bool StartTokenEng();

 private:
  bool LoadPrefixes(ofstream &log_file);

 private:
  bool SeperateAllSpecialChar(string &line);
  bool StayTogetherMultiDots(string &line);
  bool RestoreMultiDots(string &line);
  bool SeperateComma(string &line);
  bool ConvertPunctuation(string &line);
  bool SplitContractionsRightEng(string &line);
  bool WordTokenMethod(string &line);
  bool ConvertTabToSpace(string &line);
  bool EscapeSpecialChars(string &line);

 private:
  bool IsAlpha(char character);
  bool IsLower(char character);
  bool IsNumber(char character);
  bool IsAlphaNumber(char character);
  bool HaveAlpha(string &line);


 private:
  bool CheckFiles(map<string, string> &parameters);
  bool CheckFile(map<string, string> &parameters, string &file_key);

 public:
  static bool PrintTokenizerLogo();


};


}

#endif

