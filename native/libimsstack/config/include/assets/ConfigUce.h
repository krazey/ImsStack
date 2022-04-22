#ifndef CONFIG_UCE_H_
#define CONFIG_UCE_H_

#include "ImsTypeDef.h"

/**
 * Configuration for UCE (User Capability Exchange).
 */
class ConfigUce
{
public:
    ConfigUce() = delete;

public:
    static const IMS_CHAR APP_NAME[];
    static const IMS_CHAR SERVICE_NAME[];

    static const IMS_CHAR APP_CONFIG[];
};

#endif
