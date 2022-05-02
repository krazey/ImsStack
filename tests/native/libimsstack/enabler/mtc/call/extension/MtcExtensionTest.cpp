#include "IMessage.h"
#include "ISipMessage.h"
#include "IMSList.h"
#include "call/extension/MtcExtension.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

AString strOptionTag = "some_tag";

// FIXME: ISIPMessage and IMessage cannot be used for test now because they don't have
//        virtual destructor. I commented related tests below too.
#if 0
class FakeSipMessage :
        public ISIPMessage
{
public:
    FakeSipMessage(IMS_BOOL bContainsOptionTag) :
            m_bContainsOptionTag(bContainsOptionTag) {}
    virtual ~FakeSipMessage() {}

    MOCK_METHOD(void, Destroy, ());
    MOCK_METHOD(ISIPMessage*, Clone, (), (const));
    MOCK_METHOD(IMS_RESULT, AddHeader, (IN IMS_SINT32, const AString&, const AString&));
    MOCK_METHOD(IMS_UINT32, GetCSeqNumber, (), (const));
    MOCK_METHOD(AString, GetHeader, (IN IMS_SINT32, IN IMS_SINT32, const AString&), (const));
    MOCK_METHOD(IMS_SINT32, GetHeaderCount, (IN IMS_SINT32, const AString&), (const));
    MOCK_METHOD(const SIPMethod&, GetMethod, (), (const));
    MOCK_METHOD(const AString&, GetReasonPhrase, (), (const));
    MOCK_METHOD(const AString&, GetRequestURI, (), (const));
    MOCK_METHOD(IMS_SINT32, GetStatusCode, (), (const));
    MOCK_METHOD(IMS_SINT32, GetType, (), (const));
    MOCK_METHOD(IMS_RESULT, PrependHeader, (IN IMS_SINT32, const AString&, const AString&));
    MOCK_METHOD(void, RemoveHeader, (IN IMS_SINT32, const AString&));
    MOCK_METHOD(IMS_RESULT, SetHeader, (IN IMS_SINT32, const AString&, const AString&));
    MOCK_METHOD(ISIPMessageBodyPart*, CreateBodyPart, ());
    MOCK_METHOD(ISIPMessageBodyPart*, CreateSDPBodyPart,  ());
    MOCK_METHOD(IMSList<ISIPMessageBodyPart*>, GetBodyParts, (), (const));
    MOCK_METHOD(ISIPMessageBodyPart*, GetSDPBodyPart, (), (const));
    MOCK_METHOD(IMSList<ISIPMessageBodyPart*>, GetSDPBodyParts, (), (const));
    MOCK_METHOD(IMS_RESULT, CopyHeadersAndBodyParts, (const ISIPMessage*));
    MOCK_METHOD(IMS_BOOL, IsHeaderPresent, (IN IMS_SINT32, const AString&), (const));
    MOCK_METHOD(IMS_BOOL, IsMessageRPR, (), (const));
    MOCK_METHOD(IMS_BOOL, IsOptionRequired, (const AString&), (const));
    MOCK_METHOD(IMS_BOOL, IsOptionSupported, (const AString&), (const));
    MOCK_METHOD(void, RemoveBodyParts, ());
    MOCK_METHOD(ByteArray, ToByteArray, (IMS_SINT32), (const));

    IMSList<AString> GetHeaders(
            IN IMS_SINT32 /* nType */, const AString& /* strName */) const override
    {
        IMSList<AString> lstHeaders;
        if (m_bContainsOptionTag)
        {
            lstHeaders.Append(strOptionTag);
        }
        return lstHeaders;
    }

    IMS_BOOL m_bContainsOptionTag;
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
    MOCK_METHOD(const SIPMethod&, GetMethod, (), (const));
    MOCK_METHOD(const AString&, GetReasonPhrase, (), (const));
    MOCK_METHOD(IMS_SINT32, GetState, (), (const));
    MOCK_METHOD(IMS_SINT32, GetStatusCode, (), (const));

    ISIPMessage* GetMessage() const override
    {
        return m_pSipMessage;
    }

    FakeSipMessage* m_pSipMessage;
};
#endif

TEST(MtcExtensionTest, CopyConstructorTest)
{
    MtcExtension objExtension(strOptionTag);
    MtcExtension objCopiedExtension(objExtension);

    EXPECT_STREQ(
            objExtension.GetOptionTag().GetStr(),
            objCopiedExtension.GetOptionTag().GetStr());
    EXPECT_EQ(
            objExtension.IsAvailableOnRemote(),
            objCopiedExtension.IsAvailableOnRemote());
}

TEST(MtcExtensionTest, CloneTest)
{
    MtcExtension objExtension(strOptionTag);
    IMtcExtension* pCopiedExtension = objExtension.Clone();

    EXPECT_STREQ(
            objExtension.GetOptionTag().GetStr(),
            pCopiedExtension->GetOptionTag().GetStr());
    EXPECT_EQ(
            objExtension.IsAvailableOnRemote(),
            pCopiedExtension->IsAvailableOnRemote());

    delete pCopiedExtension;
}

TEST(MtcExtensionTest, InitialAvailabilityFalest)
{
    MtcExtension objExtension(strOptionTag);

    EXPECT_FALSE(objExtension.IsAvailableOnRemote());
}

TEST(MtcExtensionTest, GetOptionTagTest)
{
    MtcExtension objExtension(strOptionTag);

    EXPECT_STREQ(
            strOptionTag.GetStr(),
            objExtension.GetOptionTag().GetStr());
}

/*
TEST(MtcExtensionTest, FormatRequestDoNothingTest)
{
    MtcExtension objExtension(strOptionTag);
    IMessage* pMessageContainsTag = new FakeMessage(new FakeSipMessage(IMS_TRUE));
    IMessage* pMessageNotContainsTag = new FakeMessage(new FakeSipMessage(IMS_FALSE));

    objExtension.FormatRequest(IMessage::SESSION_START, *pMessageContainsTag);
    EXPECT_FALSE(objExtension.IsAvailableOnRemote());

    objExtension.FormatRequest(IMessage::SESSION_START, *pMessageNotContainsTag);
    EXPECT_FALSE(objExtension.IsAvailableOnRemote());

    delete pMessageContainsTag;
    delete pMessageNotContainsTag;
}

TEST(MtcExtensionTest, FormatResponseDoNothingTest)
{
    MtcExtension objExtension(strOptionTag);
    IMessage* pMessageContainsTag = new FakeMessage(new FakeSipMessage(IMS_TRUE));
    IMessage* pMessageNotContainsTag = new FakeMessage(new FakeSipMessage(IMS_FALSE));

    objExtension.FormatResponse(IMessage::SESSION_START, *pMessageContainsTag);
    EXPECT_FALSE(objExtension.IsAvailableOnRemote());

    objExtension.FormatResponse(IMessage::SESSION_START, *pMessageNotContainsTag);
    EXPECT_FALSE(objExtension.IsAvailableOnRemote());

    delete pMessageContainsTag;
    delete pMessageNotContainsTag;
}

TEST(MtcExtensionTest, HandlePrackRequestDoNothingTest)
{
    MtcExtension objExtension(strOptionTag);
    IMessage* pMessageContainsTag = new FakeMessage(new FakeSipMessage(IMS_TRUE));
    IMessage* pMessageNotContainsTag = new FakeMessage(new FakeSipMessage(IMS_FALSE));

    objExtension.HandleRequest(IMessage::SESSION_PRACK, *pMessageContainsTag);
    EXPECT_FALSE(objExtension.IsAvailableOnRemote());

    objExtension.HandleRequest(IMessage::SESSION_PRACK, *pMessageNotContainsTag);
    EXPECT_FALSE(objExtension.IsAvailableOnRemote());

    delete pMessageContainsTag;
    delete pMessageNotContainsTag;
}

TEST(MtcExtensionTest, HandlePrackResponseDoNothingTest)
{
    MtcExtension objExtension(strOptionTag);
    IMessage* pMessageContainsTag = new FakeMessage(new FakeSipMessage(IMS_TRUE));
    IMessage* pMessageNotContainsTag = new FakeMessage(new FakeSipMessage(IMS_FALSE));

    objExtension.HandleResponse(IMessage::SESSION_PRACK, *pMessageContainsTag);
    EXPECT_FALSE(objExtension.IsAvailableOnRemote());

    objExtension.HandleResponse(IMessage::SESSION_PRACK, *pMessageNotContainsTag);
    EXPECT_FALSE(objExtension.IsAvailableOnRemote());

    delete pMessageContainsTag;
    delete pMessageNotContainsTag;
}

TEST(MtcExtensionTest, HandleRequestUpdateAvailabilityTest)
{
    MtcExtension objExtension(strOptionTag);
    IMessage* pMessageContainsTag = new FakeMessage(new FakeSipMessage(IMS_TRUE));
    IMessage* pMessageNotContainsTag = new FakeMessage(new FakeSipMessage(IMS_FALSE));

    objExtension.HandleRequest(IMessage::SESSION_START, *pMessageContainsTag);
    EXPECT_TRUE(objExtension.IsAvailableOnRemote());

    objExtension.HandleRequest(IMessage::SESSION_START, *pMessageNotContainsTag);
    EXPECT_FALSE(objExtension.IsAvailableOnRemote());

    delete pMessageContainsTag;
    delete pMessageNotContainsTag;
}

TEST(MtcExtensionTest, HandleResponseUpdateAvailabilityTest)
{
    MtcExtension objExtension(strOptionTag);
    IMessage* pMessageContainsTag = new FakeMessage(new FakeSipMessage(IMS_TRUE));
    IMessage* pMessageNotContainsTag = new FakeMessage(new FakeSipMessage(IMS_FALSE));

    objExtension.HandleResponse(IMessage::SESSION_START, *pMessageContainsTag);
    EXPECT_TRUE(objExtension.IsAvailableOnRemote());

    objExtension.HandleResponse(IMessage::SESSION_START, *pMessageNotContainsTag);
    EXPECT_FALSE(objExtension.IsAvailableOnRemote());

    delete pMessageContainsTag;
    delete pMessageNotContainsTag;
}
*/
