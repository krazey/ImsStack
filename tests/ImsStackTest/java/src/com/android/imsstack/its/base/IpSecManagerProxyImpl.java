/*
 * Copyright (C) 2024 The Android Open Source Project
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
package com.android.imsstack.its.base;

import android.content.Context;
import android.net.IpSecManager;
import android.net.IpSecManager.ResourceUnavailableException;
import android.net.IpSecManager.SecurityParameterIndex;
import android.net.IpSecManager.SpiUnavailableException;
import android.net.IpSecTransform;
import android.net.annotations.PolicyDirection;

import androidx.annotation.NonNull;

import com.android.imsstack.base.SystemServiceProxy.IpSecManagerProxy;

import java.io.FileDescriptor;
import java.io.IOException;
import java.net.InetAddress;

/**
 * An implementation class to access the {@link IpSecManager}.
 */
public class IpSecManagerProxyImpl implements IpSecManagerProxy {
    private final IpSecManager mIpSecManager;

    IpSecManagerProxyImpl(Context context) {
        mIpSecManager = context.getSystemService(IpSecManager.class);
    }

    @Override
    @NonNull
    public SecurityParameterIndex allocateSecurityParameterIndex(
            @NonNull InetAddress destinationAddress, int requestedSpi)
            throws SpiUnavailableException, ResourceUnavailableException {
        return mIpSecManager.allocateSecurityParameterIndex(destinationAddress, requestedSpi);
    }

    @Override
    public void applyTransportModeTransform(@NonNull FileDescriptor socket,
            @PolicyDirection int direction, @NonNull IpSecTransform transform) throws IOException {
        // No operations.
    }

    @Override
    public void removeTransportModeTransforms(@NonNull FileDescriptor socket) throws IOException {
        // No operations.
    }
}
