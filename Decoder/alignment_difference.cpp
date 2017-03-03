/*
* $Id:
* 0061
*
* $File:
* alignment_difference.cpp
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


#include "alignment_difference.h"


namespace alignment_difference {


bool AlignmentDifference::Initialize(map<string, string> &parameters) {
  cerr<<"Initialize ...\n"<<flush;
  CheckFiles(parameters);
  return true;
}


bool AlignmentDifference::CalculateDifference() {
  cerr<<"Calculate Difference ...\n"<<flush;
  ifstream source_file (source_file_name_.c_str());
  if (!source_file) {
    cerr<<"ERROR: Please check the path of \""<<source_file_name_<<"\".\n"<<flush;
    exit (1);
  }

  ifstream target_file (target_file_name_.c_str());
  if (!target_file) {
    cerr<<"ERROR: Please check the path of \""<<target_file_name_<<"\".\n"<<flush;
    exit (1);
  }

  ifstream alignment1_file (alignment1_file_name_.c_str());
  if (!alignment1_file) {
    cerr<<"ERROR: Please check the path of \""<<alignment1_file_name_<<"\".\n"<<flush;
    exit (1);
  }

  ifstream alignment2_file (alignment2_file_name_.c_str());
  if (!alignment2_file) {
    cerr<<"ERROR: Please check the path of \""<<alignment2_file_name_<<"\".\n"<<flush;
    exit (1);
  }

  ofstream output_file (output_file_name_.c_str());
  if (!output_file) {
    cerr<<"ERROR: Can not write \""<<output_file_name_<<"\".\n"<<flush;
    exit (1);
  }


  string line_source = "";
  string line_target = "";
  string line_alignment1 = "";
  string line_alignment2 = "";

  BasicMethod basic_method;

  int line_no = 0;
  int src_words_num_all = 0, tgt_words_num_all = 0;
  int src_difference_num_all = 0, tgt_difference_num_all = 0;
  float src_difference_all = 0.0f, tgt_difference_all = 0.0f, difference_all = 0.0f;
  float max_difference = 0.0f;
  int max_difference_line_num = 0;
  while (getline (source_file, line_source)) {
    ++ line_no;
    getline(target_file, line_target);
    getline(alignment1_file, line_alignment1);
    getline(alignment2_file, line_alignment2);

    CleanString(basic_method, line_source, line_target, line_alignment1, line_alignment2);
    int src_words_num = 0;
    int tgt_words_num = 0;
    int src_difference_num = 0, tgt_difference_num = 0;
    CountAlignedNumber(line_source, line_target, line_alignment1, line_alignment2, basic_method, \
                       src_words_num, tgt_words_num, src_difference_num, tgt_difference_num);

    src_words_num_all += src_words_num;
    tgt_words_num_all += tgt_words_num;
    src_difference_num_all += src_difference_num;
    tgt_difference_num_all += tgt_difference_num;

    float src_difference = (float)src_difference_num/(float)src_words_num;
    float tgt_difference = (float)tgt_difference_num/(float)tgt_words_num;
    float difference = (src_difference + tgt_difference)/2.0f;
    if (difference > max_difference) {
      max_difference = difference;
      max_difference_line_num = line_no;
    }

    src_difference_all = (float)((float)src_difference_num_all/(float)src_words_num_all);
    tgt_difference_all = (float)((float)tgt_difference_num_all/(float)tgt_words_num_all);
    difference_all = (src_difference_all + tgt_difference_all)/2.0f;
    output_file<<"DIFF="<<difference_all
               <<" ||| SRC_DIFF="<<src_difference_all<<" SRC_ALL="<<src_words_num_all<<" SRC_DIFF_NUM_ALL="<<src_difference_num_all
               <<" ||| TGT_DIFF="<<tgt_difference_all<<" TGT_ALL="<<tgt_words_num_all<<" TGT_DIFF_NUM_ALL="<<tgt_difference_num_all
               <<" |||| DIFF="<<difference
               <<" ||| SRC_DIFF="<<src_difference<<" SRC="<<src_words_num<<" SRC_DIFF_NUM="<<src_difference_num
               <<" ||| TGT_DIFF="<<tgt_difference<<" TGT="<<tgt_words_num<<" TGT_DIFF_NUM="<<tgt_difference_num<<"\n";

    if (line_no % 1000 == 0) {
      cerr<<"\r  Processed "<<line_no<<" lines."<<flush;
    }
  }
  cerr<<"\r  Processed "<<line_no<<" lines.\n"<<flush;

  output_file<<"\n\n"<<"DIFF="<<difference_all<<" SRC_DIFF="<<src_difference_all<<" TGT_DIFF="<<tgt_difference_all<<"\n\n"
             <<"MAX_DIFF="<<max_difference<<" LINE_NUM="<<max_difference_line_num<<"\n\n";

  source_file.close();
  target_file.close();
  alignment1_file.close();
  alignment2_file.close();
  output_file.close();
  return true;

}



bool AlignmentDifference::CountAlignedNumber(string &line_source, string &line_target, string &line_alignment1, string &line_alignment2, BasicMethod &basic_method, \
                                             int &src_words_num, int &tgt_words_num, int &src_difference_num, int &tgt_difference_num) {
  vector<string> source_words, target_words;
  vector<int> v_src_aln1_aligned_num, v_src_aln2_aligned_num;
  vector<int> v_tgt_aln1_aligned_num, v_tgt_aln2_aligned_num;
  multimap<int, int> m_src_aln1_aligned_word, m_src_aln2_aligned_word;
  set<string> s_src_aln1_aligned_word, s_src_aln2_aligned_word;
  multimap<int, int> m_tgt_aln1_aligned_word, m_tgt_aln2_aligned_word;
  set<string> s_tgt_aln1_aligned_word, s_tgt_aln2_aligned_word;
  basic_method.Split(line_source, ' ', source_words);
  v_src_aln1_aligned_num.resize(source_words.size(), 0);
  v_src_aln2_aligned_num.resize(source_words.size(), 0);
  src_words_num = (int)source_words.size();

  basic_method.Split(line_target, ' ', target_words);
  v_tgt_aln1_aligned_num.resize(target_words.size(), 0);
  v_tgt_aln2_aligned_num.resize(target_words.size(), 0);
  tgt_words_num = (int)target_words.size();

  MappingAlignments(line_alignment1, v_src_aln1_aligned_num, v_tgt_aln1_aligned_num, m_src_aln1_aligned_word, s_src_aln1_aligned_word, \
                    m_tgt_aln1_aligned_word, s_tgt_aln1_aligned_word, basic_method);
  MappingAlignments(line_alignment2, v_src_aln2_aligned_num, v_tgt_aln2_aligned_num, m_src_aln2_aligned_word, s_src_aln2_aligned_word, \
                    m_tgt_aln2_aligned_word, s_tgt_aln2_aligned_word, basic_method);

  CountDifference(v_src_aln1_aligned_num, v_src_aln2_aligned_num, m_src_aln1_aligned_word, m_src_aln2_aligned_word, s_src_aln2_aligned_word, basic_method, \
                  src_difference_num);
  CountDifference(v_tgt_aln1_aligned_num, v_tgt_aln2_aligned_num, m_tgt_aln1_aligned_word, m_tgt_aln2_aligned_word, s_tgt_aln2_aligned_word, basic_method, \
                  tgt_difference_num);

  return true;
}


bool AlignmentDifference::CountDifference(vector<int> &v_aln1_aligned_num, vector<int> &v_aln2_aligned_num, \
                                          multimap<int, int> &m_aln1_aligned_word, multimap<int, int> &m_aln2_aligned_word, \
                                          set<string> &s_aln2_aligned_word, BasicMethod &basic_method,  int &difference_num) {
  for (int i = 0; i < v_aln1_aligned_num.size(); ++i) {
    if (v_aln1_aligned_num[i] == v_aln2_aligned_num[i]) {
     multimap<int, int>::iterator iter_beg = m_aln1_aligned_word.lower_bound(i);
     multimap<int, int>::iterator iter_eng = m_aln1_aligned_word.upper_bound(i);
     for (; iter_beg != iter_eng; ++iter_beg) {
        int src_position = iter_beg->first;
        int tgt_position = iter_beg->second;
        string src_tgt = basic_method.intToString(src_position) + "-" + basic_method.intToString(tgt_position);
        if (s_aln2_aligned_word.find(src_tgt) != s_aln2_aligned_word.end()) {
          continue;
        } else {
          ++difference_num;
          break;
        }
     }
    } else {
      ++difference_num;
    }
  }
  return true;
}



bool AlignmentDifference::MappingAlignments(string &line_alignment, vector<int> &v_aligned_num, vector<int> &v_aligned_num_inv, \
                                            multimap<int, int> &m_aligned_word, set<string> &s_aligned_word, \
                                            multimap<int, int> &m_aligned_word_inv, set<string> &s_aligned_word_inv, BasicMethod &basic_method) {
  vector<string> word_alignments;
  basic_method.Split(line_alignment, ' ', word_alignments);
  for (vector<string>::iterator iter = word_alignments.begin(); iter != word_alignments.end(); ++iter) {
    s_aligned_word.insert(*iter);
    vector<string> src_tgt_aln;
    basic_method.Split(*iter, '-', src_tgt_aln);
    int src_position = atoi(src_tgt_aln.at(0).c_str());
    int tgt_position = atoi(src_tgt_aln.at(1).c_str());
    if (src_position >= v_aligned_num.size() || tgt_position >= v_aligned_num_inv.size()) {
      cerr<<"Warning: src_position="<<src_position<<" src_length="<<v_aligned_num.size()<<"\n"
          <<"         tgt_position="<<tgt_position<<" tgt_length="<<v_aligned_num_inv.size()<<"\n"<<flush;
      continue;
    }
    string tgt_src = src_tgt_aln.at(1) + "-" + src_tgt_aln.at(0);
    s_aligned_word_inv.insert(tgt_src);
    v_aligned_num[src_position] ++;
    v_aligned_num_inv[tgt_position] ++;
    m_aligned_word.insert(make_pair(src_position, tgt_position));
    m_aligned_word_inv.insert(make_pair(tgt_position, src_position));
  }
  return true;
}





bool AlignmentDifference::CleanString(BasicMethod &basic_method, string &line_source, string &line_target, string &line_alignment1, string &line_alignment2) {
  basic_method.ClearIllegalChar(line_source);
  basic_method.RmEndSpace(line_source);
  basic_method.RmStartSpace(line_source);

  basic_method.ClearIllegalChar(line_target);
  basic_method.RmEndSpace(line_target);
  basic_method.RmStartSpace(line_target);

  basic_method.ClearIllegalChar(line_alignment1);
  basic_method.RmEndSpace(line_alignment1);
  basic_method.RmStartSpace(line_alignment1);

  basic_method.ClearIllegalChar(line_alignment2);
  basic_method.RmEndSpace(line_alignment2);
  basic_method.RmStartSpace(line_alignment2);
  return true;
}





bool AlignmentDifference::CheckFiles(map<string, string> &parameters) {
  string file_key = "-aln1";
  CheckFile(parameters, file_key);
  alignment1_file_name_ = parameters[file_key];

  file_key = "-aln2";
  CheckFile(parameters, file_key);
  alignment2_file_name_ = parameters[file_key];

  file_key = "-src";
  CheckFile(parameters, file_key);
  source_file_name_ = parameters[file_key];

  file_key = "-tgt";
  CheckFile(parameters, file_key);
  target_file_name_ = parameters[file_key];

  file_key = "-output";
  if(parameters.find(file_key) == parameters.end()) {
    cerr<<"ERROR: Please add parameter \"-output\" in your command line!\n";
  }
  output_file_name_ = parameters[file_key];
  return true;
}


bool AlignmentDifference::CheckFile(map<string, string> &parameters, string &file_key) {
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



/*
 * $Name: Print Logo 
 * $Function: Print Information
 * $Date: 02/11/2016, Los Angeles, CA, USA
 */
bool AlignmentDifference::PrintAlignmentDifferenceLogo() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# Alignment Difference                                            #\n"
      <<"#                                            Version 0.0.1        #\n"
      <<"#                                            liqiangneu@gmail.com #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<flush;
  return true;
}


}

