# DRXUtil
DRC/DRH experiments.

Features:
* Program DRC firmware (e.g. to write arbitrary EEPROM data)
* Program DRC language (to avoid using the settings app which isn't exactly reliable)
* Program DRC fully (e.g. to downgrade/upgrade it)
* Program DRH (feature completeness)
For modified firmwares:
* Set EEPROM region byte (to pair with an unmodded console enforcing region checks)
* Dump DRC EEPROMs (to backup)
* Pair DRC (because yes!)
* Enable DevKit menu (available on all retail DRCs), boot with L+ZL
* Enable TESTMODE (factory mode)
* Set initial boot flag to effectively factory reset the DRC

## Disclaimer
Modifying the DRC firmware and settings to non-production values can cause permanent damage.  
No one but yourself is responsible for any sort of damage resulting from using this tool.

## Building
For building you need: 
- [wut](https://github.com/devkitPro/wut)
- [libmocha](https://github.com/wiiu-env/libmocha)
- wiiu-sdl2
- wiiu-sdl2_ttf

To build the project run `make`.

## See also
- [drc-fw-patches](https://github.com/GaryOderNichts/drc-fw-patches)
