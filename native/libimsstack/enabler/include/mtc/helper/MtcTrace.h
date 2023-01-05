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

#ifndef UC_TRACE_H
#define UC_TRACE_H

#include "ServiceTrace.h"

#undef IMS_TRACE_D
#undef IMS_TRACE_I
#undef IMS_TRACE_E

#define IMS_TRACE_D(FORMAT, A1, A2, A3)        U_IMS_TRACE_D(UCTAG, FORMAT, A1, A2, A3)
#define IMS_TRACE_I(FORMAT, A1, A2, A3)        U_IMS_TRACE_I(UCTAG, FORMAT, A1, A2, A3)
#define IMS_TRACE_E(ECODE, FORMAT, A1, A2, A3) U_IMS_TRACE_E(ECODE, UCTAG, FORMAT, A1, A2, A3)

#endif
