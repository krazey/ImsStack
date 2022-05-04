#define IMS_STL_USE

#include "IuMtcService.h"
#include "JniMtcCall.h"
#include "JniMtcServiceThread.h"
#include "JniMtcUtils.h"
#include "ServiceTrace.h"
#include "call/ParticipantInfo.h"
#include <binder/Parcel.h>

using namespace android;

__IMS_TRACE_TAG_USER_DECL__("JNI.MTC");

PUBLIC
JniMtcServiceThread::JniMtcServiceThread() :
        BaseServiceThread(),
        m_nSlotId(0)
{
    IMS_TRACE_D("+JniMtcServiceThread", 0, 0, 0);
}

PUBLIC VIRTUAL JniMtcServiceThread::~JniMtcServiceThread()
{
    IMS_TRACE_D("~JniMtcServiceThread", 0, 0, 0);
}

PUBLIC
void JniMtcServiceThread::OnServiceChanged(IN IMS_SINT32 eStatus, IN IMS_SINT32 eReason)
{
    IMS_TRACE_D("OnServiceChanged [%d]", eStatus, 0, 0);
    Parcel objParcel;
    objParcel.writeInt32(IuMtcService::SERVICE_CHANGED);
    objParcel.writeInt32(eStatus);
    objParcel.writeInt32(eReason);

    SendData2Java(objParcel);
}

PUBLIC
void JniMtcServiceThread::OnIncomingCallReceived(IN IMS_UINTP nCallKey, IN CallInfo* pCallInfo,
        IN MediaInfo* pMediaInfo, IN const IMSMap<SuppType, SuppService*>& objSuppServices,
        IN ParticipantInfo* pParticipantInfo)
{
    IMS_TRACE_D("OnIncomingCallReceived", 0, 0, 0);
    Parcel objParcel;
    objParcel.writeInt32(IuMtcService::INCOMING_CALL);
    objParcel.writeInt32(nCallKey);

    // this object will be released by IMSInterface
    JniMtcCall* pMtcCall = new JniMtcCall(pfnNotifier, static_cast<IMS_SINTP>(nCallKey), m_nSlotId);

    objParcel.writeInt64(reinterpret_cast<IMS_SINTP>(pMtcCall));

    JniMtcUtils::WriteCallInfoToParcel(pCallInfo, objParcel);
    JniMtcUtils::WriteMediaInfoToParcel(pMediaInfo, objParcel);

    /* Display Info  // TODO: ParticipantInfo to be included in CallInfo? */
    // Need to set callee number when the "child number" is used.
    objParcel.writeInt32(static_cast<IMS_SINT32>(pParticipantInfo->GetOipType()));
    objParcel.writeString16(android::String16(AString::ConstNull().GetStr()));
    objParcel.writeString16(android::String16(pParticipantInfo->GetRemoteNumber().GetStr()));

    /* Supp Info */
    JniMtcUtils::WriteSuppServicesToParcel(objSuppServices, objParcel);

    objParcel.writeString16(android::String16(AString("MTCLOG").GetStr()));  // TODO: Log.

    SendData2Java(objParcel);
}
