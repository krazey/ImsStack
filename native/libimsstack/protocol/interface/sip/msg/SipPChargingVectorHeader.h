#ifndef __SIP_P_CHARGING_VECTOR_HEADER_H__
#define __SIP_P_CHARGING_VECTOR_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipPChargingVectorHeader : public SipHeaderBase
{
private:
    SipNameValue* m_pChargingVectorList;

public:
    /*constructor*/
    SipPChargingVectorHeader();
    SipPChargingVectorHeader(const SipPChargingVectorHeader& objHeader);

    /*destructor*/
    ~SipPChargingVectorHeader();
    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);
    /*virtual methods*/
    SIP_BOOL Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const override;
    /*Function for encoding of headers*/
    SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);

    /*Function for decoding of headers*/
    SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

    inline SIP_BOOL IsValidHeader() const
    {
        return (m_pChargingVectorList == SIP_NULL) ? SIP_FALSE : SIP_TRUE;
    }
};
#endif  //__SIP_P_CHARGING_VECTOR_HEADER_H__
