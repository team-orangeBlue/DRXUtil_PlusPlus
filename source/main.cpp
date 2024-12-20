#include "Gfx.hpp"
#include "ProcUI.hpp"
#include "screens/MainScreen.hpp"
#include <memory>

#include <vpad/input.h>
#include <padscore/kpad.h>
#include <sndcore2/core.h>

VPADStatus globalVPADStatus;

namespace
{

uint32_t RemapWiiMoteButtons(uint32_t buttons)
{
    uint32_t conv_buttons = 0;

    if (buttons & WPAD_BUTTON_LEFT)
        conv_buttons |= VPAD_BUTTON_LEFT;

    if (buttons & WPAD_BUTTON_RIGHT)
        conv_buttons |= VPAD_BUTTON_RIGHT;

    if (buttons & WPAD_BUTTON_DOWN)
        conv_buttons |= VPAD_BUTTON_DOWN;

    if (buttons & WPAD_BUTTON_UP)
        conv_buttons |= VPAD_BUTTON_UP;

    if (buttons & WPAD_BUTTON_PLUS)
        conv_buttons |= VPAD_BUTTON_PLUS;

    if (buttons & WPAD_BUTTON_B)
        conv_buttons |= VPAD_BUTTON_B;

    if (buttons & WPAD_BUTTON_A)
        conv_buttons |= VPAD_BUTTON_A;

    if (buttons & WPAD_BUTTON_MINUS)
        conv_buttons |= VPAD_BUTTON_MINUS;

    if (buttons & WPAD_BUTTON_HOME)
        conv_buttons |= VPAD_BUTTON_HOME;

    return conv_buttons;
}

uint32_t RemapClassicButtons(uint32_t buttons)
{
    uint32_t conv_buttons = 0;

    if (buttons & WPAD_CLASSIC_BUTTON_LEFT)
        conv_buttons |= VPAD_BUTTON_LEFT;

    if (buttons & WPAD_CLASSIC_BUTTON_RIGHT)
        conv_buttons |= VPAD_BUTTON_RIGHT;

    if (buttons & WPAD_CLASSIC_BUTTON_DOWN)
        conv_buttons |= VPAD_BUTTON_DOWN;

    if (buttons & WPAD_CLASSIC_BUTTON_UP)
        conv_buttons |= VPAD_BUTTON_UP;

    if (buttons & WPAD_CLASSIC_BUTTON_PLUS)
        conv_buttons |= VPAD_BUTTON_PLUS;

    if (buttons & WPAD_CLASSIC_BUTTON_X)
        conv_buttons |= VPAD_BUTTON_X;

    if (buttons & WPAD_CLASSIC_BUTTON_Y)
        conv_buttons |= VPAD_BUTTON_Y;

    if (buttons & WPAD_CLASSIC_BUTTON_B)
        conv_buttons |= VPAD_BUTTON_B;

    if (buttons & WPAD_CLASSIC_BUTTON_A)
        conv_buttons |= VPAD_BUTTON_A;

    if (buttons & WPAD_CLASSIC_BUTTON_MINUS)
        conv_buttons |= VPAD_BUTTON_MINUS;

    if (buttons & WPAD_CLASSIC_BUTTON_HOME)
        conv_buttons |= VPAD_BUTTON_HOME;

    if (buttons & WPAD_CLASSIC_BUTTON_ZR)
        conv_buttons |= VPAD_BUTTON_ZR;

    if (buttons & WPAD_CLASSIC_BUTTON_ZL)
        conv_buttons |= VPAD_BUTTON_ZL;

    if (buttons & WPAD_CLASSIC_BUTTON_R)
        conv_buttons |= VPAD_BUTTON_R;

    if (buttons & WPAD_CLASSIC_BUTTON_L)
        conv_buttons |= VPAD_BUTTON_L;

    return conv_buttons;
}

void UpdatePads(VPADStatus* status)
{
    KPADStatus kpad_data{};
    KPADError kpad_error;
    for (int i = 0; i < 4; i++) {
        if (KPADReadEx((KPADChan) i, &kpad_data, 1, &kpad_error) > 0) {
            if (kpad_error == KPAD_ERROR_OK && kpad_data.extensionType != 0xFF) {
                if (kpad_data.extensionType == WPAD_EXT_CORE || kpad_data.extensionType == WPAD_EXT_NUNCHUK) {
                    status->trigger |= RemapWiiMoteButtons(kpad_data.trigger);
                    status->release |= RemapWiiMoteButtons(kpad_data.release);
                    status->hold |= RemapWiiMoteButtons(kpad_data.hold);
                } else {
                    status->trigger |= RemapClassicButtons(kpad_data.classic.trigger);
                    status->release |= RemapClassicButtons(kpad_data.classic.release);
                    status->hold |= RemapClassicButtons(kpad_data.classic.hold);
                }
            }
        }
    }
    globalVPADStatus = *status;
}

}

int main(int argc, char const* argv[])
{
    ProcUI::Init();

    // call AXInit to stop already playing sounds
    AXInit();

    KPADInit();
    WPADEnableURCC(TRUE);

    Gfx::Init();

    std::unique_ptr<Screen> mainScreen = std::make_unique<MainScreen>();

    while (ProcUI::IsRunning()) {
        VPADRead(VPAD_CHAN_0, &globalVPADStatus, 1, nullptr);
        UpdatePads(&globalVPADStatus);

        if (!mainScreen->Update(globalVPADStatus)) {
            ProcUI::StopRunning();
        }

        mainScreen->Draw();
        Gfx::Render();
    }

    mainScreen.reset();

    Gfx::Shutdown();

    AXQuit();

    ProcUI::Shutdown();
    return 0;
}
