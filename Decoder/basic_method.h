/*
* $Id:
* 0007
*
* $File:
* basic_method.h
*
* $Proj:
* Decoder for Statistical Machine Translation
*
* $Func:
* basic method
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
* 2014-04-11,16:56
* 2013-03-03,20:55
* 2012-12-04,16:30
*/


#ifndef DECODER_BASIC_METHOD_H_
#define DECODER_BASIC_METHOD_H_

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
using namespace std;


namespace basic_method {

class SystemCommand {
 public:
  string sort_file_;
  string delete_command_;

 public:
  SystemCommand(string &newSortFile, string &newDel) : sort_file_(newSortFile), delete_command_(newDel) {}
};


class BasicMethod {
 public:
  typedef string::size_type STRPOS;

 public:
  bool Split(const string &phraseTable, const char &splitchar, vector< string > &dest);

 public:
  bool SplitWithStr(const string &src, const string &separator, vector< string > &dest);

 public:
  bool deleteFileList(vector< string > &fileList, SystemCommand &systemCommand);


 public:
  string size_tToString(size_t &source);
  string intToString(int &source);
  string ConvertCharToString(char &input_char);

 public:
  bool ClearIllegalChar(string &str);

 public:
  bool toUpper(string &line);
  bool ToLower(string &line);

 public:
  bool RmEndSpace(string &line);
  bool RmStartSpace(string &line);

 public:
  bool RemoveExtraSpace(string &input_string, string &output_string);
};

}

#endif

