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
package com.android.imsstack.enabler.aos;

import android.annotation.IntDef;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

public interface IAosInfoListener {

    /**
     * Notify the application that AoS ISIM state.
     *
     * @param state {@code state} is type of {@link IsimState.IsimStateDef}.
     */
    void notifyAosIsimStateChanged(@IsimState.IsimStateDef int state);

    /**
     * ISIM State
     */
    class IsimState {

        public static final int INVALID = 0;
        public static final int VALID = 1;
        public static final int REFRESH_STARTED = 2;
        public static final int REFRESH_COMPLETE = 3;

        /** @hide */
        @Retention(RetentionPolicy.SOURCE)
        @IntDef({
                IsimState.INVALID,
                IsimState.VALID,
                IsimState.REFRESH_STARTED,
                IsimState.REFRESH_COMPLETE
        })
        public @interface IsimStateDef {}

        private IsimState() {}

        /**
         * @param state The state code.
         * @return A string representation of the ISIM state.
         */
        public static String toString(@IsimStateDef int state) {
            return switch (state) {
                case INVALID -> "INVALID";
                case VALID -> "VALID";
                case REFRESH_STARTED -> "REFRESH_STARTED";
                case REFRESH_COMPLETE -> "REFRESH_COMPLETE";
                default -> "UNKNOWN(" + state + ")";
            };
        }
    }
}
