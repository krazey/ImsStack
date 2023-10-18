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

#ifndef _UCE_SUBSCRIBE_MANAGER_H_
#define _UCE_SUBSCRIBE_MANAGER_H_

#include "ImsActivityEx.h"

class ICoreService;
class UceSubscribe;

class UceSubscribeManager : public ImsActivityEx
{
    /* ------------------------------------------------------------------------------------------
        Constructor, Destructor, Operator Overloading
    ---------------------------------------------------------------------------------------------
  */
public:
    explicit UceSubscribeManager(IN const AString& strName, ICoreService* _piCoreService,
            IN const AString& strAppName, IN IMS_SINT32 nSimSlot);
    virtual ~UceSubscribeManager();
    /* ------------------------------------------------------------------------------------------
        Methods
    ---------------------------------------------------------------------------------------------
  */
public:
    IMS_BOOL QuerySingleCapability(IN const AString& strUser, IN IMS_UINT32 key);
    IMS_BOOL QueryMultiCapability(IN const ImsList<AString>& objUsers, IN IMS_UINT32 key);
    IMS_BOOL AosConnected(IMS_UINT32 conectedService);
    IMS_BOOL AosDisConnected();
    void ClosedService();  // core service closed
protected:
    virtual IMS_BOOL OnMessage(IN IMSMSG& objMsg);

private:
    IMS_BOOL RemoveSubscribe(IN UceSubscribe* subscribe);
    IMS_BOOL ClearSubscribeList();

    /* ------------------------------------------------------------------------------------------
        Variable
    ---------------------------------------------------------------------------------------------
  */
protected:
    ImsList<UceSubscribe*> m_objUceSubscribeList;  // Subscription List
    ICoreService* m_piCoreService;
    AString m_strAppName;
    IMS_SINT32 m_nSimSlot;
    IMS_UINT32 m_nConnectedServices;
};
#endif  // _UCE_SUBSCRIBE_MANAGER_H_
