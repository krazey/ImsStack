/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100428  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _STATIC_SIP_H_
#define _STATIC_SIP_H_

#include "IMSTypeDef.h"

// Initialization / Uninitialization for SIP manager
class StaticSIP
{
public:
    static IMS_BOOL Initialize();
    static void Uninitialize();

    static void InitializeForSlot(IN IMS_SINT32 nSlotId);
    static void UninitializeForSlot(IN IMS_SINT32 nSlotId);
};

#endif  // _STATIC_SIP_H_
