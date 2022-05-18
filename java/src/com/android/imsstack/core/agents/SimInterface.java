/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.imsstack.core.agents;

import com.android.imsstack.core.agents.agentif.IAgent;

/**
 * This interface provides the SIM related operations to get the SIM related information
 * or request any operations from the internal components.
 */
public interface SimInterface extends IAgent {
    /**
     * Returns the slot-id of this interface.
     *
     * @return The slot-id of this interface.
     */
    int getSlotId();

    /**
     * Returns USAT interface.
     *
     * @return The USAT interface.
     */
    UsatInterface getUsatInterface();

    /**
     * Returns the USIM service table that contains a bit field of enabled services.
     *
     * @return The value of USIM service table.
     */
    byte[] getUsimServiceTable();
}
