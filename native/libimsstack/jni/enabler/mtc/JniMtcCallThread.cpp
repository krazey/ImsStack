#define IMS_STL_USE

#include "IuMtcCall.h"
#include "JniMtcCallThread.h"
#include "JniMtcUtils.h"
#include "ServiceTrace.h"
#include <binder/Parcel.h>
#include <utils/String8.h>

using namespace android;

__IMS_TRACE_TAG_USER_DECL__("JNI.MTC");

PUBLIC
JniMtcCallThread::JniMtcCallThread() :
        BaseServiceThread(),
        m_nSlotId(0)
{
    IMS_TRACE_D("+JniMtcCallThread", 0, 0, 0);
}

PUBLIC VIRTUAL
JniMtcCallThread::~JniMtcCallThread()
{
    IMS_TRACE_D("~JniMtcCallThread", 0, 0, 0);
}

PUBLIC
void JniMtcCallThread::OnStarted(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
        IN const IMSMap<IMS_UINT32, SuppService*>& objSuppServices)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::STARTED);
    SetCallDetails(objParcel, pCallInfo, pMediaInfo, objSuppServices);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnStartFailed(IN const FailReason& objReason)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::START_FAILED);
    JniMtcUtils::WriteFailReasonToParcel(objReason, objParcel);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnProgressing(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
        IN const IMSMap<IMS_UINT32, SuppService*>& objSuppServices,
        IN IMS_BOOL bAlerted/* = IMS_FALSE*/)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::PROGRESSING);
    SetCallDetails(objParcel, pCallInfo, pMediaInfo, objSuppServices);
    objParcel.writeInt32(bAlerted ? 1 : 0);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnHeld(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
        IN const IMSMap<IMS_UINT32, SuppService*>& objSuppServices)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::HELD);
    SetCallDetails(objParcel, pCallInfo, pMediaInfo, objSuppServices);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnHoldFailed(IN const FailReason& objReason)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::HOLD_FAILED);
    JniMtcUtils::WriteFailReasonToParcel(objReason, objParcel);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnResumed(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
        IN const IMSMap<IMS_UINT32, SuppService*>& objSuppServices)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::RESUMED);
    SetCallDetails(objParcel, pCallInfo, pMediaInfo, objSuppServices);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnResumeFailed(IN const FailReason& objReason)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::RESUME_FAILED);
    JniMtcUtils::WriteFailReasonToParcel(objReason, objParcel);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnHeldBy(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
        IN const IMSMap<IMS_UINT32, SuppService*>& objSuppServices)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::HELD_BY);
    SetCallDetails(objParcel, pCallInfo, pMediaInfo, objSuppServices);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnResumedBy(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
        IN const IMSMap<IMS_UINT32, SuppService*>& objSuppServices)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::RESUMED_BY);
    SetCallDetails(objParcel, pCallInfo, pMediaInfo, objSuppServices);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnTerminated(IN const FailReason& objReason)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::TERMINATED);
    JniMtcUtils::WriteFailReasonToParcel(objReason, objParcel);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnIncomingResume(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
        IN const IMSMap<IMS_UINT32, SuppService*>& objSuppServices)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::INCOMING_RESUME);
    SetCallDetails(objParcel, pCallInfo, pMediaInfo, objSuppServices);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnIncomingUpdate(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
        IN const IMSMap<IMS_UINT32, SuppService*>& objSuppServices)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::INCOMING_UPDATE);
    SetCallDetails(objParcel, pCallInfo, pMediaInfo, objSuppServices);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnUpdated(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
        IN const IMSMap<IMS_UINT32, SuppService*>& objSuppServices)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::UPDATED);
    SetCallDetails(objParcel, pCallInfo, pMediaInfo, objSuppServices);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnUpdateFailed(IN const FailReason& objReason)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::UPDATE_FAILED);
    JniMtcUtils::WriteFailReasonToParcel(objReason, objParcel);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnUpdatedBy(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
        IN const IMSMap<IMS_UINT32, SuppService*>& objSuppServices)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::UPDATED_BY);
    SetCallDetails(objParcel, pCallInfo, pMediaInfo, objSuppServices);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnMerged(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
        IN const IMSMap<IMS_UINT32, SuppService*>& objSuppServices,
        IN const IMSList<ConfUser*>& objUsers)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::CONF_MERGED);
    SetCallDetails(objParcel, pCallInfo, pMediaInfo, objSuppServices);
    JniMtcUtils::WriteConfUsersToParcel(objUsers, objParcel);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnMergeFailed(IN const FailReason& /*objReason*/)
{
    // TODO:
}

PUBLIC
void JniMtcCallThread::OnConferenceParticipantAdded()
{
    // TODO:
}

PUBLIC
void JniMtcCallThread::OnConferenceParticipantAddFailed(IN const FailReason& /*objReason*/)
{
    // TODO:
}

PUBLIC
void JniMtcCallThread::OnConferenceParticipantRemoved()
{
    // TODO:
}

PUBLIC
void JniMtcCallThread::OnConferenceParticipantRemoveFailed(IN const FailReason& /*objReason*/)
{
    // TODO:
}

PUBLIC
void JniMtcCallThread::OnConferenceInfoChanged(IN const AString& /*strDisplayText*/,
        IN const AString /*strSubject*/, IN IMS_UINT32 /*nUserCount*/,
        IN IMS_UINT32 /*nMaxUserCount*/, IN const AString& /*strHost*/)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::CONF_NOTIFY_CONF_INFO);
    SendData2Java(objParcel);
}

PUBLIC
void JniMtcCallThread::OnConferenceParticipantsInfoChanged(IN const IMSList<ConfUser*>& objUsers)
{
    Parcel objParcel;
    objParcel.writeInt32(IuMtcCall::CONF_NOTIFY_USERS_INFO);
    JniMtcUtils::WriteConfUsersToParcel(objUsers, objParcel);
    SendData2Java(objParcel);
}

PRIVATE
void JniMtcCallThread::SetCallDetails(IN_OUT Parcel& objParcel, IN CallInfo* pCallInfo,
        IN MediaInfo* pMediaInfo, IN const IMSMap<IMS_UINT32, SuppService*>& objSuppServices)
{
    JniMtcUtils::WriteCallInfoToParcel(pCallInfo, objParcel);
    JniMtcUtils::WriteMediaInfoToParcel(pMediaInfo, objParcel);
    JniMtcUtils::WriteSuppServicesToParcel(objSuppServices, objParcel);
}
