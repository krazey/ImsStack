#ifndef _ENGINE_LOADER_H_
#define _ENGINE_LOADER_H_

#include "IMSTypeDef.h"

/**
 * @brief This class provides an interface to initialize/uninitialize the IMS engine.
 *
 * It will be called by EnablerThread to load a proper component for each slot.
 */
class EngineLoader
{
public:
    /**
     * @brief Initializes the engine with the given slot id.
     *
     * @param nSlotId The slot id to be initialized
     */
    static void Initialize(IN IMS_SINT32 nSlotId);
    /**
     * @brief Uninitializes the engine with the given slot id.
     *
     * @param nSlotId The slot id to be uninitialized
     */
    static void Uninitialize(IN IMS_SINT32 nSlotId);
};

#endif  // _ENGINE_LOADER_H_
