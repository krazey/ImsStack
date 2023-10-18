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

public interface SubsInfoInterface extends IAgent {
    /** Returns the primary IMPU string. */
    String getPrimaryImpu();
    /** Checks if IMS is enabled or not. */
    boolean isImsEnabled();
    /** Checks if ISIM is enabled for IMS service or not. */
    boolean isIsimEnabled();
    /** Checks if USIM is enabled for IMS service or not. */
    boolean isUsimEnabled();
    /** Checks if the debug mode is enabled or not. */
    boolean isDebugEnabled();
    /** Checks if the testmode is enabled or not. */
    boolean isTestModeEnabled();
}
