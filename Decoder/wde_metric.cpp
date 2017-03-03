/*
* $Id:
* 0052
*
* $File:
* wde_metric.cpp
*
* $Proj:
* Decoder for Statistical Machine Translation
*
* $Func:
* Calculate WDE Metric Score
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
* 2015-01-20,14:05, in Beijing
*/

#include "wde_metric.h"

namespace decoder_wde_metric {

/*
 * $Name: Initialize
 * $Function:
 * $Date: 20140411
 */
bool MatchInformation::Initialize( int &max_ngram, string &sentence_id ) {
  sentence_id_ = sentence_id;
  reference_count_.resize( max_ngram + 1, 0 );
  translation_count_.resize( max_ngram + 1, 0 );
  match_count_.resize( max_ngram + 1, 0 );
  return true;
}


/*
 * $Name: Process
 * $Function:
 * $Date: 20140401, in Nanjing
 */
bool WordDeletionErrorMetric::Process( map< string, string > &parameters ) {
  PrintWdeMetricLogo();
  Initialize( parameters );
  ScoreSystem();
  return true;
}


/*
 * $Name: Initialize
 * $Function:
 * $Date: 20140411
 */
bool WordDeletionErrorMetric::Initialize(map< string, string > &parameters) {
  CheckFiles(parameters);
  translation_results_file_name_ = parameters["-1best"];
  src_and_ref_file_name_ = parameters["-dev"];
  stopword_file_name_ = parameters["-stopword"];
  output_file_name_ = parameters["-out"];
  ofstream output_file(output_file_name_.c_str());
  if (!output_file) {
    cerr<<"ERROR: Please check the path of \"-out\":"<<" "<<output_file_name_<<"\n";
    exit(1);
  }
  output_file.close();

  string parameter_key = "-nref";
  string default_value = "1";
  CheckEachParameter( parameters, parameter_key, default_value );
  references_number_ = atoi( parameters[ "-nref" ].c_str() );

  PrintConfiguration();
  LoadingStopword();
  LoadingData();
  return true;
}



bool WordDeletionErrorMetric::PrintConfiguration() {
  cerr<<setfill( ' ' );
  cerr<<"  Configuration:"<<"\n"
      <<"      -nref            :"<<setw( 9 )<<references_number_<<"\n"
      <<"  Translation Results:"<<"\n"
      <<"      "<<translation_results_file_name_<<"\n"
      <<"  Source Sentences and References:"<<"\n"
      <<"      "<<src_and_ref_file_name_<<"\n"
      <<"  Stopword:"<<"\n"
      <<"      "<<stopword_file_name_<<"\n"
      <<flush;
  return true;
}


/*
 * $Name: CheckFiles
 * $Function: If the files used in calculating ibm bleu score do not exist, exit!
 * $Date: 2014-04-01, in Nanjing
 */
bool WordDeletionErrorMetric::CheckFiles( map< string, string > &parameters ) {
  string file_key = "-1best";
  CheckFile(parameters, file_key);

  file_key = "-dev";
  CheckFile(parameters, file_key);

  file_key = "-stopword";
  CheckFile(parameters, file_key);

  if (parameters.find("-out") == parameters.end()) {
    cerr<<"ERROR: Please add parameter \"-out\" in your command line!\n";
    exit(1);
  }
  return true;
}


/*
 * $Name: CheckFile
 * $Function: If the file to be checked does not exist, exit!
 * $Date: 2014-04-01
 */
bool WordDeletionErrorMetric::CheckFile( map< string, string > &parameters, string &file_key ) {
  if ( parameters.find( file_key ) != parameters.end() ) {
    ifstream in_file( parameters[ file_key ].c_str() );
    if ( !in_file ) {
      cerr<<"ERROR: Please check the path of \""<<file_key<<"\".\n"<<flush;
      exit( 1 );
    }
    in_file.clear();
    in_file.close();
  } else {
    cerr<<"ERROR: Please add parameter \""<<file_key<<"\" in your command line!\n"<<flush;
    exit( 1 );
  }
  return true;
}


/*
 * $Name: CheckEachParameter 
 * $Function:
 * $Date: 20140401, in Nanjing
 */
bool WordDeletionErrorMetric::CheckEachParameter( map< string, string > &parameters, string &parameter_key, string &default_value ) {
  if( parameters.find( parameter_key ) == parameters.end() ) {
    parameters[ parameter_key ] = default_value;
  }
  return true;
}


/*
* $Name: LoadingData 
* $Function: Loading source sentences, references, and translation results.
* $Date: 20140411
*/
bool WordDeletionErrorMetric::LoadingData() {
  cerr<<"Start Loading source sentences and references...\n"<<flush;
  clock_t start, finish;
  start = clock();

  ifstream src_and_ref( src_and_ref_file_name_.c_str() );
  if ( !src_and_ref ) {
      cerr<<"ERROR: Please check the path of \""<<src_and_ref_file_name_<<"\".\n"<<flush;
      exit( 1 );
  }

  references_data_.resize( references_number_ );
  for ( size_t i = 0; i < references_number_; ++ i ) {
    references_data_.at( i ).reserve( 5000 );
  }

  string line_of_src_and_ref;
  size_t line_no = 0;
  while( getline( src_and_ref, line_of_src_and_ref ) )
  {
    if( ( line_no % ( references_number_ + 2 ) ) == 0 ) {
      ClearIllegalChar(line_of_src_and_ref);
      source_data_.push_back( line_of_src_and_ref );
    } else if ( ( line_no % ( references_number_ + 2 ) ) != 1 ) {
      string token_line;
      Tokenization( line_of_src_and_ref, token_line );
	  RemoveStopword(token_line);
      references_data_.at( ( line_no % ( references_number_ + 2 ) ) - 2 ).push_back( token_line );
    }
    ++ line_no;
    if( line_no % 10000 == 0 ) {
      cerr<<"\r  Processed "<<line_no<<" lines. ";
    }
  }
  sentences_number_ = (int)line_no/(references_number_ + 2);
  finish = clock();
  cerr<<"\r  Processed "<<line_no<<" lines. [sent="<<sentences_number_
      <<" time="<<(double)( finish - start )/CLOCKS_PER_SEC<<" s speed="
      <<line_no/((double)( finish - start )/CLOCKS_PER_SEC)<<"sent/s]   \n"
      <<"  Done!\n"<<flush;
  src_and_ref.close();

  cerr<<"Start Loading translation results...\n"<<flush;
  start = clock();

  ifstream translation_results( translation_results_file_name_.c_str() );
  if ( !translation_results ) {
      cerr<<"ERROR: Please check the path of \""<<translation_results_file_name_<<"\".\n"<<flush;
      exit( 1 );
  }

  string line_of_tgt;
  translation_results_data_.reserve( 5000 );
  line_no = 0;
  while( getline( translation_results, line_of_tgt ) ){
    string token_line;
    Tokenization( line_of_tgt, token_line );
	RemoveStopword(token_line);
    translation_results_data_.push_back( token_line );
    ++ line_no;
    if( line_no % 10000 == 0 ) {
      cerr<<"\r  Processed "<<line_no<<" lines.";
    }
  } 
  if( sentences_number_ != line_no ){
    cerr<<"  Error: The number of references is not the same as translations!\n";
    exit( 1 );
  }
  finish = clock();
  cerr<<"\r  Processed "<<line_no<<" lines. [time="
      <<(double)( finish - start )/CLOCKS_PER_SEC<<" s speed="
      <<line_no/((double)( finish - start )/CLOCKS_PER_SEC)<<"sent/s]   \n"
      <<"  Done!\n"<<flush;
  translation_results.close();
  return true;
}


/*
* $Name: LoadingStopword 
* $Function: Loading stopwords.
* $Date: 20140411
*/
bool WordDeletionErrorMetric::LoadingStopword() {
  cerr<<"Start Loading Stopwords...\n"<<flush;
  clock_t start, finish;
  start = clock();

  ifstream stopword(stopword_file_name_.c_str());
  if (!stopword) {
      cerr<<"ERROR: Please check the path of \""<<stopword_file_name_<<"\".\n"<<flush;
      exit( 1 );
  }

  string line_of_stopwords;
  size_t line_no = 0;
  while (getline(stopword, line_of_stopwords)) {
    ClearIllegalChar(line_of_stopwords);
	RmEndSpace(line_of_stopwords);
	RmStartSpace(line_of_stopwords);
    stopword_set_.insert(line_of_stopwords);
    ++ line_no;
    if( line_no % 10000 == 0 ) {
      cerr<<"\r  Processed "<<line_no<<" lines. ";
    }
  }
  sentences_number_ = (int)line_no/(references_number_ + 2);
  finish = clock();
  cerr<<"\r  Processed "<<line_no<<" lines. [sent="<<sentences_number_
      <<" time="<<(double)( finish - start )/CLOCKS_PER_SEC<<" s speed="
      <<line_no/((double)( finish - start )/CLOCKS_PER_SEC)<<"sent/s]   \n"
      <<"  Done!\n"<<flush;
  stopword.close();
  return true;
}


/*
* $Name: RemoveStopword 
* $Function: Remove stopwords.from sentence.
* $Date: 20150205
*/
bool WordDeletionErrorMetric::RemoveStopword(string &sentence) {
  vector<string> sentence_vector;
  Split(sentence, ' ', sentence_vector);
  sentence = "";
  bool first_flag = true;
  for (vector<string>::iterator iter = sentence_vector.begin(); iter != sentence_vector.end(); ++iter) {
    if(stopword_set_.find(*iter) == stopword_set_.end()) {
      sentence += " " + *iter;
	}
  }
  RmStartSpace(sentence);
  return true;
}



/*
* $Name: Tokenization 
* $Function:
* $Date: 20150120, in Beijing
*/
bool WordDeletionErrorMetric::Tokenization( string &input_string, string &output_string ) {
  string tmp_output_string;
  tmp_output_string.reserve( 5000 );
  string tmp_string;
  tmp_string.reserve( 5000 );
  output_string.reserve( 5000 );

  ClearIllegalChar( input_string );
  ConvertSgmlTags( input_string, tmp_output_string );

  // language-dependent part (assuming Western languages)
  tmp_output_string = " " + tmp_output_string + " ";

  // lowercase the uppercase letters 
  ToLower( tmp_output_string );

  tmp_string = tmp_output_string;
  tmp_output_string = "";
  TokenizePunctuation( tmp_string, tmp_output_string );
  tmp_string = tmp_output_string;
  tmp_output_string = "";
  TokenizePeriodAndComma( tmp_string, tmp_output_string );
  tmp_string = tmp_output_string;
  tmp_output_string = "";
  TokenizeDash( tmp_string, tmp_output_string );
  tmp_string = tmp_output_string;
  tmp_output_string = "";
  RemoveExtraSpace( tmp_string, tmp_output_string );
  RmStartSpace( tmp_output_string );
  RmEndSpace( tmp_output_string );
  output_string = tmp_output_string;

  return true;
}


/*
 * $Name: ConvertSgmlTags 
 * $Function:
 * $Date: 20150120, in Beijing
 */
bool WordDeletionErrorMetric::ConvertSgmlTags( string &input_string, string &output_string ) {
  // language-independent part
  output_string = input_string;
  size_t current_position = 0;
  while( ( current_position = output_string.find( "<skipped>", current_position ) ) != string::npos ) {
    output_string.replace( current_position, 9, "" );
  }

  // convert SGML tag for quote to "
  current_position = 0;
  while( ( current_position = output_string.find( "&quot;", current_position ) ) != string::npos ) {
    output_string.replace( current_position, 6, "\"" );
  }

  // convert SGML tag for ampersand to &
  current_position = 0;
  while( ( current_position = output_string.find( "&amp;", current_position ) ) != string::npos ) {
    output_string.replace( current_position, 5, "&" );
  }

  // convert SGML tag for less-than to <
  current_position = 0;
  while( ( current_position = output_string.find( "&lt;", current_position ) ) != string::npos ) {
    output_string.replace( current_position, 4, "<" );
  }

  // convert SGML tag for greater-than to >
  current_position = 0;
  while( ( current_position = output_string.find( "&gt;", current_position ) ) != string::npos ) {
    output_string.replace( current_position, 4, ">" );
  }
  return true;
}


/*
 * $Name: TokenizePunctuation 
 * $Function:
 * $Date: 20140414
 */
bool WordDeletionErrorMetric::TokenizePunctuation( string &input_string, string &output_string ) {
  // convert "{", "|", "}", "~" to " { ", " | ", " } ", " ~ " respectively 
  // convert "[", "\", "]", "^", "_", "`" to " [ ", " \ ", " ] ", " ^ ", " _ ", " ` " respectively
  // convert "!", "\"", "#", "$", "%", "&" to " ! ", " \" ", " # ", " $ ", " % ", " & " respectively
  // convert "/" to " / "
  for( string::iterator iter = input_string.begin(); iter != input_string.end(); ++ iter ) {
    if( ( *iter >= '!' && *iter <= '&' ) || ( *iter >= '{' && *iter <= '~' ) || ( *iter >= '[' &&  *iter <= '`' ) ||
        ( *iter >= '(' && *iter <= '+' ) || ( *iter >= ':' && *iter <= '@' ) || *iter == '/' ) {
      output_string.push_back( ' ' );
      output_string.push_back( *iter );
      output_string.push_back( ' ' );
    } else {
      output_string.push_back( *iter );
    }
  }
  return true;
}


/*
* $Name: TokenizePeriodAndComma 
* $Function:
* $Date: 20140414
*/
bool WordDeletionErrorMetric::TokenizePeriodAndComma( string &input_string, string &output_string ) {
  // tokenize period and comma unless preceded by a digit
  char preceded_char = ' ';
  for( string::iterator iter = input_string.begin(); iter != input_string.end(); ++ iter ) {
    if( ( *iter == '.' || *iter == ',' ) && ( preceded_char < '0' || preceded_char > '9' ) ) {
      output_string.push_back( ' ' );
      output_string.push_back( *iter );
      output_string.push_back( ' ' );
      preceded_char = *iter;
    } else {
      output_string.push_back( *iter );
      preceded_char = *iter;
    }
  }

  // tokenize period and comma unless followed by a digit
  string tmp_output_string;
  char followed_char = ' ';
  for( string::reverse_iterator riter = output_string.rbegin(); riter != output_string.rend(); ++ riter ) {
    if( ( *riter == '.' || *riter == ',' ) && ( followed_char < '0' || followed_char > '9' ) ) {
      tmp_output_string.push_back( ' ' );
      tmp_output_string.push_back( *riter );
      tmp_output_string.push_back( ' ' );
      followed_char = *riter;
    } else {
      tmp_output_string.push_back( *riter );
      followed_char = *riter;
    }
  }
  output_string = "";
  for( string::reverse_iterator riter = tmp_output_string.rbegin(); riter != tmp_output_string.rend(); ++ riter ) {
      output_string.push_back( *riter );
  }
  return true;
}


/*
* $Name: TokenizeDash 
* $Function:
* $Date: 20140414
*/
bool WordDeletionErrorMetric::TokenizeDash( string &input_string, string &output_string ) {
  // tokenize dash when preceded by a digit
  string tmp_output_string = "";
  char preceded_char = ' ';
  for( string::iterator iter = input_string.begin(); iter != input_string.end(); ++ iter ) {
    if( *iter == '-' && preceded_char >= '0' && preceded_char <= '9' ) {
      tmp_output_string.push_back( ' ' );
      tmp_output_string.push_back( *iter );
      tmp_output_string.push_back( ' ' );
      preceded_char = *iter;
    } else {
      tmp_output_string.push_back( *iter );
      preceded_char = *iter;
    }
  }
  output_string = tmp_output_string;
  return true;
}


/*
 * $Name: Words2Ngrams 
 * $Function: Convert a string of words to an Ngram count hash
 * $Date: 20140411
 */
bool WordDeletionErrorMetric::Words2Ngrams( vector< string > &words, map< string, int > &ngrams_count ) {

  for( size_t i = 0; i < words.size(); ++ i ) {
    string ngram;
    bool first_flag = true;
    for( size_t j = i; ( j < words.size() ) && ( j - i < max_ngram_ ); ++ j ) {
      if( first_flag ) {
        ngram = words.at( j );
        ++ ngrams_count[ ngram ];
        first_flag = false;
      } else {
        ngram += " " + words.at( j );
        ++ ngrams_count[ ngram ];
      }
    }
  }
  return true;
}


/*
* $Name: ScoreSystem 
* $Function: 
* $Date: 20140416
*/
bool WordDeletionErrorMetric::ScoreSystem() {
  ofstream output_file(output_file_name_.c_str());
  if (!output_file) {
    cerr<<"ERROR: Please check the path of \"-out\":"<<" "<<output_file_name_<<"\n";
    exit(1);
  }
  MatchInformation all_match_information;
  string all_sentence_id( "all_sentence" );
  all_match_information.Initialize( max_ngram_, all_sentence_id );

  for( size_t sentence_id = 0; sentence_id < sentences_number_; ++ sentence_id ) {
    MatchInformation match_information;
    string sentence_id_str( size_tToString( sentence_id ) );
    sentence_id_str += "_sentence";
    match_information.Initialize( max_ngram_, sentence_id_str );

    ScoreSegment( sentence_id, match_information );
    CalculateWdeMetricSmoothing(match_information, output_file);
    for( size_t i = 1; i <= max_ngram_; ++ i ) {
        all_match_information.match_count_.at( i )             += match_information.match_count_.at( i );
        all_match_information.translation_count_.at( i )       += match_information.translation_count_.at( i );
        all_match_information.reference_count_.at( i )         += match_information.reference_count_.at( i );
    }
  }
  CalculateWdeMetricSmoothing(all_match_information, output_file);
  output_file.close();
  return true;
}


/*
 * $Name: ScoreSegment 
 * $Function: 
 * $Date: 20140416
 */
bool WordDeletionErrorMetric::ScoreSegment( size_t &sentence_id, MatchInformation &match_information ) {
  map< string, int > tst_ngrams_count;
  map< string, int > ref_ngrams_max;

  vector< string > tst_words_vec;
  Split( translation_results_data_.at( sentence_id ), ' ', tst_words_vec );
  Words2Ngrams( tst_words_vec, tst_ngrams_count );

  for( size_t i = 1; i <= max_ngram_; ++ i ) {
    match_information.translation_count_.at( i ) = ( ( i <= tst_words_vec.size() ) ? tst_words_vec.size() - i + 1 : 0 ); 
  }
  
  for( size_t reference_id = 0; reference_id < references_number_; ++ reference_id ) {
    vector< string > ref_words_vec;
    map< string, int > ref_ngrams_count;

    Split( references_data_.at( reference_id ).at( sentence_id ), ' ', ref_words_vec );
    Words2Ngrams( ref_words_vec, ref_ngrams_count );
    for( map< string, int >::iterator iter = ref_ngrams_count.begin(); iter != ref_ngrams_count.end(); ++ iter ) {
      if ( ref_ngrams_max.find( iter->first ) != ref_ngrams_max.end() ) {
        if( ref_ngrams_max[ iter->first ] < ref_ngrams_count[ iter->first ] ) {
          ref_ngrams_max[ iter->first ] = ref_ngrams_count[ iter->first ];
        }
      } else {
        ref_ngrams_max[ iter->first ] = ref_ngrams_count[ iter->first ];
      }
    }

    for( int j = 1; j <= max_ngram_; ++ j ) {
      match_information.reference_count_.at( j ) += ( ( j <= ref_words_vec.size() ) ? ( ref_words_vec.size() - j + 1 ) : 0 );
    }

	/*
    if( match_information.current_reference_length_ == 0 ) {
      match_information.current_reference_length_ = (int)ref_words_vec.size();
    } else {
      int reference_length = (int)ref_words_vec.size();
      int candidate_length = (int)tst_words_vec.size();
      match_information.current_reference_length_ = BrevityPenaltyClosest( match_information.current_reference_length_, reference_length, candidate_length );
    }
	*/
  }

  for( map< string, int >::iterator iter = tst_ngrams_count.begin(); iter != tst_ngrams_count.end(); ++ iter ) {
    if( ref_ngrams_max.find( iter->first ) == ref_ngrams_max.end() ) {
      continue;
    } else {
      vector< string > tmp_words_vec;
      Split( iter->first, ' ', tmp_words_vec );
      if( tst_ngrams_count[ iter->first ] <= ref_ngrams_max[ iter->first ] ) {
        match_information.match_count_.at( tmp_words_vec.size() ) += tst_ngrams_count[ iter->first ];
      } else {
        match_information.match_count_.at( tmp_words_vec.size() ) += ref_ngrams_max[ iter->first ];
      }
    }
  }
  return true;
}


/*
 * $Name: CalculateWdeMetricSmoothing
 * $Function: 
 * $Date: 20150120, in Beijing
 */
bool WordDeletionErrorMetric::CalculateWdeMetricSmoothing(MatchInformation &match_information, ofstream &output_file) {
  float wde_score = 0.0f;
  if (match_information.reference_count_.at(1) > 0) {
    wde_score = (float)match_information.match_count_.at(1) / (float)match_information.reference_count_.at(1) * (float)references_number_;
  } else {
    wde_score = 0.0f;
  }
  if (wde_score > 1) {
    wde_score = 1;
  }
  match_information.wdem_score_ = wde_score;
  output_file<<"sentence_id="<<match_information.sentence_id_<<"\tWDEM_SCORE="<<match_information.wdem_score_<<"\n";
  return true;
}


/*
 * $Name: PrintWdeMetricLogo
 * $Function:
 * $Date: 20150120, in Beijing
 */
bool WordDeletionErrorMetric::PrintWdeMetricLogo() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# Calculate Word Deletion Error Metric Score                      #\n"
      <<"#                                             Version 0.0.1       #\n"
      <<"#                                             NEUNLPLab/YAYI corp #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<flush;
  return true;
}



}



