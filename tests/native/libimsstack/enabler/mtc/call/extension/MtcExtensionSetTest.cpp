#include "IMessage.h"
#include "ISipMessage.h"
#include "ImsList.h"
#include "call/extension/MtcExtensionSet.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

MtcExtensionSet CreateExtensionSetSupportsRprOnly()
{
    IMSList<AString> lstOptionTags;
    lstOptionTags.Append(MtcExtensionSet::OPTION_TAG_RPR);

    return MtcExtensionSet(lstOptionTags);
}

MtcExtensionSet CreateExtensionSetSupportsTimerOnly()
{
    IMSList<AString> lstOptionTags;
    lstOptionTags.Append(MtcExtensionSet::OPTION_TAG_TIMER);

    return MtcExtensionSet(lstOptionTags);
}

// FIXME: ISipMessage and IMessage cannot be used for test now because they don't have
//        virtual destructor. I commented related tests below too.
#if 0
class FakeSipMessage :
        public ISipMessage
{
public:
    FakeSipMessage(IN const IMSList<AString>& lstRequiredHeaders) :
            m_lstRequiredHeaders(lstRequiredHeaders) {}
    virtual ~FakeSipMessage() {}

    MOCK_METHOD(void, Destroy, ());
    MOCK_METHOD(ISipMessage*, Clone, (), (const));
    MOCK_METHOD(IMS_RESULT, AddHeader, (IMS_SINT32, const AString&, const AString&));
    MOCK_METHOD(IMS_UINT32, GetCSeqNumber, (), (const));
    MOCK_METHOD(AString, GetHeader, (IMS_SINT32, IMS_SINT32, const AString&), (const));
    MOCK_METHOD(IMS_SINT32, GetHeaderCount, (IMS_SINT32, const AString&), (const));
    MOCK_METHOD(const SipMethod&, GetMethod, (), (const));
    MOCK_METHOD(const AString&, GetReasonPhrase, (), (const));
    MOCK_METHOD(const AString&, GetRequestUri, (), (const));
    MOCK_METHOD(IMS_SINT32, GetStatusCode, (), (const));
    MOCK_METHOD(IMS_SINT32, GetType, (), (const));
    MOCK_METHOD(IMS_RESULT, PrependHeader, (IMS_SINT32, const AString&, const AString&));
    MOCK_METHOD(void, RemoveHeader, (IMS_SINT32, const AString&));
    MOCK_METHOD(IMS_RESULT, SetHeader, (IMS_SINT32, const AString&, const AString&));
    MOCK_METHOD(ISipMessageBodyPart*, CreateBodyPart, ());
    MOCK_METHOD(ISipMessageBodyPart*, CreateSdpBodyPart,  ());
    MOCK_METHOD(IMSList<ISipMessageBodyPart*>, GetBodyParts, (), (const));
    MOCK_METHOD(ISipMessageBodyPart*, GetSdpBodyPart, (), (const));
    MOCK_METHOD(IMSList<ISipMessageBodyPart*>, GetSdpBodyParts, (), (const));
    MOCK_METHOD(IMS_RESULT, CopyHeadersAndBodyParts, (const ISipMessage*));
    MOCK_METHOD(IMS_BOOL, IsHeaderPresent, (IMS_SINT32, const AString&), (const));
    MOCK_METHOD(IMS_BOOL, IsMessageRpr, (), (const));
    MOCK_METHOD(IMS_BOOL, IsOptionRequired, (const AString&), (const));
    MOCK_METHOD(IMS_BOOL, IsOptionSupported, (const AString&), (const));
    MOCK_METHOD(void, RemoveBodyParts, ());
    MOCK_METHOD(ByteArray, ToByteArray, (IMS_SINT32), (const));

    IMSList<AString> GetHeaders(IMS_SINT32 /* nType */, const AString& /* strName */) const override
    {
        return m_lstRequiredHeaders;
    }

    IMSList<AString> m_lstRequiredHeaders;
};

class FakeMessage :
        public IMessage
{
public:
    explicit FakeMessage(FakeSipMessage* pSipMessage) :
            m_pSipMessage(pSipMessage) {}

    virtual ~FakeMessage()
    {
        delete m_pSipMessage;
    }

    MOCK_METHOD(IMS_RESULT, AddHeader, (const AString&, const AString&));
    MOCK_METHOD(IMessageBodyPart*, CreateBodyPart, ());
    MOCK_METHOD(IMSList<IMessageBodyPart*>, GetBodyParts, (), (const));
    MOCK_METHOD(IMSList<AString>, GetHeaders, (const AString&), (const));
    MOCK_METHOD(const SipMethod&, GetMethod, (), (const));
    MOCK_METHOD(const AString&, GetReasonPhrase, (), (const));
    MOCK_METHOD(IMS_SINT32, GetState, (), (const));
    MOCK_METHOD(IMS_SINT32, GetStatusCode, (), (const));

    ISipMessage* GetMessage() const override
    {
        return m_pSipMessage;
    }

    FakeSipMessage* m_pSipMessage;
};
#endif

TEST(MtcExtensionSetTest, CopyConstructor)
{
    MtcExtensionSet objExtensionSet = CreateExtensionSetSupportsRprOnly();
    MtcExtensionSet objCopiedExtensionSet(objExtensionSet);

    EXPECT_EQ(
            objExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_RPR),
            objCopiedExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_RPR));
    EXPECT_EQ(
            objExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_TIMER),
            objCopiedExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_TIMER));
}

TEST(MtcExtensionSetTest, AssignOperator)
{
    MtcExtensionSet objAssignedExtensionSet = CreateExtensionSetSupportsTimerOnly();
    MtcExtensionSet objExtensionSet = CreateExtensionSetSupportsRprOnly();

    objAssignedExtensionSet = objExtensionSet;

    EXPECT_TRUE(objAssignedExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_RPR));
    EXPECT_FALSE(objAssignedExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_TIMER));
}

TEST(MtcExtensionSetTest, IsAvailableOnBothInitiallyReturnsFalse)
{
    MtcExtensionSet objExtensionSet = CreateExtensionSetSupportsRprOnly();

    EXPECT_FALSE(objExtensionSet.IsAvailableOnBoth(MtcExtensionSet::OPTION_TAG_RPR));
    EXPECT_FALSE(objExtensionSet.IsAvailableOnBoth(MtcExtensionSet::OPTION_TAG_TIMER));
}

TEST(MtcExtensionSetTest, IsAvailableOnLocalInitiallyReturnsInitialValue)
{
    MtcExtensionSet objExtensionSet = CreateExtensionSetSupportsRprOnly();

    EXPECT_TRUE(objExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_RPR));
    EXPECT_FALSE(objExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_TIMER));
}

/*
TEST(MtcExtensionSetTest, IsSupportRequiredExtensionsReturnsFalseForNotAvailableExtension)
{
    MtcExtensionSet objExtensionSet = CreateExtensionSetSupportsRprOnly();

    IMSList<AString> lstRequiredHeaders;
    lstRequiredHeaders.Append(MtcExtensionSet::OPTION_TAG_RPR);
    IMessage* pMessageRequiresRpr = new FakeMessage(new FakeSipMessage(lstRequiredHeaders));

    EXPECT_TRUE(objExtensionSet.IsSupportRequiredExtensions(*pMessageRequiresRpr));

    delete pMessageRequiresRpr;
}

TEST(MtcExtensionSetTest, IsSupportRequiredExtensionsReturnsTrueForAvailableExtension)
{
    MtcExtensionSet objExtensionSet = CreateExtensionSetSupportsRprOnly();

    IMSList<AString> lstRequiredHeaders;
    lstRequiredHeaders.Append(MtcExtensionSet::OPTION_TAG_TIMER);
    IMessage* pMessageRequiresTimer = new FakeMessage(new FakeSipMessage(lstRequiredHeaders));

    EXPECT_FALSE(objExtensionSet.IsSupportRequiredExtensions(*pMessageRequiresTimer));

    delete pMessageRequiresTimer;
}
*/
