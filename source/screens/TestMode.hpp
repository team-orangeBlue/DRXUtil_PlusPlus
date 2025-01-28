#pragma once

#include "Screen.hpp"
#include <map>
#include <nsysccr/cdc.h>

class EnableTestMode : public Screen
{
public:
    EnableTestMode();
    virtual ~EnableTestMode();

    void Draw();

    bool Update(VPADStatus& input);

private:
    enum State {
        STATE_SELECT,
        STATE_CONFIRM,
        STATE_UPDATE,
        STATE_DONE,
        STATE_ERROR,
    } mState = STATE_SELECT;

    bool mConfirm;
    std::string mErrorText;
};