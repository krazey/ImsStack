#include <utils/String8.h>

#define IMS_STL_USE

#include "JniMtsServiceThread.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "IUMtsService.h"

using namespace android;

__IMS_TRACE_TAG_USER_DECL__("JniMtsServiceThread");

PRIVATE
JniMtsServiceThread::JniMtsServiceThread() :
        m_nNativeObj(0),
        m_pfnNotifier(NULL),
        m_nSlotId(0)
{
    IMS_TRACE_I("JniMtsServiceThread : ", 0, 0, 0);
}

IMSAppThread* JniMtsServiceThread::GetInstance()
{
    return new JniMtsServiceThread();
}

PUBLIC VIRTUAL
JniMtsServiceThread::~JniMtsServiceThread()
{
    IMS_TRACE_I("~JniMtsServiceThread : ", 0, 0, 0);
}

PUBLIC VIRTUAL
IMS_SINT32 JniMtsServiceThread::SetCallback(
        IN IMS_SINTP nNativeObj, IN CBServiceNoti pfnNotifier, IN IMS_SINT32 nSlotId /*= 0*/)
{
    IMS_TRACE_D("SetCallback : SimSlot[%d]", nSlotId, 0, 0);
    this->m_nNativeObj = nNativeObj;
    this->m_pfnNotifier = pfnNotifier;
    this->m_nSlotId = nSlotId;
    return 1;
}

PUBLIC VIRTUAL
IMS_SINT32 JniMtsServiceThread::GetSimSlot()
{
    IMS_TRACE_D("GetSimSlot[%d]", m_nSlotId, 0, 0);
    return this->m_nSlotId;
}

PROTECTED VIRTUAL
IMS_BOOL JniMtsServiceThread::Initialize()
{
    IMS_TRACE_I("Initialize : ", 0, 0, 0);

    return IMS_TRUE;
}

PROTECTED VIRTUAL
void JniMtsServiceThread::Uninitialize()
{
    IMS_TRACE_I("Uninitialize : ", 0, 0, 0);
}

PROTECTED VIRTUAL
IMS_BOOL JniMtsServiceThread::OnStart(IN IMSMSG& objMSG)
{
    IMS_TRACE_I("OnStart : %d", objMSG.GetName(), 0, 0);

    return IMS_TRUE;
}

PROTECTED VIRTUAL
IMS_BOOL JniMtsServiceThread::OnTerminate(IN IMSMSG& objMSG)
{
    IMS_TRACE_I("OnTerminate : %d", objMSG.GetName(), 0, 0);

    return IMS_TRUE;
}

PROTECTED VIRTUAL
IMS_BOOL JniMtsServiceThread::OnMessage(IN IMSMSG& objMSG)
{
    IMS_TRACE_I("OnMessage : MSG [ %d ], wParam [ %" PFLS_u " ], lParam [ %" PFLS_u " ]",
            objMSG.nMSG, objMSG.nWparam, objMSG.nLparam);

    Parcel parcel;
    parcel.writeInt32(objMSG.nMSG);

    switch (objMSG.nMSG)
    {
        case IUMtsService::REPORT_MTS_MO_STATUS:
        {
            IUMtsServiceReportMoStatusParam* pParam
                    = reinterpret_cast<IUMtsServiceReportMoStatusParam*>(objMSG.nLparam);
            if (pParam != IMS_NULL)
            {
                parcel.writeInt32(pParam->nReason);
                parcel.writeInt32(pParam->nSmsFormat);
                parcel.writeInt32(pParam->nRetryAfter);
                parcel.writeInt32(pParam->nSeqId);
                parcel.writeInt32(pParam->nSlotId);
                delete pParam;
            }
        }
        break;
        case IUMtsService::REPORT_MTS_MT_SMS:
        {
            IUMtsServiceReportMtSmsParam* pParam
                    = reinterpret_cast<IUMtsServiceReportMtSmsParam*>(objMSG.nLparam);
            if (pParam != IMS_NULL)
            {
                parcel.writeInt32(pParam->nSmsFormat);
                parcel.writeString16(android::String16(pParam->objData.ToString().GetStr()));
                parcel.writeInt32(pParam->nSlotId);
                delete pParam;
            }
        }
        break;
        default:
        {
            IMS_TRACE_E(0, "OnMessage : unknown message = %d", objMSG.nMSG , 0, 0);
            return IMS_TRUE;
        }
        break;
    }

    if (m_pfnNotifier!= NULL)
    {
        m_pfnNotifier(m_nNativeObj, parcel);
    }
    else
    {
        IMS_TRACE_E(0, "[ERROR]OnMessage : call back is NULL", 0, 0, 0);
    }

    return IMS_TRUE;
}
