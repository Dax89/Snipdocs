#include <string_view>
#include <optional>
#include <charconv>

namespace {

void detect_base(std::string_view& sv, int* res)
{
    int base = 10;

    if(sv.size() > 2 && sv[0] == '0')
    {
        switch(sv[1])
        {
            case 'x': base = 16; break;
            case 'o': base = 8; break;
            case 'b': base = 2; break;
            default: break;
        }

        sv = sv.substr(2);
    }

    if(res) *res = base;
}

} // namespace

std::optional<size_t> to_integer(std::string_view sv, int base)
{
    if(sv.empty()) return std::nullopt;

    detect_base(sv, !base ? &base : nullptr);

    size_t val{};
    auto res = std::from_chars(sv.begin(), sv.end(), val, base);
    if(res.ec != std::errc{} || res.ptr < sv.end()) return std::nullopt;
    return val;
}
