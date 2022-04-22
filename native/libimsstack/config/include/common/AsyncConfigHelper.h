/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20101022  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _ASYNC_CONFIG_HELPER_H_
#define _ASYNC_CONFIG_HELPER_H_

#include "ImsMessageDef.h"
#include "IMSActivityEx.h"

class IAsyncConfig;

class AsyncConfigHelper
    : public IMSActivityEx
{
public:
    AsyncConfigHelper();
    virtual ~AsyncConfigHelper();

public:
    void Register(IN IAsyncConfig *piConfig);
    IMS_BOOL SendTo(IN IAsyncConfig *piConfig, IN IMS_SINT32 nMSG,
            IN IMS_SINTP nParam1, IN IMS_SINTP nParam2);
    void Unregister(IN IAsyncConfig *piConfig);

private:
    // IMSActivityEx class
    virtual IMS_BOOL OnMessage(IN IMSMSG &objMSG);

    IMS_BOOL IsRegisteredConfig(IN IAsyncConfig *piConfig);

private:
    // This message will be used in the first argument in SendTo(...) method
    enum
    {
        AMSG_START = (IMS_MSG_USER + 1),
        // MSG base of configuration
        AMSG_SEND_TO = (IMS_MSG_USER + 11)
    };

    IMSList<IAsyncConfig*> objAsyncConfigs;
};

#endif // _ASYNC_CONFIG_HELPER_H_
