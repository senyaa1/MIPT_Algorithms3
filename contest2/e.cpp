#include <algorithm>
#include <cstddef>
#include <iostream>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace {
constexpr auto k_no_index = std::numeric_limits<std::size_t>::max();
}

struct Node {
  std::size_t parent_index = k_no_index;
  std::size_t depth = 0;
  std::size_t suffix_index = k_no_index;
  std::vector<std::size_t> children;
};


class SuffixArray {
 public:
  explicit SuffixArray(const std::string& combined_string)
      : text_(combined_string),
        total_length_(combined_string.length()),
        suffix_array_(total_length_, 0),
        rank_(total_length_, 0),
        lcp_(total_length_ > 0 ? total_length_ - 1 : 0, 0) {
    if (total_length_ > 0) {
      build_suffix_array();
      build_lcp();
    }
  }

  const std::vector<std::size_t>& get_suffix_array() const {
    return suffix_array_;
  }

  const std::vector<std::size_t>& get_lcp() const { return lcp_; }

 private:
  void build_suffix_array() {
    std::vector<int> equivalence_class(total_length_, 0);
    for (std::size_t i = 0; i < total_length_; ++i) {
      equivalence_class[i] = text_[i];
      suffix_array_[i] = i;
    }

    for (std::size_t offset = 1; offset < total_length_; offset *= 2) {
      std::vector<std::pair<std::pair<int, int>, std::size_t>> suffix_pairs(
          total_length_);
      for (std::size_t i = 0; i < total_length_; ++i) {
        const auto next_class =
            (i + offset < total_length_) ? equivalence_class[i + offset] : -1;
        suffix_pairs[i] = {{equivalence_class[i], next_class}, i};
      }

      std::sort(suffix_pairs.begin(), suffix_pairs.end());
      for (std::size_t i = 0; i < total_length_; ++i) {
        suffix_array_[i] = suffix_pairs[i].second;
      }

      equivalence_class[suffix_array_[0]] = 0;

      for (std::size_t i = 1; i < total_length_; ++i) {
        const auto has_new_class =
            (suffix_pairs[i].first != suffix_pairs[i - 1].first) ? 1 : 0;
        equivalence_class[suffix_array_[i]] =
            equivalence_class[suffix_array_[i - 1]] + has_new_class;
      }

      if (std::size_t(equivalence_class[suffix_array_[total_length_ - 1]]) ==
          equivalence_class.size() - 1) {
        break;
      }
    }

    for (std::size_t i = 0; i < total_length_; ++i) {
      rank_[suffix_array_[i]] = i;
    }
  }

  void build_lcp() {
    std::size_t current_lcp = 0;
    for (std::size_t i = 0; i < total_length_; ++i) {
      if (rank_[i] == total_length_ - 1) {
        current_lcp = 0;
        continue;
      }
      const auto next_suffix_index = suffix_array_[rank_[i] + 1];
      while (i + current_lcp < total_length_ &&
             next_suffix_index + current_lcp < total_length_ &&
             text_[i + current_lcp] == text_[next_suffix_index + current_lcp]) {
        ++current_lcp;
      }
      lcp_[rank_[i]] = current_lcp;
      if (current_lcp > 0) {
        --current_lcp;
      }
    }
  }

  std::string text_;
  std::size_t total_length_;
  std::vector<std::size_t> suffix_array_;
  std::vector<std::size_t> rank_;
  std::vector<std::size_t> lcp_;
};

class SuffixTree {
 public:
  SuffixTree(const std::string& string_s, const std::string& string_t)
      : length_s_(string_s.length()),
        total_length_(string_s.length() + string_t.length()) {}

  void build(const std::vector<std::size_t>& suffix_array,
             const std::vector<std::size_t>& lcp) {
    tree_.clear();
    tree_.push_back(Node());
    std::vector<std::size_t> active_path_stack;
    active_path_stack.push_back(0);

    for (std::size_t i = 0; i < total_length_; ++i) {
      const auto suffix_start = suffix_array[i];
      const auto max_suffix_length = (suffix_start < length_s_)
                                         ? length_s_ - suffix_start
                                         : total_length_ - suffix_start;
      auto lcp_value = (i == 0) ? 0UL : lcp[i - 1];
      if (lcp_value > max_suffix_length) {
        lcp_value = max_suffix_length;
      }

      auto current_node = active_path_stack.back();
      auto previous_node = k_no_index;
      while (tree_[current_node].depth > lcp_value) {
        previous_node = current_node;
        active_path_stack.pop_back();
        current_node = active_path_stack.back();
      }

      if (tree_[current_node].depth < lcp_value) {
        const auto split_node = tree_.size();
        tree_.push_back(Node());
        tree_[split_node].parent_index = current_node;
        tree_[split_node].depth = lcp_value;
        tree_[split_node].suffix_index = tree_[previous_node].suffix_index;

        for (auto& child : tree_[current_node].children) {
          if (child == previous_node) {
            child = split_node;
            break;
          }
        }

        tree_[previous_node].parent_index = split_node;
        tree_[split_node].children.push_back(previous_node);

        active_path_stack.push_back(split_node);
        current_node = split_node;
      }

      if (max_suffix_length > tree_[current_node].depth) {
        const auto leaf_node = tree_.size();
        tree_.push_back(Node());
        tree_[leaf_node].parent_index = current_node;
        tree_[leaf_node].depth = max_suffix_length;
        tree_[leaf_node].suffix_index = suffix_start;
        tree_[current_node].children.push_back(leaf_node);
        active_path_stack.push_back(leaf_node);
      }
    }
  }

  void print_dfs() const {
    if (tree_.empty()) return;

    std::vector<std::size_t> dfs_order;
    std::vector<std::size_t> new_id(tree_.size(), 0);
    std::vector<std::size_t> dfs_stack;
    dfs_stack.push_back(0);

    while (!dfs_stack.empty()) {
      const auto node = dfs_stack.back();
      dfs_stack.pop_back();
      new_id[node] = dfs_order.size();
      dfs_order.push_back(node);
      for (auto it = tree_[node].children.rbegin();
           it != tree_[node].children.rend(); ++it) {
        dfs_stack.push_back(*it);
      }
    }

    std::cout << tree_.size() << "\n";
    for (std::size_t i = 1; i < tree_.size(); ++i) {
      const auto node = dfs_order[i];
      const auto parent = tree_[node].parent_index;
      const auto parent_new_id = new_id[parent];

      const auto start_position =
          tree_[node].suffix_index + tree_[parent].depth;
      const auto end_position = tree_[node].suffix_index + tree_[node].depth;

      std::size_t string_indicator = 0;
      std::size_t left_bound = 0;
      std::size_t right_bound = 0;

      if (tree_[node].suffix_index < length_s_) {
        string_indicator = 0;
        left_bound = start_position;
        right_bound = end_position;
      } else {
        string_indicator = 1;
        left_bound = start_position - length_s_;
        right_bound = end_position - length_s_;
      }

      std::cout << parent_new_id << " " << string_indicator << " " << left_bound
                << " " << right_bound << "\n";
    }
  }

 private:
  std::size_t length_s_;
  std::size_t total_length_;
  std::vector<Node> tree_;
};

int main() {
  std::string string_s;
  std::string string_t;
  std::cin >> string_s >> string_t;

  const SuffixArray suffix_array_builder(string_s + string_t);

  SuffixTree suffix_tree_builder(string_s, string_t);
  suffix_tree_builder.build(suffix_array_builder.get_suffix_array(),
                            suffix_array_builder.get_lcp());

  suffix_tree_builder.print_dfs();

  return 0;
}
