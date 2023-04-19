/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MTC_SESSION_H_
#define MTC_SESSION_H_

#include "ImsList.h"
#include "ImsTypeDef.h"
#include "call/IMtcCall.h"
#include "call/IMtcSession.h"
#include "call/extension/MtcExtensionSet.h"
#include "call/message/IMessageSender.h"

class IMessage;
class IConferenceManager;
class IEctManager;
class IMtcAosConnector;
class IMtcCallContext;
class IMtcCallController;
class IMtcCallManager;
class IMtcDialingPlan;
class IMtcExtensionSet;
class IMtcMediaManager;
class IMtcPreconditionManager;
class IMtcService;
class IMtcSipInterfaceFactory;
class ISession;

class MtcSession final : public IMtcSession
{
public:
    explicit MtcSession(IN IMtcCallContext& objContext, IN ISession& objSession,
            IN CallType eCallType, IN IMessageSender* pMessageSender);
    virtual ~MtcSession();
    MtcSession(IN const MtcSession&) = delete;
    MtcSession& operator=(IN const MtcSession&) = delete;

    IMS_RESULT Start() override;
    IMS_RESULT SendProvisionalResponse(IN IMS_BOOL bUserAlert) override;
    IMS_RESULT SendPrack(IN IMS_BOOL bAllowReOffer) override;
    IMS_RESULT RespondToPrack(IN IMS_SINT32 eStatusCode) override;
    IMS_RESULT SendEarlyUpdate(IN UpdateType eUpdateType) override;
    IMS_RESULT RespondToEarlyUpdate(IN IMS_SINT32 eStatusCode) override;
    IMS_RESULT SendAck() override;
    IMS_RESULT Accept() override;
    IMS_RESULT Reject(IN const CallReasonInfo& objReason) override;
    IMS_RESULT Update(IN UpdateType eUpdateType, IN IMS_BOOL bIncludeAlertInfo,
            IN IMS_SINT32 eMethod) override;
    IMS_RESULT AcceptUpdate() override;
    IMS_RESULT CancelUpdate(IN const CallReasonInfo& objReason) override;
    IMS_RESULT Terminate(IMS_BOOL bUseBye, IN const CallReasonInfo& objReason) override;

    void HandleRequest(IN RequestType eType, IN const IMessage& objRequest) override;
    void HandleResponse(IN ResponseType eType, IN const IMessage& objResponse) override;

    void SetCallType(IN CallType eNewCallType) override;
    inline CallType GetCallType() const override { return m_eCallType; }
    inline CallType GetPreviousCallType() const override { return m_ePreviousCallType; }
    inline ISession& GetISession() override { return m_objSession; }
    inline MtcExtensionSet& GetExtensionSet() override { return m_objExtensionSet; }
    inline IMS_BOOL IsVideoCapable() const override { return m_bVideoCapable; }
    inline IMS_BOOL IsRttCapable() const override { return m_bRttCapable; }
    inline UpdateType GetOngoingUpdateType() const override { return m_eOngoingUpdateType; }

private:
    enum class ResultSetSdp
    {
        NO_SDP,
        FAILURE,
        SUCCESS
    };

    ImsList<IMtcExtension*> GetSupportedExtensions() const;

    void UpdateSessionProperty();
    void UpdateCallTypeFromMessage(IN const IMessage& objMessage, IN IMS_BOOL bSkipSameType);
    void UpdateCapabilityFromMessage(IN const IMessage& objMessage);
    void SetInConference(IN const IMessage& objMessage);
    void CheckCallTypeWithRegisteredFeature();
    ResultSetSdp SetSdpToSend(
            IN IMS_BOOL bAllowReOffer, IN IMS_BOOL bAnswerForOfferlessReInvite = IMS_FALSE);

    IMS_BOOL IsRegisteredFeature(IMS_UINT32 nFeature);
    IMS_BOOL IsCallWaiting() const;
    IMS_BOOL IsNeedToReliable(IN IMS_BOOL bIncludeSdp) const;
    IMS_BOOL IsNeedToRemoveSdpInPr() const;

    IMtcCallContext& m_objContext;
    ISession& m_objSession;

    IMessageSender* m_pMessageSender;
    MtcExtensionSet m_objExtensionSet;

    CallType m_eCallType;
    CallType m_ePreviousCallType;
    IMS_BOOL m_bVideoCapable;
    IMS_BOOL m_bRttCapable;
    IMS_BOOL m_bTerminated;
    UpdateType m_eOngoingUpdateType;
};

#endif
