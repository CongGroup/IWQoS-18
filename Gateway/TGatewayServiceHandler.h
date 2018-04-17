#ifndef __TGATEWAY_SERVICE_HANDLER_H__
#define __TGATEWAY_SERVICE_HANDLER_H__

#include <string>
#include <cstring>
#include <cstdint>
#include <time.h>

#include "../Caravel/RedisHelper.h"
#include "../Caravel/ThriftAdapt.h"
#include "../Caravel/TimeDiff.h"
#include "../FastORE/OREHelper.h"
#include "../Caravel/AES.h"

#include "TGatewayService.h"

#include "config.h"

using namespace std;
using namespace caravel;
using namespace GatewayServer;


class TGatewayServiceHandler : virtual public TGatewayServiceIf {
public:
	TGatewayServiceHandler();
	~TGatewayServiceHandler();

	/**
	* Increase the Counter API
	* @return Value as Binary
	*
	* @param RuleID
	*/
	void IncCounter(const string& RuleID);

	bool Init(int argc, char **argv);

private:

	RedisHelper redisHelper;


};


#endif


