#ifndef MTS_SM_UTIL_H_
#define MTS_SM_UTIL_H_

#include "ByteArray.h"
#include "IMtsClient.h"

class MtsSmUtils final
{
public:
    MtsSmUtils();
    ~MtsSmUtils();

    static MtsSmUtils* GetInstance(IN IMS_SINT32 nSlotId);
    IMS_SINT32 GetRpMr(IN const IMS_BYTE* pbySmsData);
    IMS_SINT32 GetRpMr(IN const ByteArray& objSmsData);
    IMS_SINT32 GetMti(IN const IMS_UINT32 nSmsType, IN const IMS_BYTE* objSms);
    IMS_SINT32 GetMti(IN const IMS_UINT32 nSmsType, IN const ByteArray& objSmsData);
    void PrintSmsDataBurst(IN const ByteArray& objSmsData);
    static const IMS_CHAR* GetMtiStringFrom3gpp(IN const IMS_SINT32 nMti);
    static const IMS_CHAR* GetMtiStringFrom3gpp2(IN const IMS_SINT32 nMti);

public:
    enum
    {
        MTS_SMS_FORMAT_3GPP = IMtsClient::SMSFORMAT_3GPP,
        MTS_SMS_FORMAT_3GPP2 = IMtsClient::SMSFORMAT_3GPP2,
        MTS_SMS_FORMAT_INVALID = IMtsClient::SMSFORMAT_INVALID
    };

    enum
    {
        MTS_SMS_TRX_TYPE_SEND = 1,
        MTS_SMS_TRX_TYPE_RECEIVE,
        MTS_SMS_TRX_TYPE_INVALID
    };

    enum
    {
        MTS_SMS_MTI_NONE = -1
    };

    // RP Data Unit type in 3GPP SMS
    enum
    {
        MTS_3GPP_MTI_RP_DATA_From_MS = 0,
        MTS_3GPP_MTI_RP_DATA_From_N = 1,
        MTS_3GPP_MTI_RP_ACK_From_MS = 2,
        MTS_3GPP_MTI_RP_ACK_From_N = 3,
        MTS_3GPP_MTI_RP_ERROR_From_MS = 4,
        MTS_3GPP_MTI_RP_ERROR_From_N = 5,
        MTS_3GPP_MTI_RP_SMMA = 6
    };

    // Bearer Data Unit type in 3GPP2 SMS
    enum
    {
        MTS_3GPP2_MTI_SMS_POINT_TO_POINT = 0,
        MTS_3GPP2_MTI_SMS_BROADCAST = 1,
        MTS_3GPP2_MTI_SMS_ACKNOWLEDGE = 2
    };

private:
    static MtsSmUtils* m_pMtsSmUtil;
};

#endif
