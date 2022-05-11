#ifndef __SIP_P_CHARGING_FUNCTION_ADDRESSES_HEADER_H__
#define __SIP_P_CHARGING_FUNCTION_ADDRESSES_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipPChargingFunctionAddressesHeader : public SipHeaderBase
{
private:
    /*Header Value*/
    SipNameValue* m_pChargeAddr;

public:
    /*constructor*/
    SipPChargingFunctionAddressesHeader();
    SipPChargingFunctionAddressesHeader(const SipPChargingFunctionAddressesHeader& objHeader);

    /*destructor*/
    ~SipPChargingFunctionAddressesHeader();
    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);
    /*virtual methods*/
    SIP_BOOL Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const override;
    /*Function for encoding of headers*/
    SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);

    /*Function for decoding of headers*/
    SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

    inline SIP_BOOL IsValidHeader() const
    {
        return (m_pChargeAddr == SIP_NULL) ? SIP_FALSE : SIP_TRUE;
    }
};
#endif  //__SIP_P_CHARGING_FUNCTION_ADDRESSES_HEADER_H__
