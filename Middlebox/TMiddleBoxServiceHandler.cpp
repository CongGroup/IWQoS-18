#include "TMiddleBoxServiceHandler.h"

#include <iostream>



TMiddleBoxServiceHandler::TMiddleBoxServiceHandler()
{
}


TMiddleBoxServiceHandler::~TMiddleBoxServiceHandler()
{
}

void TMiddleBoxServiceHandler::TestHeader(string& _return, const string& Token, const bool SetBit, const bool SetState)
{


	//cout << "TestHander" << endl;

	int res = filter.CheckHeader(Token,SetBit||SetState);
	
	_return.assign(to_string(res));

	if (res >= 0)
	{
		int* id = (int*)Token.data();
		string key = to_string(*id);
		if (SetBit)
		{
			//cout << "SetBit, packageID " << key << " #role "<< _return << endl;
			redisHelper.PoolPut(key, _return);
		}
		if (SetState)
		{
			//To test throughput We need a random key
			//if the key all is 0, the throughput will very low
			//key.assign(_return);
			ThriftAdapt<TGatewayServiceClient> adapt;
			adapt.Init(GATEWAY_IP, GATEWAY_PORT);
			adapt.Open();

			key.assign(key);
			key.append("state");

			//cout << "SetState the key is "<< key << endl;

			TGatewayServiceClient* client = adapt.GetClient();
			client->IncCounter(key);

			string currentStateORE;
			redisHelper.PoolGet(key, currentStateORE);
			{
				if (currentStateORE.size() > DEF_ORERIGHT_MAXSIZE)
				{
#ifdef BOX_ORE
					res = engine.CompareORE(stateORE, DEF_ORELEFT_MAXSIZE, stateORE, DEF_ORELEFT_MAXSIZE, (char*)currentStateORE.data(), DEF_ORERIGHT_MAXSIZE);

#else
					res = engine.CompareORE(stateORE, DEF_ORELEFT_MAXSIZE, (char*)currentStateORE.data(), DEF_ORERIGHT_MAXSIZE);

#endif
					if (res != 1)
					{
						//cout << "Drop cause of state, rule id is " << key << endl;
						_return.assign(to_string(-1));
						return;
					}
				}
			}

			adapt.Close();
		}
	}

}

bool TMiddleBoxServiceHandler::Init(int argc, char **argv)
{
	cout << "MiddleBox Init" << endl;

	if (argc == 3)
	{
		int ruleCount;
		char* ruleFileName = argv[1];
		
		sscanf(argv[2], "%d", &ruleCount);

		bool shutdown = false;

		if (ruleCount < 0)
		{
			shutdown = true;
			ruleCount = -ruleCount;
		}

		if (ruleCount <= 0)
		{
			ruleCount = INT32_MAX;
		}

		int type = 0;

		//redisHelper.OpenPool(REDIS_SERVER_IP, REDIS_SERVER_PORE);


		//cout << "Finish connect to Gateway." << endl;

		TimeDiff::DiffTimeInMicroSecond();

		filter.Build(ruleFileName, ruleCount);

		uint32_t buildTime = TimeDiff::DiffTimeInMicroSecond();

		cout << "Finish read rules, use "<< buildTime <<" us." << endl;

		int oreBlockSize = ORE_BLK_SIZE;

		engine.Init(ORE_KEY, oreBlockSize);
		engine.CreateLeft(stateORE, DEF_ORELEFT_MAXSIZE, 1, STATE_NUM);

		return !shutdown;
	}
	else
	{
		cout << "Usage: Middlebox rulefile rulecount" << endl;
		cout << endl;
		cout << "[rulefile]         = > The file name of rules.   " << endl;
		cout << "[rulecount]        = > The count of rule to use.   " << endl;
		cout << endl;
		return false;
	}


}

