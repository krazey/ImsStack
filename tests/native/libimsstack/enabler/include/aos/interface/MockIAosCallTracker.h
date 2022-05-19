#ifndef MOCK_I_AOS_CALL_TRACKER_H_
#define MOCK_I_AOS_CALL_TRACKER_H_

#include <gmock/gmock.h>

#include "interface/IAosCallTracker.h"

class MockIAosCallTracker : public IAosCallTracker {
public:
    MOCK_METHOD(IMS_BOOL, IsCsCallActive, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsNormalCallActive, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsEmergencyCallActive, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsVideoCallingActive, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetSlotId, (), (const, override));
    MOCK_METHOD(IMS_UINT32, GetCallState, (IN IMS_UINT32 nType), (const, override));
    MOCK_METHOD(IMS_UINT32, GetSessionType, (IN IMS_UINT32 nType), (const, override));
    MOCK_METHOD(void, SetCsCallStateWatchMode, (), (override));
    MOCK_METHOD(void, SetActiveCsCallState, (IN IMS_UINT32 nActiveCsState), (override));
    MOCK_METHOD(void, SetListener, (IN IAosCallTrackerListener* piListener), (override));
    MOCK_METHOD(void, RemoveListener, (IN IAosCallTrackerListener* piListener), (override));
};

#endif //MOCK_I_AOS_CALL_TRACKER_H_
