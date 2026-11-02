#pragma once

#include <liboscar/platform/mouse_button.h>
#include <liboscar/platform/key_modifier.h>
#include <liboscar/utilities/hash_helpers.h>

#include <cstddef>
#include <functional>

namespace osc
{
    // Represents a combination of a single `MouseButton` with zero or more `KeyModifier`s.
    class MouseCombination final {
    public:
        constexpr MouseCombination(MouseButton mouse_button = MouseButton::None) :
            mouse_button_{mouse_button}
        {}
        constexpr MouseCombination(KeyModifiers modifiers, MouseButton mouse_button) :
            modifiers_{modifiers},
            mouse_button_{mouse_button}
        {}

        friend bool operator==(const MouseCombination&, const MouseCombination&) = default;

        constexpr MouseButton mouse_button() const { return mouse_button_; }
        constexpr KeyModifiers modifiers() const { return modifiers_; }
    private:
        KeyModifiers modifiers_{};
        MouseButton mouse_button_ = MouseButton::None;
    };

    constexpr MouseCombination operator|(KeyModifiers modifiers, MouseButton mouse_button)
    {
        return MouseCombination{modifiers, mouse_button};
    }
}

template<>
struct std::hash<osc::MouseCombination> final {
    size_t operator()(const osc::MouseCombination& mc) const noexcept
    {
        return osc::hash_of(mc.mouse_button(), mc.modifiers());
    }
};
