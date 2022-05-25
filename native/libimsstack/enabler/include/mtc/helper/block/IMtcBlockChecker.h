#ifndef INTERFACE_MTC_BLOCK_CHECKER_H_
#define INTERFACE_MTC_BLOCK_CHECKER_H_

#include "helper/block/IMtcBlockRule.h"
#include "MtcDef.h"

/*
 * This is the interface to check if some operation is blocked for some reason or not.
 * The result could be unblocked, blocked, or in a pending status.
 */
class IMtcBlockChecker
{
public:
    using Result = IMtcBlockRule::Result;

    virtual ~IMtcBlockChecker() {}

    /**
     * Checks if an operation is blocked for some reason.
     * The checker could notify `IMtcBlockCheckListener` the final result later if pending.
     * Notifying listener happens only once per each checking.
     *
     * @return Result. `FailReason` is valid only when the status is `BLOCKED`.
     */
    virtual Result Check() = 0;
};

/**
 * The result of `IMtcBlockChecker::Check()` could be notified by this listener.
 */
class IMtcBlockCheckListener
{
public:
    ~IMtcBlockCheckListener() {}

    /**
     * Notifies the block check result.
     *
     * @param objResult Result. `eStatus` is not `PENDING`.
     *                  `FailReason` is valid only when `eStatus` is `BLOCKED`.
     */
    virtual void OnBlockChecked(IN IMtcBlockChecker::Result objResult) = 0;
};

#endif
