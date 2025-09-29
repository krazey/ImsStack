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
#ifndef SIP_DIALOG_STATE_H_
#define SIP_DIALOG_STATE_H_

#include "RcObject.h"

#include "SipDState.h"
#include "SipHeader.h"
#include "SipMessageInfo.h"

class SipDialogEx;
class SipDialogSharedState;

/**
 * @brief This class contains certain pieces of state needed for further message transmissions
 *        within the dialog.
 *
 * This state consists of the dialog ID, a local sequence number (used to order requests
 * from the UA to its peer), a remote sequence number (used to order requests
 * from its peer to the UA), a local URI, a remote URI, remote target, a boolean flag called
 * "secure", and a route set, which is an ordered list of URIs.
 */
class SipDialogState : public RcObject
{
public:
    class PendingRemoteTarget
    {
    public:
        PendingRemoteTarget();
        PendingRemoteTarget(IN const PendingRemoteTarget& other);
        PendingRemoteTarget(IN const AString& strKey_, IN SipHeaderBase* pSipHdr_);
        ~PendingRemoteTarget();

        PendingRemoteTarget& operator=(IN const PendingRemoteTarget&) = delete;

    public:
        AString strKey;
        SipHeaderBase* pSipHdr;
    };

public:
    explicit SipDialogState(IN IMS_BOOL bIsCaller = IMS_TRUE);
    SipDialogState(IN const SipDialogState& other);
    ~SipDialogState() override;

    SipDialogState& operator=(IN const SipDialogState&) = delete;

public:
    IMS_SINT32 CheckToTagValidity(IN const SipMessageInfo& objMsgInfo);
    IMS_SINT32 CompareTo(IN SipDialogState* pDState, IN ::SipMessage* pSipMsg,
            IN IMS_BOOL bCheckForked = IMS_FALSE);
    IMS_BOOL Equals(IN SipDialogState* pDState);

    inline const AString& GetCallId() const { return m_strCallId; }
    const ISipHeader* GetContactHeader() const;
    AString GetLocalTag() const;
    AString GetRemoteTag() const;
    inline SipHeaderBase* GetLocalTargetUri() const { return m_pLocalTargetUri; }
    inline IMS_UINT32 GetNextCSeqNumber() { return (++m_nLocalCSeq); }
    // HEADER_REQ_SESSION-ID
    inline const AString& GetSessionId() const { return m_strSessionId; }
    inline IMS_SINT32 GetState() const { return m_nState; }
    inline IMS_BOOL IsCaller() const { return m_bIsCaller; }

    IMS_BOOL InitDialogDetails(IN ::SipMessage* pSipMsg);
    IMS_BOOL InitDialogDetails(IN IMS_SINT32 nTrigger, IN const SipDialogState* pDState);
    IMS_BOOL InitRequest(IN const SipMethod& objMethod, IN_OUT ::SipMessage*& pSipMsg);
    // CONTACT_HEADER_PARAMETER_CONTROL_FOR_MID_DIALOG_REQUEST
    IMS_RESULT SetContactParameter(
            IN const AString& strParameter, IN IMS_SINT32 nOperation = 0 /*(0: ADD, 1: REMOVE)*/);
    IMS_SINT32 UpdateDialogDetails(IN const SipMessageInfo& objMsgInfo, IN IMS_SINT32 nUsageState,
            IN IMS_SINT32 nAction, IN IMS_SINT32 nTrigger);
    void UpdateLocalCSeq(IN IMS_UINT32 nCSeq);
    IMS_SINT32 ValidateRemoteCSeq(IN ::SipMessage* pSipMsg, IN IMS_SINT32 nPrevStatusCode = 0);

    // For sharing a dialog state
    IMS_BOOL AddDialogUsage(IN SipDialogEx* pDialogEx);
    void RemoveDialogUsage(IN SipDialogEx* pDialogEx);
    SipDialogEx* GetDialogUsage(IN const SipMessageInfo& objMsgInfo);
    IMS_BOOL HasMultipleDialogUsages() const;

    static IMS_BOOL IsContactMandatory(IN IMS_SINT32 nMsgType, IN const SipMethod& objMethod,
            IN IMS_SINT32 nStatusCode, IN IMS_BOOL bContactInAll1xxRequired);

private:
    void ClearRouteSet();
    IMS_BOOL CreateRouteSet(IN const SipMessageInfo& objMsgInfo);
    void UpdateComponents(IN const SipMessageInfo& objMsgInfo);
    void UpdateContact(IN const SipMessageInfo& objMsgInfo);
    IMS_BOOL UpdateRemoteUri(IN const SipMessageInfo& objMsgInfo);
    IMS_BOOL UpdateRouteSet(IN const SipMessageInfo& objMsgInfo);
    // HEADER_REQ_SESSION-ID
    void UpdateSessionId(IN const SipMessageInfo& objMsgInfo);
    void UpdateState(IN IMS_SINT32 nUsageState, IN IMS_SINT32 nAction, IN IMS_SINT32 nTrigger);

    // REMOTE_TARGET_UPDATE_FROM_MID_DIALOG_REQUEST
    void AddPendingRemoteTarget(IN const SipMessageInfo& objMsgInfo);
    void RemoveAllPendingRemoteTargets();
    void RemovePendingRemoteTarget(IN const SipMessageInfo& objMsgInfo);
    void UpdateAndRemovePendingRemoteTarget(IN const SipMessageInfo& objMsgInfo);

    // "from-change" extension
    inline IMS_BOOL IsFromChangeCapable() const
    {
        return ((m_nFromChangeOption & FROM_CHANGE_CAPABLE) == FROM_CHANGE_CAPABLE);
    }
    void SetFromChangeOption(IN IMS_SINT32 nOption);
    void UpdateFromChangeOption(IN const SipMessageInfo& objMsgInfo);

    static IMS_SINT32 CompareHeaders(IN SipHeaderBase* pNewH, IN SipHeaderBase* pExistingH,
            IN IMS_BOOL bToTagLenient, IN IMS_SINT32 nForkedMessage);
    static IMS_BOOL IsTargetRefreshMessage(IN ::SipMessage* pSipMsg);

public:
    /// Result of Dialog comparison: it is used as a result of CompareTo(...)
    enum
    {
        NOT_MATCHED = 0,
        MATCHED,
        MATCHED_DIFFERENT,
        MATCHED_FORKED_SUBSCRIBE,
        MATCHED_EARLY_NOTIFY,
        MATCHED_OVERLAP_DIALING
    };

    /// Category of forked/forking message
    enum
    {
        FORKED_INVITE = 0,
        FORKED_SUBSCRIBE,
        FORKED_ANY
    };

    /// Triggers for dialog updates
    enum
    {
        DIALOG_CANCELLED = 0,
        DIALOG_FORKED_REQUEST,
        DIALOG_FORKED_RESPONSE
    };

protected:
    /// Tracks "from-change" option
    enum
    {
        FROM_CHANGE_NONE = 0,

        FROM_CHANGE_ON_INVITE_REQUEST = 0x0001,
        FROM_CHANGE_ON_INVITE_RESPONSE = 0x0002,

        FROM_CHANGE_CAPABLE = (FROM_CHANGE_ON_INVITE_REQUEST | FROM_CHANGE_ON_INVITE_RESPONSE)
    };

    // Flag to identify a dialog creator
    //    TRUE if UA sends the request which can create a dialog
    //    FALSE if UA receives the request which can  create a dialog
    IMS_BOOL m_bIsCaller;  // caller or callee
    SipHeaderBase* m_pLocalUri;
    SipHeaderBase* m_pRemoteUri;

    // One of dialog ID
    AString m_strCallId;
    // Local via branch ID
    AString m_strLocalViaBranch;
    // Remote via branch ID
    AString m_strRemoteViaBranch;

    // If the message is sent/received over TLS & Request-URI contains a SIPS URI,
    // this flag is set to IMS_TRUE.
    IMS_BOOL m_bSecure;
    // Route set
    ImsList<SipHeaderBase*> m_objRouteSet;
    // Local target URI : multiple may be used
    SipHeaderBase* m_pLocalTargetUri;
    // Remote target URI (Contact list need to be managed ???)
    SipHeaderBase* m_pRemoteTargetUri;
    // REMOTE_TARGET_UPDATE_FROM_MID_DIALOG_REQUEST
    ImsList<PendingRemoteTarget*> m_objPendingRemoteTargets;

    // Remote & local sequence number
    IMS_UINT32 m_nLocalCSeq;
    IMS_UINT32 m_nRemoteCSeq;
    // Remote INVITE transaction sequence number
    IMS_UINT32 m_nRemoteCSeqForInvite;

    IMS_SINT32 m_nState;
    IMS_BOOL m_bPreloadedSet;
    SipDialogSharedState* m_pSharedState;

    // For local contact address retrieval
    SipHeader* m_pLocalContactHeader;
    // HEADER_REQ_SESSION-ID
    // For Session-ID header control
    AString m_strSessionId;
    IMS_SINT32 m_nFromChangeOption;
};

#endif
