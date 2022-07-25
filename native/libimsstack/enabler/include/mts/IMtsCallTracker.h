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

#ifndef INTERFACE_MTS_CALL_TRACKER_H_
#define INTERFACE_MTS_CALL_TRACKER_H_

class IMtsCallTrackerListener;

class IMtsCallTracker
{
public:
    virtual IMS_BOOL IsEmergencyCallActive() const = 0;

    virtual IMS_SINT32 GetSlotId() const = 0;
    virtual IMS_UINT32 GetCallState(IN IMS_UINT32 nType) const = 0;
    virtual IMS_UINT32 GetSessionType(IN IMS_UINT32 nType) const = 0;

    virtual void AddListener(IN IMtsCallTrackerListener* piListener) = 0;
    virtual void RemoveListener(IN IMtsCallTrackerListener* piListener) = 0;
};

#endif
