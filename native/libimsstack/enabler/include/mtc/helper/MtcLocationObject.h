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
    static IMS_BOOL IsGeolocationInfoRequired(IN IMtcCallContext& objContext);

    void SetLocationToMessage(
            IN_OUT IMessage& objMessage, IN IMS_BOOL bGeolocationRouting = IMS_FALSE);

private:
    AString CreateCid(IN const ISubscriberConfig& objSubscriberConfig) const;
    ByteArray CreateLocationBody() const;

    IMS_SINT32 GetInformationLevel() const;
    AString GetGeolocationHeader(IN const AString& strCid) const;
    AString GetContentLengthHeader(IN const ByteArray& objContent) const;
    AString GetContentIdHeader(IN const AString& strCid) const;
    AString GetContentDispositionHeader() const;

    IMtcCallContext& m_objContext;
};

#endif
