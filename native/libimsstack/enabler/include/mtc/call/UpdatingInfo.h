#ifndef UPDATING_INFO_H_
#define UPDATING_INFO_H_

#include "MtcDef.h"

class UpdatingInfo final
{
public:
    UpdatingInfo();
    virtual ~UpdatingInfo();
    UpdatingInfo(IN const UpdatingInfo&) = delete;
    UpdatingInfo& operator=(IN const UpdatingInfo&) = delete;

public:
    inline MediaInfo& GetNegotiatedInfo() { return m_objNegotiatedInfo; }
    inline MediaInfo& GetModifyingInfo() { return m_objModifyingInfo; }
    inline MediaInfo& GetAlertingInfo() { return m_objAlertingInfo; }
    inline MediaInfo& GetModifiedInfo() { return m_objModifiedInfo; }
    inline IMS_BOOL IsModifier() { return m_bModifier; }
    inline IMS_BOOL IsAlerted() { return m_bAlerted; }
    inline void SetModifier() { m_bModifier = IMS_TRUE; }
    inline void SetAlerted() { m_bAlerted = IMS_TRUE; }

    IMS_BOOL IsHeld();
    IMS_BOOL IsHeldBy();
    IMS_BOOL IsResumed();
    IMS_BOOL IsResumedBy();
    IMS_BOOL IsNeedToAlert();
    IMS_BOOL IsRequestedHoldResume();
    IMS_BOOL IsRequestedModifying();
    IMS_BOOL IsModified();

private:
    MediaInfo m_objNegotiatedInfo;
    MediaInfo m_objModifyingInfo;
    MediaInfo m_objAlertingInfo;
    MediaInfo m_objModifiedInfo;
    IMS_BOOL m_bModifier;
    IMS_BOOL m_bAlerted;
};

#endif
