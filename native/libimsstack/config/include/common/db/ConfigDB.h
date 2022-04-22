/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20101010  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _CONFIG_DB_H_
#define _CONFIG_DB_H_

#include "ImsTypeDef.h"

class ConfigDB
{
private:
    ConfigDB();

public:
    // DB field name : "id" - unique row identifier of db table
    static const IMS_CHAR FIELD_NAME_ID[];
    // DB field name : "property" - ReadOnly : "0", ReadWrite : "1"
    static const IMS_CHAR FIELD_NAME_PROPERTY[];
    // DB field name : "conf"
    static const IMS_CHAR FIELD_NAME_CONF[];

    enum
    {
        PROPERTY_READONLY = 0,
        PROPERTY_READWRITE
    };
};

#endif // _CONFIG_DB_H_
