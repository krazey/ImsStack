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
#ifndef _INTERFACE_PARAMETER_COMPONENT_H_
#define _INTERFACE_PARAMETER_COMPONENT_H_

class IParameterComponent
{
public:
    enum
    {
        NORMAL,
        HEADER,
        URI,
        INVALID
    };

    IParameterComponent() :
            m_eComponentType(NORMAL)
    {
    }

    IParameterComponent(const IParameterComponent& objParameterComponent) :
            m_eComponentType(objParameterComponent.m_eComponentType)
    {
    }
    virtual ~IParameterComponent() {}

    inline SIP_VOID SetComponentType(SIP_INT32 eType) { m_eComponentType = eType; }

    inline SIP_INT32 GetComponentType() { return m_eComponentType; }

    virtual SIP_BOOL IsValidComponent(const SIP_CHAR* pszComponent) const = 0;

private:
    SIP_INT32 m_eComponentType;
};

#endif  // _INTERFACE_PARAMETER_COMPONENT_H_
