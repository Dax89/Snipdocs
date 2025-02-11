#define _sd_gencc(a, b) a##b
#define _sd_genc(a, b) _sd_gencc(a, b)
#define _sd_genvar(n) _sd_genc(n, __LINE__)

#define scope(start, end)                                                      \
    for(int _sd_genvar(i) = (start, 0); !_sd_genvar(i); _sd_genvar(i)++, end)

#define defer(...)                                                             \
    for(int _sd_genvar(i) = 0; !_sd_genvar(i); _sd_genvar(i)++, __VA_ARGS__)
