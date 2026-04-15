#include <algorithm>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

struct Node
{
	int parent = -1;
	int depth = 0;
	int suf_idx = -1;
	std::vector<int> children;
};


class SuffixArray
{
      public:
	SuffixArray(const std::string &combinedString)
	    : text(combinedString), totalLength(combinedString.length()), suffixArray(totalLength, 0),
	      rank(totalLength, 0), lcp(totalLength > 0 ? totalLength - 1 : 0, 0)
	{
		if (totalLength > 0)
		{
			buildSuffixArray();
			buildLCP();
		}
	}

	const std::vector<int> &getSA() const
	{
		return suffixArray;
	}
	const std::vector<int> &getLCP() const
	{
		return lcp;
	}

      private:
	void buildSuffixArray()
	{
		std::vector<int> charClass(totalLength, 0);
		for (int i = 0; i < totalLength; i++)
		{
			charClass[i] = text[i];
			suffixArray[i] = i;
		}

		for (int k = 1; k < totalLength; k *= 2)
		{
			std::vector<std::pair<std::pair<int, int>, int>> pairs(totalLength);
			for (int i = 0; i < totalLength; i++)
			{
				int nextClass = (i + k < totalLength) ? charClass[i + k] : -1;
				pairs[i] = {{charClass[i], nextClass}, i};
			}
			std::sort(pairs.begin(), pairs.end());
			for (int i = 0; i < totalLength; i++)
			{
				suffixArray[i] = pairs[i].second;
			}
			charClass[suffixArray[0]] = 0;
			for (int i = 1; i < totalLength; i++)
			{
				int diff = (pairs[i].first != pairs[i - 1].first) ? 1 : 0;
				charClass[suffixArray[i]] = charClass[suffixArray[i - 1]] + diff;
			}
			if (charClass[suffixArray[totalLength - 1]] == totalLength - 1)
			{
				break;
			}
		}

		for (int i = 0; i < totalLength; i++)
		{
			rank[suffixArray[i]] = i;
		}
	}

	void buildLCP()
	{
		int currentLcp = 0;
		for (int i = 0; i < totalLength; i++)
		{
			if (rank[i] == totalLength - 1)
			{
				currentLcp = 0;
				continue;
			}
			int j = suffixArray[rank[i] + 1];
			while (i + currentLcp < totalLength && j + currentLcp < totalLength &&
			       text[i + currentLcp] == text[j + currentLcp])
			{
				currentLcp++;
			}
			lcp[rank[i]] = currentLcp;
			if (currentLcp > 0)
			{
				currentLcp--;
			}
		}
	}

	int totalLength;
	std::string text;
	std::vector<int> suffixArray;
	std::vector<int> rank;
	std::vector<int> lcp;
};

class SuffixTree
{
      public:
	SuffixTree(const std::string &s, const std::string &t)
	    : stringS(s), stringT(t), combinedString(s + t), lengthS(s.length()), totalLength((s + t).length())
	{
	}

	void build(const std::vector<int> &suffixArray, const std::vector<int> &lcp)
	{
		tree.clear();
		tree.push_back(Node());
		std::vector<int> activePathStack;
		activePathStack.push_back(0);

		for (int i = 0; i < totalLength; i++)
		{
			int suffixStart = suffixArray[i];
			int maxSuffixLen = (suffixStart < lengthS) ? lengthS - suffixStart : totalLength - suffixStart;
			int lcpValue = (i == 0) ? 0 : lcp[i - 1];
			if (lcpValue > maxSuffixLen)
			{
				lcpValue = maxSuffixLen;
			}

			int currentNode = activePathStack.back();
			int previousNode = -1;
			while (tree[currentNode].depth > lcpValue)
			{
				previousNode = currentNode;
				activePathStack.pop_back();
				currentNode = activePathStack.back();
			}

			if (tree[currentNode].depth < lcpValue)
			{
				int splitNode = tree.size();
				tree.push_back(Node());
				tree[splitNode].parent = currentNode;
				tree[splitNode].depth = lcpValue;
				tree[splitNode].suf_idx = tree[previousNode].suf_idx;

				for (int j = 0; j < (int)tree[currentNode].children.size(); j++)
				{
					if (tree[currentNode].children[j] == previousNode)
					{
						tree[currentNode].children[j] = splitNode;
						break;
					}
				}

				tree[previousNode].parent = splitNode;
				tree[splitNode].children.push_back(previousNode);

				activePathStack.push_back(splitNode);
				currentNode = splitNode;
			}

			if (maxSuffixLen > tree[currentNode].depth)
			{
				int leafNode = tree.size();
				tree.push_back(Node());
				tree[leafNode].parent = currentNode;
				tree[leafNode].depth = maxSuffixLen;
				tree[leafNode].suf_idx = suffixStart;
				tree[currentNode].children.push_back(leafNode);
				activePathStack.push_back(leafNode);
			}
		}
	}

	void printDFS() const
	{
		if (tree.empty())
			return;

		std::vector<int> dfsOrder;
		std::vector<int> newId(tree.size(), 0);
		std::vector<int> dfsStack;
		dfsStack.push_back(0);

		while (!dfsStack.empty())
		{
			int u = dfsStack.back();
			dfsStack.pop_back();
			newId[u] = dfsOrder.size();
			dfsOrder.push_back(u);
			for (int i = (int)tree[u].children.size() - 1; i >= 0; i--)
			{
				dfsStack.push_back(tree[u].children[i]);
			}
		}

		std::cout << tree.size() << "\n";
		for (int i = 1; i < (int)tree.size(); i++)
		{
			int u = dfsOrder[i];
			int p = tree[u].parent;
			int pNew = newId[p];

			int startPos = tree[u].suf_idx + tree[p].depth;
			int endPos = tree[u].suf_idx + tree[u].depth;

			int stringIndicator = 0;
			int leftBound = 0;
			int rightBound = 0;

			if (tree[u].suf_idx < lengthS)
			{
				stringIndicator = 0;
				leftBound = startPos;
				rightBound = endPos;
			}
			else
			{
				stringIndicator = 1;
				leftBound = startPos - lengthS;
				rightBound = endPos - lengthS;
			}

			std::cout << pNew << " " << stringIndicator << " " << leftBound << " " << rightBound << "\n";
		}
	}

      private:
	std::string stringS;
	std::string stringT;
	std::string combinedString;
	int lengthS;
	int totalLength;
	std::vector<Node> tree;
};

int main()
{
	std::string stringS, stringT;
	std::cin >> stringS >> stringT;

	SuffixArray saBuilder(stringS + stringT);

	SuffixTree treeBuilder(stringS, stringT);
	treeBuilder.build(saBuilder.getSA(), saBuilder.getLCP());

	treeBuilder.printDFS();

	return 0;
}
