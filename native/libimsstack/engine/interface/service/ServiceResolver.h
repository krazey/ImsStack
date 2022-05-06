#ifndef _SERVICE_RESOLVER_H_
#define _SERVICE_RESOLVER_H_

#include "IRegBinding.h"

/**
 * @brief This class is an interface to get/set IMS registration binding with IMS service.
 */
class ServiceResolver
{
private:
    ServiceResolver();

public:
    /**
     * @brief Returns the IRegBinding which matches the specified value, strAppId & strServiceId.
     *
     * @param nSlotId slot id to be retrieved
     * @param strAppId an IMS application identifier
     * @param strServiceId an IMS service identifier
     * @return Pointer to IRegBinding
     */
    static IRegBinding* GetRegBinding(
            IN IMS_SINT32 nSlotId, IN const AString& strAppId, IN const AString& strServiceId);

    /**
     * @brief Returns all the registered IRegBinding.
     *
     * @param nSlotId slot id to be retrieved
     * @param strAppId an IMS application identifier
     * @param strServiceId an IMS service identifier
     * @return List of pointer to IRegBinding
     */
    static IMSList<IRegBinding*> GetRegBindings(IN IMS_SINT32 nSlotId);

    /**
     * @brief Returns all the registered IRegBinding.
     *
     * @param nSlotId slot id to be set
     * @param strAppId an IMS application identifier
     * @param strServiceId an IMS service identifier
     * @param piRegBinding Pointer to IRegBinding to be set
     */
    static void SetRegBinding(IN IMS_SINT32 nSlotId, IN const AString& strAppId,
            IN const AString& strServiceId, IN IRegBinding* piRegBinding);
};

#endif  // _SERVICE_RESOLVER_H_
