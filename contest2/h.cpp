#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

class SuffixAutomaton {
 public:
  SuffixAutomaton(const std::string& first_string,
                  const std::string& second_string) {
    size = 1;
    last_state = 0;
    automaton.resize(2 * (first_string.length() + second_string.length() + 1) +
                     1);

    for (char character : first_string) {
      extend(character, 1);
    }

    extend('#', 0);

    for (char character : second_string) {
      extend(character, 2);
    }

    propagateMasks();
  }

  std::string findKthSubstring(uint64_t k) {
    uint64_t total_paths = dfs(0);

    if (total_paths < k) {
      return "-1";
    }

    std::string result = "";
    int current_state = 0;

    while (k > 0) {
      if (current_state != 0) {
        k--;
        if (k == 0) break;
      }

      for (int i = 0; i < ALPHABET_SIZE; i++) {
        int next_node = automaton[current_state].next[i];
        if (next_node != -1 && automaton[next_node].mask == 3) {
          if (k <= automaton[next_node].paths) {
            result += static_cast<char>('a' + i);
            current_state = next_node;
            break;
          }
          k -= automaton[next_node].paths;
        }
      }
    }

    return result;
  }

 private:
  static constexpr int ALPHABET_SIZE = 26;
  static constexpr int TOTAL_CHARS = ALPHABET_SIZE + 1;

  struct State {
    int length = 0;
    int link = -1;
    int next[TOTAL_CHARS];
    int mask = 0;
    uint64_t paths = 0;
    bool visited = false;

    State() {
      for (int i = 0; i < TOTAL_CHARS; i++) {
        next[i] = -1;
      }
    }
  };

  int getCharIndex(char character) {
    if (character == '#') return ALPHABET_SIZE;
    return character - 'a';
  }

  void extend(char character, int mask_val) {
    int char_index = getCharIndex(character);
    int current = size++;
    automaton[current].length = automaton[last_state].length + 1;
    automaton[current].mask = mask_val;

    int previous_state = last_state;
    while (previous_state != -1 &&
           automaton[previous_state].next[char_index] == -1) {
      automaton[previous_state].next[char_index] = current;
      previous_state = automaton[previous_state].link;
    }

    if (previous_state == -1) {
      automaton[current].link = 0;
      last_state = current;
      return;
    }

    int next_state = automaton[previous_state].next[char_index];
    if (automaton[previous_state].length + 1 == automaton[next_state].length) {
      automaton[current].link = next_state;
      last_state = current;
      return;
    }

    int clone = size++;
    automaton[clone].length = automaton[previous_state].length + 1;
    for (int i = 0; i < TOTAL_CHARS; i++) {
      automaton[clone].next[i] = automaton[next_state].next[i];
    }
    automaton[clone].link = automaton[next_state].link;
    automaton[clone].mask = 0;

    while (previous_state != -1 &&
           automaton[previous_state].next[char_index] == next_state) {
      automaton[previous_state].next[char_index] = clone;
      previous_state = automaton[previous_state].link;
    }

    automaton[next_state].link = clone;
    automaton[current].link = clone;

    last_state = current;
  }

  void propagateMasks() {
    std::vector<std::pair<int, int>> order;
    for (int i = 1; i < size; i++) {
      order.push_back({automaton[i].length, i});
    }

    std::sort(order.rbegin(), order.rend());

    for (const auto& item : order) {
      int current_state = item.second;
      int link = automaton[current_state].link;
      if (link != -1) {
        automaton[link].mask |= automaton[current_state].mask;
      }
    }
  }

  uint64_t dfs(int current_state) {
    if (automaton[current_state].visited) {
      return automaton[current_state].paths;
    }
    automaton[current_state].visited = true;

    uint64_t current_paths = (current_state == 0) ? 0 : 1;

    for (int i = 0; i < ALPHABET_SIZE; i++) {
      int next_state = automaton[current_state].next[i];
      if (next_state != -1 && automaton[next_state].mask == 3) {
        current_paths += dfs(next_state);
      }
    }

    automaton[current_state].paths = current_paths;
    return current_paths;
  }

  std::vector<State> automaton;
  int size;
  int last_state;
};

int main() {
  std::string first_string, second_string;
  uint64_t k;

  std::cin >> first_string >> second_string >> k;

  SuffixAutomaton automaton(first_string, second_string);

  std::cout << automaton.findKthSubstring(k) << std::endl;

  return 0;
}
