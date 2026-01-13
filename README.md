[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform: Android](https://img.shields.io/badge/Platform-Android-green.svg)](https://developer.android.com/)
[![Build: NDK](https://img.shields.io/badge/Build-NDK-orange.svg)](https://developer.android.com/ndk)

**QuickCache** is a distributed compiler cache specifically optimized for Android NDK development. Speed up your C/C++ builds for Android by caching compilation results locally and across your team.

## ✨ Features

- **🚀 Blazing Fast**: Cache hits are instant - no recompilation needed
- **🤖 Android-Optimized**: Built for NDK toolchains and Android ABIs
- **🌐 Distributed**: Share cache across team members or CI machines
- **📊 Smart Tracking**: Header dependency detection, statistics, and cache management
- **🔒 Reliable**: SQLite metadata, zstd compression, and atomic operations

## 📦 Installation

### Termux (Android)

pkg install git make clang openssl sqlite libcurl zstd
git clone https://github.com/ibrahimu8/quickcache.git
cd quickcache
make
sudo cp buildcache /usr/local/bin/  # Or add to PATH


Linux (Cross-compile for Android)


# Using Android NDK
export CC=aarch64-linux-android-clang
export CXX=aarch64-linux-android-clang++
make


🚀 Quick Start


# 1. Basic usage (replace gcc with clang for NDK)
./buildcache gcc -O2 myfile.c -o myapp

# 2. With Android NDK
./buildcache aarch64-linux-android-clang++ -O3 jni/*.cpp -o libnative.so

# 3. Check cache performance
./buildcache --stats

# 4. Clean old cache entries
./buildcache --clean 7  # Remove entries older than 7 days


🎯 Android/NDK Specific Usage


# Cache Android NDK builds
./buildcache ${NDK}/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android21-clang \
    -I${NDK}/sysroot/usr/include \
    -I${NDK}/sysroot/usr/include/aarch64-linux-android \
    your_source.c

# Different ABIs
./buildcache armv7a-linux-androideabi-clang ...  # armeabi-v7a
./buildcache aarch64-linux-android-clang ...     # arm64-v8a
./buildcache i686-linux-android-clang ...        # x86


⚙️ Configuration

Create ~/.quickcache/config.json:


{
  "cache_dir": "~/.quickcache/store",
  "max_size_mb": 1024,
  "remote_url": "https://your-cache-server.com",
  "compression_level": 3,
  "timeout_seconds": 30
}


📊 Cache Statistics


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


🏗️ Architecture


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


🤝 Contributing

Contributions welcome! This tool is especially useful for:

· Android game developers
· Cross-platform C++ projects
· Large NDK codebases
· CI/CD pipelines for Android

📄 License

MIT License - see LICENSE file.

🙏 Acknowledgments

Built specifically for the Android development community. Optimized for Termux and NDK workflows.


Star this repo if you find it useful for your Android builds! ⭐
EOF
