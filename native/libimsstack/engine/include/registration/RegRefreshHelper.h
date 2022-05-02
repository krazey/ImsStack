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

class ISIPConnection;



class RegRefreshHelper
    : public RefreshHelper
{
public:
    explicit RegRefreshHelper(IN IRefreshable *piRefreshable_);
    virtual ~RegRefreshHelper();

public:
    virtual IMS_BOOL AddSpecificHeader(IN ISIPConnection *piSC);
    virtual IMS_RESULT SendRefreshRequest(IN ISIPClientConnection *piSCC);
    virtual IMS_RESULT UpdateOnMessageReceived(IN CONST ISIPConnection *piSC);
    virtual IMS_RESULT UpdateOnMessageSent(IN CONST ISIPConnection *piSC);

    const SIPAddress& GetContactAddress() const;
    void SetContactAddress(IN CONST SIPAddress &objContactAddress);
    IMS_BOOL UpdateRefreshTimer(IN IMS_SINT32 nDuration);

protected:
    virtual void RefreshCompleted(IN ISIPClientConnection *piSCC, IN IMS_SINT32 nCode = 0);
    virtual void RefreshStarted();
    virtual void RefreshTerminated();

private:
    SIPAddress objContactAddress;
};

#endif // _REGISTRATION_REFRESH_HELPER_H_
