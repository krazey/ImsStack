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
#include "PlatformContext.h"
#include "ServiceTrace.h"
#include "device/OsPowerInfo.h"
#include "system-intf/SystemConstants.h"

__IMS_TRACE_TAG_IPL__;

static void osPowerInfo_NotifyEvent(IN OsPowerInfo* pPowerInfo, IN POWERLEVEL_ENTYPE ePowerEvent)
{
    if (pPowerInfo != IMS_NULL)
    {
        pPowerInfo->PostMsgRegisteredThread(ePowerEvent);
    }
}

class OsPowerInfoPrivate
{
public:
    inline OsPowerInfoPrivate() :
            m_nPowerValue(0),
            m_nOldPowerValue(0),
            m_ePowerLevel(POWERLEVEL_OFF),
            m_eOldPowerLevel(POWERLEVEL_OFF),
            m_pPowerInfo(IMS_NULL)
    {
    }
    inline ~OsPowerInfoPrivate() {}

    POWERLEVEL_ENTYPE GetPowerLevel() const;
    void SetPowerInfo(IN OsPowerInfo* pPowerInfo);
    void Update();
    void UpdateValue();

    void SetPowerValue(IN IMS_SINT32 nValue);
    void UpdatePowerLevel();
    void UpdatePowerValue();

    inline void NotifyPowerEvent(IN POWERLEVEL_ENTYPE ePowerEvent)
    {
        osPowerInfo_NotifyEvent(m_pPowerInfo, ePowerEvent);
    }

private:
    IMS_SINT32 m_nPowerValue;
    IMS_SINT32 m_nOldPowerValue;

    POWERLEVEL_ENTYPE m_ePowerLevel;
    POWERLEVEL_ENTYPE m_eOldPowerLevel;
    OsPowerInfo* m_pPowerInfo;
};

PUBLIC VIRTUAL POWERLEVEL_ENTYPE OsPowerInfoPrivate::GetPowerLevel() const
{
    return m_ePowerLevel;
}

PUBLIC
void OsPowerInfoPrivate::SetPowerInfo(IN OsPowerInfo* pPowerInfo)
{
    m_pPowerInfo = pPowerInfo;
}

PUBLIC
void OsPowerInfoPrivate::Update()
{
    NotifyPowerEvent(m_ePowerLevel);
}

PUBLIC
void OsPowerInfoPrivate::SetPowerValue(IN IMS_SINT32 nValue)
{
    m_nPowerValue = nValue;
}

PUBLIC
void OsPowerInfoPrivate::UpdatePowerLevel()
{
    const IMS_SINT32 BATTERY_THRESHOLDS[] = {20, 15, 4, -1};

    if (m_nPowerValue < BATTERY_THRESHOLDS[2])
    {
        m_ePowerLevel = POWERLEVEL_LOW;
    }
    else
    {
        m_ePowerLevel = POWERLEVEL_HIGH;
    }

    if (m_eOldPowerLevel != m_ePowerLevel)
    {
        Update();

        IMS_TRACE_D("PowerLevel :: %d >> %d", m_eOldPowerLevel, m_ePowerLevel, 0);

        m_eOldPowerLevel = m_ePowerLevel;
    }
}

PUBLIC
void OsPowerInfoPrivate::UpdatePowerValue()
{
    if (m_nOldPowerValue != m_nPowerValue)
    {
        m_nOldPowerValue = m_nPowerValue;
        NotifyPowerEvent(POWERLEVEL_VALUE);
    }
}

PUBLIC
OsPowerInfo::OsPowerInfo() :
        m_pPowerInfoP(new OsPowerInfoPrivate())
{
    if (m_pPowerInfoP != IMS_NULL)
    {
        m_pPowerInfoP->SetPowerInfo(this);
    }

    PlatformContext::GetInstance()->GetSystem()->AddListener(
            SystemConstants::CATEGORY_POWER, this, IMS_SLOT_0);
}

PUBLIC VIRTUAL OsPowerInfo::~OsPowerInfo()
{
    PlatformContext::GetInstance()->GetSystem()->RemoveListener(
            SystemConstants::CATEGORY_POWER, this, IMS_SLOT_0);

    if (m_pPowerInfoP != IMS_NULL)
    {
        delete m_pPowerInfoP;
    }
}

PUBLIC VIRTUAL POWERLEVEL_ENTYPE OsPowerInfo::GetPowerLevel()
{
    if (m_pPowerInfoP == IMS_NULL)
    {
        return POWERLEVEL_OFF;
    }

    return m_pPowerInfoP->GetPowerLevel();
}

PUBLIC VIRTUAL void OsPowerInfo::System_NotifyEvent(
        IN IMS_UINT32 nEvent, IN IMS_UINTP nWParam, IN IMS_UINTP nLParam)
{
    (void)nLParam;

    IMS_TRACE_D("PowerInfo :: event=%d, wp=%" PFLS_d ", lp=%" PFLS_d, nEvent, nWParam, nLParam);

    switch (nEvent)
    {
        case IMS_SYSTEM_BATTERY_CHANGED:
        {
            IMS_SINT32 nPowerValue = LONG_TO_SINT(nWParam);

            if (m_pPowerInfoP != IMS_NULL)
            {
                m_pPowerInfoP->SetPowerValue(nPowerValue);

                m_pPowerInfoP->UpdatePowerValue();

                m_pPowerInfoP->UpdatePowerLevel();
            }
            break;
        }
        default:
            // no-op
            break;
    }
}
