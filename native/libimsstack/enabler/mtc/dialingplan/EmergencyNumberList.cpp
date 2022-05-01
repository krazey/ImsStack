#include "ServicePhoneInfo.h"
#include "ServiceTrace.h"
#include "AStringBuffer.h"
#include "TextParser.h"
#include "dialingplan/EmergencyNumberList.h"

#include "dialingplan/ConstSosUrn.h"
#include "IMSMap.h"

__IMS_TRACE_TAG_IMS__;

PUBLIC
EmergencyNumberList::EmergencyNumberList(IN IMS_SINT32 nSlotID)
    : m_nSlotID(nSlotID)
    , m_nURNMappingType(URN_MAPPING_TYPE_3GPP_STANDARD)
    , m_objEMCConfigStrMap(IMSMap<AString, AString>())
{
    if (ReadOperatorConfig() == IMS_FAILURE)
    {
        IMS_TRACE_E(0, "Fail to access DB", 0, 0, 0);
    }
}

PUBLIC VIRTUAL
EmergencyNumberList::~EmergencyNumberList()
{
    m_objEMCConfigStrMap.Clear();
}

/*
Remarks

Description:
Follow the step based on 3GPP 24.229
*/
PUBLIC
AString EmergencyNumberList::GetEmergencyServiceURN(IN const AString &strNumber)
{
    IMS_SINT32 nSource = ENL_NETWORK;
    IMS_SINT32 nESCV = GetEmergencyServiceCategory(strNumber, nSource);

    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("GetEmergencyServiceURN : [%s], nSource[%s] ESCV[%d]",
            strNumber.GetStr(), ConvertSourceToString(nSource), nESCV);

    if (nESCV > ESCV_INVALID && m_nURNMappingType != URN_MAPPING_TYPE_GENERIC_ONLY)
    {
        if (nSource == ENL_NETWORK)
        {
            return HandleENLForNetwork(strNumber, nESCV);
        }
        else
        {
            return HandleENLForUICC(strNumber, nESCV);
        }
    }
    return HandleNoENL(strNumber);
}

/*
Remarks

Description:
Will be deleted. Old solution.
*/
PUBLIC
IMS_SINT32 EmergencyNumberList::GetEmergencyServiceCategory(IN const AString &strNumber)
{
    IMS_SINT32 nESCV = ESCV_INVALID;
    IMS_SINT32 nPrimaryENL = GetPrimaryENLSource();
    IMS_SINT32 nSecondaryENL = (nPrimaryENL == ENL_NETWORK) ? ENL_UICC : ENL_NETWORK;

    //---------------------------------------------------------------------------------------------

    IMS_SINT32 nSource = nPrimaryENL;

    while (1)
    {
        AStringBuffer objENL;

        if (GetENL(objENL, nSource))
        {
            nESCV = GetESCV(strNumber, objENL.GetString());
        }

        if (nESCV != ESCV_INVALID)
        {
            return nESCV;
        }

        if (nSource == nSecondaryENL)
        {
            break;
        }

        nSource = nSecondaryENL;
    }

    return nESCV;
}

/*
Remarks

Description:
Return ESCV with the priority
*/
PRIVATE
IMS_SINT32 EmergencyNumberList::GetEmergencyServiceCategory(IN const AString &strNumber,
        OUT IMS_SINT32 &nSource)
{
    IMS_SINT32 nESCV = ESCV_INVALID;
    IMS_SINT32 nPrimaryENL = GetPrimaryENLSource();
    IMS_SINT32 nSecondaryENL = (nPrimaryENL == ENL_NETWORK) ? ENL_UICC : ENL_NETWORK;

    //---------------------------------------------------------------------------------------------

    nSource = nPrimaryENL;

    while (1)
    {
        AStringBuffer objENL;

        if (GetENL(objENL, nSource))
        {
            nESCV = GetESCV(strNumber, objENL.GetString());
        }

        if (nESCV != ESCV_INVALID)
        {
            return nESCV;
        }

        if (nSource == nSecondaryENL)
        {
            break;
        }

        nSource = nSecondaryENL;
    }

    return nESCV;
}

/*
Remarks
*/
PRIVATE
IMS_SINT32 EmergencyNumberList::GetEmergencyServiceCategoryEx(IN const AString &strNumber,
        IN IMS_SINT32 nSource /* = ENL_NETWORK */)
{
    IMS_SINT32 nESCV = ESCV_INVALID;

    //---------------------------------------------------------------------------------------------

    AStringBuffer objENL;

    if (GetENL(objENL, nSource))
    {
        nESCV = GetESCV(strNumber, objENL.GetString());
    }

    return nESCV;
}

/*
Remarks

Description:
Return the preferred source in the modem
*/
PRIVATE
IMS_SINT32 EmergencyNumberList::GetPrimaryENLSource() const
{
    //---------------------------------------------------------------------------------------------
    // 0 : UICC, 1 : Network
    return PhoneInfoService::GetPhoneInfoService()->GetSubscriberInfo(m_nSlotID)
            ->GetEmergencyPriorityFromModem();
}

/*
Remarks

Description:
*/
PRIVATE
AString EmergencyNumberList::HandleENLForNetwork(IN const AString &strNumber,
        IN IMS_SINT32 nESCV)
{
    if (PhoneInfoService::GetPhoneInfoService()->GetNetworkWatcher(m_nSlotID)->GetRoamingState()
            == 0)
    {
        //Read configuration
        AString strURNForOp = GetOperatorConfigURN(strNumber);
        if (strURNForOp.GetLength() > 0)
        {
            return strURNForOp;
        }
    }

    //standard policy
    return TranslateAsEmergencyServiceURN(nESCV);
}

/*
Remarks

Description:
*/
PRIVATE
AString EmergencyNumberList::HandleENLForUICC(IN const AString &strNumber,
        IN IMS_SINT32 nESCV)
{
    if (PhoneInfoService::GetPhoneInfoService()->GetNetworkWatcher(m_nSlotID)->GetRoamingState()
            == 1)
    {
        return ConstSosUrn::GENERIC;
    }
    else
    {
        //Read configuration
        AString strURNForOp = GetOperatorConfigURN(strNumber);
        if (strURNForOp.GetLength() > 0)
        {
             return strURNForOp;
        }
    }

    //standard policy
    return TranslateAsEmergencyServiceURN(nESCV);
}

/*
Remarks

Description:
*/
PRIVATE
AString EmergencyNumberList::HandleNoENL(IN const AString &strNumber)
{
    if (PhoneInfoService::GetPhoneInfoService()->GetNetworkWatcher(m_nSlotID)->GetRoamingState()
            == 0)
    {
        //Read configuration
        AString strURNForOp = GetOperatorConfigURN(strNumber);
        if (strURNForOp.GetLength() > 0)
        {
             return strURNForOp;
        }
    }

    return ConstSosUrn::GENERIC;
}

/*
Remarks

Description:
Return Emergency number list if exist
*/
PRIVATE
IMS_BOOL EmergencyNumberList::GetENL(OUT AStringBuffer &objENL,
        IN IMS_SINT32 nSource /* = ENL_NETWORK */)
{
    AString strENL;

    //---------------------------------------------------------------------------------------------

    if (nSource == ENL_NETWORK)
    {
        IMS_TRACE_E(0, "GetENL: ENL_NETWORK - NOT SUPPORT", 0, 0, 0);
    }
    else if (nSource == ENL_UICC)
    {
        PhoneInfoService::GetPhoneInfoService()->GetSubscriberInfo(m_nSlotID)
                ->GetEmergencyNumberListFromSim(strENL);
    }

    IMS_TRACE_D("GetENL : Source[%d] Value[%s]", nSource, strENL.GetStr(), 0);

    if (strENL.GetLength() > 0)
    {
        objENL.Append(strENL);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

/*
Remarks

Description:
Return Emergency service category value in the ENL
*/
PRIVATE
IMS_SINT32 EmergencyNumberList::GetESCV(IN const AString &strNumber, IN const AString &strENL)
{
    IMS_CHAR cDelimiter = '\0';

    //---------------------------------------------------------------------------------------------

    if (strENL.Contains(TextParser::CHAR_COMMA))
    {
        cDelimiter = TextParser::CHAR_COMMA;
    }
    else if (strENL.Contains(TextParser::CHAR_SLASH))
    {
        cDelimiter = TextParser::CHAR_SLASH;
    }

    IMSList<AString> objTokens = strENL.Split(cDelimiter);


    for (IMS_UINT32 i = 0; i < objTokens.GetSize(); i++)
    {
        const AString &strToken = objTokens.GetAt(i);

        AString strValue1 = strToken.GetSubStr(0, 2);
        AString strValue2 = strToken.GetSubStr(2);

        if (strValue2.Equals(strNumber))
        {
            IMS_BOOL bOK = IMS_FALSE;
            IMS_UINT32 nESCV = strValue1.ToUInt32(&bOK, 16);
            IMS_TRACE_D("GetESCV : [%s]-->[%d], result[%d]", strValue1.GetStr(), nESCV, bOK);
            return nESCV;
        }
    }


    return ESCV_INVALID;
}

/*
Remarks

Description:
Return the matched URN regarding the number
if stored number with URN exists in the configuration.xx.xx.xml
*/
PRIVATE
AString EmergencyNumberList::GetOperatorConfigURN(IN const AString &strNumber)
{
    for(IMS_UINT32 index = 0; index < m_objEMCConfigStrMap.GetSize(); index ++)
    {
        if(strNumber.Equals(m_objEMCConfigStrMap.GetKeyAt(index)))
        {
            IMS_TRACE_I("GetOperatorConfigURN : find the matched URN", 0, 0, 0);
            return m_objEMCConfigStrMap.GetValueAt(index);
        }
    }

    IMS_TRACE_I("GetOperatorConfigURN : no matched URN", 0, 0, 0);
    return AString::ConstNull();
}

/*
Remarks

Description:
Read DB and return the result with IMSMap
*/
PRIVATE
IMS_RESULT EmergencyNumberList::ReadOperatorConfig()
{
    IMS_TRACE_D("ReadOperatorConfig: no-op", 0, 0, 0);
    return IMS_SUCCESS;
}

/*
Remarks

Description:
Return the matched Emergency Service URN regarding ESCV

ex) if ESCV is 9 (1001), it will return 0001(POLICE)
*/
PRIVATE
AString EmergencyNumberList::TranslateAsEmergencyServiceURN(IN IMS_SINT32 nESCV)
{
    AString strURN;

    //---------------------------------------------------------------------------------------------

    //Check if ESCV is multiple value
    if ((m_nURNMappingType == URN_MAPPING_TYPE_GENERIC_MULTIPLE_ESCV) && IsMultipleESCV(nESCV))
    {
        return ConstSosUrn::GENERIC;
    }

    if (nESCV & EmergencyNumberList::ESCV_POLICE)
    {
        strURN = ConstSosUrn::POLICE;
    }
    else if (nESCV & EmergencyNumberList::ESCV_AMBULANCE)
    {
        strURN = ConstSosUrn::AMBULANCE;
    }
    else if (nESCV & EmergencyNumberList::ESCV_FIREBRIGADE)
    {
        strURN = ConstSosUrn::FIRE;
    }
    else if (nESCV & EmergencyNumberList::ESCV_MARINEGUARD)
    {
        strURN = ConstSosUrn::MARINE;
    }
    else if (nESCV & EmergencyNumberList::ESCV_MOUNTAINRESCUE)
    {
        strURN = ConstSosUrn::MOUNTAIN;
    }
    else//invalid case or not matched case
    {
        strURN = ConstSosUrn::GENERIC;
    }

    IMS_TRACE_D("TranslateAsEmergencyServiceURN : URN[%s]", strURN.GetStr(), 0, 0);

    return strURN;
}

/*
Remarks

Description:
Print Source which includes ENL for debugging
*/
PRIVATE
const IMS_CHAR* EmergencyNumberList::ConvertSourceToString(IN IMS_SINT32 nSource)
{
    switch(nSource)
    {
        case ENL_NETWORK:
            return "ENL_NETWORK";
        case ENL_UICC:
            return "ENL_UICC";
        default:
            return "INVALID";
    }
}

PRIVATE
IMS_BOOL EmergencyNumberList::IsMultipleESCV(IN IMS_SINT32 nESCV)
{
    IMS_UINT32 nCount = 0;

    //---------------------------------------------------------------------------------------------

    while (nESCV > 0)
    {
        if ((nESCV & 1) == 1)
        {
            nCount++;
        }

        nESCV >>= 1;
    }

    IMS_TRACE_D("IsMultipleESCV : numberof ESCV [%d]", nCount, 0, 0);
    return (nCount > 1) ? IMS_TRUE : IMS_FALSE;
}
