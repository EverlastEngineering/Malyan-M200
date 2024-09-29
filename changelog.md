# Changelog

## Version 58 - Firmware and Web UI

> The version as sent from Malyan was 47. To avoid conflicts with existing version numbers, I decided to start the nice round number of **58**.

This release is still based on Marlin 1.1.0RC2 and doesn't address anything related to the printing _itself_ since I have a very stable setup and I didn't want to have to reslice at tweak all my gcode that I use for production.

### _Bug Fixes_
---

#### Parallel heaters - Super Fast Heatup!

Previously, the bed was only triggered to heat when the extruder heater wasn't drawing current. During preheat or temperature increase, this meant that the bed heater would cool quickly, and often didn't have enough power to regain losses during these period. I discovered this by monitoring the power consumption with a logging meter.

Sure enough, while the code I received from Malyan had that logic commented out, it's easy to see that it was in place for version 42 of the controller software that we have available. This is now removed and both heaters can go full power.

If you're unsure of the power supply's capability to handle this, then I suggest you fully preheat the bed before the extruder, which is what I did anyway. This way the bed is into a PID cycle when the extruder needs to go full tilt.

