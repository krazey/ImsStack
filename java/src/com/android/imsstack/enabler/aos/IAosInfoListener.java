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

public interface IAosInfoListener {

    /**
     * Notify the application that AoS ISIM state.
     *
     * @param state {@code state} is type of {@link IsimState}.
     */
    public void notifyAosIsimStateChanged(int state);

    /**
     * ISIM State
     */
    class IsimState {

        public static final int INVALID = 0;
        public static final int VALID = 1;
        public static final int REFRESH_STARTED = 2;
        public static final int REFRESH_COMPLETE = 3;
    }
}