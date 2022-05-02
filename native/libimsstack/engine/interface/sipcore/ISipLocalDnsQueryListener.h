#ifndef _INTERFACE_SIP_LOCAL_DNS_QUERY_LISTENER_H_
#define _INTERFACE_SIP_LOCAL_DNS_QUERY_LISTENER_H_

#include "IPAddress.h"

/**
 * @brief This class provides a listener interface for the local DNS query for a test purpose
 *        or obtaining a real host address by the service enabler.
 */
class ISIPLocalDnsQueryListener
{
public:
    /**
     * @brief Notifies the application that the host name should be resolved
     *        to send a SIP message.
     *
     * If the method returns IMS_FALSE, J180 engine will keep the original procedure
     * to resolve the host name.
     *
     * @param objLocalIP Local IP address
     * @param strHostname Host name to be resolved
     * @param objHostIP Resolved host IP address
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL LocalDnsQuery_GetHostByName(IN CONST IPAddress &objLocalIP,
            IN CONST AString &strHostname, OUT IPAddress &objHostIP) = 0;
};

#endif // _INTERFACE_SIP_LOCAL_DNS_QUERY_LISTENER_H_
