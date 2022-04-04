#ifndef _INTERFACE_PARAMETER_COMPONENT_H_
#define _INTERFACE_PARAMETER_COMPONENT_H_

class IParameterComponent
{
    public:
        virtual SIP_BOOL IsValidComponent(const SIP_CHAR* pszComponent) const = 0;
};

#endif // _INTERFACE_PARAMETER_COMPONENT_H_