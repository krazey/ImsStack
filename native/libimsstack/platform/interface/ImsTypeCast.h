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
#ifndef IMS_TYPE_CAST_H_
#define IMS_TYPE_CAST_H_

#ifdef IMS_RTTI_ENABLED

#define CONST_CAST(TYPE, VALUE) (const_cast<TYPE>(VALUE))
#define DYNAMIC_CAST(TYPE, VALUE) (dynamic_cast<TYPE>(VALUE))
#define REINTERPRET_CAST(TYPE, VALUE) (reinterpret_cast<TYPE>(VALUE))
#define STATIC_CAST(TYPE, VALUE) (static_cast<TYPE>(VALUE))

#else

// C-style type casting
#define CONST_CAST(TYPE, VALUE) (const_cast<TYPE>(VALUE))
#define DYNAMIC_CAST(TYPE, VALUE) ((TYPE)(VALUE))
#define REINTERPRET_CAST(TYPE, VALUE) (reinterpret_cast<TYPE>(VALUE))
#define STATIC_CAST(TYPE, VALUE) (static_cast<TYPE>(VALUE))

#endif  // IMS_RTTI_ENABLED

#endif
