#ifndef MOCK_AOS_APP_CONTEXT_H_
#define MOCK_AOS_APP_CONTEXT_H_

#include <gmock/gmock.h>

#include "interface/IAosAppContext.h"
#include "app/AosAppContext.h"

class MockAosAppContext : public AosAppContext {
public:
    // MockAosAppContext() {}
    // MockAosAppContext([[maybe_unused]] const MockAosAppContext& o) {}

    MockAosAppContext(IN AosStaticProfile * pProfile) : AosAppContext(pProfile) { }
    ~MockAosAppContext() { }
    // MockAosAppContext(IN CONST MockAosAppContext & objRHS)
    //     : AosAppContext(objRHS) { } // private constructor/destructor
    MOCK_METHOD(IMS_SINT32, GetSlotId, (), (const, override));
    MOCK_METHOD(const AString&, GetProfileId, (), (const, override));
    MOCK_METHOD(IAosHandle*, GetHandle, (IN CONST AString& strSrvId), (const, override));
    MOCK_METHOD(IAosHandle*, GetHandle, (IN IMS_UINT32 nServiceType), (override));
    MOCK_METHOD((IMSMap<AString, IAosHandle*>&), GetHandles, (), (override));
    MOCK_METHOD(IAosApplication*, GetApp, (), (const, override));
    MOCK_METHOD(IAosConnection*, GetConnection, (), (const, override));
    MOCK_METHOD(IAosRegistration*, GetRegistration, (), (const, override));
    MOCK_METHOD(IAosNetTracker*, GetNetTracker, (), (const, override));
    MOCK_METHOD(IAosBlock*, GetBlock, (), (const, override));
    MOCK_METHOD(IAosSubscriber*, GetSubscriber, (), (const, override));
    MOCK_METHOD(IAosPcscf*, GetPcscf, (), (const, override));
    MOCK_METHOD(AosStaticProfile*, GetStaticProfile, (), (const, override));
    MOCK_METHOD(void, SetSlotId, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(void, AddHandle, (IN CONST AString& strSrvId, IN IAosHandle* piHandle),
            (override));
    MOCK_METHOD(void, SetApp, (IN IAosApplication* piApp), (override));
    MOCK_METHOD(void, SetConnection, (IN IAosConnection* piConnection), (override));
    MOCK_METHOD(void, SetRegistration, (IN IAosRegistration* piRegistration), (override));
    MOCK_METHOD(void, SetNetTracker, (IN IAosNetTracker* piNetTracker), (override));
    MOCK_METHOD(void, SetBlock, (IN IAosBlock* piBlock), (override));
    MOCK_METHOD(void, SetSubscriber, (IN IAosSubscriber* piSubscriber), (override));
    MOCK_METHOD(void, SetPcscf, (IN IAosPcscf* piPcscf), (override));
};

#endif // MOCK_AOS_APP_CONTEXT_H_
