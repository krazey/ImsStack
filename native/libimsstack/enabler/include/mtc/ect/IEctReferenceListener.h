/*
 * author : dongo.yi@
 * version : 1.0
 * date : 20150907
 * brief :
 */

#ifndef INTERFACE_UC_ECT_REFERENCE_LISTENER_H_
#define INTERFACE_UC_ECT_REFERENCE_LISTENER_H_

#include "ServiceTrace.h"

class UCECTReference;

class IUCECTReferenceListener
{
public:
    virtual void Refer_Delivered(IN IMS_UINTP nParam) = 0;
    virtual void Refer_DeliveryFailed(IN IMS_UINTP nParam) = 0;
    virtual void Refer_Notify(IN IMS_UINTP nParam) = 0;
    virtual void Refer_Terminated(IN IMS_UINTP nParam) = 0;
    virtual void Refer_Failed(IN IMS_UINTP nParam) = 0;

    virtual void Refer_Notify_Delivered(IN IMS_UINTP nParam) = 0;
    virtual void Refer_Notify_DeliveryFailed(IN IMS_UINTP nParam) = 0;
};

class IECTReferListenBaseParam
{
public:
    inline IECTReferListenBaseParam() :
            pRefer(IMS_NULL)
    {
        IMS_TRACE_MEM("uc", "uc_M : IECTReferListenBaseParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IECTReferListenBaseParam), this, 0);
    }
    inline virtual ~IECTReferListenBaseParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IECTReferListenBaseParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IECTReferListenBaseParam), this, 0);
    }

private:
    IECTReferListenBaseParam(IN const IECTReferListenBaseParam& objRHS);
    IECTReferListenBaseParam& operator=(IN const IECTReferListenBaseParam& objRHS);

public:
    UCECTReference* pRefer;
};

class IECTReferListenDeliveredParam : public IECTReferListenBaseParam
{
public:
    inline IECTReferListenDeliveredParam() :
            IECTReferListenBaseParam()
    {
        IMS_TRACE_MEM("uc", "uc_M : IECTReferListenDeliveredParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IECTReferListenDeliveredParam), this, 0);
    }
    inline virtual ~IECTReferListenDeliveredParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IECTReferListenDeliveredParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IECTReferListenDeliveredParam), this, 0);
    }

private:
    IECTReferListenDeliveredParam(IN const IECTReferListenDeliveredParam& objRHS);
    IECTReferListenDeliveredParam& operator=(IN const IECTReferListenDeliveredParam& objRHS);

public:
};

class IECTReferListenDeliveryFailedParam : public IECTReferListenBaseParam
{
public:
    inline IECTReferListenDeliveryFailedParam() :
            IECTReferListenBaseParam(),
            eReason(-1),
            eCode(-1)
    {
        IMS_TRACE_MEM("uc", "uc_M : IECTReferListenDeliveryFailedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IECTReferListenDeliveryFailedParam), this, 0);
    }
    inline virtual ~IECTReferListenDeliveryFailedParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IECTReferListenDeliveryFailedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IECTReferListenDeliveryFailedParam), this, 0);
    }

private:
    IECTReferListenDeliveryFailedParam(IN const IECTReferListenDeliveryFailedParam& objRHS);
    IECTReferListenDeliveryFailedParam& operator=(
            IN const IECTReferListenDeliveryFailedParam& objRHS);

public:
    IMS_SINT32 eReason;
    IMS_SINT32 eCode;
};

class IECTReferListenNotifyParam : public IECTReferListenBaseParam
{
public:
    inline IECTReferListenNotifyParam() :
            IECTReferListenBaseParam(),
            aStrSubState(AString::ConstNull()),
            nStatusCode(0),
            aStrEventID(AString::ConstNull())
    {
        IMS_TRACE_MEM("uc", "uc_M : IECTReferListenNotifyParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IECTReferListenNotifyParam), this, 0);
    }
    inline virtual ~IECTReferListenNotifyParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IECTReferListenNotifyParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IECTReferListenNotifyParam), this, 0);
    }

private:
    IECTReferListenNotifyParam(IN const IECTReferListenNotifyParam& objRHS);
    IECTReferListenNotifyParam& operator=(IN const IECTReferListenNotifyParam& objRHS);

public:
    AString aStrSubState;
    IMS_SINT32 nStatusCode;
    AString aStrEventID;
};

class IECTReferListenTerminatedParam : public IECTReferListenBaseParam
{
public:
    inline IECTReferListenTerminatedParam() :
            IECTReferListenBaseParam(),
            eReason(-1),
            eCode(-1)
    {
        IMS_TRACE_MEM("uc", "uc_M : IECTReferListenTerminatedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IECTReferListenTerminatedParam), this, 0);
    }
    inline virtual ~IECTReferListenTerminatedParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IECTReferListenTerminatedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IECTReferListenTerminatedParam), this, 0);
    }

private:
    IECTReferListenTerminatedParam(IN const IECTReferListenTerminatedParam& objRHS);
    IECTReferListenTerminatedParam& operator=(IN const IECTReferListenTerminatedParam& objRHS);

public:
    IMS_SINT32 eReason;
    IMS_SINT32 eCode;
};

class IECTReferListenFailedParam : public IECTReferListenBaseParam
{
public:
    inline IECTReferListenFailedParam() :
            IECTReferListenBaseParam(),
            eReason(-1),
            eCode(-1)
    {
        IMS_TRACE_MEM("uc", "uc_M : IECTReferListenFailedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IECTReferListenFailedParam), this, 0);
    }
    inline virtual ~IECTReferListenFailedParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IECTReferListenFailedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IECTReferListenFailedParam), this, 0);
    }

private:
    IECTReferListenFailedParam(IN const IECTReferListenFailedParam& objRHS);
    IECTReferListenFailedParam& operator=(IN const IECTReferListenFailedParam& objRHS);

public:
    IMS_SINT32 eReason;
    IMS_SINT32 eCode;
};

class IECTReferListenNotifyDeliveredParam : public IECTReferListenBaseParam
{
public:
    inline IECTReferListenNotifyDeliveredParam() :
            IECTReferListenBaseParam()
    {
        IMS_TRACE_MEM("uc", "uc_M : IECTReferListenNotifyDeliveredParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IECTReferListenNotifyDeliveredParam), this, 0);
    }
    inline virtual ~IECTReferListenNotifyDeliveredParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IECTReferListenNotifyDeliveredParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IECTReferListenNotifyDeliveredParam), this, 0);
    }

private:
    IECTReferListenNotifyDeliveredParam(IN const IECTReferListenNotifyDeliveredParam& objRHS);
    IECTReferListenNotifyDeliveredParam& operator=(
            IN const IECTReferListenNotifyDeliveredParam& objRHS);

public:
};

class IECTReferListenNotifyDeliveryFailedParam : public IECTReferListenBaseParam
{
public:
    inline IECTReferListenNotifyDeliveryFailedParam() :
            IECTReferListenBaseParam(),
            nStatusCode(-1)
    {
        IMS_TRACE_MEM("uc",
                "uc_M : IECTReferListenNotifyDeliveryFailedParam"
                "[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IECTReferListenNotifyDeliveryFailedParam), this, 0);
    }
    inline virtual ~IECTReferListenNotifyDeliveryFailedParam()
    {
        IMS_TRACE_MEM("uc",
                "uc_F : IECTReferListenNotifyDeliveryFailedParam"
                "[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IECTReferListenNotifyDeliveryFailedParam), this, 0);
    }

private:
    IECTReferListenNotifyDeliveryFailedParam(
            IN const IECTReferListenNotifyDeliveryFailedParam& objRHS);
    IECTReferListenNotifyDeliveryFailedParam& operator=(
            IN const IECTReferListenNotifyDeliveryFailedParam& objRHS);

public:
    IMS_SINT32 nStatusCode;
};
#endif /*  INTERFACE_UC_ECT_REFERENCE_LISTENER_H_ */
