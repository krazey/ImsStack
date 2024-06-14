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

#include "BaseNego.h"
#include "MediaNegoUtil.h"
#include "MediaProfileFactory.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC BaseNego::BaseNego(IMS_SINT32 nSlotId) :
        ImsSlot(nSlotId),
        m_pBaseProfile(new MediaBaseProfile()),
        m_listOaModel(ImsList<OaModel*>()),
        m_pConfig(IMS_NULL),
        m_pEnvironment(IMS_NULL)
{
    IMS_TRACE_I("+BaseNego() - slot[%d]", nSlotId, 0, 0);
}

PUBLIC VIRTUAL BaseNego::~BaseNego()
{
    IMS_TRACE_I("~BaseNego()", 0, 0, 0);

    if (m_pBaseProfile != IMS_NULL)
    {
        MediaNegoUtil::ReleaseRtpPort(GetSlotId(), m_pBaseProfile->nDataPort);
        m_pBaseProfile->DeletePayloads();
    }

    delete m_pBaseProfile;
    m_pBaseProfile = IMS_NULL;

    DestroyListOaModel();
}

PUBLIC VIRTUAL void BaseNego::CreateProfiles(IN MediaEnvironment* pEnvironment,
        IN MEDIA_CONTENT_TYPE pType, IN MediaConfiguration* pConfig)
{
    if (pConfig == IMS_NULL || pEnvironment == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateProfiles() - invalid configuration", 0, 0, 0);
        return;
    }

    if (m_pBaseProfile != IMS_NULL && m_pBaseProfile->nDataPort != 0)
    {
        MediaNegoUtil::ReleaseRtpPort(GetSlotId(), m_pBaseProfile->nDataPort);
    }
    delete m_pBaseProfile;

    IMS_TRACE_I("CreateProfiles()", 0, 0, 0);

    m_pEnvironment = pEnvironment;
    m_pConfig = pConfig;
    m_pBaseProfile = MediaProfileFactory::GetInstance()->CreateProfile(
            pEnvironment, m_pConfig, GetSlotId(), pType);
}

PUBLIC VIRTUAL void BaseNego::FinalizeSdp(
        IN ISessionDescriptor* pSessionDescriptor, IN NEGO_STATE eNegoState)
{
    IMS_BOOL bFoundOaModel = IMS_FALSE;

    // reset confirmed Session check variable
    for (IMS_UINT32 i = 0; i < m_listOaModel.GetSize(); i++)
    {
        OaModel* pCheckedOaModel = m_listOaModel.GetAt(i);

        if (pCheckedOaModel != IMS_NULL)
        {
            pCheckedOaModel->bConfirmedSession = IMS_FALSE;
        }
    }

    // check latest OA model
    OaModel* pLatestOaModel = IMS_NULL;

    if (m_listOaModel.GetSize() > 0)
    {
        pLatestOaModel = m_listOaModel.GetAt(m_listOaModel.GetSize() - 1);
    }

    if (pLatestOaModel != IMS_NULL)
    {
        if ((pLatestOaModel->IsAllProfileExist() &&
                    (eNegoState == STATE_IDLE || eNegoState == STATE_NEGOTIATED)) == IMS_FALSE)
        {
            IMS_TRACE_I("FinalizeSdp() - Incomplete OaModel[%d]. Delete profile",
                    m_listOaModel.GetSize() - 1, 0, 0);
            delete pLatestOaModel;
            m_listOaModel.RemoveAt(m_listOaModel.GetSize() - 1);
        }
    }

    for (IMS_UINT32 i = 0; i < m_listOaModel.GetSize(); i++)
    {
        // get OaModel
        OaModel* pTempOaModel = m_listOaModel.GetAt(m_listOaModel.GetSize() - 1 - i);

        // find matched SessionDescriptor key
        if (pTempOaModel != IMS_NULL)
        {
            if (pTempOaModel->nSessionDescriptorKey ==
                    reinterpret_cast<IMS_SINTP>(pSessionDescriptor))
            {
                pTempOaModel->bConfirmedSession = IMS_TRUE;
                bFoundOaModel = IMS_TRUE;
                IMS_TRACE_D("FinalizeSdp() - find comfirmed Session OaModel [%d]",
                        m_listOaModel.GetSize() - i, 0, 0);
                break;
            }
        }
    }

    // SessionDescriptor key mismatch case handling, not select OaModel
    if (bFoundOaModel != IMS_TRUE && m_listOaModel.GetSize() > 0)
    {
        IMS_TRACE_D("FinalizeSdp() - not found comfirmed Session OaModel", 0, 0, 0);
    }
}

PROTECTED VIRTUAL MediaBaseProfile* BaseNego::GetLocalProfile(IN OaModel* pOaModel)
{
    return (pOaModel != IMS_NULL) ? pOaModel->pLocalProfile : IMS_NULL;
}
PROTECTED VIRTUAL MediaBaseProfile* BaseNego::GetPeerProfile(IN OaModel* pOaModel)
{
    return (pOaModel != IMS_NULL) ? pOaModel->pPeerProfile : IMS_NULL;
}
PROTECTED VIRTUAL MediaBaseProfile* BaseNego::GetNegotiatedProfile(IN OaModel* pOaModel)
{
    return (pOaModel != IMS_NULL) ? pOaModel->pNegotiatedProfile : IMS_NULL;
}

PUBLIC IMS_BOOL BaseNego::SetPort(IN IMS_UINT32 nPort)
{
    if (m_pBaseProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    MediaNegoUtil::ReleaseRtpPort(GetSlotId(), m_pBaseProfile->nDataPort);

    IMS_TRACE_I("SetPort() - Changed Data Port[%d]->[%d]", m_pBaseProfile->nDataPort, nPort, 0);

    if (nPort != 0)
    {
        m_pBaseProfile->nDataPort = MediaNegoUtil::AcquireRtpPort(GetSlotId(), nPort);
        m_pBaseProfile->nControlPort = m_pBaseProfile->nDataPort + 1;
    }
    else
    {
        m_pBaseProfile->nDataPort = 0;
        m_pBaseProfile->nControlPort = 0;

        IMS_TRACE_I("SetPort() - Data Port is 0!!!", 0, 0, 0);
    }

    return IMS_TRUE;
}

PROTECTED void BaseNego::DestroyListOaModel()
{
    while (m_listOaModel.GetSize() > 0)
    {
        OaModel* pOaModel = m_listOaModel.GetAt(0);

        if (pOaModel != IMS_NULL)
        {
            delete pOaModel;
        }
        m_listOaModel.RemoveAt(0);
    }
}
