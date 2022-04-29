#ifndef ECT_CONTROLLER_H_
#define ECT_CONTROLLER_H_

#include "call/IMtcCallManager.h"
#include "ect/EctController.h"
#include "helper/ObjectAsyncDestroyer.h"
#include "FailReason.h"

class IMtcContext;
class IMtcCall;

class EctController
{
public:
    explicit EctController(IN IMtcContext& objContext, IN CallKey nCallKey);
    virtual ~EctController();
    EctController(IN const EctController&) = delete;
    EctController& operator=(IN const EctController&) = delete;

    virtual void Transfer(IN const AString& strNumber);
    virtual void Transfer();

protected:
    IMtcCall* GetTransferor() const;
    virtual IMS_BOOL IsValid() const;
    void NotifyResult(IN IMS_RESULT nResult, IN IMS_SINT32 nReason = FAIL_REASON_NONE) const;

    IMtcContext& m_objContext;
    CallKey m_nTransferorKey;
};

#endif
