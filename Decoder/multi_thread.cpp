/*
* $Id:
* 0035
*
* $File:
* multi_thread.h
*
* $Proj:
* multi_thread interface for Statistical Machine Translation
*
* $Func:
* multi_thread
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
* 2013-04-07,09:33
*/

#include "multi_thread.h"


namespace multi_thread {

void MultiThread::Start() {
#ifdef HAVE_PTHREAD_H
  pthread_create( &hnd_, 0, &MultiThread::Wrapper, static_cast<void *>(this) );
#else
#ifdef _WIN32
  DWORD id;
  hnd_ = BEGINTHREAD(0, 0, &MultiThread::Wrapper, this, 0, &id );
#else
  Run();
#endif
#endif
}


void MultiThread::Join() {
#ifdef HAVE_PTHREAD_H
  pthread_join( hnd_, 0 );
#else
#ifdef _WIN32
  WaitForSingleObject( hnd_, INFINITE );
  CloseHandle( hnd_ );
#endif
#endif
}

}




