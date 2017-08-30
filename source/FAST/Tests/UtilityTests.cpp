#include "FAST/Testing.hpp"
#include "FAST/Utility.hpp"

using namespace fast;

TEST_CASE("Split", "[split][utility]") {
    // Default split delimiter is whitespace
    std::vector<std::string> parts = split("asd asd2 asd*  asd!");
    REQUIRE(parts.size() == 4);
    CHECK(parts[0] == "asd");
    CHECK(parts[1] == "asd2");
    CHECK(parts[2] == "asd*");
    CHECK(parts[3] == "asd!");

    parts = split("Asd = wee*", "=");
    REQUIRE(parts.size() == 2);
    CHECK(parts[0] == "Asd ");
    CHECK(parts[1] == " wee*");

    parts = split("Asd = wee*", " = ");
    REQUIRE(parts.size() == 2);
    CHECK(parts[0] == "Asd");
    CHECK(parts[1] == "wee*");

    parts = split("Asd = wee* = hmf = asd", " = ");
    REQUIRE(parts.size() == 4);
    CHECK(parts[0] == "Asd");
    CHECK(parts[1] == "wee*");
    CHECK(parts[2] == "hmf");
    CHECK(parts[3] == "asd");

    parts = split("\"Wee sad\" \"asd asd \" \"asdasd\"", "\" \"");
    REQUIRE(parts.size() == 3);
    CHECK(parts[0] == "\"Wee sad");
    CHECK(parts[1] == "asd asd ");
    CHECK(parts[2] == "asdasd\"");
}

TEST_CASE("Replace", "[replace][utility]") {
    std::string str = "asdasd asdasd* 23*";
    CHECK(replace(str, " ", "#") == "asdasd#asdasd*#23*");

    str = "\"asd \" asdd \"\"";
    CHECK(replace(str, "\"", "") == "asd  asdd ");

    str = "hmasdasdd\"";
    CHECK(replace(str, "*", "#") == str);

    str = "Hello world!";
    CHECK(replace(str, "world", "fantasy") == "Hello fantasy!");
}