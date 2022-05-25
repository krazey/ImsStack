#ifndef MTC_MEDIA_UTIL_H_
#define MTC_MEDIA_UTIL_H_

#include "IMSTypeDef.h"
#include "IMessage.h"
#include "MediaDef.h"
#include "IMtcService.h"
#include "MtcDef.h"
#include "MediaNego.h"
#include "call/IMtcCall.h"

class MtcMediaUtil
{
public:
    static IMS_SINT32 GetMediaDirectionFromSdp();
    static IMS_UINT32 GetMediaTypesFromSdp();
    static IMS_BOOL IsMediaPortValid();

    static CallType GetCallTypeFromMediaTypes(IN IMS_UINT32 eMediaTypes);
    static CallType GetCallTypeFromMediaContents(IN MEDIA_CONTENT_TYPE eMediaContents);

    static IMS_UINT32 GetMediaTypesFromCallType(IN CallType eCallType);
    static IMS_UINT32 GetMediaTypesFromMediaContents(IN MEDIA_CONTENT_TYPE eMediaContents);

    static MEDIA_CONTENT_TYPE GetMediaContentsFromMediaTypes(IN IMS_UINT32 eMediaTypes);
    static MEDIA_CONTENT_TYPE GetMediaContentsFromCallType(IN CallType eCallType);

    static MEDIA_SERVICE_TYPE GetMediaServiceType(IN ServiceType eServiceType);
    static MEDIA_NETWORK_TYPE GetMediaNetworkType(
            IN IMtcService* piMtcService, IN IMS_SINT32 nSlotId);
    static IMS_SINT32 GetFailReasonFromReportType(IN IMS_UINT32 eReportType);
    static IMS_SINT32 GetGttModeFromTextQuality(IN IMS_UINT32 eTextQuality);
};

#endif
