/*
 * Copyright (C) 2025 The Android Open Source Project
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

import static org.junit.Assert.assertEquals;

import android.testing.AndroidTestingRunner;

import com.android.imsstack.enabler.aos.IAosInfoListener.IsimState;

import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidTestingRunner.class)
public class IAosInfoListenerTest {

    @Test
    public void isimState_toString_shouldReturnCorrectNamesForKnownStates() {
        assertEquals("INVALID", IsimState.toString(IsimState.INVALID));
        assertEquals("VALID", IsimState.toString(IsimState.VALID));
        assertEquals("REFRESH_STARTED", IsimState.toString(IsimState.REFRESH_STARTED));
        assertEquals("REFRESH_COMPLETE", IsimState.toString(IsimState.REFRESH_COMPLETE));
    }

    @Test
    public void isimState_toString_shouldReturnUnknownForDefault() {
        int unknownPositiveState = 99;
        String expectedPositiveString = "UNKNOWN(" + unknownPositiveState + ")";
        assertEquals(expectedPositiveString, IsimState.toString(unknownPositiveState));

        int unknownNegativeState = -1;
        String expectedNegativeString = "UNKNOWN(" + unknownNegativeState + ")";
        assertEquals(expectedNegativeString, IsimState.toString(unknownNegativeState));
    }
}
