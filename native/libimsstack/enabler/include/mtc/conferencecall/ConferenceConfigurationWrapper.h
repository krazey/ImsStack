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
    static IMS_BOOL IsReferredByRequired();

    static IMS_BOOL IsAddUserParameter();
    // SKT always set it 1.
    static IMS_BOOL IsPackageVersionCheckRequired();
    static IMS_BOOL IsImplicitRoutingRequired();
    static IMS_BOOL IsSubscriptionFirst();
    static IMS_BOOL IsPaidPreferred();
    static IMS_BOOL IsReUseReferToUri();
    static IMS_BOOL IsReferUsed();
    // SKT always receive disconnecting status when participant leaves conference call.
    static IMS_BOOL IsDisconnectingStatusUsed();
    static IMS_BOOL IsReferToExHeaderUsed();

    static IMS_BOOL IsSubscriptionForParticipantRequired();

    // timer value. (-1) : permanent. (0) : not wait
    static IMS_SINT32 GetWaitTimeInitiation(); // 12s
    static IMS_SINT32 GetWaitTimeNotifyActive();
    static IMS_SINT32 GetWaitTimeNotifyTerminated(); // 3s
    static IMS_SINT32 GetWaitTimeSipFrag(); // 2s

    static IMS_SINT32 GetReferTypeForInvite();

private:
    static IMS_BOOL IsConferenceFeatureFlagged(IN IMS_UINT32 nFeature);
public:

private:
    enum
    {
        // Standard-based requirements
        FLAG_CONFERENCE                     = 0x00000001,
        FLAG_CONFERENCE_SUBSCRIPTION        = 0x00000002,
        FLAG_REFER_SUBSCRIPTION             = 0x00000004,
        FLAG_SUBSCRIPTION_OUTDIALOG         = 0x00000008,

        // SIP header control
        FLAG_REFERRED_BY                    = 0x00010000,

        // Operator specific requirements
        FLAG_ADD_USER_PARAMETER             = 0x00100000, // deprecated
        FLAG_SUBSCRIPTION_FOR_PARTICIPANT   = 0X00200000,
        FLAG_PACKAGE_VERSION_CHECK          = 0x01000000,
        FLAG_IMPLICIT_ROUTING               = 0x02000000,
        FLAG_SUBSCRIPTION_FIRST             = 0x04000000,
        FLAG_PAID_PREFERRED                 = 0x08000000,
        FLAG_REUSE_REFER_TO_URI_IN_INVITE   = 0x10000000,
        FLAG_USE_REFER                      = 0x20000000,  // KDDI ONLY
        FLAG_USE_DISCONNECTING_STATUS       = 0x40000000,  // SKT ONLY
        FLAG_USE_REFER_TO_EX_HDR            = 0x80000000
    };
};

#endif
