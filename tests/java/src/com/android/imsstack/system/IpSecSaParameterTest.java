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

package com.android.imsstack.system;

import static com.android.imsstack.system.IpSecSaParameter.ENCRYPTION_ALGORITHM_AES_CBC;
import static com.android.imsstack.system.IpSecSaParameter.ENCRYPTION_ALGORITHM_AES_GCM;
import static com.android.imsstack.system.IpSecSaParameter.ENCRYPTION_ALGORITHM_DES_EDE3_CBC;
import static com.android.imsstack.system.IpSecSaParameter.ENCRYPTION_ALGORITHM_NULL;
import static com.android.imsstack.system.IpSecSaParameter.INTEGRITY_ALGORITHM_AES_GMAC;
import static com.android.imsstack.system.IpSecSaParameter.INTEGRITY_ALGORITHM_HMAC_MD5_96;
import static com.android.imsstack.system.IpSecSaParameter.INTEGRITY_ALGORITHM_HMAC_SHA_1_96;
import static com.android.imsstack.system.IpSecSaParameter.INTEGRITY_ALGORITHM_NULL;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import android.os.Parcel;

import androidx.test.filters.SmallTest;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import java.util.Arrays;
import java.util.List;

@RunWith(JUnit4.class)
public class IpSecSaParameterTest {
    private static final int TEST_SPI = 1;
    private static final int TEST_ID = 12;
    private static final byte[] TEST_CK = {0x62, 0x36, 0x62, 0x6c, 0x07, 0x77, 0x78};
    private static final byte[] TEST_IK = {0x6d, 0x51, 0x27, 0x6a, 0x51, 0x31, 0x74};

    private final IpSecSaPolicy mSaPolicy1 = new IpSecSaPolicy(TEST_SPI,
            IpSecSaPolicy.DIRECTION_OUT, IpSecSaPolicy.MODE_TRANSPORT,
            "192.168.0.1", "192.168.0.2");
    private final IpSecSaPolicy mSaPolicy2 = new IpSecSaPolicy(TEST_SPI + 1,
            IpSecSaPolicy.DIRECTION_IN, IpSecSaPolicy.MODE_TRANSPORT,
            "192.168.0.2", "192.168.0.1");
    private final List<IpSecSaPolicy> mSaPolicys = List.of(mSaPolicy1, mSaPolicy2);

    @Test
    @SmallTest
    public void testIpSecSaParameter() {
        IpSecSaParameter saParameter = new IpSecSaParameter(TEST_ID,
                INTEGRITY_ALGORITHM_HMAC_SHA_1_96, TEST_IK,
                ENCRYPTION_ALGORITHM_AES_CBC, TEST_CK, mSaPolicys);

        assertEquals(TEST_ID, saParameter.getId());
        assertEquals(INTEGRITY_ALGORITHM_HMAC_SHA_1_96, saParameter.getIntegrityAlgorithm());
        assertEquals(ENCRYPTION_ALGORITHM_AES_CBC, saParameter.getEncryptionAlgorithm());
        assertTrue(Arrays.equals(TEST_IK, saParameter.getIk()));
        assertTrue(Arrays.equals(TEST_CK, saParameter.getCk()));
        assertNotNull(saParameter.getPolicys());
        assertEquals(mSaPolicys.get(0), saParameter.getPolicys().get(0));
        assertEquals(mSaPolicy1, saParameter.getPolicy(TEST_SPI));
        assertNull(saParameter.getPolicy(0));
        assertNotNull(saParameter.toString());
        assertEquals(0, saParameter.describeContents());
    }

    @Test
    @SmallTest
    public void testCreator() {
        IpSecSaParameter saParameter = new IpSecSaParameter(TEST_ID,
                INTEGRITY_ALGORITHM_HMAC_SHA_1_96, TEST_IK,
                ENCRYPTION_ALGORITHM_AES_CBC, TEST_CK, mSaPolicys);
        IpSecSaParameter newSaParameter;
        Parcel parcel = Parcel.obtain();

        try {
            saParameter.writeToParcel(parcel, 0);
            parcel.setDataPosition(0);
            newSaParameter = IpSecSaParameter.CREATOR.createFromParcel(parcel);
        } finally {
            parcel.recycle();
        }

        assertEquals(TEST_ID, newSaParameter.getId());
        assertEquals(INTEGRITY_ALGORITHM_HMAC_SHA_1_96, newSaParameter.getIntegrityAlgorithm());
        assertEquals(ENCRYPTION_ALGORITHM_AES_CBC, newSaParameter.getEncryptionAlgorithm());
        assertTrue(Arrays.equals(TEST_IK, newSaParameter.getIk()));
        assertTrue(Arrays.equals(TEST_CK, newSaParameter.getCk()));
        assertNotNull(newSaParameter.getPolicys());
        assertNotNull(newSaParameter.getPolicys().get(0));

        IpSecSaParameter[] saParameters = IpSecSaParameter.CREATOR.newArray(2);

        assertEquals(2, saParameters.length);
    }

    @Test
    @SmallTest
    public void testIntegrityAlgorithmToString() {
        assertEquals("aes-gmac",
                IpSecSaParameter.integrityAlgorithmToString(INTEGRITY_ALGORITHM_AES_GMAC));
        assertEquals("hmac-md5-96",
                IpSecSaParameter.integrityAlgorithmToString(INTEGRITY_ALGORITHM_HMAC_MD5_96));
        assertEquals("hmac-sha-1-96",
                IpSecSaParameter.integrityAlgorithmToString(INTEGRITY_ALGORITHM_HMAC_SHA_1_96));
        assertEquals("null",
                IpSecSaParameter.integrityAlgorithmToString(INTEGRITY_ALGORITHM_NULL));
        assertEquals("unknown", IpSecSaParameter.integrityAlgorithmToString(100));
    }

    @Test
    @SmallTest
    public void testEncryptionAlgorithmToString() {
        assertEquals("aes-cbc",
                IpSecSaParameter.encryptionAlgorithmToString(ENCRYPTION_ALGORITHM_AES_CBC));
        assertEquals("aes-gcm",
                IpSecSaParameter.encryptionAlgorithmToString(ENCRYPTION_ALGORITHM_AES_GCM));
        assertEquals("des-ede3-cbc",
                IpSecSaParameter.encryptionAlgorithmToString(ENCRYPTION_ALGORITHM_DES_EDE3_CBC));
        assertEquals("null",
                IpSecSaParameter.encryptionAlgorithmToString(ENCRYPTION_ALGORITHM_NULL));
        assertEquals("unknown", IpSecSaParameter.encryptionAlgorithmToString(100));
    }
}
