#include <algorithm>
#include <cstdint>
#include <iostream>
#include <set>
#include <string>
#include <vector>

struct State
{
	int32_t length = 0;
	int32_t link = -1;
	int32_t next[26] = {-1};
	std::set<int32_t> endpos;

	State()
	{
		std::fill(std::begin(next), std::end(next), -1);
	}
};

struct SuffixAutomatonData
{
	std::vector<State> automaton;
	int32_t size = 1;
	int32_t last_state = 0;
	std::vector<std::vector<int32_t>> link_tree;
	std::vector<int32_t> difference_array;
};

void ExtendAutomaton(SuffixAutomatonData &data, char ch, int32_t pos) 
{
	auto c = static_cast<int32_t>(ch - 'a');
	auto current = data.size++;
	data.automaton[current].length = data.automaton[data.last_state].length + 1;
	data.automaton[current].endpos.insert(pos);

	auto p = data.last_state;
	while (p != -1 && data.automaton[p].next[c] == -1)
	{
		data.automaton[p].next[c] = current;
		p = data.automaton[p].link;
	}

	if (p == -1)
	{
		data.automaton[current].link = 0;
	}
	else
	{
		auto q = data.automaton[p].next[c];
		if (data.automaton[p].length + 1 == data.automaton[q].length)
		{
			data.automaton[current].link = q;
		}
		else
		{
			auto clone = data.size++;
			data.automaton[clone].length = data.automaton[p].length + 1;
			for (auto i = 0; i < 26; ++i)
			{
				data.automaton[clone].next[i] = data.automaton[q].next[i];
			}
			data.automaton[clone].link = data.automaton[q].link;

			while (p != -1 && data.automaton[p].next[c] == q)
			{
				data.automaton[p].next[c] = clone;
				p = data.automaton[p].link;
			}

			data.automaton[q].link = clone;
			data.automaton[current].link = clone;
		}
	}
	data.last_state = current;
}

void ProcessStateEndpos(SuffixAutomatonData &data, int32_t u)
{
	for (auto v : data.link_tree[u])
	{
		ProcessStateEndpos(data, v);

		if (data.automaton[v].endpos.size() > data.automaton[u].endpos.size())
		{
			std::swap(data.automaton[v].endpos, data.automaton[u].endpos);
		}
		for (auto pos : data.automaton[v].endpos)
		{
			data.automaton[u].endpos.insert(pos);
		}
	}

	if (u == 0 || data.automaton[u].endpos.size() < 3)
	{
		return;
	}

	auto min_len = data.automaton[data.automaton[u].link].length + 1;
	auto max_len = data.automaton[u].length;

	auto first_pos = *data.automaton[u].endpos.begin();
	auto last_pos = *data.automaton[u].endpos.rbegin();

	auto left = min_len;
	auto right = max_len;
	auto best_length = -1;

	while (left <= right)
	{
		auto mid = left + (right - left) / 2;

		auto it2 = data.automaton[u].endpos.lower_bound(first_pos + mid);

		if (it2 != data.automaton[u].endpos.end() && *it2 + mid <= last_pos)
		{
			best_length = mid;
			left = mid + 1;
		}
		else
		{
			right = mid - 1;
		}
	}

	if (best_length >= min_len)
	{
		data.difference_array[min_len]++;
		data.difference_array[best_length + 1]--;
	}
}

int main() 
{
	std::string s;
	std::cin >> s;
	auto n = s.length();

	auto data = SuffixAutomatonData{};
	data.automaton.resize(2 * n);
	for (auto i = 0; i < n; ++i)
	{
		ExtendAutomaton(data, s[i], i);
	}

	data.link_tree.resize(data.size);
	for (auto i = 1; i < data.size; ++i)
	{
		data.link_tree[data.automaton[i].link].push_back(i);
	}

	data.difference_array.assign(n + 2, 0);

	ProcessStateEndpos(data, 0);

	auto ans = std::vector<int32_t>(n + 1, 0);
	auto current_ans = 0;

	for (auto k = 1; k <= n; ++k)
	{
		current_ans += data.difference_array[k];
		ans[k] = current_ans;
	}

	auto limit = n / 3;
	for (auto k = 1; k <= limit; ++k)
	{
		std::cout << ans[k] << (k == limit ? "" : " ");
	}
	std::cout << std::endl;

	return 0;
}
