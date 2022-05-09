#ifndef CARRIER_CONFIG_ITEMS_H_
#define CARRIER_CONFIG_ITEMS_H_

#include "IMSTypeDef.h"
#include "AString.h"
#include "IMSVector.h"

struct CarrierConfigItems
{
public:
    CarrierConfigItems() :
            nRequestUriType(0),
            bSupportSipSessionIdHeader(IMS_FALSE),
            bIncludeCallerIdServiceCodesInSipInvite(IMS_FALSE),
            bMultiendpointSupported(IMS_FALSE),
            bSessionTimerSupported(IMS_TRUE),
            nSessionPrivacyType(0),
            bPrackSupportedFor18x(IMS_TRUE),
            nConferenceSubscribeType(1),
            bVoiceQosPreconditionSupported(IMS_TRUE),
            bVoiceOnDefaultBearerSupported(IMS_FALSE),
            nDedicatedBearerWaitTimer(8000),
            objSrvccTypes(IMSVector<IMS_SINT32>()),
            nRingingTimer(90000),
            nRingbackTimer(90000),
            strConferenceFactoryUri(AString::ConstEmpty()),
            bOipSourceFromHeader(IMS_FALSE),
            nMoCallRequestTimeout(5000),
            n18xTimer(32000),
            bSupportConferenceReferSubscribe(IMS_TRUE),
            bEnableConferenceSubscribeByParticipant(IMS_FALSE),
            nConferenceSipFlowOrder(1),
            nConferenceInvitingReferType(1),
            nPolicyQosPreconditionMechanismWhileCallModification(1),
            nIncomingCallRejectCodeForUserDecline(486),
            nIncomingCallRejectCodeForNoAnswer(486),
            nPrackUpdateResponseWaitTimer(3000),
            nSessionRefreshTriggerInterval(0),
            nRegistrationRestorationModeOn504ForInvite(1),
            nPolicyOnAudioQosDeactivation(0),
            bEnableSendReinviteOnRatChange(IMS_FALSE),
            nPolicyForMediaTypeRestrictionOnCellular(0),
            nPolicyForMediaTypeRestrictionOnCellularInRoaming(0),
            nPolicyOfLocalNumbers(2),
            bDefaultEpsBearerContextUsageRestrictionOnCellular(IMS_TRUE),
            nSilentRedialInterval(0),
            nCallTypeAfterAudioAndVideoCallMerged(1),
            objShortCallCodes(IMSVector<IMS_SINT32>()),
            bValidateVerstatFeatureInRegistrationToCheckNetworkCapability(IMS_FALSE),
            bAllowMultipleCallIncludingVideoCall(IMS_TRUE),
            objRejectCodeForCsfbs(IMSVector<IMS_SINT32>()),
            nSilentRedialMaxRetryCount(0),
            nPolicyFor403ResponseForInvite(1),
            nPolicyForCheckingQosWhileCallUpgrading(0),
            bRejectOfferlessInvite(IMS_FALSE),
            objCallTerminateReasonHeaders(IMSVector<AString>()),
            objCallRejectReasonPhrases(IMSVector<AString>()),
            bVideoOnDefaultBearerSupported(IMS_FALSE),
            bVideoQosPreconditionSupported(IMS_TRUE),
            nConvertRemoteResponseTimer(20000),
            nConvertUserResponseTimer(20000),
            nPolicyOnVideoQosDeactivation(2),
            bSupportEarlySession(IMS_FALSE),
            bAllowTextWithVideo(IMS_FALSE),
            nMinimumBatteryLevelForLimitVideoCall(0),
            bSupportVideoTextFeatureInContactHeaderSimultaneously(IMS_TRUE),
            bTextOnDefaultBearerSupported(IMS_FALSE),
            bTextQosPreconditionSupported(IMS_TRUE),
            nPolicyOnTextQosDeactivation(2),
            objPidfShortCodes(IMSVector<AString>()),
            bEmergencyCallOverEmergencyPdn(IMS_FALSE),  // wifi
            nCountryCode(0),
            bRetryEmergencyOnImsPdnBool(IMS_FALSE),
            bEmergencyQosPreconditionSupported(IMS_TRUE),
            bEmergencyCallOverEmergencyPdnOnCellular(IMS_TRUE),
            bEmergencyRetryWithoutChecking380ContentForNonUeDetectableEmergencyCall(IMS_FALSE),
            nEmergencyTCallTimer(10000),
            nEmergencyRingbackTimer(10000),
            nEmergency18xTimer(20000),
            nPolicyForEemergencyUrnEscvMapping(0)
    {
    }
    ~CarrierConfigItems()
    {
        objSrvccTypes.Clear();
        objShortCallCodes.Clear();
        objRejectCodeForCsfbs.Clear();
        objCallTerminateReasonHeaders.Clear();
        objCallRejectReasonPhrases.Clear();
        objPidfShortCodes.Clear();
    }

    CarrierConfigItems(IN const CarrierConfigItems&) = delete;             // not planed
    CarrierConfigItems& operator=(IN const CarrierConfigItems&) = delete;  // not planed

public:
    // ims configurations
    IMS_SINT32 nRequestUriType;
    IMS_BOOL bSupportSipSessionIdHeader;

    // audio configurations
    IMS_BOOL bIncludeCallerIdServiceCodesInSipInvite;
    IMS_BOOL bMultiendpointSupported;
    IMS_BOOL bSessionTimerSupported;
    IMS_SINT32 nSessionPrivacyType;
    IMS_BOOL bPrackSupportedFor18x;
    IMS_SINT32 nConferenceSubscribeType;
    IMS_BOOL bVoiceQosPreconditionSupported;
    IMS_BOOL bVoiceOnDefaultBearerSupported;
    IMS_SINT32 nDedicatedBearerWaitTimer;
    IMSVector<IMS_SINT32> objSrvccTypes;
    IMS_SINT32 nRingingTimer;
    IMS_SINT32 nRingbackTimer;
    AString strConferenceFactoryUri;
    IMS_BOOL bOipSourceFromHeader;
    IMS_SINT32 nMoCallRequestTimeout;
    IMS_SINT32 n18xTimer;
    IMS_BOOL bSupportConferenceReferSubscribe;
    IMS_BOOL bEnableConferenceSubscribeByParticipant;
    IMS_SINT32 nConferenceSipFlowOrder;
    IMS_SINT32 nConferenceInvitingReferType;
    IMS_SINT32 nPolicyQosPreconditionMechanismWhileCallModification;
    IMS_SINT32 nIncomingCallRejectCodeForUserDecline;
    IMS_SINT32 nIncomingCallRejectCodeForNoAnswer;
    IMS_SINT32 nPrackUpdateResponseWaitTimer;
    IMS_SINT32 nSessionRefreshTriggerInterval;
    IMS_SINT32 nRegistrationRestorationModeOn504ForInvite;
    IMS_SINT32 nPolicyOnAudioQosDeactivation;
    IMS_BOOL bEnableSendReinviteOnRatChange;
    IMS_SINT32 nPolicyForMediaTypeRestrictionOnCellular;
    IMS_SINT32 nPolicyForMediaTypeRestrictionOnCellularInRoaming;
    IMS_SINT32 nPolicyOfLocalNumbers;
    IMS_BOOL bDefaultEpsBearerContextUsageRestrictionOnCellular;
    IMS_SINT32 nSilentRedialInterval;
    IMS_SINT32 nCallTypeAfterAudioAndVideoCallMerged;
    IMSVector<IMS_SINT32> objShortCallCodes;
    IMS_BOOL bValidateVerstatFeatureInRegistrationToCheckNetworkCapability;
    IMS_BOOL bAllowMultipleCallIncludingVideoCall;
    IMSVector<IMS_SINT32> objRejectCodeForCsfbs;
    IMS_SINT32 nSilentRedialMaxRetryCount;
    IMS_SINT32 nPolicyFor403ResponseForInvite;
    IMS_SINT32 nPolicyForCheckingQosWhileCallUpgrading;
    IMS_BOOL bRejectOfferlessInvite;
    IMSVector<AString> objCallTerminateReasonHeaders;
    IMSVector<AString> objCallRejectReasonPhrases;

    // vt configurations
    IMS_BOOL bVideoOnDefaultBearerSupported;
    IMS_BOOL bVideoQosPreconditionSupported;
    IMS_SINT32 nConvertRemoteResponseTimer;
    IMS_SINT32 nConvertUserResponseTimer;
    IMS_SINT32 nPolicyOnVideoQosDeactivation;
    IMS_BOOL bSupportEarlySession;
    IMS_BOOL bAllowTextWithVideo;
    IMS_SINT32 nMinimumBatteryLevelForLimitVideoCall;
    IMS_BOOL bSupportVideoTextFeatureInContactHeaderSimultaneously;

    // rtt configurations
    IMS_BOOL bTextOnDefaultBearerSupported;
    IMS_BOOL bTextQosPreconditionSupported;
    IMS_SINT32 nPolicyOnTextQosDeactivation;

    // wfc configurations
    IMSVector<AString> objPidfShortCodes;
    IMS_BOOL bEmergencyCallOverEmergencyPdn;
    IMS_SINT32 nCountryCode;

    // emergency configurations
    IMS_BOOL bRetryEmergencyOnImsPdnBool;
    IMS_BOOL bEmergencyQosPreconditionSupported;
    IMS_BOOL bEmergencyCallOverEmergencyPdnOnCellular;
    IMS_BOOL bEmergencyRetryWithoutChecking380ContentForNonUeDetectableEmergencyCall;
    IMS_SINT32 nEmergencyTCallTimer;
    IMS_SINT32 nEmergencyRingbackTimer;
    IMS_SINT32 nEmergency18xTimer;
    IMS_SINT32 nPolicyForEemergencyUrnEscvMapping;
};

#endif
