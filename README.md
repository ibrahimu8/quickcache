QuickCache - Distributed Compiler Cache for Android

QuickCache
QuickCache is a distributed compiler cache designed to make building code faster, smarter, and more efficient. Think of it as a watchful companion for your compiler: it remembers the results of past compilations and reuses them whenever possible, saving you time and energy.
Local cache: Stores compiled objects on your machine. If nothing has changed, QuickCache avoids recompiling, giving you near-instant builds.
Optional remote cache: Share compiled objects across multiple machines. Ideal for teams or multiple environments.
Automatic management: Old or unused cache entries are evicted automatically to keep your storage clean.
How It Works
When you compile a source file with QuickCache, it first calculates a unique hash based on the file’s contents and compiler flags.
It checks if this hash already exists in the local cache:
If yes → HIT: uses the cached object file instead of compiling again.
If no → MISS: compiles the file normally and stores the object in the cache.
Optional remote caching can upload hits or retrieve misses from a shared server, allowing builds to be reused across machines.
Stats are tracked to show hits, misses, hit rate, and space saved, helping you understand your build efficiency.
How to Use QuickCache
Basic usage:
./buildcache <compiler> <source files> -o <output>
Examples:
# Compile a single file
./buildcache gcc ~/simple.c -o ~/test

# Compile multiple files
./buildcache gcc ~/quickcache_test/*.c -o ~/quickcache_test/test_multi

# Check cache stats
./buildcache --stats

# Clean old cache entries (e.g., older than 0 days)
./buildcache --clean 0

# Test remote cache (requires configuration)
./buildcache --test-remote

Why QuickCache Exists
QuickCache is not just a cache, it’s an efficient companion. It reduces redundant compilation, helps large projects build faster, and gives developers insight into their build performance. The design, architecture, and logic are entirely engineered solo, making it a reliable tool built for real-world use.
