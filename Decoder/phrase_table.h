/*
 * $Id:
 * 0015
 *
 * $File:
 * phrasetable.h
 *
 * $Proj:
 * Decoder for Statistical Machine Translation
 *
 * $Func:
 * decoder
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
 * 2014-10-24,15:50
 * 2012-12-10,18:57
 */

#ifndef DECODER_PHRASE_TABLE_H_
#define DECODER_PHRASE_TABLE_H_

#include <string>
#include <iostream>
#include <map>
#include <fstream>
#include <ctime>
#include "basic_method.h"
#include "configuration.h"
#include "debug.h"

using namespace std;
using namespace basic_method;
using namespace decoder_configuration;
using namespace debug;

namespace phrase_table {

typedef struct phr_tab_feature {
  unsigned int srcPhr         ;
  unsigned int tgtPhr         ;
  float        transModelScore;
} PHRTABSCORES;


class ScoresOfTranslateOption {
 public:
  float prob_egivenf_     ;    // Pr(e|f), f->e translation probablilty
  float lexi_egivenf_     ;    // Lex(e|f), f->e lexical weight
  float prob_fgivene_     ;    // Pr(f|e), e->f translation probability
  float lexi_fgivene_     ;    // Lex(f|e), e->f lexical weight
  float natural_logarithm_;    // Natural Logarithm e, number of phrases
  float bi_lexi_links_    ;    // number of bi-lex links (not fired in current version)
  float empty_translation_;    // value of empty translation

 public:
  vector<float> v_freefeature_value_;

 public:
  ScoresOfTranslateOption () {}
  ScoresOfTranslateOption (float &pEGF, float &lexEGF, float &pFGE, float &lexFGE, float &nl, float &bll, float &empty_translation)
                          : prob_egivenf_(pEGF), lexi_egivenf_(lexEGF), prob_fgivene_(pFGE), lexi_fgivene_(lexFGE),
                            natural_logarithm_(nl), bi_lexi_links_(bll), empty_translation_(empty_translation) {}
  ScoresOfTranslateOption (float &pEGF, float &lexEGF, float &pFGE, float &lexFGE, float &nl, float &bll, float &empty_translation, vector<float> &ffv)
                          : prob_egivenf_(pEGF), lexi_egivenf_(lexEGF), prob_fgivene_(pFGE), lexi_fgivene_(lexFGE), 
                            natural_logarithm_(nl), bi_lexi_links_(bll), empty_translation_(empty_translation), v_freefeature_value_(ffv) {}
  ~ScoresOfTranslateOption () {}
};


class TranslationOption {
 public:
  string                  tgtOption              ;
  ScoresOfTranslateOption scoresOfTranslateOption;
  vector<bool>            v_unaligned_flag_;

 public:
  TranslationOption () {}
  TranslationOption (string &tOpt, ScoresOfTranslateOption &scOfTrOpt)
                    : tgtOption(tOpt), scoresOfTranslateOption(scOfTrOpt) {}
  TranslationOption (string &tOpt, ScoresOfTranslateOption &scOfTrOpt, vector<bool> &v_unaligned_flag)
                    : tgtOption(tOpt), scoresOfTranslateOption(scOfTrOpt), v_unaligned_flag_(v_unaligned_flag) {}
};


class TranslationOptions {
 public:
  vector<TranslationOption> translationOptions;
};


class TranslationOptionBina {
 public:
  unsigned int            tgtOptionEncode;
  float                   transModelScore;

 public:
  TranslationOptionBina() {}
  TranslationOptionBina(unsigned int &tOpt, float &sc)
                       : tgtOptionEncode(tOpt), transModelScore(sc) {}
};


class TranslationOptionsBina {
 public:
  vector<TranslationOptionBina> translationOptionsBina;
};


class PhraseTable : public BasicMethod {
 public:
  PhraseTable() {};
  ~PhraseTable() {};

 public:
  bool Init (Configuration &config);
  bool ConvertToBinary(Configuration &config);

 private:
  bool InitTgtVocab (Configuration &config);
  bool InitPhraseTable (Configuration &config);
  bool InitPhraseTableContextSensitiveWd (Configuration &config);
  bool InitPhraseTableBina (Configuration &config);

 private:
  bool addConstPhrase ();
  bool addConstPhrase (Configuration &config);
  bool addConstPhraseContextSensitiveWd (Configuration &config);
  bool addConstPhraseBina ();
  bool addConstPhrase (Configuration &config, string &srcPhrase, string &tgtPhrase);
  bool addConstPhraseContextSensitiveWd (Configuration &config, string &srcPhrase, string &tgtPhrase);
  bool addConstPhrase (string &srcPhrase, string &tgtPhrase);
  bool addConstPhraseBina (string &srcPhrase, string &tgtPhrase);

 private:
  bool checkGeneration (string &srcPhrase, string &tgtPhrase);
  bool IsEmptyTranslation (string &target_phrase);

 public:
  map<string, int> tgtVocab;
  map<string, TranslationOptions> translationsOptions;

 public:
  map< unsigned int, TranslationOptionsBina > translationsOptionsBina;
  map< string, unsigned int > srcPhr;
  map< string, unsigned int > tgtPhr;

 public:
  vector< string > tgtPhrDecode;
  map< string, unsigned int > srcPhrEncode;

 public:
  static bool printCP2BLogo();
};
}

#endif



