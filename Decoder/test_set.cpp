/*
* $Id:
* 0024
*
* $File:
* test_set.cpp
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
* 2014-01-07,21:00, English string recognition and translation.
* 2012-12-13,14:32
*/

#include "test_set.h"


namespace decoder_test_set{

/*
* $Name: Init
* $Function:
* $Date: 2013-05-27
*/
bool SourceSentence::Init(string &srcString) {
  vector< string > domain;
  SplitWithStr( srcString, " |||| ", domain );
  if ( domain.size() != 0 ) {
    domain.at( 0 ) = "<s> " + domain.at( 0 ) + " </s>";
  } else {
    domain.push_back( "<s> </s>" );
  }

  Split( domain.at( 0 ), ' ', cell_span_ );

  int tmpPunctPos = 0;
  for ( vector< string >::iterator iter = cell_span_.begin(); iter != cell_span_.end(); ++iter ) {
    if (*iter == ":" || *iter == "," || *iter == "?" || *iter == "." || *iter == "!") {
      punctuation_position_.insert( tmpPunctPos );
    }
    ++tmpPunctPos;
  }

  if ( domain.size() != 1 && domain.at( 1 ).size() != 0 ) {
    ParseGeneralizationInfo( domain.at( 1 ) );
  }
  return true;
}


/*
* $Name: Init
* $Function:
* $Date: 2013-05-27
*/
bool SourceSentence::Init(string &srcString, MeReorderingTable &meReorderingTable) {
  ClearIllegalChar( srcString );
  vector< string > domain;
  SplitWithStr( srcString, " |||| ", domain );
  if ( domain.size() != 0 ) {
    domain.at( 0 ) = "<s> " + domain.at( 0 ) + " </s>";
  } else {
    domain.push_back( "<s> </s>" );
  }

  Split( domain.at( 0 ), ' ', cell_span_ );
  int tmpPunctPos = 0;
  for ( vector< string >::iterator iter = cell_span_.begin(); iter != cell_span_.end(); ++iter ) {
    if ( meReorderingTable.meTableSrc.find( *iter ) != meReorderingTable.meTableSrc.end() ) {
      vector< float > meReorderScore( meReorderingTable.meTableSrc[ *iter ] );
      me_reorder_scores_.push_back( meReorderScore );
    } else {
      vector< float > meReorderScore( 8, 0.0 );
      me_reorder_scores_.push_back( meReorderScore );
    }
    // here must be the same with LINE 405 in this file
    if (*iter == ":" || *iter == "," || *iter == "?" || *iter == "." || *iter == "!") {
      punctuation_position_.insert( tmpPunctPos );
    }
    ++tmpPunctPos;
  }

  if ( domain.size() != 1 && domain.at( 1 ).size() != 0 ) {
    ParseGeneralizationInfo( domain.at( 1 ) );
  }
  return true;
}


/*
 * $Name: Init
 * $Function:
 * $Date: 2013-05-27
 */
bool SourceSentence::Init(string &srcString, MeReorderingTable &meReorderingTable, int &sentence_id) {
  sentence_id_ = sentence_id;
  Init(srcString, meReorderingTable);
  return true;
}


/*
 * $Name: Init
 * $Function:
 * $Date: 2013-05-27
 */
bool SourceSentence::Init (string &srcString, MeReorderingTable &meReorderingTable, ContextSensitiveWd &context_sensitive_model, int &sentence_id) {
  ClearIllegalChar (srcString);
  vector<string> domain;
  SplitWithStr (srcString, " |||| ", domain);
  if (domain.size() != 0) {
    domain.at(0) = "<s> " + domain.at( 0 ) + " </s>";
  } else {
    domain.push_back("<s> </s>");
  }

  Split(domain.at( 0 ), ' ', cell_span_);
  sentence_id_ = sentence_id;

  InitMeReorderingScores(meReorderingTable);
  InitContextSensitiveWdScores (context_sensitive_model);

  if (domain.size() != 1 && domain.at(1).size() != 0) {
    ParseGeneralizationInfo (domain.at(1));
  }
  return true;
}


bool SourceSentence::InitMeReorderingScores (MeReorderingTable &meReorderingTable) {
  int tmpPunctPos = 0;
  for (vector< string >::iterator iter = cell_span_.begin(); iter != cell_span_.end(); ++iter) {
    if (meReorderingTable.meTableSrc.find(*iter) != meReorderingTable.meTableSrc.end()) {
      vector<float> meReorderScore(meReorderingTable.meTableSrc[*iter]);
      me_reorder_scores_.push_back(meReorderScore);
    } else {
      vector<float> meReorderScore(8, 0.0);
      me_reorder_scores_.push_back(meReorderScore);
    }
    // here must be the same with LINE 405 in this file
    if (*iter == ":" || *iter == "," || *iter == "?" || *iter == "." || *iter == "!") {
      punctuation_position_.insert(tmpPunctPos);
    }
    ++tmpPunctPos;
  }
  return true;
}


bool SourceSentence::InitContextSensitiveWdScores (ContextSensitiveWd &context_sensitive_model) {
  int src_word_position = 0;
  int base_position = 14;
  for (vector<string>::iterator iter = cell_span_.begin(); iter != cell_span_.end(); ++iter) {
    ++ src_word_position;
    float w0 = 0.0f;
    float w1 = 0.0f;

    string w_minus2;
    string w_minus1;
    string w = *iter;

    if (src_word_position >= 3) {
      w_minus2 = cell_span_.at(src_word_position - 3);
      w_minus1 = cell_span_.at(src_word_position - 2);
    } else if (src_word_position == 2) {
      w_minus2 = "NULL";
      w_minus1 = cell_span_.at(src_word_position - 2);
    } else if (src_word_position == 1) {
      w_minus2 = "NULL";
      w_minus1 = "NULL";
    }

    if (context_sensitive_model.context_sensitive_wd_src_.find(w_minus2) != context_sensitive_model.context_sensitive_wd_src_.end()) {
      w0 += context_sensitive_model.context_sensitive_wd_src_[w_minus2].at(0);
      w1 += context_sensitive_model.context_sensitive_wd_src_[w_minus2].at(0 + base_position);
    }
    if (context_sensitive_model.context_sensitive_wd_src_.find(w_minus1) != context_sensitive_model.context_sensitive_wd_src_.end()) {
      w0 += context_sensitive_model.context_sensitive_wd_src_[w_minus1].at(1);
      w1 += context_sensitive_model.context_sensitive_wd_src_[w_minus1].at(1 + base_position);
    }
    if (context_sensitive_model.context_sensitive_wd_src_.find(w) != context_sensitive_model.context_sensitive_wd_src_.end()) {
      w0 += context_sensitive_model.context_sensitive_wd_src_[w].at(2);
      w1 += context_sensitive_model.context_sensitive_wd_src_[w].at(2 + base_position);
    }

    string w_plus1;
    string w_plus2;
    if (src_word_position <= cell_span_.size() - 2) {
      w_plus1 = cell_span_.at(src_word_position);
      w_plus2 = cell_span_.at(src_word_position + 1);
    } else if (src_word_position == cell_span_.size() - 1) {
      w_plus1 = cell_span_.at(src_word_position);
      w_plus2 = "NULL";
    } else if (src_word_position == cell_span_.size()) {
      w_plus1 = "NULL";
      w_plus2 = "NULL";
    }

    if (context_sensitive_model.context_sensitive_wd_src_.find(w_plus1) != context_sensitive_model.context_sensitive_wd_src_.end()) {
      w0 += context_sensitive_model.context_sensitive_wd_src_[w_plus1].at(3);
      w1 += context_sensitive_model.context_sensitive_wd_src_[w_plus1].at(3 + base_position);
    }
    if (context_sensitive_model.context_sensitive_wd_src_.find(w_plus2) != context_sensitive_model.context_sensitive_wd_src_.end()) {
      w0 += context_sensitive_model.context_sensitive_wd_src_[w_plus2].at(4);
      w1 += context_sensitive_model.context_sensitive_wd_src_[w_plus2].at(4 + base_position);
    }

    float prob0 = log(exp(w0) / (exp(w0) + exp(w1)));
    float prob1 = log(exp(w1) / (exp(w0) + exp(w1)));

    context_sensitive_wd_scores_.push_back(make_pair(prob0, prob1));
  }
  return true;
}


/*
 * $Name: SetMertCandidate
 * $Function: Set Mert Candidate
 * $Date: 2013-05-27
 */
bool SourceSentence::SetMertCandidate (const Configuration &config, const vector<float> &v_feature_value, const float &context_sentence_wd_value, string &translation) {
  if (mert_candidate_.find(translation) == mert_candidate_.end()) {
    mert_candidate_.insert(make_pair(translation, v_feature_value));
    if (config.use_context_sensitive_wd_) {
      mert_candidate_[translation].push_back(context_sentence_wd_value);
    }
  }
  return true;
}


/*
 * $Name: SetMertCandidate
 * $Function: Set Mert Candidate
 * $Date: 2015-07-14
 */
bool SourceSentence::SetMertCandidate (const Configuration &config, const vector<float> &v_feature_value, string translation) {
  if (mert_candidate_.find(translation) == mert_candidate_.end()) {
    mert_candidate_.insert(make_pair(translation, v_feature_value));
  }
  return true;
}




/*
 * $Name: 
 * $Function: 
 * $Date: 2013-05-27
 */
bool SourceSentence::SetMertCandidateCurrent (const Configuration &config, const vector<float> &v_feature_value, const vector<float> &v_context_sentence_wd_value, string &translation) {
  if (mert_candidate_current_.find(translation) == mert_candidate_current_.end()) {
    mert_candidate_current_.insert(make_pair(translation, v_feature_value));
    if (config.use_context_sensitive_wd_) {
//      mert_candidate_current_[translation].push_back(context_sentence_wd_value);
      mert_candidate_current_[translation].push_back(v_context_sentence_wd_value.at(0));
      mert_candidate_current_[translation].push_back(v_context_sentence_wd_value.at(1));
    }
  }
  return true;
}


/*
* $Name: ParseGeneralizationInfo
* $Function: 
* $Date: 2013-05-27
*/
bool SourceSentence::ParseGeneralizationInfo(string &info) {
  vector< string > infos;

  string line="";
  for ( string::iterator iter  = info.begin(); iter != info.end(); ++iter ) {
    if ( *iter == '{' ) {
      line.clear();
      continue;
    } else if ( *iter == '}' ) {
      infos.push_back( line );
      line.clear();
      continue;
    } else {
      line += *iter;
      continue;
    }
  }

  for ( vector< string >::iterator iter  = infos.begin(); iter != infos.end(); ++ iter ) {
    vector< string > domain;
    SplitWithStr( *iter, " ||| ", domain );
    if ( domain.size() < 5 || domain.at( 0 ) == "" || domain.at( 1 ) == "" || \
         domain.at( 2 ) == "" || domain.at( 3 ) == "" || domain.at( 4 ) == "" ) {
      continue;
    }

    size_t startPos = ( size_t )( atoi( domain.at( 0 ).c_str() ) + 1 );
    size_t endPos   = ( size_t )( atoi( domain.at( 1 ).c_str() ) + 2 );
    string transInternal;

    size_t symbolType    = 2;
    size_t transIntCount = 1;

    if (domain.at( 3 ) == "$number" || domain.at( 3 ) == "$date" || domain.at( 3 ) == "$time") {
      if (cell_span_.at( startPos ) != "$number" && \
          cell_span_.at( startPos ) != "$date"   && \
          cell_span_.at( startPos ) != "$time") {
        transInternal = domain.at( 2 );
      } else {
        transInternal = domain.at( 3 );
        symbolType = 0;
      }
    } else if (domain.at( 3 ) == "$literal" || domain.at( 3 ) == "$forced") {
      transInternal = domain.at( 2 );
      symbolType = 1;
      vector< string > tmpVec;
      Split( domain.at( 2 ), ' ', tmpVec );
      transIntCount = tmpVec.size();
    } else {
      transInternal = domain.at( 2 );
      vector< string > tmpVec;
      Split( domain.at( 2 ), ' ', tmpVec );
      transIntCount = tmpVec.size();
    }

    if( startPos < endPos && endPos < cell_span_.size() ) {
      GeneralizationInfo gi( startPos, endPos , domain.at( 2 ), domain.at( 3 ), symbolType );
      generalization_informations_.insert( make_pair( make_pair( startPos, endPos ), gi ) );
    }
  }
  return true;
}


/*
* $Name: Init
* $Function:
* $Date: 2013-05-27
*/
bool TestSet::Init(Configuration &config) {
  LoadTestSet( config );
  return true;
}


/*
* $Name: Init
* $Function: Init test set.
* $Date: 2013-05-27
*/
bool TestSet::Init (Configuration &config, MeReorderingTable &meReorderingTable) {
  LoadTestSet (config, meReorderingTable);
  return true;
}


/*
 * $Name: Init
 * $Function: Init test set.
 * $Date: 2013-05-27
 */
bool TestSet::Init (Configuration &config, MeReorderingTable &meReorderingTable, ContextSensitiveWd &context_sensitive_model) {
  LoadTestSet (config, meReorderingTable, context_sensitive_model);
  return true;
}



/*
* $Name: LoadTestSet
* $Function:
* $Date: 2013-05-27
*/
bool TestSet::LoadTestSet(Configuration &config) {
  ifstream inFile( config.testFile.c_str() );
  if ( !inFile ) {
    cerr<<"ERROR: Can not open file \""
        <<config.testFile
        <<"\".\n"
        <<flush;
    exit( 1 );
  } else {
    cerr<<"Loading Test-Set\n"
        <<"  >> From File: "<<config.testFile<<"\n"
        <<flush;
  }

  clock_t start, finish;
  start = clock();

  string lineOfInFile;
  size_t lineNo = 0;
  size_t srcNo  = 0;
  while ( getline( inFile, lineOfInFile ) ) {
    ClearIllegalChar( lineOfInFile );
    RmEndSpace      ( lineOfInFile );
    RmStartSpace    ( lineOfInFile );

    SourceSentence sourceSentence;
    sourceSentence.Init( lineOfInFile );

    test_set_.push_back( sourceSentence );

    finish = clock();
    if( lineNo % 100000 == 0 )
      cerr<<"\r  Processed "<<lineNo<<" lines. [src="<<srcNo<<"] [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   "
          <<flush;
    ++srcNo;
    ++lineNo;
  }
  finish = clock();
  cerr<<"\r  Processed "<<lineNo<<" lines. [src="<<srcNo<<"] [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   \n"
      <<"  Done!\n\n"<<flush;

  inFile.close();
  return true;
}


/*
 * $Name: LoadTestSet
 * $Function:
 * $Date: 2013-05-27
 */
bool TestSet::LoadTestSet (Configuration &config, MeReorderingTable &meReorderingTable) {
  ifstream inFile (config.testFile.c_str());
  if (!inFile) {
    cerr<<"ERROR: Can not open file \""
        <<config.testFile
        <<"\".\n"
        <<flush;
    exit(1);
  } else {
    cerr<<"Loading Test-Set\n"
        <<"  >> From File: "<<config.testFile<<"\n"
        <<flush;
  }

  clock_t start, finish;
  start = clock();

  string lineOfInFile;
  int lineNo = 0;
  int srcNo = 0;
  while (getline(inFile, lineOfInFile)) {
    if (config.mert_flag_) {
      if (lineNo % (config.nref_ + 2) != 0) {
        ++lineNo;
        continue;
      }
    }
    ClearIllegalChar (lineOfInFile);
    RmEndSpace (lineOfInFile);
    RmStartSpace (lineOfInFile);
    
#ifdef __DEBUG_TEST_SET__
    ofstream tmp_output("tmp_output.txt", ios::app);
    tmp_output<<"0 ||| "<<lineOfInFile<<"\n"<<flush;
    tmp_output.close();
#endif

    if (config.english_string_) {
      RecognizeAndTranslateEnString (lineOfInFile);
#ifdef __DEBUG_TEST_SET__
      ofstream tmp_output ("tmp_output.txt", ios::app);
      tmp_output<<"1 ||| "<<lineOfInFile<<"\n"<<flush;
      tmp_output.close();
#endif
    }

    SourceSentence sourceSentence;
    sourceSentence.Init (lineOfInFile, meReorderingTable, srcNo);

    test_set_.push_back (sourceSentence);

    finish = clock();
    if ( lineNo % 100000 == 0 ) {
      cerr<<"\r  Processed "<<lineNo<<" lines. [src="<<srcNo<<"] [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   "
          <<flush;
    }
    ++srcNo;
    ++lineNo;
  }
  finish = clock();
  cerr<<"\r  Processed "<<lineNo<<" lines. [src="<<srcNo<<"] [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   \n"
      <<"  Done!\n\n"<<flush;

  inFile.close();
  return true;
}


/*
 * $Name: LoadTestSet
 * $Function:
 * $Date: 2013-05-27
 */
bool TestSet::LoadTestSet (Configuration &config, MeReorderingTable &meReorderingTable, ContextSensitiveWd &context_sensitive_model) {
  ifstream inFile(config.testFile.c_str());
  if (!inFile) {
    cerr<<"ERROR: Can not open file \""
        <<config.testFile
        <<"\".\n"
        <<flush;
    exit( 1 );
  } else {
    cerr<<"Loading Test-Set\n"
        <<"  >> From File: "<<config.testFile<<"\n"
        <<flush;
  }

  clock_t start, finish;
  start = clock();

  string lineOfInFile;
  int lineNo = 0;
  int srcNo  = 0;
  while (getline(inFile, lineOfInFile)) {
    if (config.mert_flag_) {
      if (lineNo % (config.nref_ + 2) != 0) {
        ++lineNo;
        continue;
      }
    }
    ClearIllegalChar (lineOfInFile);
    RmEndSpace (lineOfInFile);
    RmStartSpace (lineOfInFile);
    
#ifdef __DEBUG_TEST_SET__
    ofstream tmp_output("tmp_output.txt", ios::app);
    tmp_output<<"0 ||| "<<lineOfInFile<<"\n"<<flush;
    tmp_output.close();
#endif

    if (config.english_string_) {
      RecognizeAndTranslateEnString(lineOfInFile);
#ifdef __DEBUG_TEST_SET__
      ofstream tmp_output("tmp_output.txt", ios::app);
      tmp_output<<"1 ||| "<<lineOfInFile<<"\n"<<flush;
      tmp_output.close();
#endif
    }

    SourceSentence sourceSentence;
    sourceSentence.Init (lineOfInFile, meReorderingTable, context_sensitive_model, srcNo);

    test_set_.push_back( sourceSentence );

    finish = clock();
    if ( lineNo % 100000 == 0 ) {
      cerr<<"\r  Processed "<<lineNo<<" lines. [src="<<srcNo<<"] [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   "
          <<flush;
    }
    ++srcNo;
    ++lineNo;
  }
  finish = clock();
  cerr<<"\r  Processed "<<lineNo<<" lines. [src="<<srcNo<<"] [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   \n"
      <<"  Done!\n\n"<<flush;

  inFile.close();
  return true;
}





/*
* $Name: RecognizeAndTranslateEnString
* $Function: For Chinese-2-English Translation. For example, English string "A B C D E F G" should be translate to "A B C D E F G".
* $Date: 2014-01-07
*/
bool TestSet::RecognizeAndTranslateEnString( string &input_sentence ) {
  if ( input_sentence == "" ) {
    return true;
  }

  vector< string > domains;
  SplitWithStr( input_sentence, " |||| ", domains );

  vector< string > words_vector;
  Split( domains.at( 0 ), ' ', words_vector );

  bool en_str_flag = false;
  int en_str_start_position = 0;
  int en_str_end_position = 0;
  int current_position = 0;
  string en_str_translation;

  for ( vector< string >::iterator iter = words_vector.begin(); iter != words_vector.end(); ++iter ) {
    if ( IsEnglishWord( *iter ) ) {
      if( !en_str_flag ) {
        en_str_translation = *iter;
        en_str_start_position = current_position;
        en_str_flag = true;
      } else {
        en_str_translation += " " + *iter;
        en_str_end_position = current_position;
      }
    } else {
      if ( en_str_end_position - en_str_start_position > 0 ) {
        string cur_generalization;
        cur_generalization = "{" + intToString( en_str_start_position ) + " ||| " + \
                             intToString( en_str_end_position ) + " ||| " + en_str_translation + \
                             " ||| $forced ||| " + en_str_translation + "}";
        domains.push_back( cur_generalization );
        en_str_flag = false;
        en_str_start_position = 0;
        en_str_end_position = 0;
        en_str_translation = "";
      } else {
        en_str_flag = false;
        en_str_start_position = 0;
        en_str_end_position = 0;
        en_str_translation = "";
      }
    }
    ++current_position;
  }

  if ( en_str_flag && en_str_end_position - en_str_start_position > 0 ) {
    string cur_generalization;
    cur_generalization = "{" + intToString( en_str_start_position ) + " ||| " + \
                         intToString( en_str_end_position ) + " ||| " + en_str_translation + \
                         " ||| $forced ||| " + en_str_translation + "}";
    domains.push_back( cur_generalization );
    en_str_flag = false;
    en_str_start_position = 0;
    en_str_end_position = 0;
    en_str_translation = "";
  }

  if ( domains.size() < 2 ) {
    return true;
  } else {
    input_sentence = domains.at( 0 ) + " |||| ";
    for( size_t pos = 1; pos < domains.size(); ++ pos ) {
      input_sentence += domains.at( pos );
    }
  }
  return true;
}

/*
* $Name: IsEnglishWord
* $Function: 
* $Date: 2014-01-07
*/
bool TestSet::IsEnglishWord( string &input_word ) {
  bool english_word_flag = true;
  if( input_word == "$number" || input_word == "$date" || input_word == "$time" ) {
    return false;
  }

  // here must be the same with LINE 89 in this file
  if(input_word == ":" || input_word == "," || input_word == "?" || input_word == "." || input_word == "!") {
    return false;
  }

  for ( string::iterator iter = input_word.begin(); iter != input_word.end(); ++iter ) {
    if ( !isalnum( ( unsigned char )*iter ) && !ispunct( ( unsigned char )*iter ) ) {
//    if ( !isalnum( ( unsigned char )*iter ) ) {
      english_word_flag = false;
      break;
    }
  }
  return english_word_flag;
}


}

