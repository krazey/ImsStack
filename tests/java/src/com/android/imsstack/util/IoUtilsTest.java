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
package com.android.imsstack.util;

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.fail;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.test.suitebuilder.annotation.SmallTest;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.io.FileOutputStream;

@RunWith(JUnit4.class)
public class IoUtilsTest {
    private static final String TEST_FILE = "test.txt";

    @Mock Context mContext;
    @Mock FileOutputStream mFileOutputStream;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        when(mContext.openFileOutput(anyString(), anyInt())).thenReturn(mFileOutputStream);
    }

    @After
    public void tearDown() throws Exception {
        mContext = null;
    }

    @Test
    @SmallTest
    public void closeQuietly() throws Exception {
        FileOutputStream fos = null;

        try {
            IoUtils.closeQuietly(fos);
        } catch (Exception unexpected) {
            fail("Unexpected exception thrown.");
        }

        try {
            fos = mContext.openFileOutput(TEST_FILE, Context.MODE_APPEND);
            assertNotNull(fos);
        } finally {
            IoUtils.closeQuietly(fos);
        }

        verify(mFileOutputStream).close();
    }
}
