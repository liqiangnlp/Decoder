/*
 * Copyright 2013 Qiang Li.
 *
 * $Id:
 * 0045
 *
 * $File:
 * tree2tree_decoder.h
 *
 * $Proj:
 * Decoder
 *
 * $Func:
 * Tree 2 Tree Decoder
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
 * 2014-10-23
 */

#ifndef DECODER_TREE2TREE_DECODER_H_
#define DECODER_TREE2TREE_DECODER_H_

#include <iostream>
#include <string>
#include <map>
#include <iomanip>
#include <list>
#include <ctime>
#include <cmath>
#include <algorithm>
#include "basic_method.h"
#include "OurTree.h"
#include "tree.h"
#include "NiuLM.h"
using namespace std;
using namespace basic_method;
using namespace decoder_our_tree;
using namespace decoder_tree;
using namespace niu_lm;

namespace decoder_tree2tree_decoder {

#define MAX_WORD_LENGTH 1024
#define MILLION 1000000
#define MAX_HYPOTHESIS_NUM_IN_CELL 100000
#define FLOAT_MIN -1.0E38
#define MODEL_SCORE_MIN -1.0E30



//////////////////////////////////////////////////////////////////////////
class Tree2TreeCell;
class UnitRules;

typedef class NonTerminalBoundary {
 public:
  int left_;
  int right_;
  Tree2TreeCell * cell_node_;       // for tree-parsing only

 public:
  NonTerminalBoundary() {
    left_ = 0;
    right_ = 0;
    cell_node_ = NULL;
  }
  NonTerminalBoundary(int &left, int &right, Tree2TreeCell * tree2tree_cell) {
    left_ = left;
    right_ = right;
    cell_node_ = tree2tree_cell;
  }
  ~NonTerminalBoundary() {
    left_ = 0;
    right_ = 0;
    cell_node_ = NULL;
  }
}* P_NonTerminal_Boundary;


class MatchedPattern {
 public:
  string key_;                               // for indexing
  vector<NonTerminalBoundary> v_matching_;   // matched variables
  UnitRules * p_unit_rules_;

 public:
  MatchedPattern() {
    p_unit_rules_ = NULL;
  }

 public:
  void Init(string &key, int &key_length);
};


//////////////////////////////////////////////////////////////////////////
class Tree2TreeFeatureValue {
 public:
  float weight_;
  float min_value_;
  float max_value_;
  float fixed_;

 public:
  Tree2TreeFeatureValue() {};
  Tree2TreeFeatureValue(const float &weight, const float &min_value, const float &max_value, const float &fixed)
                       : weight_(weight), min_value_(min_value), max_value_(max_value), fixed_(fixed) {};
};


class Tree2TreeFeatures: public BasicMethod {
 public:
  typedef map<string, string> STRPAIR;

 public:
  size_t features_number_;
  vector<Tree2TreeFeatureValue> v_feature_values_;

 public:
  bool ParsingFeatures(STRPAIR &parameters);
};


class Tree2TreeConfiguration {
 public:
  typedef map<string, string> STRPAIR;

 public:
  int      nround_;
  int      ngram_;
  int      beamsize_;
  int      nbest_;
  int      nref_;
  int      nthread_;
  int      free_feature_;

 public:
  bool     use_punct_pruning_;
  bool     use_cube_pruning_;
  bool     use_null_translation_;
  bool     output_oov_;
  bool     label_oov_;
  bool     mert_flag_;
  bool     tree_parsing_flag_;
  bool     generate_tree_structure_flag_;
  bool     grammar_encoding_flag_;
  bool     no_fltering_flag_;               // does not filter out any rule
  bool     fast_decoding_flag_;
  bool     allow_null_substitution_flag_;
  bool     allow_unary_production_flag_;
  bool     replace_generalization_flag_;
  bool     dump_left_hypothesis_flag_;
  bool     deal_with_decoding_failure_flag_;

 public:
  string   config_file_;
  string   output_file_;
  string   log_file_;

 public:
  string   lm_file_;
  string   target_vocab_file_;
  string   scfg_rule_set_file_;
  string   test_file_;

 public:
  float model_score_min_;

 public:
  Tree2TreeFeatures tree2tree_features_;

 public:
  Tree2TreeConfiguration() {
    use_punct_pruning_ = false;
    use_cube_pruning_ = false;
    use_null_translation_ = false;
    output_oov_ = false;
    label_oov_ = false;
    mert_flag_ = false;
    tree_parsing_flag_ = false;
    generate_tree_structure_flag_ = false;
    grammar_encoding_flag_ = false;
    no_fltering_flag_ = false;
    fast_decoding_flag_ = true;
    allow_null_substitution_flag_ = true;
    allow_unary_production_flag_ = true;
    replace_generalization_flag_ = true;
    dump_left_hypothesis_flag_ = true;
    deal_with_decoding_failure_flag_ = true;

    model_score_min_ = -1.0E30f;
  }
  ~Tree2TreeConfiguration() {}

 public:
  bool Init(STRPAIR &parameters);
  bool CheckParamsInConf(STRPAIR &parameters);
  bool CheckEachParamInConf(STRPAIR &parameters, string &parameter_key, string &default_value);
  bool CheckFilesInConf(STRPAIR &parameters);
  bool CheckFileInConf(STRPAIR &parameters, string &file_key);
  bool CheckFileInComm(STRPAIR &parameters, string &file_key);
  bool PrintConfig();
  bool PrintFeatures();
};


//////////////////////////////////////////////////////////////////////////
class LanguageModel : public BasicMethod {
 private:
  int lm_index_;

 public:
  LanguageModel() {};
  ~LanguageModel();

 public:
  void LoadModel(Tree2TreeConfiguration &tree2tree_config);         // Load ngram language model
  float GetProbability(vector<int> &wid, int &begin, int &end);     // Get n-gram probability, i.e. Pr(w_n|w_1...w_n-1)
  float GetProbability(int * wid, int &begin, int &end);     // Get n-gram probability, i.e. Pr(w_n|w_1...w_n-1)
  void UnloadModel();                                               // Unload language model
};


//////////////////////////////////////////////////////////////////////////
class Nonterminal {
 public:
  string symbol_;
  string symbol_source_;
  string symbol_target_;

 public:
  Nonterminal() {}
  Nonterminal(string &symbol, string &symbol_source, string &symbol_target) : symbol_(symbol), symbol_source_(symbol_source), symbol_target_(symbol_target) {};
  ~Nonterminal() {}
};


//////////////////////////////////////////////////////////////////////////
class ScoresOfTree2TreeTranslateOption {
 public:
  float number_of_target_words_;  // number of target words, feature '1'
  float number_of_phrasal_rules_; // number of phrasal_rules, feature '9'

 public:
  float pr_tgt_given_src_;        // Pr(e|f), f->e translation probability, table '0', feature '2'
  float lex_tgt_given_src_;       // Lex(e|f), f->e lexical weight, table '1', feature '3'
  float pr_src_given_tgt_;        // Pr(f|e), e->f translation probability, table '2', feature '4'
  float lex_src_given_tgt_;       // Lex(f|e), e->f lexical weight, table '3', feature '5'
  float number_of_rules_;         // Natural Logarithm e, number of rules, table '4', feature '6'
  float pr_r_give_rootr_;         // Pr(r|root(r)), table '5', feature '11'
  float pr_sr_give_rootr_;        // Pr(s(r)|root(r)), table '6', feature '12'
  float is_lexicalized_;          // IsLexicalized, table '7', feature '13'
  float is_composed_;             // IsComposed, table '8', feature '14'
  float is_low_frequency_;        // IsLowFrequency, table '9', feature '15'
  float bi_lex_links_;            // number of bi-lex links (not fired in current version, 0), table '10', feature '7'

 public:
  vector<float> v_free_feature_value_;

 public:
  ScoresOfTree2TreeTranslateOption() {
    number_of_target_words_ = 0;
    number_of_phrasal_rules_ = 0;
    pr_tgt_given_src_ = 0;
    lex_tgt_given_src_ = 0;
    pr_src_given_tgt_ = 0;
    lex_src_given_tgt_ = 0;
    number_of_rules_ = 0;
    pr_r_give_rootr_ = 0;
    pr_sr_give_rootr_ = 0;
    is_lexicalized_ = 0;
    is_composed_ = 0;
    is_low_frequency_ = 0;
    bi_lex_links_ = 0;
  }
  ~ScoresOfTree2TreeTranslateOption() {}
};


//////////////////////////////////////////////////////////////////////////
class UnitRule;

class ScfgRule {
 public:
  string root_;
  string root_source_;
  string root_target_;

 public:
  string source_;
  string target_;
  string alignment;

 public:
  vector<Nonterminal> v_nonterminal_;
  int nonterminal_count_;

 public:
  ScoresOfTree2TreeTranslateOption scores_of_translation_option_;
  vector<int> v_word_id_;
  int target_word_count_;
  string key_;

 public:
  int rule_count_;

 public:
  vector<UnitRule> v_unit_rules_;

 public:
  ScfgRule() : nonterminal_count_(0), target_word_count_(0), rule_count_(0) {}
  ~ScfgRule() {}
};


//////////////////////////////////////////////////////////////////////////
class UnitRule : public BasicMethod {
 public:
  string root_;
  string root_source_;
  string root_target_;

 public:
  string source_;
  string target_;
  string alignment;

 public:
  vector<Nonterminal> v_nonterminal_;
  int nonterminal_count_;

 public:
  ScoresOfTree2TreeTranslateOption scores_of_translation_option_;
  vector<int> v_word_id_;
  int target_word_count_;
  string key_;

 public:
  bool is_complete_;
  bool is_lexical_rule_;
  ScfgRule * p_parent_rule_;
  short id_;

 public:
   UnitRule() {
     is_complete_ = false;
     is_lexical_rule_ = false;
     nonterminal_count_ = 0;
     target_word_count_ = 0;
     id_ = 0;
   }
   ~UnitRule() {};

 public:
  bool InitBasicInformation(ScfgRule &scfg_rule);
};


//////////////////////////////////////////////////////////////////////////
class UnitRules {
 public:
  list<UnitRule> l_unit_rules_;
};


//////////////////////////////////////////////////////////////////////////
class SimpleRule {
 public:
  string root_;
  string root_source_;
  string root_target_;

 public:
  string source_;
  string target_;
  string alignment;

 public:
  vector<Nonterminal> v_nonterminal_;
  int nonterminal_count_;

 public:
  ScoresOfTree2TreeTranslateOption scores_of_translation_option_;
  vector<int> v_word_id_;
  int target_word_count_;
  string key_;

 public:
  int begin_;
  int end_;
  list<string> slots_;
  bool is_unary_rule_;
  float viterbi_score_;
  float viterbi_without_lm_score_;
  float viterbi_lm_score_;
  short state_;                  // 
  string translation_;

 public:
  SimpleRule() {
    nonterminal_count_ = 0;
    target_word_count_ = 0;

    begin_ = 0;
    end_ = 0;
    
    is_unary_rule_ = false;
    viterbi_score_ = 0;
    viterbi_without_lm_score_ = 0;
    viterbi_lm_score_ = 0;
    state_ = 0;
  }
 ~SimpleRule() {};

};


//////////////////////////////////////////////////////////////////////////
class SynchronousGrammar : public BasicMethod {
 public:
  map<string, int> m_target_vocab_;

 public:
  map<string, string> m_named_entity_label_dict_;

 public:
  map<string, UnitRules> m_translation_options_;
  map<string, UnitRules> m_translation_options_symbol_;

 public:
  list<ScfgRule> l_scfg_rule_base_;

 public:
  list<string> l_source_side_base_;

 public:
  float in_complete_hypothesis_rate_;

 public:
  time_t start_time_;

 public:
  SynchronousGrammar() {
    in_complete_hypothesis_rate_ = 0.5f;
  };
  ~SynchronousGrammar() {};

 public:
  bool InitTargetVocab(Tree2TreeConfiguration &tree2tree_config);

 public:
  bool InitNamedEntityLabelDict();

 public:
  bool GetWordId(string &translation, vector<int> &v_word_id);
  bool GetWordId(ScfgRule &scfg_rule);
  bool GetMetaWordId(ScfgRule &scfg_rule);

 public:
  int LoadScfg(Tree2TreeConfiguration &tree2tree_config);
  bool ParseScfgRule(Tree2TreeConfiguration &tree2tree_config, vector<string> &rule_domains, ScfgRule &scfg_rule);

 private:
  bool LoadUnitRules(Tree2TreeConfiguration &tree2tree_config, ifstream &in_file, ScfgRule &scfg_rule);
  bool GenerateUnitRules(Tree2TreeConfiguration &tree2tree_config, ScfgRule &scfg_rule);
  bool AddSourceSideForUnitRules(ScfgRule &scfg_rule);

 private:
  bool SortRules();

 public:
  UnitRules * FindRuleListWithSymbol(string &source);
  UnitRules * FindRuleList(string &key);

 private:
  bool ParseRootSymbol(string &root, ScfgRule &scfg_rule);

 private:
  bool ParseSourceSide(ScfgRule &scfg_rule, bool index_for_tree_parsing);

 private:
  bool ParseFeatureValues(string &features_domain, ScoresOfTree2TreeTranslateOption &scores_of_translation_option);

 private:
  bool AddOptionWithRuleSourceSide(UnitRule &unit_rule);
  bool AddOptionWithSymbol(UnitRule &unit_rule);

 private:
  bool AddOption(UnitRule &unit_rule);

 private:
  bool AddMetaRules();
  bool AddMetaRuleEntry(string &symbol);

 private:
  bool IsUnaryProduction(string &source);
  bool IsUnaryProductionForSourceSide(string &root_source, string &source);

 private:
  bool IsValidWordIdForScfg(ScfgRule &scfg_rule);
  bool InformalCheck(string &source_domain, string &target_domain, ScfgRule &scfg_rule);

 public:
  bool IsNonLexical(string &source);
  bool IsComplete(string &root);

 private:
  bool IsValid(Tree2TreeConfiguration &tree2tree_config, string &source, string &target);
  bool IsValid(string &source, string &target, string &type);

 private:
  bool IsValidForScfg(string &source, string &target);

 public:
  bool IsMetaSymbol(string &phrase);
};


//////////////////////////////////////////////////////////////////////////
class DecodingSentence {
 public:
  string sentence_;                       // input sentence, format: source sentence ||| NEU translations (optional) ||| parse tree (optional)
  list<SimpleRule> l_viterbi_rules_;      // rules that used in viterbi derivations
  list<SimpleRule> l_matched_rules_;      // rules that matched onto the input string/tree, this feature is available for tree-parsing only
  int sentence_id_;

 public:
  DecodingSentence() {
    sentence_id_ = 0;
    sentence_ = "";
  }
};


//////////////////////////////////////////////////////////////////////////
class Tree2TreeHypothesis;
class HypothesisList;

class SlotInformation {
 public:
  int slot_number_;

 public:
  vector<int> v_hypothesis_number_;
  vector<int> v_inverted_nonterminal_index_;
  vector<int> v_word_index_;

 public:
  vector<HypothesisList *> v_slot_hypothesis_;
//  vector<vector<Tree2TreeHypothesis> *> v_slot_hypothesis_;
  vector<Tree2TreeHypothesis *> v_slot_hypothesis_pointer_;

 public:
  UnitRules * p_unit_rules_;
  MatchedPattern * p_matched_pattern_;

 public:
  SlotInformation() {
    slot_number_ = 0;
    p_unit_rules_ = NULL;
    p_matched_pattern_ = NULL;
  }
  ~SlotInformation() {}

 public:
  bool Init(int &slot_number);
};


//////////////////////////////////////////////////////////////////////////
class Tree2TreeCell;
class Tree2TreeModel;

class Tree2TreeHypothesis {
 public:
  Tree2TreeCell *        tree2tree_cell_;

 public:
  float                  model_score_;
  float                  model_score_except_lm_;
  float                  language_model_score_;

 public:
  string                 translation_result_;
  int                    translation_length_;

 public:
  int                    target_word_count_;
  vector<string>         v_target_word_;
  vector<int>            v_target_word_id_;
  vector<float>          v_target_n_gram_lm_score_;
  vector<float>          v_feature_values_;

 public:
  Tree2TreeHypothesis * left_partial_translation_;
  Tree2TreeHypothesis * right_partial_translation_;

 public:
  string root_;
  UnitRule * p_rule_used_;
  MatchedPattern * p_matched_pattern_;

 public:
  Tree2TreeHypothesis() {
    translation_length_ = 0;
    model_score_ = 0;
    model_score_except_lm_ = 0;
    language_model_score_ = 0;
    target_word_count_ = 0;
    left_partial_translation_ = NULL;
    right_partial_translation_ = NULL;
    p_rule_used_ = NULL;
    p_matched_pattern_ = NULL;
  }
  ~Tree2TreeHypothesis() {
    v_target_word_.clear();
    v_target_word_id_.clear();
    v_target_n_gram_lm_score_.clear();
    v_feature_values_.clear();

    tree2tree_cell_ = NULL;
    left_partial_translation_ = NULL;
    right_partial_translation_ = NULL;
    p_rule_used_ = NULL;
    p_matched_pattern_ = NULL;
  }

 public:
  bool Init(Tree2TreeConfiguration &tree2tree_config, Tree2TreeCell &tree2tree_cell);
  bool InitFeatureScores(UnitRule &unit_rule);
  bool Create(string &translation, Tree2TreeModel &tree2tree_model);
  bool ComputeFeatureScores(UnitRule &unit_rule);

 public:
  bool Copy(Tree2TreeHypothesis * p_tree2tree_hypothesis);

 public:
  bool operator ==(Tree2TreeHypothesis &tree2tree_hypothesis);
};


//////////////////////////////////////////////////////////////////////////
class HypothesisList {
 public:
  vector<Tree2TreeHypothesis> v_tree2tree_hypothesis_;

 public:
  HypothesisList() {}
  ~HypothesisList() {}
};


//////////////////////////////////////////////////////////////////////////
class ExploredNode {
 public:
  Tree2TreeHypothesis tree2tree_hypothesis_;

 public:
  vector<int> v_offsets_;
  int last_updated_;
  int last_updated_slot_;
  int rule_offset_;
  UnitRules * p_unit_rules_;

 public:
  ExploredNode() {
    last_updated_ = 0;
    last_updated_slot_ = 0;
    rule_offset_ = 0;
    p_unit_rules_ = NULL;
  }
  ~ExploredNode() {}
};


//////////////////////////////////////////////////////////////////////////
class Tree2TreeCell {
 public:
  int begin_;
  int end_;
  bool have_punct_;
  bool have_generalization_;
  bool have_forced_;          // forced decoding for this span

 public:
  unsigned int to_l_hypothesis_update_count_;
  unsigned int to_l_span_update_count_;
  int n_;                     // n in n-best

 public:
  list<Tree2TreeHypothesis> l_translation_;

 public:
  vector<MyTreeNode *> v_tree_nodes_;
  vector<Tree2TreeCell *> v_cell_nodes_;

 public:
  vector<MatchedPattern> v_matched_pattern_;

 public:
  vector<Tree2TreeHypothesis> v_hypotheses_;

 public:
//  vector<Tree2TreeHypothesis> v_nbest_results_;
  HypothesisList v_nbest_results_;

 public:
  map<string, HypothesisList> m_hypothesis_list_with_symbol_;

 public:
  Tree2TreeCell() {
    begin_ = 0;
    end_ = 0;
    to_l_hypothesis_update_count_ = 0;
    to_l_span_update_count_ = 0;
    n_ = 0;
    have_punct_ = false;
    have_generalization_ = false;
    have_forced_ = false;
    v_tree_nodes_.reserve(1000);
    v_cell_nodes_.reserve(1000);
    v_hypotheses_.reserve(50);
  }
  ~Tree2TreeCell() {}

 public:
  bool Init(int &begin, int &end, int &beam_size);
  bool InitSyntax(int &begin, int &end, int &beam_size);

 public:
  bool AddTree2TreeHypothesis(Tree2TreeHypothesis &tree2tree_hypothesis);
  bool AddMatchedPattern(MatchedPattern &matched_pattern);

 public:
  bool ReassignCellToHypothesisInBeam(Tree2TreeCell &tree2tree_cell);
};


//////////////////////////////////////////////////////////////////////////
class Tree2TreeCells {
 public:
  vector<vector<Tree2TreeCell> > cells_;

 public:
  Tree2TreeCells() {}
  ~Tree2TreeCells() {}
};


//////////////////////////////////////////////////////////////////////////
class Tree2TreeModel {
 public:
  LanguageModel ngram_language_model_;

 public:
  SynchronousGrammar scfg_model_;

 public:
  Tree2TreeModel() {}
  ~Tree2TreeModel() {}
 
 public:
  bool Init(Tree2TreeConfiguration &tree2tree_config);
};


//////////////////////////////////////////////////////////////////////////
class Tree2TreeTestSet : public BasicMethod {
 public:
  vector<string> source_sentences_set_;
  int source_sentences_number_;
 
 public:
  Tree2TreeTestSet() {
    source_sentences_number_ = 0;
    source_sentences_set_.reserve(3000);
  }
  ~Tree2TreeTestSet() {}

 public:
  bool ReadTestSentences(string &test_file_name);
};


//////////////////////////////////////////////////////////////////////////
/*
class BaseSearchNode {
 public:
  Tree2TreeCell * tree2tree_cell_;
};
*/

/*
enum HEAPTYPE {MAXHEAP, MINHEAP};

class NodeHeapForSearch {
 public:
  vector<Tree2TreeCell *> v_tree2tree_cell_;

 public:
  HEAPTYPE type_;
  int size_;
  int count_;

 public:
  NodeHeapForSearch() {
    type_ = MAXHEAP;
    size_ = 0;
    count_ = 0;
  }
  ~NodeHeapForSearch();
  
 public:
  bool Init(int &head_size, HEAPTYPE type);
  bool Clear();
//  void Update();
//  bool Compare(int i, int j);
//  void Push()
};
*/


//////////////////////////////////////////////////////////////////////////
class Tree2TreeBasedDecoder : public BasicMethod {
 public:
  Tree2TreeConfiguration * p_tree2tree_config_;
  Tree2TreeModel * p_tree2tree_model_;

 public:
  Tree2TreeCells tree2tree_cells_;

 public:
  DecodingSentence decoding_sentence_;

 public:
  string source_sentence_;
  vector<string> v_source_words_;
  vector<bool> v_source_words_information_;

 public:
//  Tree * source_tree_;
  MyTree my_source_tree_;

 public:
  vector<Tree2TreeCell> v_cell_nodes_;
  int cell_node_number_;

 public:
  string default_symbol_;
  Tree2TreeHypothesis default_hypothesis_;

 public:
  int max_num_of_symbol_with_same_trans_;

 public:
  string translation_buf_;
  vector<float> language_model_score_buf_;

 public:
//  vector<int> word_id_buf_;
  int * word_id_buf_;

 public:
  multimap<float, ExploredNode> hypothesis_for_search_;
  vector<vector<int> > explored_;

 public:
  bool decoding_failure_;

 public:
  Tree2TreeBasedDecoder() {
    cell_node_number_ = 0;
    default_symbol_ = "NP=NP";
    max_num_of_symbol_with_same_trans_ = MAX_HYPOTHESIS_NUM_IN_CELL;
//    word_id_buf_.reserve(1024 * 100);
    word_id_buf_ = new int[1024 * 100];

    translation_buf_.reserve(1024 * 100);
    language_model_score_buf_.resize(1024 * 100, 0.0f);
    decoding_failure_ = false;
  }
  ~Tree2TreeBasedDecoder() {
    delete [] word_id_buf_;
  }

 public:
  bool Init(Tree2TreeConfiguration &tree2tree_config, Tree2TreeModel &tree2tree_model);

 private:
  bool InitSourceTreeParameters();

 private:
  bool AllocateSpaceForCellsAndInit();

 public:
  bool Decode();

 private:
  bool TreeParsingDecoding();
  bool GenerateTranslation(Tree2TreeCell &tree2tree_cell);
  bool HaveGeneralization(int &begin, int &end, MatchedPattern &matched_pattern, SlotInformation &slot_information);
  bool IsGeneralization(string &source);
  bool ApplyMatchedPattern(Tree2TreeCell &tree2tree_cell, MatchedPattern &matched_pattern, SlotInformation &slot_information, bool &have_generalization);
  bool ApplyTranslationRule(Tree2TreeCell &tree2tree_cell, UnitRules * p_unit_rules, MatchedPattern &matched_pattern, SlotInformation &slot_information, bool &have_generalization);
  bool InitSlotInformation(MatchedPattern &matched_pattern, UnitRule * p_unit_rule, SlotInformation &slot_information);
  bool HeuristicSearch(Tree2TreeCell &tree2tree_cell, UnitRule * p_unit_rule, SlotInformation &slot_information, bool &have_generalization, bool &for_non_lexical);
//  bool GenerateHypothesisWithRule(Tree2TreeCell &tree2tree_cell, UnitRule * p_unit_rule, SlotInformation &slot_information, bool &have_generalization, ExploredNode &explored_node);
  bool GenerateHypothesisWithRule(Tree2TreeCell &tree2tree_cell, UnitRule * p_unit_rule, SlotInformation &slot_information, bool &have_generalization, Tree2TreeHypothesis &tree2tree_hypothesis);
  bool ReplaceGeneralizationTranslation(SlotInformation &slot_information, string &translation);
  bool GenerateTranslationWithRule(UnitRule * p_unit_rule, SlotInformation &slot_information, string &translation, int &translation_length);
  bool HaveValidNullTranslation(SlotInformation &slot_information);
  bool ComputeUnCompleteOverallScore(UnitRule * p_unit_rule, Tree2TreeConfiguration &tree2tree_config, float &overall_score);
  
 private:
  bool CreateTranslationUsingTreeSequence();

 private:
  bool GetBestSpanSequenceForTreeParsing(vector<Tree2TreeCell *> &v_tree2tree_cell);
  float GetBestSpanSequenceForTreeParsing(vector<Tree2TreeCell *> &v_tree2tree_cell, MyTreeNode * p_root);

 private:
  bool GlueMultipleNodeTranslation(Tree2TreeCell &tree2tree_cell, vector<Tree2TreeCell *> &v_nodes);
  bool GlueTwoCells(Tree2TreeCell &tree2tree_cell, Tree2TreeCell &sub_cell_1, Tree2TreeCell &sub_cell_2, SlotInformation &slot_information);


 private:
  bool EnQueue(Tree2TreeCell &tree2tree_cell, ExploredNode &explored_node, UnitRule * p_unit_rule, SlotInformation &slot_information, bool &have_generalization, bool &for_non_lex);

 private:
  bool SortWithNonterminalAndModelScore(vector<Tree2TreeHypothesis *> &v_p_nbest_result, int left, int right);
  int CompareWithNonterminalAndModelScore(Tree2TreeHypothesis &tree2tree_hypothesis_1, Tree2TreeHypothesis &tree2tree_hypothesis_2);


 private:
  bool ApplyUnaryRulesInBeam(Tree2TreeCell &tree2tree_cell);

 private:
  bool CompleteSpanForNonterminalSymbols(Tree2TreeCell &tree2tree_cell);


 private:
  UnitRules * GetUnitRulesForSlotInformation(SlotInformation &slot_information);

 private:
  bool CopyExploredNode(ExploredNode &explored_node, int &nonterminal_count, ExploredNode &new_explored_node);
  bool CheckExplored(ExploredNode &explored_node);

 private:
  bool IsAcceptable(Tree2TreeHypothesis &tree2tree_hypothesis);

 private:
  bool ClearExplored();
  bool SetExplored(ExploredNode &explored_node);

 private:
  bool CkyDecoding();

 private:
  bool LoadPrematchedRules();
  bool LoadViterbiRules();

 private:
  bool ParseSourceSentence();

 private:
  bool MatchScfgRules();
  bool MatchRulesForTreeParsing();
  bool MatchRulesForTreeParsing(Tree2TreeCell &tree2tree_cell);

 private:
  bool AddBasicRules();

 private:
  bool ProcessUnknownWord(Tree2TreeCell& tree2tree_cell, string &phrase);

 private:
  unsigned int CompleteWithBeamPruning(Tree2TreeCell& tree2tree_cell, bool &check_rule);

 private:
  bool CheckHypothesisAvailability(Tree2TreeHypothesis * p_tree2tree_hypothesis);
  bool CheckAvailability(string &source);

 private:
  bool AddMatchedPatternUsingTreeFragment(Tree2TreeCell &tree2tree_cell, MyTreeFragment &tree_fragment);

 private:
  bool AddCellTranslation(Tree2TreeCell &tree2tree_cell, UnitRule &unit_rule, bool &output_oov, string &label);

 private:
  bool ComputeModelScore(Tree2TreeHypothesis &tree2tree_hypothesis);

 private:
  bool GetDefaultRootLabel(Tree2TreeCell &tree2tree_cell, string &target_symbol, string &label);
  bool GetDefaultRootLabel(MyTreeNode * p_tree_node, string &target_symbol, string &label);

 private:
  bool IsSentenceEnd(Tree2TreeCell &tree2tree_cell);

 private:
  bool SetTreeNodeNoRecursion(MyTreeNode *root);

 private:
  bool SetUserTranslateForTreeParsing();

 public:
  static bool PrintTree2TreeBasedDecoderLogo();
};


bool CompareUnitRule(const UnitRule &unit_rule_1, const UnitRule &unit_rule_2);
bool CompareTranslationList(const Tree2TreeHypothesis &tree2tree_hypothesis_1, const Tree2TreeHypothesis &tree2tree_hypothesis_2);


}

#endif

