
# QuickCache

**QuickCache** is a **lightweight, distributed compiler cache** designed to accelerate rebuilds, reduce redundant compilation, and support remote caching. It’s written in pure C (~1,567 lines) and is optimized for speed, simplicity, and minimal dependencies.

---

## Why QuickCache Matters

- **Faster rebuilds**: Avoid recompiling unchanged files. Local cache hits can reduce compile times from hundreds of milliseconds to just a few.
- **Distributed caching**: Share compiled objects across your team or CI/CD servers.
- **Small footprint**: Tiny binary (~116KB) and minimal runtime dependencies.
- **Offline-capable**: Works without network access, while optionally supporting remote caches.
- **Flexible and configurable**: Supports async uploads, configurable timeouts, and selective output ignoring.
- **Safe defaults**: Minimal setup required; fully usable with just a default config.

---

## Features

- **Local caching**: Reuses previously compiled objects.
- **Remote caching**: Upload and fetch from a remote cache server.
- **Async upload**: Optionally upload compiled objects in the background.
- **Configurable**: Supports `remote_url`, `auth_token`, `timeout`, `async_upload`, and `ignore_output_path`.
- **Minimal dependencies**: Only requires standard C libraries (OpenSSL, zstd, SQLite, curl optional).

---

## Installation

1. Clone the repository:

```bash
git clone https://github.com/ibrahimu8/quickcache.git
cd quickcache

2. Build QuickCache:



make


---

Configuration

QuickCache uses a configuration file at:

~/.quickcache/config

You can create an example config with:

./buildcache config_create_example

Example configuration:

# BuildCache Configuration

# Enable remote caching by setting remote_url
# remote_url=http://quickcache-server:8080

# Auth token for remote cache
# auth_token=your-secret-token-here

# Timeout in seconds for remote cache operations
# timeout=10

# Enable asynchronous uploads
async_upload=true

# Ignore output path when caching
ignore_output_path=true

ignore_output_path=true ensures that files with different output names but same content are cached consistently. This prevents human errors during builds.



---

Usage

Compile a source file using QuickCache:

./buildcache clang myfile.c -o myprog -lm

QuickCache will check local and remote caches automatically.

If an object is found in cache, compilation is skipped.

If missing, QuickCache compiles the file, stores it in cache, and optionally uploads it remotely.


Examples:

# Normal compile
./buildcache clang real_test.c -o myprog -lm

# Compile with debug flags
./buildcache clang -O0 real_test.c -o test_debug -lm

# Compile with different defines
./buildcache clang -DTEST=1 real_test.c -o testD1 -lm


---

How it Works

1. Generates a hash of the source file and compile flags.


2. Checks local cache for a matching object.


3. Checks remote cache if configured.


4. Compiles using the system compiler if cache miss.


5. Stores the compiled object in cache and uploads remotely if enabled.




---

Important Notes

Always use -lm or other required libraries in your QuickCache compile commands.

ignore_output_path=true is critical to avoid cache misses due to different output filenames.

QuickCache is compatible with any compiler that supports execvp, including clang and gcc.

Remote cache usage is optional; QuickCache works fully offline.



---

Contribution

Fix bugs, add new compiler support, or improve caching logic.

Pull requests should maintain minimal dependencies and small binary size.



---

License

MIT License – see LICENSE file for details.


---

Summary

QuickCache saves time, reduces redundant builds, and improves team productivity by caching compiled objects both locally and remotely. Proper configuration ensures reproducibility, correctness, and fewer human errors during builds.

