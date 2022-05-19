#ifndef MOCK_I_SUBSCRIBER_CONFIG_H_
#define MOCK_I_SUBSCRIBER_CONFIG_H_

#include <gmock/gmock.h>

#include "IMSVector.h"
#include "CarrierConfig.h"
#include "IAsyncConfig.h"
#include "IImsSubscriberInfo.h"
#include "ISubscriberConfig.h"

class MockISubscriberConfig : public ISubscriberConfig
{
public:
    MOCK_METHOD(ServerAddress*, GetPcscfAddress, (), (const, override));
    MOCK_METHOD(const IMSVector<ServerAddress*>&, GetPcscfAddresses, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPcscfDiscoveryMethod, (), (const, override));
    MOCK_METHOD(const IMSVector<IMS_SINT32>&, GetPcscfDiscoveryMethods, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetSubscriberCount, (), (const, override));
    MOCK_METHOD(IImsSubscriberInfo*, GetSubscriberInfo, (IN IMS_SINT32 nIndex), (const, override));
    MOCK_METHOD(IMS_BOOL, IsAkaSupported, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsDebugOn, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsServiceAllowed, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsIsimSupported, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsProvisioningDone, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsUsimSupported, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsTestMode, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetSubscriptionAttributes, (), (const, override));
    MOCK_METHOD(IConfigurable*, GetConfigurable, (), (const, override));
    MOCK_METHOD(
            void, RemoveListener, (IN ISubscriberConfigListener * piListener), (const, override));
    MOCK_METHOD(void, SetListener, (IN ISubscriberConfigListener * piListener), (const, override));
    MOCK_METHOD(const Credential&, GetCredential, (), (const, override));
    MOCK_METHOD(const AString&, GetHomeDomainName, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetIndexOfPrimaryPublicUserId, (), (const, override));
    MOCK_METHOD(const AString&, GetPhoneContext, (), (const, override));
    MOCK_METHOD(const AString&, GetPrivateUserId, (), (const, override));
    MOCK_METHOD(const AString&, GetPublicUserId, (IN IMS_SINT32 nImpuType), (const, override));
    MOCK_METHOD(const AStringArray&, GetPublicUserIds, (), (const, override));

    MOCK_METHOD(void, HandleMessage,
            (IN IMS_SINT32 nMSG, IN IMS_SINTP nParam1, IN IMS_SINTP nParam2), (override));
};

#endif  // MOCK_I_SUBSCRIBER_CONFIG_H_
