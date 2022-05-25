#ifndef MOCK_I_IMS_AOS_LISTENER_H_
#define MOCK_I_IMS_AOS_LISTENER_H_

#include <gmock/gmock.h>

#include "IMSTypeDef.h"

#include "IImsAosListener.h"

class MockIImsAosListener : public IImsAosListener
{
public:
    MOCK_METHOD(
            void, ImsAos_Connected, (IN IMS_UINT32 nFeatures, IN IMS_UINT32 nIpcan), (override));
    MOCK_METHOD(void, ImsAos_Connecting, (), (override));
    MOCK_METHOD(void, ImsAos_Disconnecting, (IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(void, ImsAos_Disconnected, (IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(void, ImsAos_Suspended, (IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(void, ImsAos_Resumed, (), (override));
};

#endif  // MOCK_I_IMS_AOS_LISTENER_H_
