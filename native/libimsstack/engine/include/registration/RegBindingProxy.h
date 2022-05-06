/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20101207  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _REG_BINDING_PROXY_H_
#define _REG_BINDING_PROXY_H_

#include "AString.h"

class CallerCapability;
class IRegistrationEx;
class IRegContact;

class RegBindingProxy
{
private:
    RegBindingProxy();

public:
    // Create / Destroy
    static IMS_BOOL CreateBinding(IN IMS_SINT32 nSlotId, IN const AString& strAppId,
            IN const AString& strServiceId, IN IRegistrationEx* piRegEx);
    static void DestroyBinding(
            IN IMS_SINT32 nSlotId, IN const AString& strAppId, IN const AString& strServiceId);
    static void DestroyBinding(IN IMS_SINT32 nSlotId, IN IRegistrationEx* piRegEx);

    // Contact binding
    static IMS_BOOL BindContact(IN IMS_SINT32 nSlotId, IN const AString& strAppId,
            IN const AString& strServiceId, IN IRegContact* piContact);
    static void UnbindContact(
            IN IMS_SINT32 nSlotId, IN const AString& strAppId, IN const AString& strServiceId);
    static void UnbindContact(IN IMS_SINT32 nSlotId, IN IRegContact* piContact);

    // Additional informations
    static void QueryCapability(IN IMS_SINT32 nSlotId, IN const AString& strAppId,
            IN const AString& strServiceId, OUT CallerCapability*& pCapability);
    static void QueryRegistrationHeaders(IN IMS_SINT32 nSlotId, IN const AString& strAppId,
            IN const AString& strServiceId, OUT AStringArray& objHeaders);
};

#endif  // _REG_BINDING_PROXY_H_
