#ifndef CONSULTATIVE_TRANSFER_CONTROLLER_H_
#define CONSULTATIVE_TRANSFER_CONTROLLER_H_

#include "ect/EctController.h"

class IMtcContext;
class IMtcCall;
class IEctControllerListener;

class ConsultativeTransferController : public EctController
{
public:
    explicit ConsultativeTransferController(IN IMtcContext& objContext, IN CallKey nCallKey,
            IN IEctControllerListener& objListener);
    virtual ~ConsultativeTransferController();
    ConsultativeTransferController(IN const ConsultativeTransferController&) = delete;
    ConsultativeTransferController& operator=(IN const ConsultativeTransferController&) = delete;

    void Transfer() override;

protected:
    IMS_BOOL IsValid() const override;
    void OnCompleted() override;

private:
    void FindTransferTarget();
    void TerminateTransferTargetCall();

    CallKey m_nTransferTargetKey;
};

#endif
