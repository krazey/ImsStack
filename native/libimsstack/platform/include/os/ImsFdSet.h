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
#ifndef IMS_FD_SET_H_
#define IMS_FD_SET_H_

#include "ImsTypeDef.h"

class ImsFdSet
{
public:
    ImsFdSet();
    ImsFdSet(IN const ImsFdSet& other);
    virtual ~ImsFdSet();

public:
    ImsFdSet& operator=(IN const ImsFdSet& other);

public:
    virtual IMS_SINT32 ClearEvent(IN IMS_SINT32 nFd, IN IMS_SINT32 nEvent);
    virtual void CopyFrom(IN const ImsFdSet* pFdSet);
    virtual IMS_SINT32 GetSignaledEvents(IN IMS_SINT32 nFd, IN_OUT IMS_SINT32& nSignaledCount);
    virtual IMS_BOOL IsEventSet(IN IMS_SINT32 nFd, IN IMS_SINT32 nEvent);
    virtual IMS_BOOL IsHighestFdRequired() const;
    virtual IMS_SINT32 SetEvent(IN IMS_SINT32 nFd, IN IMS_SINT32 nEvent);
    virtual void SetHighestFd(IN IMS_SINT32 nFd);
    virtual IMS_SINT32 WaitForEvents(IN IMS_SINT32 nMilliseconds = NO_TIMEOUT);

    inline static IMS_BOOL IsExceptEventSignaled(IN IMS_SINT32 nEvents)
    {
        return ((nEvents & EVENT_EXCEPT) != 0);
    }
    inline static IMS_BOOL IsReadEventSignaled(IN IMS_SINT32 nEvents)
    {
        return ((nEvents & EVENT_READ) != 0);
    }
    inline static IMS_BOOL IsWriteEventSignaled(IN IMS_SINT32 nEvents)
    {
        return ((nEvents & EVENT_WRITE) != 0);
    }

public:
    /// Method for the descriptor management
    enum
    {
        TYPE_SELECT = 0,
        TYPE_POLL = 1
    };

    enum
    {
        NO_TIMEOUT = (-1)
    };

    /// Events
    enum
    {
        EVENT_READ = 0x01,
        EVENT_WRITE = 0x02,
        EVENT_EXCEPT = 0x04,
        EVENT_ALL = 0x0F,
        /// TCP client socket :: poll() only
        /// SetEvent(), IsEventSet()
        EVENT_TCP_C = 0x10000,
        /// TCP sockets :: poll() only
        /// If this flag is set, the EXCEPT event will be added when any other event is installed
        /// SetEvent()
        EVENT_TCP = 0x20000
    };
};

#endif
