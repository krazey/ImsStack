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

#include "MtcDef.h"
#include "ServiceTrace.h"
#include "media/IMedia.h"
#include "offeranswer/SdpSegmentedPrecondition.h"
#include "precondition/QosStatusTable.h"
#include "precondition/QosStringDef.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
QosStatusTable::QosStatusTable() :
        m_lstAudioRecords(ImsList<QosStatusRecord*>()),
        m_lstVideoRecords(ImsList<QosStatusRecord*>()),
        m_lstTextRecords(ImsList<QosStatusRecord*>())
{
    IMS_TRACE_D("+QosStatusTable", 0, 0, 0);
}

PUBLIC VIRTUAL QosStatusTable::~QosStatusTable()
{
    IMS_TRACE_D("~QosStatusTable", 0, 0, 0);

    ClearStatusRecords(m_lstAudioRecords);
    ClearStatusRecords(m_lstVideoRecords);
    ClearStatusRecords(m_lstTextRecords);
}

PUBLIC
void QosStatusTable::UpdateStatusTableWithRemoteSdp(IN IMedia* piMedia)
{
    if (piMedia == IMS_NULL)
    {
        return;
    }

    IMediaDescriptor* piMediaDescriptor = IMS_NULL;
    if (piMedia->GetUpdateState() == IMedia::UPDATE_MODIFIED)
    {
        piMediaDescriptor = piMedia->GetProposal()->GetMediaDescriptor();
    }
    else
    {
        piMediaDescriptor = piMedia->GetMediaDescriptor();
    }

    if (piMediaDescriptor == IMS_NULL)
    {
        return;
    }

    const SdpMedia* pRemoteSdp = piMediaDescriptor->GetMediaDescriptionEx();

    if (pRemoteSdp == IMS_NULL)
    {
        IMS_TRACE_D("UpdateStatusTableWithRemoteSdp : no SDP from remote", 0, 0, 0);
        return;
    }

    IMS_SINT32 eSdpMediaType = pRemoteSdp->GetType();

    if (pRemoteSdp->GetPort() <= 0)
    {
        IMS_TRACE_D("UpdateStatusTableWithRemoteSdp : %s - port is 0, no update.",
                PS_SdpMediaType(eSdpMediaType), 0, 0);
        return;
    }
    else
    {
        IMS_TRACE_D("UpdateStatusTableWithRemoteSdp : %s", PS_SdpMediaType(eSdpMediaType), 0, 0);
    }

    InitializeDesChecked(eSdpMediaType);

    UpdateCurrentStatus(piMediaDescriptor, eSdpMediaType);
    UpdateDesiredStatus(piMediaDescriptor, eSdpMediaType);
}

PUBLIC
void QosStatusTable::UpdateLocalCurrentStatus(
        IN IMS_SINT32 eSdpMediaType, IN IMS_BOOL bLocalQoSEnabled)
{
    IMS_BOOL bLocalCurrentEnabled =
            IsCurrentStatusEnabled(eSdpMediaType, SdpPrecondition::STATUS_LOCAL);

    IMS_TRACE_D("UpdateLocalCurrentStatus : [%s] QoS Status[%s] Local Status[%s]",
            PS_SdpMediaType(eSdpMediaType), _TRACE_B_(bLocalQoSEnabled),
            _TRACE_B_(bLocalCurrentEnabled));

    if (bLocalQoSEnabled == bLocalCurrentEnabled)
    {
        IMS_TRACE_D("UpdateLocalCurrentStatus : already updated", 0, 0, 0);
        return;
    }

    // if Local QoS is enabled, set direction as direction tag of Desired Status.
    IMS_SINT32 eDesiredDir = (bLocalQoSEnabled)
            ? GetDirectionTag(eSdpMediaType, SdpAttribute::DES, SdpPrecondition::STATUS_LOCAL)
            : SdpPrecondition::DIRECTION_NONE;

    SetDirectionTag(eSdpMediaType, SdpAttribute::CURR, SdpPrecondition::STATUS_LOCAL, eDesiredDir);
}

PUBLIC
void QosStatusTable::EnableRemoteCurrentStatus(IN IMS_SINT32 eSdpMediaType)
{
    IMS_SINT32 eRemoteCurrDir =
            GetDirectionTag(eSdpMediaType, SdpAttribute::CURR, SdpPrecondition::STATUS_REMOTE);

    if (eRemoteCurrDir > SdpPrecondition::DIRECTION_NONE)  // compare with desired direction?
    {
        IMS_TRACE_D("EnableRemoteCurrentStatus : Remote Current Dir is %s, not to update.",
                PS_QosDir(eRemoteCurrDir), 0, 0);
        return;
    }

    IMS_SINT32 eRemoteDesiredDir =
            GetDirectionTag(eSdpMediaType, SdpAttribute::DES, SdpPrecondition::STATUS_REMOTE);

    SetDirectionTag(
            eSdpMediaType, SdpAttribute::CURR, SdpPrecondition::STATUS_REMOTE, eRemoteDesiredDir);
}

PUBLIC
IMS_BOOL QosStatusTable::IsCurrentStatusEnabled(
        IN IMS_SINT32 eSdpMediaType, IN IMS_SINT32 eStatusType)
{
    IMS_SINT32 eCurrDir = GetDirectionTag(eSdpMediaType, SdpAttribute::CURR, eStatusType);
    IMS_SINT32 eDesDir = GetDirectionTag(eSdpMediaType, SdpAttribute::DES, eStatusType);

    if (eCurrDir != SdpPrecondition::DIRECTION_NONE && eDesDir <= eCurrDir)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC
IMS_SINT32 QosStatusTable::GetDirectionTag(
        IN IMS_SINT32 eSdpMediaType, IN IMS_SINT32 eAttrType, IN IMS_SINT32 eStatusType)
{
    ImsList<QosStatusRecord*> lstRecords = GetStatusRecords(eSdpMediaType, eAttrType, eStatusType);
    IMS_SINT32 eDirTag = SdpPrecondition::DIRECTION_INVALID;

    for (IMS_UINT32 index = 0; index < lstRecords.GetSize(); index++)
    {
        QosStatusRecord* pRecord = lstRecords.GetAt(index);
        if ((pRecord->eAttrType == SdpAttribute::DES) &&
                (pRecord->eStrengthTag >= SdpPrecondition::STRENGTH_NONE))
        {
            continue;
        }

        if ((eDirTag == SdpPrecondition::DIRECTION_RECV &&
                    pRecord->eDirTag == SdpPrecondition::DIRECTION_SEND) ||
                (eDirTag == SdpPrecondition::DIRECTION_SEND &&
                        pRecord->eDirTag == SdpPrecondition::DIRECTION_RECV))
        {
            eDirTag = SdpPrecondition::DIRECTION_SENDRECV;
        }
        else
        {
            eDirTag = pRecord->eDirTag;
        }
    }

    return eDirTag;
}

PUBLIC
IMS_SINT32 QosStatusTable::GetStrengthTag(
        IN IMS_SINT32 eSdpMediaType, IN IMS_SINT32 eStatusType, IN IMS_SINT32 eDirTag)
{
    ImsList<QosStatusRecord*> lstRecords =
            GetStatusRecords(eSdpMediaType, SdpAttribute::DES, eStatusType, eDirTag);

    if (lstRecords.IsEmpty())
    {
        return SdpPrecondition::STRENGTH_NOTUSED;
    }

    QosStatusRecord* pRecord = lstRecords.GetAt(0);
    IMS_SINT32 eStrengthTag = pRecord->eStrengthTag;

    return eStrengthTag;
}

PUBLIC
void QosStatusTable::SetDirectionTag(IN IMS_SINT32 eSdpMediaType, IN IMS_SINT32 eAttrType,
        IN IMS_SINT32 eStatusType, IN IMS_SINT32 eDirTag)
{
    ImsList<QosStatusRecord*> objRecords = GetStatusRecords(eSdpMediaType, eAttrType, eStatusType);

    if (objRecords.IsEmpty())
    {
        return;
    }

    QosStatusRecord* pRecord = objRecords.GetAt(0);

    if ((pRecord->eDirTag != eDirTag))
    {
        IMS_TRACE_D("SetDirectionTag : media[%s] attr[%s] status[%s]",
                PS_SdpMediaType(eSdpMediaType), PS_QosAttribute(eAttrType),
                (eStatusType == SdpPrecondition::STATUS_LOCAL) ? "local" : "remote");
        IMS_TRACE_D("SetDirectionTag : [%s] -> [%s]", PS_QosDir(pRecord->eDirTag),
                PS_QosDir(eDirTag), 0);

        pRecord->eDirTag = eDirTag;
    }
}

PUBLIC
void QosStatusTable::SetStrengthTag(IN IMS_SINT32 eSdpMediaType, IN IMS_SINT32 eStatusType,
        IN IMS_SINT32 eDirTag, IN IMS_SINT32 eStrengthTag)
{
    ImsList<QosStatusRecord*> lstRecords =
            GetStatusRecords(eSdpMediaType, SdpAttribute::DES, eStatusType, eDirTag);

    if (lstRecords.IsEmpty())
    {
        return;
    }

    QosStatusRecord* pRecord = lstRecords.GetAt(0);
    if ((eStrengthTag == SdpPrecondition::STRENGTH_NOTUSED) ||
            (pRecord->eStrengthTag > eStrengthTag))
    {
        IMS_TRACE_D("SetStrengthTag : media[%s] status[%s] dir[%s]", PS_SdpMediaType(eSdpMediaType),
                (eStatusType == SdpPrecondition::STATUS_LOCAL) ? "local" : "remote",
                PS_QosDir(eDirTag));
        IMS_TRACE_D("SetStrengthTag : [%s] -> [%s]", PS_QosStrength(pRecord->eStrengthTag),
                PS_QosStrength(eStrengthTag), 0);

        pRecord->eStrengthTag = eStrengthTag;
    }

    pRecord->bDesiredCheck = IMS_TRUE;
}

PUBLIC
void QosStatusTable::SetLocalResourceConfirmed(IN IMS_SINT32 eSdpMediaType, IN IMS_BOOL bConfirmed)
{
    ImsList<QosStatusRecord*> lstStatusRecords =
            GetStatusRecords(eSdpMediaType, SdpAttribute::CURR, SdpPrecondition::STATUS_LOCAL);
    if (lstStatusRecords.GetSize() <= 0)
    {
        return;
    }

    QosStatusRecord* pRecord = lstStatusRecords.GetAt(0);
    pRecord->bLocalResourceConfirmed = bConfirmed;
}

PUBLIC
IMS_BOOL QosStatusTable::IsLocalResourceConfirmed(IN IMS_SINT32 eSdpMediaType)
{
    ImsList<QosStatusRecord*> lstStatusRecords =
            GetStatusRecords(eSdpMediaType, SdpAttribute::CURR, SdpPrecondition::STATUS_LOCAL);

    if (lstStatusRecords.GetSize() <= 0)
    {
        return IMS_FALSE;
    }

    QosStatusRecord* pRecord = lstStatusRecords.GetAt(0);
    IMS_BOOL bResult = pRecord->bLocalResourceConfirmed;
    IMS_TRACE_D("IsLocalResourceConfirmed : (%s) %s", PS_SdpMediaType(eSdpMediaType),
            _TRACE_B_(bResult), 0);

    return bResult;
}

PUBLIC
void QosStatusTable::CreateStatusRecords(IN IMS_SINT32 eSdpMediaType)
{
    ImsList<QosStatusRecord*>& lstStatusRecords = GetStatusRecords(eSdpMediaType);

    for (IMS_UINT32 index = 0; index < lstStatusRecords.GetSize(); index++)
    {
        QosStatusRecord* pRemoveRecord = lstStatusRecords.GetAt(index);

        delete pRemoveRecord;
    }

    lstStatusRecords.Clear();

    AddStatusRecord(eSdpMediaType);

    IMS_TRACE_D("CreateStatusRecords : %s", PS_SdpMediaType(eSdpMediaType), 0, 0);
}

PUBLIC
IMS_BOOL QosStatusTable::IsStatusRecordsListEmpty(IN IMS_SINT32 eSdpMediaType)
{
    const ImsList<QosStatusRecord*>& lstStatusRecords = GetStatusRecords(eSdpMediaType);
    return lstStatusRecords.IsEmpty();
}

PUBLIC
void QosStatusTable::RemoveUnusedStatusRecords(IN IMS_UINT32 eMediaTypes)
{
    if (!(eMediaTypes & MEDIATYPE_AUDIO) && !m_lstAudioRecords.IsEmpty())
    {
        IMS_TRACE_D("RemoveUnusedStatusRecords : audio", 0, 0, 0);
        ClearStatusRecords(m_lstAudioRecords);
    }

    if (!(eMediaTypes & MEDIATYPE_VIDEO) && !m_lstVideoRecords.IsEmpty())
    {
        IMS_TRACE_D("RemoveUnusedStatusRecords : video", 0, 0, 0);
        ClearStatusRecords(m_lstVideoRecords);
    }

    if (!(eMediaTypes & MEDIATYPE_TEXT) && !m_lstTextRecords.IsEmpty())
    {
        IMS_TRACE_D("RemoveUnusedStatusRecords : text", 0, 0, 0);
        ClearStatusRecords(m_lstTextRecords);
    }
}

PRIVATE
void QosStatusTable::AddStatusRecord(IN IMS_SINT32 eSdpMediaType)
{
    ImsList<QosStatusRecord*>& lstRecords = GetStatusRecords(eSdpMediaType);

    // curr - local
    lstRecords.Append(
            new QosStatusRecord(eSdpMediaType, SdpAttribute::CURR, SdpPrecondition::STATUS_LOCAL,
                    SdpPrecondition::DIRECTION_NONE, SdpPrecondition::STRENGTH_NOTUSED));
    // curr - remote
    lstRecords.Append(
            new QosStatusRecord(eSdpMediaType, SdpAttribute::CURR, SdpPrecondition::STATUS_REMOTE,
                    SdpPrecondition::DIRECTION_NONE, SdpPrecondition::STRENGTH_NOTUSED));

    // des - local
    lstRecords.Append(
            new QosStatusRecord(eSdpMediaType, SdpAttribute::DES, SdpPrecondition::STATUS_LOCAL,
                    SdpPrecondition::DIRECTION_SEND, SdpPrecondition::STRENGTH_MANDATORY));
    lstRecords.Append(
            new QosStatusRecord(eSdpMediaType, SdpAttribute::DES, SdpPrecondition::STATUS_LOCAL,
                    SdpPrecondition::DIRECTION_RECV, SdpPrecondition::STRENGTH_MANDATORY));
    // des - remote
    lstRecords.Append(
            new QosStatusRecord(eSdpMediaType, SdpAttribute::DES, SdpPrecondition::STATUS_REMOTE,
                    SdpPrecondition::DIRECTION_SEND, SdpPrecondition::STRENGTH_OPTIONAL));
    lstRecords.Append(
            new QosStatusRecord(eSdpMediaType, SdpAttribute::DES, SdpPrecondition::STATUS_REMOTE,
                    SdpPrecondition::DIRECTION_RECV, SdpPrecondition::STRENGTH_OPTIONAL));

    // conf
    lstRecords.Append(
            new QosStatusRecord(eSdpMediaType, SdpAttribute::CONF, SdpPrecondition::STATUS_REMOTE,
                    SdpPrecondition::DIRECTION_SENDRECV, SdpPrecondition::STRENGTH_NOTUSED));
}

PRIVATE
void QosStatusTable::InitializeDesChecked(IN IMS_SINT32 eSdpMediaType)
{
    IMS_TRACE_D("InitializeDesChecked : %s", PS_SdpMediaType(eSdpMediaType), 0, 0);

    ImsList<QosStatusRecord*>& lstStatusRecords = GetStatusRecords(eSdpMediaType);

    for (IMS_UINT32 index = 0; index < lstStatusRecords.GetSize(); index++)
    {
        QosStatusRecord* pStatusRecord = lstStatusRecords.GetAt(index);
        if (pStatusRecord->eAttrType == SdpAttribute::DES)
        {
            pStatusRecord->bDesiredCheck = IMS_FALSE;
        }
    }
}

PRIVATE
void QosStatusTable::UpdateCurrentStatus(
        IN IMediaDescriptor* piMediaDescriptor, IN IMS_SINT32 eSdpMediaType)
{
    const SdpSegmentedPrecondition* pCurr = DYNAMIC_CAST(const SdpSegmentedPrecondition*,
            piMediaDescriptor->GetPrecondition(SdpAttribute::CURR));
    if (pCurr == IMS_NULL)
    {
        IMS_TRACE_D("UpdateCurrentStatus : no update.", 0, 0, 0);
        return;
    }

    // curr:qos local direction - it is omitted because the status will be updated with
    // UpdateLocalCurrStatus() based on Local QoS Status

    // curr:qos remote direction
    // update remote current status based on local detail infos.
    const ImsList<SdpPrecondition::DetailInfo>& lstLocalDetails = pCurr->GetLocalDetails();
    IMS_UINT32 nSize = lstLocalDetails.GetSize();

    IMS_SINT32 eRemoteCurrDir =
            GetDirectionTag(eSdpMediaType, SdpAttribute::CURR, SdpPrecondition::STATUS_REMOTE);

    for (IMS_UINT32 index = 0; index < nSize; index++)
    {
        const SdpPrecondition::DetailInfo& detailInfo = lstLocalDetails.GetAt(index);
        const IMS_SINT32 eDirInSdp = detailInfo.GetDirection();

        if (eRemoteCurrDir != SdpPrecondition::DIRECTION_NONE &&
                eDirInSdp == SdpPrecondition::DIRECTION_NONE)
        {
            IMS_TRACE_D("UpdateCurrentStatus : no update.", 0, 0, 0);
        }
        else
        {
            IMS_TRACE_D("UpdateCurrentStatus : update remote current status", 0, 0, 0);
            SetDirectionTag(eSdpMediaType, SdpAttribute::CURR, SdpPrecondition::STATUS_REMOTE,
                    detailInfo.GetDirection());
        }
    }
}

PRIVATE
void QosStatusTable::UpdateDesiredStatus(
        IN IMediaDescriptor* piMediaDescriptor, IN IMS_SINT32 eSdpMediaType)
{
    const SdpSegmentedPrecondition* pDes = DYNAMIC_CAST(
            const SdpSegmentedPrecondition*, piMediaDescriptor->GetPrecondition(SdpAttribute::DES));
    if (pDes == IMS_NULL)
    {
        IMS_TRACE_D("UpdateDesiredStatus : no update.", 0, 0, 0);
        return;
    }

    ImsList<QosStatusRecord*>& lstStatusRecords = GetStatusRecords(eSdpMediaType);

    // des:qos strength local direction
    // update strength of local desired status based on remote detail infos.
    const ImsList<SdpPrecondition::DetailInfo>& lstRemoteDetails = pDes->GetRemoteDetails();

    IMS_TRACE_D("UpdateDesiredStatus : update local desired status, size [%d]",
            lstRemoteDetails.GetSize(), 0, 0);

    for (IMS_UINT32 index = 0; index < lstRemoteDetails.GetSize(); index++)
    {
        const SdpPrecondition::DetailInfo& detailInfo = lstRemoteDetails.GetAt(index);

        if (detailInfo.GetDirection() == SdpPrecondition::DIRECTION_SENDRECV)
        {
            SetStrengthTag(eSdpMediaType, SdpPrecondition::STATUS_LOCAL,
                    SdpPrecondition::DIRECTION_SEND, detailInfo.GetStrength());
            SetStrengthTag(eSdpMediaType, SdpPrecondition::STATUS_LOCAL,
                    SdpPrecondition::DIRECTION_RECV, detailInfo.GetStrength());
        }
        else
        {
            SetStrengthTag(eSdpMediaType, SdpPrecondition::STATUS_LOCAL, detailInfo.GetDirection(),
                    detailInfo.GetStrength());
        }
    }

    // des:qos strength remote direction
    // update strength of remote desired status based on local detail infos.
    const ImsList<SdpPrecondition::DetailInfo>& lstLocalDetails = pDes->GetLocalDetails();
    IMS_TRACE_D("UpdateDesiredStatus : update remote desired status, size [%d]",
            lstLocalDetails.GetSize(), 0, 0);

    for (IMS_UINT32 index = 0; index < lstLocalDetails.GetSize(); index++)
    {
        const SdpPrecondition::DetailInfo& detailInfo = lstLocalDetails.GetAt(index);

        if (detailInfo.GetDirection() == SdpPrecondition::DIRECTION_SENDRECV)
        {
            SetStrengthTag(eSdpMediaType, SdpPrecondition::STATUS_REMOTE,
                    SdpPrecondition::DIRECTION_SEND, detailInfo.GetStrength());
            SetStrengthTag(eSdpMediaType, SdpPrecondition::STATUS_REMOTE,
                    SdpPrecondition::DIRECTION_RECV, detailInfo.GetStrength());
        }
        else
        {
            SetStrengthTag(eSdpMediaType, SdpPrecondition::STATUS_REMOTE, detailInfo.GetDirection(),
                    detailInfo.GetStrength());
        }
    }

    // set strength as "not used" to desired status which is not using for negotiation.
    for (IMS_UINT32 index = 0; index < lstStatusRecords.GetSize(); index++)
    {
        QosStatusRecord* pStatusRecord = lstStatusRecords.GetAt(index);
        if ((pStatusRecord->eAttrType == SdpAttribute::DES) && !(pStatusRecord->bDesiredCheck))
        {
            SetStrengthTag(pStatusRecord->eSdpMediaType, pStatusRecord->eStatusType,
                    pStatusRecord->eDirTag, SdpPrecondition::STRENGTH_NOTUSED);
        }
    }
}

PRIVATE
ImsList<QosStatusRecord*>& QosStatusTable::GetStatusRecords(IN IMS_SINT32 eSdpMediaType)
{
    if (eSdpMediaType == SdpMedia::TYPE_AUDIO)
    {
        return m_lstAudioRecords;
    }
    else if (eSdpMediaType == SdpMedia::TYPE_VIDEO)
    {
        return m_lstVideoRecords;
    }
    else
    {
        return m_lstTextRecords;
    }
}

PRIVATE
ImsList<QosStatusRecord*> QosStatusTable::GetStatusRecords(IN IMS_SINT32 eSdpMediaType,
        IN IMS_SINT32 eAttrType, IN IMS_SINT32 eStatusType,
        IN IMS_SINT32 eDirTag /*= SdpPrecondition::DIRECTION_NONE*/)
{
    ImsList<QosStatusRecord*> lstStatusRecords;
    ImsList<QosStatusRecord*>& lstTempRecords = GetStatusRecords(eSdpMediaType);

    for (IMS_UINT32 index = 0; index < lstTempRecords.GetSize(); index++)
    {
        QosStatusRecord* pRecord = lstTempRecords.GetAt(index);

        if ((pRecord->eAttrType == eAttrType) && (pRecord->eStatusType == eStatusType) &&
                (pRecord->eDirTag == eDirTag || eDirTag == SdpPrecondition::DIRECTION_NONE))
        {
            lstStatusRecords.Append(pRecord);
        }
    }

    return lstStatusRecords;
}

PRIVATE
void QosStatusTable::ClearStatusRecords(IN ImsList<QosStatusRecord*>& lstRecords)
{
    IMS_UINT32 nSize = lstRecords.GetSize();
    IMS_TRACE_D("ClearStatusRecords : size[%d]", nSize, 0, 0);

    for (IMS_UINT32 index = 0; index < nSize; index++)
    {
        QosStatusRecord* pRecord = lstRecords.GetAt(index);
        if (pRecord == IMS_NULL)
        {
            continue;
        }

        delete pRecord;
    }

    lstRecords.Clear();
}
