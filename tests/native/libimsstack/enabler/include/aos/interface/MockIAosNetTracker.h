#ifndef MOCK_I_AOS_NET_TRACKER_H_
#define MOCK_I_AOS_NET_TRACKER_H_

#include <gmock/gmock.h>

#include "IMSTypeDef.h"
#include "interface/IAosNetTracker.h"

class MockIAosNetTracker : public IAosNetTracker
{
public:
    MOCK_METHOD(IMS_BOOL, IsServiceIN, (IN IMS_UINT32 nType), (override));
    MOCK_METHOD(IMS_BOOL, IsDataIn, (), (override));
    MOCK_METHOD(IMS_BOOL, IsNetworkIn, (), (override));
    MOCK_METHOD(IMS_BOOL, IsEmergencyLteAttach, (), (override));
    MOCK_METHOD(IMS_BOOL, IsSuspended, (), (override));
    MOCK_METHOD(IMS_BOOL, IsSessionContinuitySupported, (), (override));
    MOCK_METHOD(IMS_BOOL, IsServiceTimerRunning, (), (override));
    MOCK_METHOD(IMS_UINT32, GetMobileChangingNetworkType, (), (override));
    MOCK_METHOD(IMS_UINT32, GetMobileNetworkType, (), (override));
    MOCK_METHOD(IMS_SINT32, GetMobileVoiceServiceState, (), (override));
    MOCK_METHOD(IMS_UINT32, GetMobileVoiceNetworkType, (), (override));
    MOCK_METHOD(IMS_UINT32, GetNetworkType, (), (override));
    MOCK_METHOD(void, SetRatGuardTime, (IN IMS_UINT32 nGuardTime), (override));
    MOCK_METHOD(void, SetSrvOutGuardTime, (IN IMS_UINT32 nGuardTime), (override));
    MOCK_METHOD(void, SetSrvInGuardTime, (IN IMS_UINT32 nGuardTime), (override));
    MOCK_METHOD(void, SetListener, (IN IAosNetTrackerListener * piListener), (override));
    MOCK_METHOD(void, RemoveListener, (IN IAosNetTrackerListener * piListener), (override));
};

#endif  // MOCK_I_AOS_NET_TRACKER_H_
