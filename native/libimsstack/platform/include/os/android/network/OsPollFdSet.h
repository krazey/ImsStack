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
#ifndef OS_POLL_FD_SET_H_
#define OS_POLL_FD_SET_H_

#include "ImsFdSet.h"

class PollFds;

class OsPollFdSet : public ImsFdSet
{
public:
    OsPollFdSet();
    OsPollFdSet(IN const OsPollFdSet& other);
    ~OsPollFdSet() override;

public:
    OsPollFdSet& operator=(IN const OsPollFdSet& other);

public:
    // ImsFdSet class
    IMS_SINT32 ClearEvent(IN IMS_SINT32 nFd, IN IMS_SINT32 nEvent) override;
    void CopyFrom(IN const ImsFdSet* pFdSet) override;
    IMS_SINT32 GetSignaledEvents(IN IMS_SINT32 nFd, IN_OUT IMS_SINT32& nSignaledCount) override;
    IMS_BOOL IsEventSet(IN IMS_SINT32 nFd, IN IMS_SINT32 nEvent) override;
    IMS_SINT32 SetEvent(IN IMS_SINT32 nFd, IN IMS_SINT32 nEvent) override;
    IMS_SINT32 WaitForEvents(IN IMS_SINT32 nMilliseconds = NO_TIMEOUT) override;

private:
    PollFds* m_pFds;
};

#endif
