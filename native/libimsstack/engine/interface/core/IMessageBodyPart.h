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
#ifndef INTERFACE_MESSAGE_BODY_PART_H_
#define INTERFACE_MESSAGE_BODY_PART_H_

#include "ByteArray.h"

/**
 * @brief This class provides an interface to access and control SIP message body parts.
 *
 * A MessageBodyPart can contain different kinds of content,
 * for example text, an image or an audio clip.
 *
 * @see IMessage, ISipMessageBodyPart
 */
class IMessageBodyPart
{
public:
    /**
     * @brief Gets the content of this IMessageBodyPart.
     *
     * @return The content of message body part.
     */
    virtual const ByteArray& GetContent() const = 0;

    /**
     * @brief Returns the value of a header in this IMessageBodyPart.
     *
     * Only headers with the prefix "Content-" are allowed to be retrieved with this method.
     *
     * @param strName The header name, in full form
     * @return A string containing the header value or null if the header doesn't exist.
     */
    virtual AString GetHeader(IN const AString& strName) const = 0;

    /**
     * @brief Sets the content to this IMessageBodyPart.
     *
     * @param objContent The content of message body part
     * @return If the content is successfully set, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetContent(IN const ByteArray& objContent) = 0;

    /**
     * @brief Sets a header to this IMessageBodyPart.
     *
     * If the header name already exists, the value will be replaced.
     * Only header with the prefix "Content-" are allowed to set with this method.
     *
     * @param strName The header name, in full form
     * @param strValue The header value
     * @return If the header is successfully set, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetHeader(IN const AString& strName, IN const AString& strValue) = 0;
};

#endif
