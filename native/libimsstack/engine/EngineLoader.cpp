/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20170622  hwangoo.park@             Created
    </table>

    Description

*/
#include "StaticSIP.h"
#include "base/IMS.h"
#include "util/SIPConnectionNotifierManager.h"
#include "base/SubscriberTracker.h"
#include "EngineLoader.h"

// It will be called by EnablerThread to load a proper component for each slot.

PUBLIC GLOBAL void EngineLoader::Initialize(IN IMS_SINT32 nSlotId)
{
    // Service
    SubscriberTracker::GetInstance()->InitForSlot(nSlotId);

    // J180
    StaticSIP::InitializeForSlot(nSlotId);

    // J281
    IMS::Init(nSlotId);
    SIPConnectionNotifierManager::Init(nSlotId);
}

PUBLIC GLOBAL void EngineLoader::Uninitialize(IN IMS_SINT32 nSlotId)
{
    // J180
    StaticSIP::UninitializeForSlot(nSlotId);
}
