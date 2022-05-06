/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090323  neties@                   Initial Create
    20091208  toastops@                 Revisioned by Bruce for VSC service
    </table>

    Description
     IMediaDescriptor is an interface towards the media parts of the SDP.
    The interface is most useful for applications that, within the realm of a composed capability
    (ICSI, IARI and/or other service level feature tags), need to manipulate media-related fields
    of the session SDP in general, and add new application-specific attributes in particular.

    NOTE:
    The getters of the interface read fields from the last received SDP. The setters modify
    all remaining SDPs to be sent during the lifetime of the media in the session,
    or until superseded by other modifications.

    RESERVED ATTRIBUTES :
    des, curr, conf, mid, ice-pwd, ice-ufrag, candidate, remote-candidates, sendonly, recvonly,
    sendrecv, inactive, csup, creq, acap, tcap, pcfg, acfg, 3gpp_sync_info

    - RTP/AVP (StreamMedia) : rtpmap, fmtp, dccp-service-code, rtcp-mux, rtp-fb, ptime,
        maxptime, framesize, framerate, quality
    - TCP (BasicReliableMedia) : setup, connection
    - TCP/MSRP (FramedMedia) : setup, connection, accept-types, accept-wrapped-types,
        max-size, path
*/

#ifndef _MEDIA_DESCRIPTOR_H_
#define _MEDIA_DESCRIPTOR_H_

#include "media/IMediaDescriptor.h"

class IMediaState;
class SdpMediaParameter;

class MediaDescriptor : public IMediaDescriptor
{
public:
    MediaDescriptor(IN IMediaState* piMediaState_, IN IMS_SINT32 nMid_);
    virtual ~MediaDescriptor();

private:
    MediaDescriptor(IN CONST MediaDescriptor& objRHS);
    MediaDescriptor& operator=(IN CONST MediaDescriptor& objRHS);

public:
    IMS_SINT32 GetMid() const;
    void SetMid(IN IMS_SINT32 nMid);

private:
    // IMediaDescriptor interface
    virtual IMS_RESULT AddAttribute(IN CONST AString& strAttribute);
    virtual IMSList<AString> GetAttributes() const;
    virtual IMSList<AString> GetBandwidthInfo() const;
    virtual AString GetMediaDescription() const;
    virtual AString GetMediaTitle() const;
    virtual IMS_RESULT RemoveAttribute(IN CONST AString& strAttribute);
    virtual IMS_RESULT SetBandwidthInfo(IN CONST IMSList<AString>& strBandwidthInfos);
    virtual IMS_RESULT SetMediaTitle(IN CONST AString& strTitle);
    //// IMS extensions
    virtual IMS_RESULT AddAttribute(IN IMS_SINT32 nType, IN CONST AString& strAttrValue,
            IN CONST AString& strType = AString::ConstNull());
    virtual IMS_RESULT AddAttributeInt(IN IMS_SINT32 nType, IN IMS_SINT32 nAttrValue,
            IN CONST AString& strType = AString::ConstNull());
    virtual IMS_RESULT AddBandwidth(IN IMS_SINT32 nType, IN IMS_SINT32 nBandwidth,
            IN CONST AString& strType = AString::ConstNull());
    virtual const AString& GetAttribute(
            IN IMS_SINT32 nType, IN CONST AString& strType = AString::ConstNull()) const;
    virtual IMSList<AString> GetAttributes(
            IN IMS_SINT32 nType, IN CONST AString& strType = AString::ConstNull()) const;
    virtual IMS_SINT32 GetAttributeInt(
            IN IMS_SINT32 nType, IN CONST AString& strType = AString::ConstNull()) const;
    virtual IMS_SINT32 GetBandwidth(
            IN IMS_SINT32 nType, IN CONST AString& strType = AString::ConstNull()) const;
    virtual IMS_SINT32 GetDirection() const;
    virtual const SdpMedia* GetMediaDescriptionEx() const;
    virtual const IMSList<SdpMediaFormat*>& GetMediaFormats() const;
    virtual IMS_RESULT RemoveAttribute(IN CONST SdpAttribute& objAttribute);
    virtual IMS_RESULT RemoveAttribute(IN IMS_SINT32 nType,
            IN CONST AString& strAttrValue = AString::ConstNull(),
            IN CONST AString& strType = AString::ConstNull());
    virtual IMS_RESULT RemoveMediaFormat(IN IMS_SINT32 nType, IN CONST AString& strValue);
    virtual IMS_RESULT SetConnectionAddress(IN CONST AString& strAddress);
    virtual IMS_RESULT SetDirection(IN IMS_SINT32 nDirection);
    virtual IMS_RESULT SetMediaDescription(IN IMS_SINT32 nType, IN IMS_SINT32 nPort,
            IN IMS_SINT32 nTransportProtocol, IN CONST AStringArray& objFormats);
    virtual IMS_RESULT SetMediaFormat(IN CONST SdpMediaFormat* pMediaFormat);
    virtual IMS_RESULT SetMediaFormat(IN IMS_SINT32 nType, IN CONST AString& strValue,
            IN CONST AString& strAnyMAP, IN CONST AString& strFMTP);
    virtual IMS_RESULT SetPort(IN IMS_SINT32 nPort);

    virtual const SdpMedia* GetMediaDescriptionExAsLocal() const;
    virtual IPAddress GetLocalAddress() const;
    virtual IMS_SINT32 GetLocalPort() const;
    virtual IPAddress GetRemoteAddress() const;
    virtual const AString& GetRemoteAddressAsString() const;
    virtual IMS_SINT32 GetRemotePort() const;

    // IMS_SDP_PRECONDITION
    virtual const SdpPrecondition* GetPrecondition(
            IN IMS_SINT32 nAttribute, IN IMS_SINT32 nType = SdpPrecondition::TYPE_QOS) const;
    virtual IMS_RESULT RemovePrecondition(
            IN IMS_SINT32 nAttribute, IN IMS_SINT32 nType = SdpPrecondition::TYPE_QOS);
    virtual IMS_RESULT SetPrecondition(
            IN IMS_SINT32 nAttribute, IN CONST SdpPrecondition* pPrecondition);

    static IMS_BOOL IsAttributeReserved(
            IN CONST SdpMediaParameter* pMediaParam, IN CONST AString& strAttribute);

private:
    // Maximum number of reserved attributes
    enum
    {
        MAX_RESERVED_ATTRIBUTE = 37
    };

    // Index of each media type
    enum
    {
        START_COMMON = 0,
        END_COMMON = 18,

        START_STREAM_MEDIA = 19,
        END_STREAM_MEDIA = 28,

        START_FRAMED_MEDIA = 29,
        END_FRAMED_MEDIA = 34,

        START_BASIC_RELIABLE_MEDIA = 35,
        END_BASIC_RELIABLE_MEDIA = (MAX_RESERVED_ATTRIBUTE - 1)
    };

    static const IMS_CHAR* RESERVED_ATTRIBUTE[MAX_RESERVED_ATTRIBUTE];

    IMediaState* piMediaState;
    IMS_SINT32 nMid;
};

#endif  // _MEDIA_DESCRIPTOR_H_
