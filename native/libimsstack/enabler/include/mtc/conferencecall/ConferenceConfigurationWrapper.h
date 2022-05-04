#ifndef CONFERENCE_CONFIGURATION_WRAPPER_H_
#define CONFERENCE_CONFIGURATION_WRAPPER_H_

#include "IMSTypeDef.h"

// TODO: this class will be deprecated and replaced by the MtcConfigurationProxy
class ConferenceConfigurationWrapper
{
public:
    static IMS_BOOL IsSupported();
    static IMS_BOOL IsConferenceSubscriptionRequired();
    // LGU+ doesn't use Refer-sub.
    static IMS_BOOL IsReferSubscriptionRequired();
    static IMS_BOOL IsSubscriptionOutDialog();

    // SKT always set it 1.
    static IMS_BOOL IsPackageVersionCheckRequired();
    static IMS_BOOL IsSubscriptionFirst();
    static IMS_BOOL IsPaidPreferred();
    static IMS_BOOL IsReUseReferToUri();
    static IMS_BOOL IsReferUsed();
    // SKT always receive disconnecting status when participant leaves conference call.
    static IMS_BOOL IsDisconnectingStatusUsed();
    static IMS_BOOL IsReferToExHeaderUsed();

    static IMS_BOOL IsSubscriptionForParticipantRequired();

    // timer value. (-1) : permanent. (0) : not wait
    static IMS_SINT32 GetWaitTimeInitiation();  // 12s
    static IMS_SINT32 GetWaitTimeNotifyActive();
    static IMS_SINT32 GetWaitTimeNotifyTerminated();  // 3s
    static IMS_SINT32 GetWaitTimeSipFrag();           // 2s

    static IMS_SINT32 GetReferTypeForInvite();
};

#endif
