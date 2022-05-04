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
#ifndef INTERFACE_AOS_MESSAGE_HANDLER_H_
#define INTERFACE_AOS_MESSAGE_HANDLER_H_

class IAosMsgHandlerListener;

class IAosMsgHandler
{
public:
    /*
     * Sends a Message to be delivered after the specified amount of time elapses.
     *
     * Remarks
     * Parameters
     * <table>
     * parameter            description
     * ----------          ----------
     * piListener             entity which will receive the message.
     * nMessage             message to be delivered.
     * nDuration             time duration in milli-seconds.
     * </table>
     *
     * Returns
     * <table>
     * return                   description
     * ----------           ----------
     * IMS_TRUE              the message is successfully placed in to the queue.
     * IMS_FALSE             failure. usually because the message is existing in the queue.
     * </table>
     */
    virtual IMS_BOOL SendEmptyMessageDelayed(IN CONST IAosMsgHandlerListener* piListener,
            IN IMS_SINT32 nMessage, IN IMS_SINT32 nDuration) = 0;

    /*
     * Remove any pending posts of messages in the message queue.
     *
     * Remarks
     * Parameters
     * <table>
     * parameter            description
     * ----------          ----------
     * piListener             entity which is registered to receive the message.
     * nMessage             message to be delivered.
     * </table>
     *
     * Returns
     * <table>
     * return                   description
     * ----------           ----------
     * </table>
     */
    virtual void RemoveMessages(
            IN CONST IAosMsgHandlerListener* piListener, IN IMS_SINT32 nMessage) = 0;

    /*
     * Check if there are any pending posts of messages in the message queue.
     *
     * Remarks
     * Parameters
     * <table>
     * parameter            description
     * ----------          ----------
     * piListener             entity which is registered to receive the message.
     * nMessage             message to be delivered.
     * </table>
     *
     * Returns
     * <table>
     * return                   description
     * ----------           ----------
     * IMS_TRUE              the message is existing in the message queue.
     * IMS_FALSE             the message is not existing in the message queue.
     * </table>
     */
    virtual IMS_BOOL HasMessages(
            IN CONST IAosMsgHandlerListener* piListener, IN IMS_SINT32 nMessage) = 0;
};

#endif  // INTERFACE_AOS_MESSAGE_HANDLER_H_
