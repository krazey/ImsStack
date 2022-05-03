#ifndef INTERFACE_MTC_BLOCK_RULE_H_
#define INTERFACE_MTC_BLOCK_RULE_H_

#include "FailReason.h"
#include "IMSTypeDef.h"
#include "MtcDef.h"

class IMtcBlockRuleCheckListener;

/*
 * Represents a rule to check if some operation is blocked or not.
 */
class IMtcBlockRule
{
public:
    struct Result
    {
    public:
        enum class Status
        {
            UNBLOCKED,
            BLOCKED,
            PENDING,
        };

        Result(IN Status _eStatus, IN const FailReason& _objReason) :
                eStatus(_eStatus),
                objReason(_objReason)
        {
        }

        Result(IN Status _eStatus) :
                Result(_eStatus, FailReason(FAIL_REASON_NONE))
        {
        }

        Status eStatus;
        FailReason objReason;
    };

    virtual ~IMtcBlockRule() {}

    /**
     * Checks if an operation is blocked for this rule.
     * The checker could return `PENDING` status if the checking takes time. But it must notify
     * `IMtcBlockRuleCheckListener` later with the final result in this case,
     * Notifying listener happens only once per each checking.
     *
     * @param objListener Listener to be notified the result if it was in pending.
     * @return Result. `FailReason` is valid only  when the status is `BLOCKED`.
     */
    virtual Result Check(IN IMtcBlockRuleCheckListener& objListener) = 0;
};

/**
 * The result of `IMtcBlockRule::Check()` could be notified by this listener.
 */
class IMtcBlockRuleCheckListener
{
public:
    ~IMtcBlockRuleCheckListener() {}

    /**
     * Notifies the block check result.
     *
     * @param objResult Result. `eStatus` is not `PENDING`.
     *                  `FailReason` is valid only when `eStatus` is `BLOCKED`.
     */
    virtual void OnBlockRuleChecked(IN IMtcBlockRule::Result objResult) = 0;
};

#endif
