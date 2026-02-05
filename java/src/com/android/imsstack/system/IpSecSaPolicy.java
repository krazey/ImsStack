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
package com.android.imsstack.system;

import android.os.Parcel;
import android.os.Parcelable;

import com.android.imsstack.util.Log;

public class IpSecSaPolicy implements Parcelable {
    public static final int DIRECTION_IN = 0;
    public static final int DIRECTION_OUT = 1;

    public static final int MODE_TRANSPORT = 0;
    public static final int MODE_TUNNEL = 1;

    private final int mSpi;
    private final int mDirection;
    private final int mMode;
    private final String mLocalIp;
    private final String mRemoteIp;

    public IpSecSaPolicy(int spi, int direction, int mode,
            String localIp, String remoteIp) {
        mSpi = spi;
        mDirection = direction;
        mMode = mode;
        mLocalIp = localIp;
        mRemoteIp = remoteIp;
    }

    /* package */ IpSecSaPolicy(Parcel in) {
        mSpi = in.readInt();
        mDirection = in.readInt();
        mMode = in.readInt();
        mLocalIp = in.readString();
        mRemoteIp = in.readString();
    }

    public int getSpi() {
        return mSpi;
    }

    public int getDirection() {
        return mDirection;
    }

    public int getMode() {
        return mMode;
    }

    public String getLocalIp() {
        return mLocalIp;
    }

    public String getRemoteIp() {
        return mRemoteIp;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();

        sb.append("[ ");
        sb.append("spi=0x");
        sb.append(Integer.toHexString(mSpi));
        sb.append("(");
        sb.append(Integer.toUnsignedString(mSpi, 10));
        sb.append(")");
        sb.append(", direction=");
        sb.append(mDirection == 0 ? "IN" : "OUT");
        sb.append(", mode=");
        sb.append(mMode == 0 ? "transport" : "tunnel");
        sb.append(", localIp=");
        sb.append(mLocalIp);
        sb.append(", remoteIp=");
        sb.append(Log.pii(mRemoteIp));
        sb.append(" ]");

        return sb.toString();
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel out, int flags) {
        out.writeInt(mSpi);
        out.writeInt(mDirection);
        out.writeInt(mMode);
        out.writeString(mLocalIp);
        out.writeString(mRemoteIp);
    }

    public static final Creator<IpSecSaPolicy> CREATOR =
            new Creator<IpSecSaPolicy>() {
        @Override
        public IpSecSaPolicy createFromParcel(Parcel in) {
            return new IpSecSaPolicy(in);
        }

        @Override
        public IpSecSaPolicy[] newArray(int size) {
            return new IpSecSaPolicy[size];
        }
    };
}
