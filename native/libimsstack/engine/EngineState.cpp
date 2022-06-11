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
#include "base/Ims.h"
#include "EngineState.h"

// Initialization / Uninitialization for Engine
PUBLIC GLOBAL IMS_BOOL EngineState::Initialize()
{
    // Initialize a SipManager
    IMS_BOOL bResult = StaticSip::Initialize();

    // Initialize another function blocks
    Ims::Init();

    return bResult;
}

PUBLIC GLOBAL void EngineState::Uninitialize()
{
    // Releases all the resources in the reverse order of initialization ...

    // Uninitialize a SipManager
    StaticSip::Uninitialize();
}
