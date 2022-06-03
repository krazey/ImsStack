#ifndef MTC_LOCATION_OBJECT_H_
#define MTC_LOCATION_OBJECT_H_

#include "ByteArray.h"
#include "IMSTypeDef.h"
#include "call/IMtcCall.h"

class IMessage;
class IMtcCallContext;
class ISubscriberConfig;

class MtcLocationObject final
{
public:
    explicit MtcLocationObject(IN IMtcCallContext& objContext);
    ~MtcLocationObject();

public:
    IMS_BOOL IsGeolocationInfoRequired();

    void SetLocationToMessage(
            IN_OUT IMessage& objMessage, IN IMS_BOOL bGeolocationRouting = IMS_FALSE);

private:
    static const IMS_CHAR CONTENT_TYPE_PIDF_XML[];
    static const IMS_CHAR HEADER_GEOLOCATION[];
    static const IMS_CHAR HEADER_GEOLOCATION_ROUTING[];
    static const IMS_CHAR GEOLOCATION_ROUTING_NO[];
    static const IMS_CHAR GEOLOCATION_ROUTING_YES[];
    static const IMS_CHAR CONTENT_DISPOSITION_RENDER[];
    static const IMS_CHAR CONTENT_DISPOSITION_HANDLING_OPTIONAL[];

    AString CreateCid(IN const ISubscriberConfig& objSubscriberConfig) const;
    ByteArray CreateLocationBody() const;

    IMS_SINT32 GetGeolocationPidfAllowedType(IN const CallInfo& objCallInfo) const;
    IMS_SINT32 GetInformationLevel() const;
    AString GetGeolocationHeader(IN const AString& strCid) const;
    AString GetContentLengthHeader(IN const ByteArray& objContent) const;
    AString GetContentIdHeader(IN const AString& strCid) const;
    AString GetContentDispositionHeader() const;

    IMtcCallContext& m_objContext;
};

#endif
