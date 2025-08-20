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

package com.android.imsstack.enabler.mtc;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import android.os.Parcel;
import android.testing.AndroidTestingRunner;

import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidTestingRunner.class)
public class PermanentSuppInfoTest {
    @Test
    public void testConstructor() {
        PermanentSuppInfo permanentSuppInfo1 = new PermanentSuppInfo();

        assertEquals(0, permanentSuppInfo1.getServicesSize());

        permanentSuppInfo1.addServiceBool(
                PermanentSuppInfo.SUPP_TYPE_TB_CB_OUTGOING_ALL_VOICE, true);

        assertEquals(1, permanentSuppInfo1.getServicesSize());
        assertTrue(permanentSuppInfo1.isService(
                PermanentSuppInfo.SUPP_TYPE_TB_CB_OUTGOING_ALL_VOICE));

        PermanentSuppInfo copiedPermanentSuppInfo1 = new PermanentSuppInfo(permanentSuppInfo1);

        assertEquals(1, copiedPermanentSuppInfo1.getServicesSize());
        assertTrue(copiedPermanentSuppInfo1.isService(
                PermanentSuppInfo.SUPP_TYPE_TB_CB_OUTGOING_ALL_VOICE));
        assertFalse(copiedPermanentSuppInfo1.isService(PermanentSuppInfo.SUPP_TYPE_TB_CW));

        Parcel dest = Parcel.obtain();
        dest.writeInt(2);
        dest.writeInt(PermanentSuppInfo.SUPP_TYPE_TB_TIR);
        dest.writeString("");
        dest.writeInt(0);
        dest.writeInt(1);
        dest.writeInt(PermanentSuppInfo.SUPP_TYPE_TB_CB_INCOMING_ROAMING_VOICE);
        dest.writeString("");
        dest.writeInt(0);
        dest.writeInt(0);
        dest.setDataPosition(0);

        PermanentSuppInfo permanentSuppInfo2 = new PermanentSuppInfo(dest);

        assertTrue(permanentSuppInfo2.isService(PermanentSuppInfo.SUPP_TYPE_TB_TIR));
        assertTrue(permanentSuppInfo2.isService(
                PermanentSuppInfo.SUPP_TYPE_TB_CB_INCOMING_ROAMING_VOICE));

        Parcel parcelWrittenBySuppInfo = Parcel.obtain();
        permanentSuppInfo2.writeToParcel(parcelWrittenBySuppInfo, 0);
        parcelWrittenBySuppInfo.setDataPosition(0);

        PermanentSuppInfo copiedPermanentSuppInfo2 = new PermanentSuppInfo(parcelWrittenBySuppInfo);

        assertEquals(2, copiedPermanentSuppInfo2.getServicesSize());
        assertTrue(copiedPermanentSuppInfo2.isService(PermanentSuppInfo.SUPP_TYPE_TB_TIR));
        assertTrue(copiedPermanentSuppInfo2.isService(
                PermanentSuppInfo.SUPP_TYPE_TB_CB_INCOMING_ROAMING_VOICE));
    }
}
