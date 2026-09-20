/*
 * Copyright (C) 2026 The Android Open Source Project
 * SPDX-License-Identifier: Apache-2.0
 */
package com.android.imsstack.core.config;

import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertSame;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.os.Bundle;

import androidx.test.filters.SmallTest;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
@SmallTest
public class CarrierSettingsPackageTest {
    private Context mContext;
    private PackageManager mPm;
    private ApplicationInfo mInfo;
    private AssetManager mAssets;

    @Before
    public void setUp() throws Exception {
        mContext = mock(Context.class);
        mPm = mock(PackageManager.class);
        Context dataContext = mock(Context.class);
        mAssets = mock(AssetManager.class);
        mInfo = new ApplicationInfo();
        mInfo.enabled = true;
        mInfo.flags = ApplicationInfo.FLAG_SYSTEM;
        mInfo.metaData = new Bundle();
        mInfo.metaData.putInt(CarrierSettingsPackage.SCHEMA_VERSION, 1);
        when(mContext.getPackageManager()).thenReturn(mPm);
        when(mContext.getPackageName()).thenReturn("com.android.imsstack");
        when(mPm.getApplicationInfo(CarrierSettingsPackage.PACKAGE_NAME,
                PackageManager.GET_META_DATA)).thenReturn(mInfo);
        when(mPm.checkSignatures("com.android.imsstack", CarrierSettingsPackage.PACKAGE_NAME))
                .thenReturn(PackageManager.SIGNATURE_MATCH);
        when(mContext.createPackageContext(CarrierSettingsPackage.PACKAGE_NAME,
                Context.CONTEXT_RESTRICTED)).thenReturn(dataContext);
        when(dataContext.getAssets()).thenReturn(mAssets);
    }

    @Test
    public void acceptsMatchingSystemDataWithoutLoadingCode() throws Exception {
        assertSame(mAssets, CarrierSettingsPackage.loadAssets(mContext));
        verify(mContext).createPackageContext(CarrierSettingsPackage.PACKAGE_NAME,
                Context.CONTEXT_RESTRICTED);
    }

    @Test
    public void acceptsSignedSystemAppUpdates() {
        mInfo.flags = ApplicationInfo.FLAG_UPDATED_SYSTEM_APP;
        assertSame(mAssets, CarrierSettingsPackage.loadAssets(mContext));
    }

    @Test
    public void rejectsOrdinaryUserPackages() throws Exception {
        mInfo.flags = 0;
        assertNull(CarrierSettingsPackage.loadAssets(mContext));
        verify(mContext, never()).createPackageContext(
                eq(CarrierSettingsPackage.PACKAGE_NAME), anyInt());
    }

    @Test
    public void rejectsWrongSignature() {
        when(mPm.checkSignatures("com.android.imsstack", CarrierSettingsPackage.PACKAGE_NAME))
                .thenReturn(PackageManager.SIGNATURE_NO_MATCH);
        assertNull(CarrierSettingsPackage.loadAssets(mContext));
    }

    @Test
    public void rejectsMissingOrFutureSchema() {
        mInfo.metaData.putInt(CarrierSettingsPackage.SCHEMA_VERSION, 2);
        assertNull(CarrierSettingsPackage.loadAssets(mContext));
        mInfo.metaData = null;
        assertNull(CarrierSettingsPackage.loadAssets(mContext));
    }

    @Test
    public void rejectsDisabledPackages() {
        mInfo.enabled = false;
        assertNull(CarrierSettingsPackage.loadAssets(mContext));
    }

    @Test
    public void missingPackageFallsBack() throws Exception {
        when(mPm.getApplicationInfo(CarrierSettingsPackage.PACKAGE_NAME,
                PackageManager.GET_META_DATA)).thenThrow(new PackageManager.NameNotFoundException());
        assertNull(CarrierSettingsPackage.loadAssets(mContext));
    }
}
