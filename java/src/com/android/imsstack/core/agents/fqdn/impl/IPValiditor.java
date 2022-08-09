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
package com.android.imsstack.core.agents.fqdn.impl;

import android.text.TextUtils;

import com.android.imsstack.util.ImsLog;

import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.regex.PatternSyntaxException;

public class IPValiditor {
    private static Pattern sVALID_IPV4_PATTERN = null;
    private static Pattern sVALID_IPV6_PATTERN = null;
    private static final String ipv4Pattern =
        "(([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\.){3}([01]?\\d\\d?|2[0-4]\\d|25[0-5])";
    private static final String ipv6Pattern =
        "([0-9a-f]{1,4}:){7}([0-9a-f]){1,4}";

    static {
        try {
            sVALID_IPV4_PATTERN = Pattern.compile(ipv4Pattern, Pattern.CASE_INSENSITIVE);
            sVALID_IPV6_PATTERN = Pattern.compile(ipv6Pattern, Pattern.CASE_INSENSITIVE);
        }
        catch (PatternSyntaxException e) {
            ImsLog.e("" + e);
        }
    }

    public static boolean isIpAddress(String ipAddress) {
        if (true == TextUtils.isEmpty(ipAddress)) {
            ImsLog.w("Address is empty");
            return false;
        }

        Matcher objIpv4Checker = sVALID_IPV4_PATTERN.matcher(ipAddress);
        Matcher objIpv6Checker = sVALID_IPV6_PATTERN.matcher(ipAddress);
        if (null != objIpv4Checker && true == objIpv4Checker.matches()) {
            ImsLog.i("Address type is IPv4");
            return true;
        }
        else if (null != objIpv6Checker && true == objIpv6Checker.matches()) {
            ImsLog.i("Address type is IPv6");
            return true;
        }

        ImsLog.w("Address type is not IP");
        return false;
    }

}
