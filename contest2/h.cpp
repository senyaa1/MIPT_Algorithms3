#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

class SuffixAutomaton
{
      public:
	SuffixAutomaton(const std::string &s, const std::string &t)
	{
		sz = 1;
		last = 0;
		automaton.resize(2 * (s.length() + t.length() + 1) + 1);

		for (char c : s)
		{
			extend(c, 1);
		}

		extend('#', 0);

		for (char c : t)
		{
			extend(c, 2);
		}

		propagateMasks();
	}

	std::string findKthSubstring(uint64_t k)
	{
		uint64_t totalPaths = dfs(0);

		if (totalPaths < k)
		{
			return "-1";
		}

		std::string result = "";
		int v = 0;

		while (k > 0)
		{
			if (v != 0)
			{
				k--;
				if (k == 0)
					break;
			}

			for (int i = 0; i < ALPHABET_SIZE; i++)
			{
				int next_node = automaton[v].next[i];
				if (next_node != -1 && automaton[next_node].mask == 3)
				{
					if (k <= automaton[next_node].paths)
					{
						result += (char)('a' + i);
						v = next_node;
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

	struct State
	{
		int length = 0;
		int link = -1;
		int next[TOTAL_CHARS];
		int mask = 0;
		uint64_t paths = 0;
		bool visited = false;

		State()
		{
			for (int i = 0; i < TOTAL_CHARS; i++)
			{
				next[i] = -1;
			}
		}
	};

	int getCharIndex(char c)
	{
		if (c == '#')
			return ALPHABET_SIZE;
		return c - 'a';
	}

	void extend(char ch, int mask_val)
	{
		int c = getCharIndex(ch);
		int current = sz++;
		automaton[current].length = automaton[last].length + 1;
		automaton[current].mask = mask_val;

		int p = last;
		while (p != -1 && automaton[p].next[c] == -1)
		{
			automaton[p].next[c] = current;
			p = automaton[p].link;
		}

		if (p == -1)
		{
			automaton[current].link = 0;
			last = current;
			return;
		}

		int q = automaton[p].next[c];
		if (automaton[p].length + 1 == automaton[q].length)
		{
			automaton[current].link = q;
			last = current;
			return;
		}

		int clone = sz++;
		automaton[clone].length = automaton[p].length + 1;
		for (int i = 0; i < TOTAL_CHARS; i++)
		{
			automaton[clone].next[i] = automaton[q].next[i];
		}
		automaton[clone].link = automaton[q].link;
		automaton[clone].mask = 0;

		while (p != -1 && automaton[p].next[c] == q)
		{
			automaton[p].next[c] = clone;
			p = automaton[p].link;
		}

		automaton[q].link = clone;
		automaton[current].link = clone;

		last = current;
	}

	void propagateMasks()
	{
		std::vector<std::pair<int, int>> order;
		for (int i = 1; i < sz; i++)
		{
			order.push_back({automaton[i].length, i});
		}
		std::sort(order.rbegin(), order.rend());

		for (const auto &item : order)
		{
			int u = item.second;
			int link = automaton[u].link;
			if (link != -1)
			{
				automaton[link].mask |= automaton[u].mask;
			}
		}
	}

	uint64_t dfs(int u)
	{
		if (automaton[u].visited)
		{
			return automaton[u].paths;
		}
		automaton[u].visited = true;

		uint64_t currentPaths = (u == 0) ? 0 : 1;

		for (int i = 0; i < ALPHABET_SIZE; i++)
		{
			int v = automaton[u].next[i];
			if (v != -1 && automaton[v].mask == 3)
			{
				currentPaths += dfs(v);
			}
		}

		automaton[u].paths = currentPaths;
		return currentPaths;
	}

	std::vector<State> automaton;
	int sz;
	int last;
};

int main()
{
	std::string s, t;
	uint64_t k;

	std::cin >> s >> t >> k;

	SuffixAutomaton sam(s, t);

	std::cout << sam.findKthSubstring(k) << std::endl;

	return 0;
}
