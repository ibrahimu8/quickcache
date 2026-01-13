void build_command_string(int argc, char **argv, char *buf, size_t len) {
    buf[0] = '\0';
    int skip_next = 0;
    for (int i = 0; i < argc; i++) {
#ifdef IGNORE_OUTPUT_PATH
        if (skip_next) {
            skip_next = 0;
            continue;
        }
        if (strcmp(argv[i], "-o") == 0) {
            skip_next = 1;
            continue;
        }
#endif
        strncat(buf, argv[i], len - strlen(buf) - 1);
        if (i < argc - 1) {
            // Check if next argument should be skipped
#ifdef IGNORE_OUTPUT_PATH
            int next_is_minus_o = (i + 1 < argc && strcmp(argv[i + 1], "-o") == 0);
            if (!next_is_minus_o) {
                strncat(buf, " ", len - strlen(buf) - 1);
            }
#else
            strncat(buf, " ", len - strlen(buf) - 1);
#endif
        }
    }
    // Trim possible trailing space
    size_t buflen = strlen(buf);
    if (buflen > 0 && buf[buflen - 1] == ' ') {
        buf[buflen - 1] = '\0';
    }
}
