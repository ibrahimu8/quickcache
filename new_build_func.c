void build_command_string(int argc, char **argv, char *buf, size_t len) {
    const quickcache_config_t *cfg = config_get();
    int ignore_output = cfg ? cfg->ignore_output_path : 0;

    buf[0] = '\0';
    int skip_next = 0;
    for (int i = 0; i < argc; i++) {
        if (ignore_output) {
            if (skip_next) {
                skip_next = 0;
                continue;
            }
            if (strcmp(argv[i], "-o") == 0) {
                skip_next = 1;
                continue;
            }
        }
        strncat(buf, argv[i], len - strlen(buf) - 1);
        if (i < argc - 1) {
            if (ignore_output) {
                int next_is_minus_o = (i + 1 < argc && strcmp(argv[i + 1], "-o") == 0);
                if (!next_is_minus_o) {
                    strncat(buf, " ", len - strlen(buf) - 1);
                }
            } else {
                strncat(buf, " ", len - strlen(buf) - 1);
            }
        }
    }
    // Trim possible trailing space
    size_t buflen = strlen(buf);
    if (buflen > 0 && buf[buflen - 1] == ' ') {
        buf[buflen - 1] = '\0';
    }
}
