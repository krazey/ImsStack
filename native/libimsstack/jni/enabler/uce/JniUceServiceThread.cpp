/*
    Author
    <table>
    date              author                    description
    --------      --------------            ----------
    20111206      hyunho.shin@                       Created
    20121102    hyunho.shin@                       Re-Factorying
    </table>

    Description

*/

#include "JniUceServiceThread.h"

#include "IUUceService.h"
#include "IUce.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"

using namespace android;

__IMS_TRACE_TAG_USER_DECL__("IMS_UCE");

PRIVATE
JniUceServiceThread::JniUceServiceThread() :
        m_nNativeObj(0),
        m_pfnSendDataToJava(NULL)
{
    IMS_TRACE_D("UCE_M : JniUceServiceThread = %" PFLS_u, sizeof(JniUceServiceThread), 0, 0);
    IMS_TRACE_I("JniUceServiceThread : ", 0, 0, 0);
}

ImsAppThread* JniUceServiceThread::GetInstance()
{
    return new JniUceServiceThread();
}

PUBLIC VIRTUAL JniUceServiceThread::~JniUceServiceThread()
{
    IMS_TRACE_D("UCE_F : JniUceServiceThread = %" PFLS_u, sizeof(JniUceServiceThread), 0, 0);
    IMS_TRACE_I("~JniUceServiceThread : ", 0, 0, 0);
}

PUBLIC VIRTUAL int JniUceServiceThread::SetCallback(
        IN IMS_UINTP nNativeObj, IN Jni_SendDataToJava pfnSendDataToJava)
{
    IMS_TRACE_I("SetCallback : ", 0, 0, 0);
    this->m_nNativeObj = nNativeObj;
    this->m_pfnSendDataToJava = pfnSendDataToJava;
    return 1;
}
PROTECTED VIRTUAL IMS_BOOL JniUceServiceThread::Initialize()
{
    IMS_TRACE_I("Initialize : ", 0, 0, 0);
    return IMS_TRUE;
}

PROTECTED VIRTUAL void JniUceServiceThread::Uninitialize()
{
    IMS_TRACE_I("Uninitialize : ", 0, 0, 0);
}

PROTECTED VIRTUAL IMS_BOOL JniUceServiceThread::OnStart(IN IMSMSG& objMSG)
{
    IMS_TRACE_I("OnStart : %d", objMSG.GetName(), 0, 0);
    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL JniUceServiceThread::OnTerminate(IN IMSMSG& objMSG)
{
    IMS_TRACE_I("OnTerminate : %d", objMSG.GetName(), 0, 0);
    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL JniUceServiceThread::OnMessage(IN IMSMSG& objMSG)
{
    IMS_TRACE_I("OnMessage:MSG [%d], wParam[%" PFLS_u "], lParam[%" PFLS_u "]", objMSG.nMSG,
            objMSG.nWparam, objMSG.nLparam);

    Parcel parcel;
    parcel.writeInt32(objMSG.nMSG);

    switch (objMSG.nMSG)
    {
        case IUUceService::UCE_PUBLISH_RESPONSE_IND:
        {
            IUcePubResponseIndPrm* pParam = (IUcePubResponseIndPrm*)objMSG.nLparam;
            if (pParam != IMS_NULL)
            {
                parcel.writeInt32(pParam->m_nKey);
                parcel.writeInt64(pParam->m_nCapability);
                parcel.writeInt32(pParam->m_nResponseCode);
                parcel.writeString16(android::String16(pParam->m_strReason.GetStr()));
                parcel.writeInt32(pParam->m_nReasonHeaderCause);
                parcel.writeString16(android::String16(pParam->m_strReasonHeaderText.GetStr()));
                parcel.writeString16(android::String16(pParam->m_strEtag.GetStr()));
                parcel.writeInt32(pParam->m_nNeedToRetry);
                delete pParam;
            }
        }
        break;
        case IUUceService::UCE_PUBLISH_UPDATED_IND:
        {
            IUcePubUpdatedIndPrm* pParam = (IUcePubUpdatedIndPrm*)objMSG.nLparam;
            if (pParam != IMS_NULL)
            {
                parcel.writeInt64(pParam->m_nCapability);
                parcel.writeInt32(pParam->m_nResponseCode);
                parcel.writeString16(android::String16(pParam->m_strReason.GetStr()));
                parcel.writeInt32(pParam->m_nReasonHeaderCause);
                parcel.writeString16(android::String16(pParam->m_strReasonHeaderText.GetStr()));
                parcel.writeString16(android::String16(pParam->m_strEtag.GetStr()));
                parcel.writeInt32(pParam->m_nNeedToRetry);
                delete pParam;
            }
        }
        break;
        case IUUceService::UCE_PUBLISH_CMD_ERROR_IND:
        {
            IUcePubCmdErrorIndPrm* pParam = (IUcePubCmdErrorIndPrm*)objMSG.nLparam;
            if (pParam != IMS_NULL)
            {
                parcel.writeInt32(pParam->m_nKey);
                parcel.writeInt32(pParam->m_nCommandError);
                delete pParam;
            }
        }
        break;
        case IUUceService::UCE_SUBSCRIBE_RESPONSE_IND:
        {
            IUceSubResponseIndPrm* pParam = (IUceSubResponseIndPrm*)objMSG.nLparam;
            if (pParam != IMS_NULL)
            {
                parcel.writeInt32(pParam->m_nKey);
                parcel.writeInt32(pParam->m_nResponseCode);
                parcel.writeString16(android::String16(pParam->m_strReason.GetStr()));
                parcel.writeInt32(pParam->m_nReasonHeaderCause);
                parcel.writeString16(android::String16(pParam->m_strReasonHeaderText.GetStr()));
                delete pParam;
            }
        }
        break;
        case IUUceService::UCE_PRESENCE_NOTIFY_IND:
        {
            IUcePreNotifyIndPrm* pParam = (IUcePreNotifyIndPrm*)objMSG.nLparam;
            if (pParam != IMS_NULL)
            {
                parcel.writeInt32(pParam->m_nKey);
                parcel.writeInt32(pParam->m_nCount);
                for (IMS_UINT32 n = 0; n < pParam->m_nCount; n++)
                {
                    AString pidfXml = pParam->m_lstPidfXmls.GetAt(n);
                    parcel.writeString16(android::String16(pidfXml.GetStr()));
                }
                delete pParam;
            }
        }
        break;
        case IUUceService::UCE_SUBSCRIBE_CMD_ERROR_IND:
        {
            IUceSubCmdErrorIndPrm* pParam = (IUceSubCmdErrorIndPrm*)objMSG.nLparam;
            if (pParam != IMS_NULL)
            {
                parcel.writeInt32(pParam->m_nKey);
                parcel.writeInt32(pParam->m_nCommandError);
                delete pParam;
            }
        }
        break;
        case IUUceService::UCE_SUBSCRIBE_RESOURCE_TERMINATED_IND:
        {
            IUceSubResourceTerminatedIndPrm* pParam =
                    (IUceSubResourceTerminatedIndPrm*)objMSG.nLparam;
            if (pParam != IMS_NULL)
            {
                parcel.writeInt32(pParam->m_nKey);
                parcel.writeInt32(pParam->m_nCount);
                for (IMS_UINT32 i = 0; i < pParam->m_nCount; i++)
                {
                    IUceTerminatedReason* pTempContact = (pParam->m_lstTerminateContacts).GetAt(i);
                    if (pTempContact != null)
                    {
                        parcel.writeString16(
                                android::String16(pTempContact->m_strContact.GetStr()));
                        parcel.writeString16(android::String16(pTempContact->m_strReason.GetStr()));
                        pTempContact = null;
                    }
                    else
                    {
                        AString temp = AString::ConstEmpty();
                        parcel.writeString16(android::String16(temp.GetStr()));
                        parcel.writeString16(android::String16(temp.GetStr()));
                    }
                }
                delete pParam;
            }
        }
        break;
        case IUUceService::UCE_SUBSCRIBE_TERMINATED_IND:
        {
            IUceSubTerminatedIndPrm* pParam = (IUceSubTerminatedIndPrm*)objMSG.nLparam;
            if (pParam != IMS_NULL)
            {
                parcel.writeInt32(pParam->m_nKey);
                parcel.writeString16(android::String16(pParam->m_strReason.GetStr()));
                parcel.writeInt32(pParam->m_nRetryAfterMillsecond);
                delete pParam;
            }
        }
        break;
        case IUUceService::UCE_OPTIONS_RESPONSE_IND:
        {
            IUceOptionsResponseIndPrm* pParam = (IUceOptionsResponseIndPrm*)objMSG.nLparam;
            if (pParam != IMS_NULL)
            {
                parcel.writeInt32(pParam->m_nKey);
                parcel.writeInt32(pParam->m_nResponseCode);
                parcel.writeString16(android::String16(pParam->m_strReason.GetStr()));
                parcel.writeInt64(pParam->m_nTheirCaps);
                delete pParam;
            }
        }
        break;
        case IUUceService::UCE_OPTIONS_CMD_ERROR_IND:
        {
            IUceOptionsCmdErrorIndPrm* pParam = (IUceOptionsCmdErrorIndPrm*)objMSG.nLparam;
            if (pParam != IMS_NULL)
            {
                parcel.writeInt32(pParam->m_nKey);
                parcel.writeInt32(pParam->m_nCommandError);
                delete pParam;
            }
        }
        break;
        case IUUceService::UCE_OPTIONS_RECEIVED_IND:
        {
            IUceOptionsReceivedIndPrm* pParam = (IUceOptionsReceivedIndPrm*)objMSG.nLparam;
            if (pParam != IMS_NULL)
            {
                parcel.writeInt32(pParam->m_nKey);
                parcel.writeString16(android::String16(pParam->m_strRemote.GetStr()));
                parcel.writeInt64(pParam->m_nRemoteCaps);
                delete pParam;
            }
        }
        break;
        case IUUceService::UCE_IMS_AGENT_CONNECTED_IND:
        {
            parcel.writeInt64(objMSG.nWparam);
            parcel.writeInt32(LONG_TO_SINT(objMSG.nLparam));
        }
        break;
        case IUUceService::UCE_IMS_AGENT_DISCONNECTED_IND:
            break;
        case IUUceService::UCE_IMS_AGENT_REFRESHED_IND:
        {
            parcel.writeInt32(LONG_TO_SINT(objMSG.nWparam));
        }
        break;
        case IUUceService::UCE_NETWORK_CHANGED:
            parcel.writeInt32(LONG_TO_SINT(objMSG.nLparam));
            break;
        default:
            IMS_TRACE_I("OnMessage : unknown message = %d", objMSG.nMSG, 0, 0);
            return IMS_TRUE;
            break;
    }

    if (m_pfnSendDataToJava != NULL)
    {
        m_pfnSendDataToJava(m_nNativeObj, parcel);
    }
    else
    {
        IMS_TRACE_E(0, "[ERROR]OnMessage : call back is NULL", 0, 0, 0);
    }
    return IMS_TRUE;
}
