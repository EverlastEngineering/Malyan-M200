# Malyan 200 / Monoprice Mini Select V2 Firmware

This repo contains the original source code provided by Malyan and was previously unreleased to my knowledge. 

I have taken this source and improved some quality of life elements while preserving the original print functionality and ensure full compatibility with the onboard LCD controller. 

The biggest improvement is the heater warmup time; the machine can now draw full current for both heaters at the same time. In two minutes, my machine can now go from 17C/17C to 215C/55C! Another minute and the bed hits a rock solid 65C, a temperature I could only achieve in the past without the extruder heater running.

I've also forked [Joey Cortez's WebUi](https://github.com/joeycortez42/MP-Select-Mini-Web) and improved it along with the supporting functionality in the firmware for folder support, long filenames, and stability.

If you try this out, please be sure to let me know at jason@everlastengineering.com and if you have any problems, [raise an issue here on Github](https://github.com/EverlastEngineering/Malyan-M200/issues) and perhaps I can help out.
