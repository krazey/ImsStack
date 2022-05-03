#ifndef MTC_SESSION_H_
#define MTC_SESSION_H_

#include "IMSList.h"
#include "IMSTypeDef.h"
#include "call/IMtcSessionContext.h"
#include "call/message/MessageSender.h"
#include "call/extension/MtcExtensionSet.h"
#include "CallInfo.h"

class IMtcExtensionSet;
class ISession;
class MtcSipInterfaceFactory;

class MtcSession final : public IMtcSessionContext
{
public:
    MtcSession(IN IMtcCallContext& objContext, IN ISession& objSession);
    virtual ~MtcSession();
    MtcSession(IN const MtcSession&) = delete;
    MtcSession& operator=(IN const MtcSession&) = delete;

    inline ISession& GetISession() override { return m_objSession; }
    inline MessageSender& GetMessageSender() override { return m_objMessageSender; }
    inline MtcExtensionSet& GetExtensionSet() override { return m_objExtensionSet; };

    inline IMS_UINTP GetCallKey() const override { return m_objContext.GetCallKey(); }
    inline IMS_BOOL IsEct() const override { return m_objContext.IsEct(); }
    inline IMS_BOOL IsHeldByMe() const override { return m_objContext.IsHeldByMe(); }
    inline CallInfo& GetCallInfo() override { return m_objContext.GetCallInfo(); }
    inline ParticipantInfo& GetParticipantInfo() override
    {
        return m_objContext.GetParticipantInfo();
    }
    inline MtcSession* GetSession() override { return m_objContext.GetSession(); }
    inline IMtcService& GetService() override { return m_objContext.GetService(); }
    inline MtcUiNotifier& GetUiNotifier() override { return m_objContext.GetUiNotifier(); }
    inline IMtcMediaManager& GetMediaManager() override { return m_objContext.GetMediaManager(); }
    inline IMtcPreconditionManager& GetPreconditionManager() override
    {
        return m_objContext.GetPreconditionManager();
    }
    inline UpdatingInfo& GetUpdatingInfo() override { return m_objContext.GetUpdatingInfo(); }
    inline MtcSession* CreateSession(IN ISession& objSession) override
    {
        return m_objContext.CreateSession(objSession);
    }
    inline IMtcBlockChecker* CreateBlockChecker(IN const IMSList<IMtcBlockRule*>& lstRules) override
    {
        return m_objContext.CreateBlockChecker(lstRules);
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

    inline void SetSession(IN MtcSession* pSession) override { m_objContext.SetSession(pSession); }
    inline void SetHeldByMe(IN IMS_BOOL bHeldByMe) override { m_objContext.SetHeldByMe(bHeldByMe); }

private:
    IMtcCallContext& m_objContext;
    ISession& m_objSession;

    MessageSender m_objMessageSender;
    MtcExtensionSet m_objExtensionSet;

    IMSList<AString> GetSupportedOptionTags() const;
};

#endif
