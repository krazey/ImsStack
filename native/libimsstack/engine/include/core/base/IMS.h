/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090716  lovil@                    Created
    </table>

    Description

*/

#ifndef _IMS_H_
#define _IMS_H_

#include "base/IMSError.h"

class IMS
{
public:
    static void Init();
    static void Init(IN IMS_SINT32 nSlotId);

    static void SetLastError(IN IMS_SINT32 nErrorCode);
    static IMS_SINT32 GetLastError();

private:
    static void SetLastError(IN IMS_SINT32 nErrorCode, IN IMS_SINT32 nSlotId);
    static IMS_SINT32 GetLastError(IN IMS_SINT32 nSlotId);

private:
    static IMS_SINT32* ERROR_CODE;  // 0 means there is no error
};

#endif  // _IMS_H_
