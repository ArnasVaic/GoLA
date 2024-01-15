#include <catch2/catch.hpp>

#include <game.hpp>

TEST_CASE("Short game of life process for a very small state.", "[short]")
{

    const auto square = Frame<4>::from_str(
        "...."
        ".##."
        ".##."
        "....");

    REQUIRE(square.has_value());

    Game<4> game;

    game.reset(square.value());

    game.evolve();

    const auto frame = game.frame();

    REQUIRE(frame == square.value());
}