#define CATCH_CONFIG_MAIN
#include "teenypath.h"
#include "catch/catch.hpp"

#include <algorithm>

#if defined(_WIN32)
#include <direct.h>
#include <windows.h>
#endif

using namespace std;
using namespace TeenyPath;

namespace {
#if defined(_WIN32)
    path
    current_path() {
        const int path_max = 8192;
        char path_buffer[path_max];
        if (_getcwd(path_buffer, path_max) != nullptr) {
            return path(path_buffer);
        }

        return path();
    }
#elif defined(_POSIX_VERSION)
    path
    current_path() {
        constexpr size_t path_max = 8192;
        char path_buffer[path_max];
        if (getcwd(path_buffer, path_max) != nullptr) {
            return path(path_buffer);
        }

        return path();
    }

#endif
}

TEST_CASE( "File attribute functions" ) {
    const path cwd = current_path();
    REQUIRE(cwd.is_directory());
    REQUIRE(!cwd.is_symlink());
    REQUIRE(!cwd.is_regular_file());
}

TEST_CASE( "Filename member function" ) {
    const std::string filename("foo.bar");

    SECTION( "copies have the same filename" ) {
        const path filenamePath(filename);
        REQUIRE(filename == filenamePath.filename());
        REQUIRE(filename == path(filenamePath.filename()).filename());
    }

    SECTION( "different paths can have the same filename" ) {
        const path parent("/foo/bar/foo.bar");
        REQUIRE(filename == parent.filename());
    }

    SECTION( "a path with a dot in a parent will correctly get the filename" ) {
        const path dottedParent("/foo/.bar/foo.bar");
        REQUIRE(filename == dottedParent.filename());
    }

    SECTION( "an added filename can be retrieved" ) {
        path parent("/foo/bar");
        parent /= filename;
        REQUIRE(filename == parent.filename());
    }

    SECTION( "filenames without extensions can be parsed" ) {
        const path parent("/foo/bar");
        REQUIRE("bar" == parent.filename());
    }
}

TEST_CASE( "is_absolute() member function" ) {
    SECTION( "UNC paths are absolute" ) {
        const path uncAbsPath("//foo/bar/foobar");
        REQUIRE(uncAbsPath.is_absolute());
    }

#if defined(_WIN32)
    SECTION( "paths with drive at root are absolute" ) {
        const path driveAbsPath("C:\\foo\\bar");
        REQUIRE(driveAbsPath.is_absolute());
    }
#endif

    SECTION( "normal rooted unix paths are absolute" ) {
        const path unixAbsPath("/foo/bar");
        REQUIRE(unixAbsPath.is_absolute());
    }

    SECTION( "relative paths aren't absolute" ) {
        const path relPath("../foo");
        REQUIRE(!relPath.is_absolute());

        const path otherRelPath("foo/bar");
        REQUIRE(!otherRelPath.is_absolute());
    }

    SECTION ( "empty paths aren't absolute" ) {
        const path emptyPath("");
        REQUIRE(!emptyPath.is_absolute());
    }
}

TEST_CASE( "Lexically normal" ) {
    SECTION( "paths which are lexically normal" ) {
        const path p1("/foo/bar");
        REQUIRE(p1.is_lexically_normal());

        const path p2("weird/path/with../dots/..in/file.names..");
        REQUIRE(p2.is_lexically_normal());

    }

    SECTION( "paths which aren't lexically normal" ) {
        const path p1("/foo/../bar");
        REQUIRE(!p1.is_lexically_normal());

        const path p2("/foo/./bar/");
        REQUIRE(!p2.is_lexically_normal());

        const path p3("../.././././..");
        REQUIRE(!p3.is_lexically_normal());
    }
}

TEST_CASE( "Root paths" ) {
    SECTION( "paths which are root" ) {
        {
            const path p("/");
            REQUIRE(p.is_root());
        }

        {
            const path p("/foo");
            REQUIRE(p.is_root());
        }

        {
            const path p("//");
            REQUIRE(p.is_root());
        }

        {
            const path p("//foo");
            REQUIRE(p.is_root());
        }

#if defined(_WIN32)
        {
            const path p("C:\\");
            REQUIRE(p.is_root());
        }
#endif
    }

    SECTION( "paths which aren't root" ) {
        {
            const path p("/foo/bar");
            REQUIRE(!p.is_root());
        }

        {
            const path p("relative/path");
            REQUIRE(!p.is_root());
        }
    }
}

TEST_CASE( "Join list test" ) {
    const path p1("/foo/bar/baz/");
    const path p2("/near/far/wherever/you/are/");
    const path p3("pie/is/better/than/cake");
#if defined(_WIN32)
    const char list_separator = ';';
#elif defined(_POSIX_VERSION)
    const char list_separator = ':';
#endif
    const vector<path> given {
        p1, p2, p3
    };

    const string expected = p1.string() + list_separator + p2.string() + list_separator + p3.string();
    REQUIRE(expected == joinPathList(given));
}

TEST_CASE( "Parent path member function" ) {
    const path parent("/foo");

    SECTION( "parent path is equivalent to adding '..' to path" ) {
        {
            const path child("/foo/bar");
            REQUIRE(parent == (child / ".."));
            REQUIRE(parent == child.parent_path());
        }
        
        {
            const path child("/foo/bar/baz/.././../bar/.././bar/./");
            REQUIRE(parent == child / "..");
            REQUIRE(parent == child.parent_path());
        }

        {
            const path child("/foo/bar/.");
            REQUIRE(parent == child / "..");
            REQUIRE(parent == child.parent_path());
        }
    }

    SECTION( "paths with no parent return an empty path" ) {
        const path empty_path("");
        {
            const path child("");
            REQUIRE(empty_path == child / "..");
            REQUIRE(empty_path == child.parent_path());
        }

        {
            const path child(".");
            REQUIRE(empty_path == child / "..");
            REQUIRE(empty_path == child.parent_path());
        }

        {
            const path child("..");
            REQUIRE(empty_path == child / "..");
            REQUIRE(empty_path == child.parent_path());
        }
    }

    SECTION( "root's parent is root" ) {
        {
            const path root("/");
            REQUIRE(root == root / "..");
            REQUIRE(root == root.parent_path());
        }

    #if defined(_WIN32)
        {
            const path windowsRoot("c:");
            REQUIRE(windowsRoot == windowsRoot / "..");
            REQUIRE(windowsRoot == windowsRoot.parent_path());
        }
    #endif

        {
            const path uncRoot("//");
            REQUIRE(uncRoot == uncRoot / "..");
            REQUIRE(uncRoot == uncRoot.parent_path());
        }
    }
}

TEST_CASE( "Replacing extensions" ) {
    const string dottedExtension(".bar");
    const string emptyExtension("");

    SECTION( "'normal' extension replacement" ) {
        path p("/foo.baz");
        p.replace_extension(dottedExtension);
        REQUIRE(dottedExtension == p.extension());
    }

    SECTION ( "no extension is an empty string" ) {
        path p("/foo.bar");
        p.replace_extension(emptyExtension);
        REQUIRE(emptyExtension == p.extension());
    }

    SECTION ( "replacing a non-existant extension adds it" ) {
        path p("/foo");
        p.replace_extension(dottedExtension);
        REQUIRE(dottedExtension == p.extension());
    }

    SECTION( "dots don't mess up the logic" ) {
        {
            path p("beep.boop.bleep");
            p.replace_extension(dottedExtension);
            REQUIRE(dottedExtension == p.extension());
        }

        {
            path p(".foo");
            p.replace_extension(dottedExtension);
            REQUIRE(dottedExtension == p.extension());
        }
     
        {
            path p("dotted.parent/foo");
            p.replace_extension(dottedExtension);
            REQUIRE(dottedExtension == p.extension());
        }

        {
            path p("dotted.parent/foo");
            const path q = p;
            p.replace_extension(emptyExtension);
            REQUIRE(emptyExtension == p.extension());
            REQUIRE(q == p);
        }   
    }
}

TEST_CASE( "Joining path lists" ) {
    const path foo = "/foo/bar/baz";
    const path bar = "/bar/baz/sna/fu";
    const path baz = "foo/is/ws-i";
    const vector<path> expectedSplit { foo, bar, baz };
#if defined(_WIN32)
    const char list_separator = ';';
#elif defined(_POSIX_VERSION)
    const char list_separator = ':';
#endif
    const string expectedJoined = foo.string() + list_separator +
        bar.string() + list_separator + baz.string();

    SECTION ( "join works as expected" ) {
        const string actualJoined = joinPathList(expectedSplit);
        REQUIRE(expectedJoined == actualJoined);
    }

    SECTION( "split works as expected" ) {
        const vector<path> actualSplit = splitPathList(expectedJoined);
        REQUIRE(expectedSplit == actualSplit);
    }

    SECTION( "chaning the two is a no-op" ) {
        {
            const string actualJoined = joinPathList(splitPathList(expectedJoined));
            REQUIRE(expectedJoined == actualJoined);
        }

        {
            const vector<path> actualSplit = splitPathList(joinPathList(expectedSplit));
            REQUIRE(expectedSplit == actualSplit);
        }
    }
}

TEST_CASE( "Paths to string representations" ) {
#if defined(_POSIX_VERSION)
    const string stringPath("/foo/bar/baz");
    const wstring wstringPath(L"/foo/bar/baz");
#elif defined(_WIN32)
    const string stringPath("\\foo\\bar\\baz");
    const wstring wstringPath(L"\\foo\\bar\\baz");
#endif

    {
        const path p(stringPath);
        REQUIRE(stringPath == p.string());
        REQUIRE(wstringPath == p.wstring());
    }

    {
        const path p(wstringPath);
        REQUIRE(stringPath == p.string());
        REQUIRE(wstringPath == p.wstring());
    }
}
