#ifndef MTS_MESSAGE_CONTROLLER_H_
#define MTS_MESSAGE_CONTROLLER_H_

#include "IMSActivityEx.h"
#include "ICoreService.h"
#include "interface/IMtsService.h"
#include "IUMts.h"

class IMtsMessage;
class IMtsMessageControllerListener;
class IPageMessage;
class MtsDynamicLoader;
class MtsSmUtils;

class MtsMessageController final :
        public IMSActivityEx
{
public:
    MtsMessageController(IN IMS_SINT32 nSlotID, MtsDynamicLoader* pMtsDynamicLoader);
    ~MtsMessageController();

    inline void RegisterService(IMtsService* piMtsService) { m_piMtsService = piMtsService; }
    inline void UnRegisterService() { m_piMtsService = IMS_NULL; }
    void DestroyMtsMessage();

    void Add(IN IMtsMessage* piMtsMessage);
    void Remove(IN IMtsMessage* piMtsMessage);
    IMtsMessage* Search(IN const AString& strDestination);
    IMtsMessage* Search(IN const AString& strDestination, IN IMS_SINT32 nMti);
    IMtsMessage* Search(
            IN IMS_SINT32 nMessageReference, IN IMS_SINT32 nMessageType = MESSAGE_TYPE_RECEIVE);

    void RegisterNoTransactionListener(IN IMtsMessageControllerListener* piListener);
    void DeregisterNoTransactionListener(IN IMtsMessageControllerListener* piListener);
    IMS_BOOL HasMessageSendingReceiving();
    void TerminateAllPendingMessages(IN IMS_BOOL bIs1xCallTerm);
    void TerminateAllPendingMessagesEx(IN IMS_UINT32 nReason);

    const AString& GetLastIpsmgwAddr();
    void SetLastIpsmgwAddr(IN const AString& strSmgwAddr);

    void SendMtsMessage(IN IUSendSmsRequestParam* pSendParam, IN IMS_BOOL bIsSmsEServiceType);
    void ReceiveMtsMessage(IN IPageMessage* piPageMessage,  IN IMS_BOOL bIsSmsEServiceType);

    IMS_BOOL IsDeliverMessage(IN IPageMessage* piPageMessage);
    ICoreService* GetICoreService();
    MtsDynamicLoader* GetMtsUtils();

    void SetCallStateType(IN IMS_UINT32 nType, IN IMS_UINT32 nState);
    IMS_BOOL IsEmergencyCalling();

protected:
    // IMSActivityEx
    IMS_BOOL OnMessage(IN IMSMSG &objMSG);

private:
    void UpdateRPAckMap(IN IPageMessage* piPageMessage);

public:
    enum
    {
        MESSAGE_TYPE_RECEIVE = 0,
        MESSAGE_TYPE_SEND    = 1,
    };

    enum
    {
        TYPE_CS = 0,
        TYPE_NORMAL,
        TYPE_EMERGENCY
    };

    enum
    {
        STATE_IDLE = 0,
        STATE_TERMINATING = 1,
        STATE_RINGBACK = 2,
        STATE_RINGING = 3,
        STATE_ALERTING = 4,
        STATE_OFFHOOK = 5
    };

protected:
    IMS_SINT32                      m_nSlotId;

public:
    IMS_BOOL                        m_bProcessingMsg;
    IMS_UINT32                      m_nCallTypeMsg;
    IMS_UINT32                      m_nCallStateMsg;

private:
    AString                         m_strLastRcvIpsmgwAddr;
    IMSList<IMtsMessage*>           m_objMsgList;
    IMtsMessageControllerListener*  m_piMtsMessageControllerListener;
    IMSList<IMtsMessage*>           m_objRPAckedMsgs;
    IMtsService*                    m_piMtsService;

protected:
    MtsSmUtils*                     m_pMtsSmUtils;
    MtsDynamicLoader*               m_pMtsDynamicLoader;
};

#endif
