
#include "TGatewayServiceHandler.h"

#include <iostream>
#include <ctime>
#include <assert.h>

TGatewayServiceHandler::TGatewayServiceHandler()
{

}


TGatewayServiceHandler::~TGatewayServiceHandler()
{
}

//rule states format
//DEF_ORERIGHT_MAXSIZE  ORE cipher text 
//AES_BLOCK_SIZE		AES cipher text 
//sizeof(time_t)		start time
void TGatewayServiceHandler::IncCounter(const string& RuleID) 
{
	string currentState;
	char oreRight[DEF_ORERIGHT_MAXSIZE];
	char aesCounter[AES_BLOCK_SIZE] = { 0 };
	char emptyIV[AES_BLOCK_SIZE] = { 0 };
	int counter = -1;
	time_t startTime = 0;

	OREHelper oreHelper;
	int blockSize = 8;
	oreHelper.Init(ORE_KEY, blockSize);
	
	redisHelper.PoolGet(RuleID, currentState);

	if (currentState.size() > DEF_ORERIGHT_MAXSIZE)
	{
		if (currentState.size() != DEF_ORERIGHT_MAXSIZE + AES_BLOCK_SIZE + sizeof(time_t))
		{
			cout << "State len error." << endl;
		}
		memcpy(&startTime, currentState.data()+ DEF_ORERIGHT_MAXSIZE+ AES_BLOCK_SIZE, sizeof(time_t));
		//cout << "start time is " << startTime << endl;
		time_t currentTime = time(0);
		if (currentTime - startTime < STATE_NUM)
		{
			memcpy(aesCounter, currentState.data() + DEF_ORERIGHT_MAXSIZE, AES_BLOCK_SIZE);
			AES::CbcDecrypt256(aesCounter, AES_BLOCK_SIZE, aesCounter, AES_KEY, emptyIV);
			memcpy(&counter, aesCounter, sizeof(int));
		}
		else
		{
			counter = -1;
			startTime = 0;
		}
	}

	counter += 1;
	if (!startTime)
	{
		startTime = time(0);
	}
	//oreHelper.CreateRight(oreRight, DEF_ORERIGHT_MAXSIZE, counter);
	AES::CbcEncrypt256((char*)&counter, sizeof(time_t), aesCounter, AES_KEY, emptyIV);
	
	currentState.assign(oreRight, oreRight + DEF_ORERIGHT_MAXSIZE);
	currentState.append(aesCounter, aesCounter + AES_BLOCK_SIZE);
	currentState.append((char*)&startTime, (char*)&startTime+sizeof(time_t));

	//cout << "IncCounter rowID: "<< RuleID<< " Counter: " << counter << endl;

	redisHelper.PoolPut(RuleID, currentState);
}

bool TGatewayServiceHandler::Init(int argc, char **argv) {

	if (argc == 2)
	{
		cout << "Init Gateway ." << endl;



		char* redisIP;

		int middleBoxID;

		sscanf(argv[1], "%d", &middleBoxID);

		switch (middleBoxID)
		{
		case 1:
		{
			redisIP = MIDDLEBOX1;
			break;
		}
		case 2:
		{
			redisIP = MIDDLEBOX2;
			break;
		}
		case 3:
		{
			redisIP = MIDDLEBOX3;
			break;
		}
		case 4:
		{
			redisIP = MIDDLEBOX4;
			break;
		}
		case 5:
		{
			redisIP = MIDDLEBOX5;
			break;
		}
		default:
		{
			cout << "Wrong middle box id" << endl;
			return 0;
		}
		}


		redisHelper.OpenPool(redisIP, REDIS_SERVER_PORE);
		cout << "Connect to redis." << endl;
		return true;
	}
	else
	{
		cout << "Usage: Gateway MiddleBoxID " << endl;
		cout << endl;
		cout << "[MiddleBoxID]			= > The ID of MiddleBoxID.   " << endl;
		cout << endl;
		return false;
	}
}