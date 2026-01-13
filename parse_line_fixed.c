static void parse_line(char *line) {
    // Strip newline
    char *newline = strchr(line, '\n');
    if (newline) *newline = '\0';

    // Skip empty lines and comments
    if (line[0] == '\0' || line[0] == '#') return;

    // Split on '='
    char *eq = strchr(line, '=');
    if (!eq) return;

    *eq = '\0';
    char *key = line;
    char *value = eq + 1;

    // Trim whitespace
    while (*key == ' ' || *key == '\t') key++;
    while (*value == ' ' || *value == '\t') value++;

    if (strcmp(key, "remote_url") == 0) {
        strncpy(global_config.remote_url, value, sizeof(global_config.remote_url) - 1);
        global_config.remote_enabled = 1;
    } else if (strcmp(key, "auth_token") == 0) {
        strncpy(global_config.auth_token, value, sizeof(global_config.auth_token) - 1);
    } else if (strcmp(key, "timeout") == 0) {
        global_config.timeout_seconds = atoi(value);
    } else if (strcmp(key, "async_upload") == 0) {
        global_config.async_upload = (strcmp(value, "true") == 0 || strcmp(value, "1") == 0);
    } else if (strcmp(key, "ignore_output_path") == 0) {
        global_config.ignore_output_path = (strcmp(value, "true") == 0 || strcmp(value, "1") == 0);
    }
}
