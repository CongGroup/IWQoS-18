#ifndef __TMIDDLEBOX_SERVICE_HANDLER_H__
#define __TMIDDLEBOX_SERVICE_HANDLER_H__
#include <string>
#include <cstring>
#include <cstdint>

#include "config.h"

#include "TMiddleBoxService.h"
#include "TGatewayService.h"

#include "../Caravel/RedisHelper.h"
#include "../Caravel/ThriftAdapt.h"
#include "../Caravel/TimeDiff.h"
#include "../FastORE/OREHelper.h"

#include "HeaderFilter.h"

using namespace std;
using namespace caravel;
using namespace GatewayServer;
using namespace MiddleBoxServer;

class TMiddleBoxServiceHandler : virtual public TMiddleBoxServiceIf {
public:
	TMiddleBoxServiceHandler();
	~TMiddleBoxServiceHandler();


	/**
	* TestHeader API
	* @return Value as Binary
	*
	* @param Token
	* @param SetBit
	* @param SetState
	*/
	void TestHeader(string& _return, const string& Token, const bool SetBit, const bool SetState);


	bool Init(int argc, char **argv);

protected:
	
	RedisHelper redisHelper;
	HeaderFilter filter;
	OREHelper engine;

	char stateORE[DEF_ORELEFT_MAXSIZE];

};



#endif


