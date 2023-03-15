/*
 * Copyright (C) 2023 The Android Open Source ProjMultiEndpoint
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

#ifndef MULTI_ENDPOINT_FACTORY_H_
#define MULTI_ENDPOINT_FACTORY_H_

#include "ImsTypeDef.h"
#include "dialogevent/DialogInfoManager.h"
#include "dialogevent/DialogSubscription.h"
#include <memory>
#include <utility>

class AString;
class IDialogInfoManager;
class IDialogSubscription;
class IDialogSubscriptionListener;
class IMtcContext;

class MultiEndpointFactory
{
public:
    inline MultiEndpointFactory() {}
    virtual inline ~MultiEndpointFactory() {}
    MultiEndpointFactory(IN const MultiEndpointFactory&) = delete;
    MultiEndpointFactory& operator=(IN const MultiEndpointFactory&) = delete;

    inline virtual std::unique_ptr<IDialogInfoManager> CreateDialogInfoManager()
    {
        return std::make_unique<DialogInfoManager>();
    }

    inline virtual std::unique_ptr<IDialogSubscription> CreateDialogSubscription(
            IN IMtcContext& objContext, IN IDialogSubscriptionListener& objListener,
            IN const AString& strTargetUri)
    {
        return std::make_unique<DialogSubscription>(objContext, objListener, strTargetUri);
    }
};

#endif
