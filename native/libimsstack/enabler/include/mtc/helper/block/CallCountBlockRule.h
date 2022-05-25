#ifndef CALL_COUNT_BLOCK_RULE_H_
#define CALL_COUNT_BLOCK_RULE_H_

#include "IMSTypeDef.h"
#include "helper/block/IMtcBlockRule.h"

class IMtcCallContext;
class IMtcCallManager;

class CallCountBlockRule final : public IMtcBlockRule
{
public:
    explicit CallCountBlockRule(IN IMtcCallContext& objContext);
    virtual ~CallCountBlockRule();
    CallCountBlockRule(IN const CallCountBlockRule&) = delete;
    CallCountBlockRule& operator=(IN const CallCountBlockRule&) = delete;

    Result Check(IN IMtcBlockRuleCheckListener& objListener) override;

private:
    IMS_UINT32 GetActiveCallCount(IN const IMSList<IMtcCall*> lstCalls);

    IMtcCallManager& m_objCallManager;
    const CallInfo& m_objCallInfo;
    const IMS_UINT32 m_nMaxCallCount;
};

#endif
