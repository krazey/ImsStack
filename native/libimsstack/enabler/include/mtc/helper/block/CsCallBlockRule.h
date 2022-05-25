#ifndef CS_CALL_BLOCK_RULE_H_
#define CS_CALL_BLOCK_RULE_H_

#include "helper/block/IMtcBlockRule.h"

class IMtcCallContext;
class MtcImsEventReceiver;

class CsCallBlockRule final : public IMtcBlockRule
{
public:
    explicit CsCallBlockRule(IN IMtcCallContext& objContext);
    virtual ~CsCallBlockRule();
    CsCallBlockRule(IN const CsCallBlockRule&) = delete;
    CsCallBlockRule& operator=(IN const CsCallBlockRule&) = delete;

    Result Check(IN IMtcBlockRuleCheckListener& objListener) override;

private:
    const IMtcService& m_objService;
    MtcImsEventReceiver& m_objEventReceiver;
    const PeerType m_ePeerType;
};

#endif
