/*
* $Id:
* 0032
*
* $File:
* recasing.cpp
*
* $Proj:
* Recasing for Statistical Machine Translation
*
* $Func:
* recasing
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
* 2013-03-16,10:00
*/

#include "recasing.h"


namespace decoder_recasing {

/*
 * $Name: 
 * $Function: 
 * $Date: 2013-05-28
 */
bool Recasing::Init(Configuration &config, Model &model) {
  test_set_.Init( config );
  return true;
}


/*
 * $Name: 
 * $Function: 
 * $Date: 2013-05-28
 */
bool Recasing::Decode(Configuration &config, Model &model) {
  clock_t start, finish;
  start = clock();
  cerr<<"Start decoding with CYK Algorithm\n"<<flush;
  size_t lineNo = 0;

  ofstream outFile( config.output_file_.c_str() );
  if ( !outFile ) {
    cerr<<"ERROR: Can not open file \""
        <<config.output_file_
        <<"\".\n"
        <<flush;
    exit( 1 );
  }

  ofstream logFile( config.log_file_.c_str() );
  if ( !outFile ) {
    cerr<<"ERROR: Can not open file \""
        <<config.log_file_
        <<"\".\n"
        <<flush;
    exit( 1 );
  }

  for ( size_t sentID = 0; sentID < test_set_.test_set_.size(); ++sentID ) {
    ++lineNo;
    Cells cells;

    AllocateSpaceForCells( cells, test_set_.test_set_.at( sentID ) );
    Decode( config, model, cells, test_set_.test_set_.at( sentID ), outFile, logFile );
    if ( lineNo % 1000 == 0 ) {
      finish = clock();
      cerr<<"\r  Processed "<<lineNo<<" lines. [time="
          <<(double)( finish - start )/CLOCKS_PER_SEC<<"s speed="
          <<lineNo/((double)( finish - start )/CLOCKS_PER_SEC)<<"sent/s]   "
          <<flush;
    }
  }
  finish = clock();
  cerr<<"\r  Processed "<<lineNo<<" lines. [time="
      <<(double)( finish - start )/CLOCKS_PER_SEC<<" s speed="
      <<lineNo/((double)( finish - start )/CLOCKS_PER_SEC)<<"sent/s]   \n"
      <<"  Done!\n"<<flush;
  logFile.close();
  outFile.close();

  if (config.mert_flag_) {
    ofstream tmpConf(config.mert_config_file_.c_str());
    tmpConf<<config.features.features_number_<<"\n";
    int count = 0;
    for ( vector< FeatureValue >::iterator iter = config.features.feature_values_.begin(); iter != config.features.feature_values_.end(); ++iter ) {
      ++count;
      tmpConf<<count<<" "<<iter->weight_<<" "<<iter->min_value_<<" "<<iter->max_value_<<" "<<iter->fixed_<<"\n";
    }
    tmpConf.close();
  }
  return true;
}


/*
 * $Name: 
 * $Function: 
 * $Date: 2013-05-28
 */
bool Recasing::Decode(Configuration &config, Model &model, SourceSentence &sourceSentence, string &output) {
  Cells cells;
  AllocateSpaceForCells( cells, sourceSentence );
  Decode( config, model, cells, sourceSentence, output );
  return true;
}


/*
 * $Name: 
 * $Function: 
 * $Date: 2013-05-28
 */
bool Recasing::Decode(Configuration &config, Model &model, Cells &cells, SourceSentence &sourceSentence, ofstream &outFile, ofstream &logFile) {
  InitCells( config, model, cells, sourceSentence );
  if ( config.recase_method_ == "l2r" ) {
    DecodeWithLeftToRight( config, model, cells, sourceSentence );
  } else if ( config.recase_method_ == "cyk") {
    DecodeWithCYKAlgorithm( config, model, cells, sourceSentence );
  } else {
    cerr<<"ERROR: Please check the parameter \""<<config.recase_method_<<"\" in your config file.\n"<<flush;
    exit( 1 );
  }
  OutputTranslationResult( config, cells , sourceSentence, outFile, logFile );
  return true;
}


/*
 * $Name: 
 * $Function: 
 * $Date: 2013-05-28
 */
bool Recasing::Decode(Configuration &config, Model &model, Cells &cells, SourceSentence &sourceSentence, string &output) {
  InitCells( config, model, cells, sourceSentence );
  if ( config.recase_method_ == "l2r" ) {
    DecodeWithLeftToRight( config, model, cells, sourceSentence );
  } else if( config.recase_method_ == "cyk") {
    DecodeWithCYKAlgorithm( config, model, cells, sourceSentence );
  } else {
    cerr<<"ERROR: Please check the parameter \""<<config.recase_method_<<"\" in your config file.\n"<<flush;
    exit( 1 );
  }

  GenerateTranslationResult( config, cells, sourceSentence, output );
  return true;
}


/*
 * $Name: 
 * $Function: 
 * $Date: 2013-05-28
 */
bool Recasing::AllocateSpaceForCells(Cells &cells, SourceSentence &sourceSentence) {
  cells.cells_.resize( sourceSentence.cell_span_.size() + 1 );
  for ( VEC3HYPOIT iter = cells.cells_.begin(); iter != cells.cells_.end(); ++ iter ) {
    iter->resize( sourceSentence.cell_span_.size() + 1 );
  }
  return true;
}


/*
 * $Name: 
 * $Function: 
 * $Date: 2013-05-28
 */
bool Recasing::DecodeWithCYKAlgorithm(Configuration  &config, Model &model, Cells &cells, SourceSentence &sourceSentence) {
  size_t currentSentLength = sourceSentence.cell_span_.size();

  for ( size_t phraseLength = 2; phraseLength <= currentSentLength; ++ phraseLength ) {
    for ( size_t startPos = 0; ( startPos <  currentSentLength ) && ( startPos + phraseLength <= currentSentLength ); ++ startPos ) {
      size_t endPos = startPos + phraseLength;

      CELLFORCUBEPRUNING cellForCubePruning;
      if ( cells.cells_.at( startPos ).at( endPos ).size() != 0 ) {
        for ( CELL::iterator iter = cells.cells_.at( startPos ).at( endPos ).begin(); iter != cells.cells_.at( startPos ).at( endPos ).end(); ++ iter ) {
          float modelScore = iter->feature_score_except_lm_ + iter->language_model_score_;
          cellForCubePruning.insert( make_pair( modelScore, *iter ) );
        }
      }

      if ( CheckForPunctPruning( config, startPos, endPos, sourceSentence ) ) {
        for ( size_t segPos = startPos + 1; segPos < endPos; ++ segPos ) {
          if ( cells.cells_.at( startPos ).at( segPos ).size() == 0 || cells.cells_.at( segPos ).at( endPos ).size() == 0 )
            continue;
          Compose( config, model, cells.cells_.at( startPos ).at( segPos ), cells.cells_.at( segPos ).at( endPos ), startPos, segPos, endPos, cellForCubePruning, sourceSentence );
        } 
        SetBeamSize( config, cellForCubePruning, cells.cells_.at( startPos ).at( endPos ) );
      }  
    }
  }
  return true;
}


/*
 * $Name: 
 * $Function: 
 * $Date: 2013-05-28
 */
bool Recasing::DecodeWithLeftToRight(Configuration &config, Model &model, Cells &cells, SourceSentence &sourceSentence) {
  size_t currentSentLength = sourceSentence.cell_span_.size();
  for ( size_t phraseLength = 2; phraseLength <= currentSentLength; ++ phraseLength ) {
    size_t startPos = 0;
    size_t segPos = startPos + phraseLength - 1;
    size_t endPos = startPos + phraseLength; 

    CELLFORCUBEPRUNING cellForCubePruning;
    if ( cells.cells_.at( startPos ).at( endPos ).size() != 0 ) {
      for ( CELL::iterator iter = cells.cells_.at( startPos ).at( endPos ).begin(); \
            iter != cells.cells_.at( startPos ).at( endPos ).end(); ++ iter ) {
        float modelScore = iter->feature_score_except_lm_ + iter->language_model_score_;
        cellForCubePruning.insert( make_pair( modelScore, *iter ) );
      }
    }

    Compose( config, model, cells.cells_.at( startPos ).at( segPos ), cells.cells_.at( segPos ).at( endPos ), \
             startPos, segPos, endPos, cellForCubePruning, sourceSentence );
    SetBeamSize( config, cellForCubePruning, cells.cells_.at( startPos ).at( endPos ) );
  }
  return true;
}


/*
 * $Name: 
 * $Function: 
 * $Date: 2013-05-28
 */
bool Recasing::Compose(const Configuration &config, Model &model, CELL &prevCell, CELL &follCell, const size_t &startPos, \
                       const size_t &segPos, const size_t &endPos, CELLFORCUBEPRUNING &cellForCubePruning, const SourceSentence &sourceSentence) {
  if( config.use_cube_pruning_ ) {
    ComposeWithCubePruning( config, model, prevCell, follCell, startPos, segPos, endPos, cellForCubePruning, sourceSentence );
  } else {
    ComposeNaive( config, model, prevCell, follCell, startPos, segPos, endPos, cellForCubePruning, sourceSentence );
  }
  return true;
}


/*
 * $Name: 
 * $Function: 
 * $Date: 2013-05-28
 */
bool Recasing::ComposeNaive(const Configuration &config, Model &model, CELL &prevCell, CELL &follCell, \
                            const size_t &startPos, const size_t &segPos, const size_t &endPos, \
                            CELLFORCUBEPRUNING &cellForCubePruning, const SourceSentence &sourceSentence) {
  size_t prevNo = 0;
  int    count  = 0;
  for ( CELL::iterator iterPrev  = prevCell.begin(); iterPrev != prevCell.end(); ++ iterPrev ) {
    size_t follNo = 0;
    for ( CELL::iterator iterFoll  = follCell.begin(); iterFoll != follCell.end(); ++ iterFoll ) {
      ExpandHypotheses( config, model, startPos, segPos, endPos, cellForCubePruning, sourceSentence, *iterPrev, *iterFoll, count );
      ++follNo;
      if ( follNo >= config.beamsize_ - 20 ) {
        break;
      }
    }
    ++prevNo;
    if ( prevNo >= config.beamsize_ - 20 ) {
      break;
    }
  }
  return true;
}


/*
* $Name: 
* $Function: 
* $Date: 2013-05-28
*/
bool Recasing::ComposeWithCubePruning(const Configuration &config, Model &model, CELL &prevCell, CELL &follCell, \
                                      const size_t &startPos, const size_t &segPos, const size_t &endPos, \
                                      CELLFORCUBEPRUNING &cellForCubePruning, const SourceSentence &sourceSentence) {
  vector< vector< bool > > explored;
  explored.resize( prevCell.size() );
  for( vector< vector< bool > >::iterator iter  = explored.begin(); iter != explored.end(); ++ iter )
    iter->assign( follCell.size(), false );

  multimap< float, PosInfoForCubePruning > posInfo;

  int count = 0;
  float retScore = ExpandHypotheses( config, model, startPos, segPos, endPos, cellForCubePruning, \
                                     sourceSentence, prevCell.at( 0 ), follCell.at( 0 ), count );

  int posPrev = 0;
  int posFoll = 0;
  explored.at( posPrev ).at( posFoll ) = true;
  PosInfoForCubePruning pos( posPrev, posFoll );
  posInfo.insert( make_pair( retScore, pos ) );

  while ( posInfo.size() > 0 && count < config.beamsize_ ) {
    multimap< float, PosInfoForCubePruning >::iterator iterForPos = -- posInfo.end();

    if ( iterForPos->second.front_position_ + 1 < prevCell.size() && \
         !explored[ iterForPos->second.front_position_ + 1 ][ iterForPos->second.back_position_ ] ) {
      posPrev = iterForPos->second.front_position_ + 1;
      posFoll = iterForPos->second.back_position_;
      retScore = ExpandHypotheses( config, model, startPos, segPos, endPos, cellForCubePruning, \
                                   sourceSentence, prevCell.at( posPrev ), follCell.at( posFoll ), count );

      explored[ posPrev ][ posFoll ] = true;
      pos.front_position_ = posPrev;
      pos.back_position_ = posFoll;
      posInfo.insert( make_pair( retScore, pos ) );
    }

    if( iterForPos->second.back_position_ + 1 < follCell.size() && \
        !explored[ iterForPos->second.front_position_ ][ iterForPos->second.back_position_ + 1 ] ) {
      posPrev = iterForPos->second.front_position_;
      posFoll = iterForPos->second.back_position_ + 1;

      retScore = ExpandHypotheses( config, model, startPos, segPos, endPos, cellForCubePruning, \
                                   sourceSentence, prevCell.at( posPrev ), follCell.at( posFoll ), count );

      explored[ posPrev ][ posFoll ] = true;

      pos.front_position_ = posPrev;
      pos.back_position_ = posFoll;
      posInfo.insert( make_pair( retScore, pos ) );
    }
    posInfo.erase( iterForPos );
  }

  return true;
}


/*
 * $Name: 
 * $Function: 
 * $Date: 2013-05-28
 */
float Recasing::ExpandHypotheses(const Configuration &config, Model &model, const size_t &startPos, const size_t &segPos, \
                                 const size_t &endPos, CELLFORCUBEPRUNING &cellForCubePruning, const SourceSentence &sourceSentence, \
                                 const Hypothesis &prevHypo, const Hypothesis &follHypo, int &count ) {
  float  featureScoreExceptLM = prevHypo.feature_score_except_lm_ + follHypo.feature_score_except_lm_;
  string tgtOption            = prevHypo.translation_result_ + " " + follHypo.translation_result_; 
  bool composeFlag = true;
  float languageModelScore = 0;
  int   tgtWordCount       = 0;
  Hypothesis hy( featureScoreExceptLM, languageModelScore, tgtOption, composeFlag, tgtWordCount );
  BuildNewHypothesis( prevHypo, follHypo, hy );
  CalculateLMScoreForComposing( config, model, prevHypo, follHypo, hy );
  float modelScore = hy.language_model_score_ + hy.feature_score_except_lm_;
  cellForCubePruning.insert( make_pair( modelScore, hy ) );
  ++count;
  return modelScore;
}


/*
 * $Name: 
 * $Function: 
 * $Date: 2013-05-28
 */
bool Recasing::BuildNewHypothesis(const Hypothesis &hyPrev, const Hypothesis &hyFoll, Hypothesis &hy) {
  hy.target_word_count_ = hyPrev.target_word_count_ + hyFoll.target_word_count_;

  hy.target_word_id_.reserve( hy.target_word_count_ );
  hy.target_word_id_.assign( hyPrev.target_word_id_.begin(), hyPrev.target_word_id_.end() );
  for ( CONINTIT iter = hyFoll.target_word_id_.begin(); iter != hyFoll.target_word_id_.end(); ++ iter ) {
    hy.target_word_id_.push_back( *iter );
  }

  hy.target_word_.reserve( hy.target_word_count_ );
  hy.target_word_.assign( hyPrev.target_word_.begin(), hyPrev.target_word_.end() );
  for ( CONSTRIT iter = hyFoll.target_word_.begin(); iter != hyFoll.target_word_.end(); ++ iter ) {
    hy.target_word_.push_back( *iter );
  }

  hy.target_n_gram_lm_score_.reserve( hyPrev.target_n_gram_lm_score_.size() + hyFoll.target_n_gram_lm_score_.size() );
  hy.target_n_gram_lm_score_.assign( hyPrev.target_n_gram_lm_score_.begin(), hyPrev.target_n_gram_lm_score_.end() );
  for ( CONFLOIT iter = hyFoll.target_n_gram_lm_score_.begin(); iter != hyFoll.target_n_gram_lm_score_.end(); ++ iter ) {
    hy.target_n_gram_lm_score_.push_back( *iter );
  }

  return true;
}


/*
 * $Name: 
 * $Function: 
 * $Date: 2013-05-28
 */
bool Recasing::InitCells(Configuration &config, Model &model, Cells &cells, SourceSentence &sourceSentence) {
  for ( size_t phraseLength = 1; phraseLength <= config.max_phrase_length_; ++ phraseLength ) {
    for (size_t startPos = 0; ( startPos < sourceSentence.cell_span_.size() ) && \
         (startPos + phraseLength <= sourceSentence.cell_span_.size()); ++ startPos) {
      size_t endPos = startPos + phraseLength;
      string srcPhrase;
      for ( size_t i = startPos; i < endPos; ++i ) {
        srcPhrase += sourceSentence.cell_span_.at( i ) + " ";
      }

      RmEndSpace( srcPhrase );
      map< string, TranslationOptions >::iterator iter = model.phrase_table_.translationsOptions.find( srcPhrase );

      if ( iter != model.phrase_table_.translationsOptions.end() ) {
        CELLFORCUBEPRUNING hypotheses;
        for ( vector< TranslationOption >::iterator iterForTransOption = iter->second.translationOptions.begin(); \
              iterForTransOption != iter->second.translationOptions.end(); ++iterForTransOption ) {
          float proEGivenF       = config.features.feature_values_.at( 2 ).weight_ * iterForTransOption->scoresOfTranslateOption.prob_egivenf_     ;    // Pr(e|f)
          float lexEGivenF       = config.features.feature_values_.at( 3 ).weight_ * iterForTransOption->scoresOfTranslateOption.lexi_egivenf_     ;    // Lex(e|f)
          float proFGivenE       = config.features.feature_values_.at( 4 ).weight_ * iterForTransOption->scoresOfTranslateOption.prob_fgivene_     ;    // Pr(f|e)
          float lexFGivenE       = config.features.feature_values_.at( 5 ).weight_ * iterForTransOption->scoresOfTranslateOption.lexi_fgivene_     ;    // Lex(f|e)
          float naturalLogarithm = config.features.feature_values_.at( 6 ).weight_ * iterForTransOption->scoresOfTranslateOption.natural_logarithm_;

          float featureScoreExceptLM = proEGivenF + lexEGivenF + proFGivenE + lexFGivenE + naturalLogarithm;
          bool  composeFlag = false;

          float languageModelScore = 0;
          int   tgtWordCount       = 0;
          Hypothesis hy( featureScoreExceptLM , languageModelScore, iterForTransOption->tgtOption, composeFlag, tgtWordCount );

          CalculateLMScore( config, model, hy );

          // number of target-words
          hy.feature_score_except_lm_ += config.features.feature_values_.at( 1 ).weight_ * hy.target_word_count_;
          float modelScore = hy.feature_score_except_lm_ + hy.language_model_score_;
          hypotheses.insert( make_pair( modelScore, hy ) );
        }
        for ( CELLFORCUBEPRUNING::reverse_iterator riter  = hypotheses.rbegin(); riter != hypotheses.rend(); ++ riter ) {
          cells.cells_.at( startPos ).at( endPos ).push_back( riter->second );
        }
      } else {
        if ( phraseLength == 1 && cells.cells_.at( startPos ).at( endPos ).size() == 0 ) {
          if ( startPos == 0 || endPos == sourceSentence.cell_span_.size() ) {
            continue;
          }

          float  score = 0;
          string translationResult = sourceSentence.cell_span_.at( startPos );
          bool   composeFlag  = false;
          int    tgtWordCount = 1;
          Hypothesis hy( score, score, translationResult, composeFlag, tgtWordCount );
          CalculateLMScore( config, model, hy );
          hy.language_model_score_ = 0.0;
          cells.cells_.at( startPos ).at( endPos ).push_back( hy );
        } 
      }
    }
  }
  return true;
}


/*
 * $Name: 
 * $Function: 
 * $Date: 2013-05-28
 */
bool Recasing::CalculateLMScore(Configuration &config, Model &model , Hypothesis &hy) {
  Split( hy.translation_result_, ' ', hy.target_word_ );
  hy.target_word_count_ = ( int )hy.target_word_.size();

  int pos = 0;
  for ( vector< string >::iterator iter  = hy.target_word_.begin(); iter != hy.target_word_.end(); ++ iter ) {
    int wid = 0;
    if( model.phrase_table_.tgtVocab.find( *iter ) != model.phrase_table_.tgtVocab.end() ) {
      wid = model.phrase_table_.tgtVocab[ *iter ];
    } else {
      wid = model.phrase_table_.tgtVocab[ "<unk>" ];
    }

    hy.target_word_id_.push_back( wid );

    int end    = pos + 1;
    int begin  = end - config.ngram_ > 0 ? end - config.ngram_ : 0;
    float prob = model.ngram_language_model_.GetProb( hy.target_word_id_, begin, end );

#ifdef WIN32
    if ( prob < -20 ) {
#else
    if ( prob < -20 || isnan( prob ) ) {
#endif
      prob = -20;
    }
    ++pos;
    hy.target_n_gram_lm_score_.push_back( prob );
  }

  hy.language_model_score_ = 0;
  for ( vector< float >::iterator iter = hy.target_n_gram_lm_score_.begin(); iter != hy.target_n_gram_lm_score_.end(); ++ iter ) {
    hy.language_model_score_ += *iter;
  }

  hy.language_model_score_ *= config.features.feature_values_.at( 0 ).weight_;
  return true;
}


/*
 * $Name: 
 * $Function: 
 * $Date: 2013-05-28
 */
bool Recasing::CalculateLMScoreForComposing(const Configuration &config, Model &model, const Hypothesis &hyPrev, const Hypothesis &hyFoll, Hypothesis &hy) {
  if ( hyPrev.target_word_count_ != 0 && hyPrev.target_word_count_ != 0 ) {
    for ( int end = hyPrev.target_word_count_; ( end - hyPrev.target_word_count_ < config.ngram_ - 1 ) && end < hy.target_word_count_; ++ end ) {
      int tmpEnd = end + 1;
      int begin  = tmpEnd - config.ngram_ > 0 ? tmpEnd - config.ngram_ : 0;
      hy.target_n_gram_lm_score_.at( end ) = model.ngram_language_model_.GetProb( hy.target_word_id_, begin, tmpEnd );
#ifdef WIN32
      if ( hy.target_n_gram_lm_score_.at( end ) < -20 ) {
#else
      if ( hy.target_n_gram_lm_score_.at( end ) < -20 || isnan( hy.target_n_gram_lm_score_.at( end ) ) ) {
#endif
        hy.target_n_gram_lm_score_.at( end ) = -20;
      }
    }
  }

  hy.language_model_score_ = 0;
  for ( vector< float >::iterator iter  = hy.target_n_gram_lm_score_.begin(); iter != hy.target_n_gram_lm_score_.end(); ++ iter ) {
    hy.language_model_score_ += *iter;
  }

  hy.language_model_score_ *= config.features.feature_values_.at( 0 ).weight_;
  return true;
}


/*
 * $Name: 
 * $Function: 
 * $Date: 2013-05-28
 */
bool Recasing::SetBeamSize(const Configuration &config, CELLFORCUBEPRUNING &cellForCubePruning, CELL &cell) {
  if ( cell.size() != 0 ) {
    cell.clear();
  }
  size_t count = 0;    
  set< string > deduplicate;
  cell.reserve( config.beamsize_ );
  for ( CELLFORCUBEPRUNING::reverse_iterator riter = cellForCubePruning.rbegin(); riter != cellForCubePruning.rend(); ++ riter ) {
    if ( deduplicate.count( riter->second.translation_result_ ) != 0 ) {
      continue;
    }
    ++ count;
    deduplicate.insert( riter->second.translation_result_ );        
    cell.push_back( riter->second );
    if ( count >= config.beamsize_ ) {
                break;
    }
  }
  return true;
}


/*
 * $Name: 
 * $Function: 
 * $Date: 2013-05-28
 */
bool Recasing::CheckForPunctPruning(const Configuration &config, const size_t &startPos, const size_t &endPos, const SourceSentence &sourceSentence) {
  if ( !config.use_punct_pruning_ ) {
    return true;
  }

  int tmpStartPos = ( int )startPos + 1;
  int tmpEndPos   = ( int )endPos - 1;

  int boundaryWordNum = 2;

  if ( tmpEndPos > tmpStartPos && HavePunct( tmpStartPos, tmpEndPos, sourceSentence ) ) {
    bool matchLeft  = false;
    bool matchRight = false;

    for ( int i = 0; i < boundaryWordNum && ( int )startPos - i >= 0; ++i ) {
      if ( sourceSentence.punctuation_position_.find( ( int )startPos - i ) != sourceSentence.punctuation_position_.end() ) {
        matchLeft = true;
        break;
      }
    }

    for ( int i = 0; i < boundaryWordNum && ( int )endPos + i - 1 <= sourceSentence.cell_span_.size(); ++i ) {
      if ( sourceSentence.punctuation_position_.find( ( int )endPos + i - 1 ) != sourceSentence.punctuation_position_.end() ) {
        matchRight = true;
        break;
      }
    }

    matchLeft = ( matchLeft || startPos <= 1 );
    matchRight = ( matchRight || endPos >= sourceSentence.cell_span_.size() - 1 );
    return matchLeft && matchRight;
  }
  return true;
}


/*
 * $Name: 
 * $Function: 
 * $Date: 2013-05-28
 */
bool Recasing::HavePunct(const int &startPos, const int &endPos, const SourceSentence &sourceSentence) {
  for ( int i = startPos; i != endPos; ++ i ) {
    if( sourceSentence.punctuation_position_.find( i ) != sourceSentence.punctuation_position_.end() ) {
      return true;
    }
  }
  return false;
}


/*
 * $Name: 
 * $Function: 
 * $Date: 2013-05-28
 */
bool Recasing::GenerateTranslationResult(const Configuration &config, const Cells &cells, \
                                         const SourceSentence &sourceSentence, string &output) {
  size_t counter = 0;
  for (CELL::const_iterator riter = cells.cells_.at( 0 ).at( sourceSentence.cell_span_.size() ).begin(); \
       riter != cells.cells_.at( 0 ).at( sourceSentence.cell_span_.size() ).end(); ++ riter ) {
    ++counter;
    if ( counter == 1 ) {
      if ( riter->translation_result_.size() > 8 ) {
        output = riter->translation_result_.substr( 4, riter->translation_result_.length() - 9 );
      }
      UppercaseSentenceBegin( output );
    } else {
      break;
    }
  }
  return true;
}


/*
 * $Name: 
 * $Function: 
 * $Date: 2013-05-28
 */
bool Recasing::OutputTranslationResult(const Configuration &config, const Cells &cells, const SourceSentence &sourceSentence, \
                              ofstream &outFile, ofstream &logFile) {
  WriteOutputFile( config, cells, sourceSentence, outFile );
  return true;
}


/*
 * $Name: 
 * $Function: 
 * $Date: 2013-05-28
 */
bool Recasing::WriteOutputFile(const Configuration &config, const Cells &cells, \
                               const SourceSentence &sourceSentence, ofstream &outFile) {
  // write output file   
  size_t counter = 0;
  for ( CELL::const_iterator riter = cells.cells_.at( 0 ).at( sourceSentence.cell_span_.size() ).begin(); \
        riter != cells.cells_.at( 0 ).at( sourceSentence.cell_span_.size() ).end(); ++ riter ) {
    ++counter;
    if ( counter <= config.nbest_ ) {
      string tmpTranslationResult;
      if ( riter->translation_result_.size() > 8 ) {
        tmpTranslationResult = riter->translation_result_.substr( 4, riter->translation_result_.length() - 9 );
      }
      UppercaseSentenceBegin( tmpTranslationResult );
      outFile<<tmpTranslationResult;
      if ( config.nbest_ > 1 ) {
        outFile<<" |||| "<<riter->feature_score_except_lm_ + riter->language_model_score_<<"\n";
      } else {
        outFile<<"\n";
      }
    } else {
      break;
    }
  }

  if ( counter == 0 ) {
    outFile<<"\n";
  }
  if ( config.nbest_ > 1 ) {
    outFile<<"===============================\n";
  }
  return true;
}


/*
 * $Name: 
 * $Function: 
 * $Date: 2013-05-28
 */
bool Recasing::UppercaseSentenceBegin(string &str) {
  bool sentBeg = true;
  bool lastIsAlpha = false;
  for ( string::size_type i = 0; i < str.size(); ++ i ) {
    if ( sentBeg && !lastIsAlpha && islower( ( unsigned char )str.at( i ) ) ) {
      str.at( i ) = toupper( ( unsigned char )str.at( i ) );
      sentBeg = false;
      lastIsAlpha = true;
      continue;
    } else if ( sentBeg && !lastIsAlpha && isalpha( ( unsigned char )str.at( i ) ) ) {
      sentBeg = false;
      lastIsAlpha = true;
      continue;
    }

    if ( ( str.at( i ) == '.' || str.at( i ) == '?' || str.at( i ) == '!' ) && ( i + 1 ) < str.size() && str.at( i + 1 ) == ' ' ) {
      sentBeg = true;
      lastIsAlpha = false;
      continue;
    }
            
    if ( str.at( i ) != ' ' ) {
      if( isalpha( ( unsigned char )str.at( i ) ) ) {
        lastIsAlpha = true;
      } else {
        lastIsAlpha = false;
      }
    }
  }
  return true;
}


/*
 * $Name: 
 * $Function: 
 * $Date: 2013-05-28
 */
bool Recasing::PrintRecasingLogo() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# Recasing for Statistical Machine Translation                    #\n"
      <<"#                                             Version 0.0.1       #\n"
      <<"#                                             NEUNLPLab/YAYI corp #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<flush;
  return true;
}


}

