/*
 * author : hyunbin.shin@
 * version : 2.0
 * date : 201503
 * brief :
 */

#ifndef INTERFACE_UC_MEDIA_LISTENER_H_
#define INTERFACE_UC_MEDIA_LISTENER_H_

#include "ServiceTrace.h"
#include "IUMedia.h"
#include "MtcDef.h"

class IUCMediaListener
{
public:
    virtual void Media_Success(IN IMS_UINTP nParam) = 0;
    virtual void Media_Failed(IN IMS_UINTP nParam) = 0;
    virtual void Media_Notify(IN IMS_UINTP nParam) = 0;
    virtual void Media_NotifyInfo(IN IMS_UINTP nParam) = 0;
    virtual void Media_Notify_RtpInfo(IN IMS_UINTP nParam) = 0;
    virtual void Media_Notify_DraInfo(IN IMS_UINTP nParam) = 0;

public:
};

class IUCMediaSuccessParam
{
public:
    inline IUCMediaSuccessParam() :
            eReportType(MEDIA_REPORT_INVALID),
            eMediaType(MEDIATYPE_NONE)
    {
        IMS_TRACE_MEM("uc", "uc_M : IUCMediaSuccessParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUCMediaSuccessParam), this, 0);
    }
    inline virtual ~IUCMediaSuccessParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUCMediaSuccessParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUCMediaSuccessParam), this, 0);
    }

private:
    IUCMediaSuccessParam(IN const IUCMediaSuccessParam& objRHS);
    IUCMediaSuccessParam& operator=(IN const IUCMediaSuccessParam& objRHS);

public:
    IMS_SINT32 eReportType;
    IMS_UINT32 eMediaType;
};

class IUCMediaNotifyParam
{
public:
    inline IUCMediaNotifyParam() :
            eReportType(MEDIA_REPORT_INVALID),
            eMediaType(MEDIATYPE_NONE)
    {
        IMS_TRACE_MEM("uc", "uc_M : IUCMediaNotifyParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUCMediaNotifyParam), this, 0);
    }
    inline virtual ~IUCMediaNotifyParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUCMediaNotifyParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUCMediaNotifyParam), this, 0);
    }

private:
    IUCMediaNotifyParam(IN const IUCMediaNotifyParam& objRHS);
    IUCMediaNotifyParam& operator=(IN const IUCMediaNotifyParam& objRHS);

public:
    IMS_SINT32 eReportType;
    IMS_UINT32 eMediaType;
};

class IUCMediaNotifyInfoParam
{
public:
    inline IUCMediaNotifyInfoParam() :
            eReportType(MEDIA_REPORT_INVALID),
            aStrValue(AString::ConstNull()),
            nValue(-1),
            bValue(IMS_FALSE)
    {
        IMS_TRACE_MEM("uc", "uc_M : IUCMediaNotifyInfoParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUCMediaNotifyInfoParam), this, 0);
    }
    inline IUCMediaNotifyInfoParam(IN const IUCMediaNotifyInfoParam& objRHS) :
            eReportType(objRHS.eReportType),
            aStrValue(objRHS.aStrValue),
            nValue(objRHS.nValue),
            bValue(objRHS.bValue)
    {
        IMS_TRACE_MEM("uc", "uc_M : IUCMediaNotifyInfoParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUCMediaNotifyInfoParam), this, 0);
    }
    inline ~IUCMediaNotifyInfoParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUCMediaNotifyInfoParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUCMediaNotifyInfoParam), this, 0);
    }

public:
    inline IUCMediaNotifyInfoParam& operator=(IN const IUCMediaNotifyInfoParam& objRHS)
    {
        if (this != &objRHS)
        {
            eReportType = objRHS.eReportType;
            aStrValue = objRHS.aStrValue;
            nValue = objRHS.nValue;
            bValue = objRHS.bValue;
        }

        return (*this);
    }

public:
    IMS_SINT32 eReportType;

    AString aStrValue;
    IMS_SINT32 nValue;
    IMS_BOOL bValue;
};

class IUCMediaNotifyRtpInfoParam
{
public:
    inline IUCMediaNotifyRtpInfoParam() :
            pRtpInfo(IMS_NULL)
    {
        IMS_TRACE_MEM("uc", "uc_M : IUCMediaNotifyRtpInfoParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUCMediaNotifyRtpInfoParam), this, 0);
    }
    inline virtual ~IUCMediaNotifyRtpInfoParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUCMediaNotifyRtpInfoParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUCMediaNotifyRtpInfoParam), this, 0);
    }

private:
    IUCMediaNotifyRtpInfoParam(IN const IUCMediaNotifyRtpInfoParam& objRHS);
    IUCMediaNotifyRtpInfoParam& operator=(IN const IUCMediaNotifyRtpInfoParam& objRHS);

public:
    IMediaRTPInfoMsgParam* pRtpInfo;
};

class IUCMediaNotifyDraInfoParam
{
public:
    inline IUCMediaNotifyDraInfoParam() :
            pDraInfo(IMS_NULL)
    {
        IMS_TRACE_MEM("uc", "uc_M : IUCMediaNotifyDraInfoParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUCMediaNotifyDraInfoParam), this, 0);
    }
    inline virtual ~IUCMediaNotifyDraInfoParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IUCMediaNotifyDraInfoParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IUCMediaNotifyDraInfoParam), this, 0);
    }

private:
    IUCMediaNotifyDraInfoParam(IN const IUCMediaNotifyDraInfoParam& objRHS);
    IUCMediaNotifyDraInfoParam& operator=(IN const IUCMediaNotifyDraInfoParam& objRHS);

public:
    IMediaDRAMsgParam* pDraInfo;
};
#endif /*  INTERFACE_UC_MEDIA_LISTENER_H_ */
