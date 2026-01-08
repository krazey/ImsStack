/*
 * Copyright (C) 2025 The Android Open Source Project
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

package com.android.imsstack.imsservice.mmtel.sms;

/** An input stream for reading bits from a byte array. */
public class BitwiseInputStream {
    private byte[] mBuf;
    private int mPos;
    private int mEnd;

    private static final int BITS_PER_BYTE = 8;
    private static final int BITS_PER_SHORT = 16;
    private static final int BITS_PER_INT = 32;

    /** An exception indicating that a read has failed. */
    public static class AccessException extends Exception {
        private static final long serialVersionUID = 1L;

        public AccessException(String s) {
            super("BitwiseInputStream: " + s);
        }
    }

    /** Creates a new stream for the given byte array. */
    public BitwiseInputStream(byte[] buf) {
        // create a copy for preventing changes
        mBuf = buf.clone();
        mEnd = buf.length * BITS_PER_BYTE;
        mPos = 0;
    }

    /** Returns the number of bits available for reading. */
    public int available() {
        return mEnd - mPos;
    }

    /**
     * Reads a sequence of bits.
     *
     * @param bits the number of bits to read (a value between 0 and 8)
     * @return the value of the bits, as a byte
     */
    public int read(int bits) throws AccessException {
        int index = mPos / BITS_PER_BYTE;
        int offset = BITS_PER_SHORT - (mPos % BITS_PER_BYTE) - bits;
        if ((bits < 0) || (bits > BITS_PER_BYTE) || ((mPos + bits) > mEnd)) {
            throw new AccessException(
                    "illegal read " + "(pos " + mPos + ", end " + mEnd + ", bits " + bits + ")");
        }
        int data = (mBuf[index] & 0xFF) << BITS_PER_BYTE;
        if (offset < BITS_PER_BYTE) data |= mBuf[index + 1] & 0xFF;
        data >>>= offset;
        data &= (-1 >>> (BITS_PER_INT - bits));
        mPos += bits;
        return data;
    }

    /**
     * Reads a sequence of bits and returns them in a byte array.
     *
     * @param bits the number of bits to read
     * @return a byte array containing the bits
     */
    public byte[] readByteArray(int bits) throws AccessException {
        int bytes = (bits / BITS_PER_BYTE) + ((bits % BITS_PER_BYTE) > 0 ? 1 : 0);
        byte[] arr = new byte[bytes];
        for (int i = 0; i < bytes; i++) {
            int increment = Math.min(BITS_PER_BYTE, bits - (i * BITS_PER_BYTE));
            arr[i] = (byte) (read(increment) << (BITS_PER_BYTE - increment));
        }
        return arr;
    }

    /**
     * Skips a sequence of bits.
     *
     * @param bits the number of bits to skip
     */
    public void skip(int bits) throws AccessException {
        if (mPos + bits > mEnd) {
            throw new AccessException(
                    "illegal skip (pos " + mPos + ", end " + mEnd + ", bits " + bits + ")");
        }
        mPos += bits;
    }
}
