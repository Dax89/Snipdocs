#pragma once

#define _xyz_cc(a, b) a##b
#define _xyz_c(a, b) _xyz_cc(a, b)
#define _xyz_var(n) _xyz_c(n, __LINE__)

#define scope(start, end)                                                      \
    for(int _xyz_var(i) = (start, 0); !_xyz_var(i); _xyz_var(i)++, end)

#define defer(...)                                                             \
    for(int _xyz_var(i) = 0; !_xyz_var(i); _xyz_var(i)++, __VA_ARGS__)
