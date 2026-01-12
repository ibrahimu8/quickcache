QuickCache

A lightweight, distributed compiler cache written in 1,567 lines of C.

Built entirely on a phone using Termux.

 Why QuickCache?

- 9x faster rebuilds (187ms to 20ms)
- Distributed caching - share builds across your team
- Tiny footprint - 116KB binary, 20x smaller than alternatives
- Minimal dependencies - just standard libraries (OpenSSL, zstd, SQLite, curl)
- Production-ready - proper error handling, compression, statistics

 Performance


# First compilation
$ time quickcache gcc -c main.c -o main.o
real    0m0.187s

# Second compilation (cache hit)
$ time quickcache gcc -c main.c -o main.o
real    0m0.020s

