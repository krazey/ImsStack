/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20110309  joonhun.shin@             Created
    20110502  hwangoo.park@             Adapted from RCS
    </table>

    Description

*/

#include "BaseServiceThread.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_USER_DECL__("BaseService");

PUBLIC
BaseServiceThread::BaseServiceThread() :
        BaseThread(),
        nNativeObject(0),
        pfnNotifier(IMS_NULL)
{
}

PUBLIC VIRTUAL BaseServiceThread::~BaseServiceThread() {}

PUBLIC
void BaseServiceThread::SetCallback(IN IMS_SINTP nNativeObject, CBServiceNoti pfnNotifier)
{
    this->nNativeObject = nNativeObject;
    this->pfnNotifier = pfnNotifier;
}

PROTECTED VIRTUAL IMS_BOOL BaseServiceThread::OnMessage(IN IMSMSG& objMSG)
{
    IMS_TRACE_D("OnMessage (%d)", objMSG.nMSG, 0, 0);
    switch (objMSG.nMSG)
    {
        case MESSAGE_THREAD_SWITCHING:
        {
            android::Parcel* pParcel = reinterpret_cast<android::Parcel*>(objMSG.nLparam);
            SendData2Java(*pParcel, IMS_TRUE);
            delete pParcel;
        }
        break;
        default:
            break;
    }

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL BaseServiceThread::SendData2Java(
        IN const android::Parcel& objParcel, IN IMS_BOOL bThreadSwitched /* = IMS_FALSE*/)
{
    if (nNativeObject == 0)
    {
        return IMS_FALSE;
    }

    if (pfnNotifier == IMS_NULL)
    {
        return IMS_FALSE;
    }

    int nMsg = objParcel.readInt32();
    objParcel.setDataPosition(0);
    IMS_BOOL bSendCurrentThread = bThreadSwitched || !IsThreadSwitchingRequired(nMsg);

    IMS_TRACE_D("SendData2Java (%s)", _TRACE_B_(bSendCurrentThread), 0, 0);
    if (bSendCurrentThread)
    {
        (*pfnNotifier)(nNativeObject, objParcel);
        return IMS_TRUE;
    }

    android::Parcel* pParcelOut = new android::Parcel();
    pParcelOut->write(objParcel.data(), objParcel.dataSize());
    pParcelOut->setDataPosition(0);

    IThread* piThread = GetThread();
    if (piThread)
    {
        piThread->PostMessageI(
                MESSAGE_THREAD_SWITCHING, 0, reinterpret_cast<IMS_UINTP>(pParcelOut));
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL BaseServiceThread::IsThreadSwitchingRequired(
        IN IMS_SINT32 /*nMsg*/) const
{
    return IMS_TRUE;
}
