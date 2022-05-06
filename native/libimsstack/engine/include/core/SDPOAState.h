/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090622  toastops@                 Created
    </table>

    Description

*/

#ifndef _SDP_OFFER_ANSWER_STATE_H_
#define _SDP_OFFER_ANSWER_STATE_H_

#include "ISDPOAState.h"
#include "SdpParser.h"
#include "SessionParameter.h"

class ISipMessage;
class Service;
class SessionCapabilities;

class SDPOAState : public ISDPOAState
{
public:
    explicit SDPOAState(IN IMS_BOOL bSDPVersionCheck_ = IMS_TRUE,
            IN IMS_BOOL bAlwaysIncreaseSDPVersion_ = IMS_FALSE);
    SDPOAState(IN CONST SDPOAState& objRHS);
    virtual ~SDPOAState();

private:
    SDPOAState& operator=(IN CONST SDPOAState& objRHS);

public:
    // ISDPOAState interface implementations
    virtual void AbortProposal();
    virtual IMS_SINT32 CreateProposalView();
    virtual IMS_SINT32 GetSessionCurrentView(OUT SdpSessionParameter*& pSessionParam) const;
    virtual IMS_SINT32 GetSessionPeerView(OUT SdpSessionParameter*& pSessionParam) const;
    virtual IMS_SINT32 GetSessionProposalView(OUT SdpSessionParameter*& pSessionParam) const;
    virtual IMS_SINT32 CreateMediaParameter(OUT SdpMediaParameter*& pMediaParam);
    virtual IMS_SINT32 GetMediaCurrentView(
            IN IMS_SINT32 nMid, OUT SdpMediaParameter*& pMediaParam) const;
    virtual IMS_SINT32 GetMediaPeerView(
            IN IMS_SINT32 nMid, OUT SdpMediaParameter*& pMediaParam) const;
    virtual IMS_SINT32 GetMediaProposalView(
            IN IMS_SINT32 nMid, OUT SdpMediaParameter*& pMediaParam) const;
    virtual void MarkRejectedOrRemoved(IN IMS_SINT32 nMid);
    virtual void RemoveMediaParameter(IN IMS_SINT32 nMid);

    IMS_BOOL CreateCapabilities(
            IN Service* pService, IN CONST AString& strUserID, IN IMS_BOOL bMProf = IMS_FALSE);
    const SessionParameter* GetCapabilities() const;
    SessionParameter* GetCurrentView() const;
    SessionParameter* GetPeerView() const;
    SessionParameter* GetProposalView() const;
    IMS_SINT32 GetMode() const;
    IMS_BOOL GetSDP(OUT AString& strSDP) const;
    IMS_SINT32 GetState() const;
    IMS_BOOL InitiateOffer(IN IMS_SINT32 nType);
    IMS_BOOL IsOfferProgress() const;
    IMS_BOOL IsSessionChanged() const;
    IMS_SINT32 HandleOfferAnswer(IN CONST ISipMessage* piSIPMsg);
    void CompleteExchange();
    IMS_BOOL RestoreState();
    IMS_BOOL UpdateState(IN CONST ISipMessage* piSIPMsg, IN IMS_SINT32 nMessageFlow,
            IN IMS_BOOL bIsCallEstablished, IN IMS_BOOL bAllowOAForNonRPR = IMS_FALSE);
    void UpdateStateOnTransactionCompleted(
            IN CONST ISipMessage* piSIPMsg, IN IMS_SINT32 nMessageFlow);

    // REFUSE_SDP_OFFER_ANSWER_EXCHANGE {
    void CreateRefusedView();
    void DestroyRefusedView();
    SessionParameter* GetRefusedView() const;
    // }

private:
    SessionParameter* GetCurrentCapabilities();
    SessionParameter*& GetNewProposalView();
    SessionParameter*& GetNewPeerView();
    IMS_SINT32 HandleAnswer(IN CONST SdpParser& objParser);
    IMS_SINT32 HandleOffer(IN CONST SdpParser& objParser);
    void SetCapabilities();
    void SetProposedView(IN SessionParameter* pSessionParam);
    void SetCurrentView(IN SessionParameter* pSessionParam);
    void SetLastOfferMade(IN SessionParameter* pSessionParam);
    void SetOldState(IN IMS_SINT32 nOldState);
    void SetState(IN IMS_SINT32 nState);

    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

public:
    // Type of state
    enum
    {
        STATE_INVALID = (-1),
        STATE_IDLE = 0,
        STATE_OFFER_SENT,             // Offer sent by Engine
        STATE_OFFER_RECEIVED,         // Offer received by Engine
        STATE_ESTABLISHED,            // Offer/Answer exchange done
        STATE_OFFER_CHANGE_SENT,      // Counter offer made
        STATE_OFFER_CHANGE_RECEIVED,  // Counter offer received
        STATE_MAX
    };

    // Type of trigger
    enum
    {
        TRIGGER_NONE = (-1),
        TRIGGER_INVITE = 0,
        TRIGGER_ACK,
        TRIGGER_PRACK,
        TRIGGER_UPDATE,
        TRIGGER_SUCCESS_RESP,
        TRIGGER_FAILURE_RESP,
        TRIGGER_RPR,
        TRIGGER_PROVISIONAL_RESP,
        TRIGGER_CANCEL,
        TRIGGER_BYE,
        TRIGGER_MAX
    };

    // Type of SDP offer
    enum
    {
        OFFER_INVALID,
        OFFER_NEW,
        OFFER_REFRESH,
        OFFER_CHANGE,
        OFFER_MAX
    };

    // SIP message modes
    enum
    {
        // SIP message is being sent out
        MESSAGE_SENT = 0,
        MESSAGE_RECEIVED
    };

    // Mode of Offer/Answer agent
    enum
    {
        MODE_IDLE = 0,
        MODE_OFFERER,
        MODE_ANSWERER
    };

private:
    static const IMS_SINT32 STATE_SENT[STATE_MAX][TRIGGER_MAX];
    static const IMS_SINT32 STATE_RECEIVED[STATE_MAX][TRIGGER_MAX];

    // Media state
    IMS_SINT32 nState;
    IMS_SINT32 nOldState;

    // Mode of agent
    IMS_SINT32 nMode;

    // Flag to indicate whether the received/sent SDP caused any media state transition or not
    IMS_BOOL bStateChanged;

    // Flag to keep track of "glare" condition
    IMS_BOOL bOfferProgress;

    // Flag to keep track of Offer/Answer in the provisional responses
    IMS_BOOL bProvisionalRespWithSdp;

    // Flag to check SDP version
    IMS_BOOL bSDPVersionCheck;
    IMS_BOOL bAlwaysIncreaseSDPVersion;

    // Last offer that was sent by the current session
    SessionParameter* pLastOfferMade;

    // Last offer that was received by the current session
#if 0
    SessionParameter *pLastOfferReceived;
#endif

    // It provides the current session parameters as this session sees them.
    // The ports are of this session and the codecs are a subset of the offer that was made.
    // The session parameters may or may not equal the capabilities exactly.
    SessionParameter* pCurrentView;

    // It provides the peer's view of the session parameter
    // i.e. the peer's port # and media direction etc on the negotiated session parameter.
    // It will be updated only if the application give the options for it
    // while doing the media negotiation.
#if 1
    SessionParameter* pPeerView;
#endif

    // Temporary member field to be used during SDP offer/answer exchanges
    SessionParameter* pProposalView;

    // REFUSE_SDP_OFFER_ANSWER_EXCHANGE
    // Temporary member field to be used for failure case (refusing offer)
    //    - 580 Precondition Failure
    //    - BYE/CANCEL
    SessionParameter* pRefusedView;

    SessionCapabilities* pCapabilities;
};

#endif  // _SDP_OFFER_ANSWER_STATE_H_
