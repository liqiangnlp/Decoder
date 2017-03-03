/*
* $Id:
* 0061
*
* $File:
* alignment_difference.h
*
* $Proj:
* Decoder
*
* $Func:
* Calculate difference between two word alignments
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
* 02/11/2016, 22:43, Los Angeles, CA, USA
*/


#ifndef ALIGNMENT_DIFFERENCE_H_
#define ALIGNMENT_DIFFERENCE_H_

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
#include "basic_method.h"
#include "tokenizer.h"
using namespace std;
using namespace basic_method;


namespace alignment_difference {

class AlignmentDifference {

 public:
  string alignment1_file_name_;
  string alignment2_file_name_;
  string source_file_name_;
  string target_file_name_;
  string output_file_name_;
  string log_file_name_;


 public:
  AlignmentDifference() {
    alignment1_file_name_ = "";
    alignment2_file_name_ = "";
    source_file_name_ = "";
    target_file_name_ = "";
    output_file_name_ = "";
    log_file_name_ = "";
  }
  ~AlignmentDifference() {}

 public:
  bool Initialize(map<string, string> &parameters);

 public:
  bool CalculateDifference();

 private:
  bool CleanString(BasicMethod &basic_method, string &line_source, string &line_target, string &line_alignment1, string &line_alignment2);

 private:
  bool CountAlignedNumber(string &line_source, string &line_target, string &line_alignment1, string &line_alignment2, BasicMethod &basic_method, \
                          int &src_words_num, int &tgt_words_num, int &src_difference_num, int &tgt_difference_num);
  bool MappingAlignments(string &line_alignment, vector<int> &v_aligned_num, vector<int> &v_aligned_num_inv, multimap<int, int> &m_aligned_word, set<string> &s_aligned_word, \
                         multimap<int, int> &m_aligned_word_inv, set<string> &s_aligned_word_inv, BasicMethod &basic_method);
  bool CountDifference(vector<int> &v_aln1_aligned_num, vector<int> &v_aln2_aligned_num, \
                       multimap<int, int> &m_aln1_aligned_word, multimap<int, int> &m_aln2_aligned_word, \
                       set<string> &s_aln2_aligned_word,  BasicMethod &basic_method, int &difference_num);


 private:
  bool CheckFiles(map<string, string> &parameters);
  bool CheckFile(map<string, string> &parameters, string &file_key);

 public:
  static bool PrintAlignmentDifferenceLogo();
};



}

#endif

