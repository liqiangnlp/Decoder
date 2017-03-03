/*
* $Id:
* 0004
*
* $File:
* interface.cpp
*
* $Proj:
* TranslationMemoryLib for Statistical Machine Translation
*
* $Func:
* interface
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
* 2014-01-12,20:45,
* 2014-01-10,13:14,
*/


#include "interface.h"
using namespace translation_memory_interface;


bool ParseParameterInConfig( string config, map< string, string > &param );
bool CheckFileInConf( map< string, string > &param, string &fileKey );
bool PrintTranslationMemoryLogo();
bool PrintTrainingTMModelLogo();


/*
* $Name: __Init
* $Funtion: Init for translation memory
* $Date: 2014-01-12
*/
#ifndef TRAINING_TRANSLATION_MEMORY_MODEL_
void*  __init   ( const char* config ) {
  PrintTranslationMemoryLogo();
  cerr<<"Parameters:\n"<<flush;
  cerr<<"  config_file_      :  "<<config<<"\n"<<flush;
  map< string, string > parameters_for_config;
  ParseParameterInConfig( config, parameters_for_config );

  TranslationMemoryInterface* interf = new TranslationMemoryInterface();
  if ( parameters_for_config.find( "System-Log" ) == parameters_for_config.end() ) {
    cerr<<"[Error] Please add parameter 'System-Log' in your config file.\n"<<flush;
    exit( 1 );
  }
  interf->system_log_file_name_ = parameters_for_config[ "System-Log" ];

  if ( parameters_for_config.find( "Log" ) == parameters_for_config.end() ) {
    cerr<<"[Error] Please add parameter 'Log' in your config file.\n"<<flush;
    exit( 1 );
  }
  interf->log_file_.open( parameters_for_config[ "Log" ].c_str(), ios::app );
  if ( !interf->log_file_ ) {
    cerr<<"ERROR: Please check the log path of \""<<parameters_for_config[ "Log" ].c_str()<<"\".\n"<<flush;
    exit( 1 );
  }

  cerr<<"Initialize Translation Memory Model...\n"<<flush;
  string key = "Translation-Memory-Table";
  CheckFileInConf( parameters_for_config, key );
  key = "Punct-Dict";
  CheckFileInConf( parameters_for_config, key );
  key = "Ending-Punct-Dict";
  CheckFileInConf( parameters_for_config, key );

  interf->tm_model_file_name_ = parameters_for_config[ "Translation-Memory-Table" ];
  interf->punct_dict_file_name_ = parameters_for_config[ "Punct-Dict" ];
  interf->ending_punct_dict_file_name_ = parameters_for_config[ "Ending-Punct-Dict" ];

  interf->translation_memory_model_.LoadTranslationMemoryModel( interf->tm_model_file_name_ );
  interf->translation_memory_model_.LoadPunctuationDictionary( interf->punct_dict_file_name_ );
  interf->translation_memory_model_.LoadEndingPunctuationDictionary( interf->ending_punct_dict_file_name_ );

  return ( void* ) interf;
}
#else
void*  __init   ( const char* config ) {
  PrintTrainingTMModelLogo();
  cerr<<"Parameters:\n"<<flush;
  cerr<<"  config_file_      :  "<<config<<"\n"<<flush;
  map< string, string > parameters_for_config;
  ParseParameterInConfig( config, parameters_for_config );
  TranslationMemoryInterface* interf = new TranslationMemoryInterface();
  cerr<<"Initialize Translation Memory Model...\n"<<flush;
  string key = "Src-TM-File";
  CheckFileInConf( parameters_for_config, key );
  key = "Tgt-TM-File";
  CheckFileInConf( parameters_for_config, key );
  key = "Punct-Dict";
  CheckFileInConf( parameters_for_config, key );

  interf->tm_model_file_name_ = parameters_for_config[ "Translation-Memory-Table" ];
  interf->punct_dict_file_name_ = parameters_for_config[ "Punct-Dict" ];
  interf->src_tm_file_ = parameters_for_config[ "Src-TM-File" ];
  interf->tgt_tm_file_ = parameters_for_config[ "Tgt-TM-File" ];

  return ( void* ) interf;
}
#endif


/*
* $Name: __reload
* $Funtion: reload model for translation memory
* $Date: 2014-01-12
*/
#ifndef TRAINING_TRANSLATION_MEMORY_MODEL_
void __reload ( void* class_handle ) {
  cerr<<"Reload Translation Memory...\n"<<flush;
  TranslationMemoryInterface* interf = ( TranslationMemoryInterface* ) class_handle;

  cerr<<"Clear translation memory model...\n"<<flush;
  interf->translation_memory_model_.translation_memory_model_.clear();
  cerr<<"Clear punctuation dictionary...\n"<<flush;
  interf->translation_memory_model_.punctuation_dictionary_.clear();
  cerr<<"Clear ending punctuation dictionary...\n"<<flush;
  interf->translation_memory_model_.ending_punctuation_dictionary_.clear();

  interf->translation_memory_model_.LoadTranslationMemoryModel( interf->tm_model_file_name_ );
  interf->translation_memory_model_.LoadPunctuationDictionary( interf->punct_dict_file_name_ );
  interf->translation_memory_model_.LoadEndingPunctuationDictionary( interf->ending_punct_dict_file_name_ );
  return;
}
#else
void __reload ( void* class_handle ) {
  cerr<<"__reload of Training Translation Memory does not boot...\n"<<flush;
  return;
}
#endif


/*
* $Name: __do_job
* $Funtion: do job for translation memory
* $Date: 2014-01-12
*/
#ifndef TRAINING_TRANSLATION_MEMORY_MODEL_
char* __do_job( void* class_handle, const char* msg_text, int print_log, const char* log_head ) {
#ifndef WIN32
  timeval start_time, end_time;
  gettimeofday( &start_time, NULL );

  clock_t start_time_clock = clock();
  clock_t end_time_clock = 0;
#else
  clock_t start_time = clock();
  clock_t end_time = 0;
#endif

  cerr<<"Translation Memory...";

  TranslationMemoryInterface* interf = ( TranslationMemoryInterface* ) class_handle;
  string sentence( msg_text );

  if ( print_log == 1 ) {
    ofstream translation_memory_log( interf->system_log_file_name_.c_str(), ios::app );
    if ( !translation_memory_log ) {
      cerr<<"[Error] Can not open '"<<interf->system_log_file_name_<<"'\n"<<flush;
    } else {
#ifndef WIN32
      translation_memory_log<<"[ "<< start_time_clock<<" ] [INFO] [ function: TM-INPUT ] "<<log_head<<" [msg_buff: "<<sentence<<" ]\n"<<flush;
#else
      translation_memory_log<<"[ "<< start_time<<" ] [INFO] [ function: TM-INPUT ] "<<log_head<<" [msg_buff: "<<sentence<<" ]\n"<<flush;
#endif
    }
    translation_memory_log.close();
  }


  string output;
  TranslationMemory translation_memory;
  translation_memory.InterfaceForDecoder( sentence, output, interf->translation_memory_model_ );
  string final_translation_result( output );

#ifdef WIN32
  char* msg_res = new char[ final_translation_result.size() + 1 ];
  strcpy_s( msg_res, final_translation_result.size() + 1, final_translation_result.c_str() );
#else
  char* msg_res = new char[ final_translation_result.size() + 1 ];
  strncpy( msg_res, final_translation_result.c_str(), final_translation_result.size() + 1 );
#endif

#ifndef WIN32
  gettimeofday( &end_time, NULL );
  double time = ( (double)( end_time.tv_sec - start_time.tv_sec ) * 1000000 + (double)(end_time.tv_usec - start_time.tv_usec) ) / 1000000;

  end_time_clock = clock();
  double time_clock = ( double )( end_time_clock - start_time_clock )/CLOCKS_PER_SEC;
#else
  end_time = clock();
  double time = ( double )( end_time - start_time )/CLOCKS_PER_SEC;
#endif

  cerr<<" Done! [time="<<time<<"s speed="<<1.000/time<<"sent/s]   \n";
  cerr<<"INPUT  : "<<sentence <<"\n"
      <<"OUTPUT : "<<final_translation_result<<"\n\n";
  cout<<final_translation_result<<"\n";

  interf->log_file_<<"[INPUT ]  "<<sentence<<"\n"
                   <<"[OUTPUT]  "<<final_translation_result<<"\n"
                   <<"[TIME  ]  "<<time<<" s\n"
#ifndef WIN32
                   <<"[TIME_C]  "<<time_clock<<" s\n"  
#endif
                   <<"[SPEED ]  "<<double( 1.000/time )<<" sent/s\n\n"
                   <<flush;

  if ( print_log == 1 ) {
    ofstream translation_memory_log( interf->system_log_file_name_.c_str(), ios::app );
    if ( !translation_memory_log ) {
      cerr<<"[Error] Can not open '"<<interf->system_log_file_name_<<"'\n"<<flush;
    } else {
#ifndef WIN32
      translation_memory_log<<"[ "<< end_time_clock<<" ] [INFO] [ function: TM-OUTPUT ] "<<log_head<<" [msg_buff: "<<msg_res<<" ]\n"<<flush;
#else
      translation_memory_log<<"[ "<< end_time<<" ] [INFO] [ function: TM-OUTPUT ] "<<log_head<<" [msg_buff: "<<msg_res<<" ]\n"<<flush;
#endif
    }
    translation_memory_log.close();
  }

  return msg_res;
}
#else
char* __do_job( void* class_handle, const char* msg_text, int print_log, const char* log_head ) {
  TranslationMemoryInterface* interf = ( TranslationMemoryInterface* ) class_handle;
  TranslationMemory translation_memory;

  interf->translation_memory_model_.LoadPunctuationDictionary( interf->punct_dict_file_name_ );

//  ofstream out_file( parameters[ "-out" ].c_str() );
  ofstream out_file( interf->tm_model_file_name_.c_str() );
  if ( !out_file ) {
    cerr<<"ERROR: Please check the path of \""<<interf->tm_model_file_name_<<"\".\n"<<flush;
    exit( 1 );
  }

  ifstream in_src_file( interf->src_tm_file_.c_str() );
  if ( !in_src_file ) {
    cerr<<"ERROR: Please check the path of \""<<interf->src_tm_file_<<"\".\n"<<flush;
    exit( 1 );
  }
  ifstream in_tgt_file( interf->tgt_tm_file_.c_str() );
  if ( !in_tgt_file ) {
    cerr<<"ERROR: Please check the path of \""<<interf->tgt_tm_file_<<"\".\n"<<flush;
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
    translation_memory.clearIllegalChar( input_source_sentence );
    translation_memory.rmEndSpace( input_source_sentence );
    translation_memory.rmStartSpace( input_source_sentence );
    getline( in_tgt_file, input_target_sentence );
    translation_memory.clearIllegalChar( input_target_sentence );
    translation_memory.rmEndSpace( input_target_sentence );
    translation_memory.rmStartSpace( input_target_sentence );
    if ( input_source_sentence == "" || input_target_sentence == "" ) {
        ++error_count;
        continue;
    } else if ( translation_memory.CheckGeneralization( input_source_sentence, input_target_sentence ) ) {
      translation_memory.RemoveBeginningPunctuation( input_source_sentence, source_punctuation, interf->translation_memory_model_ );
      translation_memory.RemoveBeginningPunctuation( input_target_sentence, target_punctuation, interf->translation_memory_model_ );
      translation_memory.RemoveEndingPunctuation( input_source_sentence, source_punctuation, interf->translation_memory_model_ );
      translation_memory.RemoveEndingPunctuation( input_target_sentence, target_punctuation, interf->translation_memory_model_ );
      if( input_source_sentence == "" || input_target_sentence == "" ) {
        ++error_count;
        continue;
      } else {
        ++correct_count;
        interf->translation_memory_model_.translation_memory_model_.insert( make_pair( input_source_sentence, input_target_sentence ) );
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
  for ( map< string, string >::iterator iter = interf->translation_memory_model_.translation_memory_model_.begin(); \
        iter != interf->translation_memory_model_.translation_memory_model_.end(); ++iter ) {
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

  string final_translation_result( "return value" );
#ifdef WIN32
  char* msg_res = new char[ final_translation_result.size() + 1 ];
  strcpy_s( msg_res, final_translation_result.size() + 1, final_translation_result.c_str() );
#else
  char* msg_res = new char[ final_translation_result.size() + 1 ];
  strncpy( msg_res, final_translation_result.c_str(), final_translation_result.size() + 1 );
#endif
  return msg_res;
}
#endif


/*
* $Name: __destroy
* $Funtion: destroy model for translation memory
* $Date: 2014-01-12
*/
#ifndef TRAINING_TRANSLATION_MEMORY_MODEL_
void   __destroy( void* class_handle ) {
  TranslationMemoryInterface* interf = ( TranslationMemoryInterface* ) class_handle;
  interf->log_file_.clear();
  interf->log_file_.close();
  delete interf;
}
#else
void   __destroy( void* class_handle ) {
  TranslationMemoryInterface* interf = ( TranslationMemoryInterface* ) class_handle;
  delete interf;
}
#endif


/*
 * $Name: ParseParameterInConfig
 * $Funtion:
 * $Date: 2013-05-13
 */
bool ParseParameterInConfig( string config, map< string, string > &param ) {
  ifstream inputConfigFile( config.c_str() );
  if ( !inputConfigFile ) {
    cerr<<"ERROR: Config File does not exist, exit!\n"<<flush;
    exit( 1 );
  }

  string lineOfConfigFile;
  while ( getline( inputConfigFile, lineOfConfigFile ) ) {
    BasicMethod bm;
    bm.ClearIllegalChar( lineOfConfigFile );
    bm.RmStartSpace    ( lineOfConfigFile );
    bm.RmEndSpace      ( lineOfConfigFile );

    if( lineOfConfigFile == "" || *lineOfConfigFile.begin() == '#' ) {
      continue;
    } else if ( lineOfConfigFile.find( "param=\"" ) == lineOfConfigFile.npos 
                || lineOfConfigFile.find( "value=\"" ) == lineOfConfigFile.npos ) {
      continue;
    } else {
      string::size_type pos = lineOfConfigFile.find( "param=\"" );

      pos += 7;
      string key;
      for ( ; lineOfConfigFile[ pos ] != '\"' && pos < lineOfConfigFile.length(); ++pos ) {
        key += lineOfConfigFile[ pos ];
      }
      if ( lineOfConfigFile[ pos ] != '\"' ) {
        continue;
      }

      pos = lineOfConfigFile.find( "value=\"" );
      pos += 7;
      string value;

      for ( ; lineOfConfigFile[ pos ] != '\"' && pos < lineOfConfigFile.length(); ++pos ) {
        value += lineOfConfigFile[ pos ];
      }

      if ( lineOfConfigFile[ pos ] != '\"' ) {
        continue;
      }

      if ( param.find( key ) == param.end() ) {
        param.insert( make_pair( key, value ) );
      } else {
        param[ key ] = value;
      }
    }
  }
  return true;
}


/*
 * $Name: CheckFileInConf
 * $Function:
 * $Date: 2013-05-13
 */
bool CheckFileInConf( map< string, string > &param, string &fileKey ) {
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
 * $Funtion:
 * $Date:
 */
bool PrintTranslationMemoryLogo() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# Translation Memory                                              #\n"
      <<"#                                            Version 0.0.1        #\n"
      <<"#                                            NEUNLPLab/YAYI corp  #\n"
      <<"#                                            liqiangneu@gmail.com #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<flush;
  return true;
}

/*
 * $Name:
 * $Funtion:
 * $Date:
 */
bool PrintTrainingTMModelLogo() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# Training Translation Memory                                     #\n"
      <<"#                                            Version 0.0.1        #\n"
      <<"#                                            NEUNLPLab/YAYI corp  #\n"
      <<"#                                            liqiangneu@gmail.com #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<flush;
  return true;
}



