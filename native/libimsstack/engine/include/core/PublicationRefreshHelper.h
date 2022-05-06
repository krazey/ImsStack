/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100424  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _PUBLICATION_REFRESH_HELPER_H_
#define _PUBLICATION_REFRESH_HELPER_H_

#include "util/RefreshHelper.h"

class PubState;

class PublicationRefreshHelper : public RefreshHelper
{
public:
    PublicationRefreshHelper(IN IRefreshable* piRefreshable_, IN CONST PubState* pPubState_);
    virtual ~PublicationRefreshHelper();

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
    const PubState* pPubState;
};

#endif  // _PUBLICATION_REFRESH_HELPER_H_
