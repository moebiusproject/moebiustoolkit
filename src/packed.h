#pragma once

#if defined(PACKED_STRUCTS)
#define PACKED_ATTRIBUTE __attribute__((packed))
#else
#define PACKED_ATTRIBUTE
#endif
