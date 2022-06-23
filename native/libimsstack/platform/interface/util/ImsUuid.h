/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20120214  hwangoo.park@             Created
    </table>

    Description
*/

#ifndef _IMS_UUID_H_
#define _IMS_UUID_H_

#include "AStringBuffer.h"

#define ImsUuid IMSUUID
#define GetUuid GetUUID

class IMSUUID
{
public:
    enum
    {
        // Time-based version (based on platform)
        VERSION_1,
        // DCE security version with embedded POSIX UIDs - NOT IMPLEMENTED
        VERSION_2,
        // Name-based version (MD5 hashing)
        VERSION_3,
        // Randomly or pseudo-randomly generated version - NOT IMPLEMENTED
        VERSION_4,
        // Name-based version (SHA-1 hashing) - NOT IMPLEMENTED
        VERSION_5
    };

private:
    IMSUUID();

public:
    static AString GetUUID(
            IN IMS_SINT32 nVersion = VERSION_4, IN CONST AString& strName = AString::ConstNull());

private:
    static void GetUUIDv3(IN CONST AString& strName, OUT AStringBuffer& objUUID);
    static void GetUUIDv4(IN CONST AString& strRandom, OUT AStringBuffer& objUUID);
    static void GetUUIDv5(IN CONST AString& strName, OUT AStringBuffer& objUUID);
};

#endif  // _IMS_UUID_H_
