/*
* $Id:
* 0059
*
* $File:
* search_engine.h
*
* $Proj:
* Search Engine
*
* $Func:
* Search Engine
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
* 11/23/2015, 20:54, Los Angeles, USA
*/


#ifndef SEARCH_ENGINE_H_
#define SEARCH_ENGINE_H_

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <set>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <ctime>
#include <map>
#include <list>
#include <unordered_map>
#include "basic_method.h"
#include "tokenizer.h"
#include "debug.h"
using namespace std;
using namespace basic_method;
using namespace decoder_tokenizer;


namespace search_engine {

class SearchEngine {

 public:
  string corpus_file_name_;
  string index_file_name_;
  string search_file_name_;
  string output_file_name_;
  string log_file_name_;

 public:
  int word_num_;
  map<string, int> m_word_id_;

 public:
  int corpus_size_;
  vector<pair<int, int> > v_words_cnt_corpus_;  // Word count of every lines in corpus, including cnt, and cnt without stopwords 

 public:
  vector<string> v_corpus_database_;

 public:
  int ngram_;
  int topn_;

 public:
  int index_size_;
  vector<list<pair<int, int> > > v_index_sentid_cnt_;

 public:
  //map<int, int> m_sentid_frequency_;

 public:
  SearchEngine() {
    corpus_file_name_ = "";
    index_file_name_ = "";
    search_file_name_ = "";
    output_file_name_ = "";
    log_file_name_ = "";

    word_num_ = 0;
    corpus_size_ = 0;
    index_size_ = 0;
    ngram_ = 1;
  }
  ~SearchEngine() {}

 public:
  bool Initialize(map<string, string> &parameters);

 public:
  bool Search();

 private:
  bool LoadIndex();
  bool LoadCorpus();
  bool SearchInputFile();

 private:
  bool CheckFiles(map<string, string> &parameters);
  bool CheckFile(map<string, string> &parameters, string &file_key);

 private:
  bool CheckEachParameter(map<string, string> &parameters, string &parameter_key, string &default_value);

 public:
  static bool PrintSearchEngineLogo();
};


class BuildIndex {

 public:
  string corpus_file_name_;
  string stopword_file_name_;
  string output_file_name_;
  string log_file_name_;
  string white_list_file_name_;

 public:
  set<string> s_stopword_;
  set<string> s_cutoff_;
  set<string> s_cutoff_high_frequency_;
  set<string> s_white_list_;

 public:
  map<string, int> m_word_id_;
  map<string, int> m_word_frequency_;

 public:
  int ngram_;
  int cutoff_;
  float cutoff_high_frequency_;

 public:
  vector<pair<int, pair<int, int> > > v_words_cnt_corpus_;        // Word number of every lines in corpus, sentence_id, cnt, cnt remove stopwords

 public:
  map<string, list<pair<int, int> > > m_index_word_sentid_cnt_;   // string, list <sentid, cnt>

 public:
  BuildIndex() {
   corpus_file_name_ = "";
   stopword_file_name_ = "";
   output_file_name_ = "";
   log_file_name_ = "";

   ngram_ = 1;
   cutoff_ = 0;
   cutoff_high_frequency_ = 0.0f;
  }

  ~BuildIndex() {}

 public:
  bool Initialize(map<string, string> &parameters);

 public:
  bool BuildIndexSearchEngine();

 private:
  bool LoadWhiteList();
  bool LoadStopword();
  bool BuildCutoff();

 private:
  bool CheckFiles(map<string, string> &parameters);
  bool CheckFile(map<string, string> &parameters, string &file_key);

 private:
  bool CheckEachParameter(map<string, string> &parameters, string &parameter_key, string &default_value);


 public:
  static bool PrintBuildIndexLogo();
};

}

#endif

