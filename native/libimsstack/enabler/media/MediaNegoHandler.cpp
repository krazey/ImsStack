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
#include "IMediaNegoFactory.h"
#include "ImsTypeDef.h"
#include "ConcreteMediaNegoFactory.h"
#include "MediaEnvironment.h"
#include "ServiceTrace.h"
#include "audio/AudioNego.h"
#include "text/TextNego.h"
#include "video/VideoNego.h"

__IMS_TRACE_TAG_MEDIA__;

static IMS_UINTP g_nNextNegoId = 0;

PUBLIC
MediaNegoHandler::MediaNegoHandler(IMS_UINT32 nSlotId,
        std::shared_ptr<MediaEnvironment> pEnvironment,
        std::shared_ptr<IMediaNegoFactory> pFactory) :
        m_nSlotId(nSlotId),
        m_pEnvironment(pEnvironment),
        m_pMediaNegoFactory(
                pFactory != IMS_NULL ? pFactory : std::make_shared<ConcreteMediaNegoFactory>())
{
    IMS_TRACE_I("+MediaNegoHandler(): SlotId[%u]", m_nSlotId, 0, 0);
}

PUBLIC
MediaNegoHandler::~MediaNegoHandler()
{
    IMS_TRACE_I("~MediaNegoHandler(): SlotId[%u]", m_nSlotId, 0, 0);
}

PUBLIC
IMS_UINTP MediaNegoHandler::CreateMediaNego(IMS_UINTP nNegoId)
{
    IMS_TRACE_I("CreateMediaNego(): NegoId[%" PFLS_x "]", nNegoId, 0, 0);

    std::shared_ptr<MediaNego> pNewMediaNego = nullptr;

    if (nNegoId != 0)
    {
        std::shared_ptr<MediaNego> pExistingMediaNego = FindMediaNego(nNegoId);

        if (pExistingMediaNego == IMS_NULL)
        {
            IMS_TRACE_E(0, "CreateMediaNego(): Invalid NegoId[%" PFLS_x "]", nNegoId, 0, 0);
            return 0;
        }

        pNewMediaNego = m_pMediaNegoFactory->CreateForkedMediaNego(
                m_nSlotId, pExistingMediaNego, m_pEnvironment);
        if (!pNewMediaNego)
        {
            IMS_TRACE_E(0, "CreateMediaNego(): Factory failed to create forked nego.", 0, 0, 0);
            return 0;
        }
    }
    else
    {
        pNewMediaNego = m_pMediaNegoFactory->CreateMediaNego(m_nSlotId);

        if (!pNewMediaNego)
        {
            IMS_TRACE_E(0, "CreateMediaNego(): Factory failed to create new nego.", 0, 0, 0);
            return 0;
        }

        if (!pNewMediaNego->CreateProfile(m_pEnvironment))
        {
            IMS_TRACE_E(0, "CreateMediaNego(): Failed to create profile for new nego.", 0, 0, 0);
            return 0;
        }
    }

    IMS_UINTP nNewNegoId = GenerateNewNegoId();
    m_objMapMediaNego.Add(nNewNegoId, pNewMediaNego);

    IMS_TRACE_I("CreateMediaNego(): Created NegoId[%" PFLS_x "], MapSize[%d]", nNewNegoId,
            m_objMapMediaNego.GetSize(), 0);
    return nNewNegoId;
}

PUBLIC
std::shared_ptr<MediaNego> MediaNegoHandler::FindMediaNego(IMS_UINTP nNegoId)
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

    if (m_objMapMediaNego.GetSize() == 1)
    {
        IMS_TRACE_E(0, "DeleteMediaNego() - NegoId[%" PFLS_x "], cannot delete last one", nNegoId,
                0, 0);
        return IMS_FALSE;
    }

    m_objMapMediaNego.RemoveAt(nIndex);
    return IMS_TRUE;
}

PUBLIC
void MediaNegoHandler::ClearAllMediaNego()
{
    IMS_TRACE_D("ClearMediaNego(): size[%d]", m_objMapMediaNego.GetSize(), 0, 0);

    while (!m_objMapMediaNego.IsEmpty())
    {
        m_objMapMediaNego.RemoveAt(0);
    }
}

PUBLIC
IMS_BOOL MediaNegoHandler::FormSdp(IMS_UINTP nNegoId, OUT ISession* pSession,
        MEDIA_CONTENT_TYPE eType, MEDIA_DIRECTION eAudioDirection, MEDIA_DIRECTION eVideoDirection,
        MEDIA_DIRECTION eTextDirection, IMS_BOOL bEnforceReofferMode)
{
    std::shared_ptr<MediaNego> pMediaNego = FindMediaNego(nNegoId);

    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "FormSdp(): NegoId[%" PFLS_x "] not found", nNegoId, 0, 0);
        return IMS_FALSE;
    }

    return pMediaNego->FormSdp(
            pSession, eType, eAudioDirection, eVideoDirection, eTextDirection, bEnforceReofferMode);
}

PUBLIC
MEDIA_CONTENT_TYPE MediaNegoHandler::GetSupportedMediaTypesFromSdp(
        IMS_UINTP nNegoId, IN ISession* pSession)
{
    std::shared_ptr<MediaNego> pMediaNego = FindMediaNego(nNegoId);

    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(
                0, "GetSupportedMediaTypesFromSdp(): NegoId[%" PFLS_x "] not found", nNegoId, 0, 0);
        return MEDIA_TYPE_INVALID;
    }

    return pMediaNego->GetSupportedMediaTypesFromSdp(pSession);
}

PUBLIC
SdpNegotiationResult MediaNegoHandler::NegotiateSdp(IMS_UINTP nNegoId, IN ISession* pSession)
{
    std::shared_ptr<MediaNego> pMediaNego = FindMediaNego(nNegoId);
    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateSdp(): NegoId[%" PFLS_x "] not found", nNegoId, 0, 0);
        return SdpNegotiationResult(MEDIA_NEGO_ERROR_INVALID_DESCRIPTOR);
    }

    return pMediaNego->NegotiateSdp(pSession);
}

PUBLIC
void MediaNegoHandler::FinalizeSdp(IMS_UINTP nNegoId, IN ISession* pSession)
{
    std::shared_ptr<MediaNego> pMediaNego = FindMediaNego(nNegoId);

    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "FinalizeSdp(): NegoId[%" PFLS_x "] not found", nNegoId, 0, 0);
        return;
    }

    pMediaNego->FinalizeSdp(pSession);
}

PUBLIC
void MediaNegoHandler::FinalizeNegotiation(IMS_UINTP nNegoId)
{
    std::shared_ptr<MediaNego> pMediaNego = FindMediaNego(nNegoId);

    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "FinalizeNegotiation(): NegoId[%" PFLS_x "] not found", nNegoId, 0, 0);
        return;
    }

    pMediaNego->FinalizeNegotiation();
}

PUBLIC
NEGO_STATE MediaNegoHandler::GetNegoState(IMS_UINTP nNegoId)
{
    std::shared_ptr<MediaNego> pMediaNego = FindMediaNego(nNegoId);

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
    std::shared_ptr<MediaNego> pMediaNego = FindMediaNego(nNegoId);
    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetNegotiatedMediaType(): NegoId[%" PFLS_x "] not found", nNegoId, 0, 0);
        return MEDIA_TYPE_INVALID;
    }

    // Re-implement the logic here as it depends on multiple calls to MediaNego
    IMS_SINT32 eMediaMask = MEDIA_TYPE_INVALID;

    if (pMediaNego->GetNegotiatedAudioQuality() != AUDIO_CODEC_NOT_USED)
    {
        eMediaMask |= MEDIA_TYPE_AUDIO;
    }

    if (pMediaNego->GetNegotiatedVideoQuality() != VIDEO_RESOLUTION_NOT_USED)
    {
        eMediaMask |= MEDIA_TYPE_VIDEO;
    }

    if (pMediaNego->GetNegotiatedTextQuality() != TEXT_CODEC_NOT_USED)
    {
        eMediaMask |= MEDIA_TYPE_TEXT;
    }

    return static_cast<MEDIA_CONTENT_TYPE>(eMediaMask);
}

PUBLIC
IMS_SINT32 MediaNegoHandler::GetNegotiatedQuality(IMS_UINTP nNegoId, MEDIA_CONTENT_TYPE eType)
{
    std::shared_ptr<MediaNego> pMediaNego = FindMediaNego(nNegoId);
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
    std::shared_ptr<MediaNego> pMediaNego = FindMediaNego(nNegoId);
    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetNegotiatedCodecBitrate(): NegoId[%" PFLS_x "] not found", nNegoId, 0, 0);
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
IMS_FLOAT MediaNegoHandler::GetNegotiatedCodecBitrateKbps(
        IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE eMediaType)
{
    std::shared_ptr<MediaNego> pMediaNego = FindMediaNego(nNegoId);
    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(
                0, "GetNegotiatedCodecBitrateKbps(): NegoId[%" PFLS_x "] not found", nNegoId, 0, 0);
        return 0.0f;
    }

    return pMediaNego->GetNegotiatedCodecBitrateKbps(eMediaType);
}

PUBLIC
IMS_FLOAT MediaNegoHandler::GetNegotiatedCodecBandwidthKhz(
        IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE eMediaType)
{
    std::shared_ptr<MediaNego> pMediaNego = FindMediaNego(nNegoId);
    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetNegotiatedCodecBandwidthKhz(): NegoId[%" PFLS_x "] not found", nNegoId,
                0, 0);
        return 0.0f;
    }

    return pMediaNego->GetNegotiatedCodecBandwidthKhz(eMediaType);
}

PUBLIC
void MediaNegoHandler::GetNegotiatedCodecBitrateRange(IN IMS_UINTP nNegoId,
        IN MEDIA_CONTENT_TYPE eMediaType, OUT IMS_FLOAT& nBitrateStart, OUT IMS_FLOAT& nBitrateEnd)
{
    std::shared_ptr<MediaNego> pMediaNego = FindMediaNego(nNegoId);
    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetNegotiatedCodecBitrateRange(): NegoId[%" PFLS_x "] not found", nNegoId,
                0, 0);
        nBitrateStart = 0.0f;
        nBitrateEnd = 0.0f;
        return;
    }
    pMediaNego->GetNegotiatedCodecBitrateRange(eMediaType, nBitrateStart, nBitrateEnd);
}

PUBLIC
void MediaNegoHandler::GetNegotiatedCodecBandwidthRange(IN IMS_UINTP nNegoId,
        IN MEDIA_CONTENT_TYPE eMediaType, OUT IMS_FLOAT& nBandwidthStart,
        OUT IMS_FLOAT& nBandwidthEnd)
{
    std::shared_ptr<MediaNego> pMediaNego = FindMediaNego(nNegoId);
    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetNegotiatedCodecBandwidthRange(): NegoId[%" PFLS_x "] not found", nNegoId,
                0, 0);
        nBandwidthStart = 0.0f;
        nBandwidthEnd = 0.0f;
        return;
    }
    pMediaNego->GetNegotiatedCodecBandwidthRange(eMediaType, nBandwidthStart, nBandwidthEnd);
}

PUBLIC
IMS_SINT32 MediaNegoHandler::GetRemotePort(IMS_UINTP nNegoId, MEDIA_CONTENT_TYPE eType)
{
    std::shared_ptr<MediaNego> pMediaNego = FindMediaNego(nNegoId);

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
    std::shared_ptr<MediaNego> pMediaNego = FindMediaNego(nNegoId);

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
    std::shared_ptr<MediaNego> pMediaNego = FindMediaNego(nNegoId);

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
    std::shared_ptr<MediaNego> pMediaNego = FindMediaNego(nNegoId);

    if (pMediaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "SetRtpPort(): NegoId[%" PFLS_x "] not found", nNegoId, 0, 0);
        return IMS_FALSE;
    }

    IMS_BOOL bResult = IMS_TRUE;

    if (eType & MEDIA_TYPE_AUDIO)
    {
        std::shared_ptr<AudioNego> pAudioNego = pMediaNego->GetAudioNego();
        bResult &= pAudioNego != IMS_NULL ? pAudioNego->SetLocalPort(nPort) : IMS_FALSE;
    }

    if (eType & MEDIA_TYPE_VIDEO)
    {
        std::shared_ptr<VideoNego> pVideoNego = pMediaNego->GetVideoNego();
        bResult &= pVideoNego != IMS_NULL ? pVideoNego->SetLocalPort(nPort) : IMS_FALSE;
    }

    if (eType & MEDIA_TYPE_TEXT)
    {
        std::shared_ptr<TextNego> pTextNego = pMediaNego->GetTextNego();
        bResult &= pTextNego != IMS_NULL ? pTextNego->SetLocalPort(nPort) : IMS_FALSE;
    }

    return bResult;
}

PRIVATE
IMS_UINTP MediaNegoHandler::GenerateNewNegoId()
{
    return ++g_nNextNegoId;
}
