# building the wex lib on msw

## msw requirements

### boost

- download
  [boost](https://sourceforge.net/projects/boost/files/boost-binaries/)
  boost_1_88-msvc-14.3-64.exe
  and put them in `c:\libraries\boost_1_88_0`

### cmake

- you should have Visual Studio 2022 installed, that already
  has cmake support

## build wex

```bash
build-gen.ps1
```

### install wex

- As Administrator run:

```bash
cmake -P cmake_install.cmake
```
