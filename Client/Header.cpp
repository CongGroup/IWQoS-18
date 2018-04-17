#include "Header.h"
#include <sstream>

#include "../FastORE/OREHelper.h"

using namespace std;

Header::Header(string str, int id)
{
	stringstream ss(str);
	ss >> ipFrom >> ipTo >> packageSize;
	this->id = id;

	hitRule = latency = bitLatency = stateLatency = -1;

	//cout << "Load " << ipFrom << " To " << ipTo <<" size "<<packageSize<< endl;
}


Header::~Header()
{
}

void Header::init(OREHelper * oreEngine)
{
	oreEngine->CreateLeft(moreThanOREFrom, DEF_ORELEFT_MAXSIZE, 1, ipFrom);
	oreEngine->CreateLeft(lessThanOREFrom, DEF_ORELEFT_MAXSIZE, -1, ipFrom);
	oreEngine->CreateLeft(moreThanORETo, DEF_ORELEFT_MAXSIZE, 1, ipTo);
	oreEngine->CreateLeft(lessThanORETo, DEF_ORELEFT_MAXSIZE, -1, ipTo);
}

//The token format is 
// sizeof(int)				id
// DEF_ORELEFT_MAXSIZE		less than from
// DEF_ORELEFT_MAXSIZE		more than from
// DEF_ORELEFT_MAXSIZE		less than to
// DEF_ORELEFT_MAXSIZE		more than to
//Total len is sizeof(int) + DEF_ORELEFT_MAXSIZE*4
string Header::getToken()
{
	string token;
	token.assign(sizeof(int),'\0');
	memcpy((char*)token.data(), &id, sizeof(int));
	token.append(moreThanOREFrom,	moreThanOREFrom + DEF_ORELEFT_MAXSIZE);
	token.append(lessThanOREFrom,	lessThanOREFrom + DEF_ORELEFT_MAXSIZE);
	token.append(moreThanORETo,		moreThanORETo	+ DEF_ORELEFT_MAXSIZE);
	token.append(lessThanORETo,		lessThanORETo	+ DEF_ORELEFT_MAXSIZE);
	return std::move(token);
}

int Header::getPackageSize()  const
{
	return packageSize;
}

uint32_t Header::getFrom()  const
{
	return ipFrom;
}

uint32_t Header::getTo()  const
{
	return ipTo;
}
