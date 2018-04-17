#pragma once

#include <string>
#include <cstring>
#include <cstdint>
#include <vector>

#include "config.h"

#include "TMiddleBoxService.h"

#include "../Caravel/RedisHelper.h"
#include "../Caravel/ThriftAdapt.h"
#include "../Caravel/TimeDiff.h"
#include "../FastORE/OREHelper.h"

#include "Header.h"
#include "PlainTest.h"

using namespace std;
using namespace caravel;
using namespace MiddleBoxServer;


class ClientCpp
{
public:
	ClientCpp();
	~ClientCpp();

	void loadHeader(string headerFile);
	vector<Header>* getpHeaders();
	PlainTest* getTester();
	vector<int>* getLatency();

	int sendHeader(TMiddleBoxServiceClient* client, bool setbit, bool setstate);
	vector<int> computeLatency(int slice);
	vector<int> computeSize(int slice);

	int sendHeaderQuickly(TMiddleBoxServiceClient* client, bool setbit, bool setstate, int seed);

	void getTableReport(string filename);

	void setRuleFile(string fileName, int ruleCount);

	void hitTest();

	int usePackageLoc;
	int usedPackageCount;

protected:
	vector<Header> vecHeader;
	vector<int> vecLatency;
	vector<int> vecPackageSize;

	PlainTest plainTest;
};

