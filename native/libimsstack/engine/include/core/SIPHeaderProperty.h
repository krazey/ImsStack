/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100426  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _SIP_HEADER_PROPERTY_H_
#define _SIP_HEADER_PROPERTY_H_

// Property of the SIP headers for the engine
//    : It is usually used by the refresh helper (subscription/publication/ ...).
struct SIPHeaderProperty
{
    IMS_SINT32 nType;
    const IMS_CHAR* pszName;
    IMS_BOOL bSingleHeader;
};

#endif  // _SIP_HEADER_PROPERTY_H_
