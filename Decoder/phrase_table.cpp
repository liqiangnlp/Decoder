/*
 * $Id:
 * 0016
 *
 * $File:
 * phrase_table.cpp
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
 * 2012-12-10,18:58
 */

#include "phrase_table.h"


namespace phrase_table {

bool PhraseTable::Init(Configuration &config) {
  InitTgtVocab (config);
  if (!config.phrase_table_binary_flag_) {
    if (config.use_context_sensitive_wd_) {
      InitPhraseTableContextSensitiveWd (config);
    } else {
      InitPhraseTable (config);
    }
  } else {
    InitPhraseTableBina (config);
  }
  return true;
}


bool PhraseTable::InitTgtVocab(Configuration &config) {
  ifstream inFile(config.target_vocab_file_.c_str());
  if (!inFile) {
    cerr<<"ERROR: Can not open file \""<<config.target_vocab_file_<<"\".\n"<<flush;
    exit(1);
  } else {
    cerr<<"Loading Target-Vocab-File\n"<<"  >> From File: "<<config.target_vocab_file_<<"\n"<<flush;
  }
  clock_t start, finish;
  start = clock();
  string lineOfInFile;
  int    wid = 0;
  while (getline(inFile, lineOfInFile)) {
    ClearIllegalChar(lineOfInFile);
    RmEndSpace(lineOfInFile);
    RmStartSpace(lineOfInFile);
    tgtVocab.insert(make_pair(lineOfInFile, wid));
    finish = clock();
    if (wid % 100000 == 0) {
      cerr<<"\r  Processed "<<wid<<" lines. [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   "<<flush;
    }
    ++wid;
  }
  finish = clock();
  cerr<<"\r  Processed "<<wid<<" lines. [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   \n"<<"  Done!\n\n"<<flush;
  inFile.close();
  return true;
}


bool PhraseTable::InitPhraseTable (Configuration &config) {
  ifstream inFile(config.phrase_table_file_.c_str());
  if (!inFile) {
    cerr<<"ERROR: Can not open file \""<<config.phrase_table_file_.c_str()<<"\".\n"<<flush;
    exit(1);
  } else {
    cerr<<"Loading Phrase-Table\n"<<"  >> From File: "<<config.phrase_table_file_<<"\n"<<flush;
  }
  clock_t start, finish;
  start = clock();
  string lineOfInFile;
  size_t lineNo = 0  ;
  size_t invalidNum = 0;
  while (getline(inFile, lineOfInFile)) {
    ++lineNo;
    ClearIllegalChar (lineOfInFile);
    RmEndSpace (lineOfInFile);
    RmStartSpace (lineOfInFile);

    vector<string> phraseContents;
    SplitWithStr (lineOfInFile, " ||| ", phraseContents);
    if (phraseContents.size() < 3) {
      cerr<<"  WARNING: Format error in line "<<lineNo<<"\n"<<flush;
      continue;
    }

    vector<string> scoresOfOptions;
    Split (phraseContents.at( 2 ), ' ', scoresOfOptions);
    if (scoresOfOptions.size() < 6) {
      cerr<<"  WARNING: Format error in line "<<lineNo<<"\n"<<flush;
      continue;
    }

    if (!checkGeneration (phraseContents.at(0), phraseContents.at(1))) {
      ++invalidNum;
      continue;
    }

    if (!config.use_empty_translation_) {
      if (IsEmptyTranslation(phraseContents.at(1))) {
        continue;
      }
    }

    // waste time
    float proEGivenF        = (float)atof(scoresOfOptions[0].c_str());
    float lexEGivenF        = (float)atof(scoresOfOptions[1].c_str());
    float proFGivenE        = (float)atof(scoresOfOptions[2].c_str());
    float lexFGivenE        = (float)atof(scoresOfOptions[3].c_str());
    float naturalLogarithm  = (float)atof(scoresOfOptions[4].c_str());
    float biLexLinks        = (float)atof(scoresOfOptions[5].c_str());
    float empty_translation = 0.0f;

    if (config.use_empty_translation_ && IsEmptyTranslation (phraseContents.at(1))) {
      lexEGivenF = proEGivenF;
      lexFGivenE = proFGivenE;
      empty_translation = 1.0f;
      if (config.output_empty_translation_) {
          phraseContents.at(1) = "<-" + phraseContents.at(0) + "->";
      } else {
        phraseContents.at(1) = "";
      }
    }

    vector< float > freeFeatureValue(config.free_feature_, 0);
    if (config.free_feature_ != 0) {
      if ((config.free_feature_ <= scoresOfOptions.size() - 6) && \
          ( config.free_feature_ <= config.features.feature_values_.size() - 17)) {
        for (int i = 0; i < config.free_feature_; ++i) {
          freeFeatureValue.at(i) = (float)atof(scoresOfOptions.at(i + 6).c_str());
        }
      } else {
        cerr<<"ERROR: Parameter 'freefeature' in config file do not adapt to the phrasetable!\n";
        exit(1); 
      }
    }

    ScoresOfTranslateOption scoresOfTranslateOption (proEGivenF, lexEGivenF, proFGivenE, lexFGivenE, naturalLogarithm, biLexLinks, empty_translation, freeFeatureValue);
    TranslationOption translationOption(phraseContents.at(1), scoresOfTranslateOption);
    translationsOptions[phraseContents.at(0)].translationOptions.push_back(translationOption);
    finish = clock();
    if (lineNo % 100000 == 0) {
      cerr<<"\r  Processed "<<lineNo<<" lines. [INVALID="<<invalidNum<<"] [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   "<<flush;
    }
  }
  finish = clock();
  cerr<<"\r  Processed "<<lineNo<<" lines. [INVALID="<<invalidNum<<"] [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   \n"<<"  Done!\n\n"<<flush;
  inFile.close();
  addConstPhrase(config);
  return true;
}


bool PhraseTable::InitPhraseTableContextSensitiveWd (Configuration &config)  {
  ifstream inFile(config.phrase_table_file_.c_str());
  if (!inFile) {
    cerr<<"ERROR: Can not open file \""<<config.phrase_table_file_.c_str()<<"\".\n"<<flush;
    exit(1);
  } else {
    cerr<<"Loading Phrase-Table\n"<<"  >> From File: "<<config.phrase_table_file_<<"\n"<<flush;
  }
  clock_t start, finish;
  start = clock();
  string lineOfInFile;
  size_t lineNo = 0  ;
  size_t invalidNum = 0;
  while (getline(inFile, lineOfInFile)) {
    ++lineNo;
    ClearIllegalChar(lineOfInFile);
    RmEndSpace(lineOfInFile);
    RmStartSpace(lineOfInFile);

    vector<string> phraseContents;
    SplitWithStr(lineOfInFile, " ||| ", phraseContents);
    if (phraseContents.size() < 5) {
      cerr<<"  WARNING: Format error in line "<<lineNo<<"\n"<<flush;
      continue;
    }

    vector<string> scoresOfOptions;
    Split(phraseContents.at( 2 ), ' ', scoresOfOptions);
    if (scoresOfOptions.size() < 6) {
      cerr<<"  WARNING: Format error in line "<<lineNo<<"\n"<<flush;
      continue;
    }

    if (!checkGeneration(phraseContents.at(0), phraseContents.at(1))) {
      ++invalidNum;
      continue;
    }

    bool is_empty_translation = IsEmptyTranslation(phraseContents.at(1));
    if (!config.use_empty_translation_) {
      if (is_empty_translation) {
        continue;
      }
    }

    // waste time
    float proEGivenF        = (float)atof(scoresOfOptions[0].c_str());
    float lexEGivenF        = (float)atof(scoresOfOptions[1].c_str());
    float proFGivenE        = (float)atof(scoresOfOptions[2].c_str());
    float lexFGivenE        = (float)atof(scoresOfOptions[3].c_str());
    float naturalLogarithm  = (float)atof(scoresOfOptions[4].c_str());
    float biLexLinks        = (float)atof(scoresOfOptions[5].c_str());
    float empty_translation = 0.0f;

    if (config.use_empty_translation_ && is_empty_translation) {
      lexEGivenF = proEGivenF;
      lexFGivenE = proFGivenE;
      empty_translation = 1.0f;
      if (config.output_empty_translation_) {
        phraseContents.at(1) = "<-" + phraseContents.at(0) + "->";
      } else {
        phraseContents.at(1) = "";
      }
    }

    vector< float > freeFeatureValue(config.free_feature_, 0);
    if (config.free_feature_ != 0) {
      if ((config.free_feature_ <= scoresOfOptions.size() - 6) && \
          ( config.free_feature_ <= config.features.feature_values_.size() - 17)) {
        for (int i = 0; i < config.free_feature_; ++i) {
          freeFeatureValue.at(i) = (float)atof(scoresOfOptions.at(i + 6).c_str());
        }
      } else {
        cerr<<"ERROR: Parameter 'freefeature' in config file do not adapt to the phrasetable!\n";
        exit(1); 
      }
    }

    ScoresOfTranslateOption scoresOfTranslateOption( proEGivenF, lexEGivenF, proFGivenE, lexFGivenE, naturalLogarithm, biLexLinks, empty_translation, freeFeatureValue);

    vector<string> v_src_words;
    Split (phraseContents.at(0), ' ', v_src_words);
    vector<bool> v_unaligned_flag(v_src_words.size(), true);
    vector<string> v_aln;
    Split (phraseContents.at(4), ' ', v_aln);
    if (!is_empty_translation) {
      for (vector<string>::iterator iter = v_aln.begin(); iter != v_aln.end(); ++iter) {
        vector<string> v_src_tgt_aln;
        Split (*iter, '-', v_src_tgt_aln);
        if (v_src_tgt_aln.size() != 2) {
          cerr<<"  WARNING: Format error in line "<<lineNo<<"\n"<<flush;
          continue;
        }
        int src_position = atoi(v_src_tgt_aln.at(0).c_str());
        v_unaligned_flag.at(src_position) = false;
      }
    }

    TranslationOption translationOption(phraseContents.at(1), scoresOfTranslateOption, v_unaligned_flag);
    translationsOptions[phraseContents.at(0)].translationOptions.push_back(translationOption);
    finish = clock();
    if (lineNo % 100000 == 0) {
      cerr<<"\r  Processed "<<lineNo<<" lines. [INVALID="<<invalidNum<<"] [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   "<<flush;
    }
  }
  finish = clock();
  cerr<<"\r  Processed "<<lineNo<<" lines. [INVALID="<<invalidNum<<"] [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   \n"<<"  Done!\n\n"<<flush;
  inFile.close();
  addConstPhraseContextSensitiveWd (config);
  return true;
}




bool PhraseTable::InitPhraseTableBina(Configuration &config) {
  ifstream inPhrTabFile(config.phrase_table_file_.c_str(), ios::binary);
  if (!inPhrTabFile) {
    cerr<<"ERROR: Can not open file \""<<config.phrase_table_file_<<"\".\n"<<flush;
    exit(1);
  } else {
    cerr<<"Loading Phrase-Table-Binary\n"<<"  >> From File: "<<config.phrase_table_file_<<"\n"<<flush;
  }

  clock_t start, finish;
  start = clock();
  string lineOfInFile;
  size_t lineNo = 0  ;
  PHRTABSCORES phrTabScores;
  while (inPhrTabFile.read(reinterpret_cast<char *>(&phrTabScores), sizeof(phrTabScores))) {
    TranslationOptionBina translationOptionBina( phrTabScores.tgtPhr, phrTabScores.transModelScore );
    translationsOptionsBina[ phrTabScores.srcPhr ].translationOptionsBina.push_back( translationOptionBina );
    ++lineNo;
    finish = clock();
    if (lineNo % 1000000 == 0) {
      cerr<<"\r  Processed "<<lineNo<<" lines. [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   "<<flush;
    }
  }
  finish = clock();
  cerr<<"\r  Processed "<<lineNo<<" lines. [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   \n"<<"  Done!\n\n"<<flush;
  inPhrTabFile.close();
  ifstream inSrcPhrFile(config.source_phrase_file_.c_str());
  if (!inSrcPhrFile) {
    cerr<<"ERROR: Can not open file \""<<config.source_phrase_file_<<"\".\n"<<flush;
    exit(1);
  } else {
    cerr<<"Loading Phrase-Table-Binary-SrcPhr\n"<<"  >> From File: "<<config.source_phrase_file_<<"\n"<<flush;
  }
  start = clock();
  lineNo = 0;
  unsigned int encodeCnt = 0;
  while (getline(inSrcPhrFile, lineOfInFile)) {
    ClearIllegalChar(lineOfInFile);
    RmEndSpace(lineOfInFile);
    RmStartSpace(lineOfInFile);
    srcPhrEncode.insert( make_pair( lineOfInFile, encodeCnt ) );
    ++encodeCnt;
    ++lineNo;
    finish = clock();
    if (lineNo % 100000 == 0) {
      cerr<<"\r  Processed "<<lineNo<<" lines. [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   "<<flush;
    }
  }
  finish = clock();
  cerr<<"\r  Processed "<<lineNo<<" lines. [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   \n"<<"  Done!\n\n"<<flush;
  inSrcPhrFile.close();
  ifstream inTgtPhrFile(config.target_phrase_file_.c_str());
  if (!inTgtPhrFile) {
    cerr<<"ERROR: Can not open file \""<<config.target_phrase_file_<<"\".\n"<<flush;
    exit( 1 );
  } else {
    cerr<<"Loading Phrase-Table-Binary-TgtPhr\n"<<"  >> From File: "<<config.target_phrase_file_<<"\n"<<flush;
  }
  lineNo = 0;
  encodeCnt = 0;
  start = clock();
  while (getline(inTgtPhrFile, lineOfInFile)) {
    ClearIllegalChar(lineOfInFile);
    RmEndSpace(lineOfInFile);
    RmStartSpace(lineOfInFile);
    tgtPhrDecode.push_back(lineOfInFile);
    ++encodeCnt;
    ++lineNo;
    finish = clock();
    if (lineNo % 100000 == 0) {
      cerr<<"\r  Processed "<<lineNo<<" lines. [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   "<<flush;
    }
  }
  finish = clock();
  cerr<<"\r  Processed "<<lineNo<<" lines. [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   \n"<<"  Done!\n\n"<<flush;
  inTgtPhrFile.close();
  addConstPhraseBina();
  return true;
}


bool PhraseTable::addConstPhrase() {
  string srcPhrase = "<s>";
  string tgtPhrase = "<s>";
  addConstPhrase(srcPhrase, tgtPhrase);
  srcPhrase = "</s>";
  tgtPhrase = "</s>";
  addConstPhrase(srcPhrase, tgtPhrase);
  return true;
}


bool PhraseTable::addConstPhrase(Configuration &config) {
  string srcPhrase = "<s>";
  string tgtPhrase = "<s>";
  addConstPhrase( config, srcPhrase, tgtPhrase );
  srcPhrase = "</s>";
  tgtPhrase = "</s>";
  addConstPhrase( config, srcPhrase, tgtPhrase );
  return true;
}


bool PhraseTable::addConstPhraseContextSensitiveWd (Configuration &config) {
  string srcPhrase = "<s>";
  string tgtPhrase = "<s>";
  addConstPhraseContextSensitiveWd (config, srcPhrase, tgtPhrase);
  srcPhrase = "</s>";
  tgtPhrase = "</s>";
  addConstPhraseContextSensitiveWd (config, srcPhrase, tgtPhrase);
  return true;
}



bool PhraseTable::addConstPhraseBina() {
  string srcPhrase = "<s>";
  string tgtPhrase = "<s>";
  addConstPhraseBina( srcPhrase, tgtPhrase );
  srcPhrase = "</s>";
  tgtPhrase = "</s>";
  addConstPhraseBina( srcPhrase, tgtPhrase );
  return true;
}


bool PhraseTable::addConstPhraseBina(string &srcPhrase, string &tgtPhrase) {
  tgtPhrDecode.push_back(tgtPhrase);
  srcPhrEncode.insert(make_pair(srcPhrase, (unsigned int)srcPhrEncode.size()));
  PHRTABSCORES phrTabScores;
  phrTabScores.transModelScore = 0;
  phrTabScores.srcPhr = (unsigned int)(srcPhrEncode.size() - 1);
  phrTabScores.tgtPhr = (unsigned int)(tgtPhrDecode.size() - 1);
  TranslationOptionBina translationOptionBina( phrTabScores.tgtPhr, phrTabScores.transModelScore );
  translationsOptionsBina[ phrTabScores.srcPhr ].translationOptionsBina.push_back( translationOptionBina );
  return true;
}


bool PhraseTable::addConstPhrase (string &srcPhrase, string &tgtPhrase) {
  float proEGivenF        = 0.0f;
  float lexEGivenF        = 0.0f;
  float proFGivenE        = 0.0f;
  float lexFGivenE        = 0.0f;
  float naturalLogarithm  = 1.0f;
  float biLexLinks        = 0.0f;
  float empty_translation = 0.0f;
  ScoresOfTranslateOption scoresOfTranslateOption(proEGivenF, lexEGivenF, proFGivenE, lexFGivenE, naturalLogarithm, biLexLinks, empty_translation);
  TranslationOption translationOption(tgtPhrase, scoresOfTranslateOption);
  translationsOptions[srcPhrase].translationOptions.push_back(translationOption);
  return true;
}


bool PhraseTable::addConstPhrase (Configuration &config, string &srcPhrase, string &tgtPhrase) {
  float proEGivenF        = 0.0f;
  float lexEGivenF        = 0.0f;
  float proFGivenE        = 0.0f;
  float lexFGivenE        = 0.0f;
  float naturalLogarithm  = 1.0f;
  float biLexLinks        = 0.0f;
  float empty_translation = 0.0f;
  vector< float > freeFeatureValue(config.free_feature_, 0);
  ScoresOfTranslateOption scoresOfTranslateOption(proEGivenF, lexEGivenF, proFGivenE, lexFGivenE, naturalLogarithm, biLexLinks, empty_translation, freeFeatureValue);
  TranslationOption translationOption(tgtPhrase, scoresOfTranslateOption);
  translationsOptions[srcPhrase].translationOptions.push_back(translationOption);
  return true;
}


bool PhraseTable::addConstPhraseContextSensitiveWd (Configuration &config, string &srcPhrase, string &tgtPhrase) {
  float proEGivenF        = 0.0f;
  float lexEGivenF        = 0.0f;
  float proFGivenE        = 0.0f;
  float lexFGivenE        = 0.0f;
  float naturalLogarithm  = 1.0f;
  float biLexLinks        = 0.0f;
  float empty_translation = 0.0f;
  vector<float> freeFeatureValue(config.free_feature_, 0);
  ScoresOfTranslateOption scoresOfTranslateOption(proEGivenF, lexEGivenF, proFGivenE, lexFGivenE, naturalLogarithm, biLexLinks, empty_translation, freeFeatureValue);
  vector<bool> v_unaligned_flag(1, false);
  TranslationOption translationOption(tgtPhrase, scoresOfTranslateOption, v_unaligned_flag);
  translationsOptions[srcPhrase].translationOptions.push_back(translationOption);
  return true;
}


bool PhraseTable::checkGeneration(string &srcPhrase, string &tgtPhrase) {
  if (srcPhrase.find("$") != string::npos || tgtPhrase.find("$") != string::npos) {
    vector<string> srcPhrVec;
    vector<string> tgtPhrVec;
    Split(srcPhrase, ' ', srcPhrVec);
    Split(tgtPhrase, ' ', tgtPhrVec);
    map<string, int> srcGeneCount;
    map<string, int> tgtGeneCount;
    for (vector<string>::iterator iter = srcPhrVec.begin(); iter != srcPhrVec.end(); ++iter) {
      if(*iter == "$date") {
        ++srcGeneCount["$date"];
      } else if(*iter == "$time") {
        ++srcGeneCount[ "$time" ];
      } else if(*iter == "$number") {
        ++srcGeneCount[ "$number" ];
      }
    }
    for (vector<string>::iterator iter = tgtPhrVec.begin(); iter != tgtPhrVec.end(); ++ iter) {
      if( *iter == "$date" ) {
        ++ tgtGeneCount[ "$date" ];
      } else if( *iter == "$time" ) {
        ++ tgtGeneCount[ "$time" ];
      } else if( *iter == "$number" ) {
        ++ tgtGeneCount[ "$number" ];
      }
    }
    if (srcGeneCount.size() != tgtGeneCount.size()) {
      return false;
    } else {
      if( srcGeneCount[ "$date" ] != tgtGeneCount[ "$date" ] ) {
        return false;
      }
      if( srcGeneCount[ "$time" ] != tgtGeneCount[ "$time" ] ) {
          return false;
      }
      if( srcGeneCount[ "$number" ] != tgtGeneCount[ "$number" ] ) {
          return false;
      }
    }
  }
  return true;
}

bool PhraseTable::IsEmptyTranslation (string &target_phrase) {
  if (target_phrase == "<NULL>") {
    return true;
  } else {
    return false;
  }
}


bool PhraseTable::ConvertToBinary(Configuration &config) {
  printCP2BLogo();
  cerr<<"  freefeature : "<<config.free_feature_<<"\n"<<flush;
  config.PrintFeatures();
  ifstream inFile(config.testFile.c_str());
  if (!inFile) {
    cerr<<"ERROR: Can not open file \""<<config.testFile<<"\".\n"<<flush;
    exit( 1 );
  } else {
    cerr<<"Loading inputted phrase table\n"<<"  >> From File: "<<config.testFile<<"\n"<<flush;
  }
  if (config.output_file_ == "") {
    config.output_file_ = "phrase.table.bina";
  }
  ofstream outFile(config.output_file_.c_str(), ios::binary);
  if (!outFile) {
    cerr<<"ERROR: Can not open file \""<<config.output_file_<<"\".\n"<<flush;
    exit(1);
  }

  size_t lineNo = 0;
  string lineOfInFile;
  clock_t start, finish;
  start = clock();
  cerr<<"  Encode Source and Target Phrases in PhraseTable...\n";
  size_t invalidNum = 0;
  while (getline(inFile, lineOfInFile)) {
    ++lineNo;
    ClearIllegalChar(lineOfInFile);
    RmEndSpace(lineOfInFile);
    RmStartSpace(lineOfInFile);
    vector<string> domains;
    SplitWithStr(lineOfInFile, " ||| ", domains);
    if (domains.size() < 3) {
        continue;
    }
    if (domains.at(0) == "<s>" || domains.at(0) == "</s>") {
      ++invalidNum;
      continue;
    }
    if(srcPhr.find(domains.at(0)) == srcPhr.end()) {
      srcPhr.insert(make_pair(domains.at(0), 0));
    }
    if(tgtPhr.find(domains.at(1)) == tgtPhr.end()) {
      tgtPhr.insert(make_pair(domains.at(1), 0));
    }
    finish = clock();
    if (lineNo % 100000 == 0) {
      cerr<<"\r    Processed "<<lineNo<<" lines. [INVALID="<<invalidNum<<"] [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   "<<flush;
    }
  }
  cerr<<"\r    Processed "<<lineNo<<" lines. [INVALID="<<invalidNum<<"] [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   \n"<<flush;
  inFile.close();
  inFile.clear();
  start = clock();
  unsigned int vocabNo = 0;
  string srcPhrTabVocabFile = config.output_file_ + ".src.phrtab.vocab";
  ofstream outSrcPhrTabVocab(srcPhrTabVocabFile.c_str());
  cerr<<"  Output Encoded Source Phrase...\n";
  for (map<string, unsigned int>::iterator iter = srcPhr.begin(); iter != srcPhr.end(); ++iter) {
    iter->second = vocabNo;
    ++vocabNo;
    outSrcPhrTabVocab<<iter->first<<"\n";
    finish = clock();
    if (vocabNo % 100000 == 0) {
      cerr<<"\r    Processed "<<vocabNo<<" lines. [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   "<<flush;
    }
  }
  cerr<<"\r    Processed "<<vocabNo<<" lines. [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   \n"<<flush;
  outSrcPhrTabVocab.close();
  vocabNo = 0;
  string tgtPhrTabVocabFile = config.output_file_ + ".tgt.phrtab.vocab";
  ofstream outTgtPhrTabVocab( tgtPhrTabVocabFile.c_str() );
  cerr<<"  Output Encoded Target Phrase...\n";
  start = clock();
  for (map<string, unsigned int>::iterator iter = tgtPhr.begin(); iter != tgtPhr.end(); ++iter) {
    iter->second = vocabNo;
    ++vocabNo;
    outTgtPhrTabVocab<<iter->first<<"\n";
    finish = clock();
    if (vocabNo % 100000 == 0) {
      cerr<<"\r    Processed "<<vocabNo<<" lines. [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   "<<flush;
    }
  }
  cerr<<"\r    Processed "<<vocabNo<<" lines. [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   \n"<<flush;
  outTgtPhrTabVocab.close();
  PHRTABSCORES pts;
  lineNo = 0;
  invalidNum = 0;
  cerr<<"  Convert PhraseTable to Binary Format...\n";
  inFile.open(config.testFile.c_str());
  while (getline(inFile, lineOfInFile)) {
    ++lineNo;
    vector<string> domains;
    SplitWithStr(lineOfInFile, " ||| ", domains);
    if (domains.size() < 3) {
      cerr<<"  WARNING: Format error in line "<<lineNo<<"\n"<<flush;
      continue;
    }
    if (domains.at( 0 ) == "<s>" || domains.at( 0 ) == "</s>") {
      ++invalidNum;
      continue;
    }
    vector<string> scores;
    Split(domains.at( 2 ), ' ', scores);
    if (scores.size() < 6) {
      cerr<<"  WARNING: Format error in line "<<lineNo<<"\n"<<flush;
      continue;
    }
    if (!checkGeneration(domains.at(0), domains.at(1))) {
      ++invalidNum;
      continue;
    }
    if (srcPhr.find(domains.at(0)) != srcPhr.end()) {
      pts.srcPhr = srcPhr[ domains.at( 0 ) ];
    } else {
      cerr<<"  WARNING: Can not fine "<<domains.at( 0 )<<" in srcPhr."<<"\n"<<flush;
      continue;
    }
    if (tgtPhr.find(domains.at(1)) != tgtPhr.end()) {
      pts.tgtPhr = tgtPhr[domains.at(1)];
    } else {
      cerr<<"  WARNING: Can not fine "<<domains.at( 1 )<<" in tgtPhr."<<"\n"<<flush;
      continue;
    }

    pts.transModelScore = config.features.feature_values_.at( 2 ).weight_ * ( float )atof( scores.at( 0 ).c_str() ) + \
                          config.features.feature_values_.at( 3 ).weight_ * ( float )atof( scores.at( 1 ).c_str() ) + \
                          config.features.feature_values_.at( 4 ).weight_ * ( float )atof( scores.at( 2 ).c_str() ) + \
                          config.features.feature_values_.at( 5 ).weight_ * ( float )atof( scores.at( 3 ).c_str() ) + \
                          config.features.feature_values_.at( 6 ).weight_ * ( float )atof( scores.at( 4 ).c_str() ) + \
                          config.features.feature_values_.at( 7 ).weight_ * ( float )atof( scores.at( 5 ).c_str() );

    if (config.free_feature_ != 0) {
      if ((config.free_feature_ <= scores.size() - 6) && (config.free_feature_ <= config.features.feature_values_.size() - 17)) {
        for (int i = 0; i < config.free_feature_; ++i) {
          pts.transModelScore += config.features.feature_values_.at( i + 17 ).weight_ * ( float )atof( scores.at( i + 6 ).c_str() );
        }
      } else {
        cerr<<"ERROR: Parameter 'freefeature' in config file do not adapt to the phrasetable!\n";
        exit(1);
      }
    }
    outFile.write( reinterpret_cast< char * >( &pts ), sizeof( pts ) );
    finish = clock();
    if (lineNo % 100000 == 0) {
      cerr<<"\r    Processed "<<lineNo<<" lines. [INVALID="<<invalidNum<<"] [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   "<<flush;
    }
  }
  cerr<<"\r    Processed "<<lineNo<<" lines. [INVALID="<<invalidNum<<"] [Time:"<<(double)( finish - start )/CLOCKS_PER_SEC<<" s]   \n"<<flush;
  cerr<<"  Done!\n"<<flush;
  inFile.close();
  inFile.clear();
  return true;
}


bool PhraseTable::printCP2BLogo() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# Convert Phrase Table to Binary Format                           #\n"
      <<"#                                             Version 0.0.1       #\n"
      <<"#                                             NEUNLPLab/YAYI corp #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<flush;
  return true;
}


}


