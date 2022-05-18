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
void JniMtcServiceThread::OnPreIncomingCallReceived(IN IMS_ULONG nCallKey)
{
    IMS_TRACE_D("OnPreIncomingCallReceived", 0, 0, 0);
    Parcel objParcel;
    objParcel.writeInt32(IuMtcService::PRE_INCOMING_CALL);
    objParcel.writeInt64(nCallKey);

    objParcel.writeString16(android::String16(AString("MTCLOG").GetStr())); // TODO: Log.

    SendData2Java(objParcel);
}

