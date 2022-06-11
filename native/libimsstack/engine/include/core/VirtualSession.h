/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20201023  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _VIRTUAL_SESSION_H_
#define _VIRTUAL_SESSION_H_

#include "RCObject.h"
#include "ISessionState.h"
#include "Service.h"

class SipAddress;
class ISipMessage;
class SdpOaState;
class SessionDescriptor;
class Media;

class VirtualSession : public RCObject, public ISessionState
{
public:
    explicit VirtualSession(IN Service* pService_, IN const SipAddress* pUserAoR);
    VirtualSession(IN const VirtualSession& objRHS);
    virtual ~VirtualSession();

private:
    VirtualSession& operator=(IN const VirtualSession& objRHS);

public:
    IMS_BOOL CheckNSetSDPBodyPart(IN_OUT ISipMessage*& piSIPMsg);
    IMS_RESULT Notify18xResponse(IN const ISipMessage* piSIPMsg);
    void NotifyPRAckSent(IN const ISipMessage* piSIPMsg);

    Media* CreateMedia(IN const AString& strType, IN IMS_SINT32 nDirection,
            IN IMS_SINT32 nCountOfDescriptor = 0, IN IMS_BOOL bIMSExtension = IMS_TRUE);
    const IMSList<Media*>& GetMedia() const;
    SessionDescriptor* GetSessionDescriptor();
    IMS_SINT32 GetState() const;
    IMS_RESULT RemoveMedia(IN Media* pMedia);
    IMS_RESULT RemoveMedia(IN IMS_UINT32 nIndex);

protected:
    // ISessionState interface
    virtual const AString& GetConnectionAddress() const;
    virtual IMS_SINT32 GetSessionState() const;
    virtual SdpSessionParameter* GetSessionParameter() const;
    virtual const AString& GetPeerConnectionAddress() const;
    virtual SdpSessionParameter* GetPeerSessionParameter() const;
    virtual SdpSessionParameter* GetProposalSessionParameter();

    // Methods for handling SDP & Session Descriptor related operations
    IMS_BOOL CheckNCreateSessionDescriptor();
    IMS_SINT32 GetOfferAnswerState() const;
    IMS_SINT32 HandleSDPOfferAnswer(IN const ISipMessage* piSIPMsg);
    void RestoreEx();
    IMS_BOOL UpdateMedia(IN IMS_SINT32 nTrigger);
    IMS_BOOL RestoreOfferAnswerState();
    IMS_BOOL UpdateOfferAnswerStateOnMessageReceived(IN const ISipMessage* piSIPMsg);
    IMS_BOOL UpdateOfferAnswerStateOnMessageSent(IN const ISipMessage* piSIPMsg);

private:
    void Init(IN const SipAddress* pUserAoR);
    inline IMS_SINT32 GetSlotId() const
    {
        return (pService != IMS_NULL) ? pService->GetSlotId() : IMS_SLOT_ANY;
    }
    void SetState(IN IMS_SINT32 nState);

    // Methods for handling SDP & Media related operations
    IMS_BOOL AddMedia(IN Media* pMedia);
    void CleanupMedia();
    IMS_BOOL UpdateMediaOnAnswerReceived(IN IMS_SINT32 nTrigger);
    IMS_BOOL UpdateMediaOnAnswerSent(IN IMS_SINT32 nTrigger);
    IMS_BOOL UpdateMediaOnOfferReceived(IN IMS_SINT32 nTrigger);
    IMS_BOOL UpdateMediaOnOfferSent(IN IMS_SINT32 nTrigger);

    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

public:
    // Refer to ISession class
    enum
    {
        STATE_CREATED = 0,
        STATE_INITIATED = 1,
        STATE_NEGOTIATING = 2,
        STATE_ESTABLISHING = 3,
        STATE_ESTABLISHED = 4,
        STATE_RENEGOTIATING = 5,
        STATE_REESTABLISHING = 6,
        STATE_TERMINATING = 7,
        STATE_TERMINATED = 8
    };

private:
    Service* pService;
    SipAddress objUserAoR;

    IMS_SINT32 nState;
    SdpOaState* pOAState;
    SessionDescriptor* pSessionDescriptor;
    IMSList<Media*> objMedias;
};

#endif  // _VIRTUAL_SESSION_H_
