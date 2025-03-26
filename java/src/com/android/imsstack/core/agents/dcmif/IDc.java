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

package com.android.imsstack.core.agents.dcmif;

import android.content.Context;

import androidx.annotation.NonNull;

import com.android.imsstack.util.IndentingPrintWriter;

/**
 * This is interface for Data Connection agents.
*/
public interface IDc {
    /**
     * Initialize object
     *
     * @param context Context
     */
    void init(Context context);

    /**
     * Clean up ojbejct
     */
    void cleanup();

    /**
     * Dumps this instance into a readable format for dumpsys usage.
     *
     * @param pw A {@link IndentingPrintWriter} object used to write the formatted logs.
     */
    default void dump(@NonNull IndentingPrintWriter pw) {
    }
}
