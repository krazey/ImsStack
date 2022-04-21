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
#ifndef INTERFACE_AOS_TRM_H_
#define INTERFACE_AOS_TRM_H_

class IAosTrmListener
{
public:
    virtual void Trm_PriorityChanged() = 0;
};

class IAosTrm
{
public:
    virtual void SetListener(IN IAosTrmListener *piListener) = 0;
    virtual void RemoveListener(IN IAosTrmListener *piListener) = 0;

    virtual IMS_BOOL IsReady() = 0;
    virtual IMS_BOOL IsTRMSupported() = 0;
    virtual void Set(IN IMS_UINT32 nType, IN IMS_BOOL bStart) = 0;
    virtual void SetEmegency(IN IMS_UINT32 nType, IN IMS_BOOL bStart) = 0;
    virtual void SetIPCAN(IN IN IMS_UINT32 nCategory) = 0;

    enum
    {
        TYPE_NONE            = 0,
        TYPE_REG             = (0x0001),
        TYPE_SUB             = (0x0002),
        TYPE_PDN             = (0x0004)
    };
};
#endif // INTERFACE_AOS_TRM_H_
