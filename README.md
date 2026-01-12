QuickCache is a lightweight, high-performance, distributed compiler cache written in just 1,567 lines of C. It is designed to dramatically speed up your builds, reduce resource usage, and make collaboration on large codebases effortless.

Why QuickCache?

QuickCache was created with simplicity, speed, and practicality in mind. Here’s why it stands out:

· Blazing-fast rebuilds: Stop wasting time waiting for unchanged files to recompile. QuickCache can reduce rebuild times by up to 9x (for example, from 187ms to 20ms).
· Distributed caching: Share build artifacts across your team seamlessly. Everyone benefits from cached compilations, reducing redundant work.
· Tiny footprint: The compiled binary is only 116KB, making it over 20 times smaller than most alternatives, so it won’t bloat your system.
· Minimal dependencies: QuickCache relies only on standard libraries like OpenSSL, zstd, SQLite, and curl, keeping your setup lightweight and portable.
· Production-ready: Comes with proper error handling, compression support, detailed statistics, and reliability for real-world projects.

Performance Example

QuickCache is designed to be fast from the very first compilation.
