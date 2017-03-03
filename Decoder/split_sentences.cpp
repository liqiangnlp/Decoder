/*
* $Id:
* 0064
*
* $File:
* split_sentences.cpp
*
* $Proj:
* Decoder for Statistical Machine Translation
*
* $Func:
* Split Sentences
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
* 12/02/2015, 20:08, Los Angeles, USA
*/


#include "split_sentences.h"


namespace decoder_split_sentences {

/*
 * $Name: Initialize
 * $Function: 
 * $Date: 12/02/2015, Los Angeles, USA
 */
bool SplitSentences::Initialize(map<string, string> &parameters) {
  CheckFiles(parameters);
  if (parameters.find("-lang") == parameters.end() || \
     (parameters["-lang"] != "en" && parameters["-lang"] != "zh")) {
    language_ = "en";
  } else {
    language_ = parameters["-lang"];
  }
  return true; 
}


bool SplitSentences::StartSplitSentencesEng() {

  ifstream input_file (input_file_name_.c_str());
  if (!input_file) {
   cerr<<"ERROR: Please check the path of \""<<input_file_name_<<"\".\n"<<flush;
   exit (1);
  }

  ofstream output_file (output_file_name_.c_str());
  if (!output_file) {
   cerr<<"ERROR: Can not open file \""<<output_file_name_<<"\".\n"<<flush;
   exit (1);
  }

  ofstream log_file (log_file_name_.c_str());
  if (!log_file) {
    cerr<<"ERROR: Can not open file \""<<log_file_name_<<"\".\n"<<flush;
    exit (1);
  }
  log_file<<"Split Sentences\n"<<flush;

  LoadPrefixes(log_file);

  int line_no = 0;
  string line = "";
  BasicMethod basic_method;
  cerr<<"Start spliting sentences ...\n"<<flush;
  log_file<<"Start spliting sentences ...\n"<<flush;
  while (getline(input_file, line)) {
    ++line_no;
    ConvertTabToSpace(line);
    basic_method.ClearIllegalChar(line);
    basic_method.RmEndSpace(line);
    basic_method.RmStartSpace(line);
    
    if ("" == line) {
      continue;
    }
    
    string line_bak = line;
    line = "";
    basic_method.RemoveExtraSpace(line_bak, line);
    ProcessNonPeriodEnd(line);
    ProcessMultiDots(line);
    EndPunctInsideQuote(line);
    EndPunctuation(line);
    CheckAllRemainPeriods(line);
    ClearSentences(line);

    output_file<<line<<"\n";
    if (line_no % 1000 == 0) {
      cerr<<"\r  Processed "<<line_no<<" lines."<<flush;
      log_file<<"\r  Processed "<<line_no<<" lines."<<flush;
    }
  }
  cerr<<"\r  Processed "<<line_no<<" lines.\n"<<flush;
  log_file<<"\r  Processed "<<line_no<<" lines.\n"<<flush;

  input_file.close();
  output_file.close();
  log_file.close();
  return true;
}


/*
 * $Name: ProcessNonPeriodEng
 * $Function: non-period end of sentence markers (?!)
 *            followed by sentence starters.
 *            s/([?!]) +([\'\"\(\[\p{IsPi}]*[\p{IsUpper}])/$1\n$2/g
 * $Date: 12/02/2015, Los Angeles, USA
 */
bool SplitSentences::ProcessNonPeriodEnd(string &line) {
  if (line.size() >= 3) {
    for (string::iterator iter = line.begin(); iter < line.end() - 2; ++iter) {
      if ('?' == *iter || '!' == *iter) {
        if (' ' == *(iter + 1)) {
          if (IsPossibleSentenceStarter(iter + 2, line)) {
            *(iter + 1) = '\n';
          }
        }
      }
    }
  }
  return true;
}


/*
 * $Name: ProcessMultiDots
 * $Function: multi-dots followed by sentence starters
 *            s/(\.[\.]+) +([\'\"\(\[\p{IsPi}]*[\p{IsUpper}])/$1\n$2/g
 * $Date: 12/02/2015, Los Angeles, USA
 */
bool SplitSentences::ProcessMultiDots(string &line) {
  if (line.size() >= 4) {
    for (string::iterator iter = line.begin(); iter < line.end() - 2; ++iter) {
      if ((iter > line.begin()) && '.' == *iter && '.' == *(iter - 1)) {
        if (' ' == *(iter + 1)) {
          if (IsPossibleSentenceStarter(iter + 2, line)) {
            *(iter + 1) = '\n';
          }
        }
      }
    }
  }
  return true;
}


/*
 * $Name: EndPunctInsideQuote
 * $Function: Add breaks for sentences that end with some sort of punctuation 
 *            inside a quote or parenthetical and are followed by a possible 
 *            sentence starter punctuation and upper case
 *            s/([?!\.][\ ]*[\'\"\)\]\p{IsPf}]+) +([\'\"\(\[\p{IsPi}]*[\ ]*[\p{IsUpper}])/$1\n$2/g
 * $Date: 12/02/2015, Los Angeles, USA
 */
bool SplitSentences::EndPunctInsideQuote(string &line) {
  if (line.size() >= 4) {
    for(string::iterator iter = line.begin(); iter != line.end() - 3; ++iter) {
      if ('?' == *iter || '!' == *iter|| '.' == *iter) {
        string::iterator iter_1 = iter + 1;
        for (; iter_1 < line.end(); ++iter_1) {
          if (' ' == *iter_1) {
            continue;
          } else {
            break;
          }
        }

        string::iterator iter_2 = iter_1;
        for (; iter_2 < line.end(); ++iter_2) {
          if (IsPf(*iter_2)) {
            continue;
          } else {
            break;
          }
        }
        if (iter_2 == iter_1) {
          continue;
        }

        string::iterator iter_3 = iter_2;
        for (; iter_3 < line.end(); ++iter_3) {
          if (' ' == *iter_3) {
            continue;
          } else {
            break;
          }
        }
        if (iter_3 == iter_2) {
          continue;
        }

        if (IsPossibleSentenceStarter(iter_3, line)) {
          *iter_2 = '\n';
        }
      }
    }
  }
  return true;
}


/*
 * $Name: EndPunctuation
 * $Function: add breaks for sentences that end with some sort of punctuation are followed 
 *            by a sentence starter punctuation and upper case
 *            s/([?!\.]) +([\'\"\(\[\p{IsPi}]+[\ ]*[\p{IsUpper}])/$1\n$2/g
 * $Date: 12/02/2015, Los Angeles, USA
 */
bool SplitSentences::EndPunctuation(string &line) {
  if (line.size() >= 4) {
    for (string::iterator iter = line.begin(); iter < line.end() - 3; ++iter) {
      if ('?' == *iter || '!' == *iter || '.' == *iter) {
        if (' ' == *(iter + 1) && IsSentenceStarter(iter + 2, line)) {
          *(iter + 1) = '\n';
        }
      } 
    }
  }
  return true;
}


/*
 * $Name: CheckAllRemainPeriods
 * $Function: special punctuation cases are covered. Check all remaining periods.
 * $Date: 12/03/2015, Los Angeles, USA
 */
bool SplitSentences::CheckAllRemainPeriods(string &line) {
  vector<string> v_words;
  BasicMethod basic_method;
  basic_method.Split(line, ' ', v_words);
  line = "";
  for (vector<string>::iterator iter = v_words.begin(); iter != v_words.end(); ++iter) {
    if (iter->size() > 0 && '.' == *(iter->end() - 1)) {
      if (IsKnownHonorific(*iter)) {
        ; // not breaking
      } else if (IsUppercaseAcronym(*iter)) {
        ; // not breaking
      } else if (iter < v_words.end() - 1 && IsNextWordUppercaseOrNumber(*(iter + 1))) {
        string prefix = GetPrefixGivenWord(*iter);
        if (prefix.size() > 0 && 2 == m_prefix_ref_[prefix] &&  IsNumber((*(iter + 1)).at(0))) {
          ;
        } else {
          iter->push_back('\n');
        }
      }
    }
    line += *iter + " ";
  }
  return true;
}


bool SplitSentences::ClearSentences(string &line) {
  string line_bak = line;
  line = "";
  BasicMethod basic_method;
  basic_method.RemoveExtraSpace(line_bak, line);
  basic_method.RmEndSpace(line);
  basic_method.RmStartSpace(line);

  line_bak = line;
  line = "";
  for (string::iterator iter = line_bak.begin(); iter != line_bak.end(); ++iter) {
    if(' ' == *iter && (iter < line_bak.end() - 1) && '\n' == *(iter + 1)) {
      line.push_back('\n');
      ++iter;
    } else if ('\n' == *iter && (iter < line_bak.end() - 1) && ' ' == *(iter + 1)) {
      line.push_back('\n');
      ++iter;
    } else {
      line.push_back(*iter);
    }
  }

  return true;
}


bool SplitSentences::IsKnownHonorific(string &word) {
  if (word.size() >= 2 && '.' == word.at(word.size() - 1)) {
    string prefix = GetPrefixGivenWord(word);
    if ("" != prefix && 1 == m_prefix_ref_[prefix]) {
      return true;
    }
  }
  return false;
}


bool SplitSentences::IsUppercaseAcronym(string &word) {
  if (word.size() >= 3 && '.' == word.at(word.size() - 1)) {
    string prefix = GetPrefixGivenWord(word);
    if (prefix.size() >= 2 && (IsUpper(prefix.at(prefix.size() - 1)) || '-' == prefix.at(prefix.size() - 1))) {
      int i = (int)prefix.size() - 2;
      for(; i >= 0; --i) {
        if (IsUpper(prefix.at(i)) || '-' == prefix.at(i)) {
          continue;
        } else if ('.' == prefix.at(i)) {
          break;
        } else {
          i = -1;
          break;
        }
      }
      if (i >= 0) {
        return true;
      }
    }
  }
  return false;
}


///^([ ]*[\'\"\(\[\p{IsPi}]*[ ]*[\p{IsUpper}0-9])/
bool SplitSentences::IsNextWordUppercaseOrNumber(string &word) {
  for (string::iterator iter = word.begin(); iter != word.end(); ++iter) {
    if (' ' == *iter || IsPi(*iter)) {
      continue;
    } else if (IsUppercaseOrNumber(*iter)) {
      return true;
    } else {
      return false;
    }
  }
  return false;
}


bool SplitSentences::IsUppercaseOrNumber(char character) {
  if (IsUpper(character) || IsNumber(character)) {
    return true;
  } else {
    return false;
  }
}


string SplitSentences::GetPrefixGivenWord(string &word) {
  string prefix = "";
  if (word.size() >= 2 && '.' == word.at(word.size() - 1)) {
    int i = (int)word.size() - 2;
    int j = 0;
    for (; i >= 0; --i) {
      if ('.' == word.at(i)) {
        continue;
      } else {
        break;
      }
    }

    for (j = i; j >= 0; --j) {
      if (IsAlphaNumber(word.at(j)) || '.' == word.at(j) || '-' == word.at(j)) {
        continue;
      } else {
        break;
      }
    }
    if (i > j) {
      prefix = word.substr(j + 1, i - j);
    }
  }
  return prefix;
}


bool SplitSentences::IsPi(char character) {
  if ('\'' == character || '"' == character || '(' == character || '[' == character) {
    return true;
  } else {
    return false;
  }
}


bool SplitSentences::IsPf(char character) {
  if ('\'' == character || '"' == character || ')' == character || ']' == character) {
    return true;
  } else {
    return false;
  }
}


bool SplitSentences::IsLower(char character) {
  if (character >= 'a' && character <= 'z') {
   return true;
  } else {
    return false;
  }
}


bool SplitSentences::IsUpper(char character) {
  if (character >= 'A' && character <= 'Z') {
    return true;
  } else {
    return false;
  }
}


bool SplitSentences::IsNumber(char character) {
  if (character >= '0' && character <= '9') {
    return true;
  } else {
    return false;
  }
}


bool SplitSentences::IsAlphaNumber(char character) {
  if ((character >= 'a' && character <= 'z') || \
      (character >= 'A' && character <= 'Z') || \
      (character >= '0' && character <= '9')) {
    return true;
  } else {
    return false;
  }
}



bool SplitSentences::IsPossibleSentenceStarter(string::iterator iter, string &line) {
  if (IsUpper(*iter)) {
    return true;
  } else {
    string::iterator iter_1 = iter;
    for (; iter_1 < line.end(); ++iter_1) {
      if (IsPi(*iter_1)) {
        continue;
      } else {
        break;
      }
    }

    for (; iter_1 < line.end(); ++iter_1) {
      if (' ' == *iter_1) {
        continue;
      } else {
        break;
      }
    }

    if (IsUpper(*iter_1)) {
      return true;
    } else {
      return false; 
    }
  }
}


/*
 * $Name: IsSentenceStarter
 * $Function: ([\'\"\(\[\p{IsPi}]+[\ ]*[\p{IsUpper}])
 * $Date: 12/04/2015, 21:44, Los Angeles, USA
 */
bool SplitSentences::IsSentenceStarter(string::iterator iter, string &line) {
  string::iterator iter_new = iter;
  for (;iter_new < line.end(); ++iter_new) {
    if (IsPi(*iter_new)) {
      continue;
    } else {
      break;
    }
  }
  if (iter_new == iter) {
    return false;
  }

  for (;iter_new < line.end(); ++iter_new) {
    if (' ' == *iter_new) {
      continue;
    } else {
      break;
    }
  }

  if (IsUpper(*iter_new)) {
    return true;
  } else {
    return false;
  }
}


bool SplitSentences::LoadPrefixes(ofstream &log_file) {
  cerr<<"Start loading prefixes ...\n"<<flush;
  log_file<<"Start loading prefixes ...\n"<<flush;
  ifstream prefix_file (prefix_file_name_.c_str());
  if (!prefix_file) {
    cerr<<"ERROR: Please check the path of \""<<prefix_file_name_<<"\".\n"<<flush;
    log_file<<"ERROR: Please check the path of \""<<prefix_file_name_<<"\".\n"<<flush;
    exit (1);
  }

  int line_no = 0;
  string line = "";
  BasicMethod basic_method;
  while (getline(prefix_file, line)) {
    ++line_no;

    basic_method.ClearIllegalChar(line);
    basic_method.RmEndSpace(line);
    basic_method.RmStartSpace(line);

    if ("" == line) {
      continue;
    } else if ('#' == line.at(0)) {
      continue;
    } else {
      vector<string> v_tmp_prefix;
      basic_method.Split(line, ' ', v_tmp_prefix);
      if(2 == v_tmp_prefix.size()) {
        m_prefix_ref_[v_tmp_prefix[0]] = 2;
      } else {
        m_prefix_ref_[v_tmp_prefix[0]] = 1;
      }
    }

    if (line_no % 100 == 0) {
      cerr<<"\r  Processed "<<line_no<<" lines.";
      log_file<<"\r  Processed "<<line_no<<" lines.";
    }
  }
  cerr<<"\r  Processed "<<line_no<<" lines.\n";
  log_file<<"\r  Processed "<<line_no<<" lines.\n";
  prefix_file.close();
  return true;
}


bool SplitSentences::ConvertTabToSpace(string &line) {
  if (line.find('\t') != string::npos) {
    string replace_line = "";
    replace_line.reserve(line.size());
    for (string::iterator iter = line.begin(); iter != line.end(); ++iter) {
      if ('\t' == *iter) {
        replace_line.push_back(' ');
      } else {
        replace_line.push_back(*iter);
      }
    }
    line = replace_line;
    return true;
  } else {
    return true;
  }
}



bool SplitSentences::CheckFiles(map<string, string> &parameters) {
  string file_key = "-input";
  CheckFile(parameters, file_key);
  input_file_name_ = parameters[file_key];

  file_key = "-prefixfile";
  CheckFile(parameters, file_key);
  prefix_file_name_ = parameters[file_key];

  file_key = "-output";
  if(parameters.find(file_key) == parameters.end()) {
    output_file_name_ = input_file_name_ + ".splitsent";
  } else {
    output_file_name_ = parameters[file_key];
  }

  file_key = "-log";
  if (parameters.find(file_key) == parameters.end()) {
    log_file_name_ = output_file_name_ + ".log";
  } else {
    log_file_name_ = parameters[file_key];
  }
  return true;
}


/*
 * $Name: CheckFile
 * $Function: If the file to be checked does not exist, exit!
 * $Date: 12/02/2015, 21:44, Los Angeles, USA
 */
bool SplitSentences::CheckFile(map<string, string> &parameters, string &file_key) {
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
 * $Name: PrintSplitSentencesLogo
 * $Function: Print logo
 * $Date: 12/02/2015, Los Angeles, USA
 */
bool SplitSentences::PrintSplitSentencesLogo() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# Split Sentences                                                 #\n"
      <<"#                                            Version 0.0.1        #\n"
      <<"#                                            liqiangneu@gmail.com #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<flush;
  return true;
}

}

