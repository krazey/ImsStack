/**
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

#include "ServiceTrace.h"
#include "config/MediaSessionConfigFactory.h"

static MediaSessionConfigFactory* g_pMediaSessionConfigFactory = IMS_NULL;

__IMS_TRACE_TAG_MEDIA__;

PRIVATE
MediaSessionConfigFactory::MediaSessionConfigFactory() {}

PUBLIC VIRTUAL MediaSessionConfigFactory::~MediaSessionConfigFactory()
{
    for (IMS_UINT32 nIdx = 0; nIdx <= IMS_SLOT_1; nIdx++)
    {
        DestroyListSessionConfig(nIdx);
        m_mapListMediaSessionConfig.Remove(nIdx);
    }
}

PUBLIC
void MediaSessionConfigFactory::CreateMediaSessionConfig(
        IN IMS_SINT32 nSlotId, IN MEDIA_SERVICE_TYPE eServiceType)
{
    ImsList<MediaSessionConfig*>* pListMediaSessionConfig = GetListSessionConfig(nSlotId);

    if (pListMediaSessionConfig == IMS_NULL)
    {
        pListMediaSessionConfig = new ImsList<MediaSessionConfig*>();
        m_mapListMediaSessionConfig.Add(nSlotId, pListMediaSessionConfig);
    }

    if (FindMediaSessionConfig(nSlotId, eServiceType) == IMS_NULL)
    {
        MediaSessionConfig* objMediaSessionConfig = new MediaSessionConfig(nSlotId, eServiceType);
        pListMediaSessionConfig->Append(objMediaSessionConfig);
    }

    IMS_TRACE_D("CreateMediaSessionConfig - nSlotId[%d], listSize[%d], svc[%d]", nSlotId,
            pListMediaSessionConfig->GetSize(), (IMS_SINT32)eServiceType);
}

PUBLIC
void MediaSessionConfigFactory::AddMediaSessionConfig(
        IN IMS_SINT32 nSlotId, IN MediaSessionConfig* pMediaSessionConfig)
{
    ImsList<MediaSessionConfig*>* pListMediaSessionConfig = GetListSessionConfig(nSlotId);

    if (pListMediaSessionConfig == IMS_NULL)
    {
        pListMediaSessionConfig = new ImsList<MediaSessionConfig*>();
        pListMediaSessionConfig->Append(pMediaSessionConfig);

        IMS_TRACE_D("AddMediaSessionConfig - nSlotId[%d], listSize[%d]", nSlotId,
                pListMediaSessionConfig->GetSize(), 0);

        m_mapListMediaSessionConfig.Add(nSlotId, pListMediaSessionConfig);
    }
    else
    {
        for (IMS_UINT32 nIdxList = 0; nIdxList < pListMediaSessionConfig->GetSize(); nIdxList++)
        {
            MediaSessionConfig* pTempMediaSessionConfig = pListMediaSessionConfig->GetAt(nIdxList);
            if (pTempMediaSessionConfig == pMediaSessionConfig)  /// Already Have
            {
                return;
            }
        }

        pListMediaSessionConfig->Append(pMediaSessionConfig);

        IMS_TRACE_D("AddMediaSessionConfig - nSlotId[%d], listSize[%d]", nSlotId,
                pListMediaSessionConfig->GetSize(), 0);
    }
}

PUBLIC
void MediaSessionConfigFactory::DestroyListSessionConfig(IN IMS_SINT32 nSlotId)
{
    ImsList<MediaSessionConfig*>* pListMediaSessionConfig = GetListSessionConfig(nSlotId);

    if (pListMediaSessionConfig != IMS_NULL)
    {
        IMS_TRACE_D(
                "DestroyListSessionConfig - list[%d]", pListMediaSessionConfig->GetSize(), 0, 0);

        for (IMS_UINT32 nIdxList = 0; nIdxList < pListMediaSessionConfig->GetSize(); nIdxList++)
        {
            MediaSessionConfig* pMediaSessionConfig = pListMediaSessionConfig->GetAt(nIdxList);
            DestroySessionConfig(pMediaSessionConfig);
        }

        pListMediaSessionConfig->Clear();
        delete pListMediaSessionConfig;
    }
}

PUBLIC
ImsList<MediaSessionConfig*>* MediaSessionConfigFactory::GetListSessionConfig(IN IMS_SINT32 nSlotId)
{
    if (m_mapListMediaSessionConfig.GetIndexOfKey(nSlotId) >= 0)
    {
        return m_mapListMediaSessionConfig.GetValue(nSlotId);
    }

    return IMS_NULL;
}

PUBLIC
MediaSessionConfig* MediaSessionConfigFactory::FindMediaSessionConfig(
        IN IMS_SINT32 nSlotId, IN MEDIA_SERVICE_TYPE eServiceType)
{
    ImsList<MediaSessionConfig*>* pListMediaSessionConfig = GetListSessionConfig(nSlotId);

    MediaSessionConfig* pMediaSessionConfig = IMS_NULL;

    if (pListMediaSessionConfig != IMS_NULL)
    {
        for (IMS_UINT32 nIdxList = 0; nIdxList < pListMediaSessionConfig->GetSize(); nIdxList++)
        {
            pMediaSessionConfig = pListMediaSessionConfig->GetAt(nIdxList);

            if (pMediaSessionConfig->GetServiceType() == eServiceType ||
                    MEDIA_SERVICE_NONE == eServiceType)
            {
                return pMediaSessionConfig;
            }
        }
    }

    if (pListMediaSessionConfig != IMS_NULL)
    {
        if (pMediaSessionConfig == IMS_NULL && pListMediaSessionConfig->GetSize() > 0)
        {
            pMediaSessionConfig = pListMediaSessionConfig->GetAt(0);
        }
    }

    if (pMediaSessionConfig != NULL)
    {
        IMS_TRACE_D("FindMediaSessionConfig() - eServiceType[%d]",
                pMediaSessionConfig->GetServiceType(), 0, 0);
    }

    return pMediaSessionConfig;
}

PUBLIC
void MediaSessionConfigFactory::DestroySessionConfig(
        IN const MediaSessionConfig* pMediaSessionConfig)
{
    for (IMS_UINT32 nIdxSlot = 0; nIdxSlot <= IMS_SLOT_1; nIdxSlot++)
    {
        ImsList<MediaSessionConfig*>* pListMediaSessionConfig = GetListSessionConfig(nIdxSlot);

        if (pListMediaSessionConfig != IMS_NULL)
        {
            IMS_TRACE_D("DestroySessionConfig list[%d]", pListMediaSessionConfig->GetSize(), 0, 0);

            for (IMS_UINT32 nIdxList = 0; nIdxList < pListMediaSessionConfig->GetSize(); nIdxList++)
            {
                MediaSessionConfig* pTempMediaSessionConfig =
                        pListMediaSessionConfig->GetAt(nIdxList);

                if (pTempMediaSessionConfig == pMediaSessionConfig)
                {
                    pListMediaSessionConfig->RemoveAt(nIdxList);
                    return;
                }
            }
        }
    }
}

PUBLIC
MediaSessionConfigFactory* MediaSessionConfigFactory::GetInstance()
{
    if (g_pMediaSessionConfigFactory == IMS_NULL)
    {
        g_pMediaSessionConfigFactory = new MediaSessionConfigFactory();
    }

    return g_pMediaSessionConfigFactory;
}

PUBLIC
void MediaSessionConfigFactory::ReleaseInstance(MediaSessionConfigFactory* pSessionConfigFactory)
{
    if (pSessionConfigFactory != IMS_NULL && pSessionConfigFactory == g_pMediaSessionConfigFactory)
    {
        delete pSessionConfigFactory;
        g_pMediaSessionConfigFactory = IMS_NULL;
    }
}
