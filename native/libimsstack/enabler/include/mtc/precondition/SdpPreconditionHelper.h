#ifndef SDP_PRECONDITION_HELPER_H_
#define SDP_PRECONDITION_HELPER_H_

#include "ISipMessage.h"

#include "ISession.h"
#include "media/IMediaDescriptor.h"
#include "offeranswer/SdpSegmentedPrecondition.h"
#include "precondition/QosStatusTable.h"

class SdpPreconditionHelper
{
public:
    static void FormPreconditionSdp(
            IN ISession* piSession, IN QosStatusTable* pStatusTable, IN IMS_BOOL bUseConf);
    static void RemovePreconditionSdp(IN ISession* piSession);
    static void FormFailurePreconditionSdp(IN ISession* piSession);

    static IMS_UINT32 GetMediaType(IN CONST SdpMedia* pSdpMedia, IN IMS_SINT32 nMediaState);

    /* Parsing SDP Utility */
    static IMS_UINT32 GetMediaTypesBySdp(IN ISession* piSession);
    static IMS_BOOL IsPreconditionIncludedInSdp(IN ISession* piSession);
    static IMS_BOOL IsLocalResourceReservedInSdp(
            IN ISession* piSession, IN IMS_SINT32 nServiceMethod);

private:
    static void FormCurrentAttribute(
            IN IMediaDescriptor* piMediaDescriptor, IN QosStatusTable* pStatusTable);
    static void FormDesiredAttribute(
            IN IMediaDescriptor* piMediaDescriptor, IN QosStatusTable* pStatusTable);
    static void AddDesiredStatus(OUT SdpSegmentedPrecondition* objDesired,
            IN QosStatusTable* pStatusTable, IN IMS_SINT32 eSdpMediaType,
            IN IMS_SINT32 eStatusType);
    static void FormConfirmAttribute(IN IMediaDescriptor* piMediaDescriptor,
            IN QosStatusTable* pStatusTable, IN IMS_BOOL bUseConf);

    static IMediaDescriptor* GetMediaDescriptor(IN IMedia* piMedia);
    static IMS_BOOL HasReservedResourceInSdp(
            IN ISipMessage* piSipMessage, IN IMS_SINT32 eSdpMediaType);
};
#endif
