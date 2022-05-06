/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20201023  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _OPERATOR_FEATURE_RESOLVER_H_
#define _OPERATOR_FEATURE_RESOLVER_H_

#include "IMSTypeDef.h"

class ISipMessage;

class OperatorFeatureResolver
{
private:
    OperatorFeatureResolver();
    ~OperatorFeatureResolver();

public:
    static IMS_BOOL IsMessageForEarlySessionModel(IN const ISipMessage* piSIPMsg);
};

#endif  // _OPERATOR_FEATURE_RESOLVER_H_
