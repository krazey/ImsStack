#ifndef CS_CALL_BLOCK_RULE_H_
#define CS_CALL_BLOCK_RULE_H_

#include "helper/block/IMtcBlockRule.h"

class MtcImsEventReceiver;

class CsCallBlockRule final : public IMtcBlockRule
{
public:
    explicit CsCallBlockRule(IN MtcImsEventReceiver& objEventReceiver);
    virtual ~CsCallBlockRule();
    CsCallBlockRule(IN const CsCallBlockRule&) = delete;
    CsCallBlockRule& operator=(IN const CsCallBlockRule&) = delete;

    Result Check(IN IMtcBlockRuleCheckListener& objListener) override;

private:
    MtcImsEventReceiver& m_objEventReceiver;
};

#endif
