#ifndef _AOS_HANDLE_MTC_H_
#define _AOS_HANDLE_MTC_H_

#include "handle/AosHandle.h"

class AosHandleMtc
    : public AosHandle
{
public:
    AosHandleMtc
    (
        IN IAosAppContext* piAppContext,
        IN CONST AString& strAppId,
        IN CONST AString& strServiceId,
        IN CONST IMS_SINT32 nServiceType
    );
    virtual ~AosHandleMtc();

    // IAosHandle
    virtual void App_Notify();

    // IAosCallTrackerListener
    virtual void CallTracker_StateChanged(IN IMS_UINT32 nType, IN IMS_UINT32 nState);

    // IAosNetTrackerListener
    virtual void NetTracker_StatusChanged();

protected:
    virtual void InitializeServiceBlock();
    virtual void InitializeServiceFeature();
    virtual void InitializeFeatureTags();

    virtual void UpdateFeatureTags();

    virtual void ProcessImsSuspended(IN IMS_UINT32 nReason = 0);
    virtual void ProcessImsResumed(IN IMS_UINT32 nReason = 0);

    virtual void CheckSuspended();
    virtual void SetSuspendedReason(IN IMS_UINT32 nReason);
    virtual void ResetSuspendedReason(IN IMS_UINT32 nReason);

    virtual void Init();
    virtual void CleanUp();

    virtual IMS_BOOL IsHandleBlocked() const;
    virtual IMS_BOOL IsBlockForMobile(IN IMS_UINT32 nBlock) const;
    virtual IMS_BOOL IsBlockForWifi(IN IMS_UINT32 nBlock) const;

    virtual void ProcessCapabilitiesChanged(
            IN const IMSMap<IMS_UINT32, IMS_UINT32>& objNewCapabilities);
    virtual void ProcessNetworkChanged();
    virtual void ProcessVopsStateChanged(IN IMS_UINT32 nState);

private:
    IMS_UINT32 m_nVops;
};
#endif // _AOS_HANDLE_MTC_H_
