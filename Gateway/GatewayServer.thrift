/***********************************************************
 * GatewayServer Thrift Script
 * Made By XinyuWang
 * 2017/7/20
 ***********************************************************/

namespace java GatewayServer
namespace cpp GatewayServer
namespace csharp GatewayServer

//"required" and "optional" keywords are purely for documentation.

service TGatewayService {

  /**
   * Increase the Counter API
   * @return Value as Binary
   */
  void IncCounter(
    1: required binary RuleID
  );

}

