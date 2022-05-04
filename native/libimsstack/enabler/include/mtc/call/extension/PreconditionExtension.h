#ifndef PRECONDITION_EXTENSION_H_
#define PRECONDITION_EXTENSION_H_

#include "IMSTypeDef.h"
#include "call/extension/MtcExtension.h"

class IMessage;

/**
 * This class represents the precondition extension.
 */
class PreconditionExtension final : public MtcExtension
{
public:
    explicit PreconditionExtension();
    explicit PreconditionExtension(IN const PreconditionExtension& objRhs);
    virtual ~PreconditionExtension();
    PreconditionExtension& operator=(IN const PreconditionExtension&) = delete;

    IMtcExtension* Clone() const override;

    void HandleRequest(IN IMS_UINT32 nMethod, IN const IMessage& objRequest) override;
    void HandleResponse(IN IMS_UINT32 nMethod, IN const IMessage& objResponse) override;
};

#endif
