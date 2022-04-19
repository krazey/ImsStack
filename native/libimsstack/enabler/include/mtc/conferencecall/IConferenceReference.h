#ifndef INTERFACE_CONFERENCE_REFERENCE_H_
#define INTERFACE_CONFERENCE_REFERENCE_H_

#include "IMSTypeDef.h"

class CallConnectionIdManager;

// Subscription-State value of R-NOTIFY
enum class ReferSubscriptionState
{
    INVALID,
    ACTIVE,
    TERMINATED,
};

class IConferenceReference
{
public:
    virtual ~IConferenceReference() {}
    virtual IMS_RESULT SendInvite(OUT AString& strReferToUri,
            IN CallConnectionIdManager& objConnectionIdManager) = 0;
    virtual IMS_RESULT SendBye(IN AString strInvitedUri = AString::ConstEmpty()) = 0;
    virtual IMS_UINT32 GetType() const = 0;
    virtual IMS_UINT32 GetResponseCode() const = 0;
    virtual void SetForceToTerminateInterface(IN IMS_BOOL bTerminate) = 0;
};

#endif
