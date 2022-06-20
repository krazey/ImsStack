#ifndef MTC_BLOCK_CHECKER_H_
#define MTC_BLOCK_CHECKER_H_

#include "IMSList.h"
#include "IMSTypeDef.h"
#include "helper/block/IMtcBlockChecker.h"
#include "helper/block/IMtcBlockRule.h"
#include "MtcDef.h"

class IMtcCallContext;

/**
 * This class checks if a call operation is blocked or not for the given ruleset.
 * Also it provides rulesets for each call operation.
 */
class MtcBlockChecker final : public IMtcBlockChecker, public IMtcBlockRuleCheckListener
{
public:
    /**
     * Constructs a new MtcBlockChecker object.
     *
     * @param lstRules List of the rules to check. The elements are managed by this object.
     * @param pListener Listener to be notified the result if pending.
     */
    MtcBlockChecker(
            IN const IMSList<IMtcBlockRule*>& lstRules, IN IMtcBlockCheckListener* pListener);

    ~MtcBlockChecker();

    MtcBlockChecker(IN const MtcBlockChecker&) = delete;
    MtcBlockChecker& operator=(IN const MtcBlockChecker&) = delete;

    static IMSList<IMtcBlockRule*> GetCallUpdateRules(IN IMtcCallContext& objContext);

    Result Check() override;

    void OnBlockRuleChecked(IN IMtcBlockRule::Result objResult) override;

private:
    IMS_BOOL IsResultNotified();

    IMtcBlockCheckListener* m_pListener;
    IMSList<IMtcBlockRule*> m_lstRules;

    IMS_SINT32 m_nPendingCount;
};

#endif
