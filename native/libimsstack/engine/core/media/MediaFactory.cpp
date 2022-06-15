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
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "ImsCore.h"
#include "SdpMedia.h"
#include "base/Ims.h"
#include "media/FramedMedia.h"
#include "media/MediaFactory.h"
#include "media/StreamMedia.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC GLOBAL Media* MediaFactory::CreateOutgoingMedia(IN const AString& strMType,
        IN IMS_SINT32 nDirection, IN Service* pService, IN ISdpOaState* piOaState,
        IN IMS_SINT32 nCountOfDescriptor)
{
    Media* pMedia = IMS_NULL;

    if (strMType.Equals(ImsCore::MEDIA_STREAM))
    {
        pMedia = new StreamMedia(pService, piOaState);
    }
    else if (strMType.Equals(ImsCore::MEDIA_FRAMED))
    {
        pMedia = new FramedMedia(pService, piOaState);
    }
    else if (strMType.Equals(ImsCore::MEDIA_BASIC_RELIABLE))
    {
    }
    else if (strMType.Equals(ImsCore::MEDIA_BASIC_UNRELIABLE))
    {
    }
    else
    {
        IMS_TRACE_E(0, "Trying to create an unsupported media (%s)", strMType.GetStr(), 0, 0);

        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    if (pMedia == IMS_NULL)
    {
        IMS_TRACE_E(0, "Allocating a Media (%s) failed", strMType.GetStr(), 0, 0);
        return IMS_NULL;
    }

    if (!pMedia->InitInstance(nCountOfDescriptor, nDirection))
    {
        IMS_TRACE_E(0, "Creating a media (%s) failed", strMType.GetStr(), 0, 0);

        delete pMedia;
        return IMS_NULL;
    }

    return pMedia;
}

PUBLIC GLOBAL Media* MediaFactory::CreateIncomingMedia(IN IMS_SINT32 nTransportProtocol,
        IN Service* pService, IN ISdpOaState* piOaState, IN const IMSList<IMS_SINT32>& objMids)
{
    const IMS_CHAR* pszMType = IMS_NULL;
    Media* pMedia = IMS_NULL;

    switch (nTransportProtocol)
    {
        case SdpMedia::TRANSPORT_RTP_AVP:    // FALL-THROUGH
        case SdpMedia::TRANSPORT_RTP_AVPF:   // FALL-THROUGH
        case SdpMedia::TRANSPORT_RTP_SAVP:   // FALL-THROUGH
        case SdpMedia::TRANSPORT_RTP_SAVPF:  // FALL-THROUGH
        case SdpMedia::TRANSPORT_UDP_TLS_RTP_SAVP:
            pszMType = ImsCore::MEDIA_STREAM;
            pMedia = new StreamMedia(pService, piOaState);
            break;
        case SdpMedia::TRANSPORT_TCP_MSRP:  // FALL-THROUGH
        case SdpMedia::TRANSPORT_TCP_TLS_MSRP:
            pszMType = ImsCore::MEDIA_FRAMED;
            pMedia = new FramedMedia(pService, piOaState);
            break;
        case SdpMedia::TRANSPORT_UDP:
            pszMType = ImsCore::MEDIA_BASIC_UNRELIABLE;
            break;
        case SdpMedia::TRANSPORT_TCP:
            pszMType = ImsCore::MEDIA_BASIC_RELIABLE;
            break;
        default:
            IMS_TRACE_E(0, "Trying to create an unsupported media (%d)", nTransportProtocol, 0, 0);
            return IMS_NULL;
    }

    if (pMedia == IMS_NULL)
    {
        IMS_TRACE_E(0, "Allocating a Media (%s) failed", pszMType, 0, 0);
        return IMS_NULL;
    }

    if (!pMedia->InitInstance(objMids))
    {
        IMS_TRACE_E(0, "Creating a media (%s) failed", pszMType, 0, 0);

        delete pMedia;
        return IMS_NULL;
    }

    return pMedia;
}

PUBLIC GLOBAL void MediaFactory::DestroyMedia(IN Media*& pMedia)
{
    if (pMedia == IMS_NULL)
    {
        return;
    }

    delete pMedia;
    pMedia = IMS_NULL;
}
