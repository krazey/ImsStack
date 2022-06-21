/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef INTERFACE_UI_MTC_SERVICE_H_
#define INTERFACE_UI_MTC_SERVICE_H_

#include "CallReasonInfo.h"
#include "ImsMessageDef.h"
#include "IUBaseParam.h"
#include "MtcDef.h"
#include "IMSMap.h"

class ISession;

class IuMtcService
{
public:
    static const IMS_SINT32 EVENT_U2I = IMS_MSG_BASE_SERVICE;
    static const IMS_SINT32 EVENT_I2U = IMS_MSG_BASE_SERVICE + 100;
    static const IMS_SINT32 MAXIMUM = (EVENT_I2U + 99);

    // --- Event : UI to IMS ----------------------------------------------------------------------
    static const IMS_SINT32 OPEN_SESSION = (EVENT_U2I + 1);
    static const IMS_SINT32 ATTCH_SESSION = (EVENT_U2I + 2);
    static const IMS_SINT32 CLOSE_SESSION = (EVENT_U2I + 3);
    static const IMS_SINT32 REGISTER_SERVICE = (EVENT_U2I + 4);
    static const IMS_SINT32 SRVCC_STATE_CHANGED = (EVENT_U2I + 5);
    static const IMS_SINT32 SET_TERMINAL_BASED_CALL_WAITING = (EVENT_U2I + 6);
    static const IMS_SINT32 OPEN_EMERGENCY_SERVICE = (EVENT_U2I + 7);

    // HO
    static const IMS_SINT32 HO_CONFIRM = (EVENT_U2I + 50 + 1);
    static const IMS_SINT32 HO_HANDOVER = (EVENT_U2I + 50 + 2);

    // --- Event : IMS to UI ----------------------------------------------------------------------
    static const IMS_SINT32 SERVICE_CHANGED = (EVENT_I2U + 1);
    static const IMS_SINT32 E_SERVICE_CHANGED = (EVENT_I2U + 2);
    static const IMS_SINT32 PRE_INCOMING_CALL = (EVENT_I2U + 3);
    static const IMS_SINT32 INCOMING_CALL_INFO = (EVENT_I2U + 4);
    static const IMS_SINT32 AUTO_REJECTED_CALL = (EVENT_I2U + 5);

    // HO
    static const IMS_SINT32 HO_CONFIRMED = (EVENT_I2U + 50 + 1);

    static const IMS_SINT32 DIALOGS_NOTIFY_INFO = (EVENT_I2U + 60 + 1);

    enum
    {
        SERVICE_NONE = 0,
        SERVICE_VOIP = 1,
        SERVICE_VT = 2,
        SERVICE_UC = 3,
        SERVICE_EMERGENCY = 4,
        SERVICE_OPENING = 5,
    };  // SERVICE_STATE

    enum class EmergencyServiceStatus
    {
        IDLE = 0,
        OPENING = 1,
        OPENED = 2,
        UNAVAILABLE = 3,
        ES = 4,
    };

    enum
    {
        SERVICESTATUS_REASON_UNKNOWN = 0,
        SERVICESTATUS_REASON_NETWORKDISABLE = 1,
        SERVICESTATUS_REASON_SIMSINVALID = 2,
        SERVICESTATUS_REASON_BYSERVER = 3,
        SERVICESTATUS_REASON_USERSELECT = 4,
        SERVICESTATUS_REASON_FORBIDDEN = 5,
    };

    enum
    {
        ES_IDLE_REASON_UNKNOWN = -1,
        ES_IDLE_REASON_NONE = 0,
        ES_IDLE_REASON_WITH_ECM = 1,
    };

    enum
    {
        ES_UNAVAILABLE_REASON_UNKNOWN = -1,
        ES_UNAVAILABLE_REASON_NONE = 0,
        ES_UNAVAILABLE_REASON_NO_CSFB = 1,
        ES_UNAVAILABLE_REASON_SSAC = 2
    };

    inline static IMS_BOOL IsMsg(IN IMS_SINT32 nMsg)
    {
        return ((nMsg > EVENT_U2I) && (nMsg < MAXIMUM));
    }
};

class IUUCServiceBaseParam
{
public:
    inline IUUCServiceBaseParam() :
            strUIKey(AString::ConstNull()),
            nIMSKey(0)
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCServiceBaseParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCServiceBaseParam), this, 0);
    }
    inline virtual ~IUUCServiceBaseParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCServiceBaseParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCServiceBaseParam), this, 0);
    }

public:
    AString strUIKey;
    IMS_UINTP nIMSKey;
};

class IUUCServiceAttachSessionParam : public IUUCServiceBaseParam
{
public:
    inline IUUCServiceAttachSessionParam() :
            IUUCServiceBaseParam(),
            nSessionKey(0)
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCServiceAttachSessionParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCServiceAttachSessionParam), this, 0);
    }
    inline virtual ~IUUCServiceAttachSessionParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCServiceAttachSessionParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCServiceAttachSessionParam), this, 0);
    }

public:
    IMS_UINTP nSessionKey;
};

class IUUCServiceCloseSessionParam : public IUUCServiceBaseParam
{
public:
    inline IUUCServiceCloseSessionParam()
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCServiceCloseSessionParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCServiceCloseSessionParam), this, 0);
    }
    inline virtual ~IUUCServiceCloseSessionParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCServiceCloseSessionParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCServiceCloseSessionParam), this, 0);
    }

public:
};

class IUUCServiceRegisterServiceParam : public IUUCServiceBaseParam
{
public:
    inline IUUCServiceRegisterServiceParam() :
            IUUCServiceBaseParam(),
            strServiceName(AString::ConstNull())
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCServiceRegisterServiceParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCServiceRegisterServiceParam), this, 0);
    }
    inline virtual ~IUUCServiceRegisterServiceParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCServiceRegisterServiceParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCServiceRegisterServiceParam), this, 0);
    }

public:
    AString strServiceName;
};

class IUUCServiceChangedStatusParam : public IUUCServiceBaseParam
{
public:
    inline IUUCServiceChangedStatusParam() :
            IUUCServiceBaseParam(),
            eStatus(IuMtcService::SERVICE_NONE),
            eReason(IuMtcService::SERVICESTATUS_REASON_UNKNOWN)
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCServiceChangedStatusParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCServiceChangedStatusParam), this, 0);
    }
    inline virtual ~IUUCServiceChangedStatusParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCServiceChangedStatusParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCServiceChangedStatusParam), this, 0);
    }

public:
    IMS_SINT32 eStatus;
    IMS_SINT32 eReason;
};

class IUUCEServiceChangedStatusParam : public IUUCServiceBaseParam
{
public:
    inline IUUCEServiceChangedStatusParam() :
            IUUCServiceBaseParam(),
            eStatus(IuMtcService::SERVICE_NONE),
            eReason(IuMtcService::SERVICESTATUS_REASON_UNKNOWN)
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCEServiceChangedStatusParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCEServiceChangedStatusParam), this, 0);
    }
    inline virtual ~IUUCEServiceChangedStatusParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCEServiceChangedStatusParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCEServiceChangedStatusParam), this, 0);
    }

public:
    IMS_SINT32 eStatus;
    IMS_SINT32 eReason;
};

/*
class IUUCServiceIncomingSessionParam
    : public IUUCServiceBaseParam
{
public:
    inline IUUCServiceIncomingSessionParam()
        : IUUCServiceBaseParam()
        , nSessionKey(-1)
        , pCallInfo(IMS_NULL)
        , pMediaInfo(IMS_NULL)
        , objSuppServices(IMSMap<SuppType, SuppService*>())
        , aStrJNIServiceName(AString::ConstNull())
        , pISession(IMS_NULL)
        , pService(IMS_NULL)
        , eLocalService(ServiceType::UNKNOWN)
        , aStrSessionLogTag(AString::ConstNull())
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCServiceIncomingSessionParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCServiceIncomingSessionParam), this, 0);
    }
    inline virtual ~IUUCServiceIncomingSessionParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCServiceIncomingSessionParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCServiceIncomingSessionParam), this, 0);

        if (pCallInfo != IMS_NULL)
        {
            delete pCallInfo;
        }

        if (pMediaInfo != IMS_NULL)
        {
            delete pMediaInfo;
        }

        for (IMS_UINT32 i = 0; i < objSuppServices.GetSize(); ++i)
        {
            SuppService *pSS = objSuppServices.GetValueAt(i);
            if (pSS != IMS_NULL)
            {
                delete pSS;
            }
        }
        objSuppServices.Clear();

    }

private:
    IUUCServiceIncomingSessionParam(IN CONST IUUCServiceIncomingSessionParam &objRHS);
    IUUCServiceIncomingSessionParam& operator=(IN CONST IUUCServiceIncomingSessionParam &objRHS);

public:
    IMS_UINTP               nSessionKey;

    CallInfo*               pCallInfo;
    MediaInfo*              pMediaInfo;
    CallerInfo              stCallerInfo;
    IMSMap<SuppType, SuppService*>   objSuppServices;

    AString             aStrJNIServiceName;        // ( -> UCSession ) By UCApp, ( IMS -> UI ) NONE
    ISession*           pISession;                // ( -> UCSession ) By UCApp, ( IMS -> UI ) NONE
    IMtcService*        pService;
    ServiceType         eLocalService;
    AString             aStrSessionLogTag;

};
*/

/*
class IUUCIncomingCallInfoParam // IUUCServiceIncomingCallInfoParam...
    : public IUUCServiceBaseParam
{
public:
    inline IUUCIncomingCallInfoParam()
        : IUUCServiceBaseParam()
        , nServiceType(ServiceType::UNKNOWN)
        , eCallType(CallType::UNKNOWN)
        , nOIR(IuMtcService::OIPTYPE_INVALID)
        , nCNAP(IuMtcService::OIPTYPE_INVALID)
        , strOI(AString::ConstNull())
        , strCNA(AString::ConstNull())
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCIncomingCallInfoParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCIncomingCallInfoParam), this, 0);
    }
    inline virtual ~IUUCIncomingCallInfoParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCIncomingCallInfoParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCIncomingCallInfoParam), this, 0);
    }

public:
    ServiceType               nServiceType;
    CallType                  eCallType;
    IMS_SINT32                nOIR;
    IMS_SINT32                nCNAP;
    AString                   strOI;
    AString                   strCNA;
};

class IUUCAutoRejectedCallParam
    : public IUUCIncomingCallInfoParam
{
public:
    inline IUUCAutoRejectedCallParam()
        : IUUCIncomingCallInfoParam()
        , nReason(CODE_NONE)
        , piSession(IMS_NULL)
        , strJNIServiceName(AString::ConstNull())
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCAutoRejectedCallParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCAutoRejectedCallParam), this, 0);
    }
    inline virtual ~IUUCAutoRejectedCallParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCAutoRejectedCallParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCAutoRejectedCallParam), this, 0);
    }

private:
    IUUCAutoRejectedCallParam(IN CONST IUUCAutoRejectedCallParam &objRHS);
    IUUCAutoRejectedCallParam& operator=(IN CONST IUUCAutoRejectedCallParam &objRHS);

public:
    IMS_SINT32              nReason;
    ISession*               piSession;
    AString                 strJNIServiceName;
};
*/

class IUUCHOConfirmParam : public IUUCServiceBaseParam
{
public:
    inline IUUCHOConfirmParam() :
            IUUCServiceBaseParam(),
            eFrom(-1),
            eTo(-1)
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCHOConfirmParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCHOConfirmParam), this, 0);
    }
    inline virtual ~IUUCHOConfirmParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCHOConfirmParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCHOConfirmParam), this, 0);
    }

public:
    IMS_SINT32 eFrom;
    IMS_SINT32 eTo;
};

class IUUCHOHandOverParam : public IUUCServiceBaseParam
{
public:
    inline IUUCHOHandOverParam() :
            IUUCServiceBaseParam(),
            eFrom(-1),
            eTo(-1)
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCHOHandOverParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCHOHandOverParam), this, 0);
    }
    inline virtual ~IUUCHOHandOverParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCHOHandOverParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCHOHandOverParam), this, 0);
    }

public:
    IMS_SINT32 eFrom;
    IMS_SINT32 eTo;
};

class IUUCHOConfirmedParam : public IUUCServiceBaseParam
{
public:
    inline IUUCHOConfirmedParam() :
            IUUCServiceBaseParam(),
            eResult(-1),
            eReason(-1)
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCHOConfirmedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCHOConfirmedParam), this, 0);
    }
    inline virtual ~IUUCHOConfirmedParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCHOConfirmedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCHOConfirmedParam), this, 0);
    }

public:
    IMS_SINT32 eResult;
    IMS_SINT32 eReason;
};

class IUUCServiceDialogsNotifyInfoParam : public IUUCServiceBaseParam
{
public:
    inline IUUCServiceDialogsNotifyInfoParam() :
            IUUCServiceBaseParam(),
            lstDialogInfos(IMSList<DialogInfo*>())
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCServiceDialogsNotifyInfoParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCServiceDialogsNotifyInfoParam), this, 0);
    }
    inline virtual ~IUUCServiceDialogsNotifyInfoParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCServiceDialogsNotifyInfoParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCServiceDialogsNotifyInfoParam), this, 0);

        for (IMS_UINT32 i = 0; i < lstDialogInfos.GetSize(); ++i)
        {
            DialogInfo* pDialogInfo = lstDialogInfos.GetAt(i);
            if (pDialogInfo != IMS_NULL)
            {
                delete pDialogInfo;
            }
        }
        lstDialogInfos.Clear();
    }

public:
    IMSList<DialogInfo*> lstDialogInfos;
};

#endif  // INTERFACE_UI_MTC_SERVICE_H_
