#ifndef MTC_INTERNAL_MSG_DEF_H_
#define MTC_INTERNAL_MSG_DEF_H_

#include "ImsMessageDef.h"
#include "IMSTypeDef.h"
#include "CallInfo.h"
#include "IMtcService.h"
#include "MtcDef.h"
#include "ServiceTrace.h"

class ICoreService;
class IMtcService;

class MTC_INTERNAL_MSG
{
public:
    static const IMS_SINT32 MTC_DEFAULT_MSG                    = (IMS_MSG_MTC + 0);

    /* From App To Service */

    /* From App To SessMngr */
    static const IMS_SINT32 TERMINATE_SESSION_CMD           = (IMS_MSG_MTC + 1);
    static const IMS_SINT32 REMOVE_SESSION_CMD              = (IMS_MSG_MTC + 2);
    static const IMS_SINT32 UPDATE_SESSION_SERVICE_CMD      = (IMS_MSG_MTC + 3);
    static const IMS_SINT32 BLOCK_SESSION_CMD               = (IMS_MSG_MTC + 4);

    static const IMS_SINT32 MTC_MAX_MSG                      = (IMS_MSG_MTC + 99);

    /* From SessMngr To Session */
    static const IMS_SINT32 APP_MSG_BASE                        = (IMS_MSG_MTC + 100);
    static const IMS_SINT32 SERVICE_MSG_BASE                    = (IMS_MSG_MTC + 200);
    static const IMS_SINT32 SESSMNGR_MSG_BASE                   = (IMS_MSG_MTC + 300);
    static const IMS_SINT32 SESSION_MSG_BASE                    = (IMS_MSG_MTC + 400);
    static const IMS_SINT32 EARLY_SESSION_MSG_BASE              = (IMS_MSG_MTC + 600);
    static const IMS_SINT32 CONFIRMED_SESSION_MSG_BASE          = (IMS_MSG_MTC + 800);
    static const IMS_SINT32 SESSUPDATE_MSG_BASE                 = (IMS_MSG_MTC + 1000);
    static const IMS_SINT32 SESSHR_MSG_BASE                     = (IMS_MSG_MTC + 1200);
    static const IMS_SINT32 PRECONDITION_MSG_BASE               = (IMS_MSG_MTC + 1400);
    static const IMS_SINT32 MEDIA_MSG_BASE                      = (IMS_MSG_MTC + 1500);
    static const IMS_SINT32 CONFMNGR_MSG_BASE                   = (IMS_MSG_MTC + 1600);
    static const IMS_SINT32 REFER_MSG_BASE                      = (IMS_MSG_MTC + 1700);
    static const IMS_SINT32 CONFSUBMNGR_MSG_BASE                = (IMS_MSG_MTC + 1800);
    static const IMS_SINT32 CONFSUB_MSG_BASE                    = (IMS_MSG_MTC + 1900);
    static const IMS_SINT32 SRVCC_MSG_BASE                      = (IMS_MSG_MTC + 2000);
    static const IMS_SINT32 DIALOG_MSG_BASE                     = (IMS_MSG_MTC + 2100);
    static const IMS_SINT32 BLOCK_MSG_BASE                      = (IMS_MSG_MTC + 2200);
    static const IMS_SINT32 CALLTRACKER_MSG_BASE                = (IMS_MSG_MTC + 2300);
    static const IMS_SINT32 ECT_MSG_BASE                        = (IMS_MSG_MTC + 2400);
    static const IMS_SINT32 SERVICETRACKER_MSG_BASE             = (IMS_MSG_MTC + 2500);
    static const IMS_SINT32 ANALYZER_MSG_BASE                   = (IMS_MSG_MTC + 2600);
    static const IMS_SINT32 TRM_MSG_BASE                        = (IMS_MSG_MTC + 2700);

    enum
    {
        KEYTYPE_NONE                    = 0,
        KEYTYPE_IMS                     = 1,
        KEYTYPE_MTC                     = 2,
        KEYTYPE_ICORE                   = 3,
        KEYTYPE_CALLTYPE                = 4,
        KEYTYPE_SERVICETYPE             = 5,
    };

public:
    inline static IMS_BOOL IsMsg(IN IMS_SINT32 nMsg)
    { return ((nMsg > MTC_DEFAULT_MSG) && (nMsg < MTC_MAX_MSG)); }


};


class UCInterMsgTerminateSessionCmdParam
{
public:
    inline UCInterMsgTerminateSessionCmdParam()
        : eKeyType(MTC_INTERNAL_MSG::KEYTYPE_NONE)
        , nReason(0)
        , bDirect(IMS_FALSE)
        {
            IMS_TRACE_MEM("mtc", "mtc_M : UCInterMsgTerminateSessionCmdParam[%" PFLS_u "]" \
                    "[%" PFLS_x "]", sizeof(UCInterMsgTerminateSessionCmdParam), this, 0);
        }
        inline virtual ~UCInterMsgTerminateSessionCmdParam()
        {
            IMS_TRACE_MEM("mtc", "mtc_F : UCInterMsgTerminateSessionCmdParam[%" PFLS_u "]" \
                    "[%" PFLS_x "]", sizeof(UCInterMsgTerminateSessionCmdParam), this, 0);
        }

public :

    IMS_SINT32                eKeyType; // KEYTEYPE_

    union
    {
        IMS_UINTP           nIMSKey;
        IMS_SINTP           nSessionKey;
        ICoreService*       pICoreService;
        CallType            eCallType;
        ServiceType         eServiceType;
    } key;

    IMS_UINT32      nReason;
    IMS_BOOL        bDirect;                // Recommend :: IMS_FALSE, IMS_TRUE is Special Case.
};

class UCInterMsgRemoveSessionCmdParam
{
public:
    inline UCInterMsgRemoveSessionCmdParam()
        : eKeyType(MTC_INTERNAL_MSG::KEYTYPE_NONE)
        , nReason(0)

        {
            IMS_TRACE_MEM("mtc", "mtc_M : UCInterMsgRemoveSessionCmdParam[%" PFLS_u "][%" PFLS_x "]",
                    sizeof(UCInterMsgRemoveSessionCmdParam), this, 0);
        }
        inline virtual ~UCInterMsgRemoveSessionCmdParam()
        {
            IMS_TRACE_MEM("mtc", "mtc_F : UCInterMsgRemoveSessionCmdParam[%" PFLS_u "][%" PFLS_x "]",
                    sizeof(UCInterMsgRemoveSessionCmdParam), this, 0);
        }

public :

    IMS_SINT32                eKeyType; // KEYTEYPE_

    union
    {
        IMS_UINTP           nIMSKey;
        IMS_SINTP           nSessionKey;
        ICoreService*       pICoreService;
        CallType            eCallType;
        ServiceType         eServiceType;
    } key;

    IMS_UINT32      nReason;
};

class UCInterMsgUpdateSessionServiceCmdParam
{
public:
    inline UCInterMsgUpdateSessionServiceCmdParam()
        : eKeyType(MTC_INTERNAL_MSG::KEYTYPE_NONE)
        , pService(IMS_NULL)
        , eServiceType(ServiceType::UNKNOWN)
        {
            IMS_TRACE_MEM("mtc", "mtc_M : UCInterMsgUpdateSessionServiceCmdParam[%" PFLS_u "]" \
                    "[%" PFLS_x "]", sizeof(UCInterMsgUpdateSessionServiceCmdParam), this, 0);
        }
        inline virtual ~UCInterMsgUpdateSessionServiceCmdParam()
        {
            IMS_TRACE_MEM("mtc", "mtc_F : UCInterMsgUpdateSessionServiceCmdParam[%" PFLS_u "]" \
                    "[%" PFLS_x "]", sizeof(UCInterMsgUpdateSessionServiceCmdParam), this, 0);
        }

public :

    IMS_SINT32                eKeyType; // KEYTEYPE_

    union
    {
        IMS_UINTP           nIMSKey;
        IMS_SINTP           nSessionKey;
        ICoreService*       pICoreService;
        IMS_SINT32          eSessionType;
        ServiceType         eServiceType;
    } key;

    IMtcService*     pService;
    ServiceType      eServiceType;
};

class UCInterMsgBlockSessionCmdParam
{
public:
    inline UCInterMsgBlockSessionCmdParam()
        : eKeyType(MTC_INTERNAL_MSG::KEYTYPE_NONE)
        , nBlockFeatures(BLOCK_FEATURE_NONE)
        {
            IMS_TRACE_MEM("mtc", "mtc_M : UCInterMsgBlockSessionCmdParam[%" PFLS_u "][%" PFLS_x "]",
                    sizeof(UCInterMsgBlockSessionCmdParam), this, 0);
        }
        inline virtual ~UCInterMsgBlockSessionCmdParam()
        {
            IMS_TRACE_MEM("mtc", "mtc_F : UCInterMsgBlockSessionCmdParam[%" PFLS_u "][%" PFLS_x "]",
                    sizeof(UCInterMsgBlockSessionCmdParam), this, 0);
        }

public :

    IMS_SINT32              eKeyType; // KEYTEYPE_

    union
    {
        IMS_UINTP           nIMSKey;
        IMS_SINTP           nSessionKey;
        ICoreService*       pICoreService;
        IMS_SINT32          eSessionType;
        IMS_UINT32          eServiceType;
    } key;

    IMS_UINT32      nBlockFeatures;
};

#endif
