/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100428  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "StaticSip.h"
#include "base/IMS.h"
#include "EngineState.h"

// Initialization / Uninitialization for Engine
PUBLIC GLOBAL IMS_BOOL EngineState::Initialize()
{
    // Initialize a SIPManager
    IMS_BOOL bResult = StaticSIP::Initialize();

    // Initialize another function blocks
    IMS::Init();

    return bResult;
}

PUBLIC GLOBAL void EngineState::Uninitialize()
{
    // Releases all the resources in the reverse order of initialization ...

    // Uninitialize a SIPManager
    StaticSIP::Uninitialize();
}
