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

package com.android.imsstack.enabler.ssc;

import android.os.Handler;

import org.w3c.dom.Document;

public interface ISscHttpConnection {
    public void close();
    public int  sendRequest(int requestType, String requestURI, String body);
    public void setCredentialOnChallenge(String body);
    public void setTransactionHandler(Handler handler);
    public void setXuiValue(String xui);
    public Document getInputStream();
}
