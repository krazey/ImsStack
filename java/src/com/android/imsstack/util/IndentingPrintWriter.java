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

import java.io.PrintWriter;
import java.io.Writer;

/**
 * Lightweight wrapper around {@link PrintWriter} that automatically indents
 * newlines based on internal state. It also automatically wraps long lines
 * based on given line length.
 * <p>
 * Delays writing indent until first actual write on a newline, enabling indent
 * modification after newline.
 */
public class IndentingPrintWriter extends PrintWriter {
    private final String mSingleIndent;

    /** Mutable version of current indent */
    private StringBuilder mIndentBuilder = new StringBuilder();
    /** Cache of current {@link #mIndentBuilder} value */
    private char[] mCurrentIndent;

    /**
     * Flag indicating if we're currently sitting on an empty line, and that
     * next write should be prefixed with the current indent.
     */
    private boolean mEmptyLine = true;

    private char[] mSingleChar = new char[1];

    /**
     * Constructor
     */
    public IndentingPrintWriter(Writer writer, String singleIndent) {
        super(writer);
        mSingleIndent = singleIndent;
    }

    /**
     * Increases the indent starting with the next printed line.
     */
    public IndentingPrintWriter increaseIndent() {
        mIndentBuilder.append(mSingleIndent);
        mCurrentIndent = null;
        return this;
    }

    /**
     * Decreases the indent starting with the next printed line.
     */
    public IndentingPrintWriter decreaseIndent() {
        mIndentBuilder.delete(0, mSingleIndent.length());
        mCurrentIndent = null;
        return this;
    }

    @Override
    public void println() {
        write('\n');
    }

    @Override
    public void write(int c) {
        mSingleChar[0] = (char) c;
        write(mSingleChar, 0, 1);
    }

    @Override
    public void write(String s, int off, int len) {
        final char[] buf = new char[len];
        s.getChars(off, len - off, buf, 0);
        write(buf, 0, len);
    }

    @Override
    public void write(char[] buf, int offset, int count) {
        final int bufferEnd = offset + count;
        int lineStart = offset;
        int lineEnd = offset;

        // March through incoming buffer looking for newlines
        while (lineEnd < bufferEnd) {
            char ch = buf[lineEnd++];
            if (ch == '\n') {
                maybeWriteIndent();
                super.write(buf, lineStart, lineEnd - lineStart);
                lineStart = lineEnd;
                mEmptyLine = true;
            }
        }

        if (lineStart != lineEnd) {
            maybeWriteIndent();
            super.write(buf, lineStart, lineEnd - lineStart);
        }
    }

    private void maybeWriteIndent() {
        if (mEmptyLine) {
            mEmptyLine = false;
            if (mIndentBuilder.length() != 0) {
                if (mCurrentIndent == null) {
                    mCurrentIndent = mIndentBuilder.toString().toCharArray();
                }
                super.write(mCurrentIndent, 0, mCurrentIndent.length);
            }
        }
    }
}
