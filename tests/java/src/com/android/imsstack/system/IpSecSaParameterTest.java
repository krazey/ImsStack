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

import static android.net.ipsec.ike.SaProposal.INTEGRITY_ALGORITHM_HMAC_SHA2_512_256;

import static com.android.imsstack.system.IpSecSaParameter.INTEGRITY_ALGORITHM_HMAC_MD5_96;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import android.test.suitebuilder.annotation.SmallTest;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mockito;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

@RunWith(JUnit4.class)
public class IpSecSaParameterTest {

    private IpSecSaParameter mIpSecSaParameter;
    private final List<IpSecSaPolicy> mPolicys = new ArrayList<>();
    private static final byte[] TEST_CK = {0x62, 0x36, 0x62, 0x6c, 0x07, 0x77, 0x78};
    private static final byte[] TEST_IK = {0x6d, 0x51, 0x27, 0x6a, 0x51, 0x31, 0x74};
    private static final int TEST_ID = 12;

    @Before
    public void setUp() throws Exception {
        IpSecSaPolicy mIpSecSaPolicy = Mockito.mock(IpSecSaPolicy.class);
        mPolicys.add(mIpSecSaPolicy);
        mIpSecSaParameter = new IpSecSaParameter(TEST_ID, INTEGRITY_ALGORITHM_HMAC_MD5_96, TEST_IK,
                INTEGRITY_ALGORITHM_HMAC_SHA2_512_256, TEST_CK, mPolicys);
    }

    @After
    public void tearDown() throws Exception {
        mPolicys.clear();
        mIpSecSaParameter = null;
    }

    @Test
    @SmallTest
    public void getIpSecSaParameter_all() {
        assertEquals(0, mIpSecSaParameter.describeContents());

        assertTrue(Arrays.equals(TEST_CK, mIpSecSaParameter.getCk()));

        assertEquals(TEST_ID, mIpSecSaParameter.getId());

        assertNotNull(mIpSecSaParameter.getPolicys());
        assertEquals(mPolicys.get(0), mIpSecSaParameter.getPolicys().get(0));
        assertTrue(Arrays.equals(TEST_IK, mIpSecSaParameter.getIk()));

        assertEquals(INTEGRITY_ALGORITHM_HMAC_SHA2_512_256,
                mIpSecSaParameter.getEncryptionAlgorithm());

        assertEquals(INTEGRITY_ALGORITHM_HMAC_MD5_96, mIpSecSaParameter.getIntegrityAlgorithm());
    }
}
