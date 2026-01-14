
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform: Android](https://img.shields.io/badge/Platform-Android-green.svg)](https://developer.android.com/)
[![Build: NDK](https://img.shields.io/badge/Build-NDK-orange.svg)](https://developer.android.com/ndk)

# QuickCache

QuickCache is a distributed compiler cache optimized for Android NDK development. It accelerates C and C++ build times by caching compilation results locally and across multiple machines, including team members and CI environments.

## Features

- High-performance cache with instant reuse on cache hits
- Optimized for Android NDK toolchains and multiple ABIs
- Distributed caching for teams and CI/CD pipelines
- Header dependency tracking and build statistics
- Reliable storage using SQLite metadata and zstd compression

## Installation

### Termux (Android)


pkg install git make clang openssl sqlite libcurl zstd
git clone https://github.com/ibrahimu8/quickcache.git
cd quickcache
make
sudo cp buildcache /usr/local/bin/  # Or add to PATH
Linux (Cross-compiling for Android)
export CC=aarch64-linux-android-clang
export CXX=aarch64-linux-android-clang++
make
Quick Start
Copy code
Sh
# Basic usage
./buildcache gcc -O2 myfile.c -o myapp

# Android NDK example
./buildcache aarch64-linux-android-clang++ -O3 jni/*.cpp -o libnative.so

# View cache statistics
./buildcache --stats

# Remove cache entries older than 7 days
./buildcache --clean 7
Android / NDK Usage
Copy code
Sh
./buildcache ${NDK}/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android21-clang \
  -I${NDK}/sysroot/usr/include \
  -I${NDK}/sysroot/usr/include/aarch64-linux-android \
  your_source.c
Supported ABIs
Copy code
Sh
./buildcache armv7a-linux-androideabi-clang
./buildcache aarch64-linux-android-clang
./buildcache i686-linux-android-clang
Configuration
Create ~/.quickcache/config.json:
Copy code
Json
{
  "cache_dir": "~/.quickcache/store",
  "max_size_mb": 1024,
  "remote_url": "https://your-cache-server.com",
  "compression_level": 3,
  "timeout_seconds": 30
}
Cache Statistics
Copy code
Text
$ ./buildcache --stats
BuildCache Statistics (Android/NDK)
===================================
Cache hits:     42
Cache misses:   8
Hit rate:       84.0%
Data saved:     148.2 MB
Cache age:      3.2 days
ABI distribution:
  arm64-v8a:    65%
  armeabi-v7a:  25%
  x86_64:       10%
Architecture

┌─────────────────────────────────────────┐
│           Your Build Command            │
└─────────────────┬───────────────────────┘
                  │
          ┌───────▼────────┐
          │   QuickCache   │
          │  (Interceptor) │
          └───────┬────────┘
                  │
    ┌─────────────┼─────────────┐
    │             │             │
┌───▼─────┐ ┌────▼──────┐ ┌────▼──────┐
│  Hash   │ │   Cache   │ │  Execute  │
│ Compute │ │  Lookup   │ │ Compiler  │
└─────────┘ └───────────┘ └───────────┘
    │             │             │
    └─────────────┼─────────────┘
                  │
          ┌───────▼────────┐
          │ Store in Cache │
          │  (if miss)     │
          └────────────────┘


          License
MIT License. See the LICENSE file for details.
Acknowledgments
Developed for the Android development community with optimizations for Termux and Android NDK workflows.
