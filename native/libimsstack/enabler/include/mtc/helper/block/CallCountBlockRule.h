#ifndef CALL_COUNT_BLOCK_RULE_H_
#define CALL_COUNT_BLOCK_RULE_H_

#include "IMSTypeDef.h"
#include "helper/block/IMtcBlockRule.h"

class IMtcCallManager;

class CallCountBlockRule final : public IMtcBlockRule
{
public:
    explicit CallCountBlockRule(IN IMS_UINT32 nMaxCount, IN IMtcCallManager& objCallManager);
    virtual ~CallCountBlockRule();
    CallCountBlockRule(IN const CallCountBlockRule&) = delete;
    CallCountBlockRule& operator=(IN const CallCountBlockRule&) = delete;

    Result Check(IN IMtcBlockRuleCheckListener& objListener) override;

private:
    IMS_UINT32 GetActiveCallCount(IN const IMSList<IMtcCall*> lstCalls);

    const IMS_UINT32 m_nMaxCount;
    IMtcCallManager& m_objCallManager;
};

#endif
