#ifndef MOCK_I_AOS_BLOCK_H_
#define MOCK_I_AOS_BLOCK_H_

#include <gmock/gmock.h>

#include "IMSTypeDef.h"
#include "interface/IAosBlock.h"

class MockIAosBlock : public IAosBlock {
public:
    MOCK_METHOD(void, SetListener, (IN IAosBlockListener* piListener), (override));
    MOCK_METHOD(void, RemoveListener, (IN IAosBlockListener* piListener), (override));
    MOCK_METHOD(IMS_BOOL, SetBlockReason, (IN BLOCK_REASON eReason, IN IMS_BOOL bNotify),
            (override));
    MOCK_METHOD(IMS_BOOL, ResetBlockReason, (IN BLOCK_REASON eReason, IN IMS_BOOL bNotify),
            (override));
    MOCK_METHOD(void, ClearAllBlockReasons, (), (override));
    MOCK_METHOD(IMS_BOOL, PrintBlockReasons, (), (override));
    MOCK_METHOD(void, GetBlockReasons, (OUT IMSList<IMS_UINT32>& objReasons,
            IN SERVICE_TYPE eType), (override));
    MOCK_METHOD(IMS_BOOL, IsReasonBlocked, (IN BLOCK_REASON eReason, IN IMS_BOOL bOnlyEnabled,
            IN SERVICE_TYPE eType), (override));
    MOCK_METHOD(IMS_BOOL, IsCleared, (IN SERVICE_TYPE eType), (override));
};

#endif // MOCK_I_AOS_BLOCK_H_
