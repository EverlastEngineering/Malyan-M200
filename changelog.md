# Changelog

## Version 58 - Firmware and Web UI

> The version as sent from Malyan was 47. To avoid conflicts with existing version numbers, I decided to start at the nice round number of **58**.

_This release is still based on Marlin 1.1.0 RC2 and doesn't address anything related to the printing itself. Since I have a very stable setup, I didn't want to have to reslice at tweak all my gcode that I use for production. I also wanted to maintain full support for the LCD. Think of this release as a 'quality of life' release, making it more tolerable to work with on a daily basis._

### Parallel heaters - Super Fast Heatup!

Previously, preheating was very slow because the bed was only enabled to heat when the extruder heater _wasn't_ drawing current. During preheat or an extruder temperature increase, this meant that the bed heater would cool quickly, and often wasn't given PWM cycles to regain losses during these period. I discovered this by monitoring the power consumption with a logging meter after finding that a larger power supply did nothing to help heating.

Sure enough, the source had logic that only enabled the bed heater when the extruder heater wasn't needed. Interestingly, it was commented out in version 47, but it was easy to see that it was in place for version 42 of the controller software that we have had available. This is now fixed and both heaters can go full power.

If you're unsure of the power supply's capability to handle this, then I suggest you fully preheat the bed before the extruder, which is what I used to do anyway. This way, the bed is into a PID cycle when the extruder needs to go full tilt.

### Verbose Reporting Over Wifi

Previously, the commands to set parameters such as acceleration, flow, and others, were quiet and only responded with `ok`. Furthermore, the EEPROM reporting command `M503` only reported settings over USB which is often non-function or spotty. I'm still unsure of the reasoning behind this choice, but I've enabled the ability to report this data over the WebUi. Many commands that take setting changes will now report the change and if called with no parameter will report all their settings. These include:
 
 - `M118` - Echo, useful to send comments from code to UI
 - `M201` - Travel Move Limits
 - `M203` - Max Feedrate
 - `M204` - Starting Accleration
 - `M205` - Jerk and other Advanced Settings
 - `M220` - Feedrate Percentage
 - `M221` - Flow Percentage
 - `M300` - Tone "Support"

 ### `M154` and `M155`: Auto Report Position and Temperature

 This build fully supports `M154` and `M155` which push position and temperature data over the websockets connection. This includes detailed temperature data with PWM factor to show the relative power output from the heaters. Future WebUi will use this to show the heater power visually and positional data similar to how Mainsail does it.

 The data returned is that of `M114` and `M105` respectively.

 ### Full File and Folder Support from WebUi

 The previous iteration of file support from the WebUi was glitchy at best, from bugs in both the WebUi and the firmware. 

 This build supports deep folders, 64 character long file names and is much more reliable. In order to fully maintain compatiblity with the LCD controller, we employed custom methods using `M23` to traverse folders. 

 Also, when the print begins the firmware notifies the LCD immediately which allows for greater control and prevents the machine from going unresponsive during preheat.

 ### WebUi Improvements

 What started as a small set of tweaks to make my day better when printing ended up turning into a week long marathon of improvements.
 

 - Gcode input box auto capitalizes your Gcode.
 - Hitting enter in the Gcode box submits the code.
 - Up / Down arrows scroll through previous commands, and the buffer doesn't add a command if you just sent it. This allows you to hit "up->enter" over and over to send the same command, useful when troubleshooting.
 - You can clear the console with a garbage can, when you scrolllllll all the way to the top to find it haha.
 - Massive file management improvements with folders, stability and better interface.
 - No more print button, because WTF did it print?
 - Button enable / disable improvements.
 - Quick Preheat / Heater Off buttons
 - Visible indicator when a heater is on (red)
 - Smaller size movements
 - FUN NEW COLORS AND FONTS

 I have a laundry list of things I'd like to improve in this UI and may yet do so. Mostly, they centre around macros or troubleshooting.

 I will be adopting the `M155` code to get better heater information than provided by the `/inquiry/` endpoint, now that it's enabled in the firmware.

 ### Other Items To Note

 - The fan will _not_ enable if the extruder is less than 100C. It fine, just different than version 42.

 - The PID settings for the extruder need to be recalculated once the heaters are unencumbered by the PWM limitations from previous builds.

### Testing

If you'd like to try out this build, PLEASE be aware that this is only tested on my machine and I make NO claim that this will do anything but break your machine.

"If this breaks your machine, you get to keep both halves."

This build will ONLY work on a machine which already has version 42. The update procedure is the same as the OEM firmware.