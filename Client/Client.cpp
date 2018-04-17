#include <string>
#include <cstring>
#include <cstdint>
#include <iostream>


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

	if (argc == 6)
	{
		ClientCpp clientcpp;
		char* headerFileName = "\0";
		int type = 0;
		int slices = 1;
		char* ruleFileName = "\0";
		char* baseOutputName = "\0";
		char* serverIP = "\0";
		int ruleCount = 0;

		//which middle to connect
		int middleBoxID;

		headerFileName = argv[1];
		sscanf(argv[2], "%d", &clientcpp.usedPackageCount);
		//sscanf(argv[3], "%d", &usePackageLoc);
		sscanf(argv[3], "%d", &type);
		//sscanf(argv[4], "%d", &slices);
		baseOutputName = argv[4];
		sscanf(argv[5], "%d", &middleBoxID);
		//ruleFileName = argv[5];
		//sscanf(argv[6], "%d", &ruleCount);

		if (clientcpp.usedPackageCount == 0)
		{
			clientcpp.usedPackageCount = INT32_MAX;
		}

		if (ruleCount == 0)
		{
			ruleCount = INT32_MAX;
		}

		if (strlen(ruleFileName) > 2)
		{
			clientcpp.setRuleFile(ruleFileName, ruleCount);
		}

		if (slices < 1)
			slices = 1;
		
		type = 1;

		bool setNormal = type & 0x1;
		bool setBit = type & 0x2;
		bool setState = type & 0x4;
		bool setHitTest = type & 0x8;

		//test package hit at loacl
		if (setHitTest&&clientcpp.getTester()->available())
		{
			clientcpp.loadHeader(headerFileName);
			clientcpp.hitTest();
			string filename = baseOutputName;
			filename.append("-hit.csv");
			clientcpp.getTableReport(filename);
			return 0;
		}

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
		case 4:
		{
			serverIP = MIDDLEBOX4;
			break;
		}
		case 5:
		{
			serverIP = MIDDLEBOX5;
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
		cout << "Finish connect to server." << endl;

		clientcpp.loadHeader(headerFileName);

		uint32_t useTime = 0;
		uint32_t hitNum = 0;

		cout << endl;

		//clientcpp.computeSize(slices);
		if (setNormal)
		{
			cout << "In normal package"<< endl;
			hitNum = clientcpp.sendHeader(client, false, false);
			cout << "Total " << clientcpp.getpHeaders()->size() << " packages, and hit " << hitNum << " packages" << endl;
			//clientcpp.computeLatency(slices);
			string filename = baseOutputName;
			filename.append(".csv");
			clientcpp.getTableReport(filename);
		}

		if (setBit)
		{
			cout << endl << "In Set Bit" << endl;
			hitNum = clientcpp.sendHeader(client, true, false);
			cout <<"Total " << clientcpp.getpHeaders()->size() << " packages, and hit " << hitNum << " packages" << endl;
			//clientcpp.computeLatency(slices);
			string filename = baseOutputName;
			filename.append("-bit.csv");
			clientcpp.getTableReport(filename);
		}

		if (setState)
		{
			cout << endl << "In Set State" << endl;
			hitNum = clientcpp.sendHeader(client, false, true);
			cout << "Total " << clientcpp.getpHeaders()->size() << " packages, and hit " << hitNum << " packages" << endl;
			//clientcpp.computeLatency(slices);
			string filename = baseOutputName;
			filename.append("-state.csv");
			clientcpp.getTableReport(filename);
		}

		if (!setNormal && !setBit && !setState)
		{
			string filename = baseOutputName;
			filename.append("-package.csv");
			clientcpp.getTableReport(filename);
		}

		thriftAdapt.Close();

		return 0;
	}
	else
	{
		cout << "Usage: Latency HeaderFIle PackageNum Type OutputFile MiddleBox    " << endl;
		cout << endl;
		cout << "[HeaderFile]       = > The file name of header.   " << endl;
		cout << "[PackageNum]       = > The count of package to use.   " << endl;
		cout << "[Type]             = > 1: normal 2:setBit 4:setState. " << endl;
		cout << "[OutputFile]       = > The file name of OutputFile. " << endl;
		cout << "[MiddleBox]        = > The ID of MiddleBox. " << endl;
		//cout << "[RuleFile]         = > The file of rules. " << endl;
		//cout << "[RuleCount]        = > The use count of rules. " << endl;
		//cout << "[Slice]            = > The latency sliced sheet count." << endl;
		//cout << "[PackageOffset]	= > Use package begin of this.   " << endl;
		cout << endl;
		return -1;
	}


}