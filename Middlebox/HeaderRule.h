#ifndef __HEADER_RULE_H__
#define __HEADER_RULE_H__

#include "../FastORE/OREHelper.h"



class HeaderRule
{
public:
	HeaderRule(string rule,int id);
	~HeaderRule();

	//compare token
	bool filter(OREHelper* oreEngine, const  string& token);

	void init(OREHelper* oreEngine);

	int getid();

private:
	int id;

	//four Range Ciphertext

	uint32_t minFromIP;
	uint32_t maxFromIP;
	uint32_t minToIP;
	uint32_t maxToIP;

	char minFrom[DEF_ORERIGHT_MAXSIZE];
	char maxFrom[DEF_ORERIGHT_MAXSIZE];
	char minTo[DEF_ORERIGHT_MAXSIZE];
	char maxTo[DEF_ORERIGHT_MAXSIZE];


};


#endif


