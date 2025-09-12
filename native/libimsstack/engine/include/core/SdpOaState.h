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
#ifndef SDP_OFFER_ANSWER_STATE_H_
#define SDP_OFFER_ANSWER_STATE_H_

#include "ISdpOaState.h"
#include "SdpParser.h"
#include "SessionParameter.h"

class ISipMessage;
class Service;
class SessionCapabilities;

class SdpOaState : public ISdpOaState
{
public:
    explicit SdpOaState(IN IMS_BOOL bSdpVersionCheck = IMS_TRUE,
            IN IMS_BOOL bAlwaysIncreaseSdpVersion = IMS_FALSE);
    SdpOaState(IN const SdpOaState& other);
    ~SdpOaState() override;

public:
    SdpOaState& operator=(IN const SdpOaState&) = delete;

public:
    // ISdpOaState interface implementations
    void AbortProposal() override;
    IMS_SINT32 CreateProposalView() override;
    IMS_SINT32 GetSessionCurrentView(OUT SdpSessionParameter*& pSessionParam) const override;
    IMS_SINT32 GetSessionPeerView(OUT SdpSessionParameter*& pSessionParam) const override;
    IMS_SINT32 GetSessionProposalView(OUT SdpSessionParameter*& pSessionParam) const override;
    IMS_SINT32 CreateMediaParameter(OUT SdpMediaParameter*& pMediaParam) override;
    IMS_SINT32 GetMediaCurrentView(
            IN IMS_SINT32 nMid, OUT SdpMediaParameter*& pMediaParam) const override;
    IMS_SINT32 GetMediaPeerView(
            IN IMS_SINT32 nMid, OUT SdpMediaParameter*& pMediaParam) const override;
    IMS_SINT32 GetMediaProposalView(
            IN IMS_SINT32 nMid, OUT SdpMediaParameter*& pMediaParam) const override;
    void MarkRejectedOrRemoved(IN IMS_SINT32 nMid) override;
    void RemoveMediaParameter(IN IMS_SINT32 nMid) override;

    IMS_BOOL CreateCapabilities(IN Service* pService, IN IMS_BOOL bMProf = IMS_FALSE);
    const SessionParameter* GetCapabilities() const;
    /**
     * @brief Returns the current local session parameter.
     */
    inline SessionParameter* GetCurrentView() const { return m_pCurrentView; }
    /**
     * @brief Returns the peer session parameter.
     */
    inline SessionParameter* GetPeerView() const { return m_pPeerView; }
    /**
     * @brief Returns the current proposal session parameter.
     */
    inline SessionParameter* GetProposalView() const { return m_pProposalView; }
    /**
     * @brief Returns the mode of SDP offer/answer model.
     */
    inline IMS_SINT32 GetMode() const { return m_nMode; }
    IMS_BOOL GetSdp(OUT AString& strSdp) const;
    /**
     * @brief Returns the state of SDP offer/answer.
     */
    inline IMS_SINT32 GetState() const { return m_nState; }
    IMS_BOOL InitiateOffer(IN IMS_SINT32 nType);
    /**
     * @brief Checks if the offer is on progress or not.
     */
    inline IMS_BOOL IsOfferProgress() const { return m_bOfferProgress; }
    /**
     * @brief Checks if SDP OA state is in preview mode or not.
     */
    inline IMS_BOOL IsInPreviewMode() const { return m_bPreviewMode; }
    /**
     * @brief Checks if the session is changed or not.
     */
    inline IMS_BOOL IsSessionChanged() const { return m_bStateChanged; }
    IMS_SINT32 HandleOfferAnswer(IN const ISipMessage* piSipMsg);
    void CompleteExchange();
    IMS_BOOL RestoreState();
    IMS_BOOL UpdateState(IN const ISipMessage* piSipMsg, IN IMS_SINT32 nMessageFlow,
            IN IMS_BOOL bIsCallEstablished, IN IMS_BOOL bAllowOaForNonRpr = IMS_FALSE);
    void UpdateStateOnTransactionCompleted(
            IN const ISipMessage* piSipMsg, IN IMS_SINT32 nMessageFlow);

    // REFUSE_SDP_OFFER_ANSWER_EXCHANGE {
    void CreateRefusedView();
    void DestroyRefusedView();
    /**
     * @brief Returns a refused SDP view.
     */
    inline SessionParameter* GetRefusedView() const { return m_pRefusedView; }
    // }
    /**
     * @brief Increases the local session version for the current view, the capabilities,
     *        and the last offer made view if present.
     */
    void IncreaseSessionVersion();

private:
    SessionParameter* GetCurrentCapabilities();
    SessionParameter*& GetNewProposalView();
    SessionParameter*& GetNewPeerView();
    IMS_SINT32 HandleAnswer(IN const SdpParser& objParser, IN IMS_BOOL bUpdateRemoteVersion);
    IMS_SINT32 HandleOffer(IN const SdpParser& objParser);
    void SetCapabilities();
    void SetProposedView(IN SessionParameter* pSessionParam);
    void SetCurrentView(IN SessionParameter* pSessionParam);
    void SetLastOfferMade(IN SessionParameter* pSessionParam);
    void SetOldState(IN IMS_SINT32 nOldState);
    void SetState(IN IMS_SINT32 nState);
    void StartPreviewMode();
    void StopPreviewMode();

    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

public:
    /// Type of state
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

    /// Type of trigger
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

    /// Type of SDP offer
    enum
    {
        OFFER_INVALID,
        OFFER_NEW,
        OFFER_REFRESH,
        OFFER_CHANGE,
        OFFER_MAX
    };

    /// SIP message modes
    enum
    {
        /// SIP message is being sent out
        MESSAGE_SENT = 0,
        MESSAGE_RECEIVED
    };

    /// Mode of Offer/Answer agent
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
    IMS_SINT32 m_nState;
    IMS_SINT32 m_nOldState;
    // Mode of agent
    IMS_SINT32 m_nMode;
    // Flag specifying whether the current SDP OA mode is in preview mode or not.
    // 1) The preview mode is set when non-RPR message with SDP is received and
    //    kept until the call is completely established.
    // 2) Depending on the carrier-config, SDP re-negotiation can be performed in preview mode
    //    when receiving another RPR or non-RPR message, and must operate in non-preview mode
    //    when receiving an RPR message.
    IMS_BOOL m_bPreviewMode;
    // Flag to indicate whether the received/sent SDP caused any media state transition or not
    IMS_BOOL m_bStateChanged;
    // Flag to keep track of "glare" condition
    IMS_BOOL m_bOfferProgress;
    // Flag to keep track of Offer/Answer in the provisional responses
    IMS_BOOL m_bProvisionalRespWithSdp;
    // Flag to check SDP version
    IMS_BOOL m_bSdpVersionCheck;
    IMS_BOOL m_bAlwaysIncreaseSdpVersion;

    // Last offer that was sent by the current session
    SessionParameter* m_pLastOfferMade;
    // Last offer that was received by the current session
    // SessionParameter *m_pLastOfferReceived;
    // It provides the current session parameters as this session sees them.
    // The ports are of this session and the codecs are a subset of the offer that was made.
    // The session parameters may or may not equal the capabilities exactly.
    SessionParameter* m_pCurrentView;
    // It provides the peer's view of the session parameter
    // i.e. the peer's port # and media direction etc on the negotiated session parameter.
    // It will be updated only if the application give the options for it
    // while doing the media negotiation.
    SessionParameter* m_pPeerView;
    // Temporary member field to be used during SDP offer/answer exchanges
    SessionParameter* m_pProposalView;
    // REFUSE_SDP_OFFER_ANSWER_EXCHANGE
    // Temporary member field to be used for failure case (refusing offer)
    //    - 580 Precondition Failure
    //    - BYE/CANCEL
    SessionParameter* m_pRefusedView;

    SessionCapabilities* m_pCapabilities;
};

#endif
