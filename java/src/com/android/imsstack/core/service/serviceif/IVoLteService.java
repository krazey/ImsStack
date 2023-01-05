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
package com.android.imsstack.core.service.serviceif;

import android.content.Context;

public interface IVoLteService extends IService {

    int TYPE_DEFAULT = 0;
    int TYPE_CALLSETTING = TYPE_DEFAULT + 1;
    int TYPE_MAX = TYPE_DEFAULT + 2;

    void start(int slotID);

    Context getContext();
    int getSlotID();
    IService getService(int type);
}
