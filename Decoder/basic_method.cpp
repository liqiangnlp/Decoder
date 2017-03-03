/*
* $Id:
* 0008
*
* $File:
* basic_method.cpp
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
* 2012-12-04,16:29
*/


#include "basic_method.h"


namespace basic_method {

/*
 * $Name: Split
 * $Function: Split string with char
 * $Date: 2014-04-11
 */
bool BasicMethod::Split(const string &phraseTable, const char &splitchar, vector< string > &dest) {
  string::size_type splitPos = phraseTable.find(splitchar);
  string::size_type lastSplitPos = 0;
  string tempString;
  while (splitPos != string::npos) {
    tempString = phraseTable.substr(lastSplitPos, splitPos - lastSplitPos);
    if (!tempString.empty()) {
      dest.push_back(tempString);
    }
    lastSplitPos = splitPos + 1;
    splitPos = phraseTable.find(splitchar, lastSplitPos);
  }
  if (lastSplitPos < phraseTable.size()) {
    tempString = phraseTable.substr(lastSplitPos);
    dest.push_back(tempString);
  }
  if (!dest.empty()) {
    return true;
  } else {
    return false;
  }
}


/*
 * $Name: splitWithStr
 * $Function: Split string with string
 * $Date: 2014-04-11
 */
bool BasicMethod::SplitWithStr(const string &src, const string &separator, vector< string > &dest) {
  string str = src;
  string substring;
  string::size_type start = 0, index = 0;
  string::size_type separator_len = separator.size();
  while (index != string::npos && start < src.size()) {
    index = src.find(separator, start);
    if (index == 0) {
      start = start + separator_len;
      continue;
    }
    if (index == string::npos) {
      dest.push_back(src.substr(start));
      break;
    }
    dest.push_back(src.substr(start,index-start));
    start = index + separator_len;
  }
  return true;
}


/*
 * $Name: size_tToString
 * $Function: 
 * $Date: 2014-04-11
 */
string BasicMethod::size_tToString(size_t &source) {
  stringstream oss;
  oss << source;
  return oss.str();
}


/*
 * $Name: intToString
 * $Function: 
 * $Date: 2014-04-11
 */
string BasicMethod::intToString(int &source) {
  stringstream oss;
  oss << source;
  return oss.str();
}


/*
 * $Name: ConvertCharToString
 * $Function: 
 * $Date: 2014-04-11
 */
string BasicMethod::ConvertCharToString( char &input_char ) {
  stringstream oss;
  oss << input_char;
  return oss.str();
}


/*
 * $Name: ClearIllegalChar
 * $Function: 
 * $Date: 2014-04-11
 */
bool BasicMethod::ClearIllegalChar( string &str ) {
  string::size_type pos = 0;
  while( ( pos = str.find( "\r", pos ) ) != string::npos ) {
    str.replace( pos, 1, "" );
  }

  pos = 0;
  while( ( pos = str.find( "\n", pos ) ) != string::npos ) {
    str.replace( pos, 1, "" );
  }
  return true;
}


/*
 * $Name: toUpper
 * $Function: 
 * $Date: 2014-04-11
 */
bool BasicMethod::toUpper( string &str ) {
  for( string::size_type i = 0; i < str.size(); ++i ) {
    if( islower( ( unsigned char )str.at( i ) ) ) {
      str.at( i ) = toupper( ( unsigned char )str.at( i ) );
    }
  }

  return true;
}


/*
 * $Name: toLower
 * $Function: 
 * $Date: 2014-04-11
 */
bool BasicMethod::ToLower(string &str) {
  for (string::size_type i = 0; i < str.size(); ++i) {
    if (isupper((unsigned char)str.at(i))) {
      str.at(i) = tolower((unsigned char)str.at(i));
    }
  }
  return true;
}


/*
 * $Name: RmEndSpace
 * $Function: no trailing space
 * $Date: 2014-04-11
 */
bool BasicMethod::RmEndSpace(string &str) {
  if (str != "") {
    string tmpStr;
    int pos = (int)str.length() - 1;
    while (pos >= 0 && str[ pos ] == ' ') {
      --pos;
    }
    tmpStr = str.substr(0, pos + 1);
    str = tmpStr;
  }
  return true;
}


/*
 * $Name: RmStartSpace
 * $Function: no leading space
 * $Date: 2014-04-11
 */
bool BasicMethod::RmStartSpace(string &str) {
  string tmpStr;
  size_t pos = 0;
  for( string::iterator iter = str.begin(); iter != str.end(); ++iter ) {
    if( *iter != ' ' ) {
      tmpStr = str.substr( pos, str.length() - pos );
      break;
    } else {
      ++pos;
    }
  }
  str = tmpStr;
  return true;
}

/*
* $Name: RemoveExtractSpace
* $Function: One space only between words
* $Date: 2014-04-11
*/
bool BasicMethod::RemoveExtraSpace( string &input_string, string &output_string ) {
  char preceded_char = ' ';
  for( string::iterator iter = input_string.begin(); iter != input_string.end(); ++ iter ) {
    if( *iter == ' ' && preceded_char == ' ' ) {
      continue;
    } else {
      output_string.push_back( *iter );
      preceded_char = *iter;
    }
  }
  return true;
}

/*
 * $Name: deleteFileList
 * $Function: 
 * $Date: 2014-04-11
 */
bool BasicMethod::deleteFileList( vector<string> &fileList, SystemCommand &systemCommand ) {
  clock_t start,finish;
  string command;

  for( vector< string >::iterator iter = fileList.begin(); iter != fileList.end(); ++iter ) {
    command = systemCommand.delete_command_ + *iter;
    start = clock();
    cerr<<"Delete\n"
        <<"        command : "<<command<<"\n"
        <<"        input   : "<<*iter<<"\n"
        <<flush;

    system( command.c_str() );
    finish = clock();
    cerr<<"        time    : "<<(double)(finish-start)/CLOCKS_PER_SEC<<"s\n"
        <<flush;
  }

  fileList.clear();
  return true;
}

}

