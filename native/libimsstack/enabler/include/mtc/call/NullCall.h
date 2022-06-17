#ifndef NULL_CALL_H_
#define NULL_CALL_H_

#include "IMSList.h"
#include "IMSTypeDef.h"
#include "MtcDef.h"
#include "call/IMtcCall.h"

class AString;
class ISession;
class JniMtcServiceThread;
class MediaInfo;
class SuppService;
struct CallReasonInfo;

/**
 * This class represents the call that doesn't exist. It has no states and basically does nothing.
 */
class NullCall final : public IMtcCall
{
public:
    NullCall() {}
    virtual ~NullCall() {}
    NullCall(IN const NullCall&) = delete;
    NullCall& operator=(IN const NullCall&) = delete;

    inline void Attach(IN JniMtcCallThread*, IN JniMediaSessionThread*) override {}
    inline void Detach() override {}

    inline void Start(IN CallType, IN const AString&, IN MediaInfo*,
            IN const IMSMap<SuppType, SuppService*>&) override
    {
    }

    inline void StartConference(IN CallType, IN const AString&, IN MediaInfo*,
            IN const IMSMap<SuppType, SuppService*>&, IN IMSList<ConfUser*>) override
    {
    }
    inline void StartConference(IN CallType, IN const AString&, IN IMSList<ConfUser*>) override {}
    inline void HandleIncoming(IN ISession*, IN JniMtcServiceThread*) override {}
    inline void HandleUserAlert() override {}
    inline void Accept(IN CallType, IN MediaInfo*) override {}
    inline void Reject(IN const CallReasonInfo&) override {}
    inline void Hold(IN MediaInfo*) override {}
    inline void Resume(IN MediaInfo*) override {}
    inline void AcceptResume(IN CallType, IN MediaInfo*) override {}
    inline void RejectResume(IN const CallReasonInfo&) override {}
    inline void Convert(IN CallType, IN MediaInfo*) override {}
    inline void AcceptConvert(IN CallType, IN MediaInfo*) override {}
    inline void RejectConvert(IN const CallReasonInfo&) override {}
    inline void CancelConvert(IN const CallReasonInfo&) override {}
    inline void Terminate(IN const CallReasonInfo&) override {}
    inline void SendDtmf(IN const AString&, IN IMS_SINT32) override {}
    inline void SendUssd(IN const AString&) override {}
    inline void HandleSrvccSuccess() override {}
    inline void HandleSrvccFailure(IN UpdateType) override {}

    inline CallKey GetKey() const override { return -1; }
    inline State GetState() const override { return State::IDLE; }
    inline IMtcCallContext& GetCallContext() const override { return *(IMtcCallContext*)this; }
};

#endif
