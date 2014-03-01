# Build Instructions using Microsoft Visual C++

These instructions were used to build BOSS using Microsoft Visual Studio Express 2013 for Desktop, though they should also apply to other versions of MSVC.

BOSS requires the following libraries (version numbers used in latest development revision given):

* [Alphanum](http://www.davekoelle.com/files/alphanum.hpp)
* [Boost](http://www.boost.org) v1.55.0.
* [Libgit2](https://github.com/libgit2) v0.20.0.
* [UTF8-CPP](http://utfcpp.sourceforge.net/) v2.3.4.
* [wxWidgets](http://www.wxwidgets.org) v3.0.0.

BOSS expects all libraries' folders to be present in a `lib` subdirectory in the root of the repository, or otherwise installed such that the compiler and linker used can find them without suppling additional paths. All paths below are relative to the folder(s) containing the libraries and BOSS.

Alphanum and UTF8-CPP do not require any additional setup. The rest of the libraries must be built separately. Instructions for building them and BOSS itself using MSVC are given below.

#### Boost

```
bootstrap.bat
b2 toolset=msvc-12.0 threadapi=win32 link=static variant=release address-model=32 --with-program_options --with-filesystem --with-locale --with-regex --with-system --stagedir=stage-32
```

#### wxWidgets

Just build the solution provided by wxWidgets.

#### Libgit2

Libgit2 uses [CMake](http://cmake.org) to generate a MSVC solution file.

1. Set CMake up so that it builds the binaries in the `build` subdirectory of the libgit2 folder.
2. Configure CMake.
3. Undefine `BUILD_SHARED_LIBS`.
4. Generate a build system for Visual Studio 12.
5. Open the generated solution file, and build it.

You may need to make sure that the configuration properties for the Visual Studio project are set to use the multithreaded DLL runtime library (C/C++->Code Generation).

#### BOSS

Build the solution provided by BOSS.