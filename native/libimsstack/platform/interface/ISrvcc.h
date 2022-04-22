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

class ISRVCCListener
{
public:
    typedef enum
    {
        SRVCC_EVT_START = 0x01,
        SRVCC_EVT_FAILURE,
        SRVCC_EVT_SUCCESS,
        SRVCC_EVT_CANCEL,
        CALLEVENT_NOTUSED,
    } SRVCC_EVENT_ENTYPE;

public:
    virtual void NotifySRVCC(IN ISRVCCListener::SRVCC_EVENT_ENTYPE eEvt) = 0;
};

class ISRVCC
{
public:
    virtual void SubscribeSRVCCListener(IN ISRVCCListener *piListener) = 0;
    virtual void UnsubscribeSRVCCListener(IN ISRVCCListener *piListener) = 0;
};

#endif //_INTERFACE_IMS_SRVCC_H_
