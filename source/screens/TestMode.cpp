#include "TestMode.hpp"
#include "Utils.hpp"
#include "Gfx.hpp"

#include <nsysccr/cdc.h>
#include <sysapp/launch.h>
#include <nsysccr/cfg.h>

namespace {

bool SetBoardConfig(uint8_t byte)
{
    CCRCDCUicConfig cfg{};
    // custom config id which was added by the gamepad cfw
    cfg.configId = 4;
    // board config byte + crc16
    cfg.size = 3;
    cfg.data[0] = byte;
    uint16_t crc = CCRCDCCalcCRC16(cfg.data, 1);
    cfg.data[1] = crc & 0xff;
    cfg.data[2] = (crc >> 8) & 0xff;
    if (CCRCDCPerSetUicConfig(CCR_CDC_DESTINATION_DRC0, &cfg) != 0) {
        return false;
    }

    // Also update the cached eeprom
    return CCRCFGSetCachedEeprom(0, 0x106, cfg.data, cfg.size) == 0;
}

bool GetBoardConfig(uint8_t& boardConfig)
{
    uint8_t data[3];
    if (CCRCFGGetCachedEeprom(0, 0x106, data, 3) != 0) {
        return false;
    }

    uint16_t crc = (uint16_t) data[2] << 8 | data[1];
    if (CCRCDCCalcCRC16(&data[0], 1) != crc) {
        return false;
    }

    boardConfig = data[0];
    return true;
}

}

EnableTestMode::EnableTestMode()
 : mConfirm(false),
   mErrorText()
{
}

EnableTestMode::~EnableTestMode()
{
}

void EnableTestMode::Draw()
{
    DrawTopBar("TestMode");

    switch (mState)
    {
        case STATE_SELECT:
            Gfx::Print(Gfx::SCREEN_WIDTH / 2, 128, 64, Gfx::COLOR_TEXT,
                Utils::sprintf("This will reset the shipped bit in UIC EEPROM to 0.\nDRC will power off, and system menu will open.\n Execute?"),
                Gfx::ALIGN_HORIZONTAL | Gfx::ALIGN_TOP);
            for (int i = 0; i < 2; i++) {
                int yOff = 360 + static_cast<int>(i) * 100;
                Gfx::DrawRectFilled(0, yOff, Gfx::SCREEN_WIDTH, 100, Gfx::COLOR_ALT_BACKGROUND);
                Gfx::Print(68, yOff + 100 / 2, 50, Gfx::COLOR_TEXT, i == 0 ? "No" : "Yes", Gfx::ALIGN_VERTICAL);

                if (mConfirm == !!i) {
                    Gfx::DrawRect(0, yOff, Gfx::SCREEN_WIDTH, 100, 8, Gfx::COLOR_HIGHLIGHTED);
                }
            }
            break;
        case STATE_CONFIRM:
            Gfx::Print(Gfx::SCREEN_WIDTH / 2, Gfx::SCREEN_HEIGHT / 2, 64, Gfx::COLOR_TEXT,
                Utils::sprintf("Are you sure you want to enter testmode?\nThis may be dangerous."), Gfx::ALIGN_CENTER);
            break;
        case STATE_UPDATE:
            Gfx::Print(Gfx::SCREEN_WIDTH / 2, Gfx::SCREEN_HEIGHT / 2, 64, Gfx::COLOR_TEXT, "Updating board config...", Gfx::ALIGN_CENTER);
            break;
        case STATE_DONE:
            Gfx::Print(Gfx::SCREEN_WIDTH / 2, Gfx::SCREEN_HEIGHT / 2, 64, Gfx::COLOR_TEXT,
                Utils::sprintf("Complete!"),
                Gfx::ALIGN_CENTER);
            break;
        case STATE_ERROR:
            Gfx::Print(Gfx::SCREEN_WIDTH / 2, Gfx::SCREEN_HEIGHT / 2, 64, Gfx::COLOR_ERROR, "Error!\n" + mErrorText, Gfx::ALIGN_CENTER);
            break;
    }

    if (mState == STATE_SELECT) {
        DrawBottomBar("\ue07d Navigate", "\ue044 Exit", "\ue000 Confirm / \ue001 Back");
    } else if (mState == STATE_CONFIRM) {
        DrawBottomBar(nullptr, "\ue044 Exit", "\ue000 Confirm / \ue001 Back");
    } else if (mState == STATE_UPDATE) {
        DrawBottomBar(nullptr, "Please wait...", nullptr);
    } else {
        DrawBottomBar(nullptr, "\ue044 Exit", "\ue001 Back");
    }
}

bool EnableTestMode::Update(VPADStatus& input)
{
    switch (mState)
    {
        case STATE_SELECT:
            if (input.trigger & VPAD_BUTTON_B) {
                return false;
            }

            if (input.trigger & VPAD_BUTTON_A) {
                if (mConfirm) {
                    mState = STATE_CONFIRM;
                } else {
                    return false;
                }
                break;
            }

            if (input.trigger & (VPAD_BUTTON_DOWN | VPAD_BUTTON_UP)) {
                mConfirm = !mConfirm;
            }
            break;
        case STATE_CONFIRM:
            if (input.trigger & VPAD_BUTTON_B) {
                return false;
            }

            if (input.trigger & VPAD_BUTTON_A) {
                mState = STATE_UPDATE;
                break;
            }
            break;
        case STATE_UPDATE: {
            uint8_t newBoardConfig = 0x10;
            if (!SetBoardConfig(newBoardConfig)) {
                mErrorText = "Failed to set board config.\n(Is the firmware modified properly?)";
                mState = STATE_ERROR;
                break;
            }
            CCRCDCSysConsoleShutdownInd(CCR_CDC_DESTINATION_DRC0); // Power off device to apply changes
            mState = STATE_DONE;
            break;
        }
        case STATE_DONE: {
            SYSLaunchMenu();
            break;
        }
        case STATE_ERROR: {
            if (input.trigger & VPAD_BUTTON_B) {
                return false;
            }
            break;
        }
    }

    return true;
}