/*
* Copyright 2013 YaTrans Inc.
*
* $Id:
* 0040
*
* $File:
* translation_memory.cpp
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
* 2014-01-07,16:23, bug fixes
* 2013-05-06,14:46, bug fixes
*/

#include "translation_memory.h"

namespace decoder_translation_memory{

/*
 * $Name: LoadPunctuationDictionary
 * $Function: 
 * $Date: 2013-05-09
 */
bool TranslationMemoryModel::LoadPunctuationDictionary( string &punctuation_dictionary_file ) {
  cerr<<"Loading punctuation dictionary...\n"
      <<"  From File: "<<punctuation_dictionary_file<<"\n";
  ifstream in_file( punctuation_dictionary_file.c_str() );
  if ( !in_file ) {
    cerr<<"  ERROR: Please check the path of \""<<punctuation_dictionary_file<<"\".\n"<<flush;
    exit( 1 );
  }

  string punctuation_in_dictionary;
  size_t punctuation_count = 0;
  while ( getline( in_file, punctuation_in_dictionary ) ) {
      ++punctuation_count;
      ClearIllegalChar( punctuation_in_dictionary );
      RmEndSpace( punctuation_in_dictionary );
      RmStartSpace( punctuation_in_dictionary );
      punctuation_dictionary_.insert( punctuation_in_dictionary );

      cerr<<"\r  Loading "<<punctuation_count<<" punctuation."<<flush;
  }
  cerr<<"\r  Loading "<<punctuation_count<<" punctuation.\n"<<flush;
  in_file.close();
  return true;
}


/*
 * $Name: LoadEndingPunctuationDictionary
 * $Function: 
 * $Date: 2013-11-22
 */
bool TranslationMemoryModel::LoadEndingPunctuationDictionary( string &ending_punctuation_dictionary_file ) {
  cerr<<"Loading ending punctuation dictionary...\n"
      <<"  From File: "<<ending_punctuation_dictionary_file<<"\n";
  ifstream in_file( ending_punctuation_dictionary_file.c_str() );
  if ( !in_file ) {
    cerr<<"  ERROR: Please check the path of \""<<ending_punctuation_dictionary_file<<"\".\n"<<flush;
    exit( 1 );
  }

  string ending_punctuation_in_dictionary;
  size_t ending_punctuation_count = 0;
  while ( getline( in_file, ending_punctuation_in_dictionary ) ) {
    ++ending_punctuation_count;
    ClearIllegalChar( ending_punctuation_in_dictionary );
    RmEndSpace( ending_punctuation_in_dictionary );
    RmStartSpace( ending_punctuation_in_dictionary );
    ending_punctuation_dictionary_.insert( ending_punctuation_in_dictionary );

    cerr<<"\r  Loading "<<ending_punctuation_count<<" punctuation."<<flush;
  }
  cerr<<"\r  Loading "<<ending_punctuation_count<<" punctuation.\n"<<flush;
  in_file.close();
  return true;
}

/*
 * $Name: LoadTranslationMemoryModel
 * $Function:
 * $Date: 2013-05-13
 */
bool TranslationMemoryModel::LoadTranslationMemoryModel(string &translation_memory_model_file) {
  ifstream in_file( translation_memory_model_file.c_str() );
  if ( !in_file ) {
    cerr<<"ERROR: Please check the path of \""<<translation_memory_model_file<<"\".\n"<<flush;
    exit( 1 );
  }
  string input_sentence_pair;
  size_t sentence_pairs_count = 0;
  char control_char_01 = 0x01;
  cerr<<"Loading translation memory model...\n"
      <<"  >> From File: "<<translation_memory_model_file<<"\n";
  while( getline( in_file, input_sentence_pair ) ) {
    ++sentence_pairs_count;
    ClearIllegalChar( input_sentence_pair );
    vector< string > sentence_vector;
    Split( input_sentence_pair, control_char_01, sentence_vector );
    if ( sentence_vector.size() != 2 ) {
        cerr<<"WARNING: Format error in line "<<sentence_pairs_count<<".\n"<<flush;
        continue;
    } else {
        translation_memory_model_.insert( make_pair( sentence_vector.at( 0 ), sentence_vector.at( 1 ) ) );
    }

    if ( sentence_pairs_count % 10000 == 0 ) {
      cerr<<"\r  Processed "<<sentence_pairs_count<<" lines."<<flush;
    }
  }
  cerr<<"\r  Processed "<<sentence_pairs_count<<" lines.\n"<<flush;

  in_file.close();
  return true;
}


/*
 * $Name: Process
 * $Function: 
 * $Date: 2013-05-13
 */
bool TranslationMemory::Process(map<string,string> &parameters, TranslationMemoryModel &translation_memory_model) {
  if ( parameters.find( "-input" ) == parameters.end() ) {
    cerr<<"ERROR: Please add \"-input\" in your command!\n"<<flush;
    exit( 1 );
  } else if ( parameters.find( "-output" ) == parameters.end() ) {
    cerr<<"ERROR: Please add \"-output\" in your command!\n"<<flush;
    exit( 1 );
  }
  
  ifstream in_file( parameters[ "-input" ].c_str() );
  ofstream out_file( parameters[ "-output" ].c_str() );
  if ( !in_file ) {
    cerr<<"ERROR: Please check the path of \""<<parameters[ "-input" ]<<"\".\n"<<flush;
    exit( 1 );
  }
  if ( !out_file ) {
    cerr<<"ERROR: Please check the path of \""<<parameters[ "-output" ]<<"\".\n"<<flush;
    exit( 1 );
  }

  size_t input_sentence_count = 0;
  string input_sentence;
  char   control_char_01 = 0x01;

#ifndef WIN32
  timeval start_time, end_time;
  gettimeofday( &start_time, NULL );
#else
  clock_t start_time = clock();
  clock_t end_time = 0;
#endif

  cerr<<"Finding translation results...\n"<<flush;
  while( getline( in_file, input_sentence ) ) {
    ++ input_sentence_count;

    ClearIllegalChar( input_sentence );
    RmStartSpace( input_sentence );
    RmEndSpace( input_sentence );
    if ( input_sentence == "" ) {
      out_file<<"\n";
      continue;
    }

    string input_sentence_bak( input_sentence );
    string output_sentence;

    int tm_find_flag = 0;
    vector< SentenceSegmentInfo > sentence_segment_infos;
    if( ( tm_find_flag = FindSentenceAndClauseTransRes( input_sentence, output_sentence, sentence_segment_infos,translation_memory_model ) ) == TranslationMemory::UNFOUND ) {
      out_file<<input_sentence_bak<<control_char_01<<"[NOT FOUND]\n";
    } else if ( tm_find_flag == TranslationMemory::FOUND ) { 
      out_file<<input_sentence_bak<<control_char_01<<output_sentence<<"\n";
    } else if ( tm_find_flag == TranslationMemory::CLAUSETRANS ) {
      out_file<<input_sentence_bak<<control_char_01;
      for( vector< SentenceSegmentInfo >::iterator iter = sentence_segment_infos.begin(); iter != sentence_segment_infos.end(); ++iter ) {
        if( iter->translate_flag_ == TranslationMemory::FOUND ) {
          out_file<<"[CLAUSE: "<<iter->target_sentence_<<"] ";
        } else {
          out_file<<"[SRC: "<<iter->source_sentence_<<"] ";
        }
      }
      out_file<<"\n";
    }

#ifndef WIN32
    gettimeofday( &end_time, NULL );
    double time = ( (double)( end_time.tv_sec - start_time.tv_sec ) * 1000000 + (double)(end_time.tv_usec - start_time.tv_usec) ) / 1000000;
#else
    end_time = clock();
    double time = ( double )( end_time - start_time )/CLOCKS_PER_SEC;
#endif

    if ( input_sentence_count % 10000 == 0 ) {
        cerr<<"\r  Processed "<<input_sentence_count<<". [time="<<time
            <<" s speed="<<input_sentence_count/time<<"sent/s]"<<flush;
    }
  }

#ifndef WIN32
  gettimeofday( &end_time, NULL );
  double time = ( (double)( end_time.tv_sec - start_time.tv_sec ) * 1000000 + (double)(end_time.tv_usec - start_time.tv_usec) ) / 1000000;
#else
  end_time = clock();
  double time = ( double )( end_time - start_time )/CLOCKS_PER_SEC;
#endif

  cerr<<"\r  Processed "<<input_sentence_count<<". [time="<<time
      <<" s speed="<<input_sentence_count/time<<"sent/s]\n"<<flush;
  in_file.close();
  out_file.close();

  return true;
}


/*
* $Name: InterfaceForDecoder
* $Function: 
* $Date: 2014-01-12
*/
bool TranslationMemory::InterfaceForDecoder( string &input_sentence, string &output_sentence, TranslationMemoryModel &translation_memory_model ) {
  if( input_sentence == "" ) {
      output_sentence = "";
      return false;
  }

  int tm_find_flag = 0;
  vector< SentenceSegmentInfo > sentence_segment_infos;
  if ( ( tm_find_flag = FindSentenceAndClauseTransRes( input_sentence, output_sentence, \
         sentence_segment_infos, translation_memory_model ) ) == TranslationMemory::UNFOUND ) {
    output_sentence = "0 ||||| " + input_sentence;
  } else if ( tm_find_flag == TranslationMemory::CLAUSETRANS ) {
    output_sentence = "";

    for( vector< SentenceSegmentInfo >::iterator iter = sentence_segment_infos.begin(); \
         iter != sentence_segment_infos.end(); ++iter ) {
      if( iter->translate_flag_ == TranslationMemory::FOUND ) {
        if ( output_sentence != "" ) {
          output_sentence += " |||||| 1 ||||| " + iter->target_sentence_;
        } else {
          output_sentence += "1 ||||| " + iter->target_sentence_;;
        }
      } else {
        if( output_sentence != "" ) {
          output_sentence += " |||||| 0 ||||| " + iter->source_sentence_;
        } else {
          output_sentence += "0 ||||| " + iter->source_sentence_;
        }
      }
    }
  } else if ( tm_find_flag == TranslationMemory::FOUND ) {
    string tmp_sentence = output_sentence;
    output_sentence = "1 ||||| " + tmp_sentence;
  }

  return true;
}





/*
* $Name: FindSentenceAndClauseTranslationResult
* $Function: Find translation results for the whole sentence and the clauses.
* $Date: 2013-11-22
*/
int TranslationMemory::FindSentenceAndClauseTransRes(string &input_sentence, string &output_sentence, vector< SentenceSegmentInfo > &sentence_segment_infos, TranslationMemoryModel &translation_memory_model) {
  int tm_find_flag = UNFOUND;

  tm_find_flag = FindSentenceTranslationResult(input_sentence, output_sentence, translation_memory_model);

  if ( tm_find_flag == UNFOUND ) {
    tm_find_flag = FindClauseTranslationResult(input_sentence, output_sentence, sentence_segment_infos, translation_memory_model );
  } 

  return tm_find_flag;
}

/*
 * $Name: FindSentenceTranslationResult
 * $Function: Find sentence translation results.
 * $Date: 2013-11-22
 */
int TranslationMemory::FindSentenceTranslationResult(string &input_sentence, string &output_sentence, TranslationMemoryModel &translation_memory_model) {
  if ( input_sentence == "" ) {
    return UNFOUND;
  }
  ClearIllegalChar( input_sentence );
  RmEndSpace( input_sentence );
  RmStartSpace( input_sentence );
  vector< string > sentence_and_generalization;
  sentence_and_generalization.reserve( 2 );
  string::size_type tmp_position;
  if ( ( tmp_position = input_sentence.find( " |||| " ) ) != string::npos ) {
    SplitWithStr( input_sentence, " |||| ", sentence_and_generalization );
  } else {
    sentence_and_generalization.push_back( input_sentence );
  }

  vector< pair< string, string > > generalizations;
  generalizations.reserve( 10 );
  if ( sentence_and_generalization.size() == 2 ) {
    vector< string > sentence_vector;
    Split( sentence_and_generalization.at( 0 ), ' ', sentence_vector );
    ParseGeneralization( sentence_vector, sentence_and_generalization.at( 1 ), generalizations );
  } else if ( sentence_and_generalization.size() > 2 ) {
      return UNFOUND;
  }

  string ending_punctuation;
  string beginning_punctuation;
  RemoveBeginningPunctuation( sentence_and_generalization.at( 0 ), beginning_punctuation, translation_memory_model );
  RemoveEndingPunctuation( sentence_and_generalization.at( 0 ), ending_punctuation, translation_memory_model );

  if ( translation_memory_model.translation_memory_model_.find( sentence_and_generalization.at( 0 ) ) != translation_memory_model.translation_memory_model_.end() ) {
    output_sentence = translation_memory_model.translation_memory_model_[ sentence_and_generalization.at( 0 ) ];
    if ( beginning_punctuation != "" ) {
      output_sentence = beginning_punctuation + " " + output_sentence;
    }
    if ( ending_punctuation != "" ) {
      output_sentence = output_sentence + " " + ending_punctuation;
    }

    if ( generalizations.size() != 0 ) {
      for ( vector< pair< string, string > >::iterator iter = generalizations.begin(); iter != generalizations.end(); ++iter ) {
        string::size_type position;
        if ( ( position = output_sentence.find( iter->first ) ) != string::npos ) {
          output_sentence.replace( position, iter->first.size(), iter->second );
        }
      }
    }
  } else {
    return UNFOUND;
  }
  return FOUND;
}

/*
* $Name: FindClauseTranslationResult
* $Function: Find clauses translation results.
* $Date: 2013-11-22
*/
int TranslationMemory::FindClauseTranslationResult(string &input_sentence, string &output_sentence, vector< SentenceSegmentInfo > &sentence_segment_infos, TranslationMemoryModel &translation_memory_model) {
  if ( input_sentence == "" ) {
    return UNFOUND;
  }

  int tm_find_flag = UNFOUND;
  SegmentSentence( input_sentence, sentence_segment_infos, translation_memory_model );

  for ( vector< SentenceSegmentInfo >::iterator iter = sentence_segment_infos.begin(); iter != sentence_segment_infos.end(); ++iter ) {
    iter->translate_flag_ = FindSentenceTranslationResult( iter->source_sentence_, iter->target_sentence_, translation_memory_model );
    tm_find_flag += iter->translate_flag_;
  }

  if ( tm_find_flag > 0 ) {
    tm_find_flag = CLAUSETRANS;
  }

  return tm_find_flag;
}

/*
 * $Name: TrainingTMModel
 * $Function: Training translation memory model.
 * $Date: 2013-05-09
 */
bool TranslationMemory::TrainingTMModel(map< string, string > &parameters)
{
  PrintTrainingTMModelLogo();

  CheckFiles( parameters );

  TranslationMemoryModel translation_memory_model;
  translation_memory_model.LoadPunctuationDictionary( parameters[ "-punct" ] );

  ofstream out_file( parameters[ "-out" ].c_str() );
  if ( !out_file ) {
    cerr<<"ERROR: Please check the path of \""<<parameters[ "-out" ]<<"\".\n"<<flush;
    exit( 1 );
  }

  ifstream in_src_file( parameters[ "-src" ].c_str() );
  if ( !in_src_file ) {
    cerr<<"ERROR: Please check the path of \""<<parameters[ "-src" ]<<"\".\n"<<flush;
    exit( 1 );
  }
  ifstream in_tgt_file( parameters[ "-tgt" ].c_str() );
  if ( !in_tgt_file ) {
    cerr<<"ERROR: Please check the path of \""<<parameters[ "-tgt" ]<<"\".\n"<<flush;
    exit( 1 );
  }

  string input_source_sentence;
  string input_target_sentence;
  size_t sentence_count = 0;
  size_t correct_count = 0;
  size_t error_count = 0;
  char   control_char_01 = 0x01;
  string source_punctuation;
  string target_punctuation;
  cerr<<"Start training translation memory model...\n";
  while ( getline( in_src_file, input_source_sentence ) ) {
    ++sentence_count;
    ClearIllegalChar( input_source_sentence );
    RmEndSpace( input_source_sentence );
    RmStartSpace( input_source_sentence );
    getline( in_tgt_file, input_target_sentence );
    ClearIllegalChar( input_target_sentence );
    RmEndSpace( input_target_sentence );
    RmStartSpace( input_target_sentence );
    if ( input_source_sentence == "" || input_target_sentence == "" ) {
        ++error_count;
        continue;
    } else if ( CheckGeneralization( input_source_sentence, input_target_sentence ) ) {
      RemoveBeginningPunctuation( input_source_sentence, source_punctuation, translation_memory_model );
      RemoveBeginningPunctuation( input_target_sentence, target_punctuation, translation_memory_model );
      RemoveEndingPunctuation( input_source_sentence, source_punctuation, translation_memory_model );
      RemoveEndingPunctuation( input_target_sentence, target_punctuation, translation_memory_model );
      if( input_source_sentence == "" || input_target_sentence == "" ) {
        ++error_count;
        continue;
      } else {
        ++correct_count;
        translation_memory_model.translation_memory_model_.insert( make_pair( input_source_sentence, input_target_sentence ) );
      }
    } else {
      ++error_count;
      continue;
    }

    if ( sentence_count % 10000 == 0 ) {
      cerr<<"\r  Processed "<<sentence_count<<" lines. [CORRECT="<<correct_count<<" ERROR="<<error_count<<"]"<<flush;;
    }
  }
  cerr<<"\r  Processed "<<sentence_count<<" lines. [CORRENT="<<correct_count<<" ERROR="<<error_count<<"]\n"<<flush;

  cerr<<"Remove duplicate according to the source language, output final TM model...\n";
  size_t sentence_pair_count = 0;
  for ( map< string, string >::iterator iter = translation_memory_model.translation_memory_model_.begin(); \
        iter != translation_memory_model.translation_memory_model_.end(); ++iter ) {
    ++ sentence_pair_count;
    out_file<<iter->first<<control_char_01<<iter->second<<"\n";
    if ( sentence_pair_count % 10000 == 0 ) {
      cerr<<"\r  Processed "<<sentence_pair_count<<" lines."<<flush;
    }
  }
  cerr<<"\r  Processed "<<sentence_pair_count<<" lines.\n"<<flush;

  in_src_file.close();
  in_tgt_file.close();
  out_file.close();
  return true;
}


/*
 * $Name: CheckFiles
 * $Function: If the files used in training TM model does not exist, exit!
 * $Date: 2013-05-09
 */
bool TranslationMemory::CheckFiles( map< string, string > &parameters ) {
  string file_key = "-src";
  CheckFile( parameters, file_key );

  file_key = "-tgt";
  CheckFile( parameters, file_key );

  file_key = "-punct";
  CheckFile( parameters, file_key );

  if( parameters.find( "-out" ) == parameters.end() ) {
    cerr<<"ERROR: Please add parameter \"-out\" in your command line!\n";
  }
  return true;
}


/*
 * $Name: CheckFile
 * $Function: If the file to be checked does not exist, exit!
 * $Date: 2013-05-09
 */
bool TranslationMemory::CheckFile( map< string, string > &parameters, string &file_key ) {
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
 * $Name: CheckGeneralization
 * $Function:
 * $Date: 2013-05-09
 */
bool TranslationMemory::CheckGeneralization(string &source_sentence, string &target_sentence) {
  if ( source_sentence.find( "$" ) != string::npos || target_sentence.find( "$" ) != string::npos ) {
    vector< string > source_sentence_vector;
    vector< string > target_sentence_vector;
    Split( source_sentence, ' ', source_sentence_vector );
    Split( target_sentence, ' ', target_sentence_vector );

    map< string, int > source_gene_count;
    map< string, int > target_gene_count;
    for ( vector< string >::iterator iter = source_sentence_vector.begin(); iter != source_sentence_vector.end(); ++ iter ) {
      if ( *iter == "$date" ) {
        ++ source_gene_count[ "$date" ];
      } else if ( *iter == "$time" ) {
        ++ source_gene_count[ "$time" ];
      } else if ( *iter == "$number" ) {
        ++ source_gene_count[ "$number" ];
      }
    }
    for ( vector< string >::iterator iter = target_sentence_vector.begin(); iter != target_sentence_vector.end(); ++ iter ) {
      if ( *iter == "$date" ) {
        ++ target_gene_count[ "$date" ];
      } else if ( *iter == "$time" ) {
        ++ target_gene_count[ "$time" ];
      } else if( *iter == "$number" ) {
        ++ target_gene_count[ "$number" ];
      }
    }

    if ( source_gene_count.size() != target_gene_count.size() ) {
      return false;
    } else {
      if( source_gene_count[ "$date" ] != target_gene_count[ "$date" ] ) {
        return false;
      } else if ( source_gene_count[ "$time" ] != target_gene_count[ "$time" ] ) {
        return false;
      } else if ( source_gene_count[ "$number" ] != target_gene_count[ "$number" ] ) {
        return false;
      }
    }
  }
  return true;
}

/*
* $Name: RemoveBeginningPunctuation
* $Function: Remove beginning punctuation. The inputted sentence can not be null.
* $Date: 2013-11-25
*/
bool TranslationMemory::RemoveBeginningPunctuation(string &input_sentence, string &beginning_punctuation, TranslationMemoryModel &translation_memory_model) {
  if ( beginning_punctuation != "" ) {
    beginning_punctuation = "";
  }

  vector< string > word_vector;
  Split( input_sentence, ' ', word_vector );
  size_t punctuation_count = 0;
  for ( vector< string >::iterator iter = word_vector.begin(); iter != word_vector.end(); ++iter ) {
    if ( translation_memory_model.punctuation_dictionary_.find( *iter ) != translation_memory_model.punctuation_dictionary_.end() ) {
      ++punctuation_count;
      if ( beginning_punctuation != "" ) {
        beginning_punctuation = beginning_punctuation + " " + *iter; 
      } else {
        beginning_punctuation = *iter;
      }
    } else {
      break;
    }
  }

  if ( punctuation_count > 0 ) {
    input_sentence = "";
    for( size_t start = punctuation_count; start < word_vector.size(); ++ start ) {
      if( input_sentence == "" ) {
        input_sentence = word_vector.at( start );
      } else {
        input_sentence += " " + word_vector.at( start );
      }     
    }
  }
  return true;
}


/*
 * $Name: RemoveEndingPunctuation
 * $Function: Remove ending punctuation. The inputted sentence can not be null.
 * $Date: 2013-05-09
 */
bool TranslationMemory::RemoveEndingPunctuation(string &input_sentence, string &ending_punctuation, TranslationMemoryModel &translation_memory_model) {
  if ( ending_punctuation != "" ) {
    ending_punctuation = "";
  }

  vector< string > word_vector;
  Split( input_sentence, ' ', word_vector );
  size_t punctuation_count = 0;
  for ( vector< string >::reverse_iterator riter = word_vector.rbegin(); riter != word_vector.rend(); ++riter ) {
    if ( translation_memory_model.punctuation_dictionary_.find( *riter ) != translation_memory_model.punctuation_dictionary_.end() ) {
      ++punctuation_count;
      if( ending_punctuation != "" ) {
        ending_punctuation = *riter + " " + ending_punctuation;
      } else {
        ending_punctuation = *riter;
      }
    } else {
      break;
    }
  }

  if ( punctuation_count > 0 ) {
    input_sentence = "";
    size_t word_count = 0;
    for( vector< string >::iterator iter = word_vector.begin(); iter != word_vector.end() \
         && word_count < word_vector.size() - punctuation_count; ++iter ) {
      if ( input_sentence == "" ) {
        input_sentence = *iter;
      } else {
        input_sentence += " " + *iter;
      }
      ++word_count;
    }
  }
  return true;
}


/*
 * $Name: ParseGeneralization
 * $Function: Parse generalizations for translation memory.
 * $Date: 2013-05-11
 */
bool TranslationMemory::ParseGeneralization( vector< string > &word_vector, string &generalization, vector< pair< string, string > > &generalizations ) {
  vector< string > generalization_vector;

  string tmp="";
  for ( string::iterator iter  = generalization.begin(); iter != generalization.end(); ++iter ) {
    if ( *iter == '{' ) {
      tmp.clear();
      continue;
    } else if( *iter == '}' ) {
      generalization_vector.push_back( tmp );
      tmp.clear();
      continue;
    } else {
      tmp += *iter;
      continue;
    }
  }

  for ( vector< string >::iterator iter  = generalization_vector.begin(); iter != generalization_vector.end(); ++ iter ) {
    vector< string > tmp_generalization;
    SplitWithStr( *iter, " ||| ", tmp_generalization );
    if ( tmp_generalization.size() != 5 || tmp_generalization.at( 0 ) == "" || tmp_generalization.at( 1 ) == "" || \
         tmp_generalization.at( 2 ) == "" || tmp_generalization.at( 3 ) == "" || tmp_generalization.at( 4 ) == "" ) {
      continue;
    } else {
      int start_position = atoi( tmp_generalization.at( 0 ).c_str() );
      int end_position   = atoi( tmp_generalization.at( 1 ).c_str() );

      if ( end_position - start_position != 0 || start_position >= ( int )word_vector.size() || word_vector.at( start_position ) != tmp_generalization.at( 3 ) ) {
          continue;
      } else {
          generalizations.push_back( make_pair( tmp_generalization.at( 3 ), tmp_generalization.at( 2 ) ) );
      }
    }
  }
  return true;
}


/*
* $Name: ParseGeneralization
* $Function: Parse generalizations for translation memory.
* $Date: 2013-11-27
*/
bool TranslationMemory::ParseGeneralizationForClausesTM(string &informations, multimap< size_t, GeneralizationInformation > &generalization_informations) {

  vector< string > informations_vector;
  string line = "";
  for ( string::iterator iter = informations.begin(); iter != informations.end(); ++ iter ) {
    if( *iter == '{' ) {
      line.clear();
      continue;
    } else if ( *iter == '}' ) {
      informations_vector.push_back( line );
      line.clear();
      continue;
    } else {
      line += *iter;
      continue;
    }
  }

  for( vector< string >::iterator iter = informations_vector.begin(); iter != informations_vector.end(); ++iter ) {
    vector< string > domains_vector;
    SplitWithStr( *iter, " ||| ", domains_vector );
    if( domains_vector.size() < 5 || domains_vector.at( 0 ) == "" || domains_vector.at( 1 ) == "" || \
        domains_vector.at( 2 ) == "" || domains_vector.at( 3 ) == "" || domains_vector.at( 4 ) == "" ) {
      continue;
    }

    size_t start_position = ( size_t )( atoi( domains_vector.at( 0 ).c_str() ) );
    size_t end_position   = ( size_t )( atoi( domains_vector.at( 1 ).c_str() ) );

    if ( domains_vector.at( 1 ) < domains_vector.at( 0 ) ) {
      continue;
    }

    GeneralizationInformation generalization_information( start_position, end_position, domains_vector.at( 2 ), \
                                                          domains_vector.at( 3 ), domains_vector.at( 4 ) );
    generalization_informations.insert( make_pair( start_position, generalization_information ) );
  }
  return true;
}


/*
* $Name: SegmentSentence
* $Function: Segment sententce into clauses.
* $Date: 2013-11-25
*/
bool TranslationMemory::SegmentSentence(string &input_sentence, vector< SentenceSegmentInfo > &sentence_segment_infos, TranslationMemoryModel &translation_memory_model) {

  vector< string > domains_vector;
  SplitWithStr( input_sentence, " |||| ", domains_vector );
  multimap< size_t, GeneralizationInformation > generalization_informations_;
  if ( domains_vector.size() != 1 && domains_vector.at( 1 ).size() != 0 ) {
    ParseGeneralizationForClausesTM( domains_vector.at( 1 ), generalization_informations_ );
  }
  
  vector< string > input_words;
  BasicMethod basic_method;
  Split( domains_vector.at( 0 ), ' ', input_words );

  SentenceSegmentInfo sentence_segment_info;
  size_t current_position = 0;
  string clause;
  vector< GeneralizationInformation > generalization_informations_vector;
  for ( vector< string >::size_type pos = 0; pos < input_words.size(); ++pos ) {
    if ( translation_memory_model.ending_punctuation_dictionary_.find( input_words.at( pos ) ) != translation_memory_model.ending_punctuation_dictionary_.end() ) {
      clause += input_words.at( pos );
      if ( !generalization_informations_vector.empty() ) {
        clause += " |||| ";
        for( vector< GeneralizationInformation >::iterator iter = generalization_informations_vector.begin(); iter != generalization_informations_vector.end(); ++ iter ) {
          clause += "{" + size_tToString( iter->start_position_ ) + " ||| " + size_tToString( iter->end_position_ ) + \
                    " ||| " + iter->translation_result_ + " ||| " + iter->generalization_ + " ||| " + iter->raw_information_ + "}";
        }
      }

      sentence_segment_info.source_sentence_ = clause;
      sentence_segment_infos.push_back( sentence_segment_info );
      clause = "";
      generalization_informations_vector.clear();
      current_position = 0;
    } else {
      clause += input_words.at( pos ) + " ";
      multimap< size_t, GeneralizationInformation >::iterator begin_gener = generalization_informations_.lower_bound( pos );
      multimap< size_t, GeneralizationInformation >::iterator end_gener   = generalization_informations_.upper_bound( pos );
      while ( begin_gener != end_gener ) {
        size_t d_value = begin_gener->second.start_position_ - current_position;
        begin_gener->second.start_position_ = current_position;
        begin_gener->second.end_position_ -= d_value;
        generalization_informations_vector.push_back( begin_gener->second );
        ++ begin_gener;
      }

      ++ current_position;
    }
  }
  if ( clause != "" ) {
    basic_method.RmEndSpace( clause );

    if ( !generalization_informations_vector.empty() ) {
      clause += " |||| ";
      for( vector< GeneralizationInformation >::iterator iter = generalization_informations_vector.begin(); iter != generalization_informations_vector.end(); ++ iter ) {
           clause += "{" + size_tToString( iter->start_position_ ) + " ||| " + size_tToString( iter->end_position_ ) + \
                     " ||| " + iter->translation_result_ + " ||| " + iter->generalization_ + " ||| " + iter->raw_information_ + "}";
      }
    }
    sentence_segment_info.source_sentence_ = clause;
    sentence_segment_infos.push_back( sentence_segment_info );
  }

  return true;
}

/*
 * $Name: PrintTrainingTMModelLogo
 * $Function: Print training transaltion memory model logo.
 * $Date: 2013-05-09
 */
bool TranslationMemory::PrintTrainingTMModelLogo() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# Training Translation Memory Model                               #\n"
      <<"#                                             Version 0.0.1       #\n"
      <<"#                                             NEUNLPLab/YAYI corp #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<flush;
  return true;
}


/*
 * $Name: PrintTranslationMemoryLogo.
 * $Function: Print translation memory logo.
 * $Date: 2013-05-10
 */
bool TranslationMemory::PrintTranslationMemoryLogo() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# Translation Memory                                              #\n"
      <<"#                                             Version 0.0.1       #\n"
      <<"#                                             NEUNLPLab/YAYI corp #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<flush;
  return true;
}


}

