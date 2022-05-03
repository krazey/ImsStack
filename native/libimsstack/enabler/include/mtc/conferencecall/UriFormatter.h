#ifndef URI_FORMATTER_H_
#define URI_FORMATTER_H_

class IMtcCallContext;

class UriFormatter
{
public:
    static AString& GetReferToForInvite(OUT AString& strUri, IN IMtcCallContext& objContext);
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
