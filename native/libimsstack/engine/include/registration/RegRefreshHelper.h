/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090904  toastops@                 Created
    </table>

    Description

*/

#ifndef _REGISTRATION_REFRESH_HELPER_H_
#define _REGISTRATION_REFRESH_HELPER_H_

#include "SipAddress.h"
#include "util/RefreshHelper.h"

class ISipConnection;

class RegRefreshHelper : public RefreshHelper
{
public:
    explicit RegRefreshHelper(IN IRefreshable* piRefreshable_);
    virtual ~RegRefreshHelper();

public:
    virtual IMS_BOOL AddSpecificHeader(IN ISipConnection* piSC);
    virtual IMS_RESULT SendRefreshRequest(IN ISipClientConnection* piSCC);
    virtual IMS_RESULT UpdateOnMessageReceived(IN CONST ISipConnection* piSC);
    virtual IMS_RESULT UpdateOnMessageSent(IN CONST ISipConnection* piSC);

    const SipAddress& GetContactAddress() const;
    void SetContactAddress(IN CONST SipAddress& objContactAddress);
    IMS_BOOL UpdateRefreshTimer(IN IMS_SINT32 nDuration);

protected:
    virtual void RefreshCompleted(IN ISipClientConnection* piSCC, IN IMS_SINT32 nCode = 0);
    virtual void RefreshStarted();
    virtual void RefreshTerminated();

private:
    SipAddress objContactAddress;
};

#endif  // _REGISTRATION_REFRESH_HELPER_H_
