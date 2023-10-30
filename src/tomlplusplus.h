// Forwarding header that configures the library and includes the "ugly" path.

#include <QtGlobal> // For the assert macro.

#define TOML_ASSERT Q_ASSERT
#define TOML_EXCEPTIONS 0
#define TOML_HEADER_ONLY 0

#include "tomlplusplus/include/toml++/toml.hpp"
