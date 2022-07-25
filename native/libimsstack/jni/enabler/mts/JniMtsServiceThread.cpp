#define IMS_STL_USE

#include "JniMtsServiceThread.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "IuMtsService.h"
#include <utils/String8.h>

using namespace android;

__IMS_TRACE_TAG_USER_DECL__("JNI.MTS");

PUBLIC
JniMtsServiceThread::JniMtsServiceThread() :
        BaseServiceThread()
{
    IMS_TRACE_I("JniMtsServiceThread : ", 0, 0, 0);
}

PUBLIC VIRTUAL JniMtsServiceThread::~JniMtsServiceThread()
{
    IMS_TRACE_I("~JniMtsServiceThread : ", 0, 0, 0);
}

PUBLIC
void JniMtsServiceThread::ReportMoStatus(IN IMS_UINT32 nReason, IN SmsFormatType eSmsFormat,
        IN IMS_UINT8 nRetryAfter, IN IMS_SINT32 nSeqId, IN IMS_SINT32 nSlotId)
{
    IMS_TRACE_D("ReportMoStatus", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IuMtsService::REPORT_MTS_MO_STATUS);
    objParcel.writeInt32(nReason);
    objParcel.writeInt32(ConvertSmsFormatToInt(eSmsFormat));
    objParcel.writeInt32(nRetryAfter);
    objParcel.writeInt32(nSeqId);
    objParcel.writeInt32(nSlotId);

    SendData2Java(objParcel);
}

PUBLIC
void JniMtsServiceThread::ReportMtSms(
        IN SmsFormatType eSmsFormat, IN const ByteArray& objData, IN IMS_SINT32 nSlotId)
{
    IMS_TRACE_D("ReportMtSms", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IuMtsService::REPORT_MTS_MT_SMS);
    objParcel.writeInt32(ConvertSmsFormatToInt(eSmsFormat));
    objParcel.writeString16(android::String16(objData.ToString().GetStr()));
    objParcel.writeInt32(nSlotId);

    SendData2Java(objParcel);
}

PRIVATE IMS_UINT32 JniMtsServiceThread::ConvertSmsFormatToInt(IN SmsFormatType eSmsFormat)
{
    switch (eSmsFormat)
    {
        case SmsFormatType::SMSFORMAT_3GPP:
            return (IMS_UINT32)1;

        case SmsFormatType::SMSFORMAT_3GPP2:
            return (IMS_UINT32)2;

        default:
            return (IMS_UINT32)3;
    }
}
