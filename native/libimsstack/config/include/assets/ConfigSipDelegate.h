#ifndef CONFIG_SIP_DELEGATE_H_
#define CONFIG_SIP_DELEGATE_H_

#include "ImsTypeDef.h"

/**
 * Configuration for SipDelegate
 * (RCS SIP signaling handling when RCS single registration is enabled).
 */
class ConfigSipDelegate
{
public:
    ConfigSipDelegate() = delete;

public:
    static const IMS_CHAR APP_NAME[];
    static const IMS_CHAR SERVICE_NAME[];

    static const IMS_CHAR APP_CONFIG[];
};

#endif
