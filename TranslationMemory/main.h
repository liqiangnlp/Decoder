/*
* $Id:
* 0001
*
* $File:
* main.h
*
* $Proj:
* TranslationMemoryLib for Statistical Machine Translation
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
* 2014-01-10,13:14
*/

#ifndef TRANSLATIONMEMORYLIB_MAIN_H_
#define TRANSLATIONMEMORYLIB_MAIN_H_

#include <iostream>
#include "interface.h"
#ifdef _WIN32
#include "../Decoder/basic_method.h"
#include "../Decoder/translation_memory.h"
#else
#include "basic_method.h"
#include "translation_memory.h"
#endif

using namespace std;

namespace main_func
{
}


#endif
