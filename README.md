# Quest port of https://github.com/ToniMacaroni/TrickSaber.
# Updated for Beat Saber 1.13.0

Follows https://tonimacaroni.github.io/TrickSaber-Docs/Configuration (config at /sdcard/Android/data/com.beatgames.beatsaber/files/mod_cfgs/TrickSaber.json) with a few differences:
1. Her "EnableCuttingDuringTrick" is my "EnableTrickCutting"
2. Her "IsSpeedVelocityDependent" is my "IsSpinVelocityDependent"

Added options:
1. "regenerateConfig": when set to true, the config file will be completely rewritten to have all default values on next config load.
2. "ButtonOneAction": action triggered by A or X.
3. "ButtonTwoAction": action triggered by B or Y.

To change your config, simply drag your editted TrickSaber.json into the BMBF upload box (as you did for the mod zip itself).

Also, there is no UI, pending https://github.com/zoller27osu/Quest-BSML.

Changed drastically from the original Quest port by Rugtveit, so please do not bother him.

# Credits:
Ported to Beat Saber 1.13.0 by Fernthedev. Many thanks to the original authors of the Quest port for doing most of the work, and the Discord community for giving lots of support, no fuss. 

This is my first Beat Saber Mod (and successful C++ project) so many bugs and ugly code may be within the project.