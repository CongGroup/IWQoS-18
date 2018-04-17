#include <string>
#include <cstring>
#include <cstdint>
#include <iostream>
#include <ctime>

#include "config.h"

#include "TMiddleBoxService.h"

#include "../Caravel/ThriftAdapt.h"
#include "../Caravel/TimeDiff.h"
#include "../FastORE/OREHelper.h"

#include "ClientCpp.h"
#include "Header.h"


using namespace std;
using namespace caravel;
using namespace MiddleBoxServer;

int main(int argc, char ** argv)
{

	if (argc == 7)
	{
		ClientCpp clientcpp;
		char* headerFileName = "\0";
		int type = 0;
		int slices = 1;
		char* serverIP = "\0";
		int ruleCount = 0;
		int beginTime;
		int testTime;
		int seed;

		//which middle to connect
		int middleBoxID;

		headerFileName = argv[1];
		sscanf(argv[2], "%d", &type);
		sscanf(argv[3], "%d", &beginTime);
		sscanf(argv[4], "%d", &testTime);
		sscanf(argv[5], "%d", &seed);
		sscanf(argv[6], "%d", &middleBoxID);

		type =1;

		bool setNormal = type & 0x1;
		bool setBit = type & 0x2;
		bool setState = type & 0x4;

		switch (middleBoxID)
		{
		case 1:
		{
			serverIP = MIDDLEBOX1;
			break;
		}
		case 2:
		{
			serverIP = MIDDLEBOX2;
			break;
		}
		case 3:
		{
			serverIP = MIDDLEBOX3;
			break;
		}
		default:
		{
			cout << "Wrong middle box id" << endl;
			return 0;
		}
		}

		//connect to thrift
		ThriftAdapt<TMiddleBoxServiceClient> thriftAdapt;
		int middleBoxPort = 9090;
		thriftAdapt.Init(serverIP, middleBoxPort);
		thriftAdapt.Open();

		TMiddleBoxServiceClient* client = thriftAdapt.GetClient();

		clientcpp.usedPackageCount = 3200;

		clientcpp.loadHeader(headerFileName);

		srand(seed);

		uint32_t uiCurTime;
		uint32_t uiBegTime = beginTime;
		uint32_t uiEndTime = beginTime + testTime;

		uint32_t times=0;

		while (true)
		{
			uiCurTime = time(NULL);

			if (uiCurTime < uiBegTime)
			{
				continue;
			}

			if (uiCurTime >= uiEndTime)
			{
				break;
			}

			if (setNormal)
			{
				clientcpp.sendHeaderQuickly(client, false, false, rand());
			}
			else if (setBit)
			{
				clientcpp.sendHeaderQuickly(client, true, false, rand());
			}
			else if (setState)
			{
				clientcpp.sendHeaderQuickly(client, false, true, rand());
			}

			times++;

		}

		thriftAdapt.Close();

		cout << times << endl;

		return 0;
	}
	else
	{
		cout << "Usage: ThroughphtputClient HeaderFile Type Begin Time Seed MiddleBox    " << endl;
		cout << endl;
		cout << "[HeaderFile]       = > The file name of header.   " << endl;
		cout << "[Type]             = > 1: normal 2:setBit 4:setState. " << endl;
		cout << "[Begin]            = > The test begin time. " << endl;
		cout << "[Time]             = > The test go on time. " << endl;
		cout << "[Seed]				= > The random seed. " << endl;
		cout << "[MiddleBox]        = > The ID of MiddleBox. " << endl;
		cout << endl;
		return -1;
	}


}