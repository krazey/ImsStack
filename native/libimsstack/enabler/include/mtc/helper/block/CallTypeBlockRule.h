#ifndef CALL_TYPE_BLOCK_RULE_H_
#define CALL_TYPE_BLOCK_RULE_H_

#include "IMSTypeDef.h"
#include "helper/block/IMtcBlockRule.h"
#include "call/IMtcCall.h"

class IMtcCallContext;
class MtcConfigurationProxy;

class CallTypeBlockRule final : public IMtcBlockRule
{
public:
    explicit CallTypeBlockRule(IN IMtcCallContext& objContext, CallType eCallTypeToCheck);
    virtual ~CallTypeBlockRule();
    CallTypeBlockRule(IN const CallTypeBlockRule&) = delete;
    CallTypeBlockRule& operator=(IN const CallTypeBlockRule&) = delete;

    Result Check(IN IMtcBlockRuleCheckListener& objListener) override;

private:
    MtcConfigurationProxy& m_objConfiguration;
    CallType m_eCallTypeToCheck;
};

#endif
