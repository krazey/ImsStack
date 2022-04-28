#ifndef MTS_CALL_TRACKER_H_
#define MTS_CALL_TRACKER_H_

#include "IMSList.h"
#include "IMSMap.h"
#include "IMtsCallTracker.h"

class MtsCallTracker final :
        public IMtsCallTracker
{
public:
    MtsCallTracker(IN IMS_SINT32 nSlotId_);
    ~MtsCallTracker();

    // IMtsCallTracker
    IMS_BOOL IsEmergencyCallActive() const;
    IMS_SINT32 GetSlotId() const;
    IMS_UINT32 GetCallState(IN IMS_UINT32 nType) const;
    IMS_UINT32 GetSessionType(IN IMS_UINT32 nType) const;
    void AddListener(IN IMtsCallTrackerListener *piListener);
    void RemoveListener(IN IMtsCallTrackerListener* piListener);

protected:
    void AddOrUpdateCall(
            IN IMSMap<IMS_SINTP, IMS_UINT32> &objCalls, IN IMS_SINTP nKey,IN IMS_UINT32 nState);
    void RemoveCall(IN IMSMap<IMS_SINTP, IMS_UINT32> &objCalls, IN IMS_SINTP nKey);
    IMS_UINT32 GetConvertedState(IN IMS_UINT32 nState);
    IMS_UINT32 GetTotalState(IN IMSMap<IMS_SINTP, IMS_UINT32> &objCalls);
    IMS_UINT32 GetState(IN IMS_UINT32 nType) const;
    void SetState(IN IMS_UINT32 nType, IN IMS_UINT32 nState);
    void Notify(IN IMS_UINT32 nType, IN IMS_UINT32 nState);
    void ProcessEmergencyChanged(IN IMS_SINTP nKey, IN IMS_UINT32 nState);

    // IUCCallListener _UC_TO_MTC_
    void ChangedCallState(IN IMS_UINTP nParam);
    void ChangedCallTotalState(IN IMS_UINTP nParam);

    // Log
    static const IMS_CHAR* TypeToString(IN IMS_UINT32 nType);
    static const IMS_CHAR* StateToString(IN IMS_UINT32 nState);

protected:
    IMS_SINT32                          nSlotId;
    IMS_UINT32                          nEmergencyState;
    IMSMap<IMS_SINTP, IMS_UINT32>       objEmergencyCalls;
    IMSList<IMtsCallTrackerListener*>   objListeners;
};

#endif
