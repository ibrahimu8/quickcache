
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform: Android](https://img.shields.io/badge/Platform-Android-green.svg)](https://developer.android.com/)
[![Build: NDK](https://img.shields.io/badge/Build-NDK-orange.svg)](https://developer.android.com/ndk)

```markdown
# QuickCache

A high-performance distributed compiler cache for C/C++ projects, optimized for Android NDK development. QuickCache accelerates build times by caching compilation results locally and optionally sharing them across team members and CI/CD environments.

 Features

- Instant cache hits with zero compilation time on repeated builds
- Distributed caching to share compiled artifacts across teams and CI/CD pipelines
- First-class support for Android NDK cross-compilation and multiple ABIs
- Asynchronous remote uploads that don't block your build
- Build statistics to track cache performance and storage savings
- zstd compression for efficient storage
- Automatic header dependency tracking for correct cache invalidation

 How It Works

QuickCache wraps your compiler (gcc, clang, or NDK toolchains) and intercepts compilation commands. When you compile a file:

1. QuickCache computes a hash based on the source file, compiler flags, and all included headers
2. It checks if this exact compilation has been cached before
3. On a cache hit, it instantly retrieves the compiled object file
4. On a cache miss, it runs the actual compiler and stores the result for future use

This means the second time you compile the same code with the same flags, you get instant results.

 Requirements

- Linux or Termux (Android)
- GCC or Clang compiler
- OpenSSL
- SQLite3
- libcurl
- zstd

 Installation

 On Termux (Android)

```bash
pkg install git make clang openssl sqlite libcurl zstd
git clone https://github.com/ibrahimu8/quickcache.git
cd quickcache
make
```

 On Linux

```bash
# Install dependencies (Debian/Ubuntu)
sudo apt-get install build-essential libssl-dev libsqlite3-dev libcurl4-openssl-dev libzstd-dev

# Or on Fedora/RHEL
sudo dnf install gcc make openssl-devel sqlite-devel libcurl-devel libzstd-devel

# Build
git clone https://github.com/ibrahimu8/quickcache.git
cd quickcache
make

# Optional: Install system-wide
sudo make install
```

 Quick Start

 Basic Usage

Replace your compiler command with `./buildcache` followed by your normal compilation:

```bash
 Instead of:
gcc -O2 myfile.c -o myapp

 Use:
./buildcache gcc -O2 myfile.c -o myapp
```

 Android NDK Example

```bash
./buildcache aarch64-linux-android21-clang++ \
    -I${NDK}/sysroot/usr/include \
    -O3 jni/native.cpp -o libnative.so
```

 Viewing Statistics

```bash
./buildcache --stats
```

Example output:
```
BuildCache Statistics
=====================
Cache hits:     42
Cache misses:   8
Hit rate:       84.0%
Data saved:     148.2 MB
Cache age:      3.2 days
```

 Cache Management

```bash
 Remove entries older than 7 days
./buildcache --clean 7

 Remove all cache entries
./buildcache --clean

 Enforce size limit (in MB)
./buildcache --limit 1024
```

 Configuration

QuickCache can be configured by creating `~/.quickcache/config.json`:

```json
{
  "cache_dir": "~/.quickcache/store",
  "max_size_mb": 1024,
  "remote_url": "https://your-cache-server.com",
  "auth_token": "your-secret-token",
  "compression_level": 3,
  "timeout_seconds": 30,
  "async_upload": true,
  "ignore_output_path": false
}
```

 Configuration Options

- `cache_dir` - Where to store cached objects (default: `~/.quickcache/store`)
- `max_size_mb` - Maximum cache size in megabytes (default: 1024)
- `remote_url` - URL of your remote cache server (optional)
- `auth_token` - Authentication token for remote cache (optional)
- `compression_level` - zstd compression level 1-22 (default: 3)
- `timeout_seconds` - Network timeout for remote operations (default: 30)
- `async_upload` - Upload to remote cache in background (default: true)
- `ignore_output_path` - Exclude output path from cache key (default: false)

To generate an example config file:

```bash
./buildcache --config
```

 Remote Cache Setup

QuickCache supports distributed caching across multiple machines. Set up a remote cache server and configure the URL in your config file. The cache will automatically:

- Check the remote cache before compiling
- Upload successful compilations in the background
- Fall back gracefully if the remote cache is unavailable

Test your remote connection:

```bash
./buildcache --test-remote
```

 Android NDK Integration

QuickCache works seamlessly with Android NDK toolchains. It supports all common ABIs:

```bash
 ARM 64-bit
./buildcache aarch64-linux-android-clang your_source.c

 ARM 32-bit
./buildcache armv7a-linux-androideabi-clang your_source.c

 x86 64-bit
./buildcache x86_64-linux-android-clang your_source.c

 x86 32-bit
./buildcache i686-linux-android-clang your_source.c
```

 Integration with Build Systems

 With Make

```makefile
CC = ./buildcache gcc
CXX = ./buildcache g++
```

 With CMake

```bash
cmake -DCMAKE_C_COMPILER_LAUNCHER=./buildcache \
      -DCMAKE_CXX_COMPILER_LAUNCHER=./buildcache \
      ..
```

 With Gradle (Android)

Add to your `build.gradle`:

```groovy
android {
    // ...
    externalNativeBuild {
        cmake {
            arguments "-DCMAKE_C_COMPILER_LAUNCHER=/path/to/buildcache",
                      "-DCMAKE_CXX_COMPILER_LAUNCHER=/path/to/buildcache"
        }
    }
}
```

 How Cache Keys Work

QuickCache generates cache keys by hashing:

1. The source file content
2. All compiler flags (excluding output path if `ignore_output_path` is true)
3. The content of all included headers (recursively)

This means:
- Changing a source file invalidates its cache
- Changing any header it includes invalidates the cache
- Changing compiler flags creates a new cache entry
- The output filename doesn't matter (unless configured otherwise)

 Performance Tips

1. Enable `async_upload` to avoid blocking on remote uploads
2. Set `ignore_output_path: true` if you frequently change output filenames
3. Use compression level 3-6 for balance between speed and space
4. Clean old cache entries periodically with `--clean 30`
5. For CI/CD, use a shared remote cache to avoid cold starts

 Known Limitations

- Only supports compilation (not linking)
- Header tracking follows `#include` directives but not generated headers
- Remote cache requires a compatible server implementation
- Not suitable for preprocessor-heavy code that changes frequently

 Troubleshooting

 Cache not hitting when it should

Check if headers are being modified by build scripts:
```bash
 Compare hashes
./buildcache gcc -E source.c | sha256sum
```

 Remote cache not connecting

Test the connection:
```bash
./buildcache --test-remote
```

Check your config file has correct `remote_url` and `auth_token`.

 Running out of disk space

Set a cache limit:
```bash
./buildcache --limit 512  # 512 MB
```

Or clean old entries:
```bash
./buildcache --clean 7  # Remove entries older than 7 days
```

 Architecture

```
┌─────────────────────────────────────────┐
│         Your Build Command              │
└─────────────────┬───────────────────────┘
                  │
         ┌────────▼────────┐
         │   QuickCache    │
         │  (Interceptor)  │
         └────────┬────────┘
                  │
    ┌─────────────┼─────────────┐
    │             │             │
┌───▼─────┐  ┌───▼──────┐  ┌──▼───────┐
│  Hash   │  │  Cache   │  │ Execute  │
│ Compute │  │  Lookup  │  │ Compiler │
└─────────┘  └──────────┘  └──────────┘
    │             │             │
    └─────────────┼─────────────┘
                  │
         ┌────────▼────────┐
         │  Store Result   │
         │ (Local/Remote)  │
         └─────────────────┘
```

 Contributing

Contributions are welcome. Please:

1. Test your changes with both local and remote caching
2. Ensure the build passes without warnings
3. Add tests for new features
4. Update documentation as needed

 License

MIT License. See LICENSE file for details.

 Acknowledgments

Developed for the Android development community with a focus on Termux and Android NDK workflows. Inspired by ccache and sccache but optimized for mobile and cross-compilation scenarios.

 Support

For issues, questions, or feature requests, please open an issue on GitHub.
```
