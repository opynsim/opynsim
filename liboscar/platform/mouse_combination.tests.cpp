#include "mouse_combination.h"

#include <gtest/gtest.h>

using namespace osc;

TEST(MouseCombination, default_constructs_to_none_button)
{
    static_assert(MouseCombination{}.mouse_button() == MouseButton::None);
}

TEST(MouseCombination, modifiers_make_it_compare_not_equal)
{
    static_assert(MouseCombination{KeyModifier::Ctrl, MouseButton::Left} != MouseCombination{MouseButton::Left});
}

TEST(MouseCombination, operator_or_can_construct_key_combinations)
{
    static_assert(MouseButton::Left == MouseCombination{MouseButton::Left});
    static_assert((KeyModifier::Ctrl | MouseButton::Right) == MouseCombination{KeyModifier::Ctrl, MouseButton::Right});
    static_assert((KeyModifier::Ctrl | KeyModifier::Alt | MouseButton::Middle) == MouseCombination{KeyModifiers{KeyModifier::Ctrl, KeyModifier::Alt}, MouseButton::Middle});
}
