#pragma once

#include <string>
#include "../FastORE/OREHelper.h"


using std::string;
class Header
{
public:
	Header(string str, int id);
	~Header();

	void init(OREHelper* oreEngine);

	//The token format is 
	// sizeof(int)				id
	// DEF_ORELEFT_MAXSIZE		less than from
	// DEF_ORELEFT_MAXSIZE		more than from
	// DEF_ORELEFT_MAXSIZE		less than to
	// DEF_ORELEFT_MAXSIZE		more than to
	//Total len is sizeof(int) + DEF_ORELEFT_MAXSIZE*4
	string getToken();

	int  getPackageSize() const;
	uint32_t getFrom() const;
	uint32_t getTo() const;

	//packageID
	int id;
	//package hit rule num
	int hitRule;
	//package latency
	int latency;
	//package latency at bit state
	int bitLatency;
	//package latency at counter state
	int stateLatency;
		

protected:

	uint32_t ipFrom;
	uint32_t ipTo;
	uint32_t packageSize;
	
	char moreThanOREFrom[DEF_ORELEFT_MAXSIZE];
	char lessThanOREFrom[DEF_ORELEFT_MAXSIZE];
	char moreThanORETo[DEF_ORELEFT_MAXSIZE];
	char lessThanORETo[DEF_ORELEFT_MAXSIZE];
};

