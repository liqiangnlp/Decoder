/*
* $Id:
* 0014
*
* $File:
* model.cpp
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
* 2012-12-10,15:56
*/

#include "model.h"


namespace model {

bool Model::Init (Configuration &config) {
  ngram_language_model_.LoadModel (config);
  phrase_table_.Init (config);
  if (config.use_me_reorder_) {
    me_reorder_table_.init (config);
  }
  if (config.use_msd_reorder_) {
    msd_reorder_table_.init (config);
  }
  if (config.use_context_sensitive_wd_) {
    context_sensitive_wd_.Init (config);
  }
  config.PrintConfig ();
  config.PrintFeatures ();
  return true;
}


}

