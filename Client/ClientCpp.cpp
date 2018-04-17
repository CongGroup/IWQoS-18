#include "ClientCpp.h"

#include <fstream>
#include <algorithm>
#include <cstring>


#include "../FastORE/OREHelper.h"
#include "TMiddleBoxService.h"

ClientCpp::ClientCpp() :usePackageLoc(0), usedPackageCount(1)
{
}


ClientCpp::~ClientCpp()
{
}

void ClientCpp::loadHeader(string headerFile)
{
	const int maxLineSize = 64;
	char buffer[maxLineSize];
	ifstream fin(headerFile);

	int blockSize = ORE_BLK_SIZE;

	if (fin.good())
	{
		OREHelper engine;
		engine.Init(ORE_KEY, blockSize);
		vecHeader.clear();
		usedPackageCount += usePackageLoc;
		//cout << "Load max size is " << usedPackageCount <<" at loc "<<usePackageLoc<<". "<< endl;
		for(int i=0; fin.good()&&i<usedPackageCount; i++)
		{
			fin.getline(buffer, maxLineSize);
			if (usePackageLoc-->0)
			{
				i--;
				continue;
			}
			Header head(buffer, i);
			head.init(&engine);
			vecHeader.push_back(std::move(head));
		} 
		fin.close();
		//cout << "Load " << vecHeader.size() << " headers." << endl;

		vecLatency.reserve(vecHeader.size());
		vecPackageSize.reserve(vecHeader.size());
	}
	else
	{
		cout << "Open Header Failed." << endl;
	}
}

vector<Header>* ClientCpp::getpHeaders()
{
	return &vecHeader;
}

PlainTest * ClientCpp::getTester()
{
	return &plainTest;
}

vector<int>* ClientCpp::getLatency()
{
	return &vecLatency;
}



int ClientCpp::sendHeader(TMiddleBoxServiceClient* client, bool setbit, bool setstate)
{
	vector<Header>* headers = getpHeaders();
	uint64_t totalPackageSize = 0;
	uint64_t hitNum = 0;
	uint64_t hitPackageSize = 0;

	//int latency
	vecPackageSize.clear();
	vecLatency.clear();

	int totalSize = headers->size();
	int competeCount = 0;
	int scale = 100;

	for (vector<Header>::iterator it = headers->begin(); it != headers->end(); it++)
	{
		string result;

		string token = it->getToken();

		if (plainTest.available())
		{
			if (!plainTest.check(*it))
			{
				continue;
			}
		}

		TimeDiff::DiffTimeInMicroSecond();

		client->TestHeader(result, token, setbit, setstate);
		int latency = TimeDiff::DiffTimeInMicroSecond();
		vecLatency.push_back(latency);
		vecPackageSize.push_back(it->getPackageSize());
		it->hitRule = stoi(result);
		if (setbit)
		{
			it->bitLatency = latency;
		}
		else if (setstate)
		{
			it->stateLatency = latency;
		}
		else
		{
			it->latency = latency;
		}
		//cout << "-->" << it - headers->begin() << "\t:" << result << endl;

		totalPackageSize += it->getPackageSize();

		if (it->hitRule >= 0)
		{
			hitNum++;
			hitPackageSize += it->getPackageSize();
		}

		competeCount++;
		
		if (totalSize>scale && competeCount % (totalSize / scale) == 0)
		{
			cout << "Complete --> " << competeCount * 100 / (totalSize) << "%" << endl;
		}
	}

	return hitNum;
	//cout << "Use package " << headers->size() << " , total size is " << totalPackageSize << endl;
}

vector<int> ClientCpp::computeLatency(int slice)
{
	vector<int> latencyResult;
	
	int min = *min_element(vecLatency.begin(), vecLatency.end());
	int max = *max_element(vecLatency.begin(), vecLatency.end());

	cout << "Min latency is " << min << " us." << endl;
	cout << "Max latency is " << max << " us." << endl;

	int* ranges = new int[slice+1];
	ranges[0] = min;
	for (int i = 1; i < slice; i++)
	{
		ranges[i] = min + (max - min) / slice*i;
	}
	ranges[slice] = max+1;

	int counter;
	for (int i = 1; i <= slice; i++)
	{
		counter = count_if(vecLatency.begin(), vecLatency.end(),
			[i, ranges](int& it) {return it >= ranges[i - 1] && it < ranges[i]; });

		cout << "From " << ranges[i - 1] << " To " << ranges[i] << " have \t" << counter << " package" << endl;
		latencyResult.push_back(counter);
	}

	return std::move(latencyResult);
}

vector<int> ClientCpp::computeSize(int slice)
{
	vector<int> sizeResult;

	int min = *min_element(vecPackageSize.begin(), vecPackageSize.end());
	int max = *max_element(vecPackageSize.begin(), vecPackageSize.end());

	cout << "Min size is " << min << " bytes." << endl;
	cout << "Max size is " << max << " bytes." << endl;

	int* ranges = new int[slice + 1];
	ranges[0] = min;
	for (int i = 1; i < slice; i++)
	{
		ranges[i] = min + (max - min) / slice*i;
	}
	ranges[slice] = max+1;

	int counter;
	for (int i = 1; i <= slice; i++)
	{
		counter = count_if(vecPackageSize.begin(), vecPackageSize.end(),
			[i, ranges](int& it) {return it >= ranges[i - 1] && it < ranges[i]; });

		cout << "From " << ranges[i - 1] << " To " << ranges[i] << " have \t" << counter << " package" << endl;
		sizeResult.push_back(counter);
	}

	return std::move(sizeResult);
}

int ClientCpp::sendHeaderQuickly(TMiddleBoxServiceClient * client, bool setbit, bool setstate, int seed)
{
	vector<Header>* headers = getpHeaders();
	
	Header* header = &(headers->at(seed%headers->size()));

	string token = header->getToken();

	string result;

	client->TestHeader(result, token, setbit, setstate);

	return 0;
}

void ClientCpp::getTableReport(string filename)
{
	ofstream fout(filename);
	const int bufferSize = 128;
	char buffer[bufferSize];

	if (fout.good())
	{
		fout << "id,hit rule,package size,latency" << endl;
		for (vector<Header>::iterator it = vecHeader.begin();
			it != vecHeader.end(); it++)
		{
			sprintf(buffer, "%d,%d,%d,%d\n", it->id, it->hitRule, it->getPackageSize(), it->latency);
			fout << buffer;
		}
		fout.close();
	}
	else
	{
		cout << "Open file " << filename << " error." << endl;
	}

}

void ClientCpp::setRuleFile(string fileName, int ruleCount)
{
	plainTest.init(fileName, ruleCount);
}

void ClientCpp::hitTest()
{
	int hitCount = 0 ;
	for (vector<Header>::iterator it = vecHeader.begin(); it != vecHeader.end(); it++)
	{
		it->hitRule = plainTest.check(*it);
		if (it->hitRule >= 0)
			hitCount++;
	}
	cout << "Total package is " << vecHeader.size() << ", hit " << hitCount << endl;

}

