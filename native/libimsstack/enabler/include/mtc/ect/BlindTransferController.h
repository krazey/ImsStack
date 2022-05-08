#ifndef BLIND_TRANSFER_CONTROLLER_H_
#define BLIND_TRANSFER_CONTROLLER_H_

#include "ect/EctController.h"

class IMtcContext;
class IMtcCall;
class IEctControllerListener;

class BlindTransferController :
        public EctController
{
public:
    explicit BlindTransferController(IN IMtcContext& objContext, IN CallKey nCallKey,
            IN IEctControllerListener& objListener);
    virtual ~BlindTransferController();
    BlindTransferController(IN const BlindTransferController&) = delete;
    BlindTransferController& operator=(IN const BlindTransferController&) = delete;

    // IEctReferenceListener implementation
    void OnReferenceStarted() override;

    void Transfer(IN const AString& strNumber) override;

protected:
    IMS_BOOL IsValid() const override;
};

#endif
