#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <set>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

class tsort
{
private:
	std::vector<std::pair<std::string, std::string> > pairs;
	std::vector<std::pair<std::string, unsigned> > order;
public:
	tsort();
	~tsort();

	void addPair(const std::string& src, const std::string& dst);
	void sort(bool checkCycles = true);
	inline unsigned size() const
		{ return order.size(); }
	std::string nodeAtIndex(unsigned idx, unsigned* level);
};

tsort::tsort()
{
}

tsort::~tsort()
{
}

std::string tsort::nodeAtIndex(unsigned idx, unsigned* level)
{
	if (level)
		*level = order[idx].second;
	return order[idx].first;
}

void tsort::addPair(const std::string& src, const std::string& dst)
{
	pairs.push_back(std::make_pair(src, dst));
}

namespace
{
	struct tsort_node
	{
		std::string data;
		std::vector<tsort_node*> successors;
		bool visited;
		unsigned npreds;
		tsort_node(const std::string& nodeData) : data(nodeData), visited(false), npreds(0) {}
	};
}

void tsort::sort(bool checkCycles)
{
	// map of entries
	std::map<std::string, tsort_node*> nodes;
	std::set<tsort_node*> sources;
	order.clear();
	for (std::vector<std::pair<std::string, std::string> >::iterator iter = pairs.begin(); iter != pairs.end(); ++iter)
	{
		if (nodes.find(iter->first) == nodes.end())
		{
			tsort_node* newNode = new tsort_node(iter->first);
			nodes[iter->first] = newNode;
			sources.insert(newNode);
		}
		if (nodes.find(iter->second) == nodes.end())
		{
			tsort_node* newNode = new tsort_node(iter->second);
			nodes[iter->second] = newNode;
			sources.insert(newNode);
		}
		nodes[iter->first]->successors.push_back(nodes[iter->second]);
		nodes[iter->second]->npreds++;
		if (nodes[iter->second]->npreds == 1)
		{
			sources.erase(sources.find(nodes[iter->second]));
		}
	}
	unsigned base = 0;
	while (!sources.empty())
	{
		std::set<tsort_node*> thislevel;
		sources.swap(thislevel);
		for (std::set<tsort_node*>::iterator iter = thislevel.begin(); iter != thislevel.end(); ++iter)
		{
			tsort_node* node = *iter;
			if (checkCycles && node->visited)
			{
				fprintf(stderr, "cycle detected!\n");
				abort();
			}
			node->visited = true;
			order.push_back(std::make_pair(node->data, base));
			for (std::vector<tsort_node*>::iterator iter = node->successors.begin(); iter != node->successors.end(); ++iter)
			{
				tsort_node* succ = *iter;
				succ->npreds--;
				if (succ->npreds == 0)
				{
					sources.insert(succ);
				}
			}
			if (!checkCycles)
				delete node;
		}
		thislevel.clear();
		++base;
	}
	if (checkCycles)
	{
		for (std::map<std::string, tsort_node*>::iterator iter = nodes.begin(); iter != nodes.end(); ++iter)
		{
			delete iter->second;
		}
	}
}

void printHelpAndExit(const char* av0)
{
	printf("Usage: %s [options] <sourceFile\n", av0);
	printf("\t--help/-h      print this help\n");
	printf("\t--version/-v   print version information\n");
	printf("\t--cycles/-c    perform cycle checking\n");
	printf("\t--parallel/-p  print parallel operation information\n");
	printf("\t-dN            set delimiter to N\n");
	exit(0);
}

void printVersion()
{
	printf("tsort2 0.0.1\n");
	printf("Copyright (c) Alistair Lynn, 2010\n");
}

int main(int argc, char** argv)
{
	char delimiter = ' ';
	bool checkCycles = false;
	bool parallel = false;
	for (int i = 1; i < argc; ++i)
	{
		const char* arg = argv[i];
		if (!strcmp(arg, "-h") || !strcmp(arg, "--help"))
			printHelpAndExit(argv[0]);
		else if (!strcmp(arg, "-v") || !strcmp(arg, "--version"))
			printVersion();
		else if (!strcmp(arg, "-c") || !strcmp(arg, "--cycles"))
			checkCycles = true;
		else if (!strcmp(arg, "-p") || !strcmp(arg, "--parallel"))
			parallel = true;
		else if (strlen(arg) == 3 && arg[0] == '-' && arg[1] == 'd')
			delimiter = arg[2];
		else
			fprintf(stderr, "unknown argument %s ignored\n", arg);
	}
	tsort sorter;
	// add all pairs
	std::string src, dst;
	while (std::getline(std::cin, src, delimiter))
	{
		if (!std::getline(std::cin, dst, '\n'))
		{
			fprintf(stderr, "unmatched pair (src=%s)\n", src.c_str());
		}
		else
		{
			sorter.addPair(src, dst);
		}
	}
	// perform topological sort
	sorter.sort(checkCycles);
	// print out results
	unsigned lastLevel = -1U;
	for (unsigned i = 0; i < sorter.size(); ++i)
	{
		unsigned thisLevel;
		std::string node = sorter.nodeAtIndex(i, &thisLevel);
		if (parallel)
		{
			if (thisLevel != lastLevel)
				printf("%u:\n", thisLevel);
			lastLevel = thisLevel;
			putchar('\t');
		}
		printf("%s\n", node.c_str());
	}
	return 0;
}
