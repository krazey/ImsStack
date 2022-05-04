#ifndef MOCK_I_AOS_CALL_TRACKER_H_
#define MOCK_I_AOS_CALL_TRACKER_H_

#include <gmock/gmock.h>

#include "interface/IAosCallTracker.h"

class MockIAosCallTracker : public IAosCallTracker {
public:
    MOCK_METHOD(IMS_BOOL, IsCSCallActive, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsNormalCallActive, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsEmergencyCallActive, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsVideoCallingActive, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetSlotId, (), (const, override));
    MOCK_METHOD(IMS_UINT32, GetCallState, (IN IMS_UINT32 nType), (const, override));
    MOCK_METHOD(IMS_UINT32, GetSessionType, (IN IMS_UINT32 nType), (const, override));
    MOCK_METHOD(void, SetCSCallStateWatchMode, (), (override));
    MOCK_METHOD(void, SetActiveCSCallState, (IN IMS_UINT32 nActiveCSState), (override));
    MOCK_METHOD(void, SetListener, (IN IAosCallTrackerListener* piListener), (override));
    MOCK_METHOD(void, RemoveListener, (IN IAosCallTrackerListener* piListener), (override));
};

#endif //MOCK_I_AOS_CALL_TRACKER_H_
