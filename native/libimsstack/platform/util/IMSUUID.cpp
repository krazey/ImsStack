/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20120214  hwangoo.park@             Created
    </table>

    Description
*/

#include "ServiceMemory.h"
#include "ServicePhoneInfo.h"
#include "ServiceSystemTime.h"
#include "ServiceUtil.h"
#include "IMSMD5.h"
#include "IMSSHA1.h"
#include "AStringBuffer.h"
#include "IPAddress.h"
#include "IMSUUID.h"

struct UUID
{
    IMS_UINT32 nTimeLow;
    IMS_UINT16 nTimeMid;
    IMS_UINT16 nTimeHiAndVersion;
    IMS_UINT8 nClockSeqHiAndReserved;
    IMS_UINT8 nClockSeqLow;
    IMS_BYTE abyNode[6];
};

// 6ba7b817-9dad-11d1-80b4-00c04fd430c8
static const UUID NAMESPACE_IMS =
{
    0x6ba7b817,
    0x9dad,
    0x11d1,
    0x80, 0xb4,
    { 0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8 }
};

/*

Remarks

*/
PUBLIC GLOBAL
AString IMSUUID::GetUUID(IN IMS_SINT32 nVersion /* = VERSION_4 */,
        IN CONST AString &strName /* = AString::ConstNull() */)
{
    // UUID = time-low "-" time-mid "-" time-hi-and-version "-"
    // clock-seq-hi-and-reserved clock-seq-low "-" node
    // 00000000-0000-0000-0000-000000000000
    AStringBuffer objUUID(64);

    switch (nVersion)
    {
    case IMSUUID::VERSION_1: {
        // TODO: need to be adapted.
        // objUUID = UtilService::GetUtilService()->GetSystemUtil()->GetUuid();
        IMS_UINT32 nMicroSecs = IMS_SYS_GetTimeInMicroSeconds();
        AString strRandom;
        strRandom.SetNumber(nMicroSecs);
        GetUUIDv4(strRandom, objUUID);
        break;
    }
    case IMSUUID::VERSION_3:
        GetUUIDv3(strName, objUUID);
        break;

    case IMSUUID::VERSION_4:
        GetUUIDv4(strName, objUUID);
        break;

    case IMSUUID::VERSION_5:
        GetUUIDv5(strName, objUUID);
        break;

    default:
        break;
    }

    return static_cast<const AStringBuffer&>(objUUID).GetString();
}

/*

Remarks

*/
PRIVATE GLOBAL
void IMSUUID::GetUUIDv3(IN CONST AString &strName, OUT AStringBuffer &objUUID)
{
    UUID stNSID = NAMESPACE_IMS;

    // Put the namespace id in the network byte order
    stNSID.nTimeLow = IPAddress::HToNL(stNSID.nTimeLow);
    stNSID.nTimeMid = IPAddress::HToNS(stNSID.nTimeMid);
    stNSID.nTimeHiAndVersion = IPAddress::HToNS(stNSID.nTimeHiAndVersion);

    // MD5 hash
    MD5Context stMD5;
    IMS_UCHAR uacHash[16];

    IMSMD5_Initialize(&stMD5);
    IMSMD5_Update(reinterpret_cast<const IMS_UCHAR*>(&stNSID), sizeof(UUID), &stMD5);
    IMSMD5_Update(reinterpret_cast<const IMS_UCHAR*>(strName.GetStr()),
            strName.GetLength(), &stMD5);
    IMSMD5_Finalize(&stMD5, uacHash);

    // Convert UUID to the host byte order
    UUID stUUID;

    IMS_MEM_Memcpy(&stUUID, uacHash, sizeof(UUID));

    stUUID.nTimeLow = IPAddress::NToHL(stUUID.nTimeLow);
    stUUID.nTimeMid = IPAddress::NToHS(stUUID.nTimeMid);
    stUUID.nTimeHiAndVersion = IPAddress::NToHS(stUUID.nTimeHiAndVersion);

    // Set the variant & version bits
    stUUID.nTimeHiAndVersion &= 0x0FFF;
    stUUID.nTimeHiAndVersion |= (3 << 12);
    stUUID.nClockSeqHiAndReserved &= 0x3F;
    stUUID.nClockSeqHiAndReserved |= 0x80;

    objUUID.Sprintf("%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            stUUID.nTimeLow, stUUID.nTimeMid, stUUID.nTimeHiAndVersion,
            stUUID.nClockSeqHiAndReserved, stUUID.nClockSeqLow,
            stUUID.abyNode[0], stUUID.abyNode[1], stUUID.abyNode[2],
            stUUID.abyNode[3], stUUID.abyNode[4], stUUID.abyNode[5]);
}

/*

Remarks

*/
PRIVATE GLOBAL
void IMSUUID::GetUUIDv4(IN CONST AString &strRandom, OUT AStringBuffer &objUUID)
{
    UUID stNSID = NAMESPACE_IMS;

    // Put the namespace id in the network byte order
    stNSID.nTimeLow = IPAddress::HToNL(stNSID.nTimeLow);
    stNSID.nTimeMid = IPAddress::HToNS(stNSID.nTimeMid);
    stNSID.nTimeHiAndVersion = IPAddress::HToNS(stNSID.nTimeHiAndVersion);

    // SHA1 hash
    SHA1Context stSHA1;
    IMS_UCHAR uacHash[20];

    IMSSHA1_Initialize(&stSHA1);
    IMSSHA1_Update(reinterpret_cast<const IMS_UCHAR*>(&stNSID), sizeof(UUID), &stSHA1);
    IMSSHA1_Update(reinterpret_cast<const IMS_UCHAR*>(strRandom.GetStr()),
            strRandom.GetLength(), &stSHA1);
    IMSSHA1_Finalize(&stSHA1, uacHash);

    // Convert UUID to the host byte order
    UUID stUUID;

    IMS_MEM_Memcpy(&stUUID, uacHash, sizeof(UUID));

    stUUID.nTimeLow = IPAddress::NToHL(stUUID.nTimeLow);
    stUUID.nTimeMid = IPAddress::NToHS(stUUID.nTimeMid);
    stUUID.nTimeHiAndVersion = IPAddress::NToHS(stUUID.nTimeHiAndVersion);

    // Set the variant & version bits
    stUUID.nTimeHiAndVersion &= 0x0FFF;
    stUUID.nTimeHiAndVersion |= (4 << 12);
    stUUID.nClockSeqHiAndReserved &= 0x3F;
    stUUID.nClockSeqHiAndReserved |= 0x80;

    objUUID.Sprintf("%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            stUUID.nTimeLow, stUUID.nTimeMid, stUUID.nTimeHiAndVersion,
            stUUID.nClockSeqHiAndReserved, stUUID.nClockSeqLow,
            stUUID.abyNode[0], stUUID.abyNode[1], stUUID.abyNode[2],
            stUUID.abyNode[3], stUUID.abyNode[4], stUUID.abyNode[5]);
}

/*

Remarks

*/
PRIVATE GLOBAL
void IMSUUID::GetUUIDv5(IN CONST AString &strName, OUT AStringBuffer &objUUID)
{
    UUID stNSID = NAMESPACE_IMS;

    // Put the namespace id in the network byte order
    stNSID.nTimeLow = IPAddress::HToNL(stNSID.nTimeLow);
    stNSID.nTimeMid = IPAddress::HToNS(stNSID.nTimeMid);
    stNSID.nTimeHiAndVersion = IPAddress::HToNS(stNSID.nTimeHiAndVersion);

    // SHA1 hash
    SHA1Context stSHA1;
    IMS_UCHAR uacHash[20];

    IMSSHA1_Initialize(&stSHA1);
    IMSSHA1_Update(reinterpret_cast<const IMS_UCHAR*>(&stNSID), sizeof(UUID), &stSHA1);
    IMSSHA1_Update(reinterpret_cast<const IMS_UCHAR*>(strName.GetStr()),
            strName.GetLength(), &stSHA1);
    IMSSHA1_Finalize(&stSHA1, uacHash);

    // Convert UUID to the host byte order
    UUID stUUID;

    IMS_MEM_Memcpy(&stUUID, uacHash, sizeof(UUID));

    stUUID.nTimeLow = IPAddress::NToHL(stUUID.nTimeLow);
    stUUID.nTimeMid = IPAddress::NToHS(stUUID.nTimeMid);
    stUUID.nTimeHiAndVersion = IPAddress::NToHS(stUUID.nTimeHiAndVersion);

    // Set the variant & version bits
    stUUID.nTimeHiAndVersion &= 0x0FFF;
    stUUID.nTimeHiAndVersion |= (5 << 12);
    stUUID.nClockSeqHiAndReserved &= 0x3F;
    stUUID.nClockSeqHiAndReserved |= 0x80;

    objUUID.Sprintf("%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            stUUID.nTimeLow, stUUID.nTimeMid, stUUID.nTimeHiAndVersion,
            stUUID.nClockSeqHiAndReserved, stUUID.nClockSeqLow,
            stUUID.abyNode[0], stUUID.abyNode[1], stUUID.abyNode[2],
            stUUID.abyNode[3], stUUID.abyNode[4], stUUID.abyNode[5]);
}
