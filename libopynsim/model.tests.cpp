#include "model.h"

#include <libopynsim/tests/opynsim_tests_config.h>
#include <libopynsim/data_frame.h>
#include <libopynsim/model.h>
#include <libopynsim/model_states.h>
#include <libopynsim/opynsim.h>

#include <gtest/gtest.h>

#include <ranges>
#include <vector>

using namespace opyn;

TEST(Model, rotational_columns_in_returns_empty_if_given_empty_DataFrame)
{
    opyn::init();

    const Model model = read_osim(opynsim_tests_resources_directory() / "pendulum/pendulum.osim").compile();
    const DataFrame data_frame;

    ASSERT_TRUE(model.rotational_columns_in(data_frame).empty());
}

TEST(Model, rotational_columns_in_returns_rotational_columns_for_pendulum_input)
{
    opyn::init();

    const Model model = read_osim(opynsim_tests_resources_directory() / "pendulum/pendulum.osim").compile();
    const DataFrame data_frame = read_sto(opynsim_tests_resources_directory() / "pendulum/pendulum_trajectory.sto");
    const std::vector<std::string> got = model.rotational_columns_in(data_frame);
    const std::vector<std::string> expected = {"/jointset/pin/pin_coord_0/value", "/jointset/pin/pin_coord_0/speed"};

    ASSERT_EQ(got, expected);
}

TEST(Model, column_to_state_variable_mappings_returns_empty_map_when_given_empty_data_frame)
{
    opyn::init();

    const Model model = read_osim(opynsim_tests_resources_directory() / "pendulum/pendulum.osim").compile();
    const DataFrame data_frame;

    ASSERT_TRUE(model.column_to_state_variable_mappings(data_frame).empty());
}

TEST(Model, column_to_state_variable_mapping_returns_expected_result_for_basic_pendulum)
{
    opyn::init();

    const Model model = read_osim(opynsim_tests_resources_directory() / "pendulum/pendulum.osim").compile();
    const DataFrame data_frame = read_sto(opynsim_tests_resources_directory() / "pendulum/pendulum_trajectory.sto");
    const std::unordered_map<std::string, Symbol> got = model.column_to_state_variable_mappings(data_frame);
    const std::unordered_map<std::string, Symbol> expected = {
        {"/jointset/pin/pin_coord_0/value", Symbol{"/jointset/pin/pin_coord_0/value"}},
        {"/jointset/pin/pin_coord_0/speed", Symbol{"/jointset/pin/pin_coord_0/speed"}},
    };

    ASSERT_EQ(got, expected);
}

TEST(Model, states_from_data_frame_works_for_basic_pendulum)
{
    opyn::init();

    const Model model = read_osim(opynsim_tests_resources_directory() / "pendulum/pendulum.osim").compile();
    const DataFrame data_frame = read_sto(opynsim_tests_resources_directory() / "pendulum/pendulum_trajectory.sto");
    const ModelStates model_states = model.states_from_data_frame(data_frame);

    ASSERT_EQ(model_states.size(), data_frame.height());
}
