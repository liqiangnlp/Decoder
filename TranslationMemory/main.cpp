/*
* $Id:
* 0002
*
* $File:
* main.cpp
*
* $Proj:
* TranslationMemoryLib for Statistical Machine Translation
*
* $Func:
* main function
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
* 2014-01-10,13:14
*/

#include "main.h"

#ifndef TRAINING_TRANSLATION_MEMORY_MODEL_
int main( int argc, char * argv[] ) {
  if( argc < 3 ) {
    cerr<<"[USAGE] EXE CONFIG TEST\n"<<flush;
    exit( 1 );
  }

  string config( argv[ 1 ] );
  void* handle = ( void* )__init( config.c_str() );

  cerr<<argv[ 2 ]<<"\n"<<flush;
  ifstream infile( argv[ 2 ] );
  if ( !infile ) {
    cerr<<"Can not open file "<<argv[ 1 ]<<"\n"<<flush;
    exit( 1 );
  }

  string sentence;
  size_t lineNo = 0;
  while ( getline( infile, sentence ) ) {
    ++lineNo;
    char * output = __do_job( handle, sentence.c_str(), 1, sentence.c_str() );
    delete []output;
  }
  infile.clear();
  infile.close();


  __reload( handle );

  ifstream infile2( argv[ 2 ] );
  if ( !infile2 ) {
      cerr<<"Can not open file "<<argv[ 1 ]<<"\n"<<flush;
      exit( 1 );
  }

  lineNo = 0;
  while ( getline( infile2, sentence ) ) {
      ++lineNo;
      char * output = __do_job( handle, sentence.c_str(), 1, sentence.c_str() );
      delete []output;
  }
  infile.clear();
  infile.close();


    //    }

  __destroy( handle );
  return 0;
}
#else
int main( int argc, char * argv[] ) {
  if( argc < 3 ) {
    cerr<<"[USAGE] EXE CONFIG TEST\n"<<flush;
    exit( 1 );
  }

  string config( argv[ 1 ] );
  void* handle = ( void* )__init( config.c_str() );

  cerr<<argv[ 2 ]<<"\n"<<flush;
  ifstream infile( argv[ 2 ] );
  if ( !infile ) {
    cerr<<"Can not open file "<<argv[ 2 ]<<"\n"<<flush;
    exit( 1 );
  }

  string sentence( "input null" );
  char * output = __do_job( handle, sentence.c_str(), 1, sentence.c_str() );
  delete []output;

  __reload( handle );

  __destroy( handle );
  infile.clear();
  infile.close();


  handle = ( void* )__init( config.c_str() );

  cerr<<argv[ 2 ]<<"\n"<<flush;
  infile.open( argv[ 2 ] );
  if ( !infile ) {
      cerr<<"Can not open file "<<argv[ 2 ]<<"\n"<<flush;
      exit( 1 );
  }

  sentence = "input null";
  output = __do_job( handle, sentence.c_str(), 1, sentence.c_str() );
  delete []output;

  __reload( handle );

  __destroy( handle );


  return 0;
}
#endif


