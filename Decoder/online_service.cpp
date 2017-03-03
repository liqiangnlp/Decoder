/*
* $Id:
* 0038
*
* $File:
* interface.cpp
*
* $Proj:
* interface for Statistical Machine Translation
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
* 2014-01-09,13:21, English string recognition and translation for Chinese-2-English translation.
* 2013-04-10,11:03
*/


#include "online_service.h"
using namespace decoder_online_service;

#ifndef SUPPORT_OFFLINE_SERVICE_

bool ParseParameterInConfig( string config, map< string, string > &param   );
bool CheckFileInConf( map< string, string > &param, string &fileKey );
bool PrintSMTLogo();


/*
 * $Name: __init
 * $Function:
 * $Date: 2013-05-13
 */
void* __init( const char* config ) {
  PrintSMTLogo();
  cerr<<"Parameters:\n"<<flush;
  map< string, string > parameters_for_config;
  cerr<<"  config_file_      :  "<<config<<"\n"<<flush;
  ParseParameterInConfig( config, parameters_for_config );

  string key( "decoder-config-file" );
  CheckFileInConf( parameters_for_config, key );

#ifdef SUPPORT_ONLINE_SERVICE_CE_
    key = "recaser-config-file";
    CheckFileInConf( parameters_for_config, key );
#endif

#ifndef WITHOUT_TRANSLATION_MEMORY_
  key = "tm-config-file";
  CheckFileInConf( parameters_for_config, key );

  map< string, string > parameters_tm;
  cerr<<"  config_tm_        :  "<<parameters_for_config[ "tm-config-file" ]<<"\n"<<flush;
  ParseParameterInConfig( parameters_for_config[ "tm-config-file" ], parameters_tm );
#endif

  map< string, string > parameters_smt;
  cerr<<"  config_smt_       :  "<<parameters_for_config[ "decoder-config-file" ]<<"\n"<<flush;
  ParseParameterInConfig( parameters_for_config[ "decoder-config-file" ], parameters_smt );
  
#ifdef SUPPORT_ONLINE_SERVICE_CE_
  map< string, string > parameters_recaser;
  cerr<<"  config_recaser_   :  "<<parameters_for_config[ "recaser-config-file" ]<<"\n"<<flush;
  ParseParameterInConfig( parameters_for_config[ "recaser-config-file" ], parameters_recaser );
#elif defined SUPPORT_ONLINE_SERVICE_EC_
  key = "Punct-Mapping-Dict";
  CheckFileInConf( parameters_for_config, key );
#endif

  OnlineService* interf = new OnlineService();

  if( parameters_for_config.find( "system-log" ) == parameters_for_config.end() ) {
    cerr<<"[Error] Please add parameter 'system-log' in your config file.\n"<<flush;
    exit( 1 );
  }
  interf->system_log_file_name_ = parameters_for_config[ "system-log" ];

  if( parameters_for_config.find( "log" ) == parameters_for_config.end() ) {
      cerr<<"[Error] Please add parameter 'log' in your config file.\n"<<flush;
      exit( 1 );
  }
  cerr<<"  log_path_         :  "<<parameters_for_config[ "log" ]<<"\n\n"<<flush;
  interf->log_file_.open( parameters_for_config[ "log" ].c_str(), ios::app );
  if ( !interf->log_file_ ) {
    cerr<<"ERROR: Please check the log path of \""<<parameters_for_config[ "log" ]<<"\".\n"<<flush;
    exit( 1 );
  }

#ifndef WITHOUT_TRANSLATION_MEMORY_
  cerr<<"Initialize Translation Memory Model...\n"<<flush;
  interf->translation_memory_model_.LoadTranslationMemoryModel( parameters_tm[ "Translation-Memory-Table" ] );
  interf->translation_memory_model_.LoadPunctuationDictionary( parameters_tm[ "Punct-Dict" ] );
  interf->translation_memory_model_.LoadEndingPunctuationDictionary( parameters_tm[ "Ending-Punct-Dict" ] );
#endif

  interf->config_smt_.phrase_table_binary_flag_ = true;
  interf->config_smt_.support_service_flag_ = true;
  interf->config_smt_.Init( parameters_smt );
  interf->config_smt_.nbest_ = 1;
  cerr<<"\nInitialize Translation Model...\n"<<flush;
  interf->model_smt_.Init( interf->config_smt_ );

#ifdef SUPPORT_ONLINE_SERVICE_CE_
  interf->config_recasing_.recase_flag_     = true;
  interf->config_recasing_.support_service_flag_ = true;
  interf->config_recasing_.Init( parameters_recaser );
  interf->config_recasing_.nbest_ = 1;
  cerr<<"Initialize Recasing Model...\n"<<flush;
  interf->model_recasing_.Init( interf->config_recasing_ );
#elif defined SUPPORT_ONLINE_SERVICE_EC_
  interf->punctuation_map_.LoadPunctuation( parameters_for_config[ "Punct-Mapping-Dict" ] );
#endif

  return ( void* ) interf;
}


/*
 * $Name: __destroy
 * $Function:
 * $Date: 2013-05-13
 */
void __destroy( void* class_handle ) {
  OnlineService* interf = ( OnlineService* ) class_handle;
  interf->log_file_.clear();
  interf->log_file_.close();
  delete interf;
}


/*
* $Name: __reload
* $Function:
* $Date: 2014-01-09
*/
void __reload( void* class_handle ) {
  return;
}

#ifndef WITHOUT_TRANSLATION_MEMORY_
/*
 * $Name: __do_job
 * $Function:
 * $Date: 2013-05-13
 */

char* __do_job( void* class_handle, const char* msg_text ) {
#ifndef WIN32
  timeval start_time, end_time;
  gettimeofday( &start_time, NULL );

  clock_t start_time_clock = clock();
  clock_t end_time_clock = 0;
#else
  clock_t start_time = clock();
  clock_t end_time = 0;
#endif

  cerr<<"Start Translation...";

  OnlineService* interf = ( OnlineService* ) class_handle;
  string sentence( msg_text );

  string translation_memory_output;
  TranslationMemory translation_memory;
  translation_memory.InterfaceForDecoder( sentence, translation_memory_output, interf->translation_memory_model_ );

  string decoder_output;
  PhraseBasedITGDecoder phrase_based_itg_decoder;
  phrase_based_itg_decoder.InterfaceForTranslationMemory( interf->config_smt_, interf->model_smt_, translation_memory_output, decoder_output );

  string final_translation_result;
#ifdef SUPPORT_ONLINE_SERVICE_CE_
  SourceSentence sourceSentenceRec;
  sourceSentenceRec.Init( decoder_output );

  string recasing_output;
  Recasing recasing;
  recasing.Decode( interf->config_recasing_, interf->model_recasing_, sourceSentenceRec, recasing_output );

  Detokenizer detokenizer;
  detokenizer.DetokenizerEn( recasing_output, final_translation_result );
#elif defined SUPPORT_ONLINE_SERVICE_EC_
  Detokenizer detokenizer_handle;
  detokenizer_handle.DetokenizerZh( interf->punctuation_map_, decoder_output, final_translation_result );
#endif

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

  cerr<<"Done!\n"
      <<"[INPUT  ]  "<<sentence<<"\n"
      <<"[TM     ]  "<<translation_memory_output<<"\n"
      <<"[DECODER]  "<<decoder_output<<"\n";
  cerr<<"[RECDETO]  "<<final_translation_result<<"\n";
  cerr<<"[time="<<time<<"s speed="<<1.000/time<<"sent/s]   \n\n";

  interf->log_file_<<"[INPUT  ]  "<<sentence<<"\n"
                   <<"[TM     ]  "<<translation_memory_output<<"\n"
                   <<"[DECODER]  "<<decoder_output<<"\n";
  interf->log_file_<<"[RECDETO]  "<<final_translation_result<<"\n";
  interf->log_file_<<"[time="<<time<<"s speed="<<1.000/time<<"sent/s]   \n\n";

  return msg_res;
}

#else

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

  cerr<<"Start Translation...";
  OnlineService* interf = ( OnlineService* ) class_handle;
  string sentence( msg_text );


  if ( print_log == 1 ) {
    ofstream translation_memory_log( interf->system_log_file_name_.c_str(), ios::app );
    if ( !translation_memory_log ) {
      cerr<<"[Error] Can not open '"<<interf->system_log_file_name_<<"'\n"<<flush;
    } else {
#ifndef WIN32
      translation_memory_log<<"[ "<< start_time_clock<<" ] [INFO] [ function: DECODER-INPUT ] "<<log_head<<" [msg_buff: "<<sentence<<" ]\n"<<flush;
#else
      translation_memory_log<<"[ "<< start_time<<" ] [INFO] [ function: DECODER-INPUT ] "<<log_head<<" [msg_buff: "<<sentence<<" ]\n"<<flush;
#endif
    }
    translation_memory_log.close();
  }

  string decoder_output;
  PhraseBasedITGDecoder phrase_based_itg_decoder;
  phrase_based_itg_decoder.InterfaceForTranslationMemory( interf->config_smt_, interf->model_smt_, sentence, decoder_output );

  string final_translation_result;
#ifdef SUPPORT_ONLINE_SERVICE_CE_
  SourceSentence sourceSentenceRec;
  sourceSentenceRec.Init( decoder_output );

  string recasing_output;
  Recasing recasing;
  recasing.Decode( interf->config_recasing_, interf->model_recasing_, sourceSentenceRec, recasing_output );

  Detokenizer detokenizer;
  detokenizer.DetokenizerEn( recasing_output, final_translation_result );
#elif defined SUPPORT_ONLINE_SERVICE_EC_
  Detokenizer detokenizer_handle;
  detokenizer_handle.DetokenizerZh( interf->punctuation_map_, decoder_output, final_translation_result );
#elif defined SUPPORT_ONLINE_SERVICE_ANYLANG2ANYLANG_
  final_translation_result = decoder_output;
#endif

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

  cerr<<"Done!\n"
      <<"[INPUT  ]  "<<sentence<<"\n"
      <<"[DECODER]  "<<decoder_output<<"\n";
  cerr<<"[RECDETO]  "<<final_translation_result<<"\n";
  cerr<<"[time="<<time<<"s speed="<<1.000/time<<"sent/s]   \n\n";

  if ( print_log == 1 ) {
    ofstream translation_memory_log( interf->system_log_file_name_.c_str(), ios::app );
    if ( !translation_memory_log ) {
      cerr<<"[Error] Can not open '"<<interf->system_log_file_name_<<"'\n"<<flush;
    } else {
#ifndef WIN32
      translation_memory_log<<"[ "<< end_time_clock<<" ] [INFO] [ function: DECODER-OUTPUT ] "<<log_head<<" [msg_buff: "<<msg_res<<" ]\n"<<flush;
#else
      translation_memory_log<<"[ "<< end_time<<" ] [INFO] [ function: DECODER-OUTPUT ] "<<log_head<<" [msg_buff: "<<msg_res<<" ]\n"<<flush;
#endif
    }
    translation_memory_log.close();
  }


  return msg_res;
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
bool PrintSMTLogo() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# Phrase-based Statistical Machine Translation                    #\n"
      <<"#                                             Version 0.0.1       #\n"
      <<"#                                             NEUNLPLab/YAYI corp #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<flush;
  return true;
}

#endif



