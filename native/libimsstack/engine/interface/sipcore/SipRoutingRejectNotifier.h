/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20130518  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _SIP_ROUTING_REJECT_NOTIFIER_H_
#define _SIP_ROUTING_REJECT_NOTIFIER_H_

#include "IMSList.h"
#include "SipStatusCode.h"
#include "ISipRoutingRejectNotifier.h"

class ISIPMessage;
class ISIPServerConnection;

class SIPRoutingRejectNotifier
    : public ISIPRoutingRejectNotifier
{
public:
    SIPRoutingRejectNotifier();
    virtual ~SIPRoutingRejectNotifier();

private:
    SIPRoutingRejectNotifier(IN const SIPRoutingRejectNotifier& objRHS);
    SIPRoutingRejectNotifier& operator=(IN const SIPRoutingRejectNotifier& objRHS);

public:
    /*
     Checks if the SIP routing reject notification is required or not.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_BOOL                IMS_TRUE / IMS_FALSE
    </table>

    */
    IMS_BOOL IsNotificationRequired() const;

    /*
     Notifies the applications that the incoming SIP request will be rejected by the engine.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    piSIPMsg                SIP message to be rejected
    objStatusCode           Status code which will be used for request reject
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    void NotifyRequestReject(IN ISIPMessage *piSIPMsg,
            IN_OUT SIPStatusCode &objStatusCode);

    /*
     Notifies the applications that the incoming SIP request will be rejected by the engine.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    piSSC                   SIP server connection to be rejected
    objStatusCode           Status code which will be used for request reject
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    void NotifyRequestReject(IN ISIPServerConnection *piSSC,
            IN_OUT SIPStatusCode &objStatusCode);
private:
    // ISIPRoutingRejectNotifier class
    virtual void AddListener(IN ISIPRoutingRejectListener *piListener);
    virtual void RemoveListener(IN ISIPRoutingRejectListener *piListener);

private:
    IMSList<ISIPRoutingRejectListener*> objListeners;
};

#endif // _SIP_ROUTING_REJECT_NOTIFIER_H_
