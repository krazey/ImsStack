#ifndef MOCK_SESSION_INTERFACE_HOLDER_H_
#define MOCK_SESSION_INTERFACE_HOLDER_H_

#include <gmock/gmock.h>

#include "SessionInterfaceHolder.h"
class ISession;
class ICoreService;
class IInterfaceHolderListener;

class MockSessionInterfaceHolder : public SessionInterfaceHolder
{
public:
    explicit MockSessionInterfaceHolder(IN IInterfaceHolderListener& objListener) :
            SessionInterfaceHolder(objListener)
    {
    }
    ~MockSessionInterfaceHolder() {}
    MOCK_METHOD(void, SessionAlerting, (IN ISession*), (override));
    MOCK_METHOD(void, SessionReferenceReceived, (IN ISession*, IN IReference*), (override));
    MOCK_METHOD(void, SessionStarted, (IN ISession*), (override));
    MOCK_METHOD(void, SessionStartFailed, (IN ISession*), (override));
    MOCK_METHOD(void, SessionTerminated, (IN ISession * piSession), (override));
    MOCK_METHOD(void, SessionUpdated, (IN ISession*), (override));
    MOCK_METHOD(void, SessionUpdateFailed, (IN ISession*), (override));
    MOCK_METHOD(void, SessionUpdateReceived, (IN ISession*), (override));
    MOCK_METHOD(void, SessionCancelDelivered, (IN ISession*), (override));
    MOCK_METHOD(void, SessionCancelDeliveryFailed, (IN ISession*), (override));
    MOCK_METHOD(void, SessionEarlyMediaUpdated, (IN ISession*), (override));
    MOCK_METHOD(void, SessionEarlyMediaUpdateFailed, (IN ISession*), (override));
    MOCK_METHOD(void, SessionEarlyMediaUpdateReceived, (IN ISession*), (override));
    MOCK_METHOD(void, SessionForkedResponseReceived, (IN ISession*, IN ISession*), (override));
    MOCK_METHOD(void, SessionPrackDelivered, (IN ISession*), (override));
    MOCK_METHOD(void, SessionPrackDeliveryFailed, (IN ISession*), (override));
    MOCK_METHOD(void, SessionPrackReceived, (IN ISession*), (override));
    MOCK_METHOD(
            void, SessionProvisionalResponseReceived, (IN ISession*, IN IMS_UINT32), (override));
    MOCK_METHOD(void, SessionRprDeliveryFailed, (IN ISession*), (override));
    MOCK_METHOD(void, SessionRprReceived, (IN ISession*, IN IMS_UINT32), (override));
    MOCK_METHOD(
            void, SessionTransactionReceived, (IN ISession*, IN ISipServerConnection*), (override));
    MOCK_METHOD(void, Timer_TimerExpired, (IN ITimer * piTimer), (override));
    MOCK_METHOD(ISession*, GetISession,
            (IN ICoreService * pICoreService, IN const AString& strFrom, IN const AString& strTo),
            (override));
    MOCK_METHOD(void, AddISession, (IN ISession * piSession), (override));
    MOCK_METHOD(
            void, ReleaseISession, (IN ISession * piSession, IN IMS_BOOL bTerminated), (override));
    MOCK_METHOD(IMS_BOOL, IsReadyToDestroy, (IN ISession * piSession), ());
    MOCK_METHOD(void, ClearISessions, (), ());
    MOCK_METHOD(IMS_RESULT, StartTimer, (IN ISession * piSession, IN IMS_SINT32 nDuration), ());
    MOCK_METHOD(void, StopTimer, (IN ITimer * piTimer), ());
    MOCK_METHOD(ITimer*, GetTimer, (IN ISession * piSession), ());
};

#endif
