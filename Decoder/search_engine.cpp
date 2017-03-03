/*
* $Id:
* 0060
*
* $File:
* search_engine.cpp
*
* $Proj:
* Search Engine
*
* $Func:
* search engine
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
* 11/23/2015, 20:56, Los Angeles, USA
*/


#include "search_engine.h"


namespace search_engine {

///////////////////////////////////////////////////// SearchEngine Class /////////////////////////////////////////////////////


/*
 * $Name: Initialize
 * $Function: 
 * $Date: 11/23/2015, Los Angeles, USA
 */
bool SearchEngine::Initialize(map<string, string> &parameters) {
  cerr<<"Initialize ...\n"<<flush;
  CheckFiles(parameters);
  string parameter_key = "-ngram";
  string default_value = "1";
  CheckEachParameter(parameters, parameter_key, default_value);
  ngram_ = atoi(parameters[parameter_key].c_str());
  return true; 
}


bool SearchEngine::Search() {
  LoadIndex();
  LoadCorpus();
  SearchInputFile();
  return true;
}


bool SearchEngine::LoadIndex() {
  cerr<<"Loading index from "<<index_file_name_<<" ...\n"<<flush;
  ifstream index_file (index_file_name_.c_str());
  if (!index_file) {
    cerr<<"ERROR: Please check the path of \""<<index_file_name_<<"\".\n"<<flush;
    exit (1);
  }

  int line_no = 0;
  string line = "";
  BasicMethod basic_method;

  getline(index_file, line);
  basic_method.RmEndSpace(line);
  basic_method.RmStartSpace(line);
  basic_method.ClearIllegalChar(line);
  word_num_ = atoi(line.c_str());
  for (int i = 0; i < word_num_; ++i) {
    getline(index_file, line);
    basic_method.RmEndSpace(line);
    basic_method.RmStartSpace(line);
    basic_method.ClearIllegalChar(line);
    vector<string> v_id_word;
    basic_method.Split(line, '\t', v_id_word);
    m_word_id_[v_id_word.at(1)] = atoi(v_id_word.at(0).c_str());
  }

  getline(index_file, line);
  basic_method.RmEndSpace(line);
  basic_method.RmStartSpace(line);
  basic_method.ClearIllegalChar(line);
  if ("===============liqiangneu@gmail.com===============" != line) {
   cerr<<"Error: Index File "<<index_file_name_<<"is incorrect!\n"
       <<"       No have liqiangneu@gmail.com\n"<<flush;
   exit (1);
  }

  getline(index_file, line);
  basic_method.RmEndSpace(line);
  basic_method.RmStartSpace(line);
  basic_method.ClearIllegalChar(line);
  corpus_size_ = atoi(line.c_str());

//  v_words_num_corpus_.reserve(corpus_size_);
  v_words_cnt_corpus_.reserve(corpus_size_);
  for (int i = 0; i < corpus_size_; ++i) {
    getline(index_file, line);
    basic_method.RmEndSpace(line);
    basic_method.RmStartSpace(line);
    basic_method.ClearIllegalChar(line);
    vector<string> v_words_num;
    basic_method.Split(line, '\t', v_words_num);
    v_words_cnt_corpus_.push_back(make_pair(atoi(v_words_num.at(1).c_str()), atoi(v_words_num.at(2).c_str())));
  }

  getline(index_file, line);
  basic_method.RmEndSpace(line);
  basic_method.RmStartSpace(line);
  basic_method.ClearIllegalChar(line);
  if ("===============liqiangneu@gmail.com===============" != line) {
   cerr<<"Error: Index File "<<index_file_name_<<"is incorrect!\n"
    <<"       No have liqiangneu@gmail.com\n"<<flush;
   exit (1);
  }

  getline(index_file, line);
  basic_method.RmEndSpace(line);
  basic_method.RmStartSpace(line);
  basic_method.ClearIllegalChar(line);
  index_size_ = atoi(line.c_str());

  v_index_sentid_cnt_.resize(word_num_);
  int i;
  for (i = 0; i < index_size_; ++i) {
    getline(index_file, line);
    basic_method.RmEndSpace(line);
    basic_method.RmStartSpace(line);
    basic_method.ClearIllegalChar(line);

    vector<string> v_word_id_cnt;
    basic_method.Split(line, '\t', v_word_id_cnt);
    if (v_word_id_cnt.size() < 2) {
     cerr<<"Warning: Format error in "<<line_no<<" of "<<index_file_name_<<"\n"<<flush;
     continue;
    } else {
      for (int i = 1; i < v_word_id_cnt.size(); ++i) {
        vector<string> v_id_cnt;
        basic_method.Split(v_word_id_cnt[i], ' ', v_id_cnt);
        int word_id = atoi(v_word_id_cnt[0].c_str());
        int id = atoi(v_id_cnt.at(0).c_str());
        int cnt = atoi(v_id_cnt.at(1).c_str());
        v_index_sentid_cnt_.at(word_id).push_back(make_pair(id, cnt));
      }
    }
    if (i % 10000 == 0) {
      cerr<<"\r  Processed "<<i<<" lines."<<flush;
    }
  }
  cerr<<"\r  Processed "<<i<<" lines.\n"<<flush;
  index_file.close();
  return true;
}


bool SearchEngine::LoadCorpus() {
  cerr<<"Loading corpus from "<<corpus_file_name_<<" ...\n"<<flush;
  ifstream corpus_file (corpus_file_name_.c_str());
  if (!corpus_file) {
   cerr<<"ERROR: Please check the path of \""<<corpus_file_name_<<"\".\n"<<flush;
   exit (1);
  }

  string line = "";
  int line_no = 0;
  BasicMethod basic_method;
  while (getline(corpus_file, line)) {
    basic_method.RmEndSpace(line);
    basic_method.RmStartSpace(line);
    basic_method.ClearIllegalChar(line);
    v_corpus_database_.push_back(line);

    ++line_no;
    if (line_no % 10000 == 0) {
      cerr<<"\r  Processed "<<line_no<<" lines."<<flush;
    }
  }
  cerr<<"\r  Processed "<<line_no<<" lines.\n"<<flush;
  corpus_file.close();
  return true;
}


bool SearchEngine::SearchInputFile() {
  cerr<<"Searching ...\n"<<flush;
  ifstream search_file (search_file_name_.c_str());
  if (!search_file) {
   cerr<<"ERROR: Please check the path of \""<<search_file_name_<<"\".\n"<<flush;
   exit (1);
  }

  string out_file_tmp_name = output_file_name_ + ".tmp";
  ofstream out_file_tmp (out_file_tmp_name.c_str());
  if (!out_file_tmp) {
    cerr<<"ERROR: Can not open file \""<<out_file_tmp_name<<"\".\n"<<flush;
    exit(1);
  }

  ofstream out_file (output_file_name_.c_str());
  if (!out_file) {
    cerr<<"ERROR: Can not open file \""<<output_file_name_<<"\".\n"<<flush;
    exit(1);
  }


  string line = "";
  int line_no = 0;
  BasicMethod basic_method;
  while (getline(search_file, line)) {
    ++line_no;
    basic_method.RmEndSpace(line);
    basic_method.RmStartSpace(line);
    basic_method.ClearIllegalChar(line);

    if (m_word_id_.find(line) != m_word_id_.end()) {
      out_file_tmp<<line<<"\n";
      for (list<pair<int, int> >::iterator iter = v_index_sentid_cnt_[m_word_id_[line]].begin(); \
           iter != v_index_sentid_cnt_[m_word_id_[line]].end(); ++iter) {
        out_file_tmp<<iter->first<<" "<<iter->second<<"\t"<<v_corpus_database_[iter->first]<<"\n";
        m_sentid_frequency_[iter->first] += iter->second;
      }
      out_file_tmp<<"\n";
    } else {
      out_file_tmp<<line<<"\n"
              <<"-1 0\tNOT FOUND\n\n";
    }
  }

  multimap<float, int> m_score_sentid;
  for (map<int, int>::iterator iter = m_sentid_frequency_.begin(); iter != m_sentid_frequency_.end(); ++iter) {
    float score = (float)iter->second/v_words_cnt_corpus_[iter->first].second;
    m_score_sentid.insert(make_pair(score, iter->first));
  }

  for (multimap<float, int>::reverse_iterator riter = m_score_sentid.rbegin(); riter != m_score_sentid.rend(); ++riter) {
    out_file<<riter->first<<" "<<m_sentid_frequency_[riter->second]<<" "<<v_words_cnt_corpus_[riter->second].second<<" "<<v_words_cnt_corpus_[riter->second].first<<" "<<riter->second<<"\t"<<v_corpus_database_[riter->second]<<"\n";
  }

  search_file.close();
  out_file_tmp.close();
  out_file.close();
  return true;
}


bool SearchEngine::CheckFiles(map<string, string> &parameters) {
  string file_key = "-corpus";
  CheckFile(parameters, file_key);
  corpus_file_name_ = parameters[file_key];

  file_key = "-index";
  CheckFile(parameters, file_key);
  index_file_name_ = parameters[file_key];

  file_key = "-search";
  CheckFile(parameters, file_key);
  search_file_name_ = parameters[file_key];

  file_key = "-output";
  if(parameters.find(file_key) == parameters.end()) {
    cerr<<"ERROR: Please add parameter \"-output\" in your command line!\n";
  }
  output_file_name_ = parameters[file_key];
  return true;
}


/*
 * $Name: CheckFile
 * $Function: If the file to be checked does not exist, exit!
 * $Date: 11/03/2015, 12:39, Los Angeles, USA
 */
bool SearchEngine::CheckFile(map<string, string> &parameters, string &file_key) {
  if (parameters.find(file_key) != parameters.end()) {
    ifstream in_file(parameters[file_key].c_str());
    if (!in_file) {
      cerr<<"ERROR: Please check the path of \""<<file_key<<"\".\n"<<flush;
      exit (1);
    }
    in_file.clear();
    in_file.close();
  } else {
    cerr<<"ERROR: Please add parameter \""<<file_key<<"\" in your command line!\n"<<flush;
    exit (1);
  }
  return true;
}


bool BuildIndex::CheckEachParameter(map<string, string> &parameters, string &parameter_key, string &default_value) {
  if (parameters.find(parameter_key) == parameters.end()) {
    parameters[parameter_key] = default_value;
  }
  return true;
}


/*
 * $Name: Print Search 
 * $Function: Print Information
 * $Date: 11/23/2015, Los Angeles, USA
 */
bool SearchEngine::PrintSearchEngineLogo() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# Search Engine                                                   #\n"
      <<"#                                            Version 0.0.1        #\n"
      <<"#                                            liqiangneu@gmail.com #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<flush;
  return true;
}



///////////////////////////////////////////////////// BuildIndex Class /////////////////////////////////////////////////////

/*
 * $Name: Initialize
 * $Function: 
 * $Date: 11/23/2015, Los Angeles, USA
 */
bool BuildIndex::Initialize(map<string, string> &parameters) {
  cerr<<"Initialize ...\n"<<flush;
  CheckFiles(parameters);
  string parameter_key = "-ngram";
  string default_value = "1";
  CheckEachParameter(parameters, parameter_key, default_value);
  ngram_ = atoi(parameters[parameter_key].c_str());

  parameter_key = "-cutoff";
  default_value = "0";
  CheckEachParameter(parameters, parameter_key, default_value);
  cutoff_ = atoi(parameters[parameter_key].c_str());

  parameter_key = "-cutoffrhf";
  default_value = "0";
  CheckEachParameter(parameters, parameter_key, default_value);
  cutoff_high_frequency_ = (float)atof(parameters[parameter_key].c_str());

  return true; 
}


bool BuildIndex::BuildIndexSearchEngine() {
  LoadWhiteList();
  LoadStopword();
  BuildCutoff();

  ifstream corpus_file (corpus_file_name_.c_str());
  if (!corpus_file) {
    cerr<<"ERROR: Please check the path of \""<<corpus_file_name_<<"\".\n"<<flush;
    exit (1);
  }

  ofstream out_file (output_file_name_.c_str());
  if (!out_file) {
    cerr<<"ERROR: Can not open file \""<<output_file_name_<<"\".\n"<<flush;
    exit (1);
  }

  ofstream log_file (log_file_name_.c_str());
  if (!log_file) {
    cerr<<"ERROR: Can not open file \""<<log_file_name_<<"\".\n"<<flush;
    exit (1);
  }


  int line_no = 0;
  string line = "";
  BasicMethod basic_method;
  cerr<<"Build index from "<<corpus_file_name_<<" ...\n"<<flush;
  log_file<<"Build index from "<<corpus_file_name_<<" ...\n"<<flush;
  while (getline(corpus_file, line)) {
    basic_method.RmEndSpace(line);
    basic_method.RmStartSpace(line);
    basic_method.ClearIllegalChar(line);

    vector<string> v_words;
    basic_method.Split(line, ' ',v_words);
    map<string, int> m_word_cnt;
    int cnt_without_stopwords = 0;
    for (vector<string>::iterator iter = v_words.begin(); iter != v_words.end(); ++iter) {
      if (s_stopword_.find(*iter) != s_stopword_.end()) {
        continue;
      } else if (s_cutoff_.find(*iter) != s_cutoff_.end()) {
        ++cnt_without_stopwords;
        continue;
      } else if (s_cutoff_high_frequency_.find(*iter) != s_cutoff_high_frequency_.end()) {
        ++cnt_without_stopwords;
        continue;
      } else {
        ++m_word_cnt[*iter];
        ++m_word_id_[*iter];
        ++cnt_without_stopwords;
      }
    }
    v_words_cnt_corpus_.push_back(make_pair(line_no, make_pair((int)v_words.size(), cnt_without_stopwords)));

    for (map<string, int>::iterator iter = m_word_cnt.begin(); iter != m_word_cnt.end(); ++iter) {
      m_index_word_sentid_cnt_[iter->first].push_back(make_pair(line_no, iter->second));
    }
    if (line_no % 100000 == 0) {
      cerr<<"\r  Processed "<<line_no<<" lines."<<flush;
      log_file<<"\r  Processed "<<line_no<<" lines."<<flush;
    }
    ++line_no;
  }
  cerr<<"\r  Processed "<<line_no<<" lines.\n"<<flush;
  log_file<<"\r  Processed "<<line_no<<" lines.\n"<<flush;

  int id = 0;
  out_file<<m_word_id_.size()<<"\n";
  cerr<<"Output word id ...\n"<<flush;
  log_file<<"Output word id ...\n"<<flush;
  for (map<string, int>::iterator iter = m_word_id_.begin(); iter != m_word_id_.end(); ++iter) {
    iter->second = id;
    out_file<<id<<"\t"<<iter->first<<"\n";
    ++id;
    if (id % 10000 == 0) {
      cerr<<"\r  Processed "<<id<<" lines.";
      log_file<<"\r  Processed "<<id<<" lines.";
    }
  }
  cerr<<"\r  Processed "<<id<<" lines.\n";
  log_file<<"\r  Processed "<<id<<" lines.\n";

  out_file<<"===============liqiangneu@gmail.com===============\n";
  out_file<<v_words_cnt_corpus_.size()<<"\n";
  line_no = 0;
  cerr<<"Output sentence informations ...\n"<<flush;
  log_file<<"Output sentence informations ...\n"<<flush;
  for (vector<pair<int, pair<int, int> > >::iterator iter = v_words_cnt_corpus_.begin(); iter != v_words_cnt_corpus_.end(); ++iter) {
    ++line_no;
    out_file<<iter->first<<"\t"<<iter->second.first<<"\t"<<iter->second.second<<"\n";
    if (line_no % 10000 == 0) {
      cerr<<"\r  Processed "<<line_no<<" lines."<<flush;
      log_file<<"\r  Processed "<<line_no<<" lines."<<flush;
    }
  }
  cerr<<"\r  Processed "<<line_no<<" lines.\n"<<flush;
  log_file<<"\r  Processed "<<line_no<<" lines.\n"<<flush;

  out_file<<"===============liqiangneu@gmail.com===============\n";
  out_file<<m_index_word_sentid_cnt_.size()<<"\n";
  cerr<<"Output index ...\n"<<flush;
  log_file<<"Output index ...\n"<<flush;
  line_no = 0;
  for (map<string, list<pair<int, int> > >::iterator m_iter = m_index_word_sentid_cnt_.begin(); m_iter != m_index_word_sentid_cnt_.end(); ++m_iter) {
    ++line_no;
    int word_id;
    if (m_word_id_.find(m_iter->first) != m_word_id_.end()) {
      word_id = m_word_id_[m_iter->first];
      out_file<<word_id;
      for (list<pair<int, int> >::iterator l_iter = m_iter->second.begin(); l_iter != m_iter->second.end(); ++l_iter) {
       out_file<<"\t"<<l_iter->first<<" "<<l_iter->second;
      }
      out_file<<"\n";
    } else {
      continue;
    }
    if (line_no % 10000 == 0) {
      cerr<<"\r  Processed "<<line_no<<" lines."<<flush;
      log_file<<"\r  Processed "<<line_no<<" lines."<<flush;
    }
  }
  cerr<<"\r  Processed "<<line_no<<" lines.\n"<<flush;
  log_file<<"\r  Processed "<<line_no<<" lines.\n"<<flush;

  corpus_file.close();
  out_file.close();
  log_file.close();
  return true;
}


bool BuildIndex::LoadWhiteList() {
  if ("" != white_list_file_name_) {
    ifstream white_list_file (white_list_file_name_.c_str());
    if (!white_list_file) {
      cerr<<"ERROR: Please check the path of \""<<white_list_file_name_<<"\".\n"<<flush;
      exit (1);
    }
    int line_no = 0;
    string line = "";
    BasicMethod basic_method;
    cerr<<"Loading white list from "<<white_list_file_name_<<" ...\n"<<flush;
    while (getline(white_list_file, line)) {
     basic_method.RmEndSpace(line);
     basic_method.RmStartSpace(line);
     basic_method.ClearIllegalChar(line);
     s_white_list_.insert(line);
     ++line_no;
    }
    white_list_file.close();
  }
  return true;
}


bool BuildIndex::LoadStopword() {
  ifstream stopword_file (stopword_file_name_.c_str());
  if (!stopword_file) {
    cerr<<"ERROR: Please check the path of \""<<stopword_file_name_<<"\".\n"<<flush;
    exit (1);
  }

  int line_no = 0;
  string line = "";
  BasicMethod basic_method;
  cerr<<"Loading stopword from "<<stopword_file_name_<<" ...\n"<<flush;
  while (getline(stopword_file, line)) {
    basic_method.RmEndSpace(line);
    basic_method.RmStartSpace(line);
    basic_method.ClearIllegalChar(line);
    s_stopword_.insert(line);
    ++line_no;
  }
  stopword_file.close();
  return true;
}


bool BuildIndex::BuildCutoff() {
  if (0 < cutoff_ || (0.0f < cutoff_high_frequency_ && cutoff_high_frequency_ < 1.0f)) {
    cerr<<"Cutoff word index ...\n";
    ifstream corpus_file (corpus_file_name_.c_str());
    if (!corpus_file) {
      cerr<<"ERROR: Please check the path of \""<<corpus_file_name_<<"\".\n"<<flush;
      exit (1);
    }

    string line = "";
    int line_no = 0;
    BasicMethod basic_method;
    while (getline(corpus_file, line)) {
      basic_method.RmEndSpace(line);
      basic_method.RmStartSpace(line);
      basic_method.ClearIllegalChar(line);
      vector<string> v_words;
      basic_method.Split(line, ' ',v_words);
      for (vector<string>::iterator iter = v_words.begin(); iter != v_words.end(); ++iter) {
        ++m_word_frequency_[*iter];
      }
      ++line_no;
      if (line_no % 100000 == 0) {
        cerr<<"\r  Processed "<<line_no<<" lines."<<flush;
      }
    }
    cerr<<"\r  Processed "<<line_no<<" lines.\n"<<flush;

    if (0 < cutoff_) {
      cerr<<"Word frequency <= "<<cutoff_<<" will be removed from word list ...\n"<<flush;
      for (map<string, int>::iterator iter = m_word_frequency_.begin(); iter != m_word_frequency_.end(); ++iter) {
        if (iter->second <= cutoff_) {
          s_cutoff_.insert(iter->first);
        }
      }
    }
    if (0 < cutoff_high_frequency_ && cutoff_high_frequency_ < 1) {
      cerr<<"Top "<<cutoff_high_frequency_<<" high frequency words will be removed ...\n"<<flush;
      int cutoff_high_frequency_number = (int)(cutoff_high_frequency_ * (float)m_word_frequency_.size());
      cerr<<"CUTOFF_HIGH_FREQUENCY_NUMBER="<<cutoff_high_frequency_number<<"\n"<<flush;
      multimap<int, string> m_sort_frequency;
      for (map<string, int>::iterator iter = m_word_frequency_.begin(); iter != m_word_frequency_.end(); ++iter) {
        m_sort_frequency.insert(make_pair(iter->second, iter->first));
      }
      int number = 0;
      for (multimap<int, string>::reverse_iterator riter = m_sort_frequency.rbegin(); riter != m_sort_frequency.rend(); ++riter) {
        ++number;
        if (number <= cutoff_high_frequency_number) {
          if (s_white_list_.find(riter->second) == s_white_list_.end()) {
            s_cutoff_high_frequency_.insert(riter->second);
          } else {
            continue;
          }
        } else {
          break;
        }
      }
    }

    corpus_file.clear();
    corpus_file.close();
  }
  return true;
}




bool BuildIndex::CheckFiles(map<string, string> &parameters) {
  string file_key = "-corpus";
  CheckFile(parameters, file_key);
  corpus_file_name_ = parameters[file_key];

  file_key = "-stopword";
  CheckFile(parameters, file_key);
  stopword_file_name_ = parameters[file_key];

  file_key = "-whitelist";
  if (parameters.find(file_key) != parameters.end()) {
    CheckFile(parameters, file_key);
    white_list_file_name_ = parameters[file_key];
  }

  file_key = "-index";
  if (parameters.find(file_key) == parameters.end()) {
    cerr<<"ERROR: Please add parameter \"-index\" in your command line!\n";
  }
  output_file_name_ = parameters[file_key];
  log_file_name_ = output_file_name_ + ".log";
  return true;
}


/*
 * $Name: CheckFile
 * $Function: If the file to be checked does not exist, exit!
 * $Date: 11/03/2015, 12:39, Los Angeles, USA
 */
bool BuildIndex::CheckFile(map<string, string> &parameters, string &file_key) {
  if (parameters.find(file_key) != parameters.end()) {
    ifstream in_file(parameters[file_key].c_str());
    if (!in_file) {
      cerr<<"ERROR: Please check the path of \""<<file_key<<"\".\n"<<flush;
      exit (1);
    }
    in_file.clear();
    in_file.close();
  } else {
    cerr<<"ERROR: Please add parameter \""<<file_key<<"\" in your command line!\n"<<flush;
    exit (1);
  }
  return true;
}


bool SearchEngine::CheckEachParameter(map<string, string> &parameters, string &parameter_key, string &default_value) {
  if (parameters.find(parameter_key) == parameters.end()) {
    parameters[parameter_key] = default_value;
  }
  return true;
}


/*
 * $Name: Print Build Index
 * $Function: Print Information
 * $Date: 11/23/2015, Los Angeles, USA
 */
bool BuildIndex::PrintBuildIndexLogo() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# Build Index for Search Engine                                   #\n"
      <<"#                                            Version 0.0.1        #\n"
      <<"#                                            liqiangneu@gmail.com #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<flush;
  return true;
}

}

