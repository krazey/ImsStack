#ifndef CONFIG_MTC_H_
#define CONFIG_MTC_H_

#include "ImsTypeDef.h"

/**
 * Configuration for MTC (Multimedia Telephony Call).
 */
class ConfigMtc
{
public:
    ConfigMtc() = delete;

public:
    static const IMS_CHAR APP_NAME[];
    static const IMS_CHAR SERVICE_NAME[];
    static const IMS_CHAR EMERGENCY_SERVICE_NAME[];

    static const IMS_CHAR APP_CONFIG[];
};

#endif
