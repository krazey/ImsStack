#ifndef RPR_EXTENSION_H_
#define RPR_EXTENSION_H_

#include "IMSTypeDef.h"
#include "call/extension/MtcExtension.h"

class IMessage;

/**
 * This class represents the 100rel extension.
 */
class RprExtension final :
        public MtcExtension
{
public:
    explicit RprExtension();
    explicit RprExtension(IN const RprExtension& objRhs);
    virtual ~RprExtension();
    RprExtension& operator=(IN const RprExtension&) = delete;

    IMtcExtension* Clone() const override;

    void HandleRequest(IN IMS_UINT32 nMethod, IN const IMessage& objRequest) override;
    void HandleResponse(IN IMS_UINT32 nMethod, IN const IMessage& objResponse) override;
};

#endif
