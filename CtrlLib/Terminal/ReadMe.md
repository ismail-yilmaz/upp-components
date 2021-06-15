# Terminal Package for U++

Terminal package is a flexible, easy-to-use yet powerful cross-platform virtual terminal emulation widget and library written in C/C++ for [U++](https://www.ultimatepp.org/).  

Terminal package is designed from the ground up with modularity and maintainability in mind, and it is not based on the existing terminal emulation libraries.

## Table of Contents

 1.  [Requirements](#requirements)
 2.  [Installlation](#installation)
 3.  [Version](#version)
 4.  [Highlights](#highlights)
 5.  [Features](#features)
 6.  [Specifications](#specs)
 7.  [Examples](#examples)
 8.  [Screenshots](#screenshots)
 9.  [Videos](#videos)
 10. [Acknowledgements](#acknowledgements)
 11. [License](#license)
 

## [Requirements](#requirements)

- U++ (ver. >= 2020.1)
- POSIX (GNU/Linux, FreeBSD, etc.), Windows, or MacOS
- A decent C/C++ compiler that supports at least C++14. (GCC/CLANG/MinGW/MSC)
- Snacks & beer.

## [Installation](#installation)

There are two ways to install the source code of Ultimate++ terminal widget:

1. The package is immediately available via [UppHub](https://www.ultimatepp.org/app$ide$UppHub_en-us.html), the 3rd-party source package management system of U++. *This is the simplest and recommended method.* (Requires U++ ver. > 2020.2, or nighty builds.)
2. Clone or download this repository and set it up as an U++ *assembly* or *nest*.  You can find more information on U++ packages, assemblies and nests, [here](https://www.ultimatepp.org/app$ide$PackagesAssembliesAndNests$en-us.html).

##  [Version](#version)  
  
`Terminal` package loosely follows the release cycles of U++. Releases are tagged twice a year. Currently it is tagged as `2021.1` (v0.5).

## [Highlights](#highlights)

- **Terminal package completely separates the virtual terminal from the pseudo-terminal process (pty)**.
As a result, Terminal package and TerminalCtrl, are not bound by any platform-specific pty implementation. Instead, they are decoupled, and an optional pty process class, `PtyProcess` which encapsulates `POSIX`, `Windows 10` and [winpty](https://github.com/rprichard/winpty) pseudoconsole APIs, is provided with tha package simply as the default option. In this way, using TerminalCtrl on any platform supported by U++, directly as a front-end for some other terminal based services, such as SSH or TELNET, etc., has become possible. For example, U++ terminal package can be compiled, run and used on supported platforms as a pure SSH2 terminal. (See the *Examples* section.)

- **Terminal package is designed with simplicity in mind.**
A fully-fledged terminal emulation requires less than 50 sLoC. In fact, the first basic example provided with the package is only a single .cpp file with 29 sLoC, and it can run complex/heavy applications with mouse tracking and embedded images support, such as [GNU Emacs](https://www.gnu.org/software/emacs/), [vim](https://github.com/vim/vim) text editor, [Lynx](https://lynx.browser.org/), [GNUPlot](http://www.gnuplot.info/), [tmux](https://github.com/tmux/tmux/wiki), [Ranger](https://github.com/ranger/ranger), a vim inspired file manager with inline image preview support, or [mapscii](https://github.com/rastapasta/mapscii), an OpenStreetMap implementation for [xterm](https://invisible-island.net/xterm/) compatible virtual terminal emulator, or even [Jexer](https://jexer.sourceforge.io/), a java-based modern and slick text user interface (TUI) and windowing system for modern terminal emulators, and [xterm Window Manager,](https://gitlab.com/klamonte/xtermwm) with ease.

- **Terminal package combines simplicity with configurability.** 
Although it is easy to use, and requires very little coding, TerminalCtrl, is by no means restrictive. It it highly configurable.

- **TerminalCtrl is a regular U++ widget.**
It is derived from Upp::Ctrl, and is following the same basic rule: *Everthing belongs somewhere*. It supports most of the generic Ctrl methods where applicable or makes sense. Of course, If you are determined enough, you can even do some “interesting” things, such as embedding Terminal instances into TreeCtrl nodes or Arrayctrl rows. ;)

- ***Everything belongs somewhere* rule runs through the heart of Terminal package.**
There are no manual memory allocations/deallocations, no new/delete pairs, and no smart/not-so-smart/shared pointers in the code; only the containers, and extensive application of the [RAII](https://www.wikiwand.com/en/Resource_acquisition_is_initialization) principle.

- **TerminalCtrl supports true color.**
TerminalCtrl is a true color (24-bit/16M color) virtual terminal emulator. It supports RGB, CMY, CMYK, and  indexed (256-color) palettes via SGR extended colors sequences. 

- **TerminalCtrl supports inline images.**
It has a flexible infrastructure and support for inline images and image manipulation in general. It can handle [sixel graphics](https://en.wikipedia.org/wiki/Sixel?oldformat=true) with 4/16/256 colors, or high/true color.  It also supports JPG, PNG, BMP raster image formats, or raw RGB images via [iTerm2's inline images protocol](https://iterm2.com/documentation-images.html),  and [jexer image protocol](https://gitlab.com/klamonte/jexer/-/wiki_pages/jexer-images), a simple and useful wire protocol which allows terminals to display popular true color image formats. In fact, since TerminalCtrl uses the common raster decoding api of U++, theoretically it can display any raster image that has a registered decoder.  Terminal ctrl uses Upp::Display objects to display the embedded images. Client code can set the image display to one of the predefined display objects that'll process or manipulate the images before they are displayed (stretch/scale/colorize/flip/add text, etc., you name it), and the changes will immediately take place. Moreover, developers can create their own cell displays tailored for their specific needs. TerminalCtrl also supports an external image viewing mode, where the image data is handed to client code for rendering and external viewing.

- **TerminalCtrl can also run inside a web browser such as Firefox and Chromium, or their derivatives.**
Thanks to U++ team, it is possible to run U++ GUI applications from within a web browser that supports HTML-5 canvas and websockets. And TerminalCtrl is no exception. Applications using TerminalCtrl widget can basically turn into a remote terminal that can be accessed via any decent web browser (even from a smartphone!) if compiled with the TURTLE flag. (See the *Examples* section).

- **Terminal package has a `BSD 3-Clause license`**.

- **TerminalCtrl can run Crysis.**
  Amazing, isn't it?

## [Features](#features)

- Supports whatever platform U++ supports. (Linux, Windows, MacOS).
- Supports `POSIX`, `Windows (tm) 10`, and `winpty` pseudoconsole APIs via a unified, basic interface, using the PtyProcess class.
- Supports VT52/VT1xx/VT2xx, partial VT4XX/5XX, and xterm emulation modes.
- Supports user configurable device conformance levels (1, 2, 3, 4, and 0 as VT52 emulation).
- Supports both 7-bit and 8-bit I/O.
- Supports Unicode/UTF8.
- Supports user configurable, legacy “g-set” (G0/G1/G2/G3), and related shifting functions (LS0/LS1/LS1R/LS2/LS2R/LS3/LS3R/SS2/SS3).
- Supports ANSI conformance levels.
- Supports scalable fonts. (The changes in font size and/or face immediately take place.)
- Supports various terminal state, device, and mode reports.
- Supports DEC VT52 graphics charset, VT1xx line-drawing charset, VT2xx multinational charset, and VT3xx technical charset.
- Supports VT52/VT1xx/VT2xx and PC-style keyboard emulation with function keys (i.e. DECFNK).
- Supports both pre-coded accelerator keys and run-time configurable accelerator keys.
- Supports UDK (DEC’s user-defined function keys feature).
- Supports user configurable blinking text and blink interval.
- Supports Display objects.
- Supports inline images with true color (sixel, jpeg, png, bmp, tiff, etc).
- Supports external handling of images.
- Supports ANSI + aixterm colors (16 colors palette).
- Supports true color (16 million colors).
- Supports extended colors sequences .
- Supports RGB, CMY, CMYK and indexed color palettes via extended color sequences.
- Supports xterm dynamic colors and color setting (dynamic ink/paper/selection colors).
- Supports rgb, rgba, cmy, cmyk, hash6 and older hash3, hash9, hash12 color text specifications.
- Supports background color erase (BCE).
- Supports transparency, i.e. allows background images, even animations. It's up to client code.
- Supports VT4xx rectangular area operations: copy, invert, fill. erase.
- Supports VT4xx rectangular area checksum calculation and reporting.
- Supports both DEC and ISO style selective erases.
- Supports reverse wrap.
- Supports SGR overline attribute.
- Supports alternate screen buffer.
- Supports history/scrollback buffer.
- Has a user switchable scrollbar.
- Supports xterm style alternate scroll.
- Supports resize (and optional lazy resize to reduce flicker on network terminals such as SSH-based ones).
- Supports both immediate display refresh and delayed (buffered) display refresh.
- Supports xterm style mouse tracking: button, wheel, motion, focus in/out events.
- Supports a large portion of xterm's window ops (window reports and actions).
- Supports user configurable cursor styles (block, beam, underscore, blinking/steady).
- Supports cursor locking.
- Supports basic clipboard operations on texts, hyperlinks, and images.
- Supports application clipboard/selection manipulation protocol (OSC 52)
- Supports basic drag and drop operations on texts, hyperlinks and images.
- Shows drag and drop animations (i.e thumbnails/samples of images, hyperlinks and plain texts)
- Supports X11-style copy-on-select.
- Supports rectangle selection.
- Supports bracketed paste mode.
- Supports [explicit hyperlinks.](https://gist.github.com/egmontkob/eb114294efbcd5adb1944c9f3cb5feda) (OSC 8)
- Has a predefined yet completely re-programmable context menu (right mouse button menu).
- Supports window titles.
- Supports bell notifications.
- Supports VT1xx LEDs.
- Supports size hint.
- Supports U++ style data serialization (binary serialization, JSONization, XMLization).
- Supports per-widget customization (i.e no global variables or properties are used).
- Includes a Terminal.usc file for TheIDE’s layout editor.

## [Specifications](#specs)

-  For more information on the supported terminal sequences, modes, extensions, etc., see [the technical specifications document](./Specs.md).

## [Examples](#examples)

As it is already noted above, one of the strengths of TerminalCtrl is that it allows you to do more with less. Several examples are provided with the package to illustrate this point with its different aspects.

| Name                         | Description                                                        |
|---                           |---                                                                 |
| TerminalExample              | Demonstrates a simple local terminal.                              |
| SshTerminalExample           | Demonstrates a simple ssh2 terminal.                               |
| TerminalInWebBrowserExample  | Demonstrates a simple terminal, running remotely over web browsers.|
| TerminalSplitterExample      | Demonstrates multiple local terminals, using a splitter widget.    |
| SshTerminalSplitterExample   | Demonstrates multithreaded ssh2 terminals, using a splitter widget.|
| TabbedTerminalExample        | Demonstrates multiple local terminals, using a tab widget.         |
| TabbedSshTerminalExample     | Demonstrates multithreaded ssh2 terminals, using a tab widget.     |
| TerminalLayoutExample        | Demonstrates simple gui building, using TheIDE's layout editor.    |

### Basic Terminal Example

This example demonstrates the basic usage of the TerminalCtrl and its interaction with the PtyProcess class. Creating an xterm-compatible virtual terminal emulator with inline images and mouse tracking support requires only 29 sLoC:	

```C++    	
#include <Terminal/Terminal.h>
#include <PtyProcess/PtyProcess.h>

using namespace Upp;

// This example demonstrates a simple, cross-platform (POSIX/Windows)
// terminal example.

// On Windows platform PtyProcess class uses statically linked *winpty*
// library and the supplementary PtyAgent pacakges as its *default* pty
// backend. However, it also supports the Windows 10 (tm) pseudoconsole
// API via the WIN10 compiler flag. This flag can be enabled or disable
// easily via TheIDE's main package configuration dialog. (E.g: "GUI WIN10")

#ifdef PLATFORM_POSIX
const char *tshell = "SHELL";
#elif PLATFORM_WIN32
const char *tshell = "ComSpec"; // Alternatively you can use powershell...
#endif

struct TerminalExample : TopWindow {
	TerminalCtrl term;
	PtyProcess   pty;
	
	TerminalExample()
	{
		SetRect(term.GetStdSize());	// 80 x 24 cells (scaled).
		Sizeable().Zoomable().CenterScreen().Add(term.SizePos());
		term.WhenBell   = [=]()                { BeepExclamation();  };
		term.WhenTitle  = [=](String s)        { Title(s);           };
		term.WhenOutput = [=](String s)        { pty.Write(s);       };
		term.WhenLink   = [=](const String& s) { PromptOK(DeQtf(s)); };
		term.WhenResize = [=]()                { pty.SetSize(term.GetPageSize()); };
		term.InlineImages().Hyperlinks().WindowOps();
		pty.Start(GetEnv(tshell), Environment(), GetHomeDirectory());
		SetTimeCallback(-1, [=] ()
		{
			term.WriteUtf8(pty.Get());
			if(!pty.IsRunning())
				Break();
		});
	}
};

GUI_APP_MAIN
{
	TerminalExample().Run();
}
```
Yup, that's all.
 
## [Screenshots](#screenshots)

xTerm Window Manager with inline images support (sixel/png/jpg, etc.), running on the basic terminal example. (Linux)
![](https://i.imgur.com/9MwURQb.png)

tmux, running on the basic terminal example. (Linux)
![ ](https://i.imgur.com/puT5oHC.png)

GNUPlot with sixel support, running on the basic terminal example. (Linux)
![ ](https://i.imgur.com/7P0o367.png)

Jexer with sixel support, running on the ssh terminal example. (Windows)
![ ](https://imgur.com/UzZZdPE.jpg)

The public demo of [Monotty Text-based desktop environment](https://github.com/netxs-group/VTM), running on the ssh terminal example. (Linux)
![ ](https://i.imgur.com/VgIRnCK.png)

Jexer with sixel support, running on the terminal example compiled with the TURTLE flag. (Accessed via Firefox)
![ ](https://i.imgur.com/NoDImJB.png)

Far Manager, running on the terminal example. (Windows 7, via winpty backend.)
![](https://i.imgur.com/RWqyQmI.jpg)

Terminal layout example with unicode popup, hyperlinks (Windows 7)
![](https://i.imgur.com/JjUa46b.gif)

Terminal layout example with unicode popup, hyperlinks (Linux)
![](https://i.imgur.com/e5h3vjz.gif)


## [Videos](#videos)

The four short videos below show the basic capabilities of Terminal package in general, and TerminalCtrl in particular, on various platforms. See also *Highlights*, *Features*, and *Examples* sections for more details.

### On Linux

- A basic terminal example with sixel graphics, and mouse tracking support.
- Used apps and tools: Jexer text user interface (TUI), GNUPlot, Emacs, Nano, htop, ncurses demos.
- Link: https://vimeo.com/359241367

### On Windows (as an SSH2 terminal)

- A basic SSH2 terminal example with sixel graphics, and mouse tracking support.
- Used apps and tools: Jexer text user interface.
- Link: https://vimeo.com/361556973

- A multithreaded SSH2 terminal splitter example, with sixel graphics and mouse tracking support.
- Used apps and tools: Jexer text user interface, htop, GNU nano, ncurses demos.
- Link: https://vimeo.com/362532208

### On Turtle HTML-5 backend (in a web browser)

- A basic terminal example with sixel graphics, and mouse tracking support.
- Used apps and tools: Jexer text user interface.
- Link:https://vimeo.com/361558519

##  [Acknowledgements](#acknowledgements)  

*Note that below list is incomplete and to be written...*

- vttest, and other test scripts written for xterm are extensively used in testing of the TerminalCtrl. (Thanks [Thomas E. Dickey](https://invisible-island.net/home.html)!)  
- ncurses, its demos and tests are also used in developing the Terminal package.  
- [esctest](https://gitlab.com/gnachman/iterm2/-/tree/61660349070fd4c75d1dbf333db0aabf2456c938/tests/esctest) test suite, automatic unit tests for terminal emulation, is heavily used in testing of TerminalCtrl. (Thanks [George Nachman](https://github.com/gnachman)!)  
- [Jexer](https://jexer.sourceforge.io/), a modern text user interface (TUI) and [xterm Window Manager](https://gitlab.com/klamonte/xtermwm) for terminal emulators, are heavily used as a test-bed for polishing the inline images support for TerminalCtrl. And hopefully it will continue to be a test bed for future versions of the Terminal package. (Thanks [Autumn Lamonte](https://gitlab.com/klamonte)!)  
- img2sixel of [libsixel](https://github.com/saitoha/libsixel/) is used heavily for testing the sixel images support of TerminalCtrl.  
- [winpty](https://github.com/rprichard/winpty), a Windows software package providing an interface similar to a Unix pty-master for communicating with Windows console programs, is utilized as an alternative backend for PtyProcess class on Windows platform. (Thanks [Ryan Prichard!](https://github.com/rprichard))
    

##  [License](#license)
```
Copyright (c) 2019-2021, İsmail Yılmaz
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```