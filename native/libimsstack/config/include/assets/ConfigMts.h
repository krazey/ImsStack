#ifndef CONFIG_MTS_H_
#define CONFIG_MTS_H_

#include "ImsTypeDef.h"

/**
 * Configuration for MTS (Multimedia Telephony SMS).
 */
class ConfigMts
{
public:
    ConfigMts() = delete;

public:
    static const IMS_CHAR APP_NAME[];
    static const IMS_CHAR SERVICE_NAME[];
    static const IMS_CHAR EMERGENCY_SERVICE_NAME[];

    static const IMS_CHAR APP_CONFIG[];
};

#endif
