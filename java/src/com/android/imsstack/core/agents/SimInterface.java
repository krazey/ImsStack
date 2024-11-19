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

import android.annotation.NonNull;

import java.util.List;

/**
 * This interface provides the SIM related operations to get the SIM related information
 * or request any operations from the internal components.
 */
public interface SimInterface extends IAgent {
    /**
     * Returns the slot-id of the current SIM.
     */
    int getSlotId();

    /**
     * Returns the sub-id of the current SIM.
     */
    int getSubId();

    /**
     * Returns the current SIM card state.
     *
     * @return The SIM card state. Valid values are
     *         {@link Sim#STATE_UNKNOWN},
     *         {@link Sim#STATE_ABSENT},
     *         {@link Sim#STATE_PRESENT}.
     */
    @Sim.State int getSimCardState();

    /**
     * Returns the current SIM state.
     *
     * @return The SIM state. Valid values are
     *         {@link Sim#STATE_UNKNOWN},
     *         {@link Sim#STATE_ABSENT},
     *         {@link Sim#STATE_PRESENT},
     *         {@link Sim#STATE_NOT_READY},
     *         {@link Sim#STATE_LOCKED},
     *         {@link Sim#STATE_LOADED}.
     */
    @Sim.State int getSimState();

    /**
     * Checks whether the current SIM state is in a {@link Sim#STATE_LOADED} state.
     *
     * @return {@code true} if the current SIM state is in a {@link Sim#STATE_LOADED} state,
     *         {@code false} otherwise.
     */
    boolean isSimLoaded();

    /**
     * Checks whether the SIM is fully loaded.
     *
     * The fully loaded SIM states are:
     *      {@link Sim#STATE_ABSENT},
     *      {@link Sim#STATE_LOCKED},
     *      {@link Sim#STATE_LOADED}
     *
     * @return {@code true} if the SIM state is a final state, {@code false} otherwise.
     */
    boolean isSimLoadCompleted();

    /**
     * Returns the USIM service table that contains a bit field of enabled services.
     *
     * @return The value of the USIM service table or an empty byte array if not available.
     */
    @NonNull byte[] getUsimServiceTable();

    /**
     * Returns the SMS center address.
     *
     * @return The SMS center address.
     */
    String getSmscAddress();

    /**
     * Adds a listener to monitor the SIM state change.
     *
     * @param listener The listener to be set.
     */
    void addListener(Sim.Listener listener);

    /**
     * Removes the listener that was previously set.
     *
     * @param listener The listener to be removed.
     */
    void removeListener(Sim.Listener listener);

    /**
     * Returns USAT interface.
     *
     * @return The USAT interface.
     */
    UsatInterface getUsatInterface();

    /**
     * Returns the current ISIM state.
     *
     * @return The ISIM state. Valid values are
     *         {@link Sim#ISIM_STATE_UNKNOWN},
     *         {@link Sim#ISIM_STATE_NOT_PRESENT},
     *         {@link Sim#ISIM_STATE_NOT_READY},
     *         {@link Sim#ISIM_STATE_LOADED},
     *         {@link Sim#ISIM_STATE_REFRESH_STARTED}.
     */
    @Sim.IsimState int getIsimState();

    /**
     * Checks if ISIM is loaded or not.
     *
     * @return true if ISIM state is loaded, false otherwise.
     */
    boolean isIsimLoaded();

    /**
     * Returns the IMS Service Table (IST) that was loaded from the ISIM.
     *
     * @return The IMS service table or null if not present or not loaded.
     */
    byte[] getIsimServiceTable();

    /**
     * Returns the IMS private user identity (IMPI) that was loaded from the ISIM.
     *
     * @return The IMPI string or null if not present or not loaded.
     */
    String getIsimImpi();

    /**
     * Returns the IMS home network domain name that was loaded from the ISIM.
     *
     * @return The IMS home network domain name or null if not present or not loaded.
     */
    String getIsimDomain();

    /**
     * Returns the IMS public user identities (IMPU) that were loaded from the ISIM.
     *
     * @return A list of IMPU string or empty list if not present or not loaded.
     */
    @NonNull List<String> getIsimImpu();

    /**
     * Checks if GBA is available in the ISIM service table.
     *
     * @return true if GBA is available in the ISIM, false otherwise.
     */
    boolean isGbaAvailable();

    /**
     * Adds a listener to monitor the SIM state change.
     *
     * @param listener The listener to be set.
     */
    void addIsimListener(Sim.IsimListener listener);

    /**
     * Removes the listener that was previously set.
     *
     * @param listener The listener to be removed.
     */
    void removeIsimListener(Sim.IsimListener listener);
}
