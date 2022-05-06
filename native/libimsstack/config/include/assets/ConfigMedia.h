#ifndef CONFIG_MEDIA_H_
#define CONFIG_MEDIA_H_

#include "ImsTypeDef.h"

/**
 * Configuration for media capabilities of IMS engine.
 */
class ConfigMedia
{
public:
    ConfigMedia() = delete;

public:
#define CONFIG_MEDIA_MTC "ims.media.mtc"

    static const IMS_CHAR MEDIA_NAME[];
    static const IMS_CHAR MEDIA_CAPABILITIES_NAME[];

    static const IMS_CHAR MEDIA_CONFIG[];
    static const IMS_CHAR MEDIA_CAPABILITIES_CONFIG[];
};

#endif
