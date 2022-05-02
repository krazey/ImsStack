#ifndef INTERFACE_CONFERENCE_REFERENCE_LISTENER_H_
#define INTERFACE_CONFERENCE_REFERENCE_LISTENER_H_

#include "SipStatusCode.h"
#include "conferencecall/IConferenceReference.h"

class IConferenceReferenceListener
{
public:
    virtual void OnReferenceStarted(IN IConferenceReference* piConfRef) = 0;
    virtual void OnReferenceStartFailed(IN IConferenceReference* pConfRef) = 0;
    virtual void OnReferenceUpdated(IN IConferenceReference* piConfRef,
            IN SIPStatusCode nSipFragCode, IN ReferSubscriptionState eState) = 0;
};

#endif
