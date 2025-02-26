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
#ifndef OS_ISIM_H_
#define OS_ISIM_H_

#include "IDigestAka.h"
#include "ISystemListener.h"
#include "ImsIsim.h"
#include "ImsMap.h"

class IDigestAkaListener;
class IIsimListener;
class IThread;

class OsIsimDigestAka : public IDigestAka
{
public:
    explicit OsIsimDigestAka(IN ImsIsim* pIsim);
    virtual ~OsIsimDigestAka();

    OsIsimDigestAka(IN const OsIsimDigestAka&) = delete;
    OsIsimDigestAka& operator=(IN const OsIsimDigestAka&) = delete;

public:
    void Destroy() override;
    IMS_RESULT GetAuthResponse(IN const ByteArray& objChallenge) override;
    void SetListener(IN IDigestAkaListener* piListener) override;

    void NotifyAutsFailed(IN const ByteArray& objAuts);
    void NotifyMacFailed();
    void NotifyResponse(
            IN const ByteArray& objRes, IN const ByteArray& objIk, IN const ByteArray& objCk);

private:
    ImsIsim* m_pIsim;
    IDigestAkaListener* m_pDigestAkaListener;
};

class OsIsim : public ImsIsim, public ISystemListener
{
public:
    explicit OsIsim(IN IMS_SINT32 nSlotId);
    virtual ~OsIsim();

    OsIsim(IN const OsIsim&) = delete;
    OsIsim& operator=(IN const OsIsim&) = delete;

public:
    // ImsIsim class
    void DispatchServiceMessage(IN IMS_UINTP nWparam, IN IMS_UINTP nLparam) override;

    // IIsim class
    IDigestAka* CreateDigestAka() override;
    AString GetHomeDomainName() const override;
    AString GetImpi() const override;
    AStringArray GetImpu() const override;
    AStringArray GetPcscf() const override;
    inline IMS_SINT32 GetState() const override { return m_nState; }
    inline IMS_BOOL IsLoadCompleted() const override
    {
        return m_nState == STATE_LOADED || m_nState == STATE_NOT_PRESENT;
    }
    void AddListener(IN IIsimListener* piListener) override;
    void RemoveListener(IN IIsimListener* piListener) override;
    void Init() override;
    void Release() override;

    // ISystemListener class
    void System_NotifyEvent(
            IN IMS_UINT32 nEvent, IN IMS_UINTP nWParam, IN IMS_UINTP nLParam) override;

public:
    void NotifyStateChanged(IN IMS_SINT32 nState);
    void SetState(IN IMS_SINT32 nState);

    // Digest AKA
    void DestroyDigestAka(IN OsIsimDigestAka* pDigestAka);
    IMS_BOOL IsDigestAkaPresent(IN const OsIsimDigestAka* pDigestAka);

    static IMS_SINT32 ConvertSystemStateToEnum(IN const AString& strState);
    static const IMS_CHAR* SystemStateToString(IN IMS_SINT32 nSystemState);
    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

public:
    // EF fields for ISIM
    enum
    {
        EF_ID_IMPI = 0x6F02,
        EF_ID_DOMAIN = 0x6F03,
        EF_ID_IMPU = 0x6F04,
        EF_ID_PCSCF = 0x6F09
    };

    // ISIM events of platform layer (aligned with SimAgent.java)
    static const IMS_SINT32 NOTIFICATION_ISIM_STATE_CHANGED = 101;
    static const IMS_SINT32 NOTIFICATION_ISIM_AUTH = 102;

private:
    ImsList<IIsimListener*> m_objIsimListeners;
    ImsList<OsIsimDigestAka*> m_objDigestAkas;
    IThread* m_piOwnerThread;
    IMS_SINT32 m_nState;
};

#endif
