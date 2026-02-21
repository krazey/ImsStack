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

import android.annotation.NonNull;
import android.os.Parcel;
import android.os.Parcelable;

import com.android.imsstack.util.Log;

import java.util.ArrayList;
import java.util.List;

public class IpSecSaParameter implements Parcelable {
    /** Deprecated */
    public static final int INTEGRITY_ALGORITHM_HMAC_MD5_96 = 0;
    public static final int INTEGRITY_ALGORITHM_HMAC_SHA_1_96 = 1;
    public static final int INTEGRITY_ALGORITHM_AES_GMAC = 2;
    public static final int INTEGRITY_ALGORITHM_NULL = 3;

    /** Deprecated */
    public static final int ENCRYPTION_ALGORITHM_DES_EDE3_CBC = 0;
    public static final int ENCRYPTION_ALGORITHM_AES_CBC = 1;
    public static final int ENCRYPTION_ALGORITHM_NULL = 2;
    public static final int ENCRYPTION_ALGORITHM_AES_GCM = 3;

    private final int mId;
    private final int mSecurityProtocol;
    private final int mIntegrityAlgorithm;
    private final byte[] mIk;
    private final int mEncryptionAlgorithm;
    private final byte[] mCk;
    private final List<IpSecSaPolicy> mPolicys;

    public IpSecSaParameter(int id, int integrityAlgorithm, @NonNull byte[] ik,
            int encryptionAlgorithm, @NonNull byte[] ck, List<IpSecSaPolicy> policys) {
        mId = id;
        mSecurityProtocol = 1; // ESP
        mIntegrityAlgorithm = integrityAlgorithm;
        mIk = ik;
        mEncryptionAlgorithm = encryptionAlgorithm;
        mCk = ck;
        mPolicys = new ArrayList<>(policys);
    }

    /* package */ IpSecSaParameter(Parcel in) {
        mId = in.readInt();
        mSecurityProtocol = in.readInt();

        mIntegrityAlgorithm = in.readInt();
        int ikLen = in.readInt();
        mIk = new byte[ikLen];
        in.readByteArray(mIk);

        mEncryptionAlgorithm = in.readInt();
        int ckLen = in.readInt();
        mCk = new byte[ckLen];
        in.readByteArray(mCk);

        mPolicys = new ArrayList<>();
        int count = in.readInt();

        for (int i = 0; i < count; i++) {
            mPolicys.add(new IpSecSaPolicy(in));
        }
    }

    public int getId() {
        return mId;
    }

    public int getIntegrityAlgorithm() {
        return mIntegrityAlgorithm;
    }

    public int getEncryptionAlgorithm() {
        return mEncryptionAlgorithm;
    }

    public byte[] getIk() {
        return mIk;
    }

    public byte[] getCk() {
        return mCk;
    }

    public IpSecSaPolicy getPolicy(int spi) {
        for (IpSecSaPolicy p : mPolicys) {
            if (spi == p.getSpi()) {
                return p;
            }
        }

        return null;
    }

    public List<IpSecSaPolicy> getPolicys() {
        return mPolicys;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();

        sb.append("[ id=");
        sb.append(mId);
        sb.append(", securityProtocol=");
        sb.append(mSecurityProtocol);
        sb.append(", integrityAlgorithm=");
        sb.append(integrityAlgorithmToString(mIntegrityAlgorithm));
        sb.append(", ik=");
        sb.append(Log.pii(toHexString(mIk)));
        sb.append(", encryptionAlgorithm=");
        sb.append(encryptionAlgorithmToString(mEncryptionAlgorithm));
        sb.append(", ck=");
        sb.append(Log.pii(toHexString(mCk)));

        sb.append(", policys=" + mPolicys.size());

        if (mPolicys.size() > 0) {
            sb.append(", Policys=[ ");

            IpSecSaPolicy policy = mPolicys.get(0);

            sb.append(policy.toString());

            for (int i = 1; i < mPolicys.size(); ++i) {
                policy = mPolicys.get(i);
                sb.append(", ");
                sb.append(policy.toString());

            }

            sb.append(" ]");
        }

        sb.append(" ]");

        return sb.toString();
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel out, int flags) {
        out.writeInt(mId);
        out.writeInt(mSecurityProtocol);

        out.writeInt(mIntegrityAlgorithm);
        out.writeInt(mIk.length);
        out.writeByteArray(mIk);

        out.writeInt(mEncryptionAlgorithm);
        out.writeInt(mCk.length);
        out.writeByteArray(mCk);

        out.writeInt(mPolicys.size());

        for (int i = 0; i < mPolicys.size(); i++) {
            IpSecSaPolicy policy = mPolicys.get(i);
            policy.writeToParcel(out, flags);
        }
    }

    public static final Creator<IpSecSaParameter> CREATOR =
            new Creator<IpSecSaParameter>() {
        @Override
        public IpSecSaParameter createFromParcel(Parcel in) {
            return new IpSecSaParameter(in);
        }

        @Override
        public IpSecSaParameter[] newArray(int size) {
            return new IpSecSaParameter[size];
        }
    };

    /** Returns a string represented by the given encryption algorithm. */
    public static String encryptionAlgorithmToString(int algorithm) {
        return switch(algorithm) {
            case ENCRYPTION_ALGORITHM_AES_CBC -> "aes-cbc";
            case ENCRYPTION_ALGORITHM_NULL -> "null";
            case ENCRYPTION_ALGORITHM_AES_GCM -> "aes-gcm";
            case ENCRYPTION_ALGORITHM_DES_EDE3_CBC -> "des-ede3-cbc";
            default -> "unknown";
        };
    }

    /** Returns a string represented by the given integrity algorithm. */
    public static String integrityAlgorithmToString(int algorithm) {
        return switch(algorithm) {
            case INTEGRITY_ALGORITHM_HMAC_SHA_1_96 -> "hmac-sha-1-96";
            case INTEGRITY_ALGORITHM_HMAC_MD5_96 -> "hmac-md5-96";
            case INTEGRITY_ALGORITHM_NULL -> "null";
            case INTEGRITY_ALGORITHM_AES_GMAC -> "aes-gmac";
            default -> "unknown";
        };
    }

    private static String toHexString(byte[] data) {
        StringBuilder sb = new StringBuilder();
        sb.append("0x");

        for (int i = 0; i < data.length; i++) {
            sb.append(String.format("%02x", data[i] & 0xff));
        }
        return sb.toString();
    }
}
