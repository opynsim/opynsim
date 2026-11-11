#pragma once

#include <cstddef>
#include <string_view>

namespace osc
{
    template<size_t N>
    consteval std::string_view extract_filename(const char (&p)[N])  // NOLINT(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
    {
        for (auto i = static_cast<ptrdiff_t>(N) - 2; i >= 0; --i) {
            if (p[i] == '/' or p[i] == '\\') {
                return std::string_view{p + (i + 1), static_cast<size_t>((static_cast<ptrdiff_t>(N) - 2) - i)};  // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
            }
        }
        return {p, N - 1};  // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
    }
}
