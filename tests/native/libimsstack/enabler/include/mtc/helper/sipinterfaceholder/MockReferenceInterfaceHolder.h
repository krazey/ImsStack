#ifndef MOCK_REFERENCE_INTERFACE_HOLDER_H_
#define MOCK_REFERENCE_INTERFACE_HOLDER_H_

#include <gmock/gmock.h>

#include "ReferenceInterfaceHolder.h"
class ISession;
class IReference;
class IInterfaceHolderListener;

class MockReferenceInterfaceHolder : public ReferenceInterfaceHolder
{
public:
    explicit MockReferenceInterfaceHolder(IN IInterfaceHolderListener& objListener) :
            ReferenceInterfaceHolder(objListener)
    {
    }
    ~MockReferenceInterfaceHolder() {}
    MOCK_METHOD(void, ReferenceDelivered, (IN IReference*), (override));
    MOCK_METHOD(void, ReferenceDeliveryFailed, (IN IReference*), (override));
    MOCK_METHOD(void, ReferenceNotify, (IN IReference*, IN IMessage*), (override));
    MOCK_METHOD(void, ReferenceTerminated, (IN IReference * piReference), (override));
    MOCK_METHOD(void, Timer_TimerExpired, (IN ITimer * piTimer), (override));
    MOCK_METHOD(IReference*, GetIReference,
            (IN ISession * piSession, IN const AString& strReferTo, IN const AString& strMethod),
            (override));
    MOCK_METHOD(void, ReleaseIReference, (IN IReference * piReference, IN IMS_BOOL bTerminated),
            (override));
    MOCK_METHOD(IMS_BOOL, IsReadyToDestroy, (IN IReference * piReference), ());
    MOCK_METHOD(void, ClearIReferences, (), ());
    MOCK_METHOD(IMS_RESULT, StartTimer, (IN IReference * piReference, IN IMS_SINT32 nDuration), ());
    MOCK_METHOD(void, StopTimer, (IN ITimer * piTimer), ());
    MOCK_METHOD(ITimer*, GetTimer, (IN IReference * piReference), ());
};

#endif
