# upp-components

This repository contains supplementary general-purpose packages for [Ultimate++](https://www.ultimatepp.org/index.html),  a C++ cross-platform rapid application development framework focused on programmers productivity.

## Requirements

- Ultimate++ framework (release >= 2020.2)
- A C++ compiler that supports at least C++11. (GCC/CLANG/MinGW/MSC)

## Installation

At the moment there are two ways to install `upp-components`

1. Downlad the repository and move the package folders to your local `uppsrc` directories.

2. Clone or download the repository and set it up as an U++ *assembly* or *nest*. This is the recommended method. You can find more information on Ultimate++ packages, assemblies and nests, [here](https://www.ultimatepp.org/app$ide$PackagesAssembliesAndNests$en-us.html).


## Components

`upp-components` repository is organized into directories, retaining and reflecting the crucial sections of the `uppsrc` directory structure.


### Core

These components are meant to supplement `uppsrc/Core`

|**Package**  | **Description** |
|:---         |:---             |
| Ftp         | File transfer protocol encapsulation with advanced capabilities.                            |
| Job         | A lightweight and flexible multithreading tool, using promise/future mechanism.             |
| NetProxy    | A network proxy class implementing SOCKS and HTTP protocols.                                |
| OAuth       | A package encapsulating the authorization code grant flow of OAuth2 authorization protocol. |
| PtyProcess  | A cross-platform (POSIX/WinPty/WIN10) pseudo-terminal process implementation.               |
| PtyAgent    | Agent executable for the default backend (winpty) of PtyProcess package.                    |
| SSH         | A flexible, easy-to-use yet powerful libssh2 wrapper.                                       |

### CtrlLib
These components are meant to supplement `uppsrc/CtrlLib`

|**Package**  | **Description** |
|:---         |:---             |
| MessageCtrl |  A passive message widget and manager implementation.                                                  |
| Terminal    |  A cross-platform, high-end terminal emulator widget and library with very good xterm compatibility.   |

### Draw
These components are meant to supplement `uppsrc/Draw`

|**Package**  | **Description** |
|:---         |:---             |
| SixelRaster | A high performance, standalone sixel decoder plugin for U++'s global image decoder factory (StreamRaster interface) |

## Examples

You can find reference examples demonstrating the usage of the above listed packages in `upp-components/Examples` directory.

## Version

`upp-components` loosely follows the release cycles of Ultimate++. Releases are tagged twice a year.  Currently it is tagged as `2021.1`

## License

`upp-components` is made public with the BSD-3 Clause license.
