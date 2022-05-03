#ifndef VOPS_BLOCK_RULE_H_
#define VOPS_BLOCK_RULE_H_

#include "helper/block/IMtcBlockRule.h"

class IMtcService;
class MtcImsEventReceiver;

class VopsBlockRule final : public IMtcBlockRule
{
public:
    explicit VopsBlockRule(IN IMtcService& objService, IN MtcImsEventReceiver& objEventReceiver);
    virtual ~VopsBlockRule();
    VopsBlockRule(IN const VopsBlockRule&) = delete;
    VopsBlockRule& operator=(IN const VopsBlockRule&) = delete;

    Result Check(IN IMtcBlockRuleCheckListener& objListener) override;

private:
    IMtcService& m_objService;
    MtcImsEventReceiver& m_objEventReceiver;
};

#endif
