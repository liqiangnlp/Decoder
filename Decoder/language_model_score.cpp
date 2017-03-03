/*
* $Id:
* 0058
*
* $File:
* language_model_score.cpp
*
* $Proj:
* Decoder for Statistical Machine Translation
*
* $Func:
* Calculate n-gram language model score
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
* 11/03/2015, 10:04, Los Angeles, USA
*/


#include "language_model_score.h"


namespace language_model_score {

/*
 * $Name: Initialize
 * $Function: Split string with char
 * $Date: 2014-04-11
 */
bool LanguageModelScore::Initialize(map<string, string> &parameters) {
  cerr<<"Initialize...\n"<<flush;
  CheckFiles(parameters);
  string parameter_key = "-ngram";
  string default_value = "5";
  CheckEachParameter(parameters, parameter_key, default_value);
  ngram_ = atoi(parameters[parameter_key].c_str());

  Configuration config;
  config.lm_file_ = lmbin_file_;
  config.target_vocab_file_ = vocab_file_;
  config.ngram_ = ngram_;
  ngram_language_model_.LoadModel (config);
  InitTgtVocab (config);
  return true; 
}


/*
 * $Name: Process
 * $Function: Process
 * $Date: 11/03/2015, 13:56, Los Angeles, USA
 */
bool LanguageModelScore::Process() {
  cerr<<"Calculate "<<ngram_<<"-gram language model score...\n"<<flush;
  ifstream in_file (input_file_.c_str());
  if (!in_file) {
    cerr<<"ERROR: Please check the path of \""<<input_file_<<"\".\n"<<flush;
    exit (1);
  }

  ofstream out_file (output_file_.c_str());
  if (!out_file) {
    cerr<<"ERROR: Can not open file \""<<output_file_<<"\".\n"<<flush;
    exit(1);
  }


  string line;
  int line_no = 0;
  while (getline(in_file, line)) {
    ++line_no;
    float ngram_score = 0.0f;
    ngram_score = CalculateLMScore(line);
    out_file<<ngram_score<<"\n";
    if (line_no % 1000 == 0) {
      cerr<<"\r  Process "<<line_no<<" lines."<<flush;
    }
  }
  cerr<<"\r  Process "<<line_no<<" lines.\n"<<flush;

  in_file.close();
  out_file.close();
  return true;
}


/*
 * $Name: CalculateLMScore
 * $Function:
 * $Date: 11/03/2015, Los Angeles, USA
 */
float LanguageModelScore::CalculateLMScore(string &line) {
  RmEndSpace(line);
  RmStartSpace(line);
  ClearIllegalChar(line);
  vector<string> v_words;
  vector<int> v_words_id;
  vector<float> v_ngram_lm_scores;

  Split(line, ' ', v_words);
  int words_count = (int)v_words.size();
  int position = 0;
  for (vector<string>::iterator iter = v_words.begin(); iter != v_words.end(); ++iter) {
    int word_id = 0;
    if (m_target_vocab_.find(*iter) != m_target_vocab_.end()) {
      word_id = m_target_vocab_[*iter];
    } else {
      word_id = m_target_vocab_["<unk>"];
    }
    v_words_id.push_back(word_id);
    int end_position = position + 1;
    int begin_position = end_position - ngram_ > 0 ? end_position - ngram_ : 0;
    float probability = ngram_language_model_.GetProb(v_words_id, begin_position, end_position);

#ifdef WIN32
    if (probability < -20) {
#else
    if (probability < -20 || isnan(probability)) {
#endif
      probability = -20;
    }
    ++position;
    v_ngram_lm_scores.push_back(probability);
  }

  float ngram_lm_score = 0.0f;
  for (vector<float>::iterator iter = v_ngram_lm_scores.begin(); iter != v_ngram_lm_scores.end(); ++iter) {
    ngram_lm_score += *iter;
  }
  return ngram_lm_score;
}



bool LanguageModelScore::CheckFiles(map<string, string> &parameters) {
  string file_key = "-vocab";
  CheckFile(parameters, file_key);
  vocab_file_ = parameters[file_key];

  file_key = "-lmbin";
  CheckFile(parameters, file_key);
  lmbin_file_ = parameters[file_key];

  file_key = "-input";
  CheckFile(parameters, file_key);
  input_file_ = parameters[file_key];

  file_key = "-output";
  if(parameters.find(file_key) == parameters.end()) {
    cerr<<"ERROR: Please add parameter \"-output\" in your command line!\n";
  }
  output_file_ = parameters[file_key];
  return true;
}



/*
 * $Name: CheckFile
 * $Function: If the file to be checked does not exist, exit!
 * $Date: 11/03/2015, 12:39, Los Angeles, USA
 */
bool LanguageModelScore::CheckFile(map<string, string> &parameters, string &file_key) {
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


bool LanguageModelScore::CheckEachParameter(map<string, string> &parameters, string &parameter_key, string &default_value) {
  if (parameters.find(parameter_key) == parameters.end()) {
    parameters[parameter_key] = default_value;
  }
  return true;
}



bool LanguageModelScore::InitTgtVocab(Configuration &config) {
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
    m_target_vocab_.insert(make_pair(lineOfInFile, wid));
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



bool LanguageModelScore::PrintLMScoreLogo() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# Calculate N-Gram LM Score                                       #\n"
      <<"#                                            Version 0.0.1        #\n"
      <<"#                                            liqiangneu@gmail.com #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<flush;
  return true;
}


}

