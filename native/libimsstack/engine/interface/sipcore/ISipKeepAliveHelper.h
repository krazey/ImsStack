#ifndef _INTERFACE_SIP_KEEP_ALIVE_HELPER_H_
#define _INTERFACE_SIP_KEEP_ALIVE_HELPER_H_

#include "IPAddress.h"
#include "Sip.h"
#include "ISipObject.h"

class ISIPKeepAliveHelperListener;

/**
 * @brief This class provides a helper interface to control SIP keep-alive operations.
 */
class ISIPKeepAliveHelper
    : public ISIPObject
{
public:
    /**
     * @brief Sends the keep-alive packet to the specified destination.
     *
     * @param objPacket Keep-alive packet to be sent
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SendPacket(IN CONST ByteArray &objPacket) = 0;

    /**
     * @brief Sets the keep-alive helper listener to get the pong message from the server.
     *
     * @param piListener Keep-alive helper listener
     */
    virtual void SetListener(IN ISIPKeepAliveHelperListener *piListener) = 0;

     /**
      * @brief Sets the destination transport information to transmit the keep-alive packet.
      *
      * @param objIP Destination IP address
      * @param nPort Destination port number
      */
    virtual void SetTransportTupleD(IN CONST IPAddress &objIP, IN IMS_SINT32 nPort) = 0;

    /**
      * @brief Sets the source transport information to transmit the keep-alive packet.
      *
      * @param objIP Source IP address
      * @param nPort Source port number (In case of TCP with no-ipsec, it MUST be 0)
      * @param nProtocol Transport procotol to be sent\n
      *                  #SIP#TRANSPORT_UDP\n
      *                  #SIP#TRANSPORT_TCP
      */
    virtual void SetTransportTupleS(IN CONST IPAddress &objIP, IN IMS_SINT32 nPort,
            IN IMS_SINT32 nProtocol = SIP::TRANSPORT_UDP) = 0;
};

#endif // _INTERFACE_SIP_KEEP_ALIVE_HELPER_H_
