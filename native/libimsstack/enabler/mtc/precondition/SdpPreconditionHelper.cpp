/*
 * Copyright (C) 2022 The Android Open Source Project
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

#include "IMessage.h"
#include "ISessionParameter.h"
#include "ISipMessage.h"
#include "ISipMessageBodyPart.h"
#include "ImsList.h"
#include "MtcDef.h"
#include "SdpParser.h"
#include "ServiceTrace.h"
#include "SipStatusCode.h"
#include "media/IMedia.h"
#include "media/IMediaDescriptor.h"
#include "offeranswer/SdpPrecondition.h"
#include "offeranswer/SdpSegmentedPrecondition.h"
#include "precondition/QosStringUtils.h"
#include "precondition/SdpPreconditionHelper.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC SdpPreconditionHelper::SdpPreconditionHelper() {}
PUBLIC VIRTUAL SdpPreconditionHelper::~SdpPreconditionHelper() {}

PUBLIC VIRTUAL void SdpPreconditionHelper::FormPreconditionSdp(
        IN ISession* piSession, IN QosStatusTable* pStatusTable, IN IMS_BOOL bUseConf)
{
    if (piSession == IMS_NULL || pStatusTable == IMS_NULL)
    {
        IMS_TRACE_D("FormPreconditionSdp : forming fail", 0, 0, 0);
        return;
    }

    ImsList<IMedia*> lstMedias = piSession->GetMedia();
    for (IMS_UINT32 index = 0; index < lstMedias.GetSize(); index++)
    {
        IMedia* piMedia = lstMedias.GetAt(index);
        if (piMedia == IMS_NULL || piMedia->GetState() == IMedia::STATE_DELETED)
        {
            continue;
        }

        IMediaDescriptor* piMediaDescriptor = GetMediaDescriptor(piMedia);
        if (piMediaDescriptor == IMS_NULL)
        {
            continue;
        }

        piMediaDescriptor->RemovePrecondition(SdpAttribute::CURR);
        piMediaDescriptor->RemovePrecondition(SdpAttribute::DES);
        piMediaDescriptor->RemovePrecondition(SdpAttribute::CONF);

        const SdpMedia* pLocalSdp = piMediaDescriptor->GetMediaDescriptionExAsLocal();
        if (pLocalSdp == IMS_NULL)
        {
            continue;
        }

        IMS_SINT32 eSdpMediaType = pLocalSdp->GetType();
        IMS_TRACE_D("FormPreconditionSdp : [%s], start forming",
                QosStringUtils::ConvertSdpMediaType(eSdpMediaType), 0, 0);

        FormCurrentAttribute(piMediaDescriptor, pStatusTable);
        FormDesiredAttribute(piMediaDescriptor, pStatusTable);
        FormConfirmAttribute(piMediaDescriptor, pStatusTable, bUseConf);
    }
}

PUBLIC VIRTUAL void SdpPreconditionHelper::RemovePreconditionSdp(IN ISession* piSession)
{
    if (piSession == IMS_NULL)
    {
        IMS_TRACE_D("RemovePreconditionSdp : fail", 0, 0, 0);
        return;
    }

    ImsList<IMedia*> lstMedias = piSession->GetMedia();
    IMS_UINT32 nSize = lstMedias.GetSize();

    for (IMS_UINT32 index = 0; index < nSize; index++)
    {
        IMedia* piMedia = lstMedias.GetAt(index);
        if (piMedia == IMS_NULL || piMedia->GetState() == IMedia::STATE_DELETED)
        {
            continue;
        }

        IMediaDescriptor* piMediaDescriptor = GetMediaDescriptor(piMedia);
        if (piMediaDescriptor == IMS_NULL)
        {
            continue;
        }

        IMS_TRACE_D("RemovePreconditionSdp", 0, 0, 0);
        piMediaDescriptor->RemovePrecondition(SdpAttribute::CURR);
        piMediaDescriptor->RemovePrecondition(SdpAttribute::DES);
        piMediaDescriptor->RemovePrecondition(SdpAttribute::CONF);
    }
}

PUBLIC VIRTUAL void SdpPreconditionHelper::FormFailurePreconditionSdp(IN ISession* piSession)
{
    if (piSession == IMS_NULL)
    {
        IMS_TRACE_D("FormFailurePreconditionSdp : forming fail", 0, 0, 0);
        return;
    }

    IMS_TRACE_D("FormFailurePreconditionSdp", 0, 0, 0);

    piSession->CreateFailureSdp();
    ISessionParameter* piSessionParam = piSession->GetFailureSdp();

    if (piSessionParam == IMS_NULL)
    {
        IMS_TRACE_D("FormFailurePreconditionSdp : forming fail", 0, 0, 0);
        return;
    }

    for (IMS_SINT32 index = 0; index < piSessionParam->GetMediaCount(); index++)
    {
        SdpMediaParameter* pSdpMediaParam = piSessionParam->GetMediaParameter(index);

        if (pSdpMediaParam == IMS_NULL)
        {
            continue;
        }

        pSdpMediaParam->MarkRejectedOrRemoved();

        pSdpMediaParam->RemovePrecondition(SdpAttribute::CURR);
        pSdpMediaParam->RemovePrecondition(SdpAttribute::DES);
        pSdpMediaParam->RemovePrecondition(SdpAttribute::CONF);

        SdpSegmentedPrecondition objPrecondition;
        objPrecondition.AddStatus(SdpPrecondition::STATUS_LOCAL,
                SdpPrecondition::DIRECTION_SENDRECV, SdpPrecondition::STRENGTH_FAILURE);

        pSdpMediaParam->SetPrecondition(SdpAttribute::DES, &objPrecondition);
    }
}

PUBLIC VIRTUAL IMS_UINT32 SdpPreconditionHelper::GetMediaType(
        IN const SdpMedia* pSdpMedia, IN IMS_SINT32 nMediaState)
{
    IMS_UINT32 eMediaType = MEDIATYPE_NONE;

    if (pSdpMedia == IMS_NULL || pSdpMedia->GetPort() == 0)
    {
        return eMediaType;
    }

    if (pSdpMedia->GetType() == SdpMedia::TYPE_AUDIO)
    {
        eMediaType = MEDIATYPE_AUDIO;
    }
    else if (nMediaState != IMedia::STATE_DELETED)
    {
        if (pSdpMedia->GetType() == SdpMedia::TYPE_VIDEO)
        {
            eMediaType = MEDIATYPE_VIDEO;
        }
        else if (pSdpMedia->GetType() == SdpMedia::TYPE_TEXT)
        {
            eMediaType = MEDIATYPE_TEXT;
        }
    }

    return eMediaType;
}

PUBLIC VIRTUAL IMS_BOOL SdpPreconditionHelper::IsPreconditionIncludedInSdp(IN ISession* piSession)
{
    IMS_BOOL bResult = IMS_FALSE;

    if (piSession == IMS_NULL)
    {
        return bResult;
    }

    ImsList<IMedia*> lstMedias = piSession->GetMedia();

    for (IMS_UINT32 index = 0; index < lstMedias.GetSize(); index++)
    {
        IMedia* piMedia = lstMedias.GetAt(index);
        if (!piMedia)
        {
            continue;
        }

        IMediaDescriptor* piMediaDescriptor = GetMediaDescriptor(piMedia);
        if (!piMediaDescriptor)
        {
            continue;
        }

        const SdpSegmentedPrecondition* pCurr = DYNAMIC_CAST(
                SdpSegmentedPrecondition*, piMediaDescriptor->GetPrecondition(SdpAttribute::CURR));
        if (pCurr)
        {
            bResult = IMS_TRUE;
            break;
        }
    }

    IMS_TRACE_D("IsPreconditionIncludedInSdp : %s", _TRACE_B_(bResult), 0, 0);
    return bResult;
}

PUBLIC VIRTUAL IMS_BOOL SdpPreconditionHelper::IsLocalResourceReservedInSdp(
        IN ISession* piSession, IN IMS_SINT32 nServiceMethod)
{
    if (piSession == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMessage* piRequestMessage = piSession->GetPreviousRequest(nServiceMethod);
    if (piRequestMessage == IMS_NULL)
    {
        IMS_TRACE_D("IsLocalResourceReservedInSdp : no request", 0, 0, 0);
        return IMS_FALSE;
    }

    ISipMessage* piSipMessage = IMS_NULL;

    if (piRequestMessage->GetState() == IMessage::STATE_SENT)
    {
        IMS_TRACE_D("IsLocalResourceReservedInSdp : Check QoS attributes from Request[%d]",
                nServiceMethod, 0, 0);

        piSipMessage = piRequestMessage->GetMessage();
    }
    else
    {
        IMS_TRACE_D("IsLocalResourceReservedInSdp : Check QoS attributes from Response[%d]",
                nServiceMethod, 0, 0);

        ImsList<IMessage*> lstResponseMessages = piSession->GetPreviousResponses(nServiceMethod);
        if (lstResponseMessages.GetSize() <= 0)
        {
            IMS_TRACE_D("IsLocalResourceReservedInSdp : no responses", 0, 0, 0);
            return IMS_FALSE;
        }

        IMessage* piResponseMessage = IMS_NULL;
        for (IMS_UINT32 index = 0; index < lstResponseMessages.GetSize(); index++)
        {
            IMessage* piTempResponse = lstResponseMessages.GetAt(index);
            if (piTempResponse->GetState() == IMessage::STATE_SENT)
            {
                piResponseMessage = piTempResponse;
            }
        }

        if (piResponseMessage == IMS_NULL)
        {
            IMS_TRACE_D("IsLocalResourceReservedInSdp : No Response to request from DUT.", 0, 0, 0);
            return IMS_FALSE;
        }

        piSipMessage = piResponseMessage->GetMessage();
    }

    if (piSipMessage == IMS_NULL)
    {
        IMS_TRACE_D("IsLocalResourceReservedInSdp : piSipMessage is null", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_BOOL bResult = (HasReservedResourceInSdp(piSipMessage, SdpMedia::TYPE_AUDIO) &&
            HasReservedResourceInSdp(piSipMessage, SdpMedia::TYPE_VIDEO) &&
            HasReservedResourceInSdp(piSipMessage, SdpMedia::TYPE_TEXT));

    IMS_TRACE_D("IsLocalResourceReservedInSdp : Result - %s", _TRACE_B_(bResult), 0, 0);
    return bResult;
}

PRIVATE
void SdpPreconditionHelper::FormCurrentAttribute(
        IN IMediaDescriptor* piMediaDescriptor, IN QosStatusTable* pStatusTable)
{
    if (piMediaDescriptor == IMS_NULL || pStatusTable == IMS_NULL)
    {
        return;
    }

    const SdpMedia* pLocalSdp = piMediaDescriptor->GetMediaDescriptionExAsLocal();
    if (pLocalSdp == IMS_NULL)
    {
        return;
    }

    IMS_SINT32 eSdpMediaType = pLocalSdp->GetType();

    SdpSegmentedPrecondition objCurrent;
    IMS_SINT32 eDirTag = pStatusTable->GetDirectionTag(
            eSdpMediaType, SdpAttribute::CURR, SdpPrecondition::STATUS_LOCAL);

    if (eDirTag == SdpPrecondition::DIRECTION_INVALID)
    {
        IMS_TRACE_D("FormCurrentAttribute : there is no qos attribute to form SDP.", 0, 0, 0);
        return;
    }

    objCurrent.AddStatus(SdpPrecondition::STATUS_LOCAL, eDirTag);

    pStatusTable->SetLocalResourceConfirmed(eSdpMediaType,
            pStatusTable->IsCurrentStatusEnabled(eSdpMediaType, SdpPrecondition::STATUS_LOCAL));

    eDirTag = pStatusTable->GetDirectionTag(
            eSdpMediaType, SdpAttribute::CURR, SdpPrecondition::STATUS_REMOTE);
    objCurrent.AddStatus(SdpPrecondition::STATUS_REMOTE, eDirTag);

    piMediaDescriptor->SetPrecondition(SdpAttribute::CURR, &objCurrent);
}

PRIVATE
void SdpPreconditionHelper::FormDesiredAttribute(
        IN IMediaDescriptor* piMediaDescriptor, IN QosStatusTable* pStatusTable)
{
    if (piMediaDescriptor == IMS_NULL || pStatusTable == IMS_NULL)
    {
        return;
    }

    const SdpMedia* pLocalSdp = piMediaDescriptor->GetMediaDescriptionExAsLocal();
    if (pLocalSdp == IMS_NULL)
    {
        return;
    }

    IMS_SINT32 eSdpMediaType = pLocalSdp->GetType();

    SdpSegmentedPrecondition objDesired;

    AddDesiredStatus(&objDesired, pStatusTable, eSdpMediaType, SdpPrecondition::STATUS_LOCAL);
    AddDesiredStatus(&objDesired, pStatusTable, eSdpMediaType, SdpPrecondition::STATUS_REMOTE);

    if (objDesired.IsPreconditionPresent())
    {
        piMediaDescriptor->SetPrecondition(SdpAttribute::DES, &objDesired);
    }
}

PRIVATE
void SdpPreconditionHelper::AddDesiredStatus(OUT SdpSegmentedPrecondition* objDesired,
        IN QosStatusTable* pStatusTable, IN IMS_SINT32 eSdpMediaType, IN IMS_SINT32 eStatusType)
{
    if (pStatusTable == IMS_NULL || objDesired == IMS_NULL)
    {
        return;
    }

    IMS_SINT32 eSendStrength = pStatusTable->GetStrengthTag(
            eSdpMediaType, eStatusType, SdpPrecondition::DIRECTION_SEND);
    IMS_SINT32 eRecvStrength = pStatusTable->GetStrengthTag(
            eSdpMediaType, eStatusType, SdpPrecondition::DIRECTION_RECV);

    if (eSendStrength != SdpPrecondition::STRENGTH_NOTUSED && eSendStrength == eRecvStrength)
    {
        objDesired->AddStatus(eStatusType, SdpPrecondition::DIRECTION_SENDRECV, eSendStrength);
    }
    else
    {
        if (eSendStrength != SdpPrecondition::STRENGTH_NOTUSED)
        {
            objDesired->AddStatus(eStatusType, SdpPrecondition::DIRECTION_SEND, eSendStrength);
        }

        if (eRecvStrength != SdpPrecondition::STRENGTH_NOTUSED)
        {
            objDesired->AddStatus(eStatusType, SdpPrecondition::DIRECTION_RECV, eRecvStrength);
        }
    }
}

PRIVATE
void SdpPreconditionHelper::FormConfirmAttribute(IN IMediaDescriptor* piMediaDescriptor,
        IN QosStatusTable* pStatusTable, IN IMS_BOOL bUseConf)
{
    if (!bUseConf || piMediaDescriptor == IMS_NULL || pStatusTable == IMS_NULL)
    {
        return;
    }

    const SdpMedia* pLocalSdp = piMediaDescriptor->GetMediaDescriptionExAsLocal();
    if (pLocalSdp == IMS_NULL)
    {
        return;
    }

    IMS_SINT32 eSdpMediaType = pLocalSdp->GetType();

    if (pStatusTable->IsCurrentStatusEnabled(eSdpMediaType, SdpPrecondition::STATUS_REMOTE))
    {
        IMS_TRACE_D("FormConfirmAttribute : %s, don't form confirm attribute",
                QosStringUtils::ConvertSdpMediaType(eSdpMediaType), 0, 0);
        return;
    }

    IMS_TRACE_D(
            "FormConfirmAttribute : %s", QosStringUtils::ConvertSdpMediaType(eSdpMediaType), 0, 0);

    IMS_SINT32 eDirTag = pStatusTable->GetDirectionTag(
            eSdpMediaType, SdpAttribute::CONF, SdpPrecondition::STATUS_REMOTE);

    if (eDirTag == SdpPrecondition::DIRECTION_INVALID)
    {
        IMS_TRACE_D("FormConfirmAttribute : there is no qos attribute to form SDP.", 0, 0, 0);
        return;
    }

    SdpSegmentedPrecondition objConf;
    objConf.AddStatus(SdpPrecondition::STATUS_REMOTE, eDirTag);

    piMediaDescriptor->SetPrecondition(SdpAttribute::CONF, &objConf);
}

PRIVATE
IMediaDescriptor* SdpPreconditionHelper::GetMediaDescriptor(IN IMedia* piMedia)
{
    if (piMedia == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (piMedia->GetUpdateState() == IMedia::UPDATE_MODIFIED && piMedia->GetProposal() != IMS_NULL)
    {
        return piMedia->GetProposal()->GetMediaDescriptor();
    }
    else
    {
        return piMedia->GetMediaDescriptor();
    }
}

PRIVATE
IMS_BOOL SdpPreconditionHelper::HasReservedResourceInSdp(
        IN ISipMessage* piSipMessage, IN IMS_SINT32 eSdpMediaType)
{
    if (piSipMessage == IMS_NULL)
    {
        IMS_TRACE_D("HasReservedResourceInSdp : SIPMessage is null.", 0, 0, 0);
        return IMS_FALSE;
    }

    ISipMessageBodyPart* piBodyPart = piSipMessage->GetSdpBodyPart();
    if (piBodyPart == IMS_NULL)
    {
        IMS_TRACE_D("HasReservedResourceInSdp : ISipMessageBodyPart is null.", 0, 0, 0);
        return IMS_FALSE;
    }

    const ByteArray& objSdp = piBodyPart->GetContent();
    const IMS_CHAR* pSdpBody = reinterpret_cast<const IMS_CHAR*>(objSdp.GetData());

    AString strSdp(pSdpBody, objSdp.GetLength());
    SdpParser objParser;

    if (!objParser.Decode(strSdp))
    {
        IMS_TRACE_D("HasReservedResourceInSdp : Parsing SDP is failed.", 0, 0, 0);
        return IMS_FALSE;
    }

    const ImsList<SdpMediaDescription>& lstMediaDescriptions = objParser.GetMediaDescriptions();

    for (IMS_UINT32 i = 0; i < lstMediaDescriptions.GetSize(); i++)
    {
        const SdpMediaDescription& objMediaDescription = lstMediaDescriptions.GetAt(i);
        const SdpMedia& objSdpMedia = objMediaDescription.GetMedia();
        IMS_SINT32 nSdpMediaType = objSdpMedia.GetType();

        if (nSdpMediaType != eSdpMediaType)
        {
            continue;
        }

        IMS_TRACE_D("HasReservedResourceInSdp : MediaType from parsed SDP [%s]",
                QosStringUtils::ConvertSdpMediaType(nSdpMediaType), 0, 0);

        if (objSdpMedia.GetPort() == 0)
        {
            IMS_TRACE_D("HasReservedResourceInSdp : Port is 0.", 0, 0, 0);
            return IMS_FALSE;
        }

        ImsList<SdpAttribute> objAttributes = objMediaDescription.GetAttributes(SdpAttribute::CURR);
        if (objAttributes.GetSize() <= 0)
        {
            IMS_TRACE_D("HasReservedResourceInSdp : There's no Current attributes.", 0, 0, 0);
            return IMS_FALSE;
        }

        for (IMS_UINT32 j = 0; j < objAttributes.GetSize(); j++)
        {
            SdpAttribute objAttr = objAttributes.GetAt(j);
            const AString& strValue = objAttr.GetAttributeValue();

            if (strValue.Contains("local"))
            {
                return (strValue.Contains("none")) ? IMS_FALSE : IMS_TRUE;
            }
        }
    }

    IMS_TRACE_D("HasReservedResourceInSdp : There's no %s.",
            QosStringUtils::ConvertSdpMediaType(eSdpMediaType), 0, 0);
    return IMS_TRUE;
}
