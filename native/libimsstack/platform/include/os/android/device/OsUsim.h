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
#ifndef OS_USIM_H_
#define OS_USIM_H_

#include "IDigestAka.h"
#include "ImsUsim.h"
#include "system-intf/ISystemListener.h"

class IThread;
class IDigestAkaListener;

class OsUsimDigestAka
    : public IDigestAka
{
public:
    explicit OsUsimDigestAka(IN ImsUsim* pUsim);
    virtual ~OsUsimDigestAka();

    OsUsimDigestAka(IN const OsUsimDigestAka&) = delete;
    OsUsimDigestAka& operator=(IN const OsUsimDigestAka&) = delete;

public:
    void OnAuthResponseReceived(IN const ByteArray& objAuthRes);

protected:
    // IDigestAka class
    void Destroy() override;
    IMS_RESULT GetAuthResponse(IN const ByteArray& objChallenge) override;
    void SetListener(IN IDigestAkaListener* piListener) override;

private:
    ImsUsim* m_pUsim;
    ByteArray m_objAuthResponse;
    IDigestAkaListener* m_piListener;
};

class OsUsim
    : public ImsUsim
    , public ISystemListener
{
public:
    OsUsim(IN IMS_SINT32 nSlotId);
    virtual ~OsUsim();

    OsUsim(IN const OsUsim&) = delete;
    OsUsim& operator=(IN const OsUsim&) = delete;

public:
    // Digest AKA
    void DestroyDigestAka(IN OsUsimDigestAka* pDigestAka);

protected:
    IDigestAka* CreateDigestAka() override;

    // ImsUsim class
    void DispatchServiceMessage(IN IMS_UINTP nWparam, IN IMS_UINTP nLparam) override;

    // ISystemListener class
    void System_NotifyEvent(IN IMS_UINT32 nEvent,
            IN IMS_UINTP nWParam, IN IMS_UINTP nLParam) override;

public:
    // Digest AKA
    IMS_BOOL IsDigestAkaPresent(IN OsUsimDigestAka* pDigestAka);

private:
    // Result of REQUEST_USIM_AUTH(String nonce, int owner)
    // SIMStateAgent#NOTIFICATION_USIM_AUTH (106)
    static const int NOTIFICATION_USIM_AUTH = 106;

    IMSList<OsUsimDigestAka*> m_objDigestAkas;
    IThread* m_piOwnerThread;
};

#endif
