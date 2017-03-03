/*
* $Id:
* 0004
*
* $File:
* interface.cpp
*
* $Proj:
* RecaserLib for Statistical Machine Translation
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
using namespace recaser_interface;


bool ParseParameterInConfig( string config, map< string, string > &param );
bool CheckFileInConf( map< string, string > &param, string &fileKey );
bool PrintRecasingLogo();


/*
* $Name: __Init
* $Funtion: Init for Recaser
* $Date: 2014-11-10
*/
void*  __init(const char* config) {
  PrintRecasingLogo();
  cerr<<"Parameters:\n"<<flush;
  cerr<<"  config_file_      :  "<<config<<"\n"<<flush;
  map< string, string > parameters_for_config;
  ParseParameterInConfig( config, parameters_for_config );

  RecasingInterface* interf = new RecasingInterface();

  string key = "recaser-config-file";
  CheckFileInConf(parameters_for_config, key);

  map< string, string > parameters_recaser;
  cerr<<"  config_recaser_   :  "<<parameters_for_config[ "recaser-config-file" ]<<"\n"<<flush;
  ParseParameterInConfig( parameters_for_config[ "recaser-config-file" ], parameters_recaser );

  if( parameters_for_config.find( "system-log" ) == parameters_for_config.end() ) {
    cerr<<"[Error] Please add parameter 'system-log' in your config file.\n"<<flush;
    exit( 1 );
  }
  interf->system_log_file_name_ = parameters_for_config[ "system-log" ];
  interf->system_log_.open(interf->system_log_file_name_.c_str(), ios::app);
  if (!interf->system_log_) {
    cerr<<"ERROR: Please check the log path of \""<<interf->system_log_file_name_<<"\".\n"<<flush;
    exit( 1 );
  }

  interf->config_recasing_.recase_flag_     = true;
  interf->config_recasing_.support_service_flag_ = true;
  interf->config_recasing_.Init( parameters_recaser );
  interf->config_recasing_.nbest_ = 1;
  cerr<<"Initialize Recasing Model...\n"<<flush;
  interf->model_recasing_.Init( interf->config_recasing_ );

  return ( void* ) interf;
}


/*
* $Name: __reload
* $Funtion: reload model for Recaser
* $Date: 2014-11-10
*/
void __reload ( void* class_handle ) {
  cerr<<"Reload recasing not boot...\n"<<flush;
  return;
}


/*
* $Name: __do_job
* $Funtion: do job for Recaser
* $Date: 2014-11-10
*/
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

  cerr<<"Recasing...";
  RecasingInterface* interf = (RecasingInterface*) class_handle;
  string sentence(msg_text);
  string final_translation_result;

  SourceSentence source_sentence_recasing;
  source_sentence_recasing.Init(sentence);
  Recasing recasing;
  recasing.Decode(interf->config_recasing_, interf->model_recasing_, source_sentence_recasing, final_translation_result);

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
      <<"[INPUT  ]  "<<sentence<<"\n";
  cerr<<"[RECASER]  "<<final_translation_result<<"\n";
  cerr<<"[time="<<time<<"s speed="<<1.000/time<<"sent/s]   \n\n";

  interf->system_log_<<"[INPUT  ]  "<<sentence<<"\n"
                     <<"[RECASER]  "<<final_translation_result<<"\n";
  interf->system_log_<<"[time="<<time<<"s speed="<<1.000/time<<"sent/s]   \n\n";

  return msg_res;
}


/*
* $Name: __destroy
* $Funtion: destroy model for Recaser
* $Date: 2014-01-12
*/
void   __destroy( void* class_handle ) {
  RecasingInterface* interf = (RecasingInterface*) class_handle;
  interf->system_log_.clear();
  interf->system_log_.close();
  delete interf;
}


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
bool PrintRecasingLogo() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# Recasing                                                        #\n"
      <<"#                                            Version 0.0.1        #\n"
      <<"#                                            NEUNLPLab/YAYI corp  #\n"
      <<"#                                            liqiangneu@gmail.com #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<flush;
  return true;
}



