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
#ifndef INTERFACE_TRM_TIMER_H_
#define INTERFACE_TRM_TIMER_H_

class ITRMTimerListener
{
public:
    /*
        Notifies the application that the service timer is expired.
    */
    virtual void ITRMTimer_Expired(IN IMS_UINT32 nSlotId, IN IMS_UINT32 nType) = 0;
};

class ITRMTimer
{
public:
    /*
    */
    virtual void SetListener(IN ITRMTimerListener *piListener) = 0;

    /*
    */
    virtual void Start() = 0;

    /*
    */
    virtual void Stop() = 0;
};

#endif // INTERFACE_TRM_TIMER_H_
