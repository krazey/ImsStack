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
#ifndef IMS_CONST_DEF_H_
#define IMS_CONST_DEF_H_

#define IMS_SOLUTION_STORAGE_ROOT_DIR "/data/user_de/0/com.android.imsstack"
#define IMS_SOLUTION_IMS_CONFIG_DB (IMS_SOLUTION_STORAGE_ROOT_DIR "/databases/gims.db")

#define IMS_SOLUTION_URI_LEN                        128

// This definition will be provided regardless of IP version.
/* 128bit -> 21DA:00D3:0000:2F3B:02AA:00FF:FE28:9C5A = 39byte+1(null)byte */
#define IMS_SOLUTION_IP_LEN                         39

#endif
