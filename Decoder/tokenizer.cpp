/*
* $Id:
* 0062
*
* $File:
* tokenizer.cpp
*
* $Proj:
* Decoder for Statistical Machine Translation
*
* $Func:
* tokenizer
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
* 11/30/2015, 20:08, Los Angeles, USA
*/


#include "tokenizer.h"


namespace decoder_tokenizer {

/*
 * $Name: Initialize
 * $Function: 
 * $Date: 11/29/2015, Los Angeles, USA
 */
bool Tokenizer::Initialize(map<string, string> &parameters) {
  CheckFiles(parameters);
  if (parameters.find("-lang") == parameters.end() || \
     (parameters["-lang"] != "en" && parameters["-lang"] != "zh")) {
    language_ = "en";
  } else {
    language_ = parameters["-lang"];
  }
  return true; 
}


bool Tokenizer::StartTokenEng() {
  ifstream input_file (input_file_name_.c_str());
  if (!input_file) {
   cerr<<"ERROR: Please check the path of \""<<input_file_name_<<"\".\n"<<flush;
   exit (1);
  }

  ofstream output_clean_file (output_clean_file_name_.c_str());
  if (!output_clean_file) {
   cerr<<"ERROR: Can not open file \""<<output_clean_file_name_<<"\".\n"<<flush;
   exit (1);
  }

  ofstream output_file (output_file_name_.c_str());
  if (!output_file) {
   cerr<<"ERROR: Can not open file \""<<output_file_name_<<"\".\n"<<flush;
   exit (1);
  }

  ofstream output_lc_file (output_lc_file_name_.c_str());
  if (!output_lc_file) {
   cerr<<"ERROR: Can not open file \""<<output_lc_file_name_<<"\".\n"<<flush;
   exit (1);
  }

  ofstream log_file (log_file_name_.c_str());
  if (!log_file) {
    cerr<<"ERROR: Can not open file \""<<log_file_name_<<"\".\n"<<flush;
    exit (1);
  }
  log_file<<"Tokenization\n"<<flush;

  LoadPrefixes(log_file);

  int line_no = 0;
  string line = "";
  BasicMethod basic_method;
  cerr<<"Start tokenization ...\n"<<flush;
  log_file<<"Start tokenization ...\n"<<flush;
  while (getline(input_file, line)) {
    ++line_no;
    ConvertTabToSpace(line);
    basic_method.ClearIllegalChar(line);
    basic_method.RmEndSpace(line);
    basic_method.RmStartSpace(line);

    if ("" == line) {
      output_file<<"\n";
      output_lc_file<<"\n";
      output_clean_file<<"\n";
    } else {
      output_clean_file<<line<<"\n";
      line = " " + line + " ";
      SeperateAllSpecialChar(line);
      StayTogetherMultiDots(line);
      SeperateComma(line);
      ConvertPunctuation(line);
      if ("en" == language_) {
        SplitContractionsRightEng(line);
      } else {
        cerr<<"NOT HAVE!!! HAHAHAHAHA!!!\n";
        exit (1);
      }

      WordTokenMethod(line);
      string line_bak = line;
      line = "";
      basic_method.RemoveExtraSpace(line_bak, line);
      basic_method.RmEndSpace(line);
      basic_method.RmStartSpace(line);

      RestoreMultiDots(line);
      /*
      EscapeSpecialChars(line);
      */
      output_file<<line<<"\n";
      basic_method.ToLower(line);
      output_lc_file<<line<<"\n";
    }
    if (line_no % 10000 == 0) {
     cerr<<"\r  Processed "<<line_no<<" lines."<<flush;
     log_file<<"\r  Processed "<<line_no<<" lines."<<flush;
    }
  }
  cerr<<"\r  Processed "<<line_no<<" lines.\n"<<flush;
  log_file<<"\r  Processed "<<line_no<<" lines.\n"<<flush;

  input_file.close();
  output_file.close();
  output_clean_file.close();
  output_lc_file.close();
  log_file.close();
  return true;
}




/*
 * $Name: SeperateAllSpecialChar
 * $Function: Seperate out all "other" special characters
 * $Date: 11/30/2015, Los Angeles, USA
 */
bool Tokenizer::SeperateAllSpecialChar(string &line) {
  string replace_line;
  for(string::iterator iter = line.begin(); iter != line.end(); ++iter) {
   if (*iter == '~' || *iter == '@' || *iter == '^' || *iter == ':' || \
       *iter == '"' || *iter == '(' || *iter == ')' || *iter == '+' || \
       *iter == '/' || *iter == '=' || *iter == ';' || *iter == '$' || \
       *iter == '!' || *iter == '&' || *iter == '?' || *iter == '#' || \
       *iter == '%' || *iter == '_' || *iter == '*' || *iter == '[' || \
       *iter == ']' || *iter == '{' || *iter == '}' || *iter == '<' || \
       *iter == '>' || *iter == '|' || *iter == '\\') {
      replace_line += " ";
      replace_line.push_back(*iter);
      replace_line += " ";
    } else {
      replace_line.push_back(*iter);
    }
  } 
  line = replace_line;
  return true;
}


/*
 * $Name: StayTogetherMultiDots
 * $Function: Multi-dots stay together
 * $Date: 11/30/2015, Los Angeles, USA
 */
bool Tokenizer::StayTogetherMultiDots(string &line) {
  if (line.find("..") != string::npos) {
    string replace_line = "";
    for (string::iterator iter = line.begin(); iter != line.end(); ++iter) {
      if ('.' == *iter && iter != line.end() - 1 && '.' == (*(iter + 1)) ) {
        replace_line += " DOTMULTI";
        while('.' == (*(++iter)) && iter != line.end()) {
          replace_line.push_back(*iter);
        }
        if (iter != line.end()) {
          replace_line.push_back(*iter);
        }
      } else {
        replace_line.push_back(*iter);
      }
    }

    string::size_type position = replace_line.find("DOTMULTI.");
    while (position != string::npos) {
      if (replace_line.at(position + 9) != '.') {
        replace_line.replace(position, 9, "DOTDOTMULTI ");
      } else {
        replace_line.replace(position, 9, "DOTDOTMULTI");
      }
      position = replace_line.find("DOTMULTI.");
    }
    line = replace_line;
    return true;
  } else {
    return true;
  }
}


/*
 * $Name: RestoreMultiDots
 * $Function: Restore multi-dots
 * $Date: 11/30/2015, Los Angeles, USA
 */
bool Tokenizer::RestoreMultiDots(string &line) {
  string::size_type position = line.find("DOTDOTMULTI");
  while (position != string::npos) {
    line.replace(position, 11, "DOTMULTI.");
    position = line.find("DOTDOTMULTI");
  }

  position = line.find("DOTMULTI");
  while (position != string::npos) {
    line.replace(position, 8, ".");
    position = line.find("DOTMULTI");
  }
  return true;
}


bool Tokenizer::EscapeSpecialChars(string &line) {
  string replace_line;
  for (string::iterator iter = line.begin(); iter != line.end(); ++iter) {
    if ('&' == *iter) {
      replace_line += "&amp;";
    } else if ('|' == *iter) {
      replace_line += "&#124;";
    } else if ('<' == *iter) {
      replace_line += "&lt;";
    } else if ('>' == *iter) {
      replace_line += "&gt;";
    } else if ('\'' == *iter) {
      replace_line += "&apos;";
    } else if ('"' == *iter) {
      replace_line += "&quot;";
    } else if ('[' == *iter) {
      replace_line += "&#91;";
    } else if (']' == *iter) {
      replace_line += "&#93;";
    } else {
      replace_line.push_back(*iter);
    }
  }
  line = replace_line;
  return true;
}


/*
 * $Name: SeperateComma
 * $Function: Seperate out "," expect if within numbers (5,300)
 * $Date: 11/30/2015, Los Angeles, USA
 */
bool Tokenizer::SeperateComma(string &line) {
  string replace_line;
  if (line.find(',') != string::npos) {
    for (string::iterator iter = line.begin(); iter != line.end(); ++iter) {
      if (',' == *iter) {
        if (!(*(iter - 1) >= '0' && *(iter - 1) <= '9' && 
             *(iter + 1) >= '0' && *(iter + 1) <= '9')) {
          replace_line.push_back(' ');
          replace_line.push_back(*iter);
          replace_line.push_back(' ');
        } else {
          replace_line.push_back(*iter);
        }
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


/*
 * $Name: ConvertPunctuation
 * $Function: turn ` into '
 *            turn '' into "
 * $Date: 11/30/2015, Los Angeles, USA
 */
bool Tokenizer::ConvertPunctuation(string &line) {
  string replace_line = "";
  string::size_type position = line.find("`");
  while (position != string::npos) {
    line.replace(position, 1, "'");
    position = line.find("`");
  }

  position = line.find("''");
  while (position != string::npos) {
    line.replace(position, 2, " \" ");
    position = line.find("''");
  }
  return true;
}


/*
 * $Name: SplitContractionsRightEng
 * $Function: split contractions right
 * $Date: 11/30/2015, Los Angeles, USA
 */
bool Tokenizer::SplitContractionsRightEng(string &line) {
  string replace_line = "";
  if (line.find("'") != string::npos) {
    for (string::iterator iter = line.begin(); iter != line.end(); ++iter) {
      if ('\'' == *iter) {
        if (!IsAlpha(*(iter - 1)) && !IsAlpha(*(iter + 1))) {
          replace_line.push_back(' ');
          replace_line.push_back(*iter);
          replace_line.push_back(' ');
        } else if (!IsAlphaNumber(*(iter - 1)) && IsAlpha(*(iter + 1))) {
          replace_line.push_back(' ');
          replace_line.push_back(*iter);
          replace_line.push_back(' ');
        } else if (IsAlpha(*(iter - 1)) && !IsAlpha(*(iter + 1))) {
          replace_line.push_back(' ');
          replace_line.push_back(*iter);
          replace_line.push_back(' ');
        } else if (IsAlpha(*(iter - 1)) && IsAlpha(*(iter + 1))){
          replace_line.push_back(' ');
          replace_line.push_back(*iter);
        } else if (IsNumber(*(iter - 1)) && ('s' == (*(iter + 1)))) {
          replace_line.push_back(' ');
          replace_line.push_back(*iter);
        } else {
          replace_line.push_back(*iter);
        }
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


bool Tokenizer::WordTokenMethod(string &line) {
  vector<string> v_words;
  BasicMethod basic_method;
  basic_method.Split(line, ' ', v_words);
  line = "";

  int i = 0;
  for (vector<string>::iterator iter = v_words.begin(); iter != v_words.end(); ++iter) {
    string word = *iter;
    if (1 != iter->size() && '.' == *(iter->end() - 1)) {
      string pre = iter->substr(0, iter->size() - 1);
      if (pre.find(".") != string::npos && HaveAlpha(pre)) {                                     // abc.xyz.
        // no change
      } else if (m_prefix_ref_.find(pre) != m_prefix_ref_.end() && 1 == m_prefix_ref_[pre]) {    // prefix_ref have pre
        // no change
      } else if (i < (v_words.size() - 1) && IsLower(v_words.at(i + 1).at(0))) {                 // xxxx. a
        // no change
      } else if ((m_prefix_ref_.find(pre) != m_prefix_ref_.end() && 2 == m_prefix_ref_[pre]) && \
                 (i < (v_words.size() - 1) && IsNumber(v_words.at(i + 1).at(0)))) {              // No. 1, pp. 1, Art. 1
        // no change
      } else {
        word = pre + " .";
      }
    }
    line += word + " ";
    ++i;
  }

  return true;
}


bool Tokenizer::ConvertTabToSpace(string &line) {
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


bool Tokenizer::HaveAlpha(string &line) {
  for (string::iterator iter = line.begin(); iter != line.end(); ++iter) {
    if (IsAlpha(*iter)) {
      return true;
    }
  }
  return false;
}


bool Tokenizer::IsAlpha(char character) {
  if ((character >= 'a' && character <= 'z') || \
      (character >= 'A' && character <= 'Z')) {
    return true;
  } else {
    return false;
  }
}


bool Tokenizer::IsLower(char character) {
  if (character >= 'a' && character <= 'z') {
    return true;
  } else {
    return false;
  }
}


bool Tokenizer::IsNumber(char character) {
  if (character >= '0' && character <= '9') {
    return true;
  } else {
    return false;
  }
}


bool Tokenizer::IsAlphaNumber(char character) {
  if ((character >= 'a' && character <= 'z') || \
      (character >= 'A' && character <= 'Z') || \
      (character >= '0' && character <= '9')) {
    return true;
  } else {
    return false;
  }
}


bool Tokenizer::LoadPrefixes(ofstream &log_file) {
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


bool Tokenizer::CheckFiles(map<string, string> &parameters) {
  string file_key = "-input";
  CheckFile(parameters, file_key);
  input_file_name_ = parameters[file_key];

  file_key = "-prefixfile";
  CheckFile(parameters, file_key);
  prefix_file_name_ = parameters[file_key];

  file_key = "-output";
  if (parameters.find(file_key) == parameters.end()) {
    output_file_name_ = input_file_name_ + ".tok";
  } else {
    output_file_name_ = parameters[file_key];
  }

  output_clean_file_name_ = parameters["-input"] + ".clean";
  output_lc_file_name_ = output_file_name_ + ".lc";

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
 * $Date: 11/29/2015, 21:44, Los Angeles, USA
 */
bool Tokenizer::CheckFile(map<string, string> &parameters, string &file_key) {
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
 * $Name: PrintTokenizerLogo
 * $Function: Print logo
 * $Date: 11/23/2015, Los Angeles, USA
 */
bool Tokenizer::PrintTokenizerLogo() {
  cerr<<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<"# Tokenizer for Statistical Machine Translation                   #\n"
      <<"#                                            Version 0.0.1        #\n"
      <<"#                                            liqiangneu@gmail.com #\n"
      <<"####### SMT ####### SMT ####### SMT ####### SMT ####### SMT #######\n"
      <<flush;
  return true;
}

}

