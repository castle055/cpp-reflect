<h1 align="center"> cpp-reflect</h1>
<h4 align="center">C++ Reflection and Annotations Library</h4>

<p align="center">
<img alt="Language" src="https://img.shields.io/static/v1?style=for-the-badge&message=C%2B%2B&color=00599C&logo=C%2B%2B&logoColor=FFFFFF&label=">
<img alt="Tool" src="https://img.shields.io/static/v1?style=for-the-badge&message=Clang&color=064F8C&logo=C%2B%2B&logoColor=FFFFFF&label=">
<img alt="Tool" src="https://img.shields.io/static/v1?style=for-the-badge&message=CMake&color=064F8C&logo=CMake&logoColor=FFFFFF&label=">
<img alt="GitHub" src="https://img.shields.io/github/license/castle055/cpp-reflect?style=for-the-badge">
<img alt="GitHub tag (latest SemVer)" src="https://img.shields.io/github/v/tag/castle055/cpp-reflect?color=%23fcae1e&label=latest&sort=semver&style=for-the-badge">
</p>

<p align="center">
  <a href="#overview">Overview</a> •
  <a href="#requirements">Requirements</a> •
  <a href="#integration">Integration</a> •
  <a href="#usage">Usage</a> •
  <a href="#license">License</a>
</p>

[TOC]

# Overview

{TODO}

# Requirements

{TODO}

# Integration

## CMake project (recommended)

Since this is a module library, the recommended way to integrate it into your project is with CMake. This is easy to do with `FetchContent`:

```cmake
# CmakeLists.txt

include(FetchContent)

FetchContent_Declare(cpp_reflect
    GIT_REPOSITORY https://github.com/castle055/cpp-reflect.git
    GIT_TAG main # for the latest version, or a version tag such as 'v0.2.0'
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(cpp_reflect)
include_directories(${cpp_reflect_SOURCE_DIR}/include)
```

# Usage

{TODO}

# License

GPL 3.0 &nbsp;&middot;&nbsp; [LICENSE.MD](https://castle055.github.io/cpp-reflect/license-1.html)

---

> GitHub [@castle055](https://github.com/castle055) &nbsp;&middot;&nbsp;

