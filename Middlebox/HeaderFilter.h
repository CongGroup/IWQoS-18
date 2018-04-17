#ifndef __HEADER_FILTER_H__
#define __HEADER_FILTER_H__

#include <vector>
#include <string>
#include <string.h>
#include <stdint.h>

#include "HeaderRule.h"

using namespace std;

class HeaderFilter
{
public:
	HeaderFilter();

	//return -1 if all rule miss
	//else return rule ID
	int CheckHeader(const string& token, bool fastReturn);

	//init all rules
	void Build(string path, int size);

private:

	OREHelper engine;

	//存放所有的Rules
	vector<HeaderRule> vecHeaderRules;

};



#endif
