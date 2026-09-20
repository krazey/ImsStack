/*
 * Copyright (C) 2026 The Android Open Source Project
 * SPDX-License-Identifier: Apache-2.0
 */
package com.android.imsstack.core.config;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.util.Log;

/** Opens the separately installed, product-signed carrier data without loading its code. */
public final class CarrierSettingsPackage {
    public static final String PACKAGE_NAME = "org.lineageos.carriersettings";
    public static final String SCHEMA_VERSION = PACKAGE_NAME + ".SCHEMA_VERSION";
    private static final String TAG = "CarrierSettingsPackage";

    private CarrierSettingsPackage() {}

    public static AssetManager loadAssets(Context context) {
        PackageManager pm = context.getPackageManager();
        try {
            ApplicationInfo info = pm.getApplicationInfo(PACKAGE_NAME,
                    PackageManager.GET_META_DATA);
            if (!info.enabled
                    || (info.flags & (ApplicationInfo.FLAG_SYSTEM
                            | ApplicationInfo.FLAG_UPDATED_SYSTEM_APP)) == 0
                    || pm.checkSignatures(context.getPackageName(), PACKAGE_NAME)
                            != PackageManager.SIGNATURE_MATCH
                    || info.metaData == null || info.metaData.getInt(SCHEMA_VERSION, 0) != 1) {
                Log.w(TAG, "Ignoring disabled, untrusted or incompatible carrier data package");
                return null;
            }
            // No CONTEXT_INCLUDE_CODE: the package supplies assets, never executable policy.
            return context.createPackageContext(PACKAGE_NAME, Context.CONTEXT_RESTRICTED)
                    .getAssets();
        } catch (PackageManager.NameNotFoundException | SecurityException e) {
            Log.w(TAG, "Carrier data unavailable; using built-in and framework configuration", e);
            return null;
        }
    }
}
