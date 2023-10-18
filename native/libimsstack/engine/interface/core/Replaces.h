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
#ifndef REPLACES_H_
#define REPLACES_H_

#include "AString.h"

/**
 * @brief This class defines a helper class to access Replaces header.
 */
class Replaces
{
public:
    Replaces();
    Replaces(IN const AString& strCallId, IN const AString& strLocalTag,
            IN const AString& strRemoteTag, IN IMS_BOOL bIsEarlyOnly = IMS_FALSE);
    Replaces(IN const Replaces& other);
    ~Replaces();

public:
    Replaces& operator=(IN const Replaces& other);

public:
    /**
     * @brief Parses Replaces header.
     *
     * @param strReplacesHeader Replaces header string
     * @param bUas Flag to indicate that UA mode of this Replaces header is UAS
     * @return If it's successfully parsed, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    IMS_BOOL Create(IN const AString& strReplacesHeader, IN IMS_BOOL bUas = IMS_TRUE);
    /**
     * @brief Checks if the given Replaces object is the same or not.
     *
     * @param pOther Pointer to Replaces object to be compared
     * @return If both Replaces are the same, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    IMS_BOOL Equals(IN const Replaces* pOther) const;
    /**
     * @brief Gets call-id value.
     *
     * @return Call-id value of Replaces header.
     */
    inline const AString& GetCallId() const { return m_strCallId; }
    /**
     * @brief Gets from-tag value.
     *
     * @return From-tag value of Replaces header.
     */
    inline const AString& GetFromTag() const { return m_strFromTag; }
    /**
     * @brief Gets to-tag value.
     *
     * @return To-tag value of Replaces header.
     */
    inline const AString& GetToTag() const { return m_strToTag; }
    /**
     * @brief Checks if it has "early-only" parameter or not.
     *
     * @return If it has "early-only" parameter, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsEarlyOnly() const { return m_bIsEarlyOnly; }
    /**
     * @brief Checks if it's the same dialog or not.
     *
     * It conducts the dialog comparison.
     *
     * @return If it's the same dialog, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    IMS_BOOL IsSameDialog(IN const Replaces* pOther) const;
    /**
     * @brief Returns SIP header string representation of this Repaces object.
     *
     * @return A string representation of this Replaces object.
     */
    AString ToString(IN IMS_BOOL bPercentEncoding) const;

public:
    class Dialog
    {
    public:
        inline Dialog(IN const AString& strCallId, IN const AString& strLocalTag,
                IN const AString& strRemoteTag) :
                m_strCallId(strCallId),
                m_strLocalTag(strLocalTag),
                m_strRemoteTag(strRemoteTag)
        {
        }

        inline ~Dialog() {}

        Dialog() = delete;
        Dialog(IN const Dialog&) = delete;
        Dialog& operator=(IN const Dialog&) = delete;

    public:
        inline IMS_BOOL Equals(IN const Dialog* pOther) const
        {
            if (pOther == IMS_NULL)
            {
                return IMS_FALSE;
            }

            if (!m_strCallId.Equals(pOther->m_strCallId))
            {
                return IMS_FALSE;
            }

            if (!m_strLocalTag.Equals(pOther->m_strLocalTag))
            {
                return IMS_FALSE;
            }

            if (!m_strRemoteTag.Equals(pOther->m_strRemoteTag))
            {
                return IMS_FALSE;
            }

            return IMS_TRUE;
        }

        inline const AString& GetCallId() const { return m_strCallId; }
        inline const AString& GetLocalTag() const { return m_strLocalTag; }
        inline const AString& GetRemoteTag() const { return m_strRemoteTag; }

    private:
        const AString& m_strCallId;
        const AString& m_strLocalTag;
        const AString& m_strRemoteTag;
    };

private:
    AString m_strCallId;
    AString m_strFromTag;
    AString m_strToTag;
    IMS_BOOL m_bIsEarlyOnly;

    Dialog* m_pDialog;
};

#endif
