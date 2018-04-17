#include <iostream>
#include <stdio.h>
#include <string>
#include <random>
#include <fstream>
#include "../FastORE/OREHelper.h"
#include "../Caravel/RedisHelper.h"

#include "PlainTest.h"
#include "ClientCpp.h"
using namespace std;
using namespace caravel;

void genHeader(string outputPath, int packageCount, double hitRate, PlainTest& rules, int ruleCount)
{
	random_device rd;
	const int iseed = rd();
	const int packageSize = 60;

	mt19937 engine(iseed);
	uniform_int_distribution<int> drules(0, ruleCount - 1);
	

	ofstream fout(outputPath);

	if (fout.good())
	{
		for (int i = 0; i < packageCount; i++)
		{
			PlainTest::rule& r = rules.getRule(drules(engine));

			uniform_int_distribution<unsigned int> dfrom(r.minFrom + 1, r.maxFrom - 1);
			uniform_int_distribution<unsigned int> dto(r.minTo+1, r.maxTo-1);

			fout << dfrom(engine) << " " << dto(engine) << " " << packageSize << endl;
		}

		fout.close();
	}
	else
	{
		cout << "error on open file" << outputPath << endl;
	}

}

void plainTest()
{
	string ruleFileName("firewall.rule");
	string packageFileName("header20000.head");


	//oreTest();
	PlainTest plainTest;

	plainTest.init(ruleFileName, 1600);

	ClientCpp clientcpp;
	clientcpp.usedPackageCount = 10000;
	clientcpp.loadHeader(packageFileName);


	int hitCount = 0;
	for (int i = 0; i < clientcpp.usedPackageCount; i++)
	{
		if (plainTest.check(clientcpp.getpHeaders()->at(i)) >= 0)
		{
			hitCount++;
		}
	}
	cout << "Local test hit count is " << hitCount << endl;
}

int main()
{
	//plainTest();

	string ruleFileName("firewall.rule");
	PlainTest plainTest;
	plainTest.init(ruleFileName, 1600);
	genHeader("header20000.head", 20000, 1, plainTest, 1600);
}


//
//void oreTest()
//{
//	const int boundary = 5;
//
//	char left[DEF_ORELEFT_MAXSIZE];
//	char right[boundary][DEF_ORERIGHT_MAXSIZE];
//
//	OREHelper engine;
//
//	//the block_len is ore block len, [2|4|8]
//	engine.Init("Hello world!", 8);
//
//	//these three can be anything but not bull
//	string tableName = "table";
//	string columnName = "column";
//	uint32_t counter = 1;
//
//	cout << "Begin enc data." << endl;
//
//	for (int i = 0; i < boundary; i++)
//	{
//		engine.CreateRight(right[i], DEF_ORERIGHT_MAXSIZE, i);
//		//break;
//	}
//
//	cout << "Begin query." << endl;
//
//	int round = boundary;
//
//	while (round--)
//	{
//		int query = rand() % boundary;
//		query = -1;
//		cout << "The query is " << query << endl;
//
//		// query compare with ciphertext result is >
//		engine.CreateLeft(left, DEF_ORELEFT_MAXSIZE, 1, query);
//
//		cout << "  Find less than query: ";
//
//		for (int i = 0; i < boundary; i++)
//		{
//			int res = engine.CompareORE(left, DEF_ORELEFT_MAXSIZE, right[i], DEF_ORERIGHT_MAXSIZE);
//			if (res > 0)
//				cout << i << " ";
//			//break;
//		}
//		cout << endl;
//		//break;
//	}
//
//	cout << "Bye." << endl;
//}
