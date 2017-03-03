/*
 * $Id:
 * 0010
 *
 * $File:
 * phrasebased_decoder.cpp
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
 * 2013-05-07,15:38
 * 2013-04-23,10:27
 * 2013-03-06,15:04
 * 2013-03-01,15:28
 * 2013-02-28,15:45
 * 2013-01-08,15:29
 * 2012-12-04,19:20
*/

#include "phrasebased_decoder.h"

namespace decoder_phrasebased_decoder {

/*
 * $Name: InterfaceForTranslationMemory
 * $Function: Initializes the test set for decoder.
 * $Date: 2013-05-08
 */
bool PhraseBasedITGDecoder::InterfaceForTranslationMemory(Configuration &config, Model &model, \
                                                          string &translation_memory_output, string &decoder_output) {
  if( translation_memory_output == "" ) {
    decoder_output = "";
    return false;
  }

  vector< string > clauses;
  string tmp_output;
  SplitWithStr( translation_memory_output, " |||||| ", clauses );

  for ( vector< string >::iterator iter = clauses.begin(); iter != clauses.end(); ++iter ) {
//    cerr<<*iter<<"\n";
    vector< string > translation_flag_and_sentence;
    SplitWithStr( *iter, " ||||| ", translation_flag_and_sentence );
    if ( translation_flag_and_sentence.size() != 2 ) {
      cerr<<"Warning: Format Error!\n"
          <<"Input  : "<<*iter<<"\n";
      continue;
    }
//    cerr<<"  FLAG="<<translation_flag_and_sentence.at( 0 )<<"  INPUT="<<translation_flag_and_sentence.at( 1 )<<"\n";
    if ( translation_flag_and_sentence.at( 0 ) == "0" ) {
      if( config.english_string_ ) {
        TestSet test_set;
        test_set.ClearIllegalChar( translation_flag_and_sentence.at( 1 ) );
        test_set.RmEndSpace      ( translation_flag_and_sentence.at( 1 ) );
        test_set.RmStartSpace    ( translation_flag_and_sentence.at( 1 ) );
        
#ifdef SUPPORT_ONLINE_SERVICE_CE_
        test_set.RecognizeAndTranslateEnString( translation_flag_and_sentence.at( 1 ) );
#endif
//        cerr<<"[DEINPUT]  "<<translation_flag_and_sentence.at( 1 )<<"\n";
      }

      SourceSentence source_sentence;
      source_sentence.Init(translation_flag_and_sentence.at( 1 ), model.me_reorder_table_);

      Decode(config, model, source_sentence, tmp_output);
      decoder_output += " " + tmp_output;
      tmp_output = "";
    } else if ( translation_flag_and_sentence.at( 0 ) == "1" ) {
      decoder_output += " " + translation_flag_and_sentence.at( 1 );
    }
  }
  return true;
}




/*
 * $Name: Init
 * $Function: Initializes the test set for decoder.
 * $Date: 2013-05-08
 */
bool PhraseBasedITGDecoder::Init(Configuration &config, Model &model) {
  if (config.use_context_sensitive_wd_) {
    test_set_.Init (config, model.me_reorder_table_, model.context_sensitive_wd_);
  } else {
    test_set_.Init (config, model.me_reorder_table_);
  }

  return true;
}


/*
 * $Name: Decode
 * $Function: Offline translation services interface.
 *            Translates inputted sentences for offline translation services. 
 * $Date: 2013-05-08
 */
bool PhraseBasedITGDecoder::Decode (Configuration &config, Model &model) {
  clock_t start, finish;
  start = clock();
  cerr<<"Start decoding with CYK Algorithm\n"<<flush;
  size_t lineNo = 0;

  ofstream out_file (config.output_file_.c_str());
  if (!out_file) {
    cerr<<"ERROR: Can not open file \""
        <<config.output_file_
        <<"\".\n"
        <<flush;
    exit( 1 );
  }

  ofstream log_file (config.log_file_.c_str());
  if (!log_file) {
    cerr<<"ERROR: Can not open file \""
        <<config.log_file_
        <<"\".\n"
        <<flush;
    exit(1);
  }

  for (size_t sentID = 0; sentID < test_set_.test_set_.size(); ++sentID) {
    ++lineNo;
    Cells cells;

    AllocateSpaceForCells (cells, test_set_.test_set_.at(sentID));
    Decode (config, model, cells, test_set_.test_set_.at(sentID), out_file, log_file);
    if (lineNo % 1 == 0) {
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
  log_file.close();
  out_file.close();

  if (config.mert_flag_) {
    WriteMertConfig (config);
    WriteMertTmp (config, test_set_.test_set_);
//    WriteMertTmp ();
  }

  return true;
}


/*
 * $Name: Decode
 * $Function: Online translation service interface. 
 *            Translates inputted sentences for online translation services. 
 * $Date: 2013-05-08
 */
bool PhraseBasedITGDecoder::Decode(Configuration  &config , Model &model, SourceSentence &source_sentence, string &output) {
  Cells cells;
  AllocateSpaceForCells( cells, source_sentence );
  Decode( config, model, cells, source_sentence, output );
  return true;
}


/*
 * $Name: Decode
 * $Function: Translates inputted sentences for offline translation services.
 * $Date: 2013-05-08
 */
bool PhraseBasedITGDecoder::Decode(Configuration &config, Model &model, Cells &cells , SourceSentence &source_sentence, ofstream &outFile, ofstream &logFile) {
  InitCellsWithGeneralization( config, model, cells, source_sentence );
  InitCells( config, model, cells, source_sentence );
  ClearCellsWithForcedType( config, model, cells, source_sentence );
  DecodeWithCYKAlgorithm( config, model, cells, source_sentence );
  OutputTranslationResults( config, cells , source_sentence, outFile, logFile );
  return true;
}


/*
 * $Name: Decode
 * $Function: Translates inputted sentences for online translation services.
 * $Date: 2013-05-08
 */
bool PhraseBasedITGDecoder::Decode (Configuration &config, Model &model, Cells &cells, SourceSentence &source_sentence, string &output) {
  InitCellsWithGeneralization (config, model, cells, source_sentence);
  InitCells (config, model, cells, source_sentence);
  ClearCellsWithForcedType( config, model, cells, source_sentence );
  DecodeWithCYKAlgorithm( config, model, cells, source_sentence );
  GenerateTranslationResults( config, cells, source_sentence, output );
#ifdef __DEBUG_ONLINESERVICE_LOG__
  ofstream tmpOutputFile( "tmp.output.txt", ios::app );
  ofstream tmpLogFile( "tmp.log.txt", ios::app );
  OutputTranslationResults( config, cells, source_sentence, tmpOutputFile, tmpLogFile );
  tmpOutputFile.close();
  tmpLogFile.close();
#endif

  return true;
}


/*
 * $Name: DecodeWithCYKAlgorithm
 * $Function: The CYK algorithm is selected to translate.
 * $Date: 2013-05-08
 */
bool PhraseBasedITGDecoder::DecodeWithCYKAlgorithm (Configuration &config, Model &model, Cells &cells, SourceSentence &source_sentence) {
  size_t currentSentLength = source_sentence.cell_span_.size();
  for (size_t phraseLength = 2; phraseLength <= currentSentLength; ++ phraseLength) {
    for (size_t startPos = 0; (startPos < currentSentLength) && (startPos + phraseLength <= currentSentLength); ++ startPos) {
      size_t endPos = startPos + phraseLength;
      CELLFORCUBEPRUNING cellForCubePruning;
      if (cells.cells.at(startPos).at(endPos).size() != 0) {
        for (CELL::iterator iter = cells.cells.at(startPos).at(endPos).begin(); iter != cells.cells.at(startPos).at(endPos).end(); ++ iter) {
          float modelScore = iter->feature_score_except_lm_ + iter->language_model_score_;
          cellForCubePruning.insert(make_pair(modelScore, *iter));
        }
      }
      if (CheckForPunctPruning (config, startPos, endPos, source_sentence)) {
        if (config.use_cube_pruning_ || !config.use_cube_pruninginc_) {
          for (size_t segPos = startPos + 1; segPos < endPos; ++ segPos) {
            bool swapFlag = CanBeReversed (config, startPos, segPos, endPos, currentSentLength, source_sentence);
            if (cells.cells.at(startPos).at(segPos).size() == 0 || cells.cells.at(segPos).at(endPos).size() == 0){
              continue;
            }
            Compose (config, model, cells.cells.at(startPos).at(segPos), cells.cells.at(segPos).at(endPos), startPos, segPos, endPos, cellForCubePruning, swapFlag, source_sentence);
          }
        } else if (config.use_cube_pruninginc_) {
          ComposeWithCubePruningUpgrade (config, model, cells, startPos, endPos, cellForCubePruning, source_sentence);
        }
        SetBeamSize (config, cellForCubePruning, cells.cells.at(startPos).at(endPos));
      }
    }
  }
  return true;
}


/*
 * $Name:
 * $Function:
 * $Date:
 */
bool PhraseBasedITGDecoder::CheckForPunctPruning(const Configuration &config, const size_t &start_position, \
                                                 const size_t &end_position, const SourceSentence &source_sentence) {
  if ( !config.use_punct_pruning_ ) {
    return true;
  }

  int tmpStartPos = ( int )start_position + 1;
  int tmpEndPos   = ( int )end_position   - 1;
  int boundaryWordNum = 2;
  if ( tmpEndPos > tmpStartPos && HavePunct( tmpStartPos, tmpEndPos, source_sentence ) ) {
    bool matchLeft  = false;
    bool matchRight = false;
    for ( int i = 0; i < boundaryWordNum && ( int )start_position - i >= 0; ++i ) {
      if ( source_sentence.punctuation_position_.find( ( int )start_position - i ) != source_sentence.punctuation_position_.end() ) {
        matchLeft = true;
        break;
      }
    }
    for ( int i = 0; i < boundaryWordNum && ( int )end_position + i - 1 <= source_sentence.cell_span_.size(); ++i ) {
      if ( source_sentence.punctuation_position_.find( ( int )end_position + i - 1 ) != source_sentence.punctuation_position_.end() ) {
        matchRight = true;
        break;
      }
    }

    matchLeft  = ( matchLeft  || start_position <= 1                                  );
    matchRight = ( matchRight || end_position   >= source_sentence.cell_span_.size() - 1 );
    return matchLeft && matchRight;
  }
  return true;
}


/*
 * $Name:
 * $Function:
 * $Date:
 */
bool PhraseBasedITGDecoder::HavePunct(const int &start_position, const int &end_position, const SourceSentence &source_sentence) {
  for ( int i = start_position; i != end_position; ++ i ) {
    if ( source_sentence.punctuation_position_.find( i ) != source_sentence.punctuation_position_.end() ) {
      return true;
    }
  }
  return false;
}


/*
 * $Name:
 * $Function:
 * $Date:
 */
bool PhraseBasedITGDecoder::CanBeReversed(const Configuration &config, const size_t &start_position, const size_t &split_position, \
                                          const size_t &end_position, const size_t &current_sent_length, const SourceSentence &source_sentence) {
  /*    
  if( ( int )( split_position - start_position ) > ( int )optionsOfPhraseBasedITGDecoder.maxdd ||
      ( int )( end_position - split_position   ) > ( int )optionsOfPhraseBasedITGDecoder.maxdd    )
            return false;
  */
  if ( ( int )( end_position - start_position ) > config.max_reordering_distance_ ) {
    return false;
  }

  if ( start_position == 0 || end_position == current_sent_length ) {
    return false;
  }

  // For Chinese-to-English SMT task, punctuations are useful marks that strongly recommend not to reorder sub-hypothses.
  if ( HavePunct( ( int )start_position, ( int )end_position, source_sentence ) ) {
    return false;
  }

  return true;
}


/*
 * $Name: SetBeamSize
 * $Function: Sets beamsize for cells.
 * $Date: 2013-05-08
 */
bool PhraseBasedITGDecoder::SetBeamSize(const Configuration &config, CELLFORCUBEPRUNING &cellForCubePruning, CELL &cell) {
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
 * $Name: Compose
 * $Function: Composes each pair of neighbor sub-cells with in cell[j_1,j_2] using the monotonic
 *            or inverted translation rule to generate the derivations in cell[j_1,j_2].
 * $Date: 2013-05-08
 */
bool PhraseBasedITGDecoder::Compose (const Configuration &config, Model &model, CELL &front_cell, CELL &back_cell, \
                                     const size_t &start_position, const size_t &split_position, const size_t &end_position, \
                                     CELLFORCUBEPRUNING &cell_for_cube_pruning, const bool &swap_flag, \
                                     const SourceSentence &source_sentence) {
  if (config.use_cube_pruning_) {
    ComposeWithCubePruning (config, model, front_cell, back_cell, start_position, split_position, end_position, \
                            cell_for_cube_pruning, swap_flag, source_sentence);
  } else {
    ComposeNaive (config, model, front_cell, back_cell, start_position, split_position, end_position, \
                  cell_for_cube_pruning, swap_flag, source_sentence);
  }
  return true;
}


/*
 * $Name: ComposeNaive
 * $Function: A naive implementation of composing.
 * $Date: 2013-05-08
 */
bool PhraseBasedITGDecoder::ComposeNaive(const Configuration &config, Model &model, CELL &front_cell, CELL &back_cell, \
                                         const size_t &start_position, const size_t &split_position, const size_t &end_position, \
                                         CELLFORCUBEPRUNING &cell_for_cube_pruning, const bool &swap_flag, \
                                         const SourceSentence &source_sentence) {
  size_t front_number = 0;
  int    count  = 0;

  for ( CELL::iterator front_iter = front_cell.begin(); front_iter != front_cell.end(); ++ front_iter ) {
    size_t back_number = 0;
    for ( CELL::iterator back_iter = back_cell.begin(); back_iter != back_cell.end(); ++ back_iter ) {
      ExpandHypotheses( config, model, start_position, split_position, end_position, \
                        cell_for_cube_pruning, swap_flag, source_sentence, \
                        *front_iter, *back_iter, count );
      ++back_number;
      if ( back_number >= config.beamsize_ - 20 ) {
        break;
      }
    }
    ++front_number;
    if ( front_number >= config.beamsize_ - 20 ) {
      break;
    }
  }
  return true;
}


/*
 * $Name: ComposeWithCubePruning
 * $Function: When composing each pair of neighbor sub-cells, we use cube pruning method.
 * $Date: 2013-05-08
 */
bool PhraseBasedITGDecoder::ComposeWithCubePruning (const Configuration &config, Model &model, CELL &front_cell, CELL &back_cell, \
                                                    const size_t &start_position, const size_t &split_position, const size_t &end_position, \
                                                    CELLFORCUBEPRUNING &cell_for_cube_pruning, const bool &swap_flag, \
                                                    const SourceSentence &source_sentence) {
  vector<vector<bool> > explored;
  explored.resize(front_cell.size());
  for (vector<vector<bool> >::iterator iter  = explored.begin(); iter != explored.end(); ++ iter) {
    iter->assign(back_cell.size(), false);
  }

  multimap<float, PosInfoForCubePruning> posInfo;

  int count = 0;
  float retScore = ExpandHypotheses (config, model, start_position, split_position, end_position, cell_for_cube_pruning, \
                                     swap_flag, source_sentence, front_cell.at(0), back_cell.at(0), count);

  int posPrev = 0;
  int posFoll = 0;
  explored.at(posPrev).at(posFoll) = true;
  PosInfoForCubePruning pos(posPrev, posFoll);
  posInfo.insert(make_pair(retScore, pos));

  while (posInfo.size() > 0 && count < config.beamsize_) {
    multimap<float, PosInfoForCubePruning>::iterator iterForPos = -- posInfo.end();
    if (iterForPos->second.front_position_ + 1 < front_cell.size() && \
        !explored[iterForPos->second.front_position_ + 1][iterForPos->second.back_position_]) {
      posPrev = iterForPos->second.front_position_ + 1;
      posFoll = iterForPos->second.back_position_;
      retScore = ExpandHypotheses (config, model, start_position, split_position, end_position, cell_for_cube_pruning, \
                                   swap_flag, source_sentence, front_cell.at(posPrev), back_cell.at(posFoll), count);
      explored[posPrev][posFoll] = true;
      pos.front_position_ = posPrev;
      pos.back_position_ = posFoll;
      posInfo.insert(make_pair(retScore, pos));
    }

    if (iterForPos->second.back_position_ + 1 < back_cell.size() && \
        !explored[iterForPos->second.front_position_][iterForPos->second.back_position_ + 1]) {
      posPrev = iterForPos->second.front_position_;
      posFoll = iterForPos->second.back_position_ + 1;
      retScore = ExpandHypotheses (config, model, start_position, split_position, end_position, cell_for_cube_pruning, \
                                   swap_flag, source_sentence, front_cell.at( posPrev ), back_cell.at( posFoll ), count);
      explored[posPrev][posFoll] = true;
      pos.front_position_ = posPrev;
      pos.back_position_ = posFoll;
      posInfo.insert(make_pair(retScore, pos));
    }
    posInfo.erase(iterForPos);
  }
  return true;
}


/*
 * $Name: ComposeWithCubePruningInc
 * $Function: When composing each pair of neighbor sub-cells, we use the upgrade version of cube pruning method.
 * $Date: 2013-05-08
 */
bool PhraseBasedITGDecoder::ComposeWithCubePruningUpgrade(const Configuration &config, Model &model, const Cells &cells, \
                                                          const size_t &start_position, const size_t &end_position, \
                                                          CELLFORCUBEPRUNING &cell_for_cube_pruning, const SourceSentence &source_sentence) {
  vector< vector< vector< bool > > > explored;
  explored.resize( end_position - start_position - 1 );
  size_t segPos = start_position + 1;
  for ( vector< vector< vector< bool > > >::iterator iter = explored.begin(); iter != explored.end(); ++ iter ) {
    iter->resize( cells.cells.at( start_position ).at( segPos ).size() );
    for ( vector< vector< bool > >::iterator iter_in = iter->begin(); iter_in != iter->end(); ++iter_in ) {
      iter_in->assign( cells.cells.at( segPos ).at( end_position ).size(), false );
    }
    ++segPos;
  }

  multimap< float, PosInfoForCubePruningInc > posInfo;
  int currentSentLength = ( int )source_sentence.cell_span_.size();
  vector< bool > swapFlag;
  for ( size_t segPos = start_position + 1; segPos < end_position; ++segPos ) {
    bool tmpSwapFlag = CanBeReversed( config, start_position, segPos, end_position, currentSentLength, source_sentence );
    swapFlag.push_back( tmpSwapFlag );
  }

  int count = 0;
  for ( size_t segPos = start_position + 1; segPos < end_position; ++segPos ) {
    if ( cells.cells.at( start_position ).at( segPos ).size() == 0 || \
         cells.cells.at( segPos ).at( end_position ).size() == 0 ) {
        continue;
    }

    float retScore = ExpandHypotheses( config, model, start_position, segPos, end_position, cell_for_cube_pruning, \
                                       swapFlag.at( segPos - start_position - 1 ), source_sentence, \
                                       cells.cells.at( start_position ).at( segPos ).at( 0 ), \
                                       cells.cells.at( segPos ).at( end_position ).at( 0 ), count );
    int posPrev = 0;
    int posFoll = 0;
    explored.at( segPos - start_position - 1 ).at( posPrev ).at( posFoll ) = true;
    PosInfoForCubePruningInc pos( ( int )segPos, posPrev, posFoll );
    posInfo.insert( make_pair( retScore, pos ) ); 

    if ( posPrev + 1 < cells.cells.at( start_position ).at( segPos ).size() ) {
      retScore = ExpandHypotheses( config, model, start_position, segPos, end_position, cell_for_cube_pruning, \
                                   swapFlag.at( segPos - start_position - 1 ), source_sentence, \
                                   cells.cells.at( start_position ).at( segPos ).at( 1 ), \
                                   cells.cells.at( segPos ).at( end_position ).at( 0 ), count );
      explored.at( segPos - start_position - 1 ).at( 1 ).at( 0 ) = true;
      pos.front_position_ = 1;
      pos.back_position_ = 0;
      posInfo.insert( make_pair( retScore, pos ) );
    }
    if ( posFoll + 1 < cells.cells.at( segPos ).at( end_position ).size() ) {
      retScore = ExpandHypotheses( config, model, start_position, segPos, end_position, cell_for_cube_pruning, \
                                   swapFlag.at( segPos - start_position - 1 ), source_sentence, \
                                   cells.cells.at( start_position ).at( segPos ).at( 0 ), \
                                   cells.cells.at( segPos ).at( end_position ).at( 1 ), count );
      explored.at( segPos - start_position - 1 ).at( 0 ).at( 1 ) = true;
      pos.front_position_ = 0;
      pos.back_position_ = 1;
      posInfo.insert( make_pair( retScore, pos ) );
    }
  }

  float retScore = 0;
  int   posPrev  = 0;
  int   posFoll  = 0;
  PosInfoForCubePruningInc pos;
  count = 0;

  while ( posInfo.size() > 0 && count < config.beamsize_ ) {
    multimap< float, PosInfoForCubePruningInc >::iterator iterForPos = -- posInfo.end();

    if ( iterForPos->second.front_position_ + 1 < cells.cells.at( start_position ).at( iterForPos->second.split_position_ ).size() && \
         !explored[ iterForPos->second.split_position_ - start_position - 1 ][ iterForPos->second.front_position_ + 1 ][ iterForPos->second.back_position_ ] ) {
      posPrev = iterForPos->second.front_position_ + 1;
      posFoll = iterForPos->second.back_position_    ;
      retScore = ExpandHypotheses( config, model, start_position, iterForPos->second.split_position_, end_position, cell_for_cube_pruning, \
                             swapFlag.at( iterForPos->second.split_position_ - start_position - 1 ), source_sentence, \
                             cells.cells.at( start_position ).at( iterForPos->second.split_position_ ).at( posPrev ), \
                             cells.cells.at( iterForPos->second.split_position_ ).at( end_position ).at( posFoll ), count );
      explored.at( iterForPos->second.split_position_ - start_position - 1 ).at( posPrev ).at( posFoll ) = true;
      pos.split_position_ = iterForPos->second.split_position_;
      pos.front_position_ = posPrev;
      pos.back_position_ = posFoll;
      posInfo.insert( make_pair( retScore, pos ) );
    }

    if ( iterForPos->second.back_position_ + 1 < cells.cells.at( iterForPos->second.split_position_ ).at( end_position ).size() && \
         !explored[ iterForPos->second.split_position_ - start_position - 1 ][ iterForPos->second.front_position_ ][ iterForPos->second.back_position_ + 1 ] ) {
      posPrev = iterForPos->second.front_position_;
      posFoll = iterForPos->second.back_position_ + 1;
      retScore = ExpandHypotheses( config, model, start_position, iterForPos->second.split_position_, end_position, cell_for_cube_pruning, \
                                   swapFlag.at( iterForPos->second.split_position_ - start_position - 1 ), source_sentence, \
                                   cells.cells.at( start_position ).at( iterForPos->second.split_position_ ).at( posPrev ), \
                                   cells.cells.at( iterForPos->second.split_position_ ).at( end_position ).at( posFoll ), count );
      explored.at( iterForPos->second.split_position_ - start_position - 1 ).at( posPrev ).at( posFoll ) = true;
      pos.split_position_ = iterForPos->second.split_position_;
      pos.front_position_ = posPrev;
      pos.back_position_ = posFoll;
      posInfo.insert( make_pair( retScore, pos ) );
    }
    posInfo.erase( iterForPos );
  }
  return true;
}


/*
 * $Name: ExpandHypotheses
 * $Function: Expands hypotheses.
 * $Date: 2013-05-08
 */
float PhraseBasedITGDecoder::ExpandHypotheses (const Configuration &config, Model &model, \
                                               const size_t &startPos, const size_t &segPos, const size_t &endPos, \
                                               CELLFORCUBEPRUNING &cellForCubePruning, const bool &swapFlag, const SourceSentence &source_sentence, \
                                               const Hypothesis &prevHypo, const Hypothesis &follHypo, int &count) {
  float  featureScoreExceptLM = prevHypo.feature_score_except_lm_ + follHypo.feature_score_except_lm_;
  string tgtOption            = prevHypo.translation_result_ + " " + follHypo.translation_result_; 
  RmEndSpace(tgtOption);
  RmStartSpace(tgtOption);

  bool composeFlag = true;
  float languageModelScore = 0;
  int   tgtWordCount       = 0;
  Hypothesis hy(featureScoreExceptLM, languageModelScore, tgtOption, composeFlag, tgtWordCount);
  BuildNewHypothesis (prevHypo, follHypo, hy);
  CalculateLMScoreForComposing (config, model, prevHypo, follHypo, hy);
  SaveMEReorderValue (prevHypo, follHypo, hy);
  CalculateMEReorderScore (config, startPos, segPos, endPos, source_sentence, prevHypo, follHypo, hy); 
  hy.feature_score_except_lm_ += hy.me_reordering_score_mono_;
  float modelScore = hy.language_model_score_ + hy.feature_score_except_lm_;

  if (config.mert_flag_) {
    hy.feature_values_.resize(config.features.features_number_);
    hy.feature_values_.at(0) = hy.language_model_score_/config.features.feature_values_.at(0).weight_;
    hy.feature_values_.at(1) = prevHypo.feature_values_.at(1) + follHypo.feature_values_.at(1);
    hy.feature_values_.at(2) = prevHypo.feature_values_.at(2) + follHypo.feature_values_.at(2);
    hy.feature_values_.at(3) = prevHypo.feature_values_.at(3) + follHypo.feature_values_.at(3);
    hy.feature_values_.at(4) = prevHypo.feature_values_.at(4) + follHypo.feature_values_.at(4);
    hy.feature_values_.at(5) = prevHypo.feature_values_.at(5) + follHypo.feature_values_.at(5);
    hy.feature_values_.at(6) = prevHypo.feature_values_.at(6) + follHypo.feature_values_.at(6);
    hy.feature_values_.at(7) = prevHypo.feature_values_.at(7) + follHypo.feature_values_.at(7);
    hy.feature_values_.at(8) = prevHypo.feature_values_.at(8) + follHypo.feature_values_.at(8);
    hy.feature_values_.at(9) = prevHypo.feature_values_.at(9) + follHypo.feature_values_.at(9) \
                             + hy.me_reordering_score_mono_/config.features.feature_values_.at(9).weight_;

    if (config.use_context_sensitive_wd_) {
      hy.v_context_sensitive_wd_values_.at(0) = prevHypo.v_context_sensitive_wd_values_.at(0) + follHypo.v_context_sensitive_wd_values_.at(0);
      hy.v_context_sensitive_wd_values_.at(1) = prevHypo.v_context_sensitive_wd_values_.at(1) + follHypo.v_context_sensitive_wd_values_.at(1);
    }

    for (int i = 0; i < config.free_feature_; ++ i) {
      hy.feature_values_.at(i + 17) = prevHypo.feature_values_.at(i + 17) + follHypo.feature_values_.at(i + 17);
    }
  }

  cellForCubePruning.insert(make_pair(modelScore, hy));
  ++count;
  float retScore = modelScore;
  if (swapFlag) {
    string tgtOptionSwap = follHypo.translation_result_ + " " + prevHypo.translation_result_;
    RmEndSpace(tgtOptionSwap);
    RmStartSpace(tgtOptionSwap);

    float languageModelScoreSwap = 0;
    Hypothesis hySwap(featureScoreExceptLM, languageModelScoreSwap, tgtOptionSwap, composeFlag, tgtWordCount);

    BuildNewHypothesis (follHypo, prevHypo, hySwap);
    CalculateLMScoreForComposing (config, model, follHypo, prevHypo, hySwap);
    SaveMEReorderValue (follHypo, prevHypo, hySwap);
    hySwap.feature_score_except_lm_ += hy.me_reordering_score_swap_;
    float modelScoreSwap = hySwap.language_model_score_ + hySwap.feature_score_except_lm_;

    if (config.mert_flag_) {
      hySwap.feature_values_.resize(config.features.features_number_);
      hySwap.feature_values_.at(0) = hySwap.language_model_score_/config.features.feature_values_.at(0).weight_;
      hySwap.feature_values_.at(1) = follHypo.feature_values_.at(1) + prevHypo.feature_values_.at(1);
      hySwap.feature_values_.at(2) = follHypo.feature_values_.at(2) + prevHypo.feature_values_.at(2);
      hySwap.feature_values_.at(3) = follHypo.feature_values_.at(3) + prevHypo.feature_values_.at(3);
      hySwap.feature_values_.at(4) = follHypo.feature_values_.at(4) + prevHypo.feature_values_.at(4);
      hySwap.feature_values_.at(5) = follHypo.feature_values_.at(5) + prevHypo.feature_values_.at(5);
      hySwap.feature_values_.at(6) = follHypo.feature_values_.at(6) + prevHypo.feature_values_.at(6);
      hySwap.feature_values_.at(7) = follHypo.feature_values_.at(7) + prevHypo.feature_values_.at(7);
      hySwap.feature_values_.at(8) = follHypo.feature_values_.at(8) + prevHypo.feature_values_.at(8);
      hySwap.feature_values_.at(9) = follHypo.feature_values_.at(9) + prevHypo.feature_values_.at(9) \
                                   + hy.me_reordering_score_swap_/config.features.feature_values_.at(9).weight_;

      if (config.use_context_sensitive_wd_) {
        hySwap.v_context_sensitive_wd_values_.at(0) = follHypo.v_context_sensitive_wd_values_.at(0) + prevHypo.v_context_sensitive_wd_values_.at(0);
        hySwap.v_context_sensitive_wd_values_.at(1) = follHypo.v_context_sensitive_wd_values_.at(1) + prevHypo.v_context_sensitive_wd_values_.at(1);
      }

      for (int i = 0; i < config.free_feature_; ++ i) {
        hySwap.feature_values_.at(i + 17) = follHypo.feature_values_.at(i + 17) + prevHypo.feature_values_.at(i + 17);
      }
    }

    cellForCubePruning.insert(make_pair(modelScoreSwap, hySwap));
    ++count;
    retScore = modelScoreSwap > modelScore ? modelScoreSwap : modelScore;
  }
  return retScore;
}


/*
 * $Name: InitCells
 * $Function: Given a source sentence, all the cells are initialized with the phrase 
 *            translations appear in the phrase translation table.
 * $Date: 2013-05-08
 */
bool PhraseBasedITGDecoder::InitCells (Configuration &config, Model &model, Cells &cells, SourceSentence &source_sentence) {
  if (!config.phrase_table_binary_flag_) {
    InitCellsNormal (config, model, cells, source_sentence);
  } else {
    InitCellsBinary (config, model, cells, source_sentence);
  }
  return true;
}


/*
 * $Name: InitCellsNormal
 * $Function: Given a source sentence, all the cells are initialized with the phrase
 *            translations appear in the text phrase translation table file.
 * $Date: 2013-05-08
 */
bool PhraseBasedITGDecoder::InitCellsNormal (Configuration &config, Model &model, Cells &cells, SourceSentence &source_sentence) {
  for (size_t phraseLength = 1; phraseLength <= config.max_phrase_length_; ++ phraseLength) {
    for (size_t startPos = 0; (startPos < source_sentence.cell_span_.size()) && (startPos + phraseLength <= source_sentence.cell_span_.size()); ++ startPos) {
      size_t endPos = startPos + phraseLength;
      string srcPhrase;
      for (size_t i = startPos; i < endPos; ++i) {
        srcPhrase += source_sentence.cell_span_.at(i) + " ";
      }

      RmEndSpace(srcPhrase);
      map<string, TranslationOptions>::iterator iter = model.phrase_table_.translationsOptions.find(srcPhrase);

      if (iter != model.phrase_table_.translationsOptions.end()) {
        CELLFORCUBEPRUNING hypotheses;
        for (vector<TranslationOption>::iterator iterForTransOption = iter->second.translationOptions.begin(); iterForTransOption != iter->second.translationOptions.end(); ++iterForTransOption) {
          float proEGivenF        = config.features.feature_values_.at(2).weight_ * iterForTransOption->scoresOfTranslateOption.prob_egivenf_     ;    // Pr(e|f)
          float lexEGivenF        = config.features.feature_values_.at(3).weight_ * iterForTransOption->scoresOfTranslateOption.lexi_egivenf_     ;    // Lex(e|f)
          float proFGivenE        = config.features.feature_values_.at(4).weight_ * iterForTransOption->scoresOfTranslateOption.prob_fgivene_     ;    // Pr(f|e)
          float lexFGivenE        = config.features.feature_values_.at(5).weight_ * iterForTransOption->scoresOfTranslateOption.lexi_fgivene_     ;    // Lex(f|e)
          float naturalLogarithm  = config.features.feature_values_.at(6).weight_ * iterForTransOption->scoresOfTranslateOption.natural_logarithm_;
          float biLexLinks        = config.features.feature_values_.at(7).weight_ * iterForTransOption->scoresOfTranslateOption.bi_lexi_links_    ;
          float empty_translation = config.features.feature_values_.at(8).weight_ * iterForTransOption->scoresOfTranslateOption.empty_translation_;
          float featureScoreExceptLM = proEGivenF + lexEGivenF + proFGivenE + lexFGivenE + naturalLogarithm + biLexLinks + empty_translation;
          if (config.free_feature_ != 0) {
            if ((config.free_feature_ <= iterForTransOption->scoresOfTranslateOption.v_freefeature_value_.size()) && (config.free_feature_ <= config.features.feature_values_.size() - 17)) {
              for (int i = 0; i < config.free_feature_; ++ i) {
                featureScoreExceptLM += config.features.feature_values_.at(i + 17).weight_ * iterForTransOption->scoresOfTranslateOption.v_freefeature_value_.at(i);
              }
            } else {
              cerr<<"ERROR: Parameter 'freefeature' in config file do not adapt to the phrasetable!\n";
              exit( 1 );
            }
          }

          // added by qiang, 20150706
          float context_sensitive_wd_value_aln = 0.0f;
          float context_sensitive_wd_value_unaln = 0.0f;
          if (config.use_context_sensitive_wd_) {
            for (int i = 0; i < iterForTransOption->v_unaligned_flag_.size(); ++i) {
              if (iterForTransOption->v_unaligned_flag_.at(i) == true) {
                featureScoreExceptLM += config.features.v_context_sensitive_wd_feature_.at(1).weight_ * source_sentence.context_sensitive_wd_scores_.at(i + startPos).second;
                if (config.mert_flag_) {
                  context_sensitive_wd_value_unaln += source_sentence.context_sensitive_wd_scores_.at(i + startPos).second;
                }
              } else {
                featureScoreExceptLM += config.features.v_context_sensitive_wd_feature_.at(0).weight_ * source_sentence.context_sensitive_wd_scores_.at(i + startPos).first;
                if (config.mert_flag_) {
                  context_sensitive_wd_value_aln += source_sentence.context_sensitive_wd_scores_.at(i + startPos).first;
                }
              }
            }
          }
          // added by qiang, 20150706
          bool  composeFlag = false;
          float languageModelScore = 0;
          int   tgtWordCount = 0;
          Hypothesis hy(featureScoreExceptLM , languageModelScore, iterForTransOption->tgtOption, composeFlag, tgtWordCount);
          TranslationLog translationLog(startPos, endPos, iterForTransOption->tgtOption);
          hy.translation_log_.push_back(translationLog);
          CalculateLMScore (config, model, hy);

          hy.me_reorder_scores_.assign(8, 0.0);
          if (hy.target_word_count_ != 0) {
            if (model.me_reorder_table_.meTableTgt.find(hy.target_word_.at(0)) != model.me_reorder_table_.meTableTgt.end()) {
              hy.me_reorder_scores_.at(0) = model.me_reorder_table_.meTableTgt[hy.target_word_.at(0)].at(0);
              hy.me_reorder_scores_.at(2) = model.me_reorder_table_.meTableTgt[hy.target_word_.at(0)].at(2);
              hy.me_reorder_scores_.at(4) = model.me_reorder_table_.meTableTgt[hy.target_word_.at(0)].at(4);
              hy.me_reorder_scores_.at(6) = model.me_reorder_table_.meTableTgt[hy.target_word_.at(0)].at(6);
            }

            if (model.me_reorder_table_.meTableTgt.find(hy.target_word_.at(hy.target_word_count_ - 1)) != model.me_reorder_table_.meTableTgt.end()) {
              hy.me_reorder_scores_.at(1) = model.me_reorder_table_.meTableTgt[hy.target_word_.at(hy.target_word_count_ - 1)].at(1);
              hy.me_reorder_scores_.at(3) = model.me_reorder_table_.meTableTgt[hy.target_word_.at(hy.target_word_count_ - 1)].at(3);
              hy.me_reorder_scores_.at(5) = model.me_reorder_table_.meTableTgt[hy.target_word_.at(hy.target_word_count_ - 1)].at(5);
              hy.me_reorder_scores_.at(7) = model.me_reorder_table_.meTableTgt[hy.target_word_.at(hy.target_word_count_ - 1)].at(7);
            }
          }

          if (config.mert_flag_) {
            hy.feature_values_.resize(config.features.features_number_);
            hy.feature_values_.at(0) = hy.language_model_score_/config.features.feature_values_.at(0).weight_;
            hy.feature_values_.at(1) = (float)hy.target_word_count_;
            hy.feature_values_.at(2) = iterForTransOption->scoresOfTranslateOption.prob_egivenf_     ;    // Pr(e|f)
            hy.feature_values_.at(3) = iterForTransOption->scoresOfTranslateOption.lexi_egivenf_     ;    // Lex(e|f)
            hy.feature_values_.at(4) = iterForTransOption->scoresOfTranslateOption.prob_fgivene_     ;    // Pr(f|e)
            hy.feature_values_.at(5) = iterForTransOption->scoresOfTranslateOption.lexi_fgivene_     ;    // Lex(f|e)
            hy.feature_values_.at(6) = iterForTransOption->scoresOfTranslateOption.natural_logarithm_;
            hy.feature_values_.at(7) = iterForTransOption->scoresOfTranslateOption.bi_lexi_links_    ;
            hy.feature_values_.at(8) = iterForTransOption->scoresOfTranslateOption.empty_translation_;

            if (config.use_context_sensitive_wd_) {
                hy.v_context_sensitive_wd_values_.at(0) = context_sensitive_wd_value_aln;
                hy.v_context_sensitive_wd_values_.at(1) = context_sensitive_wd_value_unaln;
            }
          
            if (config.free_feature_ != 0) {
              if ((config.free_feature_ <= iterForTransOption->scoresOfTranslateOption.v_freefeature_value_.size()) \
                   && (config.free_feature_ <= config.features.feature_values_.size() - 17)) {
                for (int i = 0; i < config.free_feature_; ++ i) {
                  hy.feature_values_.at(i + 17) = iterForTransOption->scoresOfTranslateOption.v_freefeature_value_.at(i);
                }
              } else {
                cerr<<"ERROR: Parameter 'freefeature' in config file do not adapt to the phrasetable!\n";
                exit (1);
              }
            }
          }

          // number of target-words
          hy.feature_score_except_lm_ += config.features.feature_values_.at(1).weight_ * hy.target_word_count_;
          float modelScore = hy.feature_score_except_lm_ + hy.language_model_score_;
          hypotheses.insert(make_pair(modelScore, hy));
        }
        for (CELLFORCUBEPRUNING::reverse_iterator riter  = hypotheses.rbegin(); riter != hypotheses.rend(); ++ riter) {
          cells.cells.at(startPos).at(endPos).push_back(riter->second);
        }
      } else {
        if (phraseLength == 1 && cells.cells.at(startPos).at(endPos).size() == 0) {
          if (startPos == 0 || endPos == source_sentence.cell_span_.size()) {
            continue;
          }
          float  score = 0;
          string translationResult;
          int tgtWordCount = 0;
          if (config.label_oov_) {
            translationResult = "<" + source_sentence.cell_span_.at(startPos) + ">";
            tgtWordCount = 1;
          } else {
            translationResult = "";
            tgtWordCount = 0;
          }
          bool   composeFlag  = false;
          Hypothesis hy(score, score, translationResult, composeFlag, tgtWordCount);
          TranslationLog translationLog(startPos, endPos, translationResult);
          hy.translation_log_.push_back(translationLog);
          CalculateLMScore (config, model, hy);
          hy.me_reorder_scores_.assign(8, 0.0);
          hy.language_model_score_ = 0.0;
          if (config.mert_flag_) {
            hy.feature_values_.resize(config.features.features_number_);
            if (config.use_context_sensitive_wd_) {
              hy.v_context_sensitive_wd_values_.resize(2, 0.0f);
            }
          }
          cells.cells.at(startPos).at(endPos).push_back(hy);
        } 
      }
    }
  }
  return true;
}


/*
 * $Name: InitCellsBinary
 * $Function: Given a source sentence, all the cells are initialized with the phrase
 *            translations appear in the binary phrase translation table file.
 * $Date: 2013-05-08
 */
bool PhraseBasedITGDecoder::InitCellsBinary(Configuration  &config, Model &model, Cells &cells, SourceSentence &source_sentence) {
  for ( size_t phraseLength = 1; phraseLength <= config.max_phrase_length_; ++ phraseLength ) {
    for( size_t startPos = 0; ( startPos < source_sentence.cell_span_.size() ) \
        && ( startPos + phraseLength <= source_sentence.cell_span_.size() ); ++ startPos ) {
      size_t endPos = startPos + phraseLength;
      string srcPhrase;
      for( size_t i = startPos; i < endPos; ++i ) {
        srcPhrase += source_sentence.cell_span_.at( i ) + " ";
      }
      RmEndSpace( srcPhrase );
      unsigned int encode = 0;
      if ( model.phrase_table_.srcPhrEncode.find( srcPhrase ) != model.phrase_table_.srcPhrEncode.end() ) {
        encode = model.phrase_table_.srcPhrEncode[ srcPhrase ];
        map< unsigned int, TranslationOptionsBina >::iterator iter = model.phrase_table_.translationsOptionsBina.find( encode );
        if ( iter != model.phrase_table_.translationsOptionsBina.end() ) {
          CELLFORCUBEPRUNING hypotheses;
          for ( vector< TranslationOptionBina >::iterator iterForTransOptionBina = iter->second.translationOptionsBina.begin(); \
                iterForTransOptionBina != iter->second.translationOptionsBina.end(); ++iterForTransOptionBina ) {
            float featureScoreExceptLM = iterForTransOptionBina->transModelScore;
            bool  composeFlag = false;
            float languageModelScore = 0;
            int   tgtWordCount       = 0;
            string transRes = model.phrase_table_.tgtPhrDecode.at( iterForTransOptionBina->tgtOptionEncode );
            Hypothesis hy( featureScoreExceptLM , languageModelScore, transRes, composeFlag, tgtWordCount );
            TranslationLog translationLog( startPos, endPos, transRes );
            hy.translation_log_.push_back( translationLog );
            CalculateLMScore( config, model, hy );
            hy.me_reorder_scores_.assign( 8, 0.0 );
            if ( model.me_reorder_table_.meTableTgt.find( hy.target_word_.at( 0 ) ) != model.me_reorder_table_.meTableTgt.end() ) {
              hy.me_reorder_scores_.at( 0 ) = model.me_reorder_table_.meTableTgt[ hy.target_word_.at( 0 ) ].at( 0 );
              hy.me_reorder_scores_.at( 2 ) = model.me_reorder_table_.meTableTgt[ hy.target_word_.at( 0 ) ].at( 2 );
              hy.me_reorder_scores_.at( 4 ) = model.me_reorder_table_.meTableTgt[ hy.target_word_.at( 0 ) ].at( 4 );
              hy.me_reorder_scores_.at( 6 ) = model.me_reorder_table_.meTableTgt[ hy.target_word_.at( 0 ) ].at( 6 );
            }

            if( model.me_reorder_table_.meTableTgt.find( hy.target_word_.at( hy.target_word_count_ - 1 ) ) != model.me_reorder_table_.meTableTgt.end() ) {
              hy.me_reorder_scores_.at( 1 ) = model.me_reorder_table_.meTableTgt[ hy.target_word_.at( hy.target_word_count_ - 1 ) ].at( 1 );
              hy.me_reorder_scores_.at( 3 ) = model.me_reorder_table_.meTableTgt[ hy.target_word_.at( hy.target_word_count_ - 1 ) ].at( 3 );
              hy.me_reorder_scores_.at( 5 ) = model.me_reorder_table_.meTableTgt[ hy.target_word_.at( hy.target_word_count_ - 1 ) ].at( 5 );
              hy.me_reorder_scores_.at( 7 ) = model.me_reorder_table_.meTableTgt[ hy.target_word_.at( hy.target_word_count_ - 1 ) ].at( 7 );
            }

            // number of target-words
            hy.feature_score_except_lm_ += config.features.feature_values_.at( 1 ).weight_ * hy.target_word_count_;
            float modelScore = hy.feature_score_except_lm_ + hy.language_model_score_;
            hypotheses.insert( make_pair( modelScore, hy ) );
          }
          for ( CELLFORCUBEPRUNING::reverse_iterator riter  = hypotheses.rbegin(); riter != hypotheses.rend(); ++ riter ) {
            cells.cells.at( startPos ).at( endPos ).push_back( riter->second );
          }
        } else {
          if ( phraseLength == 1 && cells.cells.at( startPos ).at( endPos ).size() == 0 ) {
            if( startPos == 0 || endPos == source_sentence.cell_span_.size() ){
              continue;
            }
            float  score = 0;
            string translationResult;
            if ( config.label_oov_ ) {
              translationResult = "<" + source_sentence.cell_span_.at( startPos ) + ">";
            } else {
              translationResult = source_sentence.cell_span_.at( startPos );
            }

            bool   composeFlag  = false;
            int    tgtWordCount = 1;
            Hypothesis hy( score, score, translationResult, composeFlag, tgtWordCount );
            TranslationLog translationLog( startPos, endPos, translationResult ); 
            hy.translation_log_.push_back( translationLog );
            CalculateLMScore( config, model, hy );
            hy.me_reorder_scores_.assign( 8, 0.0 );
            hy.language_model_score_ = 0.0;
            if ( config.mert_flag_ ) {
              hy.feature_values_.resize( config.features.features_number_ );
            }
            cells.cells.at( startPos ).at( endPos ).push_back( hy );
          }
        }
      } else {
        if ( phraseLength == 1 && cells.cells.at( startPos ).at( endPos ).size() == 0 ) {
          if ( startPos == 0 || endPos == source_sentence.cell_span_.size() ) {
            continue;
          }
          float  score = 0;
          string translationResult;    
          if( config.label_oov_ ) {
            translationResult = "<" + source_sentence.cell_span_.at( startPos ) + ">";
          } else {
            translationResult = source_sentence.cell_span_.at( startPos );
          }

          bool   composeFlag  = false;
          int    tgtWordCount = 1;
          Hypothesis hy( score, score, translationResult, composeFlag, tgtWordCount );
          TranslationLog translationLog( startPos, endPos, translationResult ); 
          hy.translation_log_.push_back( translationLog );
          CalculateLMScore( config, model, hy );
          hy.me_reorder_scores_.assign( 8, 0.0 );
          hy.language_model_score_ = 0.0;
          if ( config.mert_flag_ ) {
            hy.feature_values_.resize( config.features.features_number_ );
          }
          cells.cells.at( startPos ).at( endPos ).push_back( hy );
        } 
      }
    }
  }
  return true;
}


bool PhraseBasedITGDecoder::ClearCellsWithForcedType(Configuration &config, Model &model, Cells &cells, SourceSentence &source_sentence) {
  for ( multimap< pair< size_t, size_t >, GeneralizationInfo >::iterator iter = source_sentence.generalization_informations_.begin(); \
       iter != source_sentence.generalization_informations_.end(); ++ iter ) {
    if ( iter->second.symbol_type_ == 1 ) {
      if ( iter->second.end_position_ - iter->second.start_position_ >= 1 ) {
        for ( size_t len = 1; len <= iter->second.end_position_ - iter->second.start_position_; ++len ) {
          for ( size_t startPos = iter->second.start_position_; startPos < iter->second.end_position_ \
                && ( startPos + len ) <= iter->second.end_position_; ++startPos ) {
            size_t endPos = iter->second.start_position_ + len;
            cells.cells.at( startPos ).at( endPos ).clear();
          }
        }
        float  score        = 0;
        int    tgtWordCount = 1;
        bool   composeFlag  = false;
        Hypothesis hy( score, score, iter->second.translation_result_, composeFlag, tgtWordCount );
        TranslationLog translationLog( iter->second.start_position_, iter->second.end_position_, iter->second.translation_result_ );
        hy.translation_log_.push_back( translationLog );
        CalculateLMScore( config, model, hy );
        hy.me_reorder_scores_.assign( 8, 0.0 );
        hy.feature_values_.assign( config.features.features_number_, 0.0 );
        cells.cells.at( iter->second.start_position_ ).at( iter->second.end_position_ ).push_back( hy );
      }
    }
  }
  return true;
}


/*
 * $Name: InitCellsWithGeneralization
 * $Function:
 * $Date: 2013-05-08
 */
bool PhraseBasedITGDecoder::InitCellsWithGeneralization (Configuration &config, Model &model, Cells &cells, SourceSentence &source_sentence) {
  for (multimap< pair< size_t, size_t >, GeneralizationInfo >::iterator iter = source_sentence.generalization_informations_.begin(); \
       iter != source_sentence.generalization_informations_.end(); ++ iter) {
    // normal symbol Type, i.e. "$person"
    if (iter->second.symbol_type_ == 2) {
      float  score             = 0.0                  ;
      string translationResult = iter->second.translation_result_;
      bool   composeFlag       = false                ;
      int    tgtWordCount      = 0                    ;
      Hypothesis hy(score, score, translationResult, composeFlag, tgtWordCount);
      TranslationLog translationLog(iter->second.start_position_, iter->second.end_position_, translationResult);
      hy.translation_log_.push_back(translationLog);
      CalculateLMScore (config, model, hy);
      hy.me_reorder_scores_.assign(8, 0.0);

      if (model.me_reorder_table_.meTableTgt.find(hy.target_word_.at(0)) != model.me_reorder_table_.meTableTgt.end()) {
        hy.me_reorder_scores_.at(0) = model.me_reorder_table_.meTableTgt[hy.target_word_.at(0)].at(0);
        hy.me_reorder_scores_.at(2) = model.me_reorder_table_.meTableTgt[hy.target_word_.at(0)].at(2);
        hy.me_reorder_scores_.at(4) = model.me_reorder_table_.meTableTgt[hy.target_word_.at(0)].at(4);
        hy.me_reorder_scores_.at(6) = model.me_reorder_table_.meTableTgt[hy.target_word_.at(0)].at(6);
      }
      if (model.me_reorder_table_.meTableTgt.find(hy.target_word_.at(hy.target_word_count_ - 1)) != model.me_reorder_table_.meTableTgt.end()) {
        hy.me_reorder_scores_.at(1) = model.me_reorder_table_.meTableTgt[hy.target_word_.at(hy.target_word_count_ - 1)].at(1);
        hy.me_reorder_scores_.at(3) = model.me_reorder_table_.meTableTgt[hy.target_word_.at(hy.target_word_count_ - 1)].at(3);
        hy.me_reorder_scores_.at(5) = model.me_reorder_table_.meTableTgt[hy.target_word_.at(hy.target_word_count_ - 1)].at(5);
        hy.me_reorder_scores_.at(7) = model.me_reorder_table_.meTableTgt[hy.target_word_.at(hy.target_word_count_ - 1)].at(7);
      }
      hy.language_model_score_ = 0;
      hy.feature_score_except_lm_ = config.features.feature_values_.at(1).weight_ * hy.target_word_count_ + config.features.feature_values_.at(6).weight_;
      if (config.mert_flag_) {
        hy.feature_values_.resize(config.features.features_number_);
        hy.feature_values_.at(1) = (float)hy.target_word_count_;
        hy.feature_values_.at(6) = 1;
        if (config.use_context_sensitive_wd_) {
          hy.v_context_sensitive_wd_values_.resize(2, 0.0f);
        }
      }
      cells.cells.at(iter->second.start_position_).at(iter->second.end_position_).push_back(hy);
    } else if (iter->second.symbol_type_ == 1) {
      // forced type.
      float  score             = 0.0                  ;
      string translationResult = iter->second.translation_result_;
      bool   composeFlag       = false                ;
      int    tgtWordCount      = 0                    ;
      Hypothesis hy(score, score, translationResult, composeFlag, tgtWordCount);
      TranslationLog translationLog(iter->second.start_position_, iter->second.end_position_, translationResult);
      hy.translation_log_.push_back(translationLog);
      CalculateLMScore (config, model, hy);
      hy.me_reorder_scores_.assign(8, 0.0);
      if (model.me_reorder_table_.meTableTgt.find(hy.target_word_.at(0)) != model.me_reorder_table_.meTableTgt.end()) {
        hy.me_reorder_scores_.at(0) = model.me_reorder_table_.meTableTgt[hy.target_word_.at(0)].at(0);
        hy.me_reorder_scores_.at(2) = model.me_reorder_table_.meTableTgt[hy.target_word_.at(0)].at(2);
        hy.me_reorder_scores_.at(4) = model.me_reorder_table_.meTableTgt[hy.target_word_.at(0)].at(4);
        hy.me_reorder_scores_.at(6) = model.me_reorder_table_.meTableTgt[hy.target_word_.at(0)].at(6);
      }
      if (model.me_reorder_table_.meTableTgt.find(hy.target_word_.at(hy.target_word_count_ - 1)) != model.me_reorder_table_.meTableTgt.end()) {
        hy.me_reorder_scores_.at(1) = model.me_reorder_table_.meTableTgt[hy.target_word_.at(hy.target_word_count_ - 1)].at(1);
        hy.me_reorder_scores_.at(3) = model.me_reorder_table_.meTableTgt[hy.target_word_.at(hy.target_word_count_ - 1)].at(3);
        hy.me_reorder_scores_.at(5) = model.me_reorder_table_.meTableTgt[hy.target_word_.at(hy.target_word_count_ - 1)].at(5);
        hy.me_reorder_scores_.at(7) = model.me_reorder_table_.meTableTgt[hy.target_word_.at(hy.target_word_count_ - 1)].at(7);
      }
      hy.language_model_score_ = 0;
      if (!cells.cells.at(iter->second.start_position_).at(iter->second.end_position_).empty()) {
        cells.cells.at(iter->second.start_position_).at(iter->second.end_position_).clear();
      }
      hy.feature_score_except_lm_ = config.features.feature_values_.at(1).weight_ * hy.target_word_count_ + config.features.feature_values_.at(6).weight_;
      if (config.mert_flag_) {
        hy.feature_values_.resize(config.features.features_number_);
        hy.feature_values_.at(1) = (float)hy.target_word_count_;
        hy.feature_values_.at(6) = 1;
        if (config.use_context_sensitive_wd_) {
          hy.v_context_sensitive_wd_values_.resize(2, 0.0f);
        }
      }
      cells.cells.at(iter->second.start_position_).at(iter->second.end_position_).push_back(hy);
    }
  }
  return true;
}


/*
 * $Name: AllocateSpaceForCells
 * $Function: Allocates space for cells.
 * $Date: 2013-05-08
 */
bool PhraseBasedITGDecoder::AllocateSpaceForCells(Cells &cells, SourceSentence &source_sentence) {
  cells.cells.resize( source_sentence.cell_span_.size() + 1 );
  for ( VEC3HYPOIT iter = cells.cells.begin(); iter != cells.cells.end(); ++ iter ) {
    iter->resize( source_sentence.cell_span_.size() + 1 );
  }
  return true;
}


/*
 * $Name: BuildNewHypothesis
 * $Function:
 * $Date: 2013-05-09
 */
bool PhraseBasedITGDecoder::BuildNewHypothesis(const Hypothesis &front_hypothesis, const Hypothesis &back_hypothesis, Hypothesis &hypothesis) {
  hypothesis.target_word_count_ = front_hypothesis.target_word_count_ + back_hypothesis.target_word_count_;

  hypothesis.target_word_id_.reserve( hypothesis.target_word_count_ );
  hypothesis.target_word_id_.assign( front_hypothesis.target_word_id_.begin(), front_hypothesis.target_word_id_.end() );
  for ( CONINTIT iter = back_hypothesis.target_word_id_.begin(); iter != back_hypothesis.target_word_id_.end(); ++ iter ) {
    hypothesis.target_word_id_.push_back( *iter );
  }

  hypothesis.target_word_.reserve( hypothesis.target_word_count_ );
  hypothesis.target_word_.assign( front_hypothesis.target_word_.begin(), front_hypothesis.target_word_.end() );
  for ( CONSTRIT iter = back_hypothesis.target_word_.begin(); iter != back_hypothesis.target_word_.end(); ++ iter ) {
    hypothesis.target_word_.push_back( *iter );
  }

  hypothesis.target_n_gram_lm_score_.reserve( front_hypothesis.target_n_gram_lm_score_.size() + back_hypothesis.target_n_gram_lm_score_.size() );
  hypothesis.target_n_gram_lm_score_.assign( front_hypothesis.target_n_gram_lm_score_.begin(), front_hypothesis.target_n_gram_lm_score_.end() );
  for ( CONFLOIT iter = back_hypothesis.target_n_gram_lm_score_.begin(); iter != back_hypothesis.target_n_gram_lm_score_.end(); ++ iter ) {
    hypothesis.target_n_gram_lm_score_.push_back( *iter );
  }

  hypothesis.translation_log_.reserve( front_hypothesis.translation_log_.size() + back_hypothesis.translation_log_.size() );
  hypothesis.translation_log_.assign( front_hypothesis.translation_log_.begin(), front_hypothesis.translation_log_.end() );
  for ( CONTRAIT iter = back_hypothesis.translation_log_.begin(); iter != back_hypothesis.translation_log_.end(); ++ iter ) {
    hypothesis.translation_log_.push_back( *iter );
  }

  return true;
}


/*
 * $Name: CalculateLMScore
 * $Function:
 * $Date: 2013-05-09
 */
bool PhraseBasedITGDecoder::CalculateLMScore(Configuration &config, Model &model, Hypothesis &hy) {
  Split( hy.translation_result_, ' ', hy.target_word_ );
  hy.target_word_count_ = ( int )hy.target_word_.size();
  int pos = 0;
  for ( vector< string >::iterator iter  = hy.target_word_.begin(); iter != hy.target_word_.end(); ++ iter ) {
    int wid = 0;
    if ( model.phrase_table_.tgtVocab.find( *iter ) != model.phrase_table_.tgtVocab.end() ) {
      wid = model.phrase_table_.tgtVocab[ *iter ];
    } else {
      wid = model.phrase_table_.tgtVocab[ "<unk>" ];
    }
    hy.target_word_id_.push_back( wid );
    int end    = pos + 1;
    int begin  = end - config.ngram_ > 0 ? end - config.ngram_ : 0;
    float prob = model.ngram_language_model_.GetProb( hy.target_word_id_, begin, end );

#ifdef WIN32
    if( prob < -20 ) {
#else
    if( prob < -20 || isnan( prob ) ) {
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
 * $Name: CalculateLMScoreForComposing
 * $Function:
 * $Date: 2013-05-09
 */
bool PhraseBasedITGDecoder::CalculateLMScoreForComposing(const Configuration &config, Model &model, const Hypothesis &hyPrev, \
                                                         const Hypothesis &hyFoll, Hypothesis &hy) {
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
 * $Date:
 */
bool PhraseBasedITGDecoder::CalculateMEReorderScore (const Configuration &config, const size_t &start_position, const size_t &split_position, \
                                                     const size_t &end_position, const SourceSentence &source_sentence, \
                                                     const Hypothesis &front_hypothesis, const Hypothesis &back_hypothesis, Hypothesis &hypothesis) {
  float w0 = 0.0;
  float w1 = 0.0;

  w0 += source_sentence.me_reorder_scores_.at(start_position).at(0);
  w1 += source_sentence.me_reorder_scores_.at(start_position).at(4);
  w0 += source_sentence.me_reorder_scores_.at(split_position - 1).at(1);
  w1 += source_sentence.me_reorder_scores_.at(split_position - 1).at(5);
  w0 += source_sentence.me_reorder_scores_.at(split_position).at(2);
  w1 += source_sentence.me_reorder_scores_.at(split_position).at(6);
  w0 += source_sentence.me_reorder_scores_.at(end_position - 1).at(3);
  w1 += source_sentence.me_reorder_scores_.at(end_position - 1).at(7);

  w0 += front_hypothesis.me_reorder_scores_.at(0);
  w1 += front_hypothesis.me_reorder_scores_.at(4);
  w0 += front_hypothesis.me_reorder_scores_.at(1);
  w1 += front_hypothesis.me_reorder_scores_.at(5);
  w0 += back_hypothesis.me_reorder_scores_.at(2);
  w1 += back_hypothesis.me_reorder_scores_.at(6);
  w0 += back_hypothesis.me_reorder_scores_.at(3);
  w1 += back_hypothesis.me_reorder_scores_.at(7);

  hypothesis.me_reordering_score_mono_ = (float)(log(exp(w0) / (exp(w0) + exp(w1))) * config.features.feature_values_.at(9).weight_);
  hypothesis.me_reordering_score_swap_ = (float)(log(exp(w1) / (exp(w0) + exp(w1))) * config.features.feature_values_.at(9).weight_);

  return true;
}


/*
 * $Name: SaveMEReorderValue
 * $Function:
 * $Date:
 */
bool PhraseBasedITGDecoder::SaveMEReorderValue (const Hypothesis &front_hypothesis, const Hypothesis &back_hypothesis, Hypothesis &hypothesis) {
  hypothesis.me_reorder_scores_.assign(8, 0.0);
  hypothesis.me_reorder_scores_.at(0) = front_hypothesis.me_reorder_scores_.at(0);
  hypothesis.me_reorder_scores_.at(1) = back_hypothesis.me_reorder_scores_.at(1);
  hypothesis.me_reorder_scores_.at(2) = front_hypothesis.me_reorder_scores_.at(2);
  hypothesis.me_reorder_scores_.at(3) = back_hypothesis.me_reorder_scores_.at(3);
  hypothesis.me_reorder_scores_.at(4) = front_hypothesis.me_reorder_scores_.at(4);
  hypothesis.me_reorder_scores_.at(5) = back_hypothesis.me_reorder_scores_.at(5);
  hypothesis.me_reorder_scores_.at(6) = front_hypothesis.me_reorder_scores_.at(6);
  hypothesis.me_reorder_scores_.at(7) = back_hypothesis.me_reorder_scores_.at(7);
  return true;
}


/*
 * $Name:
 * $Function:
 * $Date:
 */
bool PhraseBasedITGDecoder::GenerateTranslationResults(const Configuration &config, const Cells &cells, const SourceSentence &source_sentence, string &output) {
  size_t counter = 0;
  for ( CELL::const_iterator riter  = cells.cells.at( 0 ).at( source_sentence.cell_span_.size() ).begin(); \
        riter != cells.cells.at( 0 ).at( source_sentence.cell_span_.size() ).end(); ++ riter ) {
    ++counter;
    if ( counter == 1 ) {
      size_t pos = 0;
      for ( vector< TranslationLog >::const_iterator iter = riter->translation_log_.begin(); iter != riter->translation_log_.end(); ++ iter ) {
        if ( pos != 0 && pos != ( riter->translation_log_.size() - 1 ) ) {
          string tmpTransRes   = iter->translation_result_;
          for ( size_t iPos = iter->start_position_; iPos != iter->end_position_; ++iPos ) {
            if ( source_sentence.generalization_informations_.find( make_pair( iPos, iPos + 1 ) ) != source_sentence.generalization_informations_.end() ) {
              GENEINFOIT_CONST iterGeneBeg = source_sentence.generalization_informations_.lower_bound( make_pair( iPos, iPos + 1 ) );
              GENEINFOIT_CONST iterGeneEnd = source_sentence.generalization_informations_.upper_bound( make_pair( iPos, iPos + 1 ) );
              if ( iterGeneBeg != iterGeneEnd ) {
                if ( iterGeneBeg->second.symbol_ == "$number" || \
                     iterGeneBeg->second.symbol_ == "$date"   || \
                     iterGeneBeg->second.symbol_ == "$time" ) {
                  size_t posOfGene = tmpTransRes.find( iterGeneBeg->second.symbol_ );
                  if ( posOfGene != string::npos ) {
                    tmpTransRes = tmpTransRes.replace( posOfGene, iterGeneBeg->second.symbol_.size(), iterGeneBeg->second.translation_result_ );
                  }
                }
              }
            }
          }
          if ( !config.output_oov_ ) {
            if ( tmpTransRes.at( 0 ) != '<' || tmpTransRes.at( tmpTransRes.size() - 1 ) != '>' ){
              output += tmpTransRes + " ";
            }
          } else {
            output += tmpTransRes + " ";        
          }
        }
        ++pos;
      }    
      if ( output != "" ) {
        output = output.substr( 0, output.size() - 1 );
      }
    } else {
      break;
    }
  }
  return true;
}


/*
 * $Name:
 * $Function:
 * $Date:
 */
bool PhraseBasedITGDecoder::WriteMertConfig(Configuration &config) {
  ofstream tmpConf(config.mert_config_file_.c_str());
  if (config.use_context_sensitive_wd_) {
//    tmpConf<<config.features.features_number_ + 1<<"\n";
    tmpConf<<config.features.features_number_ + config.features.cswd_feature_number_<<"\n";
  } else {
    tmpConf<<config.features.features_number_<<"\n";
  }
  int count = 0;
  for ( vector< FeatureValue >::iterator iter = config.features.feature_values_.begin(); iter != config.features.feature_values_.end(); ++iter ) {
    ++count;
    tmpConf<<count<<" "<<iter->weight_<<" "<<iter->min_value_<<" "<<iter->max_value_<<" "<<iter->fixed_<<"\n";
  }
  if (config.use_context_sensitive_wd_) {
    ++count;
//    tmpConf<<count<<" "<<config.features.context_sensitive_wd_feature_.weight_<<" "<<config.features.context_sensitive_wd_feature_.min_value_<<" "<<config.features.context_sensitive_wd_feature_.max_value_<<" "<<config.features.context_sensitive_wd_feature_.fixed_<<"\n";
    tmpConf<<count<<" "
           <<config.features.v_context_sensitive_wd_feature_.at(0).weight_<<" "
           <<config.features.v_context_sensitive_wd_feature_.at(0).min_value_<<" "
           <<config.features.v_context_sensitive_wd_feature_.at(0).max_value_<<" "
           <<config.features.v_context_sensitive_wd_feature_.at(0).fixed_<<"\n";
    ++count;
    tmpConf<<count<<" "
           <<config.features.v_context_sensitive_wd_feature_.at(1).weight_<<" "
           <<config.features.v_context_sensitive_wd_feature_.at(1).min_value_<<" "
           <<config.features.v_context_sensitive_wd_feature_.at(1).max_value_<<" "
           <<config.features.v_context_sensitive_wd_feature_.at(1).fixed_<<"\n";
  }
  tmpConf.close();
  return true;
}


/*
 * $Name: WriterMertTmp
 * $Function: Write Mert Tmp File
 * $Date: 20150714
 */
bool PhraseBasedITGDecoder::WriteMertTmp (const Configuration &config, vector<SourceSentence> &test_set) {
  ofstream mert_tmp_file (config.mert_tmp_file_.c_str());
  if (!mert_tmp_file) {
    cerr<<"ERROR: Can not open file \""
        <<config.mert_tmp_file_
        <<"\".\n"
        <<flush;
    exit(1);
  }

  for (vector<SourceSentence>::iterator iter_vec_ss = test_set.begin(); iter_vec_ss != test_set.end(); ++iter_vec_ss) {
    multimap<float, pair<string, vector<float> > > mm_score_mert_candidate_current;
    for (map<string, vector<float> >::const_iterator iter_map_sf = iter_vec_ss->mert_candidate_current_.begin(); iter_map_sf != iter_vec_ss->mert_candidate_current_.end(); ++iter_map_sf) {
      float score = 0.0f;
      int feature_id = 0;
      for (vector<FeatureValue>::const_iterator iter_v_f = config.features.feature_values_.begin(); iter_v_f != config.features.feature_values_.end(); ++iter_v_f) {
        ++feature_id;
        score += iter_v_f->weight_ * iter_map_sf->second.at(feature_id - 1);
      }
      if (config.use_context_sensitive_wd_) {
        ++feature_id;
        score += config.features.v_context_sensitive_wd_feature_.at(0).weight_ * iter_map_sf->second.at(feature_id - 1);
        ++feature_id;
        score += config.features.v_context_sensitive_wd_feature_.at(1).weight_ * iter_map_sf->second.at(feature_id - 1);
      }
      mm_score_mert_candidate_current.insert(make_pair(score, make_pair(iter_map_sf->first, iter_map_sf->second)));
      iter_vec_ss->SetMertCandidate(config, iter_map_sf->second, iter_map_sf->first);
    }

    for (multimap<float, pair<string, vector<float> > >::reverse_iterator riter_mm = mm_score_mert_candidate_current.rbegin(); riter_mm != mm_score_mert_candidate_current.rend(); ++riter_mm) {
      mert_tmp_file<<riter_mm->second.first<<" |||| ";
      int feature_id = 0;
      for (vector<float>::iterator iter_v = riter_mm->second.second.begin(); iter_v != riter_mm->second.second.end(); ++iter_v) {
        ++feature_id;
        mert_tmp_file<<feature_id<<":"<<*iter_v<<" ";
      }
      mert_tmp_file<<"|||| "<<riter_mm->first<<"\n";
    }

    multimap<float, pair<string, vector<float> > > mm_score_mert_candidate;
    for (map<string, vector<float> >::const_iterator iter_map_sf = iter_vec_ss->mert_candidate_.begin(); iter_map_sf != iter_vec_ss->mert_candidate_.end(); ++iter_map_sf) {
      if (iter_vec_ss->mert_candidate_current_.find(iter_map_sf->first) != iter_vec_ss->mert_candidate_current_.end()) {
        continue;
      } else {
        float score = 0.0f;
        int feature_id = 0;
        for (vector<FeatureValue>::const_iterator iter_v_f = config.features.feature_values_.begin(); iter_v_f != config.features.feature_values_.end(); ++iter_v_f) {
          ++feature_id;
          score += iter_v_f->weight_ * iter_map_sf->second.at(feature_id - 1);
        }
        if (config.use_context_sensitive_wd_) {
          ++feature_id;
          score += config.features.v_context_sensitive_wd_feature_.at(0).weight_ * iter_map_sf->second.at(feature_id - 1);
          ++feature_id;
          score += config.features.v_context_sensitive_wd_feature_.at(1).weight_ * iter_map_sf->second.at(feature_id - 1);
        }
        mm_score_mert_candidate.insert(make_pair(score, make_pair(iter_map_sf->first, iter_map_sf->second)));
      }
    }

    for (multimap<float, pair<string, vector<float> > >::reverse_iterator riter_mm = mm_score_mert_candidate.rbegin(); riter_mm != mm_score_mert_candidate.rend(); ++riter_mm) {
      mert_tmp_file<<riter_mm->second.first<<" |||| ";
      int feature_id = 0;
      for (vector<float>::iterator iter_v = riter_mm->second.second.begin(); iter_v != riter_mm->second.second.end(); ++iter_v) {
        ++feature_id;
        mert_tmp_file<<feature_id<<":"<<*iter_v<<" ";
      }
      mert_tmp_file<<"|||| "<<riter_mm->first<<"\n";
    }

    mert_tmp_file<<"===============================\n";
    iter_vec_ss->mert_candidate_current_.clear();
  }

  mert_tmp_file.close();
  return true;
}


/*
 * $Name:
 * $Function:
 * $Date:
 */
bool PhraseBasedITGDecoder::WriteMertTmpOld(const Configuration &config, const Hypothesis &hypothesis, string &translation, ofstream &out_file) {
  out_file<<translation<<" |||| ";
  int count = 0;
  float score = 0;
  for (vector<float>::const_iterator iter = hypothesis.feature_values_.begin(); iter != hypothesis.feature_values_.end(); ++ iter) {
    out_file<<count+1<<":"<<*iter<<" ";
    score += config.features.feature_values_.at(count).weight_ * (*iter);
    ++count;
  }
  if (config.use_context_sensitive_wd_) {
//    out_file<<count+1<<":"<<hypothesis.context_sensitive_wd_values_<<" ";
//    score += config.features.context_sensitive_wd_feature_.weight_ * hypothesis.context_sensitive_wd_values_;
    out_file<<count + 1<<":"<<hypothesis.v_context_sensitive_wd_values_.at(0)<<" ";
    score += config.features.v_context_sensitive_wd_feature_.at(0).weight_ * hypothesis.v_context_sensitive_wd_values_.at(0);
    ++count;
    out_file<<count + 1<<":"<<hypothesis.v_context_sensitive_wd_values_.at(1)<<" ";
    score += config.features.v_context_sensitive_wd_feature_.at(1).weight_ * hypothesis.v_context_sensitive_wd_values_.at(1);
    ++count;
  }
  out_file<<"|||| "<<score<<"\n";
  return true;
}


/*
 * $Name:
 * $Function:
 * $Date:
 */
bool PhraseBasedITGDecoder::OutputTranslationResults (const Configuration &config, const Cells &cells, SourceSentence &source_sentence, ofstream &outFile, ofstream &logFile) {
  WriteOutputtedFile (config, cells, source_sentence, outFile);
  WriteLogFile (cells, source_sentence, logFile);
  return true;
}


/*
 * $Name:
 * $Function
 * $Date:
 */
bool PhraseBasedITGDecoder::WriteOutputtedFile (const Configuration  &config, const Cells &cells, SourceSentence &source_sentence, ofstream &outFile) {
  // write output file
  size_t counter = 0;
  for (CELL::const_iterator riter = cells.cells.at(0).at(source_sentence.cell_span_.size()).begin(); riter != cells.cells.at(0).at(source_sentence.cell_span_.size()).end(); ++ riter) {
    ++counter;
    if (counter <= config.nbest_) {
      size_t pos = 0;
      string tmpVecRes("");
      for (vector< TranslationLog >::const_iterator iter = riter->translation_log_.begin(); iter != riter->translation_log_.end(); ++ iter) {
        // write output file



        if (pos != 0 && pos != (riter->translation_log_.size() - 1)) {
          string tmpTransRes = iter->translation_result_;
          for (size_t iPos = iter->start_position_; iPos != iter->end_position_; ++iPos) {
            if (source_sentence.generalization_informations_.find(make_pair(iPos, iPos + 1)) != source_sentence.generalization_informations_.end()) {
              GENEINFOIT_CONST iterGeneBeg = source_sentence.generalization_informations_.lower_bound(make_pair(iPos, iPos + 1));
              GENEINFOIT_CONST iterGeneEnd = source_sentence.generalization_informations_.upper_bound(make_pair(iPos, iPos + 1));
              if (iterGeneBeg != iterGeneEnd) {
                if (iterGeneBeg->second.symbol_ == "$number" || \
                    iterGeneBeg->second.symbol_ == "$date"   || \
                    iterGeneBeg->second.symbol_ == "$time") {
                  size_t posOfGene = tmpTransRes.find(iterGeneBeg->second.symbol_);
                  if (posOfGene != string::npos) {
                    tmpTransRes = tmpTransRes.replace(posOfGene, iterGeneBeg->second.symbol_.size(), iterGeneBeg->second.translation_result_);
                  }
                } else if (iterGeneBeg->second.symbol_ == "$forced" || iterGeneBeg->second.symbol_ == "$literal") {
                  tmpTransRes = iterGeneBeg->second.translation_result_;
                }
              }
            }
          }
          if (!config.output_oov_) {
            if (!tmpTransRes.empty()) {
              if (tmpTransRes.at( 0 ) != '<' || tmpTransRes.at( tmpTransRes.size() - 1 ) != '>') {
                tmpVecRes += tmpTransRes + " ";
              }
            }
          } else {
            tmpVecRes += tmpTransRes + " ";
          }
        }
        ++pos;
      }    
      if (tmpVecRes != "") {
        tmpVecRes = tmpVecRes.substr(0, tmpVecRes.size() - 1);
      }
      if (config.mert_flag_) {
//      source_sentence.SetMertCandidate(config, riter->feature_values_, riter->context_sensitive_wd_values_, tmpVecRes);
//      source_sentence.SetMertCandidateCurrent(config, riter->feature_values_, riter->context_sensitive_wd_values_, tmpVecRes);
        source_sentence.SetMertCandidateCurrent(config, riter->feature_values_, riter->v_context_sensitive_wd_values_, tmpVecRes);
        WriteMertTmpOld(config, *riter, tmpVecRes, outFile);
      } else {
        outFile<<tmpVecRes<<"\n";
      }
    } else {
      break;
    }
  }
  if (counter == 0) {
    outFile<<"\n";
  }
  if (config.nbest_ > 1) {
    outFile<<"===============================\n";
  }
  return true;
}


/*
 * $Name:
 * $Function:
 * $Date:
 */
bool PhraseBasedITGDecoder::WriteLogFile(const Cells &cells , const SourceSentence &source_sentence, ofstream &logFile) {
  for ( vector< string >::const_iterator iter = source_sentence.cell_span_.begin(); iter != source_sentence.cell_span_.end(); ++ iter ) {
    logFile<<*iter<<" ";
  }
  logFile<<"\n";
  size_t counter = 0;
  for ( CELL::const_iterator riter = cells.cells.at( 0 ).at( source_sentence.cell_span_.size() ).begin(); \
        riter != cells.cells.at( 0 ).at( source_sentence.cell_span_.size() ).end(); ++ riter ) {
    ++counter;
    logFile<<counter                   <<" ||| "
           <<riter->translation_result_<<" ||| "
           <<riter->feature_score_except_lm_ + riter->language_model_score_<<"\n";

    for ( vector< TranslationLog >::const_iterator iter = riter->translation_log_.begin(); iter != riter->translation_log_.end(); ++ iter ) {
      logFile<<"["<<iter->start_position_<<","<<iter->end_position_<<"] ";
      for ( size_t i = iter->start_position_; i < iter->end_position_; ++i ) {
        logFile<<source_sentence.cell_span_.at( i )<<" ";
      }
      logFile<<"=> "<<iter->translation_result_;
      for ( size_t iPos = iter->start_position_; iPos < iter->end_position_; ++iPos ) {
        if ( source_sentence.generalization_informations_.find( make_pair( iPos, iPos + 1 ) ) != source_sentence.generalization_informations_.end() ) {
          GENEINFOIT_CONST iterGeneBeg = source_sentence.generalization_informations_.lower_bound( make_pair( iPos, iPos + 1 ) );
          GENEINFOIT_CONST iterGeneEnd = source_sentence.generalization_informations_.upper_bound( make_pair( iPos, iPos + 1 ) );
          while ( iterGeneBeg != iterGeneEnd ) {
            logFile<<" ||| "<<iterGeneBeg->second.translation_result_;
            ++iterGeneBeg;
          }
        }
      }
      logFile<<"\n";
    }
    logFile<<"\n";
  }

  if ( counter == 0 ) {
    logFile<<"=========================================\n";
  }
  logFile<<"=========================================\n";
  return true;
}


/*
 * $Name:
 * $Function:
 * $Date:
 */
bool PhraseBasedITGDecoder::GetFeatureWeight(Configuration &config, string &fn, multimap<float, vector<float> > &bleuAndScores) {
  ifstream inFn(fn.c_str());
  string line;
  vector<string> tmpWeights;
  float curBleu = 0;
  vector<float> curWeight;
  while (getline(inFn, line)) {
    if (line.find("Before optimization - BLEU") != string::npos) {
      vector<string> tmpVec;
      Split (line, ' ', tmpVec);
      curBleu = (float)atof(tmpVec.at( 4 ).c_str());
      continue;
    }

    if (line.find("Final Weights") != string::npos) {
      Split(line, ' ', tmpWeights);
      break;
    } else {
      continue;
    }
  }

  if (tmpWeights.size() > 3) {
    if (config.use_context_sensitive_wd_) {
      for (int i = 0; i < tmpWeights.size() - 3 - config.features.cswd_feature_number_; ++ i) {
        curWeight.push_back(config.features.feature_values_.at(i).weight_);
        config.features.feature_values_.at(i).weight_ = (float)atof(tmpWeights.at(i + 3).c_str());
      }

//      curWeight.push_back(config.features.context_sensitive_wd_feature_.weight_);
//      config.features.context_sensitive_wd_feature_.weight_ = (float)atof(tmpWeights.at(tmpWeights.size() - 1).c_str());
      curWeight.push_back(config.features.v_context_sensitive_wd_feature_.at(0).weight_);
      curWeight.push_back(config.features.v_context_sensitive_wd_feature_.at(1).weight_);
      config.features.v_context_sensitive_wd_feature_.at(0).weight_ = (float)atof(tmpWeights.at(tmpWeights.size() - 2).c_str());
      config.features.v_context_sensitive_wd_feature_.at(1).weight_ = (float)atof(tmpWeights.at(tmpWeights.size() - 1).c_str());

    } else {
      for ( int i = 0; i < tmpWeights.size() - 3; ++ i ) {
        curWeight.push_back( config.features.feature_values_.at( i ).weight_ );
        config.features.feature_values_.at( i ).weight_ = ( float )atof( tmpWeights.at( i + 3 ).c_str() );
      }
    }
  }
  bleuAndScores.insert( make_pair( curBleu, curWeight ) );
  if ( config.features.feature_values_.size() != 0 ) {
    cerr<<"Inputted  Weights ("<<curWeight.size()<<") : ";
    for ( vector< float >::iterator iter = curWeight.begin(); iter != curWeight.end(); ++iter ) {
      cerr<<*iter<<" ";
    }
    cerr<<"\n";
    if (config.use_context_sensitive_wd_) {
      cerr<<"Optimized Weights ("<<config.features.feature_values_.size() + config.features.cswd_feature_number_<<") : ";
    } else {
      cerr<<"Optimized Weights ("<<config.features.feature_values_.size()<<") : ";
    }
    for ( vector< FeatureValue >::iterator iter = config.features.feature_values_.begin(); iter != config.features.feature_values_.end(); ++ iter ) {
      cerr<<iter->weight_<<" ";
    }
    if (config.use_context_sensitive_wd_) {
//      cerr<<config.features.context_sensitive_wd_feature_.weight_<<" ";
      cerr<<config.features.v_context_sensitive_wd_feature_.at(0).weight_<<" "
          <<config.features.v_context_sensitive_wd_feature_.at(1).weight_<<" ";
    }
    cerr<<"\n\n";
  }

  inFn.close();
  return true;
}


/*
 * $Name:
 * $Function:
 * $Date:
 */
bool PhraseBasedITGDecoder::GenerateTranslationResults(Configuration &config, Model &model, multimap< float, vector< float > > &bleuAndScores) {
  cerr<<"Generate the final translation results...\n";
  if (config.use_context_sensitive_wd_) {
    cerr<<"Inputted  Weights ("<<config.features.feature_values_.size() + 1<<") : ";
  } else {
    cerr<<"Inputted  Weights ("<<config.features.feature_values_.size()<<") : ";
  }

  ofstream conf( config.config_file_.c_str(), ios::app );
  conf<<"\n## MAX BLEU SCORE="<<bleuAndScores.rbegin()->first<<"\n";
  conf<<"param=\"weights\"                          value=\"";
  conf<<setiosflags( ios::fixed )<<setprecision( 3 );
  if ( bleuAndScores.size() != 0 ) {
    multimap< float, vector< float > >::reverse_iterator riter = bleuAndScores.rbegin();
    if (config.use_context_sensitive_wd_) {
      for (int i = 0; i < config.features.feature_values_.size(); ++i) {
        config.features.feature_values_.at(i).weight_ = riter->second.at(i);
        cerr<<config.features.feature_values_.at(i).weight_<<" ";
        if (i == 0) {
          conf<<config.features.feature_values_.at(i).weight_;
        } else {
          conf<<" "<<config.features.feature_values_.at(i).weight_;
        }
      }
      config.features.v_context_sensitive_wd_feature_.at(0).weight_ = riter->second.at(riter->second.size() - 2);
      config.features.v_context_sensitive_wd_feature_.at(1).weight_ = riter->second.at(riter->second.size() - 1);
      cerr<<config.features.v_context_sensitive_wd_feature_.at(0).weight_<<" "
          <<config.features.v_context_sensitive_wd_feature_.at(1).weight_<<"\n";
      conf<<"\"\n\n";
      conf<<"param=\"context-sensitive-wd-weights\"     value=\""
          <<config.features.v_context_sensitive_wd_feature_.at(0).weight_<<" "
          <<config.features.v_context_sensitive_wd_feature_.at(1).weight_<<"\"\n\n";
    } else {
      int i = 0;
      for ( vector< float >::iterator iter = riter->second.begin(); iter != riter->second.end(); ++iter ) {
        config.features.feature_values_.at( i ).weight_ = *iter;
        cerr<<*iter<<" ";
        if ( i == 0 ) {
          conf<<*iter;
        } else {
          conf<<" "<<*iter;
        }
        ++i;
      }
      conf<<"\"\n";
      cerr<<"\n";
    }
  }
  conf.close();
  Decode( config, model );
  return true;
}


bool PhraseBasedITGDecoder::PrintPhraseBasedDecoderLogo() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# PB-Decoder for Statistical Machine Translation                  #\n"
      <<"#                                             Version 0.0.1       #\n"
      <<"#                                             NEUNLPLab/YAYI corp #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<flush;
  return true;
}


bool PhraseBasedITGDecoder::PrintPhraseBasedDecoderBinaLogo() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# PB-Decoder (Binary) for Statistical Machine Translation         #\n"
      <<"#                                             Version 0.0.1       #\n"
      <<"#                                             NEUNLPLab/YAYI corp #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<flush;
  return true;
}


bool PhraseBasedITGDecoder::PrintPhraseBasedTrainingLogo() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# Training for PB-Decoder (MERT) for SMT                          #\n"
      <<"#                                             Version 0.0.1       #\n"
      <<"#                                             NEUNLPLab/YAYI corp #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<flush;
  return true;
}


}

