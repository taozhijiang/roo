#include <gmock/gmock.h>
#include <string>

#include <other/FilesystemUtil.h>


using namespace ::testing;
using namespace roo;


TEST(FilesystemUtilTest, FilePathTest) {

    std::string test_path = " /etc/profile.d///lang.sh  ";

    std::string n_path = FilesystemUtil::normalize_path(test_path);
    ASSERT_THAT(n_path, StrEq("/etc/profile.d/lang.sh"));

    ASSERT_THAT(FilesystemUtil::accessable(test_path, R_OK), true);

    ASSERT_THAT(FilesystemUtil::filename(test_path), StrEq("lang.sh"));
    ASSERT_THAT(FilesystemUtil::directory(test_path), StrEq("/etc/profile.d"));

}



TEST(FilesystemUtilTest, FileRWTest) {

    std::string message = "I am \n nicol\r\n"
                          "xxxa abs";

    std::string test_filename = "testfile.txt";
    ::unlink(test_filename.c_str());

    int code = 0;

    code = FilesystemUtil::append_file(test_filename, message);
    ASSERT_THAT(code, Eq(0));

    std::string read_content;
    code = FilesystemUtil::read_file(test_filename, read_content);
    ASSERT_THAT(code, Eq(0));
    ASSERT_THAT(read_content, StrEq(message));

    std::vector<std::string> vec {};
    code = FilesystemUtil::read_file(test_filename, vec);
    ASSERT_THAT(code, Eq(0));
    ASSERT_THAT(vec.size(), Eq(3));
    ASSERT_THAT(vec[0], Eq("I am \n"));
    ASSERT_THAT(vec[1], Eq(" nicol\r\n"));
    ASSERT_THAT(vec[2], Eq("xxxa abs"));

}