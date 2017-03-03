/*
* $Id:
* 0002
*
* $File:
* main.cpp
*
* $Proj:
* Decoder for Statistical Machine Translation
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
* 2012-12-04,09:45
*/

#include "main.h"

#ifdef SUPPORT_OFFLINE_SERVICE_

int main (int argc, char * argv[]) {
  GetParameter getParameter;
  if (getParameter.GetAllFunctions (argc)) {
    exit (1);
  } else if (getParameter.GetAllFunctions (argc, string( argv[ 1 ]))) {
    exit (1);
  } else if (getParameter.GetParameters (argc, string( argv[ 1 ]))) {
    exit (1);
  }

  Dispatcher dispatcher;
  dispatcher.ResolveParameters (argc, argv);
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

    /*
    int i = 10;
    vector< InterfaceMultiThread > vecIfmt;

    for( int i = 0; i < 4; ++i )
    {
        InterfaceMultiThread ifmt;
        ifmt.threadId = i;
        ifmt.nthread  = 4;
        vecIfmt.push_back( ifmt );
    }
    for( int i = 0; i < 4; ++i )
    {
        vecIfmt.at( i ).Start();
    }
    for( int i = 0; i < 4; ++i )
    {
        vecIfmt.at( i ).Join();
    }
    cerr<<"\n  [ThreadsNumber="<<4<<"]\n"
        <<"  Done!\n"
        <<flush;
    */

  cerr<<"Input File: "<<argv[ 2 ]<<"\n\n"<<flush;
  ifstream infile( argv[ 2 ] );
  if ( !infile ) {
    cerr<<"Can not open file "<<argv[ 1 ]<<"\n"<<flush;
    exit( 1 );
  }

  string sentence;
  size_t lineNo = 0;
  while ( getline( infile, sentence ) ) {
    ++lineNo;
#ifndef WITHOUT_TRANSLATION_MEMORY_
    char * output = __do_job( handle, sentence.c_str() );
#else
    char * output = __do_job( handle, sentence.c_str(), 1, sentence.c_str() );
#endif

    delete []output;
  }
    //    }

  infile.close();
  __destroy( handle );
  return 0;
}

#endif
