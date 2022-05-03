#ifndef PROCESSING_CALL_BLOCK_RULE_H_
#define PROCESSING_CALL_BLOCK_RULE_H_

#include "IMSTypeDef.h"
#include "helper/block/IMtcBlockRule.h"

class IMtcCallManager;

class ProcessingCallBlockRule final : public IMtcBlockRule
{
public:
    explicit ProcessingCallBlockRule(IN IMtcCallManager& objCallManager);
    virtual ~ProcessingCallBlockRule();
    ProcessingCallBlockRule(IN const ProcessingCallBlockRule&) = delete;
    ProcessingCallBlockRule& operator=(IN const ProcessingCallBlockRule&) = delete;

    Result Check(IN IMtcBlockRuleCheckListener& objListener) override;

private:
    IMS_BOOL IsOtherIdleCallExists(IN const IMSList<IMtcCall*>& lstCalls);
    IMS_BOOL IsIncomingCallExists(IN const IMSList<IMtcCall*>& lstCalls);
    IMS_BOOL IsOutgoingCallExists(IN const IMSList<IMtcCall*>& lstCalls);
    IMS_BOOL IsEmergencyCallExists(IN IMtcCallManager& objCallManager);

    IMtcCallManager& m_objCallManager;
};

#endif
