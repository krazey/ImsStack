#ifndef TERMINAL_BASED_CALL_WAITING_BLOCK_RULE_H_
#define TERMINAL_BASED_CALL_WAITING_BLOCK_RULE_H_

#include "IMSTypeDef.h"
#include "helper/block/IMtcBlockRule.h"

class IMtcCallContext;
class IMtcCallManager;

class TerminalBasedCallWaitingBlockRule final : public IMtcBlockRule
{
public:
    explicit TerminalBasedCallWaitingBlockRule(IN IMtcCallContext& objContext);
    virtual ~TerminalBasedCallWaitingBlockRule();
    TerminalBasedCallWaitingBlockRule(IN const TerminalBasedCallWaitingBlockRule&) = delete;
    TerminalBasedCallWaitingBlockRule& operator=(
            IN const TerminalBasedCallWaitingBlockRule&) = delete;

    Result Check(IN IMtcBlockRuleCheckListener& objListener) override;

private:
    IMS_UINT32 GetActiveCallCount(IN const IMSList<IMtcCall*> lstCalls);

    IMtcService& m_objService;
    IMtcCallManager& m_objCallManager;
};

#endif
