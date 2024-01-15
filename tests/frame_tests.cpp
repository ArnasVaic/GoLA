#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <game.hpp>

using namespace std;



TEST_CASE("Correct frame created from string", "[short]") {
    const auto frame = Frame<3>::from_str(
        ".#."
        "#.."
        "..#"
    );

    REQUIRE(frame.has_value());
    REQUIRE(!frame->get(0, 0));
    REQUIRE(frame->get(0, 1));
    REQUIRE(!frame->get(0, 2));

    REQUIRE(frame->get(1, 0));
    REQUIRE(!frame->get(1, 1));
    REQUIRE(!frame->get(1, 2));

    REQUIRE(!frame->get(2, 0));
    REQUIRE(!frame->get(2, 1));
    REQUIRE(frame->get(2, 2));
}