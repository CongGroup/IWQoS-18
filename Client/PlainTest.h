#pragma once
#include <string>
#include <cstdint>
#include <vector>

#include "Header.h"
using namespace std;
class PlainTest
{
public:
	
	struct rule
	{
		uint32_t minFrom;
		uint32_t maxFrom;
		uint32_t minTo;
		uint32_t maxTo;

		bool hit(uint32_t from, uint32_t to)
		{
			return from > minFrom&&from<maxFrom&&to>minTo&&to < maxTo;
		}
	};

	void init(string ruleFile, int useCount);
	int check(const Header& header);
	bool available();
	rule& getRule(int n) { return rules.at(n); }

private:
	vector<rule> rules;
};

