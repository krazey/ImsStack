
#ifndef IMS_ANDROID_JAVA_WMS_H_
#define IMS_ANDROID_JAVA_WMS_H_

#include "IMSActivityEx.h"
#include "IIMSActivityControl.h"

#include "IMtsClient.h"
#include "IMtsClientListener.h"

class AString;
class IThread;
class System;

class AndroidJavaWMS : public IMtsClient, public IMSActivityEx
{
private:
    AndroidJavaWMS(IN IMS_SINT32 nSlotId = 0);
    virtual ~AndroidJavaWMS(void);

public:
    static AndroidJavaWMS* GetAndroidJavaWMS(IN IMS_SINT32 nSlotId = 0);
    static void DestroyAndroidJavaWMS(IN IMS_SINT32 nSlotId = 0);
    static IMS_BOOL StartUp();
    static void CleanUp();

    // IMtsClient class
    virtual IMS_RESULT Init() override;
    virtual IMS_RESULT Release() override;
    inline virtual IMSWMS_UINT32 GetFeature() override { return FEATURE_NONE; }
    virtual IMS_UINT32 ReportMtSMS(IN IMS_UINT32 nSmsFormat, IN IMS_UINT32 nSmsLength,
            IN CONST IMS_BYTE* pbySmsData, IN IMS_SINT32 nSlotId) override;
    virtual IMS_RESULT ReportMoStatus(IN IMS_UINT32 nReason, IN IMS_UINT32 nSmsFormat,
            IN IMS_UINT8 nRetryAfter = 0, IN IMS_SINT32 nSeqId = -1,
            IN IMS_SINT32 nSlotId = -1) override;
    inline virtual void SetListener(IN IMtsClientListener* iscl) override { (void)iscl; }

    // IMSActivityEx class
    virtual IMS_BOOL OnMessage(IN IMSMSG& objMSG) override;

private:
    static AndroidJavaWMS* s_pAndroidJavaWms[];
    IMS_SINT32 m_nSlotId;
};

#endif
