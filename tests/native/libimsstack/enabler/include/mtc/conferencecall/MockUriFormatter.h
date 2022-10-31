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

#ifndef MOCK_URI_FORMATTER_H_
#define MOCK_URI_FORMATTER_H_

#include "ImsTypeDef.h"
#include "UriFormatter.h"
#include <gmock/gmock.h>

class MockUriFormatter : public UriFormatter
{
public:
    // MOCK_METHOD(static AString&, GetReferToForInvite, (OUT AString& strUri, IN IMtcCallContext&
    // objContext, IN IMS_BOOL bEnforcePaid), ()); // free function MOCK_METHOD(static AString&,
    // GetReferToForInvite, (OUT AString& strUri, IN IMtcCallContext& objContext, IN const ConfUser*
    // pConfUser), ()); // free function MOCK_METHOD(static AString&, GetReferToForBye, (OUT
    // AString& strUri, IN const ConfUser* pConfUser, IN const AString& strInviteduri), ()); // free
    // function MOCK_METHOD(static void, ConvertToValidSipUri, (IN_OUT AString& strUri, IN
    // IMtcCallContext& objContext), ()); // free function
};

#endif
