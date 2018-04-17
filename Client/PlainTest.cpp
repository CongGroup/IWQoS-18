#include "PlainTest.h"
#include <fstream>
#include <sstream>


void PlainTest::init(string ruleFile, int useCount)
{
	const int maxLineSize = 64;
	char buffer[maxLineSize];
	ifstream fin(ruleFile);

	if (fin.good())
	{
		rules.clear();
		for (int i = 0; fin.good() && i < useCount; i++)
		{
			fin.getline(buffer, maxLineSize);
			stringstream ss(buffer);
			rule curRule;
			ss >> curRule.minFrom >> curRule.maxFrom >> curRule.minTo >> curRule.maxTo;
			if (curRule.minFrom != 0)
				curRule.minFrom--;
			if (curRule.minTo != 0)
				curRule.minTo--;

			rules.push_back(curRule);
			//if (i == 0)
			//{
			//	cout << curRule.minFrom << " " << curRule.maxFrom << " " << curRule.minTo << " " << curRule.maxTo << endl;
			//}
		}
		fin.close();
		cout << "Load " << rules.size() << " rules." << endl;
	}
	else
	{
		cout << "Open rule file " << ruleFile << " Failed." << endl;
	}
}

int PlainTest::check(const Header& header)
{
	int no = 0;
	for (auto it = rules.begin(); it != rules.end(); it++, no++)
	{
		if (it->hit(header.getFrom(), header.getTo()))
			break;
		if (it == rules.begin())
		{
			cout << header.getFrom() << "-->" << header.getTo() << endl;
			cout << it->minFrom << " " << it->maxFrom << ":" << it->minTo << " " << it->maxTo << endl;
		}
	}
		
	if (no == rules.size())
		no = -1;
	return no;
}

bool PlainTest::available()
{
	return rules.size() > 0;
}
