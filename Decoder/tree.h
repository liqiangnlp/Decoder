/*
 * $Id:
 * 0001
 *
 * $File:
 * OurTree.h
 *
 * $Proj:
 * Tree Structure
 *
 * $Func:
 * header file of tree function
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
 * 2014-10-29,12:09
 */

#ifndef DECODER_TREE_H_
#define DECODER_TREE_H_

#include <string>
#include <vector>
#include <list>
#include <iostream>
using namespace std;

namespace decoder_tree {

#define MAX_TREE_NODE_NUMBER 100000
#define MAX_TREE_EDGE_NUMBER 100000
#define MAX_WORD_NUMBER_IN_TREE 256

#define MILLION 1000000

struct MyTreeEdge;
class MyTreeFragment;

typedef struct MyTreeNode {
  string label_;
  string word_;
  bool is_leaf_;
  vector<MyTreeEdge *> v_edges_;
  int id_;                              // node id
  int begin_;                           // beginning of corresponding span
  int end_;                             // end of corresponding span
  vector<MyTreeFragment> v_tree_fragments_;     // tree fragments rooting at the node
}* PMyTreeNode;


class MyTreeFragment {
 public:
  MyTreeNode * root_;
  string frontier_sequence_;                        // sequence of terminals and frontier non-terminals
  list<MyTreeNode *> l_frontier_non_terminals_;     // frontier non-terminals

  short word_number_;
  short non_terminal_number_;
  short depth_;

 public:
  MyTreeFragment() {
    word_number_ = 0;
    non_terminal_number_ = 0;
    depth_ = 0;
    frontier_sequence_.reserve(1000);
  }
  MyTreeFragment(MyTreeNode * root) {
    root_ = root;
    word_number_ = 0;
    non_terminal_number_ = 0;
    depth_ = 0;
    frontier_sequence_.reserve(1000);
  }
  MyTreeFragment(MyTreeFragment * tree_fragment) {
    root_ = tree_fragment->root_;
    frontier_sequence_ = tree_fragment->frontier_sequence_;
    l_frontier_non_terminals_ = tree_fragment->l_frontier_non_terminals_;
    word_number_ = tree_fragment->word_number_;
    non_terminal_number_ = tree_fragment->non_terminal_number_;
    depth_ = tree_fragment->depth_;
  }
  ~MyTreeFragment() {}

 public:
  bool AddNonTerminal(MyTreeNode * non_terminal, bool from_left);
  bool AddWord(MyTreeNode * non_terminal, bool from_left);

 public:
  bool Join(MyTreeFragment * fragment_1, MyTreeFragment * fragment_2);

};


typedef struct MyTreeEdge {
  MyTreeNode * parent_;               // parent node (head)
  vector<MyTreeNode *> v_children_;     // child nodes (tails)
}* PMyTreeEdge;


typedef struct MyTreeStringToken {
  string token_;
  int id_;
}* PMyTreeStringToken;


class MyTree {
 public:
  vector<MyTreeNode> v_node_base_;
  vector<MyTreeEdge> v_edge_base_;

 public:
  MyTreeNode * root_;
  vector<MyTreeNode> v_leaves_;

 public:
  short max_word_number_;
  short max_nontermianl_number_;
  short max_depth_;
  int max_edge_number_per_node_;
  int max_fragment_number_per_node_;

 public:
  MyTree() {
    v_node_base_.reserve(MAX_TREE_NODE_NUMBER);
    v_edge_base_.reserve(MAX_TREE_EDGE_NUMBER);
    v_leaves_.reserve(MAX_WORD_NUMBER_IN_TREE);

    max_word_number_ = 4;
    max_nontermianl_number_ = 5;
    max_depth_ = 3;
    max_edge_number_per_node_ = MILLION;
    max_fragment_number_per_node_ = MILLION;
  }
  ~MyTree() {}

 public:
  bool CreateTree(string &tree_string);

 private:
  bool TokenizeString(string &tree_string, vector<MyTreeStringToken> &v_tokens);
  bool GenerateToken(string &token_string, int id, MyTreeStringToken &my_tree_string_token);

 private:
  MyTreeNode * BuildSubTree(vector<MyTreeStringToken> &v_tokens, int &position);

 private:
  MyTreeNode * CreateTreeNode(string &label, int id);

 private:
  MyTreeEdge * CreateTreeEdge(MyTreeNode * parent, int children_number);

 public:
  bool GenerateTreeFragments(bool &generate_tree_structure_flag);

 private:
  bool GenerateTreeFragments(MyTreeNode * root, bool &generate_tree_structure_flag);

 private:
  bool IsValidTreeJoin(MyTreeFragment * fragment_1, MyTreeFragment * fragment_2);

};



}

#endif
