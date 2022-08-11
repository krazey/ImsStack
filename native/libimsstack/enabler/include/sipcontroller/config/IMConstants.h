#ifndef _IM_CONSTANTS_H_
#define _IM_CONSTANTS_H_

#include "IMSTypeDef.h"

class IMConstants
{
public:
    static const IMS_CHAR TAG_OMA_IM[];
    static const IMS_CHAR TAG_FILETRANSFER[];
    static const IMS_CHAR TAG_HTTP_FILETRANSFER[];
    static const IMS_CHAR TAG_GEOLOCATIONPUSH[];

    static const IMS_CHAR TAG_CPM_SESSION[];
    static const IMS_CHAR TAG_CPM_MSG[];
    static const IMS_CHAR TAG_CPM_DEFERRED[];
    static const IMS_CHAR TAG_CPM_LARGE_MSG[];
    static const IMS_CHAR TAG_CPM_FILE_TRANSFER[];
    static const IMS_CHAR TAG_CPM_SYSTEM_MSG[];

    static const IMS_CHAR TAG_CPM_CFS[];
    static const IMS_CHAR TAG_CPM_NFS[];
};

#endif  // _IM_CONSTANTS_H_
