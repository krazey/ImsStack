/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20170622  hwangoo.park@             Created
    </table>

    Description

*/
#include "StaticSip.h"
#include "base/Ims.h"
#include "util/SipConnectionNotifierManager.h"
#include "base/SubscriberTracker.h"
#include "EngineLoader.h"

// It will be called by EnablerThread to load a proper component for each slot.

PUBLIC GLOBAL void EngineLoader::Initialize(IN IMS_SINT32 nSlotId)
{
    // Service
    SubscriberTracker::GetInstance()->InitForSlot(nSlotId);

    // sipcore
    StaticSip::InitializeForSlot(nSlotId);

    // core
    Ims::Init(nSlotId);
    SipConnectionNotifierManager::Init(nSlotId);
}

PUBLIC GLOBAL void EngineLoader::Uninitialize(IN IMS_SINT32 nSlotId)
{
    // sipcore
    StaticSip::UninitializeForSlot(nSlotId);
}
