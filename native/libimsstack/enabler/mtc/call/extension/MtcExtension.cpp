#include "IMessage.h"
#include "ISipHeader.h"
#include "ServiceTrace.h"
#include "call/extension/MtcExtension.h"
#include "call/extension/MtcExtensionSet.h"
#include "utility/MessageUtil.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcExtension::MtcExtension(IN const AString& strOptionTag) :
        m_strOptionTag(strOptionTag),
        m_bRequiredOnRemote(IMS_FALSE),
        m_bSupportedOnRemote(IMS_FALSE)
{
}

PUBLIC
MtcExtension::MtcExtension(IN const MtcExtension& objRhs) :
        m_strOptionTag(objRhs.m_strOptionTag),
        m_bRequiredOnRemote(objRhs.m_bRequiredOnRemote),
        m_bSupportedOnRemote(objRhs.m_bSupportedOnRemote)
{
}

PUBLIC VIRTUAL
MtcExtension::~MtcExtension()
{
}

PUBLIC VIRTUAL
IMtcExtension* MtcExtension::Clone() const
{
    return new MtcExtension(*this);
}

PUBLIC VIRTUAL
IMS_BOOL MtcExtension::IsAvailableOnRemote() const
{
    return m_bRequiredOnRemote || m_bSupportedOnRemote;
}

PUBLIC VIRTUAL
const AString& MtcExtension::GetOptionTag() const
{
    return m_strOptionTag;
}

PUBLIC VIRTUAL
void MtcExtension::FormatRequest(IN IMS_UINT32 /* nMethod */, IN_OUT IMessage& /* objRequest */)
{
}

PUBLIC VIRTUAL
void MtcExtension::FormatResponse(IN IMS_UINT32 /* nMethod */, IN_OUT IMessage& /* objResponse */)
{
}

PUBLIC VIRTUAL
void MtcExtension::HandleRequest(IN IMS_UINT32 /* nMethod */, IN const IMessage& objRequest)
{
    UpdateFromRequireAndSupportedHeader(objRequest);
}

PUBLIC VIRTUAL
void MtcExtension::HandleResponse(IN IMS_UINT32 /* nMethod */, IN const IMessage& objResponse)
{
    UpdateFromRequireAndSupportedHeader(objResponse);
}

PRIVATE
void MtcExtension::UpdateFromRequireAndSupportedHeader(IN const IMessage& objMessage)
{
    m_bRequiredOnRemote =
            MessageUtil::HasValue(&objMessage, GetOptionTag(), ISIPHeader::REQUIRE);
    m_bSupportedOnRemote =
            MessageUtil::HasValue(&objMessage, GetOptionTag(), ISIPHeader::SUPPORTED);

    IMS_TRACE_D("UpdateFromRequireAndSupportedHeader : Tag[%s] Require[%s] Supported[%s]",
            m_strOptionTag.GetStr(),
            _TRACE_B_(m_bRequiredOnRemote),
            _TRACE_B_(m_bSupportedOnRemote));
}
