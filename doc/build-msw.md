# building the wex lib and the syncped application on msw

## msw requirements

### boost

- download
  [boost](https://boostorg.jfrog.io/ui/native/main/release/1.80.0/binaries)
  boost_1_80-msvc-14.3-64.exe
  and put them e.g. in `c:\local\boost_1_80_0`

### cmake

- you should have Visual Studio 2022 installed, that already
  has cmake support

## build wex

- the easiest way is using the command-line tool devenv

```bash
cmake -DBOOST_ROOT=c:\local\boost_1_80_0 -DCMAKE_BUILD_TYPE=Release ..
devenv wex.sln /build Release
```

- do a new cmake (to find the built libraries from previous step)

```bash
cmake ..
```

### install wex

- As Administrator run:

```bash
cmake -P cmake_install.cmake
```

### check Findwex location

- Findwex as used by building sp, and should be installed in the actual
  cmake share Modules as used by Visual Studio

## build syncped

```bash
cmake -DBOOST_ROOT=c:\local\boost_1_80_0 -DCMAKE_BUILD_TYPE=Release
  -DCMAKE_INSTALL_PREFIX="c:\program files (x86)\wex" ..
```
