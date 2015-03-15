## Introduction ##
This Windows tray application allows you to select a subset of your current audio output devices and quickly switch between them with a simple double-click on the icon.

## Features ##
Taskbar Sound Switcher is a Win32 application that runs in the Windows taskbar area. With a simple double-click on the taskbar icon, you can instantly change the audio output device from a list of pre-selected audio output devices on your computer.  A right-click allows you to select the device you want directly from a drop-down menu or you can re-select which devices you wish to toggle between.  Your selection is saved to a configuration file and remembered each time you start the app.

## Usage ##

When you first open the application, you'll be greeted with a pop-up dialog that shows all of your audio output devices.  This is the same list you'll see if you right-click the built-in sound tray icon and select 'Playback devices'

![http://i61.tinypic.com/2eap0yq.jpg](http://i61.tinypic.com/2eap0yq.jpg)

From this list, select all the different devices you'd like to quick-switch between. You can pick as many devices from this list if you'd like. In my case, I am choosing between my speakers and my Logitech headset.

![http://i62.tinypic.com/jfewq8.jpg](http://i62.tinypic.com/jfewq8.jpg)


You'll now see the tray icon in your taskbar area.  By double-clicking on the icon, you can change between the devices you selected between.
![http://i62.tinypic.com/yhx0m.jpg](http://i62.tinypic.com/yhx0m.jpg)

The tray icon can change if your device happens to have keywords in the name of the device that indicate it's a headset or speakers
![http://i62.tinypic.com/107tobl.jpg](http://i62.tinypic.com/107tobl.jpg)

If you right-click on the tray icon, you'll get a quick-select menu that would allow you to select a desired audio output, re-select the list of devices you want to toggle between, or exit the app.

![http://i60.tinypic.com/28lc6fc.jpg](http://i60.tinypic.com/28lc6fc.jpg)


## Supported Platforms ##
This is an Windows-based application. Unfortunately, the audio device switching routines are undocumented and officially unsupported by Microsoft.  While these routines have been tested on a number of platforms, there is no guarantee they will work for all devices and all configurations.

If you find a situation in which Taskbar Sound Switcher does not work, please report your audio device and OS/Service Pack version.

Tested platforms:
Windows 7
Windows 8.0
Windows 8.1

## Legal ##
This application is distributed under the 'new' BSD license:

Copyright (c) 2015, Matthew Fife
All rights reserved.
Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.