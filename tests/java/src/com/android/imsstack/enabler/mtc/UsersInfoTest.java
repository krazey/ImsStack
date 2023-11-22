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

package com.android.imsstack.enabler.mtc.conf;

import static org.junit.Assert.assertEquals;

import android.os.Parcel;
import android.testing.AndroidTestingRunner;

import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidTestingRunner.class)
public class UsersInfoTest {
    @Test
    public void testConstructor() {
        UsersInfo usersInfo1 = new UsersInfo();

        assertEquals(0, usersInfo1.getSize());

        usersInfo1.addUser(new UsersInfo.User());

        assertEquals(1, usersInfo1.getSize());

        UsersInfo copiedUsersInfo = new UsersInfo(usersInfo1);

        assertEquals(1, copiedUsersInfo.getSize());

        Parcel dest = Parcel.obtain();
        dest.writeInt(1);
        dest.writeLong(123);
        dest.writeString("ABC");
        dest.writeString("DEF");
        dest.writeString("GHI");
        dest.writeString("JKL");
        dest.writeInt(0);
        dest.writeInt(0);
        dest.writeInt(0);
        dest.writeInt(0);
        dest.setDataPosition(0);

        UsersInfo usersInfo2 = new UsersInfo(dest);

        assertEquals("ABC", usersInfo2.getUser(0).target);

        Parcel parcelWrittenByUsersInfo = Parcel.obtain();
        usersInfo2.writeToParcel(parcelWrittenByUsersInfo, 0);
        parcelWrittenByUsersInfo.setDataPosition(0);

        UsersInfo usersInfo4 = new UsersInfo(parcelWrittenByUsersInfo);

        assertEquals("ABC", usersInfo4.getUser(0).target);
    }
}
