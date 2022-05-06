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
#ifndef OS_SOCKET_DEF_H_
#define OS_SOCKET_DEF_H_

#include <sys/socket.h>

#ifndef SHUT_RD
#define SHUT_RD 0
#endif

#ifndef SHUT_WR
#define SHUT_WR 1
#endif

#ifndef SHUT_RDWR
#define SHUT_RDWR 2
#endif

#ifndef FD_READ
#define FD_READ 0x01
#endif

#ifndef FD_WRITE
#define FD_WRITE 0x02
#endif

#ifndef FD_ACCEPT
#define FD_ACCEPT 0x08
#endif

#ifndef FD_CONNECT
#define FD_CONNECT 0x10
#endif

#ifndef FD_CLOSE
#define FD_CLOSE 0x20
#endif

// IMS extensions
#ifndef FD_TCP_C
#define FD_TCP_C 0x10000
#endif

#ifndef FD_TCP
#define FD_TCP 0x20000
#endif

#ifndef FD_RDWR
#define FD_RDWR (FD_READ | FD_WRITE)
#endif

#ifndef SOCKET
#define SOCKET IMS_SINT32
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif

enum
{
    SHUTDOWN_RX = SHUT_RD,
    SHUTDOWN_TX = SHUT_WR,
    SHUTDOWN_BOTH = SHUT_RDWR
};

enum
{
    EVENT_FD_ALL = (FD_ACCEPT | FD_CLOSE | FD_CONNECT | FD_READ | FD_WRITE)
};

#endif
