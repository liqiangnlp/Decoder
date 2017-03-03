/*
* $Id:
* 0033
*
* $File:
* detokenizer.h
*
* $Proj:
* Detokenizer for Statistical Machine Translation
*
* $Func:
* detokenizer
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
* 2013-03-17,20:16
*/

#ifndef DECODER_DETOKENIZER_H_
#define DECODER_DETOKENIZER_H_

#include <iostream>
#include <iomanip>
#include <map>
#include <utility>
#include <string>
#include <cctype>
#include <ctime>
#include "basic_method.h"

#ifndef WIN32
#include <sys/time.h>
#endif

using namespace std;
using namespace basic_method;

namespace decoder_detokenizer {

class PunctuationMap: public BasicMethod{
 public:
  map< string, pair< string, string > > punctuation_dictionary_;

 public:
  bool LoadPunctuation( string &punctuation_file );
};

class Detokenizer: public BasicMethod {
 public:
  string language_;
  string input_file_;
  string output_file_;

 public:
  Detokenizer(){}
  ~Detokenizer(){}

 public:
  // for offline service
  bool DetokenizerEn( map< string, string > &parameters );
  bool DetokenizerZh( map< string, string > &parameters, PunctuationMap &punctuation_map );

 public:
  // for online service
  bool DetokenizerEn( string &input_sentence, string &output_sentence );
  bool DetokenizerZh( PunctuationMap &punctuation_map, string &input_sentence, string &output_sentence );

 private:
  bool ReplaceSpecChars( string &str );
  bool DetokenEnStart( string &str );
  bool DetokenZhStart( PunctuationMap &punctuation_map, string &str );
  bool isAbbreviation( string &str );
  bool isAlphaAndNumber( char character );
  bool isLeftDelimiter( string &str );
  bool isRightDelimiter( string &str );
  bool isQuotMarks( string &str );
  bool isHyphen( string &str );
  bool isSpaces( string &str );
  bool isDefinedMark( string &str );
  bool CheckFilesInConf( map< string, string > &param );
  bool CheckFileInConf( map< string, string > &param, string &fileKey );
  bool PrintConfig();

 public:
  static bool PrintDetokenizerLogo();
};

}

#endif


