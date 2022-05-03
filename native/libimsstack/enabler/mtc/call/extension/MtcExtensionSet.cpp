#include "IMSList.h"
#include "ISipHeader.h"
#include "ServiceTrace.h"
#include "call/extension/IMtcExtension.h"
#include "call/extension/MtcExtension.h"
#include "call/extension/MtcExtensionSet.h"
#include "call/extension/PreconditionExtension.h"
#include "call/extension/RprExtension.h"
#include "utility/MessageUtil.h"

__IMS_TRACE_TAG_COM_MTC__;

const AString MtcExtensionSet::OPTION_TAG_EARLY_DIALOG_TERMINATED = "199";
const AString MtcExtensionSet::OPTION_TAG_FROM_CHANGE = "from-change";
const AString MtcExtensionSet::OPTION_TAG_HISTORY_INFO = "histinfo";
const AString MtcExtensionSet::OPTION_TAG_PRECONDITION = "precondition";
const AString MtcExtensionSet::OPTION_TAG_REPLACES = "replaces";
const AString MtcExtensionSet::OPTION_TAG_RPR = "100rel";
const AString MtcExtensionSet::OPTION_TAG_TARGET_DIALOG = "tdialog";
const AString MtcExtensionSet::OPTION_TAG_TIMER = "timer";

PUBLIC
MtcExtensionSet::MtcExtensionSet(IN const IMSList<AString>& lstOptionTags)
{
    for (IMS_UINT32 nIndex = 0; nIndex < lstOptionTags.GetSize(); nIndex++)
    {
        const AString& strOptionTag = lstOptionTags.GetAt(nIndex);
        IMtcExtension* pExtension = CreateExtension(strOptionTag);
        if (pExtension)
        {
            m_objExtensions.Add(strOptionTag, pExtension);
        }
    }
}

PUBLIC
MtcExtensionSet::MtcExtensionSet(IN const MtcExtensionSet& objRhs)
{
    CopyFrom(objRhs);
}

PUBLIC VIRTUAL MtcExtensionSet::~MtcExtensionSet()
{
    Clear();
}

PUBLIC
MtcExtensionSet& MtcExtensionSet::operator=(IN const MtcExtensionSet& objRhs)
{
    if (this != &objRhs)
    {
        CopyFrom(objRhs);
    }

    return *this;
}

PUBLIC
IMS_BOOL MtcExtensionSet::IsAvailableOnBoth(IN const AString& strOptionTag) const
{
    IMS_SLONG nIndex = m_objExtensions.GetIndexOfKey(strOptionTag);
    return nIndex >= 0 && m_objExtensions.GetValueAt(nIndex)->IsAvailableOnRemote();
}

PUBLIC
IMS_BOOL MtcExtensionSet::IsAvailableOnLocal(IN const AString& strOptionTag) const
{
    return m_objExtensions.GetIndexOfKey(strOptionTag) >= 0;
}

PUBLIC
IMS_BOOL MtcExtensionSet::IsSupportRequiredExtensions(IN const IMessage& pMessage) const
{
    IMSList<AString> lstRequiredExtensions;
    MessageUtil::GetHeaders(&pMessage, ISipHeader::REQUIRE, lstRequiredExtensions);

    for (IMS_UINT32 nIndex = 0; nIndex < lstRequiredExtensions.GetSize(); nIndex++)
    {
        const AString& strOptionTag = lstRequiredExtensions.GetAt(nIndex);
        if (!IsAvailableOnLocal(strOptionTag))
        {
            IMS_TRACE_I(
                    "IsSupportRequiredExtensions : Not support %s", strOptionTag.GetStr(), 0, 0);
            return IMS_FALSE;
        }
    }
    return IMS_TRUE;
}

PUBLIC VIRTUAL void MtcExtensionSet::FormatRequest(
        IN IMS_UINT32 nMethod, IN_OUT IMessage& objRequest)
{
    for (IMS_UINT32 nIndex = 0; nIndex < m_objExtensions.GetSize(); nIndex++)
    {
        m_objExtensions.GetValueAt(nIndex)->FormatRequest(nMethod, objRequest);
    }
}

PUBLIC VIRTUAL void MtcExtensionSet::FormatResponse(
        IN IMS_UINT32 nMethod, IN_OUT IMessage& objResponse)
{
    for (IMS_UINT32 nIndex = 0; nIndex < m_objExtensions.GetSize(); nIndex++)
    {
        m_objExtensions.GetValueAt(nIndex)->FormatResponse(nMethod, objResponse);
    }
}

PUBLIC VIRTUAL void MtcExtensionSet::HandleRequest(
        IN IMS_UINT32 nMethod, IN const IMessage& objRequest)
{
    for (IMS_UINT32 nIndex = 0; nIndex < m_objExtensions.GetSize(); nIndex++)
    {
        m_objExtensions.GetValueAt(nIndex)->HandleRequest(nMethod, objRequest);
    }
}

PUBLIC VIRTUAL void MtcExtensionSet::HandleResponse(
        IN IMS_UINT32 nMethod, IN const IMessage& objResponse)
{
    for (IMS_UINT32 nIndex = 0; nIndex < m_objExtensions.GetSize(); nIndex++)
    {
        m_objExtensions.GetValueAt(nIndex)->HandleResponse(nMethod, objResponse);
    }
}

PRIVATE
void MtcExtensionSet::CopyFrom(IN const MtcExtensionSet& objRhs)
{
    Clear();

    for (IMS_UINT32 nIndex = 0; nIndex < objRhs.m_objExtensions.GetSize(); nIndex++)
    {
        m_objExtensions.Add(objRhs.m_objExtensions.GetKeyAt(nIndex),
                objRhs.m_objExtensions.GetValueAt(nIndex)->Clone());
    }
}

PRIVATE
void MtcExtensionSet::Clear()
{
    for (IMS_UINT32 nIndex = 0; nIndex < m_objExtensions.GetSize(); nIndex++)
    {
        delete m_objExtensions.GetValueAt(nIndex);
    }
    m_objExtensions.Clear();
}

PRIVATE
IMtcExtension* MtcExtensionSet::CreateExtension(IN const AString& strOptionTag) const
{
    if (strOptionTag.EqualsIgnoreCase(OPTION_TAG_PRECONDITION))
    {
        return new PreconditionExtension();
    }
    else if (strOptionTag.EqualsIgnoreCase(OPTION_TAG_RPR))
    {
        return new RprExtension();
    }

    return new MtcExtension(strOptionTag);
}
