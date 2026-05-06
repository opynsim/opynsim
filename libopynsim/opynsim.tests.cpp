#include "opynsim.h"

#include <libopynsim/tests/opynsim_tests_config.h>
#include <libopynsim/model_specification.h>

#include <gtest/gtest.h>

using namespace opyn;

TEST(opynsim, read_osim_throws_if_file_doesnt_exist)
{
    opyn::init();

    ASSERT_ANY_THROW({ read_osim("/this/probably/doesnt/exist"); });
}

TEST(opynsim, read_osim_works_when_given_a_file_that_does_exist)
{
    opyn::init();

    ASSERT_NO_THROW({ read_osim(opynsim_tests_resources_directory() / "models/Blank/blank.osim"); });
}

TEST(opynsim, example_specification_double_pendulum_works)
{
    opyn::init();

    ASSERT_NO_THROW({ example_specification_double_pendulum(); });
}

TEST(opynsim, compile_specification_works_on_blank_ModelSpecification)
{
    opyn::init();

    const ModelSpecification model_specification;
    ASSERT_NO_THROW({ compile_specification(model_specification); });
}

TEST(opynsim, compile_specification_works_on_more_complicated_example_OpenSim_model)
{
    opyn::init();

    const ModelSpecification model_specification = read_osim(opynsim_tests_resources_directory() / "models/RajagopalModel/Rajagopal2015.osim");
    ASSERT_NO_THROW({ compile_specification(model_specification); });
}
