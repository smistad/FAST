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

    parts = split("test", "/");
    REQUIRE(parts.size() == 1);
    CHECK(parts[0] == "test");
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

TEST_CASE("getFileName", "[getFileName][utility][fast]") {
    std::string str = "C:/asd/asdasda asd/asd.txt";
    CHECK(getFileName(str) == "asd.txt");
    str = "D:\\asd\\dasd asd\\asd asd.txt";
    CHECK(getFileName(str) == "asd asd.txt");
    str = "/home/asd/asd.txt";
    CHECK(getFileName(str) == "asd.txt");
    str = "/home/asd/asd asd";
    CHECK(getFileName(str) == "asd asd");
    str = "asd/klhasd/asd_asd.txt";
    CHECK(getFileName(str) == "asd_asd.txt");
    str = "asd\\klhasd\\asd_asd.txt";
    CHECK(getFileName(str) == "asd_asd.txt");
    str = "asd asd.txt";
    CHECK(getFileName(str) == "asd asd.txt");
}

TEST_CASE("createDirectories", "[createDirectories][utility][fast]") {
    std::string name = "create_directories_test";
    CHECK_NOTHROW(createDirectories(name));
    CHECK(isDir(name));

    std::string name2 = "create_directories_test2/test2/";
    CHECK_NOTHROW(createDirectories(name2));
    CHECK(isDir(name2));

    std::string name3 = "create_directories_test3/test3/test3/";
    CHECK_NOTHROW(createDirectories(name3));
    CHECK(isDir(name3));

#ifdef WIN32
    std::string name4 = "create_directories_test4\\test4\\";
    CHECK_NOTHROW(createDirectories(name4));
    CHECK(isDir(name4));
#endif
}