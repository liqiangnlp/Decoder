/*
 * $Id:
 * 0003
 *
 * $File:
 * parameter_controller.h
 *
 * $Proj:
 * Decoder for Statistical Machine Translation
 *
 * $Func:
 * control parameters
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
 * 2012-12-04,09:50
 */

#ifndef DECODER_PARAMETER_CONTROLLER_H_
#define DECODER_PARAMETER_CONTROLLER_H_

#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;

namespace parameter_controller {
#define __DEBUG_PARAMETER_CONTROLLER__

class GetParameter {
 public:
  void getParamsOfPhraseBasedDecoder    ();
  void getParamsOfPhraseBasedDecoderBina();
  void getParamsOfHierarchyDecoder      ();
  void getParamsOfTree2TreeBasedDecoder ();
  void getParamsOfRecasingDecoder       ();
  void getParamsOfTokenizer             ();
  void getParamsOfDetokenizer           ();
  void getParamsOfSplitSentences        ();
  void getPhramsOfConvertPT2Binary      ();
  void GetPhramsOfTranslationMemory     ();
  void GetPhramsOfTrainingTMModel       ();
  void GetPhramsOfCalculateIbmBleu      ();
  void GetPhramsOfCalculateWdeMetric    ();
  void GetPhramsOfRescoring             ();
  void GetPhramsOfLMScore               ();
  void GetPhramsOfBuildIndex            ();
  void GetPhramsOfSearch                ();
  void GetPhramsOfAscii                 ();
  void GetPhramsOfAlignmentDifference   ();

 public:
  bool GetParameters  (int argc, string argv);
  bool GetAllFunctions(int argc);
  bool GetAllFunctions(int argc, string argv);

};

}

#endif


