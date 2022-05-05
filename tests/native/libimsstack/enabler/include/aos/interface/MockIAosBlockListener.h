#ifndef MOCK_I_AOS_BLOCK_LISTENER_H_
#define MOCK_I_AOS_BLOCK_LISTENER_H_

#include <gmock/gmock.h>

#include "IMSTypeDef.h"
#include "interface/IAosBlockListener.h"

class MockIAosBlockListener : public IAosBlockListener {
public:
    MOCK_METHOD(void, Block_Changed, (IN IMS_UINT32 nType, IN IMS_UINT32 nParam), (override));
};

#endif // MOCK_I_AOS_BLOCK_LISTENER_H_
