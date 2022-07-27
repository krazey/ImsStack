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

class IsimEfContent
{
public:
    // Type of EF
    enum
    {
        EF_TRANSPARENT = 1,
        EF_LINEAR_FIXED = 2
    };

public:
    inline IsimEfContent(IN IMS_SINT32 nType = EF_TRANSPARENT) :
            m_nType(nType),
            m_bReadRequested(IMS_FALSE),
            m_bReady(IMS_FALSE),
            m_nCount(0),
            m_nIndexToRead(1),
            m_objRecords(IMSList<ByteArray>())
    {
    }
    inline IsimEfContent(IN const IsimEfContent& other) :
            m_nType(other.m_nType),
            m_bReadRequested(other.m_bReadRequested),
            m_bReady(other.m_bReady),
            m_nCount(other.m_nCount),
            m_nIndexToRead(other.m_nIndexToRead),
            m_objRecords(other.m_objRecords)
    {
    }
    inline virtual ~IsimEfContent() {}

public:
    inline IsimEfContent& operator=(IN const IsimEfContent& other)
    {
        if (this != &other)
        {
            m_nType = other.m_nType;
            m_bReadRequested = other.m_bReadRequested;
            m_bReady = other.m_bReady;
            m_nCount = other.m_nCount;
            m_nIndexToRead = other.m_nIndexToRead;
            m_objRecords = other.m_objRecords;
        }

        return (*this);
    }

public:
    inline void AddRecord(IN const IMS_BYTE* pbyData, IN IMS_SINT32 nSize)
    {
        ByteArray objRecord(pbyData, nSize);
        m_objRecords.Append(objRecord);
    }

    inline void Clear()
    {
        m_bReadRequested = IMS_FALSE;
        m_bReady = IMS_FALSE;
        m_nCount = 0;
        m_nIndexToRead = 1;
        m_objRecords.Clear();
    }
    inline void ClearRecords()
    {
        m_bReadRequested = IMS_FALSE;
        m_nIndexToRead = 1;
        m_objRecords.Clear();
    }

    inline IMS_SINT32 GetCount() const { return m_nCount; }
    inline IMS_SINT32 GetIndexToRead() const { return m_nIndexToRead; }
    inline const IMSList<ByteArray>& GetRecords() const { return m_objRecords; }

    inline IMS_SINT32 GetType() const { return m_nType; }
    inline IMS_BOOL IsAllRecordsReadCompleted() const
    {
        return (m_nCount == static_cast<IMS_SINT32>(m_objRecords.GetSize()));
    }
    inline IMS_BOOL IsReadRequested() const { return m_bReadRequested; }
    inline IMS_BOOL IsReady() const { return m_bReady; }

    inline void SetCount(IN IMS_SINT32 nCount) { m_nCount = nCount; }
    inline void SetIndexToRead(IN IMS_SINT32 nIndex) { m_nIndexToRead = nIndex; }
    inline void SetRecord(IN const IMS_BYTE* pbyData, IN IMS_SINT32 nSize)
    {
        ByteArray objRecord(pbyData, nSize);

        m_objRecords.Clear();
        m_objRecords.Append(objRecord);
    }
    inline void SetReadRequested(IN IMS_BOOL bReadRequested) { m_bReadRequested = bReadRequested; }
    inline void SetReady(IN IMS_BOOL bReady) { m_bReady = bReady; }

private:
    // Type of EF - transparent / linear_fixed
    IMS_SINT32 m_nType;

    // Checks if the READ operation is requested or not
    IMS_BOOL m_bReadRequested;
    // Ready state to read EF record value
    IMS_BOOL m_bReady;
    // Count of EF record
    IMS_SINT32 m_nCount;
    // For read operation (tracking the current index to read)
    IMS_SINT32 m_nIndexToRead;
    // Data of EF record
    IMSList<ByteArray> m_objRecords;
};

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
    OsIsim(IN IMS_SINT32 nSlotId);
    virtual ~OsIsim();

    OsIsim(IN const OsIsim&) = delete;
    OsIsim& operator=(IN const OsIsim&) = delete;

public:
    // ImsIsim class
    void DispatchServiceMessage(IN IMS_UINTP nWparam, IN IMS_UINTP nLparam) override;

    // IIsim class
    void ClearRecords() override;
    IDigestAka* CreateDigestAka() override;
    IMS_RESULT GetField(IN IMS_SINT32 nField) override;
    IMS_RESULT GetHomeDomainName() override;
    IMS_RESULT GetImpi() override;
    IMS_RESULT GetImpu() override;
    IMS_SINT32 GetState() const override;
    IMS_BOOL IsReady() override;
    void AddListener(IN IIsimListener* piListener) override;
    void RemoveListener(IN IIsimListener* piListener) override;
    IMS_RESULT Init() override;
    void Release() override;
    IMS_RESULT Start(IN IMS_SINT32 nEfs = EF_ALL) override;

    // ISystemListener class
    void System_NotifyEvent(
            IN IMS_UINT32 nEvent, IN IMS_UINTP nWParam, IN IMS_UINTP nLParam) override;

public:
    IMS_RESULT GetFileAttributes(IN IMS_SINT32 nRequiredEfs, IN IMS_SINT32 nEf,
            IN IMS_SINT32 nFileId, IN IMS_BOOL bOptionalField = IMS_FALSE,
            IN IMS_SINT32 nEfContentType = IsimEfContent::EF_TRANSPARENT);
    IMS_RESULT GetRecord(IN IMS_SINT32 nFileId);
    IMS_BOOL IsAllAttributesReady() const;
    void NotifyAuthResult(IN IMS_RESULT nResult);
    void NotifyError(IN IMS_UINT32 nError);
    void NotifyError(IN IMS_SINT32 nFileId, IN IMS_SINT32 nOperation);
    void NotifyRecordReadCompleted(IN IMS_SINT32 nFileId, IN const IsimEfContent& objContent);
    void ReleaseUimClient(IN IMS_BOOL bFromApp = IMS_FALSE);
    void SetRecord(IN IMS_SINT32 nFileId, IN const IMS_BYTE* pbyData, IN IMS_SINT32 nSize);
    void SetRecordAttributes(
            IN IMS_SINT32 nFileId, IN IMS_BOOL bReady, IN IMS_SINT32 nRecordCount = 0);
    void SetState(IN IMS_SINT32 nState);

    // Digest AKA
    void DestroyDigestAka(IN OsIsimDigestAka* pDigestAka);
    IMS_BOOL IsDigestAkaPresent(IN OsIsimDigestAka* pDigestAka);

public:
    static IMS_SINT32 ConvertSimStateToEnum(IN const AString& strState);

public:
    // EF fields for ISIM
    enum
    {
        EF_ID_IMPI = 0x6F02,
        EF_ID_DOMAIN = 0x6F03,
        EF_ID_IMPU = 0x6F04,
        EF_ID_IST = 0x6F07,  // ISIM service table
        EF_ID_PCSCF = 0x6F09
    };

    // Operation of ISIM
    enum
    {
        OPERATION_READ_RECORD = 0
    };

    // ISIM events of platform layer (aligned with SimAgent.java)
    static const IMS_SINT32 NOTIFICATION_ISIM_STATE_REFRESH = 101;
    static const IMS_SINT32 NOTIFICATION_ISIM_STATE_CHANGED = 102;
    static const IMS_SINT32 NOTIFICATION_ISIM_READ_FILE_ATTRIBUTE = 103;
    static const IMS_SINT32 NOTIFICATION_ISIM_READ_RECORD = 104;
    static const IMS_SINT32 NOTIFICATION_ISIM_AUTH = 105;

private:
    IMSList<IIsimListener*> m_objIsimListeners;
    IMSList<OsIsimDigestAka*> m_objDigestAkas;

    IMS_BOOL m_bInitialized;
    // ISIM interworking via Android Telephony Framework (using TelephonyManager)
    IThread* m_piOwnerThread;
    IMS_SINT32 m_nState;
    // <EF id, EF content>
    IMSMap<IMS_SINT32, IsimEfContent> m_objEfContents;

    // Count of authentication failure
    enum
    {
        MAX_AUTH_FAILURE_COUNT_FOR_RECOVERY = 2
    };
    IMS_SINT32 m_nCountForAuthFailed;
};

#endif
