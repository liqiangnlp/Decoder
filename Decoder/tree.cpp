/*
 * $Id:
 * 0001
 *
 * $File:
 * OurTree.cpp
 *
 * $Proj:
 * Tree Structure
 *
 * $Func:
 * source file of tree function
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

#include "tree.h"

namespace decoder_tree {

bool MyTreeFragment::AddNonTerminal(MyTreeNode * non_terminal, bool from_left) {
  if ("" == frontier_sequence_) {
    frontier_sequence_ = "#" + non_terminal->label_;
  } else {
    if (from_left) {
      frontier_sequence_ = "#" + non_terminal->label_ + " " + frontier_sequence_;
    } else {
      frontier_sequence_ = frontier_sequence_ + " #" + non_terminal->label_;
    }
  }

  if (from_left) {
    l_frontier_non_terminals_.push_front(non_terminal);
  } else {
    l_frontier_non_terminals_.push_back(non_terminal);
  }
  ++non_terminal_number_;
  return true;
}


bool MyTreeFragment::AddWord(MyTreeNode * non_terminal, bool from_left) {
  if ("" == frontier_sequence_) {
    frontier_sequence_ = non_terminal->word_;
  } else {
    if (from_left) {
      frontier_sequence_ = non_terminal->word_ + " " +frontier_sequence_;
    } else {
      frontier_sequence_ = frontier_sequence_ + " " + non_terminal->word_;
    }
  }
  ++word_number_;
  return true;
}


bool MyTreeFragment::Join(MyTreeFragment * fragment_1, MyTreeFragment * fragment_2) {
  frontier_sequence_ = fragment_1->frontier_sequence_ + " " + fragment_2->frontier_sequence_;
  for (list<MyTreeNode *>::iterator iter = fragment_1->l_frontier_non_terminals_.begin(); iter != fragment_1->l_frontier_non_terminals_.end(); ++iter) {
    l_frontier_non_terminals_.push_back(*iter);
  }
  for (list<MyTreeNode *>::iterator iter = fragment_2->l_frontier_non_terminals_.begin(); iter != fragment_2->l_frontier_non_terminals_.end(); ++iter) {
    l_frontier_non_terminals_.push_back(*iter);
  }
  word_number_ = fragment_1->word_number_ + fragment_2->word_number_;
  non_terminal_number_ = fragment_1->non_terminal_number_ + fragment_2->non_terminal_number_;
  depth_ = fragment_1->depth_ + fragment_2->depth_;
  return true;
}





//////////////////////////////////////////////////////////////////////////
bool MyTree::CreateTree(string &tree_string) {
  vector<MyTreeStringToken> v_tokens;
  TokenizeString(tree_string, v_tokens);
  int position = 1;
  if (0 == v_tokens.size()) {
    v_tokens.clear();
    return false;
  }
  MyTreeStringToken *first_token = &v_tokens.at(0);
  MyTreeStringToken *last_token = &v_tokens.at(v_tokens.size() - 1);
  if ("(" != first_token->token_ || ")" != last_token->token_) {
    v_tokens.clear();
    return false;
  }

//  v_leaves_.push_back("");
  MyTreeNode tmp_my_tree_node;
  v_leaves_.push_back(tmp_my_tree_node);

  root_ = BuildSubTree(v_tokens, position);
  if (position != v_tokens.size() - 1) {
    root_ = NULL;
    return false;
  }
  return true;
}

bool MyTree::TokenizeString(string &tree_string, vector<MyTreeStringToken> &v_tokens) {
  string left_bracket = "(";
  string right_bracket = ")";
  MyTreeStringToken token;
  string token_string;
  token_string.reserve(1000);
  int position = 0;
  while (position < tree_string.size()) {
    char current_char = tree_string.at(position);
    if ('(' == current_char) {
      bool special_flag = false;
      int tmp_position = position + 1;
      while (tmp_position < tree_string.size()) {
        if (')' == tree_string.at(tmp_position)) {
          special_flag = true;
          break;
        } else if (' ' == tree_string.at(tmp_position)) {
          break;
        }
        ++tmp_position;
      }
      if (special_flag && tmp_position > position + 1) {
        token_string = tree_string.substr(position, tmp_position - position + 1);
        GenerateToken(token_string, (int)v_tokens.size(), token);
        v_tokens.push_back(token);
        token_string = "";
        position = tmp_position;
      } else {
        if ("" != token_string) {
          GenerateToken(token_string, (int)v_tokens.size(), token);
          v_tokens.push_back(token);
        }
        token_string = "";
        GenerateToken(left_bracket, (int)v_tokens.size(), token);
        v_tokens.push_back(token);
      }
    } else if (')' == current_char) {
      if ("" != token_string) {
        GenerateToken(token_string, (int)v_tokens.size(), token);
        v_tokens.push_back(token);
      }
      token_string = "";
      GenerateToken(right_bracket, (int)v_tokens.size(), token);
      v_tokens.push_back(token);
    } else if (' ' == current_char) {
      if ("" != token_string) {
        GenerateToken(token_string, (int)v_tokens.size(), token);
        v_tokens.push_back(token);
      }
      token_string = "";
    } else {
      token_string.push_back(current_char);
    }
    ++position;
  }
  return true;
}


bool MyTree::GenerateToken(string &token_string, int id, MyTreeStringToken &my_tree_string_token) {
  my_tree_string_token.token_ = token_string;
  my_tree_string_token.id_ = id;
  return true;
}


MyTreeNode * MyTree::BuildSubTree(vector<MyTreeStringToken> &v_tokens, int &position) {
  MyTreeStringToken * token = &v_tokens.at(position);
  if (position >= v_tokens.size()) {
    return NULL;
  } 
  if ("(" != token->token_) {
    return NULL;
  }
  ++position;
  token = &v_tokens.at(position);

//  root.Init(token->token_);
  MyTreeNode * current_tree_node = CreateTreeNode(token->token_, (int)v_node_base_.size());

  ++position;
  token = &v_tokens.at(position);

  bool is_leaf_node = false;
  if ((position < v_tokens.size()) && ("(" != token->token_)) {
    is_leaf_node = true;
  }
  if (position < v_tokens.size() - 1) {
    MyTreeStringToken * next_token = &v_tokens.at(position + 1);
    if (")" == next_token->token_) {
      is_leaf_node = true;
    }
  }

  if (is_leaf_node) {               // leaf node
    token = &v_tokens.at(position);
    current_tree_node->word_ = token->token_;
    current_tree_node->is_leaf_ = true;
    current_tree_node->begin_ = (int)v_leaves_.size();
    current_tree_node->end_ = current_tree_node->begin_;
    v_leaves_.push_back(*current_tree_node);
    ++position;
  } else {
    int begin = 256;
    int end = -1;
    MyTreeEdge * edge = CreateTreeEdge(current_tree_node, 3);
    while (position < v_tokens.size()) {
      token = &v_tokens.at(position);
      if (")" == token->token_) {
        break;
      }
      MyTreeNode * child_node = NULL;
      if ("(" == token->token_) {
        child_node = BuildSubTree(v_tokens, position);
      }
      if (NULL == child_node) {
        return NULL;
      }
      if (begin > child_node->begin_) {
        begin = child_node->begin_;
      }
      if (end < child_node->end_) {
        end = child_node->end_;
      }
      edge->v_children_.push_back(child_node);
    }
    current_tree_node->is_leaf_ = false;
    current_tree_node->v_edges_.push_back(edge);
    current_tree_node->begin_ = begin;
    current_tree_node->end_ = end;
  }

  if (position >= v_tokens.size()) {
    return NULL;
  }
  ++position;
  return current_tree_node;
}


MyTreeNode * MyTree::CreateTreeNode(string &label, int id) {
  MyTreeNode node;
  node.is_leaf_ = false;
  node.begin_ = 256;
  node.end_ = -1;
  if ("" != label) {
    node.label_ = label;
  }
  node.id_ = id;
  v_node_base_.push_back(node);
  return &v_node_base_.at(v_node_base_.size() - 1);
}


MyTreeEdge * MyTree::CreateTreeEdge(MyTreeNode * parent, int children_number) {
  MyTreeEdge edge;
  edge.parent_ = parent;
  if (children_number >= 0) {
//    edge.children_.resize(children_number);
  } else {
//    edge.children_.resize(3);
  }
  v_edge_base_.push_back(edge);
  return &v_edge_base_.at(v_edge_base_.size() - 1);
}


bool MyTree::GenerateTreeFragments(bool &generate_tree_structure_flag) {
  for (int i = (int)v_node_base_.size() - 1; i >= 0; --i) {
    MyTreeNode * node = &v_node_base_.at(i);
    GenerateTreeFragments(node, generate_tree_structure_flag);
  }
  return true;
}


bool MyTree::GenerateTreeFragments(MyTreeNode * root, bool &generate_tree_structure_flag) {
  int key_length = 0;
  root->v_tree_fragments_.reserve(2);

  // root
  MyTreeFragment base_tree_fragment(root);
  base_tree_fragment.AddNonTerminal(root, true);

  if (generate_tree_structure_flag) {
    ;
  }
  root->v_tree_fragments_.push_back(base_tree_fragment);
  if (root->is_leaf_) {
    // word
    MyTreeFragment leaf_tree_fragment(root);
    leaf_tree_fragment.AddWord(root, true);
    if (generate_tree_structure_flag) {
      ;
    }
    root->v_tree_fragments_.push_back(leaf_tree_fragment);
  } else {
    // build it recursively
    for (int e = 0; e < root->v_edges_.size(); ++e) {
      MyTreeEdge * edge = root->v_edges_.at(e);

      vector<MyTreeFragment> v_fragments;

      int max_fragment_number = max_fragment_number_per_node_ / (int)root->v_edges_.size() + 1;
      for (int i = 0; i < edge->v_children_.size(); ++i) {
        // loop for each child node (tail of edge)
        MyTreeNode * child = edge->v_children_.at(i);
        vector<MyTreeFragment> *child_fragments = &child->v_tree_fragments_;
        if (0 == i) {
          v_fragments.reserve(child_fragments->size());
          for (int j = 0; j < child_fragments->size(); ++j) {
            MyTreeFragment * frag = &child_fragments->at(j);
            MyTreeFragment new_frag(frag);
            new_frag.root_ = root;
            v_fragments.push_back(new_frag);
          }
        } else {
          vector<MyTreeFragment> v_new_fragments;
          v_new_fragments.reserve(v_fragments.size());
          for (int j = 0; j < child_fragments->size(); ++j) {
            MyTreeFragment * child_fragment = &child_fragments->at(j);
            for (int k = 0; k < v_fragments.size(); ++k) {
              MyTreeFragment * current_fragment = &v_fragments.at(k);

              if (!IsValidTreeJoin(current_fragment, child_fragment)) {
                continue;
              }

              MyTreeFragment new_fragment;
              new_fragment.Join(current_fragment, child_fragment);
              new_fragment.root_ = root;
              v_new_fragments.push_back(new_fragment);
            }
            if (v_new_fragments.size() >= max_fragment_number) {
              break;
            }
          }
          if (0 != v_fragments.size()) {
            v_fragments.clear();
          }

          v_fragments.reserve(v_new_fragments.size());
          v_fragments.insert(v_fragments.begin(), v_new_fragments.begin(), v_new_fragments.end());
        }
      }
      if (generate_tree_structure_flag) {
        ;
      }
      root->v_tree_fragments_.reserve(root->v_tree_fragments_.size() + v_fragments.size());
      root->v_tree_fragments_.insert(root->v_tree_fragments_.end(), v_fragments.begin(), v_fragments.end());
    }
  }
  return true;
}


bool MyTree::IsValidTreeJoin(MyTreeFragment * fragment_1, MyTreeFragment * fragment_2) {
  if (fragment_1->word_number_ + fragment_2->word_number_ > max_word_number_) {
    return false;
  }
  if (fragment_1->non_terminal_number_ + fragment_2->non_terminal_number_ > max_nontermianl_number_) {
    return false;
  }
  if (fragment_1->depth_ > max_depth_ || fragment_2->depth_ > max_depth_) {
    return false;
  }
  return true;
}


}


