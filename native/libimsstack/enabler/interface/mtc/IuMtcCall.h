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

#ifndef INTERFACE_UI_MTC_CALL_H_
#define INTERFACE_UI_MTC_CALL_H_

#include "CallReasonInfo.h"
#include "ImsMessageDef.h"
#include "ImsMessage.h"
#include "MtcDef.h"
#include "ImsMap.h"
#include "call/IMtcCall.h"

class IuMtcService;
class IMtcService;
class MtcCall;
class IDialogEvent;

class IuMtcCall
{
public:
    static const IMS_SINT32 EVENT_U2I = IMS_MSG_BASE_SESSION;
    static const IMS_SINT32 EVENT_I2U = IMS_MSG_BASE_SESSION + 100;

    static const IMS_SINT32 EVENT_CONF_U2I = EVENT_U2I + 30;
    static const IMS_SINT32 EVENT_CONF_I2U = EVENT_I2U + 30;

    static const IMS_SINT32 EVENT_ECT_U2I = EVENT_U2I + 60;
    static const IMS_SINT32 EVENT_ECT_I2U = EVENT_I2U + 60;

    static const IMS_SINT32 EVENT_MEDIA_I2U = EVENT_I2U + 90;

    static const IMS_SINT32 MAXIMUM = (EVENT_I2U + 99);

    // UI to IMS events
    static const IMS_SINT32 START = (EVENT_U2I + 1);
    static const IMS_SINT32 STARTCONF = (EVENT_U2I + 2);
    static const IMS_SINT32 USER_ALERT = (EVENT_U2I + 3);
    static const IMS_SINT32 ACCEPT = (EVENT_U2I + 4);
    static const IMS_SINT32 REJECT = (EVENT_U2I + 5);
    static const IMS_SINT32 HOLD = (EVENT_U2I + 6);
    static const IMS_SINT32 RESUME = (EVENT_U2I + 7);
    static const IMS_SINT32 SEND_DTMF = (EVENT_U2I + 8);
    static const IMS_SINT32 TERMINATE = (EVENT_U2I + 9);
    static const IMS_SINT32 UPDATE = (EVENT_U2I + 10);
    static const IMS_SINT32 ACCEPT_UPDATE = (EVENT_U2I + 11);
    static const IMS_SINT32 REJECT_UPDATE = (EVENT_U2I + 12);
    static const IMS_SINT32 CANCEL_UPDATE = (EVENT_U2I + 13);
    static const IMS_SINT32 ACCEPT_RESUME = (EVENT_U2I + 14);
    static const IMS_SINT32 REJECT_RESUME = (EVENT_U2I + 15);

    static const IMS_SINT32 SEND_USSD = (EVENT_U2I + 16);

    static const IMS_SINT32 CONF_EXPAND = (EVENT_CONF_U2I + 1);
    static const IMS_SINT32 CONF_MERGE = (EVENT_CONF_U2I + 2);
    static const IMS_SINT32 CONF_JOIN = (EVENT_CONF_U2I + 3);
    static const IMS_SINT32 CONF_DROP = (EVENT_CONF_U2I + 4);
    static const IMS_SINT32 CONF_DELETE = (EVENT_CONF_U2I + 5);

    static const IMS_SINT32 ECT_START = (EVENT_ECT_U2I + 1);
    static const IMS_SINT32 PUSH_CALL = (EVENT_ECT_U2I + 2);
    static const IMS_SINT32 CANCEL_CALL_PUSH = (EVENT_ECT_U2I + 3);
    static const IMS_SINT32 ECT_START_BLIND = (EVENT_ECT_U2I + 4);

    static const IMS_SINT32 ATTACH = (EVENT_U2I + 98);
    static const IMS_SINT32 OPEN = (EVENT_U2I + 99);

    // IMS to UI events
    static const IMS_SINT32 STARTED = (EVENT_I2U + 1);
    static const IMS_SINT32 START_FAILED = (EVENT_I2U + 2);
    static const IMS_SINT32 PROGRESSING = (EVENT_I2U + 3);
    static const IMS_SINT32 HELD = (EVENT_I2U + 4);
    static const IMS_SINT32 HOLD_FAILED = (EVENT_I2U + 5);
    static const IMS_SINT32 HELD_BY = (EVENT_I2U + 6);
    static const IMS_SINT32 RESUMED = (EVENT_I2U + 7);
    static const IMS_SINT32 RESUME_FAILED = (EVENT_I2U + 8);
    static const IMS_SINT32 RESUMED_BY = (EVENT_I2U + 9);
    static const IMS_SINT32 TERMINATED = (EVENT_I2U + 10);
    static const IMS_SINT32 INCOMING_UPDATE = (EVENT_I2U + 11);
    static const IMS_SINT32 UPDATED = (EVENT_I2U + 12);
    static const IMS_SINT32 UPDATE_FAILED = (EVENT_I2U + 13);
    static const IMS_SINT32 UPDATED_BY = (EVENT_I2U + 14);
    static const IMS_SINT32 NOTIFY_INFO = (EVENT_I2U + 15);
    static const IMS_SINT32 INCOMING_RESUME = (EVENT_I2U + 16);
    static const IMS_SINT32 SET_PROPERTY = (EVENT_I2U + 17);
    static const IMS_SINT32 INCOMING_CALL_RECEIVED = (EVENT_I2U + 18);

    static const IMS_SINT32 CONF_EXPANDED = (EVENT_CONF_I2U + 1);
    static const IMS_SINT32 CONF_EXPANDFAILED = (EVENT_CONF_I2U + 2);
    static const IMS_SINT32 CONF_EXPANDED_BY = (EVENT_CONF_I2U + 3);
    static const IMS_SINT32 CONF_MERGED = (EVENT_CONF_I2U + 4);
    static const IMS_SINT32 CONF_MERGEFAILED = (EVENT_CONF_I2U + 5);
    static const IMS_SINT32 CONF_JOINED = (EVENT_CONF_I2U + 6);
    static const IMS_SINT32 CONF_DROPPED = (EVENT_CONF_I2U + 7);
    static const IMS_SINT32 CONF_DELETED = (EVENT_CONF_I2U + 8);
    static const IMS_SINT32 CONF_NOTIFY_USERS_INFO = (EVENT_CONF_I2U + 9);
    static const IMS_SINT32 CONF_NOTIFY_CONF_INFO = (EVENT_CONF_I2U + 10);

    static const IMS_SINT32 ECT_COMPLETED = (EVENT_ECT_I2U + 1);
    static const IMS_SINT32 REPLACED_BY = (EVENT_ECT_I2U + 2);
    static const IMS_SINT32 CALL_PUSH_COMPLETED = (EVENT_ECT_I2U + 3);

    static const IMS_SINT32 CODEC_INFO_UPDATED = (EVENT_MEDIA_I2U + 1);

    inline static IMS_BOOL IsMsg(IN IMS_SINT32 nMsg)
    {
        return ((nMsg > EVENT_U2I) && (nMsg < MAXIMUM));
    }
};

class IUUCSessionBaseParam
{
public:
    inline IUUCSessionBaseParam() :
            strUIKey(AString::ConstNull()),
            nIMSKey(0),
            aStrLogTag(AString::ConstNull())
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionBaseParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionBaseParam), this, 0);
    }
    inline IUUCSessionBaseParam(IN const IUUCSessionBaseParam& objRHS) :
            strUIKey(objRHS.strUIKey),
            nIMSKey(objRHS.nIMSKey)
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionBaseParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionBaseParam), this, 0);
    }
    inline virtual ~IUUCSessionBaseParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionBaseParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionBaseParam), this, 0);
    }

private:
    IUUCSessionBaseParam& operator=(IN const IUUCSessionBaseParam& objRHS);

public:
    AString strUIKey;
    IMS_UINTP nIMSKey;
    AString aStrLogTag;
};

class IUUCSessionFailParam
{
public:
    inline IUUCSessionFailParam() :
            objReason(CallReasonInfo(CODE_NONE))
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionFailParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionFailParam), this, 0);
    }
    inline virtual ~IUUCSessionFailParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionFailParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionFailParam), this, 0);
    }

private:
    IUUCSessionFailParam(IN const IUUCSessionFailParam& objRHS);
    IUUCSessionFailParam& operator=(IN const IUUCSessionFailParam& objRHS);

public:
    CallReasonInfo objReason;
};

class IUUCSessionFailedParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionFailedParam() :
            IUUCSessionBaseParam(),
            objReason(CallReasonInfo(CODE_NONE)),
            aStrPhrase(AString::ConstEmpty())
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionFailedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionFailedParam), this, 0);
    }
    inline virtual ~IUUCSessionFailedParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionFailedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionFailedParam), this, 0);
    }

private:
    IUUCSessionFailedParam(IN const IUUCSessionFailedParam& objRHS);
    IUUCSessionFailedParam& operator=(IN const IUUCSessionFailedParam& objRHS);

public:
    CallReasonInfo objReason;
    AString aStrPhrase;
};

/* ------------------------------------------------------------------------------------------------
    UI 2 IMS Params
------------------------------------------------------------------------------------------------ */

class IUUCSessionStartParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionStartParam() :
            IUUCSessionBaseParam(),
            eCallType(CallType::VOIP),
            aStrTarget(AString::ConstNull()),
            pMediaInfo(IMS_NULL),
            objSuppServices(IMSMap<SuppType, SuppService*>()),
            pService(IMS_NULL),
            pDialog(IMS_NULL)
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionStartParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionStartParam), this, 0);
    }
    inline virtual ~IUUCSessionStartParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionStartParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionStartParam), this, 0);

        if (pMediaInfo != IMS_NULL)
        {
            delete pMediaInfo;
        }

        for (IMS_UINT32 i = 0; i < objSuppServices.GetSize(); ++i)
        {
            SuppService* pSS = objSuppServices.GetValueAt(i);
            if (pSS != IMS_NULL)
            {
                delete pSS;
            }
        }
        objSuppServices.Clear();
    }

private:
    IUUCSessionStartParam(IN CONST IUUCSessionStartParam& objRHS);
    IUUCSessionStartParam& operator=(IN CONST IUUCSessionStartParam& objRHS);

public:
    CallType eCallType;
    AString aStrTarget;

    MediaInfo* pMediaInfo;
    IMSMap<SuppType, SuppService*> objSuppServices;

    /* -------------------------------------------------- */
    IMtcService* pService;
    IDialogEvent* pDialog;
};


class IUUCSessionConferenceParam
{
public:
    inline IUUCSessionConferenceParam() :
            lstUsers(IMSList<ConfUser*>()),
            eCreateType(CONF_CREATE_NONE)
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionConferenceParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionConferenceParam), this, 0);
    }
    inline virtual ~IUUCSessionConferenceParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionConferenceParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionConferenceParam), this, 0);

        for (IMS_UINT32 i = 0; i < lstUsers.GetSize(); ++i)
        {
            ConfUser* pUser = lstUsers.GetAt(i);
            if (pUser != IMS_NULL)
            {
                delete pUser;
            }
        }
        lstUsers.Clear();
    }

public:
    IMSList<ConfUser*> lstUsers;
    IMS_UINT32 eCreateType;
};


class IUUCSessionConferenceCallParam :
        public IUUCSessionStartParam  // to cover StartConfParam
        ,
        public IUUCSessionConferenceParam
{
public:
    inline IUUCSessionConferenceCallParam() :
            IUUCSessionStartParam(),
            IUUCSessionConferenceParam(),
            pCallInfo(IMS_NULL)
    {
    }
    inline virtual ~IUUCSessionConferenceCallParam()
    {
        if (pCallInfo != IMS_NULL)
        {
            delete pCallInfo;
        }
    }

private:
    IUUCSessionConferenceCallParam(IN CONST IUUCSessionConferenceCallParam& objRHS);
    IUUCSessionConferenceCallParam& operator=(IN CONST IUUCSessionConferenceCallParam& objRHS);

public:
    CallInfo* pCallInfo;
};


class IUUCSessionStartConfParam : public IUUCSessionStartParam, public IUUCSessionConferenceParam
{
public:
    inline IUUCSessionStartConfParam() :
            IUUCSessionStartParam(),
            IUUCSessionConferenceParam()
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionStartConfParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionStartConfParam), this, 0);
    }
    inline virtual ~IUUCSessionStartConfParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionStartConfParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionStartConfParam), this, 0);
    }

public:
};


class IUUCSessionUserAlertParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionUserAlertParam() :
            IUUCSessionBaseParam()
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionUserAlertParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionUserAlertParam), this, 0);
    }
    inline virtual ~IUUCSessionUserAlertParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionUserAlertParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionUserAlertParam), this, 0);
    }

public:
};


class IUUCSessionAcceptParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionAcceptParam() :
            IUUCSessionBaseParam(),
            eCallType(CallType::VOIP),
            pMediaInfo(IMS_NULL)
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionAcceptParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionAcceptParam), this, 0);
    }
    inline virtual ~IUUCSessionAcceptParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionAcceptParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionAcceptParam), this, 0);

        if (pMediaInfo != IMS_NULL)
        {
            delete pMediaInfo;
        }
    }

private:
    IUUCSessionAcceptParam(IN CONST IUUCSessionAcceptParam& objRHS);
    IUUCSessionAcceptParam& operator=(IN CONST IUUCSessionAcceptParam& objRHS);

public:
    CallType eCallType;
    MediaInfo* pMediaInfo;
};


class IUUCSessionRejectParam : public IUUCSessionBaseParam, public IUUCSessionFailParam
{
public:
    inline IUUCSessionRejectParam() :
            IUUCSessionBaseParam(),
            IUUCSessionFailParam()
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionRejectParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionRejectParam), this, 0);
    }
    inline virtual ~IUUCSessionRejectParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionRejectParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionRejectParam), this, 0);
    }

public:
};


class IUUCSessionHoldParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionHoldParam() :
            IUUCSessionBaseParam(),
            pMediaInfo(IMS_NULL)
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionHoldParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionHoldParam), this, 0);
    }
    inline virtual ~IUUCSessionHoldParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionHoldParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionHoldParam), this, 0);

        if (pMediaInfo != IMS_NULL)
        {
            delete pMediaInfo;
        }
    }

private:
    IUUCSessionHoldParam(IN CONST IUUCSessionHoldParam& objRHS);
    IUUCSessionHoldParam& operator=(IN CONST IUUCSessionHoldParam& objRHS);

public:
    MediaInfo* pMediaInfo;
};


class IUUCSessionResumeParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionResumeParam() :
            IUUCSessionBaseParam(),
            pMediaInfo(IMS_NULL)
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionResumeParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionResumeParam), this, 0);
    }
    inline virtual ~IUUCSessionResumeParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionResumeParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionResumeParam), this, 0);

        if (pMediaInfo != IMS_NULL)
        {
            delete pMediaInfo;
        }
    }

private:
    IUUCSessionResumeParam(IN CONST IUUCSessionResumeParam& objRHS);
    IUUCSessionResumeParam& operator=(IN CONST IUUCSessionResumeParam& objRHS);

public:
    MediaInfo* pMediaInfo;
};


class IUUCSessionAcceptResumeParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionAcceptResumeParam() :
            IUUCSessionBaseParam(),
            eCallType(CallType::VOIP),
            pMediaInfo(IMS_NULL)
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionAcceptResumeParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionAcceptResumeParam), this, 0);
    }
    inline virtual ~IUUCSessionAcceptResumeParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionAcceptResumeParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionAcceptResumeParam), this, 0);

        if (pMediaInfo != IMS_NULL)
        {
            delete pMediaInfo;
        }
    }

private:
    IUUCSessionAcceptResumeParam(IN CONST IUUCSessionAcceptResumeParam& objRHS);
    IUUCSessionAcceptResumeParam& operator=(IN CONST IUUCSessionAcceptResumeParam& objRHS);

public:
    CallType eCallType;
    MediaInfo* pMediaInfo;
};


class IUUCSessionRejectResumeParam : public IUUCSessionBaseParam, public IUUCSessionFailParam
{
public:
    inline IUUCSessionRejectResumeParam() :
            IUUCSessionBaseParam(),
            IUUCSessionFailParam()
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionRejectResumeParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionRejectResumeParam), this, 0);
    }
    inline virtual ~IUUCSessionRejectResumeParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionRejectResumeParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionRejectResumeParam), this, 0);
    }

public:
};


class IUUCSessionSendDTMFParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionSendDTMFParam() :
            IUUCSessionBaseParam(),
            strSignal(AString::ConstNull()),
            nDuration(0)
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionSendDTMFParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionSendDTMFParam), this, 0);
    }
    inline virtual ~IUUCSessionSendDTMFParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionSendDTMFParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionSendDTMFParam), this, 0);
    }

public:
    AString strSignal;
    IMS_SINT32 nDuration;
};


class IUUCSessionTerminateParam : public IUUCSessionBaseParam, public IUUCSessionFailParam
{
public:
    inline IUUCSessionTerminateParam() :
            IUUCSessionBaseParam(),
            IUUCSessionFailParam()
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionTerminateParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionTerminateParam), this, 0);
    }
    inline virtual ~IUUCSessionTerminateParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionTerminateParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionTerminateParam), this, 0);
    }

public:
};


class IUUCSessionUpdateParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionUpdateParam() :
            IUUCSessionBaseParam(),
            eCallType(CallType::VOIP),
            pMediaInfo(IMS_NULL),
            eReOfferMode(0)
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionUpdateParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionUpdateParam), this, 0);
    }
    inline virtual ~IUUCSessionUpdateParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionUpdateParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionUpdateParam), this, 0);

        if (pMediaInfo != IMS_NULL)
        {
            delete pMediaInfo;
        }
    }

private:
    IUUCSessionUpdateParam(IN CONST IUUCSessionUpdateParam& objRHS);
    IUUCSessionUpdateParam& operator=(IN CONST IUUCSessionUpdateParam& objRHS);

public:
    CallType eCallType;
    MediaInfo* pMediaInfo;

    IMS_UINT32 eReOfferMode;
};


class IUUCSessionAcceptUpdateParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionAcceptUpdateParam() :
            IUUCSessionBaseParam(),
            eCallType(CallType::VOIP),
            pMediaInfo(IMS_NULL)
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionAcceptUpdateParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionAcceptUpdateParam), this, 0);
    }
    inline virtual ~IUUCSessionAcceptUpdateParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionAcceptUpdateParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionAcceptUpdateParam), this, 0);

        if (pMediaInfo != IMS_NULL)
        {
            delete pMediaInfo;
        }
    }

private:
    IUUCSessionAcceptUpdateParam(IN CONST IUUCSessionAcceptUpdateParam& objRHS);
    IUUCSessionAcceptUpdateParam& operator=(IN CONST IUUCSessionAcceptUpdateParam& objRHS);

public:
    CallType eCallType;
    MediaInfo* pMediaInfo;
};


class IUUCSessionRejectUpdateParam : public IUUCSessionBaseParam, public IUUCSessionFailParam
{
public:
    inline IUUCSessionRejectUpdateParam() :
            IUUCSessionBaseParam(),
            IUUCSessionFailParam()
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionRejectUpdateParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionRejectUpdateParam), this, 0);
    }
    inline virtual ~IUUCSessionRejectUpdateParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionRejectUpdateParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionRejectUpdateParam), this, 0);
    }

public:
};


class IUUCSessionCancelUpdateParam : public IUUCSessionBaseParam, public IUUCSessionFailParam
{
public:
    inline IUUCSessionCancelUpdateParam() :
            IUUCSessionBaseParam(),
            IUUCSessionFailParam()
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionCancelUpdateParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionCancelUpdateParam), this, 0);
    }

    inline virtual ~IUUCSessionCancelUpdateParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionCancelUpdateParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionCancelUpdateParam), this, 0);
    }

public:
};


class IUUCSessionSendTransactionParam : public IUUCSessionBaseParam, public IUUCSessionFailParam
{
public:
    inline IUUCSessionSendTransactionParam() :
            IUUCSessionBaseParam(),
            IUUCSessionFailParam(),
            aStrExt(AString::ConstNull()),
            aStrUSSI(AString::ConstNull())

    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionSendTransactionParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionSendTransactionParam), this, 0);
    }
    inline virtual ~IUUCSessionSendTransactionParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionSendTransactionParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionSendTransactionParam), this, 0);
    }

public:
    AString aStrExt;
    AString aStrUSSI;
};


class IUUCSessionAttachParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionAttachParam() :
            IUUCSessionBaseParam()
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionAttachParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionAttachParam), this, 0);
    }
    inline virtual ~IUUCSessionAttachParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionAttachParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionAttachParam), this, 0);
    }

public:
};

class IUUCSessionSetPropertyParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionSetPropertyParam() :
            IUUCSessionBaseParam(),
            name(AString::ConstNull()),
            v1(0),
            v2(AString::ConstNull()),
            v3(IMS_NULL)
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionSetPropertyParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionSetPropertyParam), this, 0);
    }
    inline virtual ~IUUCSessionSetPropertyParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionSetPropertyParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionSetPropertyParam), this, 0);
    }

public:
    AString name;
    IMS_UINT32 v1;
    AString v2;
    IMS_BYTE* v3;
};

/* ------------------------------------------------------------------------------------------------
    IMS 2 UI Params
------------------------------------------------------------------------------------------------ */

class IUUCSessionStartedParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionStartedParam() :
            IUUCSessionBaseParam(),
            pCallInfo(IMS_NULL),
            pMediaInfo(IMS_NULL),
            objSuppServices(IMSMap<SuppType, SuppService*>())
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionStartedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionStartedParam), this, 0);
    }
    inline virtual ~IUUCSessionStartedParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionStartedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionStartedParam), this, 0);

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
            SuppService* pSS = objSuppServices.GetValueAt(i);
            if (pSS != IMS_NULL)
            {
                delete pSS;
            }
        }
        objSuppServices.Clear();
    }

private:
    IUUCSessionStartedParam(IN CONST IUUCSessionStartedParam& objRHS);
    IUUCSessionStartedParam& operator=(IN CONST IUUCSessionStartedParam& objRHS);

public:
    CallInfo* pCallInfo;
    MediaInfo* pMediaInfo;
    IMSMap<SuppType, SuppService*> objSuppServices;
};


class IUUCSessionStartFailedParam : public IUUCSessionFailedParam
{
public:
    inline IUUCSessionStartFailedParam() :
            IUUCSessionFailedParam()
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionStartFailedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionStartFailedParam), this, 0);
    }
    inline virtual ~IUUCSessionStartFailedParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionStartFailedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionStartFailedParam), this, 0);
    }

public:
};


class IUUCSessionProgressingParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionProgressingParam() :
            IUUCSessionBaseParam(),
            pCallInfo(IMS_NULL),
            pMediaInfo(IMS_NULL),
            objSuppServices(IMSMap<SuppType, SuppService*>()),
            bAlerted(IMS_FALSE)
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionProgressingParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionProgressingParam), this, 0);
    }
    inline virtual ~IUUCSessionProgressingParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionProgressingParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionProgressingParam), this, 0);

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
            SuppService* pSS = objSuppServices.GetValueAt(i);
            if (pSS != IMS_NULL)
            {
                delete pSS;
            }
        }
        objSuppServices.Clear();
    }

private:
    IUUCSessionProgressingParam(IN CONST IUUCSessionProgressingParam& objRHS);
    IUUCSessionProgressingParam& operator=(IN CONST IUUCSessionProgressingParam& objRHS);

public:
    CallInfo* pCallInfo;
    MediaInfo* pMediaInfo;
    IMSMap<SuppType, SuppService*> objSuppServices;
    IMS_BOOL bAlerted;
};


class IUUCSessionHeldParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionHeldParam() :
            IUUCSessionBaseParam(),
            pCallInfo(IMS_NULL),
            pMediaInfo(IMS_NULL),
            objSuppServices(IMSMap<SuppType, SuppService*>())
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionHeldParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionHeldParam), this, 0);
    }
    inline virtual ~IUUCSessionHeldParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionHeldParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionHeldParam), this, 0);

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
            SuppService* pSS = objSuppServices.GetValueAt(i);
            if (pSS != IMS_NULL)
            {
                delete pSS;
            }
        }
        objSuppServices.Clear();
    }

private:
    IUUCSessionHeldParam(IN CONST IUUCSessionHeldParam& objRHS);
    IUUCSessionHeldParam& operator=(IN CONST IUUCSessionHeldParam& objRHS);

public:
    CallInfo* pCallInfo;
    MediaInfo* pMediaInfo;
    IMSMap<SuppType, SuppService*> objSuppServices;
};


class IUUCSessionHoldFailedParam : public IUUCSessionFailedParam
{
public:
    inline IUUCSessionHoldFailedParam() :
            IUUCSessionFailedParam()
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionHoldFailedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionHoldFailedParam), this, 0);
    }
    inline virtual ~IUUCSessionHoldFailedParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionHoldFailedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionHoldFailedParam), this, 0);
    }

public:
};


class IUUCSessionHeldByParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionHeldByParam() :
            IUUCSessionBaseParam(),
            pCallInfo(IMS_NULL),
            pMediaInfo(IMS_NULL),
            objSuppServices(IMSMap<SuppType, SuppService*>())
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionHeldByParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionHeldByParam), this, 0);
    }
    inline virtual ~IUUCSessionHeldByParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionHeldByParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionHeldByParam), this, 0);

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
            SuppService* pSS = objSuppServices.GetValueAt(i);
            if (pSS != IMS_NULL)
            {
                delete pSS;
            }
        }
        objSuppServices.Clear();
    }

private:
    IUUCSessionHeldByParam(IN CONST IUUCSessionHeldByParam& objRHS);
    IUUCSessionHeldByParam& operator=(IN CONST IUUCSessionHeldByParam& objRHS);

public:
    CallInfo* pCallInfo;
    MediaInfo* pMediaInfo;
    IMSMap<SuppType, SuppService*> objSuppServices;
};


class IUUCSessionResumedParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionResumedParam() :
            IUUCSessionBaseParam(),
            pCallInfo(IMS_NULL),
            pMediaInfo(IMS_NULL),
            objSuppServices(IMSMap<SuppType, SuppService*>())
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionResumedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionResumedParam), this, 0);
    }
    inline virtual ~IUUCSessionResumedParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionResumedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionResumedParam), this, 0);

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
            SuppService* pSS = objSuppServices.GetValueAt(i);
            if (pSS != IMS_NULL)
            {
                delete pSS;
            }
        }
        objSuppServices.Clear();
    }

private:
    IUUCSessionResumedParam(IN CONST IUUCSessionResumedParam& objRHS);
    IUUCSessionResumedParam& operator=(IN CONST IUUCSessionResumedParam& objRHS);

public:
    CallInfo* pCallInfo;
    MediaInfo* pMediaInfo;
    IMSMap<SuppType, SuppService*> objSuppServices;
};


class IUUCSessionResumeFailedParam : public IUUCSessionFailedParam
{
public:
    inline IUUCSessionResumeFailedParam() :
            IUUCSessionFailedParam()
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionResumeFailedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionResumeFailedParam), this, 0);
    }
    inline virtual ~IUUCSessionResumeFailedParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionResumeFailedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionResumeFailedParam), this, 0);
    }

public:
};


class IUUCSessionResumeByParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionResumeByParam() :
            IUUCSessionBaseParam(),
            pCallInfo(IMS_NULL),
            pMediaInfo(IMS_NULL),
            objSuppServices(IMSMap<SuppType, SuppService*>())
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionResumeByParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionResumeByParam), this, 0);
    }
    inline virtual ~IUUCSessionResumeByParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionResumeByParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionResumeByParam), this, 0);

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
            SuppService* pSS = objSuppServices.GetValueAt(i);
            if (pSS != IMS_NULL)
            {
                delete pSS;
            }
        }
        objSuppServices.Clear();
    }

private:
    IUUCSessionResumeByParam(IN CONST IUUCSessionResumeByParam& objRHS);
    IUUCSessionResumeByParam& operator=(IN CONST IUUCSessionResumeByParam& objRHS);

public:
    CallInfo* pCallInfo;
    MediaInfo* pMediaInfo;
    IMSMap<SuppType, SuppService*> objSuppServices;
};


class IUUCSessionIncomingResumeParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionIncomingResumeParam() :
            IUUCSessionBaseParam(),
            pCallInfo(IMS_NULL),
            pMediaInfo(IMS_NULL),
            objSuppServices(IMSMap<SuppType, SuppService*>())
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionIncomingResumeParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionIncomingResumeParam), this, 0);
    }
    inline virtual ~IUUCSessionIncomingResumeParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionIncomingResumeParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionIncomingResumeParam), this, 0);

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
            SuppService* pSS = objSuppServices.GetValueAt(i);
            if (pSS != IMS_NULL)
            {
                delete pSS;
            }
        }
        objSuppServices.Clear();
    }

private:
    IUUCSessionIncomingResumeParam(IN CONST IUUCSessionIncomingResumeParam& objRHS);
    IUUCSessionIncomingResumeParam& operator=(IN CONST IUUCSessionIncomingResumeParam& objRHS);

public:
    CallInfo* pCallInfo;
    MediaInfo* pMediaInfo;
    IMSMap<SuppType, SuppService*> objSuppServices;
};


class IUUCSessionTerminatedParam : public IUUCSessionFailedParam
{
public:
    inline IUUCSessionTerminatedParam() :
            IUUCSessionFailedParam()
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionTerminatedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionTerminatedParam), this, 0);
    }
    inline virtual ~IUUCSessionTerminatedParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionTerminatedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionTerminatedParam), this, 0);
    }

public:
};


class IUUCSessionImcomingUpdateParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionImcomingUpdateParam() :
            IUUCSessionBaseParam(),
            pCallInfo(IMS_NULL),
            pMediaInfo(IMS_NULL),
            objSuppServices(IMSMap<SuppType, SuppService*>())
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionImcomingUpdateParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionImcomingUpdateParam), this, 0);
    }
    inline virtual ~IUUCSessionImcomingUpdateParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionImcomingUpdateParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionImcomingUpdateParam), this, 0);

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
            SuppService* pSS = objSuppServices.GetValueAt(i);
            if (pSS != IMS_NULL)
            {
                delete pSS;
            }
        }
        objSuppServices.Clear();
    }

private:
    IUUCSessionImcomingUpdateParam(IN CONST IUUCSessionImcomingUpdateParam& objRHS);
    IUUCSessionImcomingUpdateParam& operator=(IN CONST IUUCSessionImcomingUpdateParam& objRHS);

public:
    CallInfo* pCallInfo;
    MediaInfo* pMediaInfo;
    IMSMap<SuppType, SuppService*> objSuppServices;
};


class IUUCSessionUpdatedParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionUpdatedParam() :
            IUUCSessionBaseParam(),
            pCallInfo(IMS_NULL),
            pMediaInfo(IMS_NULL),
            objSuppServices(IMSMap<SuppType, SuppService*>())
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionUpdatedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionUpdatedParam), this, 0);
    }
    inline virtual ~IUUCSessionUpdatedParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionUpdatedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionUpdatedParam), this, 0);

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
            SuppService* pSS = objSuppServices.GetValueAt(i);
            if (pSS != IMS_NULL)
            {
                delete pSS;
            }
        }
        objSuppServices.Clear();
    }

private:
    IUUCSessionUpdatedParam(IN CONST IUUCSessionUpdatedParam& objRHS);
    IUUCSessionUpdatedParam& operator=(IN CONST IUUCSessionUpdatedParam& objRHS);

public:
    CallInfo* pCallInfo;
    MediaInfo* pMediaInfo;
    IMSMap<SuppType, SuppService*> objSuppServices;
};


class IUUCSessionUpdateFailedParam : public IUUCSessionFailedParam
{
public:
    inline IUUCSessionUpdateFailedParam() :
            IUUCSessionFailedParam()
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionUpdateFailedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionUpdateFailedParam), this, 0);
    }
    inline virtual ~IUUCSessionUpdateFailedParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionUpdateFailedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionUpdateFailedParam), this, 0);
    }

public:
};


class IUUCSessionUpdatedByParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionUpdatedByParam() :
            IUUCSessionBaseParam(),
            pCallInfo(IMS_NULL),
            pMediaInfo(IMS_NULL),
            objSuppServices(IMSMap<SuppType, SuppService*>())
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionUpdatedByParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionUpdatedByParam), this, 0);
    }
    inline virtual ~IUUCSessionUpdatedByParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionUpdatedByParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionUpdatedByParam), this, 0);

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
            SuppService* pSS = objSuppServices.GetValueAt(i);

            if (pSS != IMS_NULL)
            {
                delete pSS;
            }
        }
        objSuppServices.Clear();
    }

private:
    IUUCSessionUpdatedByParam(IN CONST IUUCSessionUpdatedByParam& objRHS);
    IUUCSessionUpdatedByParam& operator=(IN CONST IUUCSessionUpdatedByParam& objRHS);

public:
    CallInfo* pCallInfo;
    MediaInfo* pMediaInfo;
    IMSMap<SuppType, SuppService*> objSuppServices;
};


class IUUCSessionNotifyRemoteMediaParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionNotifyRemoteMediaParam() :
            IUUCSessionBaseParam(),
            strConnection(AString::ConstNull())
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionNotifyRemoteMediaParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionNotifyRemoteMediaParam), this, 0);
    }
    inline virtual ~IUUCSessionNotifyRemoteMediaParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionNotifyRemoteMediaParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionNotifyRemoteMediaParam), this, 0);
    }

public:
    AString strConnection;
};

class IUUCSessionNotifyInfoParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionNotifyInfoParam() :
            IUUCSessionBaseParam(),
            eType(0),
            aStrValue(AString::ConstNull()),
            nValue(-1),
            bValue(IMS_FALSE)

    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionNotifyInfoParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionNotifyInfoParam), this, 0);
    }
    inline virtual ~IUUCSessionNotifyInfoParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionNotifyInfoParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionNotifyInfoParam), this, 0);
    }

public:
    IMS_UINT32 eType;

    AString aStrValue;
    IMS_SINT32 nValue;
    IMS_BOOL bValue;
};


class IUUCSessionConfMergedParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionConfMergedParam() :
            IUUCSessionBaseParam(),
            pCallInfo(IMS_NULL),
            pMediaInfo(IMS_NULL),
            objSuppServices(IMSMap<SuppType, SuppService*>()),
            lstConfUsers(IMSList<ConfUser*>())
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionConfMergedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionConfMergedParam), this, 0);
    }
    inline virtual ~IUUCSessionConfMergedParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionConfMergedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionConfMergedParam), this, 0);

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
            SuppService* pSS = objSuppServices.GetValueAt(i);
            if (pSS != IMS_NULL)
            {
                delete pSS;
            }
        }
        objSuppServices.Clear();

        for (IMS_UINT32 i = 0; i < lstConfUsers.GetSize(); ++i)
        {
            ConfUser* pUser = lstConfUsers.GetAt(i);
            if (pUser != IMS_NULL)
            {
                delete pUser;
            }
        }
        lstConfUsers.Clear();
    }

private:
    IUUCSessionConfMergedParam(IN CONST IUUCSessionConfMergedParam& objRHS);
    IUUCSessionConfMergedParam& operator=(IN CONST IUUCSessionConfMergedParam& objRHS);

public:
    CallInfo* pCallInfo;
    MediaInfo* pMediaInfo;
    IMSMap<SuppType, SuppService*> objSuppServices;
    IMSList<ConfUser*> lstConfUsers;
};


class IUUCSessionConfMergeFailedParam : public IUUCSessionFailedParam
{
public:
    inline IUUCSessionConfMergeFailedParam() :
            IUUCSessionFailedParam()
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionConfMergeFailedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionConfMergeFailedParam), this, 0);
    }
    inline virtual ~IUUCSessionConfMergeFailedParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionConfMergeFailedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionConfMergeFailedParam), this, 0);
    }

public:
};


class IUUCSessionConfExpandedParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionConfExpandedParam() :
            IUUCSessionBaseParam(),
            pCallInfo(IMS_NULL),
            pMediaInfo(IMS_NULL),
            objSuppServices(IMSMap<SuppType, SuppService*>())
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionConfExpandedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionConfExpandedParam), this, 0);
    }
    inline virtual ~IUUCSessionConfExpandedParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionConfExpandedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionConfExpandedParam), this, 0);

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
            SuppService* pSS = objSuppServices.GetValueAt(i);
            if (pSS != IMS_NULL)
            {
                delete pSS;
            }
        }
        objSuppServices.Clear();
    }

private:
    IUUCSessionConfExpandedParam(IN CONST IUUCSessionConfExpandedParam& objRHS);
    IUUCSessionConfExpandedParam& operator=(IN CONST IUUCSessionConfExpandedParam& objRHS);

public:
    CallInfo* pCallInfo;
    MediaInfo* pMediaInfo;
    IMSMap<SuppType, SuppService*> objSuppServices;
};


class IUUCSessionConfExpandFailedParam : public IUUCSessionFailedParam
{
public:
    inline IUUCSessionConfExpandFailedParam() :
            IUUCSessionFailedParam()
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionConfExpandFailedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionConfExpandFailedParam), this, 0);
    }
    inline virtual ~IUUCSessionConfExpandFailedParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionConfExpandFailedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionConfExpandFailedParam), this, 0);
    }

public:
};


class IUUCSessionConfExpandedByParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionConfExpandedByParam() :
            IUUCSessionBaseParam(),
            pCallInfo(IMS_NULL),
            pMediaInfo(IMS_NULL),
            objSuppServices(IMSMap<SuppType, SuppService*>()),
            nReplaceKey(0)
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionConfExpandedByParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionConfExpandedByParam), this, 0);
    }
    inline virtual ~IUUCSessionConfExpandedByParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionConfExpandedByParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionConfExpandedByParam), this, 0);

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
            SuppService* pSS = objSuppServices.GetValueAt(i);
            if (pSS != IMS_NULL)
            {
                delete pSS;
            }
        }
        objSuppServices.Clear();
    }

private:
    IUUCSessionConfExpandedByParam(IN CONST IUUCSessionConfExpandedByParam& objRHS);
    IUUCSessionConfExpandedByParam& operator=(IN CONST IUUCSessionConfExpandedByParam& objRHS);

public:
    CallInfo* pCallInfo;
    MediaInfo* pMediaInfo;
    IMSMap<SuppType, SuppService*> objSuppServices;

    IMS_SINTP nReplaceKey;
};


class IUUCSessionConfJoinedParam : public IUUCSessionFailedParam
{
public:
    inline IUUCSessionConfJoinedParam() :
            IUUCSessionFailedParam(),
            bResult(IMS_TRUE)
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionConfJoinedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionConfJoinedParam), this, 0);
    }
    inline virtual ~IUUCSessionConfJoinedParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionConfJoinedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionConfJoinedParam), this, 0);
    }

public:
    IMS_BOOL bResult;
};


class IUUCSessionConfDroppedParam : public IUUCSessionFailedParam
{
public:
    inline IUUCSessionConfDroppedParam() :
            IUUCSessionFailedParam(),
            bResult(IMS_TRUE)
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionConfDroppedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionConfDroppedParam), this, 0);
    }
    inline virtual ~IUUCSessionConfDroppedParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionConfDroppedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionConfDroppedParam), this, 0);
    }

public:
    IMS_BOOL bResult;
};


class IUUCSessionConfDeletedParam : public IUUCSessionFailedParam
{
public:
    inline IUUCSessionConfDeletedParam() :
            IUUCSessionFailedParam(),
            bResult(IMS_TRUE)
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionConfDeletedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionConfDeletedParam), this, 0);
    }
    inline virtual ~IUUCSessionConfDeletedParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionConfDeletedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionConfDeletedParam), this, 0);
    }

public:
    IMS_BOOL bResult;
};


class IUUCSessionConfNotifyUsersInfoParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionConfNotifyUsersInfoParam() :
            IUUCSessionBaseParam(),
            objUsers(IMSList<ConfUser*>())
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionConfNotifyUsersInfoParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionConfNotifyUsersInfoParam), this, 0);
    }
    inline virtual ~IUUCSessionConfNotifyUsersInfoParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionConfNotifyUsersInfoParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionConfNotifyUsersInfoParam), this, 0);

        for (IMS_UINT32 i = 0; i < objUsers.GetSize(); ++i)
        {
            ConfUser* pUser = objUsers.GetAt(i);
            if (pUser != IMS_NULL)
            {
                delete pUser;
            }
        }
        objUsers.Clear();
    }

public:
    IMSList<ConfUser*> objUsers;
};


class IUUCSessionConfNotifyConfInfoParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionConfNotifyConfInfoParam() :
            IUUCSessionBaseParam(),
            aStrDisplayText(AString::ConstNull()),
            aStrSubject(AString::ConstNull()),
            nMaxUserCount(-1),
            nUserCount(0),
            aStrHostEntity(AString::ConstNull())
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionConfNotifyConfInfoParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionConfNotifyConfInfoParam), this, 0);
    }
    inline virtual ~IUUCSessionConfNotifyConfInfoParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionConfNotifyConfInfoParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionConfNotifyConfInfoParam), this, 0);
    }

public:
    AString aStrDisplayText;
    AString aStrSubject;
    IMS_SINT32 nMaxUserCount;

    IMS_UINT32 nUserCount;
    AString aStrHostEntity;
};


class IUUCSessionECTCompletedParam : public IUUCSessionFailedParam
{
public:
    inline IUUCSessionECTCompletedParam() :
            IUUCSessionFailedParam(),
            bResult(IMS_FALSE)
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionECTCompletedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionECTCompletedParam), this, 0);
    }
    inline virtual ~IUUCSessionECTCompletedParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionECTCompletedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionECTCompletedParam), this, 0);
    }

public:
    IMS_BOOL bResult;
};


class IUUCSessionECTStartParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionECTStartParam() :
            IUUCSessionBaseParam(),
            pCallInfo(IMS_NULL)
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionECTStartParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionECTStartParam), this, 0);
    }
    inline virtual ~IUUCSessionECTStartParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionECTStartParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionECTStartParam), this, 0);

        if (pCallInfo != IMS_NULL)
        {
            delete pCallInfo;
        }
    }

private:
    IUUCSessionECTStartParam(IN CONST IUUCSessionECTStartParam& objRHS);
    IUUCSessionECTStartParam& operator=(IN CONST IUUCSessionECTStartParam& objRHS);

public:
    CallInfo* pCallInfo;
};


class IUUCSessionECTStartBlindParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionECTStartBlindParam() :
            IUUCSessionBaseParam(),
            pCallInfo(IMS_NULL),
            aStrTarget(AString::ConstNull())
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionECTStartBlindParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionECTStartBlindParam), this, 0);
    }
    inline virtual ~IUUCSessionECTStartBlindParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionECTStartBlindParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionECTStartBlindParam), this, 0);

        if (pCallInfo != IMS_NULL)
        {
            delete pCallInfo;
        }
    }

private:
    IUUCSessionECTStartBlindParam(IN CONST IUUCSessionECTStartBlindParam& objRHS);
    IUUCSessionECTStartBlindParam& operator=(IN CONST IUUCSessionECTStartBlindParam& objRHS);

public:
    CallInfo* pCallInfo;
    AString aStrTarget;
};


class IUUCSessionECTStartedParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionECTStartedParam() :
            IUUCSessionBaseParam(),
            pCallInfo(IMS_NULL),
            pMediaInfo(IMS_NULL),
            objSuppServices(IMSMap<SuppType, SuppService*>()),
            nReplaceKey(0),
            nType(0)
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionECTStartedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionECTStartedParam), this, 0);
    }
    inline virtual ~IUUCSessionECTStartedParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionECTStartedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionECTStartedParam), this, 0);

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
            SuppService* pSS = objSuppServices.GetValueAt(i);
            if (pSS != IMS_NULL)
            {
                delete pSS;
            }
        }
        objSuppServices.Clear();
    }

private:
    IUUCSessionECTStartedParam(IN CONST IUUCSessionECTStartedParam& objRHS);
    IUUCSessionECTStartedParam& operator=(IN CONST IUUCSessionECTStartedParam& objRHS);

public:
    CallInfo* pCallInfo;
    MediaInfo* pMediaInfo;
    IMSMap<SuppType, SuppService*> objSuppServices;
    IMS_SINTP nReplaceKey;
    IMS_UINTP nType;
};


class IUUCSessionCallPushCompletedParam : public IUUCSessionFailedParam
{
public:
    inline IUUCSessionCallPushCompletedParam() :
            IUUCSessionFailedParam(),
            bResult(IMS_FALSE)
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionCallPushCompletedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionCallPushCompletedParam), this, 0);
    }
    inline virtual ~IUUCSessionCallPushCompletedParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionCallPushCompletedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionCallPushCompletedParam), this, 0);
    }

public:
    IMS_BOOL bResult;
};


class IUUCSessionCallPushParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionCallPushParam() :
            IUUCSessionBaseParam(),
            strTargetDevice(AString::ConstNull())
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionCallPushParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionCallPushParam), this, 0);
    }
    inline virtual ~IUUCSessionCallPushParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionCallPushParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionCallPushParam), this, 0);
    }

public:
    AString strTargetDevice;
};


class IUUCSessionCancelCallPushParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionCancelCallPushParam() :
            IUUCSessionBaseParam()
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionCancelCallPushParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionCancelCallPushParam), this, 0);
    }
    inline virtual ~IUUCSessionCancelCallPushParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionCancelCallPushParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionCancelCallPushParam), this, 0);
    }

public:
};


class IUUCSessionCodecInfoUpdateParam : public IUUCSessionBaseParam
{
public:
    inline IUUCSessionCodecInfoUpdateParam() :
            IUUCSessionBaseParam(),
            aStrCodecQuality(AString::ConstNull()),
            aStrCodecBandwidth(AString::ConstNull()),
            aStrCodecBitrate(AString::ConstNull())
    {
        IMS_TRACE_MEM("uc", "uc_M : IUUCSessionCodecInfoUpdateParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionCodecInfoUpdateParam), this, 0);
    }
    inline virtual ~IUUCSessionCodecInfoUpdateParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUUCSessionCodecInfoUpdateParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUUCSessionCodecInfoUpdateParam), this, 0);
    }

public:
    AString aStrCodecQuality;
    AString aStrCodecBandwidth;
    AString aStrCodecBitrate;
};

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
inline const IMS_CHAR* UCStrSessionEvtU2I(IN IMS_SINT32 eEvent)
{
    switch (eEvent)
    {
        case IuMtcCall::START:
            return "START";
        case IuMtcCall::STARTCONF:
            return "STARTCONF";
        case IuMtcCall::USER_ALERT:
            return "USER_ALERT";
        case IuMtcCall::ACCEPT:
            return "ACCEPT";
        case IuMtcCall::REJECT:
            return "REJECT";
        case IuMtcCall::HOLD:
            return "HOLD";
        case IuMtcCall::RESUME:
            return "RESUME";
        case IuMtcCall::SEND_DTMF:
            return "SEND_DTMF";
        case IuMtcCall::TERMINATE:
            return "TERMINATE";
        case IuMtcCall::UPDATE:
            return "UPDATE";
        case IuMtcCall::ACCEPT_UPDATE:
            return "ACCEPT_UPDATE";
        case IuMtcCall::REJECT_UPDATE:
            return "REJECT_UPDATE";
        case IuMtcCall::CANCEL_UPDATE:
            return "CANCEL_UPDATE";
        case IuMtcCall::ACCEPT_RESUME:
            return "ACCEPT_RESUME";
        case IuMtcCall::REJECT_RESUME:
            return "REJECT_RESUME";
        case IuMtcCall::SEND_USSD:
            return "SEND_USSD";
        case IuMtcCall::CONF_EXPAND:
            return "CONF_EXPAND";
        case IuMtcCall::CONF_MERGE:
            return "CONF_MERGE";
        case IuMtcCall::CONF_JOIN:
            return "CONF_JOIN";
        case IuMtcCall::CONF_DROP:
            return "CONF_DROP";
        case IuMtcCall::CONF_DELETE:
            return "CONF_DELETE";
        case IuMtcCall::ATTACH:
            return "ATTACH";
        case IuMtcCall::PUSH_CALL:
            return "PUSH_CALL";
        case IuMtcCall::CANCEL_CALL_PUSH:
            return "CANCEL_CALL_PUSH";
        case IuMtcCall::ECT_START:
            return "ECT_START";
        case IuMtcCall::ECT_START_BLIND:
            return "ECT_START_BLIND";

        default:
            return "__INVALID__";
    }
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
inline const IMS_CHAR* UCStrSessionEvtI2U(IN IMS_SINT32 eEvent)
{
    switch (eEvent)
    {
        case IuMtcCall::STARTED:
            return "STARTED";
        case IuMtcCall::START_FAILED:
            return "START_FAILED";
        case IuMtcCall::PROGRESSING:
            return "PROGRESSING";
        case IuMtcCall::HELD:
            return "HELD";
        case IuMtcCall::HOLD_FAILED:
            return "HOLD_FAILED";
        case IuMtcCall::HELD_BY:
            return "HELD_BY";
        case IuMtcCall::RESUMED:
            return "RESUMED";
        case IuMtcCall::RESUME_FAILED:
            return "RESUME_FAILED";
        case IuMtcCall::RESUMED_BY:
            return "RESUMED_BY";
        case IuMtcCall::TERMINATED:
            return "TERMINATED";
        case IuMtcCall::INCOMING_UPDATE:
            return "INCOMING_UPDATE";
        case IuMtcCall::UPDATED:
            return "UPDATED";
        case IuMtcCall::UPDATE_FAILED:
            return "UPDATE_FAILED";
        case IuMtcCall::UPDATED_BY:
            return "UPDATED_BY";
        case IuMtcCall::NOTIFY_INFO:
            return "NOTIFY_INFO";
        case IuMtcCall::INCOMING_RESUME:
            return "INCOMING_RESUME";
        case IuMtcCall::SET_PROPERTY:
            return "SET_PROPERTY";
        case IuMtcCall::CONF_EXPANDED:
            return "CONF_EXPANDED";
        case IuMtcCall::CONF_EXPANDFAILED:
            return "CONF_EXPANDFAILED";
        case IuMtcCall::CONF_EXPANDED_BY:
            return "CONF_EXPANDED_BY";
        case IuMtcCall::CONF_MERGED:
            return "CONF_MERGED";
        case IuMtcCall::CONF_MERGEFAILED:
            return "CONF_MERGEFAILED";
        case IuMtcCall::CONF_JOINED:
            return "CONF_JOINED";
        case IuMtcCall::CONF_DROPPED:
            return "CONF_DROPPED";
        case IuMtcCall::CONF_DELETED:
            return "CONF_DELETED";
        case IuMtcCall::CONF_NOTIFY_USERS_INFO:
            return "CONF_NOTIFY_USERS_INFO";
        case IuMtcCall::CONF_NOTIFY_CONF_INFO:
            return "CONF_NOTIFY_CONF_INFO";
        case IuMtcCall::CALL_PUSH_COMPLETED:
            return "CALL_PUSH_COMPLETED";
        case IuMtcCall::ECT_COMPLETED:
            return "ECT_COMPLETED";
        case IuMtcCall::REPLACED_BY:
            return "REPLACED_BY";
        case IuMtcCall::CODEC_INFO_UPDATED:
            return "CODEC_INFO_UPDATED";

        default:
            return "__INVALID__";
    }
}
#endif  // INTERFACE_UI_MTC_CALL_H_
