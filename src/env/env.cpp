#include "./env.h"

const char *CACHE_DIR = "/.cache/ddupes";

char *fetchHomeDirectory() { return getenv("HOME"); }
