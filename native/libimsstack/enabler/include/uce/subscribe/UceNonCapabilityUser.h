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

#ifndef UCE_NON_CAPABILITY_USER_H_
#define UCE_NON_CAPABILITY_USER_H_

class UceNonCapabilityUser
{
public:
    explicit UceNonCapabilityUser(const AString& strId, const AString& strReason) :
            m_strId(strId),
            m_strReason(strReason){};
    virtual ~UceNonCapabilityUser() {}

    AString& GetId() { return m_strId; }
    AString& GetReason() { return m_strReason; }

private:
    AString m_strId;
    AString m_strReason;
};

class UceNonCapabilityUsers
{
public:
    explicit UceNonCapabilityUsers(){};
    virtual ~UceNonCapabilityUsers()
    {
        for (IMS_UINT32 i = 0; i < m_lstUceNonCapabilityUsers.GetSize(); i++)
        {
            if (m_lstUceNonCapabilityUsers.GetAt(i) != null)
                delete m_lstUceNonCapabilityUsers.GetAt(i);
        }
    }
    void SetNonCapabilityUser(UceNonCapabilityUser* user)
    {
        m_lstUceNonCapabilityUsers.Append(user);
    }

    IMSList<UceNonCapabilityUser*> GetNonCapabilityUser() const
    {
        return m_lstUceNonCapabilityUsers;
    }

private:
    IMSList<UceNonCapabilityUser*> m_lstUceNonCapabilityUsers;
};

class UcePidfXmls
{
public:
    explicit UcePidfXmls(){};
    virtual ~UcePidfXmls() {}
    void SetPidfXml(const AString& pidfXml) { m_lstPidfXmls.Append(pidfXml); }

    IMSList<AString> GetPidfXmls() const { return m_lstPidfXmls; }

private:
    IMSList<AString> m_lstPidfXmls;
};
#endif /* UCE_NON_CAPABILITY_USER_H_ */
