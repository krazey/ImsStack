#ifndef MTC_SESSION_H_
#define MTC_SESSION_H_

#include "IMSList.h"
#include "IMSTypeDef.h"
#include "call/IMtcCall.h"
#include "call/IMtcSessionContext.h"
#include "call/message/MessageSender.h"
#include "call/extension/MtcExtensionSet.h"

class IMessage;
class IMtcExtensionSet;
class ISession;
class MtcSipInterfaceFactory;

class MtcSession final : public IMtcSessionContext
{
public:
    explicit MtcSession(
            IN IMtcCallContext& objContext, IN ISession& objSession, IN CallType eCallType);
    virtual ~MtcSession();
    MtcSession(IN const MtcSession&) = delete;
    MtcSession& operator=(IN const MtcSession&) = delete;

    IMS_RESULT Start();
    IMS_RESULT Terminate(IMS_BOOL bUseBye, IN const CallReasonInfo& objReason);

    void HandleRequest(IN IMS_UINT32 nMethod, IN const IMessage& objRequest);
    void HandleResponse(IN IMS_UINT32 nMethod, IN const IMessage& objResponse);

    inline void SetCallType(IN CallType eCallType) { m_eCallType = eCallType; }  // TODO: tmp method

    inline CallType GetCallType() const override { return m_eCallType; }
    inline IMS_BOOL IsVideoCapable() const override { return m_bVideoCapable; }
    inline IMS_BOOL IsRttCapable() const override { return m_bRttCapable; }
    inline ISession& GetISession() override { return m_objSession; }
    inline MessageSender& GetMessageSender() override { return m_objMessageSender; }
    inline MtcExtensionSet& GetExtensionSet() override { return m_objExtensionSet; }

    inline IMS_UINTP GetCallKey() const override { return m_objContext.GetCallKey(); }
    inline IMS_BOOL IsHeldByMe() const override { return m_objContext.IsHeldByMe(); }
    inline IMS_BOOL IsUssi() const override { return m_objContext.IsUssi(); }
    inline CallInfo& GetCallInfo() override { return m_objContext.GetCallInfo(); }
    inline ParticipantInfo& GetParticipantInfo() override
    {
        return m_objContext.GetParticipantInfo();
    }
    inline MtcSession* GetSession(IN const ISession* piSession) override
    {
        return m_objContext.GetSession(piSession);
    }
    inline MtcSession* GetSession() override { return m_objContext.GetSession(); }
    inline IMtcService& GetService() override { return m_objContext.GetService(); }
    inline MtcUiNotifier& GetUiNotifier() override { return m_objContext.GetUiNotifier(); }
    inline IMtcMediaManager& GetMediaManager() override { return m_objContext.GetMediaManager(); }
    inline IMtcPreconditionManager& GetPreconditionManager() override
    {
        return m_objContext.GetPreconditionManager();
    }
    inline UssiController* GetUssiController() override { return m_objContext.GetUssiController(); }
    inline UpdatingInfo& GetUpdatingInfo() override { return m_objContext.GetUpdatingInfo(); }
    MtcSession* CreateSession(IN ISession* piSession) override
    {
        return m_objContext.CreateSession(piSession);
    }
    MtcSession* CreateSession() override { return m_objContext.CreateSession(); }
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
    inline MtcDialingPlan& GetDialingPlan() override { return m_objContext.GetDialingPlan(); }
    inline IMtcService* GetServiceByType(IN ServiceType eServiceType) override
    {
        return m_objContext.GetServiceByType(eServiceType);
    }
    inline IMtcCallManager& GetCallManager() override { return m_objContext.GetCallManager(); }
    inline MtcCallController& GetCallController() override
    {
        return m_objContext.GetCallController();
    }
    inline MtcVonrManager& GetVonrManager() override { return m_objContext.GetVonrManager(); }
    inline MtcConfigurationProxy& GetConfigurationProxy() override
    {
        return m_objContext.GetConfigurationProxy();
    }
    inline CallStateProxy& GetCallStateProxy() override { return m_objContext.GetCallStateProxy(); }
    inline MtcImsEventReceiver& GetImsEventReceiver() override
    {
        return m_objContext.GetImsEventReceiver();
    }
    inline MtcAosConnector* GetAosConnector(IN ServiceType eServiceType) override
    {
        return m_objContext.GetAosConnector(eServiceType);
    }
    inline MtcSipInterfaceFactory& GetSipInterfaceFactory() override
    {
        return m_objContext.GetSipInterfaceFactory();
    }
    inline ConferenceManager& GetConferenceManager() override
    {
        return m_objContext.GetConferenceManager();
    }
    inline EctManager* GetEctManager() override { return m_objContext.GetEctManager(); }
    inline MtcEmergencyServiceManager* GetEmergencyServiceManager() override
    {
        return m_objContext.GetEmergencyServiceManager();
    }

    inline void SetHeldByMe(IN IMS_BOOL bHeldByMe) override { m_objContext.SetHeldByMe(bHeldByMe); }

private:
    IMSList<AString> GetSupportedOptionTags() const;
    void UpdateSessionProperty();
    void UpdateCallTypeFromMessage(IN const IMessage& objMessage);
    void UpdateCapabilityFromMessage(IN const IMessage& objMessage);
    void UpdateSessionIdFromMessage(IN const IMessage& objMessage);

    AString GenerateSessionId() const;
    IMS_BOOL IsRegisteredFeature(IMS_UINT32 nFeature);

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
