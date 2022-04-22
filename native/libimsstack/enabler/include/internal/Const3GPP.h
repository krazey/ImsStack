/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20130318  hwangoo.park@             Created
    </table>

    Description
     This class defines the constant values for 3GPP specifications.
*/

#ifndef _CONST_3GPP_H_
#define _CONST_3GPP_H_

#include "ImsTypeDef.h"

class Const3GPP
{
public:
    //// FEATURE TAG
    // SMS over IMS
    static const IMS_CHAR FEATURE_TAG_SMSIP[];

    //// IARI

    //// ICSI
    // Multimedia Telephony
    static const IMS_CHAR ICSI_MMTEL[];
};

#endif // _CONST_3GPP_H_
