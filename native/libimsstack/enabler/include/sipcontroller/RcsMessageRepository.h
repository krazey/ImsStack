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
#ifndef _RCS_MESSAGE_REPOSITORY_H_
#define _RCS_MESSAGE_REPOSITORY_H_

#include "ImsList.h"
class RcsMessageTracker;

class RcsMessageRepository
{
public:
    RcsMessageRepository();
    virtual ~RcsMessageRepository();

public:
    IMS_BOOL Add(IN RcsMessageTracker* pRcsMessageTracker);
    IMS_BOOL Remove(IN IMS_UINTP nSessionId);
    RcsMessageTracker* Get(IN IMS_UINTP nSessionId);
    RcsMessageTracker* GetAt(IN IMS_UINT32 nIndex);
    IMS_UINT32 GetSize() const;
    void Clear();

private:
    ImsList<RcsMessageTracker*> objRcsMessageTrackers;
};

#endif  // _PAGER_MESSAGE_REPOSITORY_H_