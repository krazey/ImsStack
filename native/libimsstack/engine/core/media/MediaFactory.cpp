/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091208  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ImsCore.h"
#include "base/Ims.h"
#include "SdpMedia.h"
#include "media/StreamMedia.h"
#include "media/FramedMedia.h"
#include "media/MediaFactory.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC GLOBAL Media* MediaFactory::CreateOutgoingMedia(IN CONST AString& strMType,
        IN IMS_SINT32 nDirection, IN Service* pService, IN ISdpOaState* piOAState,
        IN IMS_SINT32 nCountOfDescriptor)
{
    Media* pMedia = IMS_NULL;

    //---------------------------------------------------------------------------------------------

    if (strMType.Equals(ImsCore::MEDIA_STREAM))
    {
        pMedia = new StreamMedia(pService, piOAState);
    }
    else if (strMType.Equals(ImsCore::MEDIA_FRAMED))
    {
        pMedia = new FramedMedia(pService, piOAState);
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
        IN Service* pService, IN ISdpOaState* piOAState, IN CONST IMSList<IMS_SINT32>& objMids)
{
    const IMS_CHAR* pszMType = IMS_NULL;
    Media* pMedia = IMS_NULL;

    //---------------------------------------------------------------------------------------------

    switch (nTransportProtocol)
    {
        case SdpMedia::TRANSPORT_RTP_AVP:
        case SdpMedia::TRANSPORT_RTP_AVPF:
        case SdpMedia::TRANSPORT_RTP_SAVP:
        case SdpMedia::TRANSPORT_RTP_SAVPF:
        case SdpMedia::TRANSPORT_UDP_TLS_RTP_SAVP:
            pszMType = ImsCore::MEDIA_STREAM;
            pMedia = new StreamMedia(pService, piOAState);
            break;

        case SdpMedia::TRANSPORT_TCP_MSRP:
        case SdpMedia::TRANSPORT_TCP_TLS_MSRP:
            pszMType = ImsCore::MEDIA_FRAMED;
            pMedia = new FramedMedia(pService, piOAState);
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
    //---------------------------------------------------------------------------------------------

    if (pMedia == IMS_NULL)
        return;

    delete pMedia;
    pMedia = IMS_NULL;
}
