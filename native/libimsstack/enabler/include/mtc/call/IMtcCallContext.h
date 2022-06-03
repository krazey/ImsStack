#ifndef INTERFACE_MTC_CALL_CONTEXT_H_
#define INTERFACE_MTC_CALL_CONTEXT_H_

#include "IMSList.h"
#include "IMSTypeDef.h"
#include "IMtcCall.h"
#include "IMtcContext.h"
#include "JniCallInfo.h"

class IMtcBlockChecker;
class IMtcBlockRule;
class IMtcMediaManager;
class IMtcPreconditionManager;
class IMtcService;
class ISession;
class ISipClientConnection;
class MtcSession;
class MtcSupplementaryService;
class MtcTimerWrapper;
class MtcUiNotifier;
class ParticipantInfo;
class UpdatingInfo;
class UssiController;

class IMtcCallContext : public IMtcContext
{
public:
    virtual ~IMtcCallContext(){};

    virtual IMS_UINTP GetCallKey() const = 0;
    virtual IMS_BOOL IsHeldByMe() const = 0;
    virtual IMS_BOOL IsUssi() const = 0;

    virtual CallInfo& GetCallInfo() = 0;
    virtual ParticipantInfo& GetParticipantInfo() = 0;
    virtual MtcSession* GetSession() = 0;
    virtual IMtcService& GetService() = 0;
    virtual MtcUiNotifier& GetUiNotifier() = 0;
    virtual IMtcMediaManager& GetMediaManager() = 0;
    virtual IMtcPreconditionManager& GetPreconditionManager() = 0;
    virtual MtcTimerWrapper& GetTimer() = 0;
    virtual MtcSupplementaryService& GetSupplementaryService() = 0;
    virtual UpdatingInfo& GetUpdatingInfo() = 0;
    virtual UssiController* GetUssiController() = 0;

    virtual void SetSession(IN MtcSession* pSession) = 0;
    virtual void SetHeldByMe(IN IMS_BOOL bHeldByMe) = 0;

    virtual MtcSession* CreateSession(IN ISession& objSession, IN CallType eCallType) = 0;
    virtual IMtcBlockChecker* CreateBlockChecker(IN const IMSList<IMtcBlockRule*>& lstRules) = 0;
    virtual JniCallInfo CreateJniCallInfo() = 0;
    virtual ISipClientConnection* CreateClientConnection(IN IMS_SINT32 nMethod) = 0;

    virtual void DeleteUpdatingInfo() = 0;
};

#endif
