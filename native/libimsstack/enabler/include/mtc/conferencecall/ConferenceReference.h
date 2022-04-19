#ifndef CONFERENCE_REFERENCE_H_
#define CONFERENCE_REFERENCE_H_

#include "IReferenceListener.h"
#include "conferencecall/IConferenceReference.h"

class CallConnectionIdManager;
class IReference;
class IMtcCall;
class IMtcContext;
class IConferenceReferenceListener;

class ConferenceReference final :
        public IReferenceListener,
        public IConferenceReference
{
public:
    explicit ConferenceReference(IN IMtcContext& objContext, IN CallKey nConfCallKey,
            IN ConfUser* pConfUser, IN IConferenceReferenceListener& objListener);
    explicit ConferenceReference(IN IMtcContext& objContext, IN CallKey nConfCallKey,
            IN IMSList<ConfUser*>& objConfUsers, IN IConferenceReferenceListener& objListener);
    virtual ~ConferenceReference();
    ConferenceReference(IN const ConferenceReference&) = delete;
    ConferenceReference& operator=(IN const ConferenceReference&) = delete;

public:
    // IReferenceListener interface implementation
    void ReferenceDelivered(IN IReference* piReference) override;
    void ReferenceDeliveryFailed(IN IReference* piReference) override;
    void ReferenceNotify(IN IReference* piReference, IN IMessage* piNotify) override;
    void ReferenceTerminated(IN IReference* piReference) override;

    // IConferenceReference interface implementation
    IMS_RESULT SendInvite(OUT AString& strReferToUri,
            IN CallConnectionIdManager& objConnectionIdManager) override;
    IMS_RESULT SendBye(IN AString strInvitedUri = AString::ConstEmpty()) override;
    IMS_UINT32 GetType() const override;
    IMS_UINT32 GetResponseCode() const override;
    inline void SetForceToTerminateInterface(IN IMS_BOOL bTerminate) override
    { m_bForceToTerminateInterface = bTerminate;}

private:
    void GetReferToUri(OUT AString& strUri, IN IMtcCall* pi1To1Call) const;
    void SetReplaces(IN IMtcCall* pi1To1Call);
    void SetReferredByHeader();
    void SetHeadersForReferTo(OUT AString& strHeadersForReferTo);

private:
    static const IMS_CHAR METHOD_INVITE[];
    static const IMS_CHAR METHOD_BYE[];

    IMtcContext& m_objContext;
    CallKey m_nConfCallKey;
    IConferenceReferenceListener& m_objListener;
    IMS_UINT32 m_nType;
    ConfUser* m_pConfUser;
    IMSList<ConfUser*> m_objConfUsers;
    IReference* m_piReference;
    IMS_BOOL m_bForceToTerminateInterface;
};

#endif
