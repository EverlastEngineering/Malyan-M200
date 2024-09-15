# Malyan OEM Source Code for M200 / Mini Select

I'd reached out a number of times to Malyan to get the OEM source for the M200 and Yongzong Lin graciously reply in June 2024 with the OEM source for the factory firmware for the M200.

He did note this, however:

> As far as I know, Marlin already support M200/M300 officially. We tried to build Marlin on VSCode successfully for M300 some month ago, which is our recommended way. 
>
> Also I can provide you our modified Marlin source code file, but you may need IAR IDE to compile. Here is the link: https://we.tl/t-Fnka1HMlG1
>
> Please aware that, M200 have many hardware revisions. You need to make sure your configuration is correct. 

Of interest to me is the custom handling of the `{}` codes and communication with the second serial line. 

Some goals with this:

- ensure all the factory options are available in Marlin 2
- use as a resource for Klipper configuration
- recompile a Marlin V1 firmware with the option to allow both beds to heat at once (which is the reason for the slow warmup times)
