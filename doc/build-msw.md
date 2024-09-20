# building the wex lib on msw

## msw requirements

### boost

- download
  [boost](https://boostorg.jfrog.io/ui/native/main/release/1.80.0/binaries)
  boost_1_80-msvc-14.3-64.exe
  and put them e.g. in `c:\libraries\boost_1_80_0`

### cmake

- you should have Visual Studio 2022 installed, that already
  has cmake support

## build wex

- see build-gen.ps1

### install wex

- As Administrator run:

```bash
cmake -P cmake_install.cmake
```
