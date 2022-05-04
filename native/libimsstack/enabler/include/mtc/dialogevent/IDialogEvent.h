#ifndef INTERFACE_DIALOG_EVENT_H_
#define INTERFACE_DIALOG_EVENT_H_

#include "AString.h"
#include "IDocument.h"
#include "IMessage.h"
#include "IMSMap.h"

typedef struct DialogLR
{
    DialogLR() :
            aStrIdentity(AString::ConstNull()),
            aStrIdentity_Display(AString::ConstNull()),
            aStrTarget_Uri(AString::ConstNull()),
            objTargetParam(IMSMap<AString, AString>())
    {
    }

    AString aStrIdentity;          // element
    AString aStrIdentity_Display;  // optional / attribute

    // local_target element
    AString aStrTarget_Uri;                   // attribute
    IMSMap<AString, AString> objTargetParam;  // attribute - pname pvalue

    // local_session-description element
    // TODO

} DialogLR;

class IDialogEvent
{
public:
    virtual AString Init(IN IElement* pDialogElement);
    virtual void DeInit();

public:
    virtual IMS_BOOL UpdateDialogInfo(
            IN IMS_UINT32 nVersion, IN IMS_UINT32 eState, IN AString aStrEntity);
    virtual IMS_BOOL Update(IN IElement* pDialogElement);
    virtual IMS_BOOL IsDialog(IN IElement* pDialogElement);

    virtual AString GetID();
    virtual AString GetCallID();
    virtual AString GetLocalTag();
    virtual AString GetRemoteTag();
    virtual AString GetDirection();
    virtual IMS_UINT32 GetState();
    virtual AString GetStateEvent();
    virtual AString GetStateCode();
    virtual IMS_UINT32 GetDuration();
    virtual AString GetReferredBy();
    virtual AString GetReferredByDisplay();
    virtual void GetLocal(OUT DialogLR* pstDialogLR);
    virtual AString GetLocalIdentity();
    virtual AString GetLocalIdentityDisplay();
    virtual AString GetLocalpVal(IN AString aStrpName);
    virtual void GetRemote(OUT DialogLR* pstDialogLR);
    virtual AString GetRemoteIdentity();
    virtual AString GetRemoteIdentityDisplay();
    virtual AString GetRemotepVal(IN AString aStrpName);
    virtual IMS_BOOL EnablePulled();
};
#endif  // INTERFACE_DIALOG_EVENT_H_
