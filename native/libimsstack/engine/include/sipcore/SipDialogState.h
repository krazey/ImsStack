/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090302  toastops@                 Created
    </table>

    Description
     This class contains certain pieces of state needed for further message transmissions
    within the dialog. This state consists of the dialog ID, a local sequence number (used
    to order requests from the UA to its peer), a remote sequence number (used to order
    requests from its peer to the UA), a local URI, a remote URI, remote target, a boolean
    flag called "secure", and a route set, which is an ordered list of URIs.
    This class is a base class for SipDialog and SipSLSubscription class.
*/

#ifndef _SIP_DIALOG_STATE_H_
#define _SIP_DIALOG_STATE_H_

#include "RCObject.h"
#include "SipDState.h"
#include "SipMessageInfo.h"

class ISipHeader;
class SIPHeader;
class SIPDialogSharedState;
class SIPDialogEx;

/*
This class contains certain pieces of state needed for further message transmissions
within the dialog.

Example

See Also
*/
class SIPDialogState : public RCObject
{
public:
    class PendingRemoteTarget
    {
    public:
        PendingRemoteTarget();
        PendingRemoteTarget(IN CONST AString& strKey_, IN SipHeaderBase* pstHeader_);
        ~PendingRemoteTarget();

    private:
        PendingRemoteTarget(IN CONST PendingRemoteTarget& objRHS);
        PendingRemoteTarget& operator=(IN CONST PendingRemoteTarget& objRHS);

    public:
        AString strKey;
        SipHeaderBase* pstHeader;
    };

public:
    explicit SIPDialogState(IN IMS_BOOL bIsCaller_ = IMS_TRUE);
    SIPDialogState(IN CONST SIPDialogState& objRHS);
    virtual ~SIPDialogState();

private:
    SIPDialogState& operator=(IN CONST SIPDialogState& objRHS);

public:
    IMS_SINT32 CheckToTagValidity(IN CONST SIPMessageInfo& objMInfo);
    IMS_SINT32 CompareTo(IN SIPDialogState* pDState, IN SipMessage* pstMessage,
            IN IMS_BOOL bCheckForked = IMS_FALSE);
    IMS_BOOL Equals(IN SIPDialogState* pDState);

    inline const AString& GetCallId() const { return strCallId; }
    const ISipHeader* GetContactHeader() const;
    AString GetLocalTag() const;
    AString GetRemoteTag() const;
    SipHeaderBase* GetLocalTargetURI() const;
    inline IMS_UINT32 GetNextCSeqNumber() { return (++nLocalCSeq); }
    // HEADER_REQ_SESSION-ID
    inline const AString& GetSessionId() const { return strSessionId; }
    inline IMS_SINT32 GetState() const { return nState; }
    inline IMS_BOOL IsCaller() const { return bIsCaller; }

    IMS_BOOL InitDialogDetails(IN SipMessage* pstMessage);
    IMS_BOOL InitDialogDetails(IN IMS_SINT32 nTrigger, IN SIPDialogState* pDState);
    IMS_BOOL InitRequest(IN CONST SipMethod& objMethod, IN_OUT SipMessage*& pstMessage);
    // CONTACT_HEADER_PARAMETER_CONTROL_FOR_MID_DIALOG_REQUEST
    IMS_RESULT SetContactParameter(
            IN CONST AString& strParameter, IN IMS_SINT32 nOperation = 0 /* (0: ADD, 1: REMOVE) */);
    IMS_SINT32 UpdateDialogDetails(IN CONST SIPMessageInfo& objMInfo, IN IMS_SINT32 nUsageState,
            IN IMS_SINT32 nAction, IN IMS_SINT32 nTrigger);
    void UpdateLocalCSeq(IN IMS_UINT32 nCSeq);
    IMS_SINT32 ValidateRemoteCSeq(IN SipMessage* pstMessage, IN IMS_SINT32 nPrevStatusCode = 0);

    // For sharing a dialog state
    IMS_BOOL AddDialogUsage(IN SIPDialogEx* pDialogEx);
    void RemoveDialogUsage(IN SIPDialogEx* pDialogEx);
    SIPDialogEx* GetDialogUsage(IN CONST SIPMessageInfo& objMInfo);
    IMS_BOOL HasMultipleDialogUsages() const;

    static IMS_BOOL IsContactMandatory(IN IMS_SINT32 nMsgType, IN CONST SipMethod& objMethod,
            IN IMS_SINT32 nStatusCode, IN IMS_BOOL bContactInAll1xxRequired);

private:
    void ClearRouteSet();
    IMS_BOOL CreateRouteSet(IN CONST SIPMessageInfo& objMInfo);
    IMS_BOOL UpdateComponents(IN CONST SIPMessageInfo& objMInfo);
    IMS_BOOL UpdateContact(IN CONST SIPMessageInfo& objMInfo);
    IMS_BOOL UpdateRemoteURI(IN CONST SIPMessageInfo& objMInfo);
    IMS_BOOL UpdateRouteSet(IN CONST SIPMessageInfo& objMInfo);
    // HEADER_REQ_SESSION-ID
    void UpdateSessionId(IN CONST SIPMessageInfo& objMInfo);
    void UpdateState(IN IMS_SINT32 nUsageState, IN IMS_SINT32 nAction, IN IMS_SINT32 nTrigger);

    // REMOTE_TARGET_UPDATE_FROM_MID_DIALOG_REQUEST
    void AddPendingRemoteTarget(IN CONST SIPMessageInfo& objMInfo);
    void RemoveAllPendingRemoteTargets();
    void RemovePendingRemoteTarget(IN CONST SIPMessageInfo& objMInfo);
    void UpdateAndRemovePendingRemoteTarget(IN CONST SIPMessageInfo& objMInfo);

    // "from-change" extension
    IMS_BOOL IsFromChangeCapable() const;
    void ClearFromChangeOption(IN IMS_SINT32 nOption);
    void SetFromChangeOption(IN IMS_SINT32 nOption);
    void UpdateFromChangeOption(IN CONST SIPMessageInfo& objMInfo);

    static IMS_SINT32 CompareHeaders(IN SipHeaderBase* pstNewH, IN SipHeaderBase* pstExistingH,
            IN IMS_BOOL bToTagLenient, IN IMS_SINT32 nForkedMessage);
    static IMS_BOOL IsTargetRefreshMessage(IN SipMessage* pstMessage);

public:
    // Result of Dialog comparison: it is used as a result of CompareTo(...)
    enum
    {
        NOT_MATCHED = 0,
        MATCHED,
        MATCHED_DIFFERENT,
        MATCHED_FORKED_SUBSCRIBE,
        MATCHED_EARLY_NOTIFY,
        MATCHED_OVERLAP_DIALING
    };

    // Category of forked/forking message
    enum
    {
        FORKED_INVITE = 0,
        FORKED_SUBSCRIBE,
        FORKED_ANY
    };

    // Triggers for dialog updates
    enum
    {
        DIALOG_CANCELLED = 0,
        DIALOG_FORKED_REQUEST,
        DIALOG_FORKED_RESPONSE
    };

protected:
    // Tracks "from-change" option
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
    IMS_BOOL bIsCaller;  // caller or callee

    // Local URI
    SipHeaderBase* pstLocalURI;

    // Remote URI
    SipHeaderBase* pstRemoteURI;

    // One of dialog ID
    AString strCallId;

    // Local via branch ID
    AString strLocalViaBranch;

    // Remote via branch ID
    AString strRemoteViaBranch;

    // If the message is sent/received over TLS & Request-URI contains a SIPS URI,
    // this flag is set to IMS_TRUE.
    IMS_BOOL bSecure;

    // Route set
    IMSList<SipHeaderBase*> objRouteSet;

    // Local target URI : multiple may be used
    SipHeaderBase* pstLocalTargetURI;

    // Remote target URI (Contact list need to be managed ???)
    SipHeaderBase* pstRemoteTargetURI;
    // REMOTE_TARGET_UPDATE_FROM_MID_DIALOG_REQUEST
    IMSList<PendingRemoteTarget*> objPendingRemoteTargets;

    // Remote & local sequence number
    IMS_UINT32 nLocalCSeq;
    IMS_UINT32 nRemoteCSeq;

    // Remote INVITE transaction sequence number
    IMS_UINT32 nRemoteCSeqForINVITE;

    // 3 for Session class (Sequence number for Reliable Provisional Response)
    //  Remote & local response sequence number
    // IMS_UINT32    nLocalRSeq;
    // IMS_UINT32    nRemoteRSeq;

    // Remote & local regular txn's sequence number
    // IMS_UINT32    nLocalRegCSeq;
    // IMS_UINT32    nRemoteRegCSeq;
    IMS_SINT32 nState;

    IMS_BOOL bPreloadedSet;

    SIPDialogSharedState* pSharedState;

    // For local contact address retrieval
    SIPHeader* pLocalContactHeader;

    // HEADER_REQ_SESSION-ID
    // For Session-ID header control
    AString strSessionId;

    IMS_SINT32 nFromChangeOption;
};

#endif  // _SIP_DIALOG_STATE_H_
