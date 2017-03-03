/*
* $Id:
* 0002
*
* $File:
* main.cpp
*
* $Proj:
* RecaserLib for Statistical Machine Translation
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



