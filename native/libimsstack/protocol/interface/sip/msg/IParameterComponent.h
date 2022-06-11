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

    inline SIP_VOID SetComponentType(SIP_INT32 eType) { m_eComponentType = eType; }

    inline SIP_INT32 GetComponentType() { return m_eComponentType; }

    virtual SIP_BOOL IsValidComponent(const SIP_CHAR* pszComponent) const = 0;

private:
    SIP_INT32 m_eComponentType;
};

#endif  // _INTERFACE_PARAMETER_COMPONENT_H_
