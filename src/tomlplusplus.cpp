// Translation unit for compilation of the TOML implementation. The rest of the
// files can include "tomlplusplus.h" without being "header only" now, so
// compilation is sped up a bit, specially if used more times in the future.

#define TOML_IMPLEMENTATION
#include "tomlplusplus.h"
