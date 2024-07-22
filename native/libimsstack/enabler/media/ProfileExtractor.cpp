// Copyright 2024 Google LLC

#include "ServiceTrace.h"

#include "ProfileExtractor.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC ProfileExtractor::ProfileExtractor(IN const MEDIA_CONTENT_TYPE eType)
{
    IMS_TRACE_I("+ProfileExtractor() media type[%d]", eType, 0, 0);
    m_eType = eType;
}

PUBLIC VIRTUAL ProfileExtractor::~ProfileExtractor()
{
    IMS_TRACE_I("~ProfileExtractor()", 0, 0, 0);
}

PROTECTED
IMS_BOOL ProfileExtractor::MakeCapaNegoProfileFromSdp(
        IN IMediaDescriptor* pDescriptor, OUT MediaBaseProfile::CapaNego* pObjCapaNego)
{
    if (pDescriptor == IMS_NULL || pObjCapaNego == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_I("MakeCapaNegoProfileFromSdp() - media[%d]", m_eType, 0, 0);

    if (MakeAcfgProfileFromSdp(pDescriptor, pObjCapaNego) == IMS_TRUE)
    {
        return IMS_TRUE;
    }

    IMS_BOOL bRet = IMS_FALSE;
    bRet = MakeTcapProfileFromSdp(pDescriptor, pObjCapaNego);
    bRet |= MakeAcapProfileFromSdp(pDescriptor, pObjCapaNego);
    bRet |= MakePcfgProfileFromSdp(pDescriptor, pObjCapaNego);

    return bRet;
}

PROTECTED
IMS_BOOL ProfileExtractor::MakeAcfgProfileFromSdp(
        IN IMediaDescriptor* pDescriptor, OUT MediaBaseProfile::CapaNego* pObjCapaNego)
{
    ImsList<AString> lstAcfgAttr = pDescriptor->GetAttributes(SdpAttribute::ACFG);

    if (lstAcfgAttr.GetSize() > 0)
    {
        pObjCapaNego->SetAcfg(lstAcfgAttr.GetAt(0));
        IMS_TRACE_I("MakeAcfgProfileFromSdp() - Answer Case, media[%d], acfg[%s]", m_eType,
                pObjCapaNego->GetAcfg().GetStr(), 0);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL ProfileExtractor::MakeTcapProfileFromSdp(
        IN IMediaDescriptor* pDescriptor, OUT MediaBaseProfile::CapaNego* pObjCapaNego)
{
    // Get transport capability(TCAP) list -"'number' SP 'Tcap'" pair
    ImsList<AString> lstTcapAttr = pDescriptor->GetAttributes(SdpAttribute::TCAP);
    for (IMS_UINT32 i = 0; i < lstTcapAttr.GetSize(); i++)
    {
        AString strTcapline = lstTcapAttr.GetAt(i);
        if (strTcapline.GetLength() == 0)
        {
            continue;
        }

        ImsList<AString> lstSplitSpace = strTcapline.Split(' ');
        IMS_SINT32 nTcapInitNum = 0;

        // save Tcap String to CapaNego Obj
        for (IMS_UINT32 j = 0; j < lstSplitSpace.GetSize(); j++)
        {
            if (j == 0)
            {
                nTcapInitNum = lstSplitSpace.GetAt(j).ToInt32();
                IMS_TRACE_I("MakeTcapProfileFromSdp() - media[%d], nTcapInitNum[%d]", m_eType,
                        nTcapInitNum, 0);
            }
            else
            {
                AString strTcap = "";
                // mapped - key : 'number' value:'Tcap'
                strTcap.Sprintf("%s", lstSplitSpace.GetAt(j).GetStr());
                pObjCapaNego->GetMapTcap().Add(nTcapInitNum, strTcap);
                IMS_TRACE_I("MakeTcapProfileFromSdp() - media[%d], add map[%d - %s]", m_eType,
                        nTcapInitNum, strTcap.GetStr());
                nTcapInitNum++;
            }
        }
    }

    if (pObjCapaNego->GetMapTcap().GetSize() == 0)
    {
        IMS_TRACE_I("MakeTcapProfileFromSdp() - media[%d], no tcap value in SDP", m_eType, 0, 0);
        return IMS_FALSE;
    }
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL ProfileExtractor::MakeAcapProfileFromSdp(
        IN IMediaDescriptor* pDescriptor, OUT MediaBaseProfile::CapaNego* pObjCapaNego)
{
    // Get attribute capability(ACAP) list -"'number' SP 'Acap'" pair
    ImsList<AString> lstAcapAttr = pDescriptor->GetAttributes(SdpAttribute::ACAP);
    for (IMS_UINT32 i = 0; i < lstAcapAttr.GetSize(); i++)
    {
        IMS_SINT32 nAcapNum = 0;
        AString strAcap = "";
        AString strAcapline = lstAcapAttr.GetAt(i);
        if (strAcapline.GetLength() == 0)
        {
            continue;
        }

        ImsList<AString> lstSplitSpace = strAcapline.Split(' ');

        // save Acap String to CapaNego Obj
        for (IMS_UINT32 j = 0; j < lstSplitSpace.GetSize(); j++)
        {
            if (j == 0)
            {
                nAcapNum = lstSplitSpace.GetAt(j).ToInt32();
            }
            else
            {
                if (strAcap.GetLength() == 0)
                {
                    strAcap.Append(lstSplitSpace.GetAt(j));
                }
                else
                {
                    strAcap.Append("" + lstSplitSpace.GetAt(j));
                }
            }
        }
        // save Acap String...
        if (strAcap.GetLength() != 0)
        {
            IMS_TRACE_I("MakeAcapProfileFromSdp() - media[%d], add map[%d - %s]", m_eType, nAcapNum,
                    strAcap.GetStr());
            pObjCapaNego->GetMapAcap().Add(nAcapNum, strAcap);
        }
    }

    if (pObjCapaNego->GetMapAcap().GetSize() == 0)
    {
        IMS_TRACE_I("MakeAcapProfileFromSdp() - media[%d], no acap value in SDP", m_eType, 0, 0);
        return IMS_FALSE;
    }

    pObjCapaNego->SetAttCapaInPcfg(IMS_TRUE);

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL ProfileExtractor::MakePcfgProfileFromSdp(
        IN IMediaDescriptor* pDescriptor, OUT MediaBaseProfile::CapaNego* pObjCapaNego)
{
    // Get Potential configuration list (pcfg) -"'prio #' SP"t=Tcap #' SP 'a=Acap #'" pair
    pObjCapaNego->SetListPcfg(pDescriptor->GetAttributes(SdpAttribute::PCFG));

    if (pObjCapaNego->GetListPcfg().GetSize() == 0)
    {
        IMS_TRACE_I("MakePcfgProfileFromSdp() - media[%d], no pcfg value in SDP", m_eType, 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_I("MakePcfgProfileFromSdp() - media[%d]", m_eType, 0, 0);
    return IMS_TRUE;
}
