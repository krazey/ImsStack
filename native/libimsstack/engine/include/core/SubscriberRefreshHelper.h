/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100330  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _SUBSCRIBER_REFRESH_HELPER_H_
#define _SUBSCRIBER_REFRESH_HELPER_H_

#include "util/RefreshHelper.h"

class SubState;

class SubscriberRefreshHelper : public RefreshHelper
{
public:
    SubscriberRefreshHelper(IN IRefreshable* piRefreshable_, IN CONST SubState* pSubState_);
    virtual ~SubscriberRefreshHelper();

public:
    virtual IMS_BOOL AddSpecificHeader(IN ISipConnection* piSC);
    virtual IMS_RESULT SendRefreshRequest(IN ISipClientConnection* piSCC);
    virtual IMS_RESULT UpdateOnMessageReceived(IN CONST ISipConnection* piSC);
    virtual IMS_RESULT UpdateOnMessageSent(IN CONST ISipConnection* piSC);

protected:
    virtual void RefreshCompleted(IN ISipClientConnection* piSCC, IN IMS_SINT32 nCode = 0);
    virtual void RefreshStarted();
    virtual void RefreshTerminated();

private:
public:
private:
    const SubState* pSubState;
};

#endif  // _SUBSCRIBER_REFRESH_HELPER_H_
