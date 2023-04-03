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
package com.android.imsstack.system;

/**
 * An interface for providing the call related information.
 */
public interface ISystemAPICallInfo {
    /**
     * Checks whether the specified number is an emergency number or not.
     *
     * @param number The number to be checked.
     * @return {@code true} if the specified number is an emergency number, {@code false} otherwise.
     */
    int isEmergencyNumber(String number);

    /**
     * Returns the call state of the other slot.
     */
    int getCallStateInOtherSlot();
}
