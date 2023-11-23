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

package com.android.imsstack.enabler.mtc;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import android.os.Parcel;
import android.testing.AndroidTestingRunner;

import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidTestingRunner.class)
public class SuppInfoTest {
    @Test
    public void testConstructor() {
        SuppInfo suppInfo1 = new SuppInfo();

        assertEquals(0, suppInfo1.getServiceSize());

        suppInfo1.addService_int(SuppInfo.TYPE_CALLERID, SuppInfo.CALLERID_IDENTITY);

        assertEquals(1, suppInfo1.getServiceSize());
        assertTrue(suppInfo1.isService(SuppInfo.TYPE_CALLERID));

        SuppInfo copiedSuppInfo1 = new SuppInfo(suppInfo1);

        assertEquals(1, copiedSuppInfo1.getServiceSize());
        assertTrue(suppInfo1.isService(SuppInfo.TYPE_CALLERID));
        assertFalse(suppInfo1.isService(SuppInfo.TYPE_CNAP));

        Parcel dest = Parcel.obtain();
        dest.writeInt(3);
        dest.writeInt(SuppInfo.TYPE_CALLERID);
        dest.writeString("");
        dest.writeInt(SuppInfo.CALLERID_IDENTITY);
        dest.writeInt(0);
        dest.writeInt(SuppInfo.TYPE_CNAP);
        dest.writeString("test");
        dest.writeInt(0);
        dest.writeInt(0);
        dest.writeInt(SuppInfo.TYPE_GTT);
        dest.writeString("");
        dest.writeInt(0);
        dest.writeInt(1);
        dest.setDataPosition(0);

        SuppInfo suppInfo2 = new SuppInfo(dest);

        assertEquals(3, suppInfo2.getServiceSize());

        Parcel parcelWrittenBySuppInfo = Parcel.obtain();
        suppInfo2.writeToParcel(parcelWrittenBySuppInfo, 0);
        parcelWrittenBySuppInfo.setDataPosition(0);

        SuppInfo copiedSuppInfo = new SuppInfo(parcelWrittenBySuppInfo);

        assertEquals(3, copiedSuppInfo.getServiceSize());
        assertTrue(copiedSuppInfo.isService(SuppInfo.TYPE_CALLERID));
        assertTrue(copiedSuppInfo.isService(SuppInfo.TYPE_CNAP));
        assertTrue(copiedSuppInfo.isService(SuppInfo.TYPE_GTT));
    }
}
