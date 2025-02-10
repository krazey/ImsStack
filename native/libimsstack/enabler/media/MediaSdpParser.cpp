/**
 * Copyright (C) 2024 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ServiceTrace.h"

#include "MediaSdpParser.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC MediaSdpParser::MediaSdpParser(IN const MEDIA_CONTENT_TYPE eType)
{
    IMS_TRACE_I("+MediaSdpParser() media type[%d]", eType, 0, 0);
    m_eType = eType;
}

PUBLIC VIRTUAL MediaSdpParser::~MediaSdpParser()
{
    IMS_TRACE_I("~MediaSdpParser() media type[%d]", m_eType, 0, 0);
}

PROTECTED
void MediaSdpParser::Parse(IN ISessionDescriptor* pSessionDescriptor,
        IN IMediaDescriptor* pDescriptor, OUT MediaBaseProfile* pProfile)
{
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "Parse() - media type[%d], invalid argument", m_eType, 0, 0);
        return;
    }

    IMS_TRACE_I("Parse(), media type[%d]", m_eType, 0, 0);

    // IP
    pProfile->SetIpAddress(pDescriptor->GetRemoteAddress());

    // data port
    pProfile->SetDataPort(pDescriptor->GetRemotePort());

    // control port
    IMS_SINT32 nControlPortFromSdp = pDescriptor->GetAttributeInt(SdpAttribute::RTCP);
    IMS_BOOL bControlPortExistedinSdp = (nControlPortFromSdp != IMediaDescriptor::INVALID_VALUE);
    IMS_SINT32 nControlPort =
            (bControlPortExistedinSdp) ? nControlPortFromSdp : pProfile->GetDataPort() + 1;

    pProfile->SetControlPort(nControlPort);

    IMS_TRACE_I("Parse() - Ip[%s], Data Port[%d], Control Port[%d]",
            pProfile->GetIpAddress().ToCharString(), pProfile->GetDataPort(),
            pProfile->GetControlPort());

    // bandwidth as, rs, rr
    pProfile->SetBandwidthAs(pDescriptor->GetBandwidth(SdpBandwidth::TYPE_AS));
    pProfile->SetBandwidthRs(pDescriptor->GetBandwidth(SdpBandwidth::TYPE_RS));
    pProfile->SetBandwidthRr(pDescriptor->GetBandwidth(SdpBandwidth::TYPE_RR));

    IMS_TRACE_I("Parse() - AS[%d], RS[%d], RR[%d]", pProfile->GetBandwidthAs(),
            pProfile->GetBandwidthRs(), pProfile->GetBandwidthRr());

    // direction
    pProfile->SetDirection((MEDIA_DIRECTION)pDescriptor->GetDirection());

    if (pProfile->GetDirection() == MEDIA_DIRECTION_INVALID)
    {
        IMS_TRACE_D("Parse() -  media type[%d], invalid direction", m_eType, 0, 0);
        // check session level attribute Direction
        pProfile->SetDirection((MEDIA_DIRECTION)pSessionDescriptor->GetDirection());

        if (pProfile->GetDirection() == MEDIA_DIRECTION_INVALID)
        {
            pProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
        }
    }

    IMS_TRACE_I("Parse() - direction[%d]", pProfile->GetDirection(), 0, 0);
}

PROTECTED
IMS_BOOL MediaSdpParser::ParseCapaNego(
        IN IMediaDescriptor* pDescriptor, OUT MediaBaseProfile::CapaNego* pObjCapaNego)
{
    if (pDescriptor == IMS_NULL || pObjCapaNego == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_I("ParseCapaNego() - media[%d]", m_eType, 0, 0);

    if (ParseAcfg(pDescriptor, pObjCapaNego) == IMS_TRUE)
    {
        return IMS_TRUE;
    }

    IMS_BOOL bRet = IMS_FALSE;
    bRet = ParseTcap(pDescriptor, pObjCapaNego);
    bRet |= ParseAcap(pDescriptor, pObjCapaNego);
    bRet |= ParsePcfg(pDescriptor, pObjCapaNego);

    return bRet;
}

PROTECTED
IMS_BOOL MediaSdpParser::ParseAcfg(
        IN IMediaDescriptor* pDescriptor, OUT MediaBaseProfile::CapaNego* pObjCapaNego)
{
    ImsList<AString> lstAcfgAttr = pDescriptor->GetAttributes(SdpAttribute::ACFG);

    if (lstAcfgAttr.GetSize() > 0)
    {
        pObjCapaNego->SetAcfg(lstAcfgAttr.GetAt(0));
        IMS_TRACE_I("ParseAcfg() - Answer Case, media[%d], acfg[%s]", m_eType,
                pObjCapaNego->GetAcfg().GetStr(), 0);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL MediaSdpParser::ParseTcap(
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
                IMS_TRACE_I("ParseTcap() - media[%d], nTcapInitNum[%d]", m_eType, nTcapInitNum, 0);
            }
            else
            {
                AString strTcap = "";
                // mapped - key : 'number' value:'Tcap'
                strTcap.Sprintf("%s", lstSplitSpace.GetAt(j).GetStr());
                pObjCapaNego->GetMapTcap().Add(nTcapInitNum, strTcap);
                IMS_TRACE_I("ParseTcap() - media[%d], add map[%d - %s]", m_eType, nTcapInitNum,
                        strTcap.GetStr());
                nTcapInitNum++;
            }
        }
    }

    if (pObjCapaNego->GetMapTcap().GetSize() == 0)
    {
        IMS_TRACE_I("ParseTcap() - media[%d], no tcap value in SDP", m_eType, 0, 0);
        return IMS_FALSE;
    }
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MediaSdpParser::ParseAcap(
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
            IMS_TRACE_I("ParseAcap() - media[%d], add map[%d - %s]", m_eType, nAcapNum,
                    strAcap.GetStr());
            pObjCapaNego->GetMapAcap().Add(nAcapNum, strAcap);
        }
    }

    if (pObjCapaNego->GetMapAcap().GetSize() == 0)
    {
        IMS_TRACE_I("ParseAcap() - media[%d], no acap value in SDP", m_eType, 0, 0);
        return IMS_FALSE;
    }

    pObjCapaNego->SetAttCapaInPcfg(IMS_TRUE);

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MediaSdpParser::ParsePcfg(
        IN IMediaDescriptor* pDescriptor, OUT MediaBaseProfile::CapaNego* pObjCapaNego)
{
    // Get Potential configuration list (pcfg) -"'prio #' SP"t=Tcap #' SP 'a=Acap #'" pair
    pObjCapaNego->SetListPcfg(pDescriptor->GetAttributes(SdpAttribute::PCFG));

    if (pObjCapaNego->GetListPcfg().GetSize() == 0)
    {
        IMS_TRACE_I("ParsePcfg() - media[%d], no pcfg value in SDP", m_eType, 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_I("ParsePcfg() - media[%d]", m_eType, 0, 0);
    return IMS_TRUE;
}
