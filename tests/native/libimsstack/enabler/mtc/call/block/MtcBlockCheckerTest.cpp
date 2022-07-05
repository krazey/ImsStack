#include "call/block/MtcBlockChecker.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using Result = IMtcBlockChecker::Result;

const CallReasonInfo objDefaultReason(CODE_NONE);
const CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);

class TestListener :
        public IMtcBlockCheckListener
{
public:
    TestListener() :
            m_eStatus(Result::Status::BLOCKED),
            m_objReason(objDefaultReason) {}
    virtual ~TestListener() {}

    void OnBlockChecked(IN Result objResult) override
    {
        m_eStatus = objResult.eStatus;
        m_objReason = objResult.objReason;
    }

    Result::Status m_eStatus;
    CallReasonInfo m_objReason;
};

class TestRule :
        public IMtcBlockRule
{
public:
    TestRule(IN Result::Status eStatus, IN CallReasonInfo objReason) :
            m_eStatus(eStatus),
            m_objReason(objReason)
    {
    }
    TestRule(IN Result::Status eStatus) :
            TestRule(eStatus, objDefaultReason) {}
    virtual ~TestRule() {}

    Result Check(IN IMtcBlockRuleCheckListener& /* objListener */) override
    {
        return Result(m_eStatus, m_objReason);
    }

private:
    Result::Status m_eStatus;
    CallReasonInfo m_objReason;
};

TEST(MtcBlockCheckerTest, CheckEmptyRuleReturnsUnblocked)
{
    IMSList<IMtcBlockRule*> lstRules;
    MtcBlockChecker objChecker(lstRules, nullptr);

    Result objResult = objChecker.Check();

    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST(MtcBlockCheckerTest, CheckAllUnblockedRuleReturnsUnblocked)
{
    IMSList<IMtcBlockRule*> lstRules;
    lstRules.Append(new TestRule(Result::Status::UNBLOCKED));
    lstRules.Append(new TestRule(Result::Status::UNBLOCKED));
    MtcBlockChecker objChecker(lstRules, nullptr);

    Result objResult = objChecker.Check();

    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST(MtcBlockCheckerTest, CheckSomeBlockedRuleReturnsBlocked)
{
    IMSList<IMtcBlockRule*> lstRules;
    lstRules.Append(new TestRule(Result::Status::UNBLOCKED));
    lstRules.Append(new TestRule(Result::Status::PENDING));
    lstRules.Append(new TestRule(Result::Status::BLOCKED, objReason));
    MtcBlockChecker objChecker(lstRules, nullptr);

    Result objResult = objChecker.Check();

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
}

TEST(MtcBlockCheckerTest, CheckSomePendingRuleReturnsPending)
{
    IMSList<IMtcBlockRule*> lstRules;
    lstRules.Append(new TestRule(Result::Status::UNBLOCKED));
    lstRules.Append(new TestRule(Result::Status::PENDING));
    MtcBlockChecker objChecker(lstRules, nullptr);

    Result objResult = objChecker.Check();

    EXPECT_EQ(Result::Status::PENDING, objResult.eStatus);
}

TEST(MtcBlockCheckerTest, AllPendingRuleUnblockedNotifiesListener)
{
    IMSList<IMtcBlockRule*> lstRules;
    lstRules.Append(new TestRule(Result::Status::PENDING));
    lstRules.Append(new TestRule(Result::Status::PENDING));
    TestListener objListener;
    MtcBlockChecker objChecker(lstRules, &objListener);

    Result objResult = objChecker.Check();
    objChecker.OnBlockRuleChecked(Result(Result::Status::UNBLOCKED, objDefaultReason));
    objChecker.OnBlockRuleChecked(Result(Result::Status::UNBLOCKED, objDefaultReason));

    EXPECT_EQ(Result::Status::UNBLOCKED, objListener.m_eStatus);
}

TEST(MtcBlockCheckerTest, SomePendingRuleUnblockedNotNotifiesListener)
{
    IMSList<IMtcBlockRule*> lstRules;
    lstRules.Append(new TestRule(Result::Status::PENDING));
    lstRules.Append(new TestRule(Result::Status::PENDING));
    TestListener objListener;
    MtcBlockChecker objChecker(lstRules, &objListener);

    Result objResult = objChecker.Check();
    objChecker.OnBlockRuleChecked(Result(Result::Status::UNBLOCKED, objDefaultReason));

    EXPECT_NE(Result::Status::UNBLOCKED, objListener.m_eStatus);
}

TEST(MtcBlockCheckerTest, SomePendingRuleBlockedNotifiesListener)
{
    IMSList<IMtcBlockRule*> lstRules;
    lstRules.Append(new TestRule(Result::Status::PENDING));
    lstRules.Append(new TestRule(Result::Status::PENDING));
    TestListener objListener;
    MtcBlockChecker objChecker(lstRules, &objListener);

    Result objResult = objChecker.Check();
    objChecker.OnBlockRuleChecked(Result(Result::Status::BLOCKED, objReason));

    EXPECT_EQ(Result::Status::BLOCKED, objListener.m_eStatus);
    EXPECT_EQ(objReason.nCode, objListener.m_objReason.nCode);
    EXPECT_EQ(objReason.nExtraCode, objListener.m_objReason.nExtraCode);
}

TEST(MtcBlockCheckerTest, ListenerNotNotifiedAfterNotifiedOnce)
{
    IMSList<IMtcBlockRule*> lstRules;
    lstRules.Append(new TestRule(Result::Status::PENDING));
    lstRules.Append(new TestRule(Result::Status::PENDING));
    TestListener objListener;
    MtcBlockChecker objChecker(lstRules, &objListener);

    Result objResult = objChecker.Check();
    objChecker.OnBlockRuleChecked(Result(Result::Status::BLOCKED, objReason));
    objChecker.OnBlockRuleChecked(Result(Result::Status::UNBLOCKED, objDefaultReason));

    EXPECT_EQ(Result::Status::BLOCKED, objListener.m_eStatus);
}
