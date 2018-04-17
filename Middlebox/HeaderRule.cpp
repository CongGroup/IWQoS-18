#include "HeaderRule.h"
#include <sstream>
#include <iostream>

#include "config.h"


HeaderRule::HeaderRule(string rule, int id)
{
	stringstream ss(rule);
	ss >> minFromIP >> maxFromIP >> minToIP >> maxToIP;
	if (minFromIP != 0)
		minFromIP--;
	if (minToIP != 0)
		minToIP--;
	this->id = id;
}


HeaderRule::~HeaderRule()
{

}

bool HeaderRule::filter(OREHelper* oreEngine, const string& token)
{
	
	if (token.size() != DEF_ORELEFT_MAXSIZE * 4 + sizeof(int))
	{
		cout << "Error token" << endl;
		return false;
	}


	int res;

#ifdef BOX_ORE
	char* compLeftMin = (char*)token.data() + sizeof(int);
	char* compLeftMax = (char*)token.data() + sizeof(int) + DEF_ORELEFT_MAXSIZE;

	res = oreEngine->CompareORE(compLeftMin, DEF_ORELEFT_MAXSIZE, compLeftMax, DEF_ORELEFT_MAXSIZE, minFrom, DEF_ORERIGHT_MAXSIZE);
	if (res != 1)
	{
		return false;
	}

	compLeftMin += DEF_ORELEFT_MAXSIZE*2;
	compLeftMax += DEF_ORELEFT_MAXSIZE*2;

	res = oreEngine->CompareORE(compLeftMin, DEF_ORELEFT_MAXSIZE, compLeftMax, DEF_ORELEFT_MAXSIZE, minTo, DEF_ORERIGHT_MAXSIZE);
	if (res != 1)
	{
		return false;
	}

#else

	char* compLeft = (char*)token.data() + sizeof(int);
	res = oreEngine->CompareORE(compLeft, DEF_ORELEFT_MAXSIZE, minFrom, DEF_ORERIGHT_MAXSIZE);
	if (res != 1)
	{
		return false;
	}

	compLeft += DEF_ORELEFT_MAXSIZE;
	res = oreEngine->CompareORE(compLeft, DEF_ORELEFT_MAXSIZE, maxFrom, DEF_ORERIGHT_MAXSIZE);
	if (res != 1)
	{
		return false;
	}

	compLeft += DEF_ORELEFT_MAXSIZE;
	res = oreEngine->CompareORE(compLeft, DEF_ORELEFT_MAXSIZE, minTo, DEF_ORERIGHT_MAXSIZE);
	if (res != 1)
	{
		return false;
	}

	compLeft += DEF_ORELEFT_MAXSIZE;
	res = oreEngine->CompareORE(compLeft, DEF_ORELEFT_MAXSIZE, maxTo, DEF_ORERIGHT_MAXSIZE);
	if (res != 1)
	{
		return false;
	}
#endif

	return true;
}

void HeaderRule::init(OREHelper * oreEngine)
{
#ifdef BOX_ORE
	oreEngine->CreateRight(minFrom, DEF_ORERIGHT_MAXSIZE, minFromIP, maxFromIP);
	oreEngine->CreateRight(minTo, DEF_ORERIGHT_MAXSIZE, minToIP, maxToIP);
#else
	oreEngine->CreateRight(minFrom, DEF_ORERIGHT_MAXSIZE, minFromIP);
	oreEngine->CreateRight(maxFrom, DEF_ORERIGHT_MAXSIZE, maxFromIP);
	oreEngine->CreateRight(minTo, DEF_ORERIGHT_MAXSIZE, minToIP);
	oreEngine->CreateRight(maxTo, DEF_ORERIGHT_MAXSIZE, maxToIP);
#endif

}

int HeaderRule::getid() { return id; }
