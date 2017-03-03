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

#ifndef _MULTI_THREAD_H_
#define _MULTI_THREAD_H_

#ifndef WIN32
#define HAVE_PTHREAD_H // for Linux
#endif

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#else
#ifdef _WIN32
#include <windows.h>
#include <process.h>
#endif
#endif

#if defined HAVE_PTHREAD_H
#define _USE_NTHREAD 1
#endif

#if( defined( _WIN32 ) && !defined( __CYGWIN__ ) )
#define _USE_NTHREAD 1
#define BEGINTHREAD(src, stack, func, arg, flag, id) \
    (HANDLE)_beginthreadex((void *)(src), (unsigned)(stack), \
    (unsigned(_stdcall *)(void *))(func), (void *)(arg), \
    (unsigned)(flag),(unsigned *)(id))
#endif

namespace multi_thread {

class MultiThread {
 private:
#ifdef HAVE_PTHREAD_H
  pthread_t hnd_;
#else
#ifdef _WIN32
  HANDLE hnd_;
#endif
#endif

 public:
  int id      ;
  int threadId;
  int nthread ;

 public:
  MultiThread(){}
  virtual ~MultiThread(){}

 public:
  static void* Wrapper( void *ptr ) {
    MultiThread *p = ( MultiThread * )ptr;
    p->Run();
    return 0;
  }

 public:
  void Start();
  void Join ();
  virtual void Run(){}
};

}

#endif


