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
#ifndef INTERFACE_SYSTEM_LISTENER_H_
#define INTERFACE_SYSTEM_LISTENER_H_

// This interface will be used between OsXxx and System
class ISystemListener
{
public:
    virtual void System_NotifyEvent(
            IN IMS_UINT32 nEvent, IN IMS_UINTP nWParam, IN IMS_UINTP nLParam) = 0;
};

#endif
