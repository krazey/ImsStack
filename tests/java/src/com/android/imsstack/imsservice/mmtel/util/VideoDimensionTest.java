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

package com.android.imsstack.imsservice.mmtel.util;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import android.util.Log;

import com.android.imsstack.*;

import org.junit.After;
import org.junit.Before;
import org.junit.runner.*;
import org.junit.runners.JUnit4;
import org.junit.Test;

@RunWith(JUnit4.class)
public class VideoDimensionTest {
    private int mWidth = 10;
    private int mHeight = 20;
    private VideoDimension mVideoDimension;
    public static final String TAG = "VideoDimensionTest";

    @Before
    public void setUp() throws Exception {
         mVideoDimension = new VideoDimension(mWidth, mHeight);
    }

    @Test
    public void testGetWidth(){
        Log.d(TAG, " Unit Test");
        assertEquals(mWidth , mVideoDimension.getWidth());
    }

    @Test
    public void testGetHeight(){
        assertEquals(mHeight , mVideoDimension.getHeight());
    }

    @Test
    public void testToString(){
        assertNotNull(mVideoDimension);
    }

    @After
    public void tearDown() throws Exception {
        mVideoDimension = null;
    }
}
