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
#ifndef SERVICE_FILTER_CRITERIA_H_
#define SERVICE_FILTER_CRITERIA_H_

#include "ImsMap.h"

#include "IServiceFilterCriteria.h"

class ServiceFilterCriteria final : public IServiceFilterCriteria
{
public:
    ServiceFilterCriteria();
    ~ServiceFilterCriteria() override;

    ServiceFilterCriteria(IN const ServiceFilterCriteria&) = delete;
    ServiceFilterCriteria& operator=(IN const ServiceFilterCriteria&) = delete;

public:
    // IServiceFilterCriteria class
    IMS_UINT32 AddTriggerPoint(IN const TriggerPoint& objTriggerPoint) override;
    void RemoveTriggerPoint(IN IMS_SINT32 nTriggerPointId) override;
    void RemoveAllTriggerPoints() override;
    void SetCalleePreference(
            IN const SipMethod& objMethod, IN IMS_BOOL bCalleePreference = IMS_TRUE) override;

    IMS_UINT32 Evaluate(IN const ISipMessage* piSipMsg) const;
    IMS_BOOL IsCalleePreferenceSupported(IN const SipMethod& objMethod) const;
    inline IMS_BOOL IsEmpty() const { return m_objTriggerPoints.IsEmpty(); }

private:
    // < SIP method, Flag of callee preference >
    ImsMap<IMS_SINT32, IMS_BOOL> m_objCalleePreferences;

    IMS_UINT32 m_nNextTriggerPointId;
    ImsMap<IMS_UINT32, TriggerPoint*> m_objTriggerPoints;
};

#endif
