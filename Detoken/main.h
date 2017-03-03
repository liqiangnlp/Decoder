/*
* $Id:
* 0001
*
* $File:
* main.h
*
* $Proj:
* DetokenLib for Statistical Machine Translation
*
* $Func:
* header file of main function
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
* 2014-10-10,12:34
*/

#ifndef DETOKENLIB_MAIN_H_
#define DETOKENLIB_MAIN_H_

#include <iostream>
#include "interface.h"
#ifdef _WIN32
#include "../Decoder/detokenizer.h"
#else
#include "detokenizer.h"
#endif

using namespace std;

namespace main_func
{
}


#endif
