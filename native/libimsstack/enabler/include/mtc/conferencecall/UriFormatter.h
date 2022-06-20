#ifndef URI_FORMATTER_H_
#define URI_FORMATTER_H_

#include "ImsTypeDef.h"
#include "MtcDef.h"

class AString;
class IMtcCallContext;

class UriFormatter
{
public:
    static AString& GetReferToForInvite(OUT AString& strUri, IN IMtcCallContext& objContext,
            IN IMS_BOOL bEnforcePaid = IMS_FALSE);
    static AString& GetReferToForInvite(
            OUT AString& strUri, IN IMtcCallContext& objContext, IN const ConfUser* pConfUser);
    static AString& GetReferToForBye(
            OUT AString& strUri, IN const ConfUser* pConfUser, IN const AString& strInviteduri);

private:
    static void ConvertToValidSipUri(IN_OUT AString& strUri, IN IMtcCallContext& objContext);

private:
    static const IMS_CHAR STR_USER_PHONE[];
};

#endif
