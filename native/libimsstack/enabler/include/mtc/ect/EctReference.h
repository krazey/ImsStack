#ifndef ECT_REFERENCE_H_
#define ECT_REFERENCE_H_

#include "IReferenceListener.h"
#include "call/IMtcCall.h"

class IReference;
class IMtcCall;
class IMtcContext;
class IEctReferenceListener;

class EctReference final : public IReferenceListener
{
public:
    explicit EctReference(IN IMtcContext& objContext, IN CallKey nTransfereeKey,
            IN IEctReferenceListener& objListener);
    ~EctReference();
    EctReference(IN const EctReference&) = delete;
    EctReference& operator=(IN const EctReference&) = delete;

public:
    // IReferenceListener interface implementation
    void ReferenceDelivered(IN IReference* piReference) override;
    void ReferenceDeliveryFailed(IN IReference* piReference) override;
    void ReferenceNotify(IN IReference* piReference, IN IMessage* piNotify) override;
    void ReferenceTerminated(IN IReference* piReference) override;

    IMS_RESULT SendInvite(IN CallKey nTransferTargetKey);
    IMS_RESULT SendInvite(IN const AString& strTransferTarget);
    IMS_UINT32 GetResponseCode() const;

private:
    IMS_RESULT SendRefer(IN const AString& strReferToUri, IN IMtcCall* piTransferTargetCall);
    AString GetReferToUri(IN const IMtcCall* piTransferTargetCall) const;
    AString GetReferToUri(IN const AString& strTransferTarget) const;
    void SetReplaces(IN IMtcCall* piTransferTargetCall);
    void SetReferredByHeader();
    void SetHeadersForReferTo(OUT AString& strHeadersForReferTo);

private:
    IMtcContext& m_objContext;
    CallKey m_nTransfereeKey;
    IEctReferenceListener& m_objListener;
    IReference* m_piReference;
};

#endif
