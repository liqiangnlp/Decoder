/*
* $Id:
* 0034
*
* $File:
* detokenizer.cpp
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
* 2013-03-17,20:17
*/


#include "detokenizer.h"


namespace decoder_detokenizer {

bool PunctuationMap::LoadPunctuation( string &punctuation_file ) {
  ifstream pInFile( punctuation_file.c_str() );
  if ( !pInFile ) {
    cerr<<"ERROR: Can not open file \""
        <<punctuation_file
        <<"\".\n"
        <<flush;
    exit( 1 );
  } else {
    cerr<<"Loading Punct File\n"
        <<"  >> From File: "<<punctuation_file<<"\n"
        <<flush;
  }

  clock_t start, finish;
  start = clock();

  string lineOfInFile;
  size_t lineNo = 0;
  while ( getline( pInFile, lineOfInFile ) ) {
    ClearIllegalChar( lineOfInFile );
    vector< string > punct;
    Split( lineOfInFile, '\t', punct );
    if ( punct.size() == 3 ) {
        punctuation_dictionary_.insert( make_pair( punct.at( 0 ), make_pair( punct.at( 1 ), punct.at( 2 ) ) ) );
    }

    finish = clock();
    if ( lineNo % 10000 == 0 ) {
      cerr<<"\r  Processed "<<lineNo<<" lines. [Time:"
          <<(double)( finish - start )/CLOCKS_PER_SEC<<" s speed="
          <<lineNo/((double)( finish - start )/CLOCKS_PER_SEC)<<"sent/s]   "
          <<flush;
    }
    ++lineNo;
  }

  finish = clock();
  cerr<<"\r  Processed "<<lineNo<<" lines. [Time:"
      <<(double)( finish - start )/CLOCKS_PER_SEC<<" s speed="
      <<lineNo/((double)( finish - start )/CLOCKS_PER_SEC)<<"sent/s]   \n"
      <<"  Done!\n\n"<<flush;

  pInFile.close();

  return true;
}



/*
 * $Name:
 * $Function:
 * $Date:
 */
//bool Detokenizer::Init( map< string, string > &param ) {
//  CheckFilesInConf( param );
//  PrintConfig();
//  if( language_ == "zh" ) {
//    LoadPunct();
//  }
//  return true;
//}


/*
 * $Name:
 * $Function:
 * $Date:
 */
/*
bool Detokenizer::Process() {
  if( language_ == "en" ) {
    DetokenizerEn();
  } else {
    DetokenizerZh();
  }
  return true;
}
*/


/*
 * $Name: DetokenizerEn
 * $Function: for offline service 
 * $Date: 2013-05-15
 */
bool Detokenizer::DetokenizerEn( map< string, string > &parameters ) {
  CheckFilesInConf( parameters );
  PrintConfig();
  ifstream inFile( input_file_.c_str() );
  if ( !inFile ) {
    cerr<<"ERROR: Can not open file \""
        <<input_file_
        <<"\".\n"
        <<flush;
    exit( 1 );
  } else {
    cerr<<"Loading Inputted File\n"
        <<"  >> From File: "<<input_file_<<"\n"
        <<flush;
  }

  ofstream outFile( output_file_.c_str() );
  if ( !outFile ) {
    cerr<<"ERROR: Can not open file \""
        <<output_file_
        <<"\".\n"
        <<flush;
    exit( 1 );
  }

  clock_t start, finish;
  start = clock();

  string lineOfInFile;
  size_t lineNo = 0;
  while ( getline( inFile, lineOfInFile ) ) {
    ClearIllegalChar( lineOfInFile );
    if ( !isSpaces( lineOfInFile ) && !isDefinedMark( lineOfInFile ) ) {
      ReplaceSpecChars( lineOfInFile );
      DetokenEnStart  ( lineOfInFile );
      RmEndSpace      ( lineOfInFile );
      RmStartSpace    ( lineOfInFile );
    }

    outFile<<lineOfInFile<<"\n";

    finish = clock();
    if ( lineNo % 5000 == 0 ) {
      cerr<<"\r  Processed "<<lineNo<<" lines. [Time:"
          <<(double)( finish - start )/CLOCKS_PER_SEC<<" s speed="
          <<lineNo/((double)( finish - start )/CLOCKS_PER_SEC)<<"sent/s]   "
          <<flush;
    }
    ++lineNo;
  }
  finish = clock();
  cerr<<"\r  Processed "<<lineNo<<" lines. [Time:"
      <<(double)( finish - start )/CLOCKS_PER_SEC<<" s speed="
      <<lineNo/((double)( finish - start )/CLOCKS_PER_SEC)<<"sent/s]   \n"
      <<"  Done!\n\n"<<flush;

  inFile.close();
  outFile.close();
  return true;
}


/*
 * $Name: DetokenizerZh
 * $Function: for offline service 
 * $Date: 2013-05-15
 */
bool Detokenizer::DetokenizerZh( map< string, string > &parameters, PunctuationMap &punctuation_map ) {
  CheckFilesInConf( parameters );
  PrintConfig();
  ifstream inFile( input_file_.c_str() );
  if ( !inFile ) {
    cerr<<"ERROR: Can not open file \""
        <<input_file_
        <<"\".\n"
        <<flush;
    exit( 1 );
  } else {
    cerr<<"Loading Inputted File\n"
        <<"  >> From File: "<<input_file_<<"\n"
        <<flush;
  }

  ofstream outFile( output_file_.c_str() );
  if ( !outFile ) {
    cerr<<"ERROR: Can not open file \""
        <<output_file_
        <<"\".\n"
        <<flush;
    exit( 1 );
  }

  clock_t start, finish;
  start = clock();

  string lineOfInFile;
  size_t lineNo = 0;
  while ( getline( inFile, lineOfInFile ) ) {
    ClearIllegalChar( lineOfInFile );
    if ( !isSpaces( lineOfInFile ) && !isDefinedMark( lineOfInFile ) ) {
      DetokenZhStart( punctuation_map, lineOfInFile );
    }

    outFile<<lineOfInFile<<"\n";

    finish = clock();
    if ( lineNo % 10000 == 0 ) {
      cerr<<"\r  Processed "<<lineNo<<" lines. [Time:"
          <<(double)( finish - start )/CLOCKS_PER_SEC<<" s speed="
          <<lineNo/((double)( finish - start )/CLOCKS_PER_SEC)<<"sent/s]   "
          <<flush;
    }
    ++lineNo;
  }

  finish = clock();
  cerr<<"\r  Processed "<<lineNo<<" lines. [Time:"
      <<(double)( finish - start )/CLOCKS_PER_SEC<<" s speed="
      <<lineNo/((double)( finish - start )/CLOCKS_PER_SEC)<<"sent/s]   \n"
      <<"  Done!\n\n"<<flush;

  inFile.close();
  outFile.close();
  return true;
}


/*
 * $Name: DetokenizerEn
 * $Function: for online service 
 * $Date: 2013-05-15
 */
bool Detokenizer::DetokenizerEn( string &input_sentence, string &output_sentence ) {
  output_sentence = input_sentence;
  ClearIllegalChar( output_sentence );
  if ( !isSpaces( output_sentence ) && !isDefinedMark( output_sentence ) ) {
    ReplaceSpecChars( output_sentence );
    DetokenEnStart  ( output_sentence );
    RmEndSpace      ( output_sentence );
    RmStartSpace    ( output_sentence );
  }
  return true;
}


/*
 * $Name: DetokenizerZh
 * $Function: for online service
 * $Date: 2013-05-15
 */
bool Detokenizer::DetokenizerZh( PunctuationMap &punctuation_map, string &input_sentence, string &output_sentence ) {
  output_sentence = input_sentence;
  ClearIllegalChar( output_sentence );
  if ( !isSpaces( output_sentence ) && !isDefinedMark( output_sentence ) ) {
    DetokenZhStart( punctuation_map, output_sentence );
  }
  return true;
}


/*
 * $Name:
 * $Function:
 * $Date:
 */
bool Detokenizer::ReplaceSpecChars( string &str ) {
  string::size_type pos = 0;
  while ( ( pos = str.find( " @-@ ", pos ) ) != string::npos ) {
    str.replace( pos, 5, "-" );
  }

  pos = 0;
  while ( ( pos = str.find( "&bar;", pos ) ) != string::npos ) {
    str.replace( pos, 5, "|" );
  }

  pos = 0;
  while ( ( pos = str.find( "&lt;", pos ) ) != string::npos ) {
    str.replace( pos, 4, "<" );
  }

  pos = 0;
  while ( ( pos = str.find( "&gt;", pos ) ) != string::npos ) {
    str.replace( pos, 4, ">" );
  }

  pos = 0;
  while ( ( pos = str.find( "&bra;", pos ) ) != string::npos ) {
    str.replace( pos, 5, "[" );
  }

  pos = 0;
  while ( ( pos = str.find( "&ket;", pos ) ) != string::npos ) {
    str.replace( pos, 5, "]" );
  }

  pos = 0;
  while ( ( pos = str.find( "&quot;", pos ) ) != string::npos ) {
    str.replace( pos, 6, "\"" );
  }

  pos = 0;
  while ( ( pos = str.find( "&apos;", pos ) ) != string::npos ) {
    str.replace( pos, 6, "'" );
  }

  pos = 0;
  while ( ( pos = str.find( "&#91;", pos ) ) != string::npos ) {
    str.replace( pos, 5, "[" );
  }

  pos = 0;
  while ( ( pos = str.find( "&#93;", pos ) ) != string::npos ) {
    str.replace( pos, 5, "]" );
  }

  pos = 0;
  while ( ( pos = str.find( "&amp;", pos ) ) != string::npos ) {
    str.replace( pos, 5, "&" );
  }
  return true;
}


/*
 * $Name:
 * $Function:
 * $Date:
 */
bool Detokenizer::DetokenEnStart( string &str ) {
  vector< string > words;
  Split( str, ' ', words );
  if ( words.size() != 0 ) {
    str = "";
    string connector = " ";
    map< string, int > quoteCount;
    quoteCount[ "'"  ] = 0;
    quoteCount[ "\"" ] = 0;
    string preWord = "";
    int wordCnt = 0;

    for ( vector< string >::iterator iter = words.begin(); iter != words.end(); ++iter ) {
      if ( isLeftDelimiter( *iter ) ) {
        str = str + connector + *iter;
        connector = "";
      } else if ( isRightDelimiter( *iter ) ) {
        str = str + *iter;
        connector = " ";
      } else if ( isHyphen( *iter ) ) {
        str = str + *iter;
        connector = "";
      } else if ( ( wordCnt > 0 ) && isAbbreviation( *iter ) && preWord.size() > 0 && \
//                  isalnum( ( unsigned char )preWord.at( preWord.size() - 1 ) ) ) {
                  isAlphaAndNumber( preWord.at( preWord.size() - 1 ) ) ) {
        str = str + *iter;
        connector = " ";
      } else if ( isQuotMarks( *iter ) ) {
        if ( quoteCount.find( *iter ) == quoteCount.end() ) {
          quoteCount[ *iter ] = 0;
        }

        if ( quoteCount[ *iter ] % 2 == 0 ) {
          if ( *iter == "'" && wordCnt > 0 && preWord.size() > 0 && preWord.at( preWord.size() - 1 ) == 's' ) {
            str = str + *iter;
            connector = " ";
          } else {
            str = str + connector + *iter;
            connector = "";
            ++quoteCount[ *iter ];
          }
        } else {
          str = str + *iter;
          connector = " ";
          ++quoteCount[ *iter ];
        }
      } else {
        str = str + connector + *iter;
        connector = " ";
      }
      preWord = *iter;
      ++wordCnt;
    }
  }
  return true;
}


/*
 * $Name:
 * $Function:
 * $Date:
 */
bool Detokenizer::DetokenZhStart( PunctuationMap &punctuation_map, string &str ) {
  vector< string > words;
  Split( str, ' ', words );
  if ( words.size() != 0 ) {
    str = "";
    string connector = "";
    map< string, int > quoteCount;
    quoteCount[ "'"  ] = 0;
    quoteCount[ "\"" ] = 0;
    string preWord = "";
    int wordCnt = 0;

    for ( vector< string >::iterator iter = words.begin(); iter != words.end(); ++iter ) {
      if ( *iter == "," ) {
        str = str + punctuation_map.punctuation_dictionary_[ "," ].first;
        connector = "";
      } else if ( *iter == "." ) {
        str = str + punctuation_map.punctuation_dictionary_[ "." ].first;
        connector = "";
      } else if ( *iter == "?" ) {
          str = str + punctuation_map.punctuation_dictionary_[ "?" ].first;
          connector = "";
      } else if ( *iter == "!" ) {
          str = str + punctuation_map.punctuation_dictionary_[ "!" ].first;
          connector = "";
      } else if ( *iter == ";" ) {
          str = str + punctuation_map.punctuation_dictionary_[ ";" ].first;
          connector = "";
      } else if ( *iter == "(" ) {
        str = str + punctuation_map.punctuation_dictionary_[ "(" ].first;
        connector = "";
      } else if ( *iter == ")" ) {
        str = str + punctuation_map.punctuation_dictionary_[ ")" ].first;
        connector = "";
      } else if ( *iter == "\"" || *iter == "'" ) {
        if ( quoteCount[ *iter ] % 2 == 0 ) {
          if ( *iter == "\"") {
            str = str + punctuation_map.punctuation_dictionary_[ "\"" ].first;
          } else {
            str = str + punctuation_map.punctuation_dictionary_[ "'" ].first;
          }
          connector = "";
          ++quoteCount[ *iter ];
        } else {
          if ( *iter == "\"" ) {
            str = str + punctuation_map.punctuation_dictionary_[ "\"" ].second;
          } else {
            str = str + punctuation_map.punctuation_dictionary_[ "'"].second;
          }
          connector = "";
          ++quoteCount[ *iter ];
        }
      } else {
        if ( preWord != "" && ( ( preWord.at( preWord.size() - 1 ) >= 65 && preWord.at( preWord.size() - 1 ) <= 90 ) || \
            ( preWord.at( preWord.size() - 1 ) >= 97 && preWord.at( preWord.size() - 1 ) <= 122 ) ) ) {
          if( iter->size() != 0 && ( (iter->at( 0 ) >= 65 && iter->at( 0 ) <= 90) || (iter->at( 0 ) >= 97 && iter->at( 0 ) <= 122) ) ) {
            str = str + " " + *iter;
          } else {
            str = str + connector + *iter;
            connector = "";
          }
        } else {
          str = str + connector + *iter;
          connector = "";
        }
      }
      preWord = *iter;
      ++wordCnt;
    }
  }
  return true;
}


/*
 * $Name:
 * $Function:
 * $Date:
 */
bool Detokenizer::isAbbreviation( string &str ) {
  if( str.size() > 1 && str.at( 0 ) == '\'' ) {
//  if( !isalpha( ( unsigned char )str.at( 1 ) ) ) {
    if( !( ( str.at( 1 ) >= 'a' && str.at( 1 ) <= 'z' ) || \
         ( str.at( 1 ) >= 'A' && str.at( 1 ) <= 'Z' ) ) ) {
      return false;
    }
  } else {
    return false;
  }
  return true;
}


/*
 * $Name:
 * $Function:
 * $Date:
 */
bool Detokenizer::isAlphaAndNumber( char character ) {
  if( ( character >= '0' && character <= '9' ) || \
      ( character >= 'a' && character <= 'z' ) ||
      ( character >= 'A' && character <= 'Z' )    ) {
    return true;
  }
  return false;
}


/*
 * $Name:
 * $Function:
 * $Date:
 */
bool Detokenizer::isLeftDelimiter( string &str ) {
  for ( string::iterator iter = str.begin(); iter != str.end(); ++ iter ) {
    if ( *iter != '$' && *iter != '(' && *iter != '[' && *iter != '{' ) {
                return false;
    }
  }
  return true;
}


/*
 * $Name:
 * $Function:
 * $Date:
 */
bool Detokenizer::isRightDelimiter( string &str ) {
  for ( string::iterator iter = str.begin(); iter != str.end(); ++ iter ) {
    if( *iter != ',' && *iter != '.' && *iter != '?' && *iter != '!' && *iter != ':' && *iter != ';' && \
        *iter != '\\' && *iter != '%' && *iter != '}' && *iter != ']' && *iter != ')' ) {
      return false;
    }
  }
  return true;
}


/*
 * $Name:
 * $Function:
 * $Date:
 */
bool Detokenizer::isQuotMarks( string &str ) {
  for ( string::iterator iter = str.begin(); iter != str.end(); ++iter ) {
    if( *iter != '\'' && *iter != '"' ) {
      return false;
    }
  }
  return true;
}


/*
 * $Name:
 * $Function:
 * $Date:
 */
bool Detokenizer::isHyphen( string &str ) {
  for ( string::iterator iter = str.begin(); iter != str.end(); ++iter ) {
    if ( *iter != '-' && *iter != '/' ) {
      return false;
    }
  }
  return true;
}


/*
 * $Name:
 * $Function:
 * $Date:
 */
bool Detokenizer::isSpaces( string &str ) {
  if( str.size() == 0 ) {
    return true;
  }

  for( string::iterator iter = str.begin(); iter != str.end(); ++iter ) {
    if ( *iter != ' ' ) {
      return false;
    }
  }
  return true;
}


/*
 * $Name:
 * $Function:
 * $Date:
 */
bool Detokenizer::isDefinedMark( string &str ) {
  if( str.size() > 2 && str.at( 0 ) == '<' && str.at( str.size() - 1 ) == '>' ) {
    return true;
  }
  return false;
}


/*
 * $Name:
 * $Function:
 * $Date:
 */
bool Detokenizer::CheckFilesInConf( map< string, string > &param ) {
  if( param.find( "-lang" ) == param.end() || ( param[ "-lang" ] != "en" && param[ "-lang" ] != "zh" ) ) {
    language_ = "en";
  } else {
    language_ = param[ "-lang" ];
  }

  // check for inputted file
  string fileKey = "-input";
  CheckFileInConf( param, fileKey );
  input_file_ = param[ "-input" ];

  // check for outputted file
  if( param.find( "-output" ) == param.end() ) {
    output_file_ = param[ "-input" ] + ".detoken";
  } else {
    output_file_ = param[ "-output" ];
  }

  return true;
}


/*
 * $Name:
 * $Function:
 * $Date:
 */
bool Detokenizer::CheckFileInConf( map< string, string > &param, string &fileKey ) {
  if( param.find( fileKey ) != param.end() ) {
    ifstream inFile( param[ fileKey ].c_str() );
    if ( !inFile ) {
      cerr<<"ERROR: Please check the path of \""<<fileKey<<"\".\n"<<flush;
      exit( 1 );
    }
    inFile.clear();
    inFile.close();
  } else {
    cerr<<"ERROR: Please add parameter \""<<fileKey<<"\" in your config file.\n"<<flush;
    exit( 1 );
  }
  return true;
}


/*
 * $Name:
 * $Function:
 * $Date:
 */
bool Detokenizer::PrintConfig() {
  cerr<<setfill( ' ' );
  cerr<<"Configuration:"<<"\n"
      <<"    -lang    :  "<<language_<<"\n"
      <<"    -input   :  "<<input_file_<<"\n"
      <<"    -output  :  "<<output_file_<<"\n\n"
      <<flush;

  return true;
}


/*
 * $Name:
 * $Function:
 * $Date:
 */
bool Detokenizer::PrintDetokenizerLogo() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# Detokenizer for Statistical Machine Translation                 #\n"
      <<"#                                             Version 0.0.1       #\n"
      <<"#                                             NEUNLPLab/YAYI corp #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<flush;
  return true;
}

}

