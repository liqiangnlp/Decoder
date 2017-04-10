/*
* $Id:
* 0003
*
* $File:
* parameter_controller.cpp
*
* $Proj:
* Decoder for Statistical Machine Translation
*
* $Func:
* header file of control parameters
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
* 2012-12-04,09:56
*/

#include "parameter_controller.h"


namespace parameter_controller {

void GetParameter::getParamsOfPhraseBasedDecoder() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# PB-Decoder for Statistical Machine Translation                  #\n"
      <<"#                                             Version 0.0.1       #\n"
      <<"#                                             NEUNLPLab/YAYI corp #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"[USAGE]\n"
      <<"                   Decoder           --PBD\n"
      <<"[OPTION]\n"
      <<"                   -MERT     :   MERT Flag.\n"
      <<"                   -config   :   Config File.\n"
      <<"                   -dev      :   Development Set for MERT.\n"
      <<"                   -test     :   Test File for Decoding.\n"
      <<"                   -output   :   Outputted File.\n"
      <<"                   -log      :   Log File.\n"
      <<"[EXAMPLE]\n"
      <<"    >> Phrase-Based Decoder for Statistical Machine Translation\n"
      <<"                 Decoder       --PBD       [-config FILE]\n"
      <<"                                           [-test   FILE]\n"
      <<"                                           [-output FILE]\n"
      <<flush;
  return;
}


void GetParameter::getParamsOfPhraseBasedDecoderBina() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# PB-Decoder (Binary) for Statistical Machine Translation         #\n"
      <<"#                                             Version 0.0.1       #\n"
      <<"#                                             NEUNLPLab/YAYI corp #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"[USAGE]\n"
      <<"                   Decoder           --PBDB\n"
      <<"[OPTION]\n"
      <<"                   -config   :   Config File.\n"
      <<"                   -test     :   Test File for Decoding.\n"
      <<"                   -output   :   Outputted File.\n"
      <<"                   -nthread  :   Parameter for Multi Threads.\n"
      <<"[EXAMPLE]\n"
      <<"    >> Phrase-Based Decoder for Statistical Machine Translation\n"
      <<"                 Decoder       --PBDB      [-config FILE]\n"
      <<"                                           [-test   FILE]\n"
      <<"                                           [-output FILE]\n"
      <<flush;
  return;
}


void GetParameter::getParamsOfHierarchyDecoder() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# HB-Decoder for Statistical Machine Translation                  #\n"
      <<"#                                             Version 0.0.1       #\n"
      <<"#                                             NEUNLPLab/YAYI corp #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"[USAGE]\n"
      <<"                   Decoder           --HBD\n"
      <<"[OPTION]\n"
      <<"                   To be continued...\n"
      <<"[EXAMPLE]\n"
      <<" >> Hierarchy Phrase-Based Decoder for Statistical Machine Translation\n"
      <<"                   Decoder   --HBD ...\n"
      <<flush;

#ifdef __DEBUG_PARAMETER_CONTROLLER__
  cerr<<"To be continued... Exit!\n"<<flush;
#endif

  exit( 1 );
}


void GetParameter::getParamsOfTree2TreeBasedDecoder() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# SB-Decoder for Statistical Machine Translation                  #\n"
      <<"#                                             Version 0.0.1       #\n"
      <<"#                                             NEUNLPLab/YAYI corp #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"[USAGE]\n"
      <<"                   Decoder           --T2T\n"
      <<"[OPTION]\n"
      <<"                   -config   :   Config File.\n"
      <<"                   -test     :   Test File for Decoding.\n"
      <<"                   -output   :   Translation Results.\n"
      <<"[EXAMPLE]\n"
      <<" >> Tree-2-Tree Based Decoder for Statistical Machine Translation\n"
      <<"                   Decoder   --T2T        [-config FILE]\n"
      <<"                                          [-test   FILE]\n"
      <<"                                          [-output FILE]\n"
      <<flush;

#ifdef __DEBUG_PARAMETER_CONTROLLER__
  cerr<<"To be continued... Exit!\n"<<flush;
#endif

  exit( 1 );
}


void GetParameter::getParamsOfRecasingDecoder() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# Recasing for Statistical Machine Translation                    #\n"
      <<"#                                             Version 0.0.1       #\n"
      <<"#                                             NEUNLPLab/YAYI corp #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"[USAGE]\n"
      <<"                   Decoder           --REC\n"
      <<"[OPTION]\n"
      <<"                   -config   :   Config File.\n"
      <<"                   -test     :   Test File for Recasing.\n"
      <<"                   -output   :   Outputted File.\n"
      <<"[EXAMPLE]\n"
      <<"    >> Recasing for Statistical Machine Translation\n"
      <<"                 Decoder      --REC        [-config FILE]\n"
      <<"                                           [-test   FILE]\n"
      <<"                                           [-output FILE]\n"
      <<flush;
  return;
}


void GetParameter::getParamsOfTokenizer() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# Tokenizer for Statistical Machine Translation                   #\n"
      <<"#                                            Version 0.0.1        #\n"
      <<"#                                            liqiangneu@gmail.com #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"[USAGE]\n"
      <<"                   Decoder           --TOKEN\n"
      <<"[OPTION]\n"
      <<"                     -lang   :   Language (\"en\" or \"zh\").\n"
      <<"                    -input   :   Inputted  File.\n"
      <<"               -prefixfile   :   Inputted Prefix File.\n"
      <<"                   -output   :   Outputted File.\n"
      <<"[EXAMPLE]\n"
      <<"    >> Tokenizer for Statistical Machine Translation\n"
      <<"                 Decoder      --TOKEN      [-lang         en]\n"
      <<"                                           [-input      FILE]\n"
      <<"                                           [-prefixfile FILE]\n"
      <<"                                           [-output     FILE]\n"
      <<flush;
  return;
}


void GetParameter::getParamsOfSplitSentences() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# Split Sentences                                                 #\n"
      <<"#                                            Version 0.0.1        #\n"
      <<"#                                            liqiangneu@gmail.com #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"[USAGE]\n"
      <<"                   Decoder           --SPLITSENT\n"
      <<"[OPTION]\n"
      <<"                     -lang   :   Language (\"en\" or \"zh\").\n"
      <<"                    -input   :   Inputted  File.\n"
      <<"               -prefixfile   :   Inputted Prefix File.\n"
      <<"                   -output   :   Outputted File.\n"
      <<"[EXAMPLE]\n"
      <<"    >> Split Sentences\n"
      <<"                 Decoder    --SPLITSENT    [-lang         en]\n"
      <<"                                           [-input      FILE]\n"
      <<"                                           [-prefixfile FILE]\n"
      <<"                                           [-output     FILE]\n"
      <<flush;
  return;
}


void GetParameter::getParamsOfDetokenizer() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# Detokenizer for Statistical Machine Translation                 #\n"
      <<"#                                             Version 0.0.1       #\n"
      <<"#                                             NEUNLPLab/YAYI corp #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"[USAGE]\n"
      <<"                   Decoder           --DET\n"
      <<"[OPTION]\n"
      <<"                   -input    :   Inputted  File.\n"
      <<"                   -output   :   Outputted File.\n"
      <<"                   -lang     :   Language (\"en\" or \"zh\").\n"
      <<"                   -punct    :   Punctuation Set.\n"
      <<"[EXAMPLE]\n"
      <<"    >> Detokenizer for Statistical Machine Translation\n"
      <<"                 Decoder      --DET        [-input  FILE]\n"
      <<"                                           [-output FILE]\n"
      <<flush;
  return;
}


void GetParameter::getPhramsOfConvertPT2Binary() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# CONPT2B for Statistical Machine Translation                     #\n"
      <<"#                                             Version 0.0.1       #\n"
      <<"#                                             NEUNLPLab/YAYI corp #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"[USAGE]\n"
      <<"                   Decoder           --CP2B\n"
      <<"[OPTION]\n"
      <<"                   -config   :   Configuration File.\n"
      <<"                   -input    :   Inputted      File.\n"
      <<"                   -output   :   Outputted     File.\n"
      <<"[EXAMPLE]\n"
      <<"    >> CONPT2B for Statistical Machine Translation\n"
      <<"                 Decoder      --CP2B       [-config FILE]\n"
      <<"                                           [-input  FILE]\n"
      <<"                                           [-output FILE]\n"
      <<flush;
  return;
}


void GetParameter::GetPhramsOfTranslationMemory() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# Translation Memory                                              #\n"
      <<"#                                             Version 0.0.1       #\n"
      <<"#                                             NEUNLPLab/YAYI corp #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"[USAGE]\n"
      <<"                   Decoder           --TM\n"
      <<"[OPTION]\n"
      <<"                   -config   :   Configuration File.\n"
      <<"                   -input    :   Inputted      File.\n"
      <<"                   -output   :   Outputted     File.\n"
      <<"[EXAMPLE]\n"
      <<"    >> Translation Memory\n"
      <<"                 Decoder      --TM         [-config FILE]\n"
      <<"                                           [-input  FILE]\n"
      <<"                                           [-output FILE]\n"
      <<flush;
  return;
}


/*
 * $Name: GetPhramsOfTrainingTMModel
 * $Function:
 * $Date: 2013-05-09
 */
void GetParameter::GetPhramsOfTrainingTMModel() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# Training Translation Memory Model                               #\n"
      <<"#                                             Version 0.0.1       #\n"
      <<"#                                             NEUNLPLab/YAYI corp #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"[USAGE]\n"
      <<"                   Decoder           --TTMM\n"
      <<"[OPTION]\n"
      <<"                   -punct  :   Inputted Punctuation Dictionary.\n"
      <<"                   -src    :   Inputted Source Language File.\n"
      <<"                   -tgt    :   Inputted Target Language File.\n"
      <<"                   -out    :   Outputted TM Model File.\n"
      <<"[EXAMPLE]\n"
      <<"    >> Training Translation Memory Model\n"
      <<"                   Decoder      --TTMM       [-punct FILE]\n"
      <<"                                             [-src   FILE]\n"
      <<"                                             [-tgt   FILE]\n"
      <<"                                             [-out   FILE]\n"
      <<flush;
  return;
}


/*
 * $Name: GetPhramsOfCalculateIBMBLEU
 * $Function:
 * $Date: 2013-05-09
 */
void GetParameter::GetPhramsOfCalculateIbmBleu() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# Calculate IBM BLEU Score                                        #\n"
      <<"#                                             Version 0.0.1       #\n"
      <<"#                                             NEUNLPLab/YAYI corp #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"[USAGE]\n"
      <<"                   Decoder           --BLEU\n"
      <<"[OPTION]\n"
      <<"                   -1best  :   Inputted 1-best Translation Results.\n"
      <<"                   -dev    :   Inputted Development Set.\n"
      <<"                   -nref   :   The number of references.\n"
      <<"                   -out    :   Outputted IBM BLEU Score.\n"
      <<"[EXAMPLE]\n"
      <<"    >> Calculate IBM BLEU Score\n"
      <<"                   Decoder      --BLEU       [-1best FILE]\n"
      <<"                                             [-dev   FILE]\n"
      <<"                                             [-nref   NUM]\n"
      <<"                                             [-out   FILE]\n"
      <<flush;
  return;
}


void GetParameter::GetPhramsOfCalculateWdeMetric() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# Calculate WDE Metric Score                                      #\n"
      <<"#                                             Version 0.0.1       #\n"
      <<"#                                             NEUNLPLab/YAYI corp #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"[USAGE]\n"
      <<"                   Decoder           --WDEM\n"
      <<"[OPTION]\n"
      <<"                   -1best   :   Inputted 1-best Translation Results.\n"
      <<"                     -dev   :   Inputted Development Set.\n"
	  <<"                -stopword   :   Inputted Stopword File.\n"
      <<"                    -nref   :   The number of references.\n"
      <<"                     -out   :   Outputted WDE Metric Score.\n"
      <<"[EXAMPLE]\n"
      <<"    >> Calculate WDE Metric Score\n"
      <<"                   Decoder      --WDEM       [-1best    FILE]\n"
      <<"                                             [-dev      FILE]\n"
	  <<"                                             [-stopword FILE]\n"
      <<"                                             [-nref      NUM]\n"
      <<"                                             [-out      FILE]\n"
      <<flush;
  return;
}


void GetParameter::GetPhramsOfRescoring() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# Rescoring                                                       #\n"
      <<"#                                            Version 0.0.1        #\n"
      <<"#                                            liqiangneu@gmail.com #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"[USAGE]\n"
      <<"                   Decoder           --RESCORING\n"
      <<"[OPTION]\n"
      <<"                     -dev   :   Inputted Development Set.\n"
      <<"                -devnbest   :   Inputted Development N-Best.\n"
      <<"                    -test   :   Inputted Test Set.\n"
      <<"               -testnbest   :   Inputted Test N-Best.\n"
      <<"             -initweights   :   Inputted Init Weights File.\n"
      <<"                    -nref   :   The Number of References.\n"
      <<"                  -nround   :   Number of Mert Run.\n"
      <<"                     -out   :   Outputted Test 1-Best.\n"
      <<"[EXAMPLE]\n"
      <<"    >> Calculate WDE Metric Score\n"
      <<"                   Decoder    --RESCORING    [-dev         FILE]\n"
      <<"                                             [-devnbest    FILE]\n"
      <<"                                             [-test        FILE]\n"
      <<"                                             [-testnbest   FILE]\n"
      <<"                                             [-initweights FILE]\n"
      <<"                                             [-nref         NUM]\n"
      <<"                                             [-nround       NUM]\n"
      <<"                                             [-out         FILE]\n"
      <<flush;
  return;
}


void GetParameter::GetPhramsOfLMScore() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# Calculate N-Gram LM Score                                       #\n"
      <<"#                                            Version 0.0.1        #\n"
      <<"#                                            liqiangneu@gmail.com #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"[USAGE]\n"
      <<"                   Decoder           --LM\n"
      <<"[OPTION]\n"
      <<"                   -lmbin   :   Binary LM file.\n"
      <<"                   -vocab   :   Vocab file.\n"
      <<"                   -ngram   :   The order of n-gram.\n"
      <<"                   -input   :   Inputted file.\n"
      <<"                  -output   :   Outputted file.\n"
      <<"[EXAMPLE]\n"
      <<"    >> Calculate N-Gram LM Score\n"
      <<"                   Decoder    --LM           [-lmbin       FILE]\n"
      <<"                                             [-vocab       FILE]\n"
      <<"                                             [-ngram        NUM]\n"
      <<"                                             [-input       FILE]\n"
      <<"                                             [-output      FILE]\n"
      <<flush;
  return;
}



void GetParameter::GetPhramsOfBuildIndex() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# Build Index for Searching                                       #\n"
      <<"#                                            Version 0.0.1        #\n"
      <<"#                                            liqiangneu@gmail.com #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"[USAGE]\n"
      <<"                   Decoder           --BUILDINDEX\n"
      <<"[OPTION]\n"
      <<"                    -lang   :   Language (\"en\" or \"zh\").\n"
      <<"                  -corpus   :   Inputted Corpus file.\n"
      <<"                -stopword   :   Inputted Stopword file.\n"
      <<"              -prefixfile   :   Inputted Prefix File.\n"
      <<"               -whitelist   :   Inputted White List File.\n"
      <<"                  -cutoff   :   Words whose frequency is equal to or\n"
      <<"                                less than 'cutoff' will be removed.\n"
      <<"               -cutoffrhf   :   Words that are relative top n/m will be removed.\n"
      <<"                   -index   :   Outputted Index file.\n"
      <<"[EXAMPLE]\n"
      <<"    >> Build Index for Search Engine\n"
      <<"                   Decoder    --BUILDINDEX    [-corpus     FILE]\n"
      <<"                                              [-lang         en]\n"
      <<"                                              [-stopword   FILE]\n"
      <<"                                              [-prefixfile FILE]\n"
      <<"                                              [-cutoff      NUM]\n"
      <<"                                              [-index      FILE]\n"
      <<flush;
  return;
}


void GetParameter::GetPhramsOfSearch() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# Search Engine                                                   #\n"
      <<"#                                            Version 0.0.1        #\n"
      <<"#                                            liqiangneu@gmail.com #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"[USAGE]\n"
      <<"                   Decoder           --SEARCH\n"
      <<"[OPTION]\n"
      <<"                  -corpus   :   Inputted Corpus file.\n"
      <<"                   -index   :   Inputted Index file.\n"
      <<"                    -topn   :   Top n best.\n"
      <<"                  -search   :   Inputted Search file.\n"
      <<"                  -output   :   Outputted Search Result.\n"
      <<"[EXAMPLE]\n"
      <<"    >> Search Engine\n"
      <<"                   Decoder    --SEARCH      [-corpus   FILE]\n"
      <<"                                            [-index    FILE]\n"
      <<"                                            [-topn       50]\n"
      <<"                                            [-search   FILE]\n"
      <<"                                            [-output   FILE]\n"
      <<flush;
  return;
}


void GetParameter::GetPhramsOfAscii() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# ASCII                                                           #\n"
      <<"#                                            Version 0.0.1        #\n"
      <<"#                                            liqiangneu@gmail.com #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"[USAGE]\n"
      <<"                   Decoder           --ASCII\n"
      <<"[OPTION]\n"
      <<"                   -input   :   Inputted file.\n"
      <<"                  -output   :   Outputted file.\n"
      <<"[EXAMPLE]\n"
      <<"    >> Search Engine\n"
      <<"                   Decoder      --ASCII      [-input    FILE]\n"
      <<"                                             [-output   FILE]\n"
      <<flush;
  return;
}




void GetParameter::GetPhramsOfAlignmentDifference() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# Alignment Difference                                            #\n"
      <<"#                                            Version 0.0.1        #\n"
      <<"#                                            liqiangneu@gmail.com #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"[USAGE]\n"
      <<"                   Decoder           --ALNDIFF\n"
      <<"[OPTION]\n"
      <<"                   -aln1    :   Inputted alignment 1.\n"
      <<"                   -aln2    :   Inputted alignment 2.\n"
      <<"                    -src    :   Inputted source file.\n"
      <<"                    -tgt    :   Inputted target file.\n"
      <<"                 -output    :   Outputted file.\n"
      <<"[EXAMPLE]\n"
      <<"    >> Alignment Difference\n"
      <<"                   Decoder      --ALNDIFF    [-aln1     FILE]\n"
      <<"                                             [-aln2     FILE]\n"
      <<"                                             [-src      FILE]\n"
      <<"                                             [-tgt      FILE]\n"
      <<"                                             [-output   FILE]\n"
      <<flush;
  return;
}




bool GetParameter::GetAllFunctions(int argc) {
  if (argc <= 1) {
    cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
        <<"# Decoder for Statistical Machine Translation                     #\n"
        <<"#                                             Version 0.0.1       #\n"
        <<"#                                             NEUNLPLab/YAYI corp #\n"
        <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
        <<"[USAGE]\n"
        <<"          Decoder           <action>           [OPTIONS]\n"
        <<"[ACTION]\n"
        <<"               --PBD          :   Phrase-Based Decoder.\n"
        <<"               --PBDB         :   Phrase-Based Decoder (Binary).\n"
        <<"               --HBD          :   Hierarchy Phrase-Based Decoder.\n"
        <<"               --T2T          :   Tree2Tree Syntax-Based Decoder.\n"
        <<"               --REC          :   Recasing Tool.\n"
        <<"               --TOKEN        :   Tokenizer Tool.\n"
        <<"               --DET          :   Detokenizer Tool.\n"
        <<"               --SPLITSENT    :   Split Sentences Tool.\n"
        <<"               --CP2B         :   Convert Phrase Table to Binary Format.\n"
        <<"               --TM           :   Translation Memory.\n"
        <<"               --TTMM         :   Training Translation Memory Model.\n"
        <<"               --BLEU         :   Calculate IBM BLEU Score.\n"
        <<"               --WDEM         :   Calculate WDE Metric Score.\n"
        <<"               --RESCORING    :   Rescoring.\n"
        <<"               --LM           :   Calculate N-Gram LM Score.\n"
        <<"               --BUILDINDEX   :   Build Index for Searching.\n"
        <<"               --SEARCH       :   Search Engine.\n"
        <<"               --ASCII        :   Filter Sentences That Have UN-ASCII.\n"
        <<"               --ALNDIFF      :   Calculate Difference between Two Alignments.\n"
        <<"[TOOLS]\n"
        <<"     >> Get Options of Phrase-Based Decoder\n"
        <<"                  Decoder            --PBD\n"
        <<"     >> Get Options of Phrase-Based Decoder\n"
        <<"                  Decoder            --PBDB\n"
        <<"     >> Get Options of Hierarchy Phrase-Based Decoder\n"
        <<"                  Decoder            --HBD\n"
        <<"     >> Get Options of Tree2Tree Syntax-Based Decoder\n"
        <<"                  Decoder            --T2T\n"
        <<"     >> Get Options of Recasing Tool\n"
        <<"                  Decoder            --REC\n"
        <<"     >> Get Options of Tokenizer Tool\n"
        <<"                  Decoder            --TOKEN\n"
        <<"     >> Get Options of Detokenizer Tool\n"
        <<"                  Decoder            --DET\n"
        <<"     >> Get Options of Split Sentences Tool\n"
        <<"                  Decoder            --SPLITSENT\n"
        <<"     >> Get Options of Convert PT to Binary Format.\n"
        <<"                  Decoder            --CP2B\n"
        <<"     >> Get Options of Translation Memory\n"
        <<"                  Decoder            --TM\n"
        <<"     >> Get Options of Training Translation Memory Model\n"
        <<"                  Decoder            --TTMM\n"
        <<"     >> Get Options of Calculating IBM BLEU Score\n"
        <<"                  Decoder            --BLEU\n"
        <<"     >> Get Options of Calculating WDE Metric Score\n"
        <<"                  Decoder            --WDEM\n"
        <<"     >> Get Options of Rescoring\n"
        <<"                  Decoder            --RESCORING\n"
        <<"     >> Get Options of Calculating N-Gram LM Score\n"
        <<"                  Decoder            --LM\n"
        <<"     >> Get Options of Building Index for Searching\n"
        <<"                  Decoder            --BUILDINDEX\n"
        <<"     >> Get Options of Search Engine\n"
        <<"                  Decoder            --SEARCH\n"
        <<"     >> Get Options of ASCII\n"
        <<"                  Decoder            --ASCII\n"
        <<"     >> Get Options of Calculating Alignment Difference\n"
        <<"                  Decoder            --ALNDIFF\n\n"
        <<flush;
    return true;
  }
  return false;
}


bool GetParameter::GetAllFunctions(int argc, string argv) {
  if ((argc % 2 != 0) || (argc % 2 == 0) &&
      ((argv != "--PBD") &&
       (argv != "--PBDB") &&
       (argv != "--HBD") &&
       (argv != "--T2T") &&
       (argv != "--REC") &&
       (argv != "--DET") &&
       (argv != "--CP2B") &&
       (argv != "--TM") &&
       (argv != "--TTMM") &&
       (argv != "--BLEU") &&
	   (argv != "--WDEM") && 
       (argv != "--RESCORING") &&
       (argv != "--LM") &&
       (argv != "--BUILDINDEX") &&
       (argv != "--SEARCH") &&
       (argv != "--TOKEN") && 
       (argv != "--SPLITSENT") &&
       (argv != "--ASCII") &&
       (argv != "--ALNDIFF"))) {
    cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
        <<"# Decoder for Statistical Machine Translation                     #\n"
        <<"#                                             Version 0.0.1       #\n"
        <<"#                                             NEUNLPLab/YAYI corp #\n"
        <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
        <<"[USAGE]\n"
        <<"          Decoder           <action>           [OPTIONS]\n"
        <<"[ACTION]\n"
        <<"               --PBD          :   Phrase-Based Decoder.\n"
        <<"               --PBDB         :   Phrase-Based Decoder (Binary).\n"
        <<"               --HBD          :   Hierarchy Phrase-Based Decoder.\n"
        <<"               --T2T          :   Tree2Tree Syntax-Based Decoder.\n"
        <<"               --REC          :   Recasing Tool.\n"
        <<"               --TOKEN        :   Tokenizer Tool.\n"
        <<"               --DET          :   Detokenizer Tool.\n"
        <<"               --SPLITSENT    :   Split Sentences Tool.\n"
        <<"               --CP2B         :   Convert Phrase Table to Binary Format.\n"
        <<"               --TM           :   Translation Memory.\n"
        <<"               --TTMM         :   Training Translation Memory Model.\n"
        <<"               --BLEU         :   Calculate IBM BLEU Score.\n"
        <<"               --WDEM         :   Calculate WDE Metric Score.\n"
        <<"               --RESCORING    :   Rescoring.\n"
        <<"               --LM           :   Calculate N-Gram LM Score.\n"
        <<"               --BUILDINDEX   :   Build Index for Searching.\n"
        <<"               --SEARCH       :   Search Engine.\n"
        <<"               --ASCII        :   Filter Sentences That Have UN-ASCII.\n"
        <<"               --ALNDIFF      :   Calculate Difference between Two Alignments.\n"
        <<"[TOOLS]\n"
        <<"     >> Get Options of Phrase-Based Decoder\n"
        <<"                  Decoder            --PBD\n"
        <<"     >> Get Options of Phrase-Based Decoder\n"
        <<"                  Decoder            --PBDB\n"
        <<"     >> Get Options of Hierarchy Phrase-Based Decoder\n"
        <<"                  Decoder            --HBD\n"
        <<"     >> Get Options of Tree2Tree Syntax-Based Decoder\n"
        <<"                  Decoder            --T2T\n"
        <<"     >> Get Options of Recasing Tool\n"
        <<"                  Decoder            --REC\n"
        <<"     >> Get Options of Tokenizer Tool\n"
        <<"                  Decoder            --TOKEN\n"
        <<"     >> Get Options of Detokenizer Tool\n"
        <<"                  Decoder            --DET\n"
        <<"     >> Get Options of Split Sentences Tool\n"
        <<"                  Decoder            --SPLITSENT\n"
        <<"     >> Get Options of Convert PT to Binary Format.\n"
        <<"                  Decoder            --CP2B\n"
        <<"     >> Get Options of Translation Memory\n"
        <<"                  Decoder            --TM\n"
        <<"     >> Get Options of Training Translation Memory Model\n"
        <<"                  Decoder            --TTMM\n"
        <<"     >> Get Options of Calculate IBM BLEU Score\n"
        <<"                  Decoder            --BLEU\n"
        <<"     >> Get Options of Calculate WDE Metric Score\n"
        <<"                  Decoder            --WDEM\n"
        <<"     >> Get Options of Rescoring\n"
        <<"                  Decoder            --RESCORING\n"
        <<"     >> Get Options of Calculating N-Gram LM Score\n"
        <<"                  Decoder            --LM\n"
        <<"     >> Get Options of Building Index for Searching\n"
        <<"                  Decoder            --BUILDINDEX\n"
        <<"     >> Get Options of Search Engine\n"
        <<"                  Decoder            --SEARCH\n"
        <<"     >> Get Options of ASCII\n"
        <<"                  Decoder            --ASCII\n"
        <<"     >> Get Options of Calculating Alignment Difference\n"
        <<"                  Decoder            --ALNDIFF\n\n"
        <<flush;
    return true;
  }
  return false;
}


bool GetParameter::GetParameters(int argc, string argv) {
  if (argc == 2) {
    if (argv == "--PBD") getParamsOfPhraseBasedDecoder();
    else if (argv == "--PBDB") getParamsOfPhraseBasedDecoderBina();
    else if (argv == "--HBD") getParamsOfHierarchyDecoder();
    else if (argv == "--T2T") getParamsOfTree2TreeBasedDecoder();
    else if (argv == "--REC") getParamsOfRecasingDecoder();
    else if (argv == "--TOKEN") getParamsOfTokenizer();
    else if (argv == "--DET") getParamsOfDetokenizer();
    else if (argv == "--SPLITSENT") getParamsOfSplitSentences();
    else if (argv == "--CP2B") getPhramsOfConvertPT2Binary();
    else if (argv == "--TM") GetPhramsOfTranslationMemory();
    else if (argv == "--TTMM") GetPhramsOfTrainingTMModel();
    else if (argv == "--BLEU") GetPhramsOfCalculateIbmBleu();
    else if (argv == "--WDEM") GetPhramsOfCalculateWdeMetric();
    else if (argv == "--RESCORING") GetPhramsOfRescoring();
    else if (argv == "--LM") GetPhramsOfLMScore();
    else if (argv == "--BUILDINDEX") GetPhramsOfBuildIndex();
    else if (argv == "--SEARCH") GetPhramsOfSearch();
    else if (argv == "--ASCII") GetPhramsOfAscii();
    else if (argv == "--ALNDIFF") GetPhramsOfAlignmentDifference();
    return true;
  }
  return false;
}


}


