# QuickCache - Distributed Compiler Cache

QuickCache is a distributed compiler cache designed to speed up repeated builds by caching compiled objects locally and remotely.  

Think of it like `ccache`, but with remote caching capabilities.

---

## Features

- Local caching of compiled objects
- Optional remote cache support
- Automatic cache eviction based on size/age
- Asynchronous uploads for minimal build latency
- Works with GCC and other compilers
- Automated testing & benchmarking scripts included

---

## Installation

### Dependencies

QuickCache requires:

- GCC or Clang
- `make`
- `curl` development libraries
- `pthread` support
- Linux environment

On Debian/Ubuntu:

```bash
sudo apt update
sudo apt install build-essential libcurl4-openssl-dev

Build from Source

git clone git@github.com:ibrahimu8/quickcache.git
cd quickcache
make

Optional: Run the installer script to set up helper scripts:

chmod +x install_quickcache.sh
./install_quickcache.sh


---

Usage

QuickCache works as a wrapper around your compiler:

./buildcache <compiler> <source files> -o <output>

Examples

Compile a single file:

./buildcache gcc test.c -o test

Check cache stats:

./buildcache --stats

Clean cache entries:

./buildcache --clean [days]

Set a maximum cache size:

./buildcache --limit 100   # limit to 100 MB

Test remote cache:

./buildcache --test-remote


---

Automated Testing

QuickCache comes with scripts to test and benchmark the cache:

chmod +x test_quickcache.sh
./test_quickcache.sh

You will see:

Cache MISS/HIT behavior

Upload to remote cache

Local hits for repeated builds

Automatic cleanup of old cache entries


Benchmark script outputs timing for each build step.


---

Notes

The network.c fix ensures usleep works across all Linux distributions.

Remote caching is optional. If disabled, QuickCache functions as a local cache only.

SSH setup is recommended for pushing/pulling remote cache configurations or contributing updates to this repo.



---

Contributing

Pull requests and issues are welcome!
If you fork QuickCache, please make sure to test with test_quickcache.sh and ensure local & remote caching works as expected.

