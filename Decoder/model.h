/*
 * $Id:
 * 0013
 *
 * $File:
 * model.h
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
 * 2012-12-10,15:55
 */

#ifndef DECODER_MODEL_H__
#define DECODER_MODEL_H__

#include "language_model.h"
#include "phrase_table.h"
#include "me_reordering_table.h"
#include "msd_reordering_table.h"
#include "context_sensitive_wd.h"
#include "test_set.h"
#include "configuration.h"
#include "debug.h"

using namespace phrase_table        ;
using namespace language_model      ;
using namespace decoder_me_reordering_table ;
using namespace decoder_context_sensitive_wd;
using namespace msd_reordering_table;
using namespace decoder_test_set;
using namespace decoder_configuration;
using namespace debug               ;


namespace model {

class Model {
 public:
  LanguageModel      ngram_language_model_;
  PhraseTable        phrase_table_;
  MsdReorderingTable msd_reorder_table_;
  MeReorderingTable  me_reorder_table_;
  ContextSensitiveWd context_sensitive_wd_;

 public:
  Model () {};
  ~Model () {};

 public:
  bool Init (Configuration &config);

};

}

#endif


