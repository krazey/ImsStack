/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100428  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _ENGINE_STATE_H_
#define _ENGINE_STATE_H_

// Initialization / Uninitialization for Engine
class EngineState
{
public:
    static IMS_BOOL Initialize();
    static void Uninitialize();
};

#endif  // _ENGINE_STATE_H_
