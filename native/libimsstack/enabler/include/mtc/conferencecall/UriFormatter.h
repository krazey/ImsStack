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

#ifndef URI_FORMATTER_H_
#define URI_FORMATTER_H_

#include "ImsTypeDef.h"

class AString;
class IMtcCallContext;
class MtcConfigurationProxy;
struct ConfUser;

class UriFormatter
{
public:
    static AString& GetReferToForInvite(OUT AString& strUri, IN IMtcCallContext& objContext,
            IN IMS_BOOL bEnforcePaid = IMS_FALSE);
    static AString& GetReferToForInvite(
            OUT AString& strUri, IN IMtcCallContext& objContext, IN const ConfUser* pConfUser);
    static AString& GetReferToForBye(OUT AString& strUri, IN MtcConfigurationProxy& objConfig,
            IN const ConfUser* pConfUser, IN const AString& strInvitedUri);

private:
    static void ConvertToValidSipUri(IN_OUT AString& strUri, IN IMtcCallContext& objContext);

private:
    static const IMS_CHAR STR_USER_PHONE[];
};

#endif
