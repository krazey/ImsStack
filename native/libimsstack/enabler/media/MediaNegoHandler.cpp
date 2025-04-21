/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "MediaNegoHandler.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC
MediaNegoHandler::MediaNegoHandler(
        IMS_UINT32 nSlotId, std::shared_ptr<MediaEnvironment> pEnvironment) :
        m_nSlotId(nSlotId),
        m_pEnvironment(pEnvironment)
{
    IMS_TRACE_I("+MediaNegoHandler(): SlotId[%u]", m_nSlotId, 0, 0);
}

PUBLIC
MediaNegoHandler::~MediaNegoHandler()
{
    IMS_TRACE_I("~MediaNegoHandler(): SlotId[%u]", m_nSlotId, 0, 0);
    ClearAllMediaNego();
}

PUBLIC
MediaNego* MediaNegoHandler::CreateMediaNego(IMS_UINTP nNegoId)
{
    IMS_TRACE_I("CreateMediaNego(): NegoId[%" PFLS_x "]", nNegoId, 0, 0);

    // Create new MediaNego
    MediaNego* pMediaNego = new MediaNego(m_nSlotId);

    // Copy Existed Media Nego with nego id
    if (nNegoId != 0)
    {
        MediaNego* objExistingNego = FindMediaNego(nNegoId);

        if (objExistingNego == IMS_NULL)
        {
            IMS_TRACE_I("CreateMediaNego(): invalid negoId", 0, 0, 0);
            return IMS_NULL;
        }

        pMediaNego->Forking(objExistingNego);
    }
    else
    {
        pMediaNego->CreateProfile(m_pEnvironment);
    }

    m_objMapMediaNego.Add(reinterpret_cast<IMS_UINTP>(pMediaNego), pMediaNego);
    return pMediaNego;
}

PUBLIC
MediaNego* MediaNegoHandler::FindMediaNego(IMS_UINTP nNegoId)
{
    IMS_SLONG nIndex = m_objMapMediaNego.GetIndexOfKey(nNegoId);
    if (nIndex < 0)
    {
        IMS_TRACE_E(0, "FindMediaNego(): invalid NegoId[%" PFLS_x "]", nNegoId, 0, 0);
        return IMS_NULL;
    }

    return m_objMapMediaNego.GetValueAt(nIndex);
}

PUBLIC
IMS_BOOL MediaNegoHandler::DeleteMediaNego(IMS_UINTP nNegoId)
{
    IMS_TRACE_D("DeleteMediaNego(): NegoId[%" PFLS_x "], Size[%d]", nNegoId,
            m_objMapMediaNego.GetSize(), 0);

    if (nNegoId == UNDEFINED_NEGO_ID)
    {
        return IMS_FALSE;
    }

    IMS_SLONG nIndex = m_objMapMediaNego.GetIndexOfKey(nNegoId);

    if (nIndex < 0)
    {
        IMS_TRACE_E(0, "DeleteMediaNego(): invalid NegoId[%" PFLS_x "]", nNegoId, 0, 0);
        return IMS_FALSE;
    }

    MediaNego* pMediaNego = m_objMapMediaNego.GetValueAt(nIndex);
    m_objMapMediaNego.RemoveAt(nIndex);

    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "DeleteMediaNego(): pMediaNego is NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    delete pMediaNego;

    return IMS_TRUE;
}

PUBLIC
void MediaNegoHandler::ClearAllMediaNego()
{
    IMS_TRACE_D("ClearMediaNego(): size[%d]", m_objMapMediaNego.GetSize(), 0, 0);

    while (!m_objMapMediaNego.IsEmpty())
    {
        MediaNego* pMediaNego = m_objMapMediaNego.GetValueAt(0);

        if (pMediaNego != IMS_NULL)
        {
            delete pMediaNego;
        }

        m_objMapMediaNego.RemoveAt(0);
    }

    m_objMapMediaNego.Clear();
}

PUBLIC
IMS_BOOL MediaNegoHandler::FormSdp(IMS_UINTP nNegoId, OUT ISession* pSession,
        MEDIA_CONTENT_TYPE eType, IMS_SINT32 nAudioDirection, IMS_SINT32 nVideoDirection,
        IMS_SINT32 nTextDirection, IMS_BOOL bEnforceReofferMode)
{
    MediaNego* pMediaNego = FindMediaNego(nNegoId);

    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "FormSdp(): NegoId[%" PFLS_x "] not found", nNegoId, 0, 0);
        return IMS_FALSE;
    }

    return pMediaNego->FormSdp(
            pSession, eType, nAudioDirection, nVideoDirection, nTextDirection, bEnforceReofferMode);
}

PUBLIC
MEDIA_CONTENT_TYPE MediaNegoHandler::GetSupportedMediaTypesFromSdp(
        IMS_UINTP nNegoId, IN ISession* pSession)
{
    MediaNego* pMediaNego = FindMediaNego(nNegoId);

    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(
                0, "GetSupportedMediaTypesFromSdp(): NegoId[%" PFLS_x "] not found", nNegoId, 0, 0);
        return MEDIA_TYPE_INVALID;
    }

    return pMediaNego->GetSupportedMediaTypesFromSdp(pSession);
}

PUBLIC
IMS_BOOL MediaNegoHandler::NegotiateSdp(IMS_UINTP nNegoId, IN ISession* pSession,
        OUT IMS_SINT32* nAudioDirection, OUT IMS_SINT32* nVideoDirection,
        OUT IMS_SINT32* nTextDirection, OUT MediaNego::MediaNegoResult& errorReason)
{
    if (!nAudioDirection || !nVideoDirection || !nTextDirection)
    {
        IMS_TRACE_E(0, "NegotiateSdp(): Output direction pointers are NULL", 0, 0, 0);
        errorReason = MediaNego::ERROR_INVALID_DESCRIPTOR;
        return IMS_FALSE;
    }

    MediaNego* pMediaNego = FindMediaNego(nNegoId);
    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateSdp(): NegoId[%" PFLS_x "] not found", nNegoId, 0, 0);
        errorReason = MediaNego::ERROR_INVALID_DESCRIPTOR;  // Indicate failure due to missing Nego
        *nAudioDirection = MEDIA_DIRECTION_INVALID;
        *nVideoDirection = MEDIA_DIRECTION_INVALID;
        *nTextDirection = MEDIA_DIRECTION_INVALID;
        return IMS_FALSE;
    }

    return pMediaNego->NegotiateSdp(
            pSession, *nAudioDirection, *nVideoDirection, *nTextDirection, errorReason);
}

PUBLIC
void MediaNegoHandler::FinalizeSdp(IMS_UINTP nNegoId, IN ISession* pSession)
{
    MediaNego* pMediaNego = FindMediaNego(nNegoId);

    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "FinalizeSdp(): NegoId[%" PFLS_x "] not found", nNegoId, 0, 0);
        return;
    }

    pMediaNego->FinalizeSdp(pSession);
}

PUBLIC
NEGO_STATE MediaNegoHandler::GetNegoState(IMS_UINTP nNegoId)
{
    MediaNego* pMediaNego = FindMediaNego(nNegoId);

    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetNegoState(): NegoId[%" PFLS_x "] not found", nNegoId, 0, 0);
        return STATE_NOTUSED;
    }

    return pMediaNego->GetNegoState();
}

PUBLIC
MEDIA_CONTENT_TYPE MediaNegoHandler::GetNegotiatedMediaType(IMS_UINTP nNegoId)
{
    MediaNego* pMediaNego = FindMediaNego(nNegoId);
    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetNegotiatedMediaType(): NegoId[%" PFLS_x "] not found", nNegoId, 0, 0);
        return MEDIA_TYPE_INVALID;
    }

    // Re-implement the logic here as it depends on multiple calls to MediaNego
    MEDIA_CONTENT_TYPE eMedia = MEDIA_TYPE_INVALID;

    if (pMediaNego->GetNegotiatedAudioQuality() != AUDIO_CODEC_NOT_USED)
    {
        eMedia = static_cast<MEDIA_CONTENT_TYPE>(
                (static_cast<IMS_SINT32>(eMedia) | static_cast<IMS_SINT32>(MEDIA_TYPE_AUDIO)));
    }

    if (pMediaNego->GetNegotiatedVideoQuality() != VIDEO_RESOLUTION_NOT_USED)
    {
        eMedia = static_cast<MEDIA_CONTENT_TYPE>(
                (static_cast<IMS_SINT32>(eMedia) | static_cast<IMS_SINT32>(MEDIA_TYPE_VIDEO)));
    }

    if (pMediaNego->GetNegotiatedTextQuality() != TEXT_CODEC_NOT_USED)
    {
        eMedia = static_cast<MEDIA_CONTENT_TYPE>(
                (static_cast<IMS_SINT32>(eMedia) | static_cast<IMS_SINT32>(MEDIA_TYPE_TEXT)));
    }

    return eMedia;
}

PUBLIC
IMS_SINT32 MediaNegoHandler::GetNegotiatedQuality(IMS_UINTP nNegoId, MEDIA_CONTENT_TYPE eType)
{
    MediaNego* pMediaNego = FindMediaNego(nNegoId);
    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetNegotiatedQuality(): NegoId[%" PFLS_x "] not found", nNegoId, 0, 0);
        return 0;
    }

    switch (eType)
    {
        case MEDIA_TYPE_AUDIO:
            return static_cast<IMS_SINT32>(pMediaNego->GetNegotiatedAudioQuality());
        case MEDIA_TYPE_VIDEO:
            return static_cast<IMS_SINT32>(pMediaNego->GetNegotiatedVideoQuality());
        case MEDIA_TYPE_TEXT:
            return static_cast<IMS_SINT32>(pMediaNego->GetNegotiatedTextQuality());
        default:
            IMS_TRACE_I("GetNegotiatedQuality(): Invalid media type[%d]", eType, 0, 0);
            return 0;
    }
}

PUBLIC
IMS_SINT32 MediaNegoHandler::GetNegotiatedCodecBitrate(IMS_UINTP nNegoId, MEDIA_CONTENT_TYPE eType)
{
    MediaNego* pMediaNego = FindMediaNego(nNegoId);
    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetNegotiatedCodecBitrate(): NegoId[%" PFLS_x "] not found", nNegoId, 0, 0);
        return 0;
    }

    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(
                0, "GetNegotiatedCodecBitrate(): Can't find NegoId[%" PFLS_x "]", nNegoId, 0, 0);
        return 0;
    }

    if (MEDIA_IS_CONTAINED_THIS_TYPE(eType, MEDIA_TYPE_AUDIO))
    {
        std::shared_ptr<AudioNego> pAudioNego = pMediaNego->GetAudioNego();

        if (pAudioNego == IMS_NULL)
        {
            return static_cast<IMS_SINT32>(AUDIO_CODEC_BITRATE_MAX);
        }

        return static_cast<IMS_SINT32>(pAudioNego->GetNegotiatedAudioCodecRate());
    }
    else if (MEDIA_IS_CONTAINED_THIS_TYPE(eType, MEDIA_TYPE_VIDEO))
    {
        std::shared_ptr<VideoNego> pVideoNego = pMediaNego->GetVideoNego();

        if (pVideoNego == IMS_NULL)
        {
            return static_cast<IMS_SINT32>(VIDEO_RESOLUTION_INVALID);
        }

        return static_cast<IMS_SINT32>(pVideoNego->GetNegotiatedResolution());
    }
    else if (MEDIA_IS_CONTAINED_THIS_TYPE(eType, MEDIA_TYPE_TEXT))
    {
        return TEXT_CODEC_BITRATE_DEFAULT;
    }

    return 0;
}

PUBLIC
IMS_SINT32 MediaNegoHandler::GetRemotePort(IMS_UINTP nNegoId, MEDIA_CONTENT_TYPE eType)
{
    MediaNego* pMediaNego = FindMediaNego(nNegoId);

    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetRemotePort(): Can't find NegoId[%" PFLS_x "]", nNegoId, 0, 0);
        return MEDIA_PORT_INVALID;
    }

    switch (eType)
    {
        case MEDIA_TYPE_AUDIO:
        {
            std::shared_ptr<AudioNego> pAudioNego = pMediaNego->GetAudioNego();
            return (pAudioNego) ? pAudioNego->GetRemotePort() : MEDIA_PORT_INVALID;
        }
        case MEDIA_TYPE_VIDEO:
        {
            std::shared_ptr<VideoNego> pVideoNego = pMediaNego->GetVideoNego();
            return (pVideoNego) ? pVideoNego->GetRemotePort() : MEDIA_PORT_INVALID;
        }
        case MEDIA_TYPE_TEXT:
        {
            std::shared_ptr<TextNego> pTextNego = pMediaNego->GetTextNego();
            return (pTextNego) ? pTextNego->GetRemotePort() : MEDIA_PORT_INVALID;
        }
        default:
            return MEDIA_PORT_INVALID;
    }
}

PUBLIC
MEDIA_DIRECTION MediaNegoHandler::GetNegotiatedDirection(
        IMS_UINTP nNegoId, MEDIA_CONTENT_TYPE eType)
{
    MediaNego* pMediaNego = FindMediaNego(nNegoId);

    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetNegotiatedMediaDirection(): Can't find nMediaNegoId[%" PFLS_x "]",
                nNegoId, 0, 0);
        return MEDIA_DIRECTION_INVALID;
    }

    switch (eType)
    {
        case MEDIA_TYPE_AUDIO:
            return pMediaNego->GetNegotiatedAudioDirection();
        case MEDIA_TYPE_VIDEO:
            return pMediaNego->GetNegotiatedVideoDirection();
        case MEDIA_TYPE_TEXT:
            return pMediaNego->GetNegotiatedTextDirection();
        default:
            IMS_TRACE_I("GetNegotiatedDirection(): Invalid media type[%d]", eType, 0, 0);
            return MEDIA_DIRECTION_INVALID;
    }
}

const IpAddress& MediaNegoHandler::GetNegotiatedRemoteAddress(
        IMS_UINTP nNegoId, MEDIA_CONTENT_TYPE eType)
{
    MediaNego* pMediaNego = FindMediaNego(nNegoId);

    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(
                0, "GetNegotiatedRemoteAddress(): NegoId[%" PFLS_x "] not found", nNegoId, 0, 0);
        return IpAddress::NONE;
    }

    switch (eType)
    {
        case MEDIA_TYPE_AUDIO:
        {
            std::shared_ptr<AudioNego> pAudioNego = pMediaNego->GetAudioNego();
            return (pAudioNego != IMS_NULL) ? pAudioNego->GetNegotiatedRemoteAddress()
                                            : IpAddress::NONE;
        }
        case MEDIA_TYPE_VIDEO:
        {
            std::shared_ptr<VideoNego> pVideoNego = pMediaNego->GetVideoNego();
            return (pVideoNego != IMS_NULL) ? pVideoNego->GetNegotiatedRemoteAddress()
                                            : IpAddress::NONE;
        }
        case MEDIA_TYPE_TEXT:
        {
            std::shared_ptr<TextNego> pTextNego = pMediaNego->GetTextNego();
            return (pTextNego != IMS_NULL) ? pTextNego->GetNegotiatedRemoteAddress()
                                           : IpAddress::NONE;
        }
        default:
            IMS_TRACE_E(0, "GetNegotiatedRemoteAddress(): Invalid media type[%d]", eType, 0, 0);
            return IpAddress::NONE;
    }
}

PUBLIC
IMS_BOOL MediaNegoHandler::SetRtpPort(IMS_UINTP nNegoId, MEDIA_CONTENT_TYPE eType, IMS_UINT32 nPort)
{
    MediaNego* pMediaNego = FindMediaNego(nNegoId);

    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "SetRtpPort(): NegoId[%" PFLS_x "] not found", nNegoId, 0, 0);
        return IMS_FALSE;
    }

    IMS_BOOL bResult = IMS_FALSE;

    if (eType & MEDIA_TYPE_AUDIO)
    {
        std::shared_ptr<AudioNego> pAudioNego = pMediaNego->GetAudioNego();
        if (pAudioNego != IMS_NULL)
        {
            bResult &= pAudioNego->SetLocalPort(nPort);
        }
    }

    if (eType & MEDIA_TYPE_VIDEO)
    {
        std::shared_ptr<VideoNego> pVideoNego = pMediaNego->GetVideoNego();
        if (pVideoNego != IMS_NULL)
        {
            bResult &= pVideoNego->SetLocalPort(nPort);
        }
    }

    if (eType & MEDIA_TYPE_TEXT)
    {
        std::shared_ptr<TextNego> pTextNego = pMediaNego->GetTextNego();
        if (pTextNego != IMS_NULL)
        {
            bResult &= pTextNego->SetLocalPort(nPort);
        }
    }

    return bResult;
}

const ImsMap<IMS_UINTP, MediaNego*>& MediaNegoHandler::GetMediaNegoMap() const
{
    return m_objMapMediaNego;
}
