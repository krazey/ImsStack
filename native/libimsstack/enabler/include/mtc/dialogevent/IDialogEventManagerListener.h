/*
 * author : hyunbin.shin@
 * version : 2.0
 * date : 201503
 * brief :
 */

#ifndef INTERFACE_DIALOG_EVENT_MNGR_LISTENER_H_
#define INTERFACE_DIALOG_EVENT_MNGR_LISTENER_H_

#include "CallReasonInfo.h"
#include "ServiceTrace.h"

class IDEMngrListener
{
public:
    virtual void DEMngr_Terminated(IN IMS_UINTP nParam) = 0;
};

class IDEMngrListenBaseParam
{
public:
    inline IDEMngrListenBaseParam()
    {
        IMS_TRACE_MEM("uc", "uc_M : IDEMngrListenBaseParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IDEMngrListenBaseParam), this, 0);
    }
    inline virtual ~IDEMngrListenBaseParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IDEMngrListenBaseParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IDEMngrListenBaseParam), this, 0);
    }

private:
    IDEMngrListenBaseParam(IN const IDEMngrListenBaseParam& objRHS);
    IDEMngrListenBaseParam& operator=(IN const IDEMngrListenBaseParam& objRHS);

public:
};

class IDEMngrTerminatedParam : public IDEMngrListenBaseParam
{
public:
    inline IDEMngrTerminatedParam() :
            IDEMngrListenBaseParam(),
            terminatedReason(CallReasonInfo(CODE_NONE)),
            bDestroy(IMS_FALSE)
    {
        IMS_TRACE_MEM("uc", "uc_M : IDEMngrTerminatedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IDEMngrTerminatedParam), this, 0);
    }
    inline virtual ~IDEMngrTerminatedParam()
    {
        IMS_TRACE_MEM("uc", "uc_F : IDEMngrTerminatedParam[%" PFLS_u "][%" PFLS_x "]",
                sizeof(IDEMngrTerminatedParam), this, 0);
    }

private:
    IDEMngrTerminatedParam(IN const IDEMngrTerminatedParam& objRHS);
    IDEMngrTerminatedParam& operator=(IN const IDEMngrTerminatedParam& objRHS);

public:
    CallReasonInfo terminatedReason;
    IMS_BOOL bDestroy;
};

#endif /*  INTERFACE_DIALOG_EVENT_MNGR_LISTENER_H_ */
