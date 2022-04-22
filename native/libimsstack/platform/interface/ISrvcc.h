/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20130319  changik.jeong@            Created
    </table>

    Description
    This interface gives how to get SRVCC events during VoLTE.
    A module that needs to receive SRVCC events implements ISRVCCListener and then
    it will get NotifySRVCC() invoked when SRVCC move takes place.
*/

#ifndef _INTERFACE_IMS_SRVCC_H_
#define _INTERFACE_IMS_SRVCC_H_

#include "ImsMessageDef.h"
#include "ServiceThread.h"
#include "ServiceMessage.h"

class ISrvccListener;

class ISrvcc
{
public:
    enum SRVCC_EVENT_ENTYPE
    {
        SRVCC_EVENT_START = 1,
        SRVCC_EVENT_FAILURE = 2,
        SRVCC_EVENT_SUCCESS = 3,
        SRVCC_EVENT_CANCEL = 4
    };

public:
    virtual void SubscribeSrvccListener(IN ISrvccListener* piListener) = 0;
    virtual void UnsubscribeSrvccListener(IN ISrvccListener* piListener) = 0;
};

class ISrvccListener
{
public:
    virtual void Srvcc_NotifyEventChanged(IN ISrvcc::SRVCC_EVENT_ENTYPE eEvent) = 0;
};

#endif //_INTERFACE_IMS_SRVCC_H_
