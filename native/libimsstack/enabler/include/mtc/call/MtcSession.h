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
#include "IMSTypeDef.h"
#include "call/IMtcCall.h"
#include "call/IMtcSession.h"
#include "call/IMtcSessionContext.h"
#include "call/message/MessageSender.h"
#include "call/extension/MtcExtensionSet.h"

class IMessage;
class IConferenceManager;
class IEctManager;
class IMtcAosConnector;
class IMtcCallController;
class IMtcCallManager;
class IMtcContext;
class IMtcDialingPlan;
class IMtcExtensionSet;
class IMtcMediaManager;
class IMtcPreconditionManager;
class IMtcService;
class IMtcSipInterfaceFactory;
class IMtcVonrManager;
class ISession;

class MtcSession final : public IMtcSession, public IMtcSessionContext
{
public:
    explicit MtcSession(
            IN IMtcCallContext& objContext, IN ISession& objSession, IN CallType eCallType);
    virtual ~MtcSession();
    MtcSession(IN const MtcSession&) = delete;
    MtcSession& operator=(IN const MtcSession&) = delete;

    IMS_RESULT Start() override;
    IMS_RESULT SendProvisionalResponse(IN IMS_BOOL bUserAlert) override;
    IMS_RESULT SendPrack() override;
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

    void HandleRequest(IN IMS_UINT32 nMethod, IN const IMessage& objRequest) override;
    void HandleResponse(IN IMS_UINT32 nMethod, IN const IMessage& objResponse) override;

    inline void SetCallType(IN CallType eCallType) override { m_eCallType = eCallType; }
    inline CallType GetCallType() const override { return m_eCallType; }
    inline ISession& GetISession() override { return m_objSession; }
    inline MtcExtensionSet& GetExtensionSet() override { return m_objExtensionSet; }
    inline IMS_BOOL IsVideoCapable() const override { return m_bVideoCapable; }
    inline IMS_BOOL IsRttCapable() const override { return m_bRttCapable; }

    inline IMS_UINTP GetCallKey() const override { return m_objContext.GetCallKey(); }
    inline IMS_BOOL IsHeldByMe() const override { return m_objContext.IsHeldByMe(); }
    inline IMS_BOOL IsUssi() const override { return m_objContext.IsUssi(); }
    inline CallInfo& GetCallInfo() override { return m_objContext.GetCallInfo(); }
    inline ParticipantInfo& GetParticipantInfo() override
    {
        return m_objContext.GetParticipantInfo();
    }
    inline IMtcSession* GetSession(IN const ISession* piSession) const override
    {
        return m_objContext.GetSession(piSession);
    }
    inline IMtcSession* GetSession() const override { return m_objContext.GetSession(); }
    inline IMtcService& GetService() override { return m_objContext.GetService(); }
    inline MtcUiNotifier& GetUiNotifier() override { return m_objContext.GetUiNotifier(); }
    inline IMtcMediaManager& GetMediaManager() override { return m_objContext.GetMediaManager(); }
    inline IMtcPreconditionManager& GetPreconditionManager() override
    {
        return m_objContext.GetPreconditionManager();
    }
    inline UssiController* GetUssiController() override { return m_objContext.GetUssiController(); }
    inline IMSList<IMtcCall*> GetOtherCalls() override { return m_objContext.GetOtherCalls(); }
    inline UpdatingInfo& GetUpdatingInfo() override { return m_objContext.GetUpdatingInfo(); }
    IMtcSession* CreateSession(IN ISession* piSession) override
    {
        return m_objContext.CreateSession(piSession);
    }
    IMtcSession* CreateSession() override { return m_objContext.CreateSession(); }
    inline IMtcBlockChecker* CreateBlockChecker(IN const IMSList<IMtcBlockRule*>& lstRules) override
    {
        return m_objContext.CreateBlockChecker(lstRules);
    }
    inline JniCallInfo CreateJniCallInfo() override { return m_objContext.CreateJniCallInfo(); }
    inline ISipClientConnection* CreateClientConnection(IN IMS_SINT32 nMethod) override
    {
        return m_objContext.CreateClientConnection(nMethod);
    }
    inline void RemoveSession(IN const ISession* piSession) override
    {
        m_objContext.RemoveSession(piSession);
    }
    inline void RemoveInactiveSessions(IN const ISession* piActiveSession) override
    {
        m_objContext.RemoveInactiveSessions(piActiveSession);
    }
    inline void DeleteUpdatingInfo() override { return m_objContext.DeleteUpdatingInfo(); }
    inline MtcTimerWrapper& GetTimer() override { return m_objContext.GetTimer(); }
    inline MtcSupplementaryService& GetSupplementaryService() override
    {
        return m_objContext.GetSupplementaryService();
    }
    inline IMS_SINT32 GetSlotId() override { return m_objContext.GetSlotId(); }
    inline IMtcDialingPlan& GetDialingPlan() override { return m_objContext.GetDialingPlan(); }
    inline IMtcService* GetServiceByType(IN ServiceType eServiceType) override
    {
        return m_objContext.GetServiceByType(eServiceType);
    }
    inline IMtcCallManager& GetCallManager() override { return m_objContext.GetCallManager(); }
    inline IMtcCallController& GetCallController() override
    {
        return m_objContext.GetCallController();
    }
    inline IMtcVonrManager& GetVonrManager() override { return m_objContext.GetVonrManager(); }
    inline MtcConfigurationProxy& GetConfigurationProxy() override
    {
        return m_objContext.GetConfigurationProxy();
    }
    inline ICallStateProxy& GetCallStateProxy() override
    {
        return m_objContext.GetCallStateProxy();
    }
    inline IMtcImsEventReceiver& GetImsEventReceiver() override
    {
        return m_objContext.GetImsEventReceiver();
    }
    inline IMtcAosConnector* GetAosConnector(IN ServiceType eServiceType) override
    {
        return m_objContext.GetAosConnector(eServiceType);
    }
    inline IMtcSipInterfaceFactory& GetSipInterfaceFactory() override
    {
        return m_objContext.GetSipInterfaceFactory();
    }
    inline IConferenceManager& GetConferenceManager() override
    {
        return m_objContext.GetConferenceManager();
    }
    inline IEctManager* GetEctManager() override { return m_objContext.GetEctManager(); }
    inline MtcEmergencyServiceManager* GetEmergencyServiceManager() override
    {
        return m_objContext.GetEmergencyServiceManager();
    }

    inline void SetHeldByMe(IN IMS_BOOL bHeldByMe) override { m_objContext.SetHeldByMe(bHeldByMe); }

private:
    enum class ResultSetSdp
    {
        NO_SDP,
        FAILURE,
        SUCCESS
    };

    ImsList<IMtcExtension*> GetSupportedExtensions() const;

    void UpdateSessionProperty();
    void UpdateCallTypeFromMessage(IN const IMessage& objMessage);
    void UpdateCapabilityFromMessage(IN const IMessage& objMessage);
    void UpdateSessionIdFromMessage(IN const IMessage& objMessage);
    void SetInConference(IN const IMessage& objMessage);
    void CheckCallTypeWithRegisteredFeature();
    ResultSetSdp SetSdpToSend(IN IMS_BOOL bAllowReOffer);

    AString GenerateSessionId() const;
    IMS_BOOL IsRegisteredFeature(IMS_UINT32 nFeature);
    IMS_BOOL IsCallWaiting() const;
    IMS_BOOL IsNeedToReliable(IN IMS_BOOL bIncludeSdp) const;

    IMtcCallContext& m_objContext;
    ISession& m_objSession;

    MessageSender m_objMessageSender;
    MtcExtensionSet m_objExtensionSet;

    CallType m_eCallType;
    IMS_BOOL m_bVideoCapable;
    IMS_BOOL m_bRttCapable;
    IMS_BOOL m_bTerminated;
    AString m_strSessionIdHeader;
};

#endif
