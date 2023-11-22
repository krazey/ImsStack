/*
 * Copyright (C) 2023 The Android Open Source Project
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

package com.android.imsstack.enabler.mtc.externalcalls;

import static org.junit.Assert.assertEquals;

import android.os.Parcel;
import android.testing.AndroidTestingRunner;

import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidTestingRunner.class)
public class ExternalCallsTest {
    @Test
    public void testConstructor() {
        Parcel dest1 = Parcel.obtain();
        dest1.writeInt(1);
        dest1.writeString("123");
        dest1.writeString("DEF");
        dest1.writeString("GHI");
        dest1.writeInt(1);
        dest1.writeInt(1);
        dest1.writeInt(1);
        dest1.writeInt(1);
        dest1.setDataPosition(0);

        ExternalCalls externalCalls1 = new ExternalCalls(dest1);

        assertEquals(1, externalCalls1.getImsExternalCallStates().size());
        assertEquals(123, externalCalls1.getImsExternalCallState(0).getCallId());

        Parcel dest2 = Parcel.obtain();
        dest2.writeInt(2);
        dest2.writeString("123");
        dest2.writeString("DEF");
        dest2.writeString("GHI");
        dest2.writeInt(1);
        dest2.writeInt(1);
        dest2.writeInt(1);
        dest2.writeInt(1);
        dest2.writeString("456");
        dest2.writeString("MNO");
        dest2.writeString("PQR");
        dest2.writeInt(0);
        dest2.writeInt(0);
        dest2.writeInt(0);
        dest2.writeInt(0);
        dest2.setDataPosition(0);

        ExternalCalls externalCalls2 = new ExternalCalls(dest2);

        assertEquals(2, externalCalls2.getImsExternalCallStates().size());

        ExternalCalls copiedExternalCalls = new ExternalCalls(externalCalls2);

        assertEquals(2, copiedExternalCalls.getImsExternalCallStates().size());

        Parcel parcelWrittenByExternalCalls = Parcel.obtain();
        copiedExternalCalls.writeToParcel(parcelWrittenByExternalCalls, 0);
        parcelWrittenByExternalCalls.setDataPosition(0);

        ExternalCalls externalCalls3 = new ExternalCalls(parcelWrittenByExternalCalls);

        assertEquals(2, externalCalls3.getImsExternalCallStates().size());
    }
}
