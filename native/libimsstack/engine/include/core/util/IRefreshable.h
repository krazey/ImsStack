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
#ifndef INTERFACE_REFRESHABLE_H_
#define INTERFACE_REFRESHABLE_H_

class ISipClientConnection;

class IRefreshable
{
public:
    virtual void Refreshable_RefreshCompleted(
            IN ISipClientConnection* piScc, IN IMS_SINT32 nCode = 0) = 0;
    virtual IMS_BOOL Refreshable_RefreshStarted() = 0;
    virtual void Refreshable_RefreshTerminated() = 0;
};

#endif
