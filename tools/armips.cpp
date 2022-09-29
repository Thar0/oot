// armips assembler v0.11
// https://github.com/Kingcom/armips/
// To simplify compilation, all files have been concatenated into one.
// MIPS only, ARM is not included.

/*
The MIT License (MIT)

Copyright (c) 2009-2020 Kingcom

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

// file: Util/FileSystem.h

#define GHC_FILESYSTEM_FWD
#define GHC_FILESYSTEM_IMPLEMENTATION
//---------------------------------------------------------------------------------------
//
// ghc::filesystem - A C++17-like filesystem implementation for C++11/C++14/C++17
//
//---------------------------------------------------------------------------------------
//
// Copyright (c) 2018, Steffen Schümann <s.schuemann@pobox.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//---------------------------------------------------------------------------------------
//
// To dynamically select std::filesystem where available, you could use:
//
// #if defined(__cplusplus) && __cplusplus >= 201703L && defined(__has_include) && __has_include(<filesystem>)
// #include <filesystem>
// namespace fs = std::filesystem;
// #else
// #include <ghc/filesystem.hpp>
// namespace fs = ghc::filesystem;
// #endif
//
//---------------------------------------------------------------------------------------
#ifndef GHC_FILESYSTEM_H
#define GHC_FILESYSTEM_H

// #define BSD manifest constant only in
// sys/param.h
#ifndef _WIN32
#include <sys/param.h>
#endif

#ifndef GHC_OS_DETECTED
#if defined(__APPLE__) && defined(__MACH__)
#define GHC_OS_MACOS
#elif defined(__linux__)
#define GHC_OS_LINUX
#if defined(__ANDROID__)
#define GHC_OS_ANDROID
#endif
#elif defined(_WIN64)
#define GHC_OS_WINDOWS
#define GHC_OS_WIN64
#elif defined(_WIN32)
#define GHC_OS_WINDOWS
#define GHC_OS_WIN32
#elif defined(__svr4__)
#define GHC_OS_SYS5R4
#elif defined(BSD)
#define GHC_OS_BSD
#else
#error "Operating system currently not supported!"
#endif
#define GHC_OS_DETECTED
#endif

#if defined(GHC_FILESYSTEM_IMPLEMENTATION)
#define GHC_EXPAND_IMPL
#define GHC_INLINE
#ifdef GHC_OS_WINDOWS
#define GHC_FS_API
#define GHC_FS_API_CLASS
#else
#define GHC_FS_API __attribute__((visibility("default")))
#define GHC_FS_API_CLASS __attribute__((visibility("default")))
#endif
#elif defined(GHC_FILESYSTEM_FWD)
#define GHC_INLINE
#ifdef GHC_OS_WINDOWS
#define GHC_FS_API extern
#define GHC_FS_API_CLASS
#else
#define GHC_FS_API extern
#define GHC_FS_API_CLASS
#endif
#else
#define GHC_EXPAND_IMPL
#define GHC_INLINE inline
#define GHC_FS_API
#define GHC_FS_API_CLASS
#endif

#ifdef GHC_EXPAND_IMPL

#ifdef GHC_OS_WINDOWS
#include <windows.h>
// additional includes
#include <shellapi.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <wchar.h>
#include <winioctl.h>
#else
#include <dirent.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#ifdef GHC_OS_ANDROID
#include <android/api-level.h>
#if __ANDROID_API__ < 12
#include <sys/syscall.h>
#endif
#include <sys/vfs.h>
#define statvfs statfs
#else
#include <sys/statvfs.h>
#endif
#if !defined(__ANDROID__) || __ANDROID_API__ >= 26
#include <langinfo.h>
#endif
#endif
#ifdef GHC_OS_MACOS
#include <Availability.h>
#endif

#include <algorithm>
#include <cctype>
#include <chrono>
#include <clocale>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <memory>
#include <stack>
#include <stdexcept>
#include <string>
#include <system_error>
#include <type_traits>
#include <utility>
#include <vector>

#else  // GHC_EXPAND_IMPL
#include <chrono>
#include <fstream>
#include <memory>
#include <stack>
#include <stdexcept>
#include <string>
#include <system_error>
#ifdef GHC_OS_WINDOWS
#include <vector>
#endif
#endif  // GHC_EXPAND_IMPL

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Behaviour Switches (see README.md, should match the config in test/filesystem_test.cpp):
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// LWG #2682 disables the since then invalid use of the copy option create_symlinks on directories
// configure LWG conformance ()
#define LWG_2682_BEHAVIOUR
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// LWG #2395 makes crate_directory/create_directories not emit an error if there is a regular
// file with that name, it is superceded by P1164R1, so only activate if really needed
// #define LWG_2935_BEHAVIOUR
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// LWG #2937 enforces that fs::equivalent emits an error, if !fs::exists(p1)||!exists(p2)
#define LWG_2937_BEHAVIOUR
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// UTF8-Everywhere is the original behaviour of ghc::filesystem. With this define you can
// enable the more standard conforming implementation option that uses wstring on Windows
// as ghc::filesystem::string_type.
// #define GHC_WIN_WSTRING_STRING_TYPE
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Raise errors/exceptions when invalid unicode codepoints or UTF-8 sequences are found,
// instead of replacing them with the unicode replacement character (U+FFFD).
// #define GHC_RAISE_UNICODE_ERRORS
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// ghc::filesystem version in decimal (major * 10000 + minor * 100 + patch)
#define GHC_FILESYSTEM_VERSION 10303L

#if !defined(GHC_WITH_EXCEPTIONS) && (defined(__EXCEPTIONS) || defined(__cpp_exceptions) || defined(_CPPUNWIND))
#define GHC_WITH_EXCEPTIONS
#endif
#if !defined(GHC_WITH_EXCEPTIONS) && defined(GHC_RAISE_UNICODE_ERRORS)
#error "Can't raise unicode errors whith exception support disabled"
#endif

namespace ghc {
namespace filesystem {

// temporary existing exception type for yet unimplemented parts
class GHC_FS_API_CLASS not_implemented_exception : public std::logic_error
{
public:
    not_implemented_exception()
        : std::logic_error("function not implemented yet.")
    {
    }
};

template<typename char_type>
class path_helper_base
{
public:
    using value_type = char_type;
#ifdef GHC_OS_WINDOWS
    static constexpr value_type preferred_separator = '\\';
#else
    static constexpr value_type preferred_separator = '/';
#endif
};

#if  __cplusplus < 201703L
template <typename char_type>
constexpr char_type path_helper_base<char_type>::preferred_separator;
#endif
    
// 30.10.8 class path
class GHC_FS_API_CLASS path
#if defined(GHC_OS_WINDOWS) && defined(GHC_WIN_WSTRING_STRING_TYPE)
#define GHC_USE_WCHAR_T
    : private path_helper_base<std::wstring::value_type>
{
public:
    using path_helper_base<std::wstring::value_type>::value_type;
#else
    : private path_helper_base<std::string::value_type>
{
public:
    using path_helper_base<std::string::value_type>::value_type;
#endif
    using string_type = std::basic_string<value_type>;
    using path_helper_base<value_type>::preferred_separator;
    
    // 30.10.10.1 enumeration format
    /// The path format in wich the constructor argument is given.
    enum format {
        generic_format,  ///< The generic format, internally used by
                         ///< ghc::filesystem::path with slashes
        native_format,   ///< The format native to the current platform this code
                         ///< is build for
        auto_format,     ///< Try to auto-detect the format, fallback to native
    };

    template <class T>
    struct _is_basic_string : std::false_type
    {
    };
    template <class CharT, class Traits, class Alloc>
    struct _is_basic_string<std::basic_string<CharT, Traits, Alloc>> : std::true_type
    {
    };
#ifdef __cpp_lib_string_view
    template <class CharT>
    struct _is_basic_string<std::basic_string_view<CharT>> : std::true_type
    {
    };
#endif

    template <typename T1, typename T2 = void>
    using path_type = typename std::enable_if<!std::is_same<path, T1>::value, path>::type;
#ifdef GHC_USE_WCHAR_T
    template <typename T>
    using path_from_string = typename std::enable_if<_is_basic_string<T>::value || std::is_same<char const*, typename std::decay<T>::type>::value || std::is_same<char*, typename std::decay<T>::type>::value ||
                                                         std::is_same<wchar_t const*, typename std::decay<T>::type>::value || std::is_same<wchar_t*, typename std::decay<T>::type>::value,
                                                     path>::type;
    template <typename T>
    using path_type_EcharT = typename std::enable_if<std::is_same<T, char>::value || std::is_same<T, char16_t>::value || std::is_same<T, char32_t>::value, path>::type;
#else
    template <typename T>
    using path_from_string = typename std::enable_if<_is_basic_string<T>::value || std::is_same<char const*, typename std::decay<T>::type>::value || std::is_same<char*, typename std::decay<T>::type>::value, path>::type;
    template <typename T>
    using path_type_EcharT = typename std::enable_if<std::is_same<T, char>::value || std::is_same<T, char16_t>::value || std::is_same<T, char32_t>::value || std::is_same<T, wchar_t>::value, path>::type;
#endif
    // 30.10.8.4.1 constructors and destructor
    path() noexcept;
    path(const path& p);
    path(path&& p) noexcept;
    path(string_type&& source, format fmt = auto_format);
    template <class Source, typename = path_from_string<Source>>
    path(const Source& source, format fmt = auto_format);
    template <class InputIterator>
    path(InputIterator first, InputIterator last, format fmt = auto_format);
#ifdef GHC_WITH_EXCEPTIONS
    template <class Source, typename = path_from_string<Source>>
    path(const Source& source, const std::locale& loc, format fmt = auto_format);
    template <class InputIterator>
    path(InputIterator first, InputIterator last, const std::locale& loc, format fmt = auto_format);
#endif
    ~path();

    // 30.10.8.4.2 assignments
    path& operator=(const path& p);
    path& operator=(path&& p) noexcept;
    path& operator=(string_type&& source);
    path& assign(string_type&& source);
    template <class Source>
    path& operator=(const Source& source);
    template <class Source>
    path& assign(const Source& source);
    template <class InputIterator>
    path& assign(InputIterator first, InputIterator last);

    // 30.10.8.4.3 appends
    path& operator/=(const path& p);
    template <class Source>
    path& operator/=(const Source& source);
    template <class Source>
    path& append(const Source& source);
    template <class InputIterator>
    path& append(InputIterator first, InputIterator last);

    // 30.10.8.4.4 concatenation
    path& operator+=(const path& x);
    path& operator+=(const string_type& x);
#ifdef __cpp_lib_string_view
    path& operator+=(std::basic_string_view<value_type> x);
#endif
    path& operator+=(const value_type* x);
    path& operator+=(value_type x);
    template <class Source>
    path_from_string<Source>& operator+=(const Source& x);
    template <class EcharT>
    path_type_EcharT<EcharT>& operator+=(EcharT x);
    template <class Source>
    path& concat(const Source& x);
    template <class InputIterator>
    path& concat(InputIterator first, InputIterator last);

    // 30.10.8.4.5 modifiers
    void clear() noexcept;
    path& make_preferred();
    path& remove_filename();
    path& replace_filename(const path& replacement);
    path& replace_extension(const path& replacement = path());
    void swap(path& rhs) noexcept;

    // 30.10.8.4.6 native format observers
    const string_type& native() const;  // this implementation doesn't support noexcept for native()
    const value_type* c_str() const;    // this implementation doesn't support noexcept for c_str()
    operator string_type() const;
    template <class EcharT, class traits = std::char_traits<EcharT>, class Allocator = std::allocator<EcharT>>
    std::basic_string<EcharT, traits, Allocator> string(const Allocator& a = Allocator()) const;
    std::string string() const;
    std::wstring wstring() const;
    std::string u8string() const;
    std::u16string u16string() const;
    std::u32string u32string() const;

    // 30.10.8.4.7 generic format observers
    template <class EcharT, class traits = std::char_traits<EcharT>, class Allocator = std::allocator<EcharT>>
    std::basic_string<EcharT, traits, Allocator> generic_string(const Allocator& a = Allocator()) const;
    const std::string& generic_string() const;  // this is different from the standard, that returns by value
    std::wstring generic_wstring() const;
    std::string generic_u8string() const;
    std::u16string generic_u16string() const;
    std::u32string generic_u32string() const;

    // 30.10.8.4.8 compare
    int compare(const path& p) const noexcept;
    int compare(const string_type& s) const;
#ifdef __cpp_lib_string_view
    int compare(std::basic_string_view<value_type> s) const;
#endif
    int compare(const value_type* s) const;

    // 30.10.8.4.9 decomposition
    path root_name() const;
    path root_directory() const;
    path root_path() const;
    path relative_path() const;
    path parent_path() const;
    path filename() const;
    path stem() const;
    path extension() const;

    // 30.10.8.4.10 query
    bool empty() const noexcept;
    bool has_root_name() const;
    bool has_root_directory() const;
    bool has_root_path() const;
    bool has_relative_path() const;
    bool has_parent_path() const;
    bool has_filename() const;
    bool has_stem() const;
    bool has_extension() const;
    bool is_absolute() const;
    bool is_relative() const;

    // 30.10.8.4.11 generation
    path lexically_normal() const;
    path lexically_relative(const path& base) const;
    path lexically_proximate(const path& base) const;

    // 30.10.8.5 iterators
    class iterator;
    using const_iterator = iterator;
    iterator begin() const;
    iterator end() const;

private:
    using impl_value_type = std::string::value_type;
    using impl_string_type = std::basic_string<impl_value_type>;
    friend class directory_iterator;
    void append_name(const char* name);
    static constexpr impl_value_type generic_separator = '/';
    template <typename InputIterator>
    class input_iterator_range
    {
    public:
        typedef InputIterator iterator;
        typedef InputIterator const_iterator;
        typedef typename InputIterator::difference_type difference_type;

        input_iterator_range(const InputIterator& first, const InputIterator& last)
            : _first(first)
            , _last(last)
        {
        }

        InputIterator begin() const { return _first; }
        InputIterator end() const { return _last; }

    private:
        InputIterator _first;
        InputIterator _last;
    };
    friend void swap(path& lhs, path& rhs) noexcept;
    friend size_t hash_value(const path& p) noexcept;
    static void postprocess_path_with_format(impl_string_type& p, format fmt);
    impl_string_type _path;
#ifdef GHC_OS_WINDOWS
    impl_string_type native_impl() const;
    mutable string_type _native_cache;
#else
    const impl_string_type& native_impl() const;
#endif
};

// 30.10.8.6 path non-member functions
GHC_FS_API void swap(path& lhs, path& rhs) noexcept;
GHC_FS_API size_t hash_value(const path& p) noexcept;
GHC_FS_API bool operator==(const path& lhs, const path& rhs) noexcept;
GHC_FS_API bool operator!=(const path& lhs, const path& rhs) noexcept;
GHC_FS_API bool operator<(const path& lhs, const path& rhs) noexcept;
GHC_FS_API bool operator<=(const path& lhs, const path& rhs) noexcept;
GHC_FS_API bool operator>(const path& lhs, const path& rhs) noexcept;
GHC_FS_API bool operator>=(const path& lhs, const path& rhs) noexcept;

GHC_FS_API path operator/(const path& lhs, const path& rhs);

// 30.10.8.6.1 path inserter and extractor
template <class charT, class traits>
std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& os, const path& p);
template <class charT, class traits>
std::basic_istream<charT, traits>& operator>>(std::basic_istream<charT, traits>& is, path& p);

// 30.10.8.6.2 path factory functions
template <class Source, typename = path::path_from_string<Source>>
path u8path(const Source& source);
template <class InputIterator>
path u8path(InputIterator first, InputIterator last);

// 30.10.9 class filesystem_error
class GHC_FS_API_CLASS filesystem_error : public std::system_error
{
public:
    filesystem_error(const std::string& what_arg, std::error_code ec);
    filesystem_error(const std::string& what_arg, const path& p1, std::error_code ec);
    filesystem_error(const std::string& what_arg, const path& p1, const path& p2, std::error_code ec);
    const path& path1() const noexcept;
    const path& path2() const noexcept;
    const char* what() const noexcept override;

private:
    std::string _what_arg;
    std::error_code _ec;
    path _p1, _p2;
};

class GHC_FS_API_CLASS path::iterator
{
public:
    using value_type = const path;
    using difference_type = std::ptrdiff_t;
    using pointer = const path*;
    using reference = const path&;
    using iterator_category = std::bidirectional_iterator_tag;

    iterator();
    iterator(const impl_string_type::const_iterator& first, const impl_string_type::const_iterator& last, const impl_string_type::const_iterator& pos);
    iterator& operator++();
    iterator operator++(int);
    iterator& operator--();
    iterator operator--(int);
    bool operator==(const iterator& other) const;
    bool operator!=(const iterator& other) const;
    reference operator*() const;
    pointer operator->() const;

private:
    impl_string_type::const_iterator increment(const std::string::const_iterator& pos) const;
    impl_string_type::const_iterator decrement(const std::string::const_iterator& pos) const;
    void updateCurrent();
    impl_string_type::const_iterator _first;
    impl_string_type::const_iterator _last;
    impl_string_type::const_iterator _root;
    impl_string_type::const_iterator _iter;
    path _current;
};

struct space_info
{
    uintmax_t capacity;
    uintmax_t free;
    uintmax_t available;
};

// 30.10.10, enumerations
enum class file_type {
    none,
    not_found,
    regular,
    directory,
    symlink,
    block,
    character,
    fifo,
    socket,
    unknown,
};

enum class perms : uint16_t {
    none = 0,

    owner_read = 0400,
    owner_write = 0200,
    owner_exec = 0100,
    owner_all = 0700,

    group_read = 040,
    group_write = 020,
    group_exec = 010,
    group_all = 070,

    others_read = 04,
    others_write = 02,
    others_exec = 01,
    others_all = 07,

    all = 0777,
    set_uid = 04000,
    set_gid = 02000,
    sticky_bit = 01000,

    mask = 07777,
    unknown = 0xffff
};

enum class perm_options : uint16_t {
    replace = 3,
    add = 1,
    remove = 2,
    nofollow = 4,
};

enum class copy_options : uint16_t {
    none = 0,

    skip_existing = 1,
    overwrite_existing = 2,
    update_existing = 4,

    recursive = 8,

    copy_symlinks = 0x10,
    skip_symlinks = 0x20,

    directories_only = 0x40,
    create_symlinks = 0x80,
    create_hard_links = 0x100
};

enum class directory_options : uint16_t {
    none = 0,
    follow_directory_symlink = 1,
    skip_permission_denied = 2,
};

// 30.10.11 class file_status
class GHC_FS_API_CLASS file_status
{
public:
    // 30.10.11.1 constructors and destructor
    file_status() noexcept;
    explicit file_status(file_type ft, perms prms = perms::unknown) noexcept;
    file_status(const file_status&) noexcept;
    file_status(file_status&&) noexcept;
    ~file_status();
    // assignments:
    file_status& operator=(const file_status&) noexcept;
    file_status& operator=(file_status&&) noexcept;
    // 30.10.11.3 modifiers
    void type(file_type ft) noexcept;
    void permissions(perms prms) noexcept;
    // 30.10.11.2 observers
    file_type type() const noexcept;
    perms permissions() const noexcept;

private:
    file_type _type;
    perms _perms;
};

using file_time_type = std::chrono::time_point<std::chrono::system_clock>;

// 30.10.12 Class directory_entry
class GHC_FS_API_CLASS directory_entry
{
public:
    // 30.10.12.1 constructors and destructor
    directory_entry() noexcept = default;
    directory_entry(const directory_entry&) = default;
    directory_entry(directory_entry&&) noexcept = default;
#ifdef GHC_WITH_EXCEPTIONS
    explicit directory_entry(const path& p);
#endif
    directory_entry(const path& p, std::error_code& ec);
    ~directory_entry();

    // assignments:
    directory_entry& operator=(const directory_entry&) = default;
    directory_entry& operator=(directory_entry&&) noexcept = default;

    // 30.10.12.2 modifiers
#ifdef GHC_WITH_EXCEPTIONS
    void assign(const path& p);
#endif
    void assign(const path& p, std::error_code& ec);
#ifdef GHC_WITH_EXCEPTIONS
    void replace_filename(const path& p);
#endif
    void replace_filename(const path& p, std::error_code& ec);
#ifdef GHC_WITH_EXCEPTIONS
    void refresh();
#endif
    void refresh(std::error_code& ec) noexcept;

    // 30.10.12.3 observers
    const filesystem::path& path() const noexcept;
    operator const filesystem::path&() const noexcept;
#ifdef GHC_WITH_EXCEPTIONS
    bool exists() const;
#endif
    bool exists(std::error_code& ec) const noexcept;
#ifdef GHC_WITH_EXCEPTIONS
    bool is_block_file() const;
#endif
    bool is_block_file(std::error_code& ec) const noexcept;
#ifdef GHC_WITH_EXCEPTIONS
    bool is_character_file() const;
#endif
    bool is_character_file(std::error_code& ec) const noexcept;
#ifdef GHC_WITH_EXCEPTIONS
    bool is_directory() const;
#endif
    bool is_directory(std::error_code& ec) const noexcept;
#ifdef GHC_WITH_EXCEPTIONS
    bool is_fifo() const;
#endif
    bool is_fifo(std::error_code& ec) const noexcept;
#ifdef GHC_WITH_EXCEPTIONS
    bool is_other() const;
#endif
    bool is_other(std::error_code& ec) const noexcept;
#ifdef GHC_WITH_EXCEPTIONS
    bool is_regular_file() const;
#endif
    bool is_regular_file(std::error_code& ec) const noexcept;
#ifdef GHC_WITH_EXCEPTIONS
    bool is_socket() const;
#endif
    bool is_socket(std::error_code& ec) const noexcept;
#ifdef GHC_WITH_EXCEPTIONS
    bool is_symlink() const;
#endif
    bool is_symlink(std::error_code& ec) const noexcept;
#ifdef GHC_WITH_EXCEPTIONS
    uintmax_t file_size() const;
#endif
    uintmax_t file_size(std::error_code& ec) const noexcept;
#ifdef GHC_WITH_EXCEPTIONS
    uintmax_t hard_link_count() const;
#endif
    uintmax_t hard_link_count(std::error_code& ec) const noexcept;
#ifdef GHC_WITH_EXCEPTIONS
    file_time_type last_write_time() const;
#endif
    file_time_type last_write_time(std::error_code& ec) const noexcept;

#ifdef GHC_WITH_EXCEPTIONS
    file_status status() const;
#endif
    file_status status(std::error_code& ec) const noexcept;

#ifdef GHC_WITH_EXCEPTIONS
    file_status symlink_status() const;
#endif
    file_status symlink_status(std::error_code& ec) const noexcept;
    bool operator<(const directory_entry& rhs) const noexcept;
    bool operator==(const directory_entry& rhs) const noexcept;
    bool operator!=(const directory_entry& rhs) const noexcept;
    bool operator<=(const directory_entry& rhs) const noexcept;
    bool operator>(const directory_entry& rhs) const noexcept;
    bool operator>=(const directory_entry& rhs) const noexcept;

private:
    friend class directory_iterator;
    filesystem::path _path;
    file_status _status;
    file_status _symlink_status;
    uintmax_t _file_size = 0;
#ifndef GHC_OS_WINDOWS
    uintmax_t _hard_link_count = 0;
#endif
    time_t _last_write_time = 0;
};

// 30.10.13 Class directory_iterator
class GHC_FS_API_CLASS directory_iterator
{
public:
    class GHC_FS_API_CLASS proxy
    {
    public:
        const directory_entry& operator*() const& noexcept { return _dir_entry; }
        directory_entry operator*() && noexcept { return std::move(_dir_entry); }

    private:
        explicit proxy(const directory_entry& dir_entry)
            : _dir_entry(dir_entry)
        {
        }
        friend class directory_iterator;
        friend class recursive_directory_iterator;
        directory_entry _dir_entry;
    };
    using iterator_category = std::input_iterator_tag;
    using value_type = directory_entry;
    using difference_type = std::ptrdiff_t;
    using pointer = const directory_entry*;
    using reference = const directory_entry&;

    // 30.10.13.1 member functions
    directory_iterator() noexcept;
#ifdef GHC_WITH_EXCEPTIONS
    explicit directory_iterator(const path& p);
    directory_iterator(const path& p, directory_options options);
#endif
    directory_iterator(const path& p, std::error_code& ec) noexcept;
    directory_iterator(const path& p, directory_options options, std::error_code& ec) noexcept;
    directory_iterator(const directory_iterator& rhs);
    directory_iterator(directory_iterator&& rhs) noexcept;
    ~directory_iterator();
    directory_iterator& operator=(const directory_iterator& rhs);
    directory_iterator& operator=(directory_iterator&& rhs) noexcept;
    const directory_entry& operator*() const;
    const directory_entry* operator->() const;
#ifdef GHC_WITH_EXCEPTIONS
    directory_iterator& operator++();
#endif
    directory_iterator& increment(std::error_code& ec) noexcept;

    // other members as required by 27.2.3, input iterators
#ifdef GHC_WITH_EXCEPTIONS
    proxy operator++(int)
    {
        proxy p{**this};
        ++*this;
        return p;
    }
#endif
    bool operator==(const directory_iterator& rhs) const;
    bool operator!=(const directory_iterator& rhs) const;

private:
    friend class recursive_directory_iterator;
    class impl;
    std::shared_ptr<impl> _impl;
};

// 30.10.13.2 directory_iterator non-member functions
GHC_FS_API directory_iterator begin(directory_iterator iter) noexcept;
GHC_FS_API directory_iterator end(const directory_iterator&) noexcept;

// 30.10.14 class recursive_directory_iterator
class GHC_FS_API_CLASS recursive_directory_iterator
{
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = directory_entry;
    using difference_type = std::ptrdiff_t;
    using pointer = const directory_entry*;
    using reference = const directory_entry&;

    // 30.10.14.1 constructors and destructor
    recursive_directory_iterator() noexcept;
#ifdef GHC_WITH_EXCEPTIONS
    explicit recursive_directory_iterator(const path& p);
    recursive_directory_iterator(const path& p, directory_options options);
#endif
    recursive_directory_iterator(const path& p, directory_options options, std::error_code& ec) noexcept;
    recursive_directory_iterator(const path& p, std::error_code& ec) noexcept;
    recursive_directory_iterator(const recursive_directory_iterator& rhs);
    recursive_directory_iterator(recursive_directory_iterator&& rhs) noexcept;
    ~recursive_directory_iterator();

    // 30.10.14.1 observers
    directory_options options() const;
    int depth() const;
    bool recursion_pending() const;

    const directory_entry& operator*() const;
    const directory_entry* operator->() const;

    // 30.10.14.1 modifiers recursive_directory_iterator&
    recursive_directory_iterator& operator=(const recursive_directory_iterator& rhs);
    recursive_directory_iterator& operator=(recursive_directory_iterator&& rhs) noexcept;
#ifdef GHC_WITH_EXCEPTIONS
    recursive_directory_iterator& operator++();
#endif
    recursive_directory_iterator& increment(std::error_code& ec) noexcept;

#ifdef GHC_WITH_EXCEPTIONS
    void pop();
#endif
    void pop(std::error_code& ec);
    void disable_recursion_pending();

    // other members as required by 27.2.3, input iterators
#ifdef GHC_WITH_EXCEPTIONS
    directory_iterator::proxy operator++(int)
    {
        directory_iterator::proxy proxy{**this};
        ++*this;
        return proxy;
    }
#endif
    bool operator==(const recursive_directory_iterator& rhs) const;
    bool operator!=(const recursive_directory_iterator& rhs) const;

private:
    struct recursive_directory_iterator_impl
    {
        directory_options _options;
        bool _recursion_pending;
        std::stack<directory_iterator> _dir_iter_stack;
        recursive_directory_iterator_impl(directory_options options, bool recursion_pending)
            : _options(options)
            , _recursion_pending(recursion_pending)
        {
        }
    };
    std::shared_ptr<recursive_directory_iterator_impl> _impl;
};

// 30.10.14.2 directory_iterator non-member functions
GHC_FS_API recursive_directory_iterator begin(recursive_directory_iterator iter) noexcept;
GHC_FS_API recursive_directory_iterator end(const recursive_directory_iterator&) noexcept;

// 30.10.15 filesystem operations
#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API path absolute(const path& p);
#endif
GHC_FS_API path absolute(const path& p, std::error_code& ec);

#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API path canonical(const path& p);
#endif
GHC_FS_API path canonical(const path& p, std::error_code& ec);

#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API void copy(const path& from, const path& to);
#endif
GHC_FS_API void copy(const path& from, const path& to, std::error_code& ec) noexcept;
#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API void copy(const path& from, const path& to, copy_options options);
#endif
GHC_FS_API void copy(const path& from, const path& to, copy_options options, std::error_code& ec) noexcept;

#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API bool copy_file(const path& from, const path& to);
#endif
GHC_FS_API bool copy_file(const path& from, const path& to, std::error_code& ec) noexcept;
#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API bool copy_file(const path& from, const path& to, copy_options option);
#endif
GHC_FS_API bool copy_file(const path& from, const path& to, copy_options option, std::error_code& ec) noexcept;

#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API void copy_symlink(const path& existing_symlink, const path& new_symlink);
#endif
GHC_FS_API void copy_symlink(const path& existing_symlink, const path& new_symlink, std::error_code& ec) noexcept;

#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API bool create_directories(const path& p);
#endif
GHC_FS_API bool create_directories(const path& p, std::error_code& ec) noexcept;

#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API bool create_directory(const path& p);
#endif
GHC_FS_API bool create_directory(const path& p, std::error_code& ec) noexcept;

#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API bool create_directory(const path& p, const path& attributes);
#endif
GHC_FS_API bool create_directory(const path& p, const path& attributes, std::error_code& ec) noexcept;

#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API void create_directory_symlink(const path& to, const path& new_symlink);
#endif
GHC_FS_API void create_directory_symlink(const path& to, const path& new_symlink, std::error_code& ec) noexcept;

#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API void create_hard_link(const path& to, const path& new_hard_link);
#endif
GHC_FS_API void create_hard_link(const path& to, const path& new_hard_link, std::error_code& ec) noexcept;

#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API void create_symlink(const path& to, const path& new_symlink);
#endif
GHC_FS_API void create_symlink(const path& to, const path& new_symlink, std::error_code& ec) noexcept;

#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API path current_path();
#endif
GHC_FS_API path current_path(std::error_code& ec);
#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API void current_path(const path& p);
#endif
GHC_FS_API void current_path(const path& p, std::error_code& ec) noexcept;

GHC_FS_API bool exists(file_status s) noexcept;
#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API bool exists(const path& p);
#endif
GHC_FS_API bool exists(const path& p, std::error_code& ec) noexcept;

#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API bool equivalent(const path& p1, const path& p2);
#endif
GHC_FS_API bool equivalent(const path& p1, const path& p2, std::error_code& ec) noexcept;

#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API uintmax_t file_size(const path& p);
#endif
GHC_FS_API uintmax_t file_size(const path& p, std::error_code& ec) noexcept;

#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API uintmax_t hard_link_count(const path& p);
#endif
GHC_FS_API uintmax_t hard_link_count(const path& p, std::error_code& ec) noexcept;

GHC_FS_API bool is_block_file(file_status s) noexcept;
#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API bool is_block_file(const path& p);
#endif
GHC_FS_API bool is_block_file(const path& p, std::error_code& ec) noexcept;
GHC_FS_API bool is_character_file(file_status s) noexcept;
#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API bool is_character_file(const path& p);
#endif
GHC_FS_API bool is_character_file(const path& p, std::error_code& ec) noexcept;
GHC_FS_API bool is_directory(file_status s) noexcept;
#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API bool is_directory(const path& p);
#endif
GHC_FS_API bool is_directory(const path& p, std::error_code& ec) noexcept;
#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API bool is_empty(const path& p);
#endif
GHC_FS_API bool is_empty(const path& p, std::error_code& ec) noexcept;
GHC_FS_API bool is_fifo(file_status s) noexcept;
#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API bool is_fifo(const path& p);
#endif
GHC_FS_API bool is_fifo(const path& p, std::error_code& ec) noexcept;
GHC_FS_API bool is_other(file_status s) noexcept;
#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API bool is_other(const path& p);
#endif
GHC_FS_API bool is_other(const path& p, std::error_code& ec) noexcept;
GHC_FS_API bool is_regular_file(file_status s) noexcept;
#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API bool is_regular_file(const path& p);
#endif
GHC_FS_API bool is_regular_file(const path& p, std::error_code& ec) noexcept;
GHC_FS_API bool is_socket(file_status s) noexcept;
#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API bool is_socket(const path& p);
#endif
GHC_FS_API bool is_socket(const path& p, std::error_code& ec) noexcept;
GHC_FS_API bool is_symlink(file_status s) noexcept;
#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API bool is_symlink(const path& p);
#endif
GHC_FS_API bool is_symlink(const path& p, std::error_code& ec) noexcept;

#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API file_time_type last_write_time(const path& p);
#endif
GHC_FS_API file_time_type last_write_time(const path& p, std::error_code& ec) noexcept;
#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API void last_write_time(const path& p, file_time_type new_time);
#endif
GHC_FS_API void last_write_time(const path& p, file_time_type new_time, std::error_code& ec) noexcept;

#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API void permissions(const path& p, perms prms, perm_options opts = perm_options::replace);
#endif
GHC_FS_API void permissions(const path& p, perms prms, std::error_code& ec) noexcept;
GHC_FS_API void permissions(const path& p, perms prms, perm_options opts, std::error_code& ec);

#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API path proximate(const path& p, std::error_code& ec);
GHC_FS_API path proximate(const path& p, const path& base = current_path());
#endif
GHC_FS_API path proximate(const path& p, const path& base, std::error_code& ec);

#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API path read_symlink(const path& p);
#endif
GHC_FS_API path read_symlink(const path& p, std::error_code& ec);

GHC_FS_API path relative(const path& p, std::error_code& ec);
#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API path relative(const path& p, const path& base = current_path());
#endif
GHC_FS_API path relative(const path& p, const path& base, std::error_code& ec);

#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API bool remove(const path& p);
#endif
GHC_FS_API bool remove(const path& p, std::error_code& ec) noexcept;

#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API uintmax_t remove_all(const path& p);
#endif
GHC_FS_API uintmax_t remove_all(const path& p, std::error_code& ec) noexcept;

#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API void rename(const path& from, const path& to);
#endif
GHC_FS_API void rename(const path& from, const path& to, std::error_code& ec) noexcept;

#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API void resize_file(const path& p, uintmax_t size);
#endif
GHC_FS_API void resize_file(const path& p, uintmax_t size, std::error_code& ec) noexcept;

#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API space_info space(const path& p);
#endif
GHC_FS_API space_info space(const path& p, std::error_code& ec) noexcept;

#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API file_status status(const path& p);
#endif
GHC_FS_API file_status status(const path& p, std::error_code& ec) noexcept;

GHC_FS_API bool status_known(file_status s) noexcept;

#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API file_status symlink_status(const path& p);
#endif
GHC_FS_API file_status symlink_status(const path& p, std::error_code& ec) noexcept;

#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API path temp_directory_path();
#endif
GHC_FS_API path temp_directory_path(std::error_code& ec) noexcept;

#ifdef GHC_WITH_EXCEPTIONS
GHC_FS_API path weakly_canonical(const path& p);
#endif
GHC_FS_API path weakly_canonical(const path& p, std::error_code& ec) noexcept;

// Non-C++17 add-on std::fstream wrappers with path
template <class charT, class traits = std::char_traits<charT>>
class basic_filebuf : public std::basic_filebuf<charT, traits>
{
public:
    basic_filebuf() {}
    ~basic_filebuf() override {}
    basic_filebuf(const basic_filebuf&) = delete;
    const basic_filebuf& operator=(const basic_filebuf&) = delete;
    basic_filebuf<charT, traits>* open(const path& p, std::ios_base::openmode mode)
    {
#if defined(GHC_OS_WINDOWS) && !defined(__GNUC__)
        return std::basic_filebuf<charT, traits>::open(p.wstring().c_str(), mode) ? this : 0;
#else
        return std::basic_filebuf<charT, traits>::open(p.string().c_str(), mode) ? this : 0;
#endif
    }
};

template <class charT, class traits = std::char_traits<charT>>
class basic_ifstream : public std::basic_ifstream<charT, traits>
{
public:
    basic_ifstream() {}
#if defined(GHC_OS_WINDOWS) && !defined(__GNUC__)
    explicit basic_ifstream(const path& p, std::ios_base::openmode mode = std::ios_base::in)
        : std::basic_ifstream<charT, traits>(p.wstring().c_str(), mode)
    {
    }
    void open(const path& p, std::ios_base::openmode mode = std::ios_base::in) { std::basic_ifstream<charT, traits>::open(p.wstring().c_str(), mode); }
#else
    explicit basic_ifstream(const path& p, std::ios_base::openmode mode = std::ios_base::in)
        : std::basic_ifstream<charT, traits>(p.string().c_str(), mode)
    {
    }
    void open(const path& p, std::ios_base::openmode mode = std::ios_base::in) { std::basic_ifstream<charT, traits>::open(p.string().c_str(), mode); }
#endif
    basic_ifstream(const basic_ifstream&) = delete;
    const basic_ifstream& operator=(const basic_ifstream&) = delete;
    ~basic_ifstream() override {}
};

template <class charT, class traits = std::char_traits<charT>>
class basic_ofstream : public std::basic_ofstream<charT, traits>
{
public:
    basic_ofstream() {}
#if defined(GHC_OS_WINDOWS) && !defined(__GNUC__)
    explicit basic_ofstream(const path& p, std::ios_base::openmode mode = std::ios_base::out)
        : std::basic_ofstream<charT, traits>(p.wstring().c_str(), mode)
    {
    }
    void open(const path& p, std::ios_base::openmode mode = std::ios_base::out) { std::basic_ofstream<charT, traits>::open(p.wstring().c_str(), mode); }
#else
    explicit basic_ofstream(const path& p, std::ios_base::openmode mode = std::ios_base::out)
        : std::basic_ofstream<charT, traits>(p.string().c_str(), mode)
    {
    }
    void open(const path& p, std::ios_base::openmode mode = std::ios_base::out) { std::basic_ofstream<charT, traits>::open(p.string().c_str(), mode); }
#endif
    basic_ofstream(const basic_ofstream&) = delete;
    const basic_ofstream& operator=(const basic_ofstream&) = delete;
    ~basic_ofstream() override {}
};

template <class charT, class traits = std::char_traits<charT>>
class basic_fstream : public std::basic_fstream<charT, traits>
{
public:
    basic_fstream() {}
#if defined(GHC_OS_WINDOWS) && !defined(__GNUC__)
    explicit basic_fstream(const path& p, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
        : std::basic_fstream<charT, traits>(p.wstring().c_str(), mode)
    {
    }
    void open(const path& p, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) { std::basic_fstream<charT, traits>::open(p.wstring().c_str(), mode); }
#else
    explicit basic_fstream(const path& p, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
        : std::basic_fstream<charT, traits>(p.string().c_str(), mode)
    {
    }
    void open(const path& p, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) { std::basic_fstream<charT, traits>::open(p.string().c_str(), mode); }
#endif
    basic_fstream(const basic_fstream&) = delete;
    const basic_fstream& operator=(const basic_fstream&) = delete;
    ~basic_fstream() override {}
};

typedef basic_filebuf<char> filebuf;
typedef basic_filebuf<wchar_t> wfilebuf;
typedef basic_ifstream<char> ifstream;
typedef basic_ifstream<wchar_t> wifstream;
typedef basic_ofstream<char> ofstream;
typedef basic_ofstream<wchar_t> wofstream;
typedef basic_fstream<char> fstream;
typedef basic_fstream<wchar_t> wfstream;

class GHC_FS_API_CLASS u8arguments
{
public:
    u8arguments(int& argc, char**& argv);
    ~u8arguments()
    {
        _refargc = _argc;
        _refargv = _argv;
    }

    bool valid() const { return _isvalid; }

private:
    int _argc;
    char** _argv;
    int& _refargc;
    char**& _refargv;
    bool _isvalid;
#ifdef GHC_OS_WINDOWS
    std::vector<std::string> _args;
    std::vector<char*> _argp;
#endif
};

//-------------------------------------------------------------------------------------------------
//  Implementation
//-------------------------------------------------------------------------------------------------

namespace detail {
// GHC_FS_API void postprocess_path_with_format(path::impl_string_type& p, path::format fmt);
enum utf8_states_t { S_STRT = 0, S_RJCT = 8 };
GHC_FS_API void appendUTF8(std::string& str, uint32_t unicode);
GHC_FS_API bool is_surrogate(uint32_t c);
GHC_FS_API bool is_high_surrogate(uint32_t c);
GHC_FS_API bool is_low_surrogate(uint32_t c);
GHC_FS_API unsigned consumeUtf8Fragment(const unsigned state, const uint8_t fragment, uint32_t& codepoint);
enum class portable_error {
    none = 0,
    exists,
    not_found,
    not_supported,
    not_implemented,
    invalid_argument,
    is_a_directory,
};
GHC_FS_API std::error_code make_error_code(portable_error err);
#ifdef GHC_OS_WINDOWS
GHC_FS_API std::error_code make_system_error(uint32_t err = 0);
#else
GHC_FS_API std::error_code make_system_error(int err = 0);
#endif
}  // namespace detail

namespace detail {

#ifdef GHC_EXPAND_IMPL

GHC_INLINE std::error_code make_error_code(portable_error err)
{
#ifdef GHC_OS_WINDOWS
    switch (err) {
        case portable_error::none:
            return std::error_code();
        case portable_error::exists:
            return std::error_code(ERROR_ALREADY_EXISTS, std::system_category());
        case portable_error::not_found:
            return std::error_code(ERROR_PATH_NOT_FOUND, std::system_category());
        case portable_error::not_supported:
            return std::error_code(ERROR_NOT_SUPPORTED, std::system_category());
        case portable_error::not_implemented:
            return std::error_code(ERROR_CALL_NOT_IMPLEMENTED, std::system_category());
        case portable_error::invalid_argument:
            return std::error_code(ERROR_INVALID_PARAMETER, std::system_category());
        case portable_error::is_a_directory:
#ifdef ERROR_DIRECTORY_NOT_SUPPORTED
            return std::error_code(ERROR_DIRECTORY_NOT_SUPPORTED, std::system_category());
#else
            return std::error_code(ERROR_NOT_SUPPORTED, std::system_category());
#endif
    }
#else
    switch (err) {
        case portable_error::none:
            return std::error_code();
        case portable_error::exists:
            return std::error_code(EEXIST, std::system_category());
        case portable_error::not_found:
            return std::error_code(ENOENT, std::system_category());
        case portable_error::not_supported:
            return std::error_code(ENOTSUP, std::system_category());
        case portable_error::not_implemented:
            return std::error_code(ENOSYS, std::system_category());
        case portable_error::invalid_argument:
            return std::error_code(EINVAL, std::system_category());
        case portable_error::is_a_directory:
            return std::error_code(EISDIR, std::system_category());
    }
#endif
    return std::error_code();
}

#ifdef GHC_OS_WINDOWS
GHC_INLINE std::error_code make_system_error(uint32_t err)
{
    return std::error_code(err ? static_cast<int>(err) : static_cast<int>(::GetLastError()), std::system_category());
}
#else
GHC_INLINE std::error_code make_system_error(int err)
{
    return std::error_code(err ? err : errno, std::system_category());
}
#endif
    
#endif  // GHC_EXPAND_IMPL

template <typename Enum>
using EnableBitmask = typename std::enable_if<std::is_same<Enum, perms>::value || std::is_same<Enum, perm_options>::value || std::is_same<Enum, copy_options>::value || std::is_same<Enum, directory_options>::value, Enum>::type;
}  // namespace detail

template <typename Enum>
detail::EnableBitmask<Enum> operator&(Enum X, Enum Y)
{
    using underlying = typename std::underlying_type<Enum>::type;
    return static_cast<Enum>(static_cast<underlying>(X) & static_cast<underlying>(Y));
}

template <typename Enum>
detail::EnableBitmask<Enum> operator|(Enum X, Enum Y)
{
    using underlying = typename std::underlying_type<Enum>::type;
    return static_cast<Enum>(static_cast<underlying>(X) | static_cast<underlying>(Y));
}

template <typename Enum>
detail::EnableBitmask<Enum> operator^(Enum X, Enum Y)
{
    using underlying = typename std::underlying_type<Enum>::type;
    return static_cast<Enum>(static_cast<underlying>(X) ^ static_cast<underlying>(Y));
}

template <typename Enum>
detail::EnableBitmask<Enum> operator~(Enum X)
{
    using underlying = typename std::underlying_type<Enum>::type;
    return static_cast<Enum>(~static_cast<underlying>(X));
}

template <typename Enum>
detail::EnableBitmask<Enum>& operator&=(Enum& X, Enum Y)
{
    X = X & Y;
    return X;
}

template <typename Enum>
detail::EnableBitmask<Enum>& operator|=(Enum& X, Enum Y)
{
    X = X | Y;
    return X;
}

template <typename Enum>
detail::EnableBitmask<Enum>& operator^=(Enum& X, Enum Y)
{
    X = X ^ Y;
    return X;
}

#ifdef GHC_EXPAND_IMPL

namespace detail {

GHC_INLINE bool in_range(uint32_t c, uint32_t lo, uint32_t hi)
{
    return (static_cast<uint32_t>(c - lo) < (hi - lo + 1));
}

GHC_INLINE bool is_surrogate(uint32_t c)
{
    return in_range(c, 0xd800, 0xdfff);
}

GHC_INLINE bool is_high_surrogate(uint32_t c)
{
    return (c & 0xfffffc00) == 0xd800;
}

GHC_INLINE bool is_low_surrogate(uint32_t c)
{
    return (c & 0xfffffc00) == 0xdc00;
}

GHC_INLINE void appendUTF8(std::string& str, uint32_t unicode)
{
    if (unicode <= 0x7f) {
        str.push_back(static_cast<char>(unicode));
    }
    else if (unicode >= 0x80 && unicode <= 0x7ff) {
        str.push_back(static_cast<char>((unicode >> 6) + 192));
        str.push_back(static_cast<char>((unicode & 0x3f) + 128));
    }
    else if ((unicode >= 0x800 && unicode <= 0xd7ff) || (unicode >= 0xe000 && unicode <= 0xffff)) {
        str.push_back(static_cast<char>((unicode >> 12) + 224));
        str.push_back(static_cast<char>(((unicode & 0xfff) >> 6) + 128));
        str.push_back(static_cast<char>((unicode & 0x3f) + 128));
    }
    else if (unicode >= 0x10000 && unicode <= 0x10ffff) {
        str.push_back(static_cast<char>((unicode >> 18) + 240));
        str.push_back(static_cast<char>(((unicode & 0x3ffff) >> 12) + 128));
        str.push_back(static_cast<char>(((unicode & 0xfff) >> 6) + 128));
        str.push_back(static_cast<char>((unicode & 0x3f) + 128));
    }
    else {
#ifdef GHC_RAISE_UNICODE_ERRORS
        throw filesystem_error("Illegal code point for unicode character.", str, std::make_error_code(std::errc::illegal_byte_sequence));
#else
        appendUTF8(str, 0xfffd);
#endif
    }
}

// Thanks to Bjoern Hoehrmann (https://bjoern.hoehrmann.de/utf-8/decoder/dfa/)
// and Taylor R Campbell for the ideas to this DFA approach of UTF-8 decoding;
// Generating debugging and shrinking my own DFA from scratch was a day of fun!
GHC_INLINE unsigned consumeUtf8Fragment(const unsigned state, const uint8_t fragment, uint32_t& codepoint)
{
    static const uint32_t utf8_state_info[] = {
        // encoded states
        0x11111111u, 0x11111111u, 0x77777777u, 0x77777777u, 0x88888888u, 0x88888888u, 0x88888888u, 0x88888888u, 0x22222299u, 0x22222222u, 0x22222222u, 0x22222222u, 0x3333333au, 0x33433333u, 0x9995666bu, 0x99999999u,
        0x88888880u, 0x22818108u, 0x88888881u, 0x88888882u, 0x88888884u, 0x88888887u, 0x88888886u, 0x82218108u, 0x82281108u, 0x88888888u, 0x88888883u, 0x88888885u, 0u,          0u,          0u,          0u,
    };
    uint8_t category = fragment < 128 ? 0 : (utf8_state_info[(fragment >> 3) & 0xf] >> ((fragment & 7) << 2)) & 0xf;
    codepoint = (state ? (codepoint << 6) | (fragment & 0x3fu) : (0xffu >> category) & fragment);
    return state == S_RJCT ? static_cast<unsigned>(S_RJCT) : static_cast<unsigned>((utf8_state_info[category + 16] >> (state << 2)) & 0xf);
}
    
GHC_INLINE bool validUtf8(const std::string& utf8String)
{
    std::string::const_iterator iter = utf8String.begin();
    unsigned utf8_state = S_STRT;
    std::uint32_t codepoint = 0;
    while (iter < utf8String.end()) {
        if ((utf8_state = consumeUtf8Fragment(utf8_state, static_cast<uint8_t>(*iter++), codepoint)) == S_RJCT) {
            return false;
        }
    }
    if (utf8_state) {
        return false;
    }
    return true;
}

}  // namespace detail
    
#endif
    
namespace detail {

template <class StringType, typename std::enable_if<(sizeof(typename StringType::value_type) == 1)>::type* = nullptr>
inline StringType fromUtf8(const std::string& utf8String, const typename StringType::allocator_type& alloc = typename StringType::allocator_type())
{
    return StringType(utf8String.begin(), utf8String.end(), alloc);
}

template <class StringType, typename std::enable_if<(sizeof(typename StringType::value_type) == 2)>::type* = nullptr>
inline StringType fromUtf8(const std::string& utf8String, const typename StringType::allocator_type& alloc = typename StringType::allocator_type())
{
    StringType result(alloc);
    result.reserve(utf8String.length());
    std::string::const_iterator iter = utf8String.begin();
    unsigned utf8_state = S_STRT;
    std::uint32_t codepoint = 0;
    while (iter < utf8String.end()) {
        if ((utf8_state = consumeUtf8Fragment(utf8_state, static_cast<uint8_t>(*iter++), codepoint)) == S_STRT) {
            if (codepoint <= 0xffff) {
                result += static_cast<typename StringType::value_type>(codepoint);
            }
            else {
                codepoint -= 0x10000;
                result += static_cast<typename StringType::value_type>((codepoint >> 10) + 0xd800);
                result += static_cast<typename StringType::value_type>((codepoint & 0x3ff) + 0xdc00);
            }
            codepoint = 0;
        }
        else if (utf8_state == S_RJCT) {
#ifdef GHC_RAISE_UNICODE_ERRORS
            throw filesystem_error("Illegal byte sequence for unicode character.", utf8String, std::make_error_code(std::errc::illegal_byte_sequence));
#else
            result += static_cast<typename StringType::value_type>(0xfffd);
            utf8_state = S_STRT;
            codepoint = 0;
#endif
        }
    }
    if (utf8_state) {
#ifdef GHC_RAISE_UNICODE_ERRORS
        throw filesystem_error("Illegal byte sequence for unicode character.", utf8String, std::make_error_code(std::errc::illegal_byte_sequence));
#else
        result += static_cast<typename StringType::value_type>(0xfffd);
#endif
    }
    return result;
}

template <class StringType, typename std::enable_if<(sizeof(typename StringType::value_type) == 4)>::type* = nullptr>
inline StringType fromUtf8(const std::string& utf8String, const typename StringType::allocator_type& alloc = typename StringType::allocator_type())
{
    StringType result(alloc);
    result.reserve(utf8String.length());
    std::string::const_iterator iter = utf8String.begin();
    unsigned utf8_state = S_STRT;
    std::uint32_t codepoint = 0;
    while (iter < utf8String.end()) {
        if ((utf8_state = consumeUtf8Fragment(utf8_state, static_cast<uint8_t>(*iter++), codepoint)) == S_STRT) {
            result += static_cast<typename StringType::value_type>(codepoint);
            codepoint = 0;
        }
        else if (utf8_state == S_RJCT) {
#ifdef GHC_RAISE_UNICODE_ERRORS
            throw filesystem_error("Illegal byte sequence for unicode character.", utf8String, std::make_error_code(std::errc::illegal_byte_sequence));
#else
            result += static_cast<typename StringType::value_type>(0xfffd);
            utf8_state = S_STRT;
            codepoint = 0;
#endif
        }
    }
    if (utf8_state) {
#ifdef GHC_RAISE_UNICODE_ERRORS
        throw filesystem_error("Illegal byte sequence for unicode character.", utf8String, std::make_error_code(std::errc::illegal_byte_sequence));
#else
        result += static_cast<typename StringType::value_type>(0xfffd);
#endif
    }
    return result;
}

template <typename charT, typename traits, typename Alloc, typename std::enable_if<(sizeof(charT) == 1), int>::type size = 1>
inline std::string toUtf8(const std::basic_string<charT, traits, Alloc>& unicodeString)
{
    return std::string(unicodeString.begin(), unicodeString.end());
}

template <typename charT, typename traits, typename Alloc, typename std::enable_if<(sizeof(charT) == 2), int>::type size = 2>
inline std::string toUtf8(const std::basic_string<charT, traits, Alloc>& unicodeString)
{
    std::string result;
    for (auto iter = unicodeString.begin(); iter != unicodeString.end(); ++iter) {
        char32_t c = *iter;
        if (is_surrogate(c)) {
            ++iter;
            if (iter != unicodeString.end() && is_high_surrogate(c) && is_low_surrogate(*iter)) {
                appendUTF8(result, (char32_t(c) << 10) + *iter - 0x35fdc00);
            }
            else {
#ifdef GHC_RAISE_UNICODE_ERRORS
                throw filesystem_error("Illegal code point for unicode character.", result, std::make_error_code(std::errc::illegal_byte_sequence));
#else
                appendUTF8(result, 0xfffd);
                if(iter == unicodeString.end()) {
                    break;
                }
#endif
            }
        }
        else {
            appendUTF8(result, c);
        }
    }
    return result;
}

template <typename charT, typename traits, typename Alloc, typename std::enable_if<(sizeof(charT) == 4), int>::type size = 4>
inline std::string toUtf8(const std::basic_string<charT, traits, Alloc>& unicodeString)
{
    std::string result;
    for (auto c : unicodeString) {
        appendUTF8(result, static_cast<uint32_t>(c));
    }
    return result;
}

template <typename charT>
inline std::string toUtf8(const charT* unicodeString)
{
    return toUtf8(std::basic_string<charT, std::char_traits<charT>>(unicodeString));
}

}  // namespace detail

#ifdef GHC_EXPAND_IMPL

namespace detail {

GHC_INLINE bool startsWith(const std::string& what, const std::string& with)
{
    return with.length() <= what.length() && equal(with.begin(), with.end(), what.begin());
}

}  // namespace detail

GHC_INLINE void path::postprocess_path_with_format(path::impl_string_type& p, path::format fmt)
{
#ifdef GHC_RAISE_UNICODE_ERRORS
    if(!detail::validUtf8(p)) {
        path t;
        t._path = p;
        throw filesystem_error("Illegal byte sequence for unicode character.", t, std::make_error_code(std::errc::illegal_byte_sequence));
    }
#endif
    switch (fmt) {
#ifndef GHC_OS_WINDOWS
        case path::auto_format:
        case path::native_format:
#endif
        case path::generic_format:
            // nothing to do
            break;
#ifdef GHC_OS_WINDOWS
        case path::auto_format:
        case path::native_format:
            if (detail::startsWith(p, std::string("\\\\?\\"))) {
                // remove Windows long filename marker
                p.erase(0, 4);
                if (detail::startsWith(p, std::string("UNC\\"))) {
                    p.erase(0, 2);
                    p[0] = '\\';
                }
            }
            for (auto& c : p) {
                if (c == '\\') {
                    c = '/';
                }
            }
            break;
#endif
    }
    if (p.length() > 2 && p[0] == '/' && p[1] == '/' && p[2] != '/') {
        std::string::iterator new_end = std::unique(p.begin() + 2, p.end(), [](path::value_type lhs, path::value_type rhs) { return lhs == rhs && lhs == '/'; });
        p.erase(new_end, p.end());
    }
    else {
        std::string::iterator new_end = std::unique(p.begin(), p.end(), [](path::value_type lhs, path::value_type rhs) { return lhs == rhs && lhs == '/'; });
        p.erase(new_end, p.end());
    }
}

#endif  // GHC_EXPAND_IMPL

template <class Source, typename>
inline path::path(const Source& source, format fmt)
    : _path(detail::toUtf8(source))
{
    postprocess_path_with_format(_path, fmt);
}
template <>
inline path::path(const std::wstring& source, format fmt)
{
    _path = detail::toUtf8(source);
    postprocess_path_with_format(_path, fmt);
}
template <>
inline path::path(const std::u16string& source, format fmt)
{
    _path = detail::toUtf8(source);
    postprocess_path_with_format(_path, fmt);
}
template <>
inline path::path(const std::u32string& source, format fmt)
{
    _path = detail::toUtf8(source);
    postprocess_path_with_format(_path, fmt);
}

#ifdef __cpp_lib_string_view
template <>
inline path::path(const std::string_view& source, format fmt)
{
    _path = detail::toUtf8(std::string(source));
    postprocess_path_with_format(_path, fmt);
}
#ifdef GHC_USE_WCHAR_T
template <>
inline path::path(const std::wstring_view& source, format fmt)
{
    _path = detail::toUtf8(std::wstring(source).c_str());
    postprocess_path_with_format(_path, fmt);
}
#endif
#endif

template <class Source, typename>
inline path u8path(const Source& source)
{
    return path(source);
}
template <class InputIterator>
inline path u8path(InputIterator first, InputIterator last)
{
    return path(first, last);
}

template <class InputIterator>
inline path::path(InputIterator first, InputIterator last, format fmt)
    : path(std::basic_string<typename std::iterator_traits<InputIterator>::value_type>(first, last), fmt)
{
    // delegated
}

#ifdef GHC_EXPAND_IMPL

namespace detail {

GHC_INLINE bool equals_simple_insensitive(const char* str1, const char* str2)
{
#ifdef GHC_OS_WINDOWS
#ifdef __GNUC__
    while (::tolower((unsigned char)*str1) == ::tolower((unsigned char)*str2++)) {
        if (*str1++ == 0)
            return true;
    }
    return false;
#else
    return 0 == ::_stricmp(str1, str2);
#endif
#else
    return 0 == ::strcasecmp(str1, str2);
#endif
}

GHC_INLINE const char* strerror_adapter(char* gnu, char*)
{
    return gnu;
}

GHC_INLINE const char* strerror_adapter(int posix, char* buffer)
{
    if(posix) {
        return "Error in strerror_r!";
    }
    return buffer;
}

template <typename ErrorNumber>
GHC_INLINE std::string systemErrorText(ErrorNumber code = 0)
{
#if defined(GHC_OS_WINDOWS)
    LPVOID msgBuf;
    DWORD dw = code ? static_cast<DWORD>(code) : ::GetLastError();
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&msgBuf, 0, NULL);
    std::string msg = toUtf8(std::wstring((LPWSTR)msgBuf));
    LocalFree(msgBuf);
    return msg;
#else
    char buffer[512];
    return strerror_adapter(strerror_r(code ? code : errno, buffer, sizeof(buffer)), buffer);
#endif
}

#ifdef GHC_OS_WINDOWS
using CreateSymbolicLinkW_fp = BOOLEAN(WINAPI*)(LPCWSTR, LPCWSTR, DWORD);
using CreateHardLinkW_fp = BOOLEAN(WINAPI*)(LPCWSTR, LPCWSTR, LPSECURITY_ATTRIBUTES);

GHC_INLINE void create_symlink(const path& target_name, const path& new_symlink, bool to_directory, std::error_code& ec)
{
    std::error_code tec;
    auto fs = status(target_name, tec);
    if ((fs.type() == file_type::directory && !to_directory) || (fs.type() == file_type::regular && to_directory)) {
        ec = detail::make_error_code(detail::portable_error::not_supported);
        return;
    }
#if defined(__GNUC__) && __GNUC__ >= 8
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
    static CreateSymbolicLinkW_fp api_call = reinterpret_cast<CreateSymbolicLinkW_fp>(GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "CreateSymbolicLinkW"));
#if defined(__GNUC__) && __GNUC__ >= 8
#pragma GCC diagnostic pop
#endif
    if (api_call) {
        if (api_call(detail::fromUtf8<std::wstring>(new_symlink.u8string()).c_str(), detail::fromUtf8<std::wstring>(target_name.u8string()).c_str(), to_directory ? 1 : 0) == 0) {
            auto result = ::GetLastError();
            if (result == ERROR_PRIVILEGE_NOT_HELD && api_call(detail::fromUtf8<std::wstring>(new_symlink.u8string()).c_str(), detail::fromUtf8<std::wstring>(target_name.u8string()).c_str(), to_directory ? 3 : 2) != 0) {
                return;
            }
            ec = detail::make_system_error(result);
        }
    }
    else {
        ec = detail::make_system_error(ERROR_NOT_SUPPORTED);
    }
}

GHC_INLINE void create_hardlink(const path& target_name, const path& new_hardlink, std::error_code& ec)
{
#if defined(__GNUC__) && __GNUC__ >= 8
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
    static CreateHardLinkW_fp api_call = reinterpret_cast<CreateHardLinkW_fp>(GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "CreateHardLinkW"));
#if defined(__GNUC__) && __GNUC__ >= 8
#pragma GCC diagnostic pop
#endif
    if (api_call) {
        if (api_call(detail::fromUtf8<std::wstring>(new_hardlink.u8string()).c_str(), detail::fromUtf8<std::wstring>(target_name.u8string()).c_str(), NULL) == 0) {
            ec = detail::make_system_error();
        }
    }
    else {
        ec = detail::make_system_error(ERROR_NOT_SUPPORTED);
    }
}
#else
GHC_INLINE void create_symlink(const path& target_name, const path& new_symlink, bool, std::error_code& ec)
{
    if (::symlink(target_name.c_str(), new_symlink.c_str()) != 0) {
        ec = detail::make_system_error();
    }
}

GHC_INLINE void create_hardlink(const path& target_name, const path& new_hardlink, std::error_code& ec)
{
    if (::link(target_name.c_str(), new_hardlink.c_str()) != 0) {
        ec = detail::make_system_error();
    }
}
#endif

template <typename T>
GHC_INLINE file_status file_status_from_st_mode(T mode)
{
#ifdef GHC_OS_WINDOWS
    file_type ft = file_type::unknown;
    if ((mode & _S_IFDIR) == _S_IFDIR) {
        ft = file_type::directory;
    }
    else if ((mode & _S_IFREG) == _S_IFREG) {
        ft = file_type::regular;
    }
    else if ((mode & _S_IFCHR) == _S_IFCHR) {
        ft = file_type::character;
    }
    perms prms = static_cast<perms>(mode & 0xfff);
    return file_status(ft, prms);
#else
    file_type ft = file_type::unknown;
    if (S_ISDIR(mode)) {
        ft = file_type::directory;
    }
    else if (S_ISREG(mode)) {
        ft = file_type::regular;
    }
    else if (S_ISCHR(mode)) {
        ft = file_type::character;
    }
    else if (S_ISBLK(mode)) {
        ft = file_type::block;
    }
    else if (S_ISFIFO(mode)) {
        ft = file_type::fifo;
    }
    else if (S_ISLNK(mode)) {
        ft = file_type::symlink;
    }
    else if (S_ISSOCK(mode)) {
        ft = file_type::socket;
    }
    perms prms = static_cast<perms>(mode & 0xfff);
    return file_status(ft, prms);
#endif
}

GHC_INLINE path resolveSymlink(const path& p, std::error_code& ec)
{
#ifdef GHC_OS_WINDOWS
#ifndef REPARSE_DATA_BUFFER_HEADER_SIZE
    typedef struct _REPARSE_DATA_BUFFER
    {
        ULONG ReparseTag;
        USHORT ReparseDataLength;
        USHORT Reserved;
        union
        {
            struct
            {
                USHORT SubstituteNameOffset;
                USHORT SubstituteNameLength;
                USHORT PrintNameOffset;
                USHORT PrintNameLength;
                ULONG Flags;
                WCHAR PathBuffer[1];
            } SymbolicLinkReparseBuffer;
            struct
            {
                USHORT SubstituteNameOffset;
                USHORT SubstituteNameLength;
                USHORT PrintNameOffset;
                USHORT PrintNameLength;
                WCHAR PathBuffer[1];
            } MountPointReparseBuffer;
            struct
            {
                UCHAR DataBuffer[1];
            } GenericReparseBuffer;
        } DUMMYUNIONNAME;
    } REPARSE_DATA_BUFFER;
#ifndef MAXIMUM_REPARSE_DATA_BUFFER_SIZE
#define MAXIMUM_REPARSE_DATA_BUFFER_SIZE (16 * 1024)
#endif
#endif

    std::shared_ptr<void> file(CreateFileW(p.wstring().c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, 0), CloseHandle);
    if (file.get() == INVALID_HANDLE_VALUE) {
        ec = detail::make_system_error();
        return path();
    }

    std::shared_ptr<REPARSE_DATA_BUFFER> reparseData((REPARSE_DATA_BUFFER*)std::calloc(1, MAXIMUM_REPARSE_DATA_BUFFER_SIZE), std::free);
    ULONG bufferUsed;
    path result;
    if (DeviceIoControl(file.get(), FSCTL_GET_REPARSE_POINT, 0, 0, reparseData.get(), MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &bufferUsed, 0)) {
        if (IsReparseTagMicrosoft(reparseData->ReparseTag)) {
            switch (reparseData->ReparseTag) {
                case IO_REPARSE_TAG_SYMLINK:
                    result = std::wstring(&reparseData->SymbolicLinkReparseBuffer.PathBuffer[reparseData->SymbolicLinkReparseBuffer.PrintNameOffset / sizeof(WCHAR)], reparseData->SymbolicLinkReparseBuffer.PrintNameLength / sizeof(WCHAR));
                    break;
                case IO_REPARSE_TAG_MOUNT_POINT:
                    result = std::wstring(&reparseData->MountPointReparseBuffer.PathBuffer[reparseData->MountPointReparseBuffer.PrintNameOffset / sizeof(WCHAR)], reparseData->MountPointReparseBuffer.PrintNameLength / sizeof(WCHAR));
                    break;
                default:
                    break;
            }
        }
    }
    else {
        ec = detail::make_system_error();
    }
    return result;
#else
    size_t bufferSize = 256;
    while (true) {
        std::vector<char> buffer(bufferSize, static_cast<char>(0));
        auto rc = ::readlink(p.c_str(), buffer.data(), buffer.size());
        if (rc < 0) {
            ec = detail::make_system_error();
            return path();
        }
        else if (rc < static_cast<int>(bufferSize)) {
            return path(std::string(buffer.data(), static_cast<std::string::size_type>(rc)));
        }
        bufferSize *= 2;
    }
    return path();
#endif
}

#ifdef GHC_OS_WINDOWS
GHC_INLINE time_t timeFromFILETIME(const FILETIME& ft)
{
    ULARGE_INTEGER ull;
    ull.LowPart = ft.dwLowDateTime;
    ull.HighPart = ft.dwHighDateTime;
    return static_cast<time_t>(ull.QuadPart / 10000000ULL - 11644473600ULL);
}

GHC_INLINE void timeToFILETIME(time_t t, FILETIME& ft)
{
    LONGLONG ll;
    ll = Int32x32To64(t, 10000000) + 116444736000000000;
    ft.dwLowDateTime = static_cast<DWORD>(ll);
    ft.dwHighDateTime = static_cast<DWORD>(ll >> 32);
}

template <typename INFO>
GHC_INLINE uintmax_t hard_links_from_INFO(const INFO* info)
{
    return static_cast<uintmax_t>(-1);
}

template <>
GHC_INLINE uintmax_t hard_links_from_INFO<BY_HANDLE_FILE_INFORMATION>(const BY_HANDLE_FILE_INFORMATION* info)
{
    return info->nNumberOfLinks;
}

template <typename INFO>
GHC_INLINE file_status status_from_INFO(const path& p, const INFO* info, std::error_code&, uintmax_t* sz = nullptr, time_t* lwt = nullptr) noexcept
{
    file_type ft = file_type::unknown;
    if ((info->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
        ft = file_type::symlink;
    }
    else {
        if ((info->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            ft = file_type::directory;
        }
        else {
            ft = file_type::regular;
        }
    }
    perms prms = perms::owner_read | perms::group_read | perms::others_read;
    if (!(info->dwFileAttributes & FILE_ATTRIBUTE_READONLY)) {
        prms = prms | perms::owner_write | perms::group_write | perms::others_write;
    }
    std::string ext = p.extension().generic_string();
    if (equals_simple_insensitive(ext.c_str(), ".exe") || equals_simple_insensitive(ext.c_str(), ".cmd") || equals_simple_insensitive(ext.c_str(), ".bat") || equals_simple_insensitive(ext.c_str(), ".com")) {
        prms = prms | perms::owner_exec | perms::group_exec | perms::others_exec;
    }
    if (sz) {
        *sz = static_cast<uintmax_t>(info->nFileSizeHigh) << (sizeof(info->nFileSizeHigh) * 8) | info->nFileSizeLow;
    }
    if (lwt) {
        *lwt = detail::timeFromFILETIME(info->ftLastWriteTime);
    }
    return file_status(ft, prms);
}

#endif

GHC_INLINE bool is_not_found_error(std::error_code& ec)
{
#ifdef GHC_OS_WINDOWS
    return ec.value() == ERROR_FILE_NOT_FOUND || ec.value() == ERROR_PATH_NOT_FOUND || ec.value() == ERROR_INVALID_NAME;
#else
    return ec.value() == ENOENT || ec.value() == ENOTDIR;
#endif
}

GHC_INLINE file_status symlink_status_ex(const path& p, std::error_code& ec, uintmax_t* sz = nullptr, uintmax_t* nhl = nullptr, time_t* lwt = nullptr) noexcept
{
#ifdef GHC_OS_WINDOWS
    file_status fs;
    WIN32_FILE_ATTRIBUTE_DATA attr;
    if (!GetFileAttributesExW(detail::fromUtf8<std::wstring>(p.u8string()).c_str(), GetFileExInfoStandard, &attr)) {
        ec = detail::make_system_error();
    }
    else {
        ec.clear();
        fs = detail::status_from_INFO(p, &attr, ec, sz, lwt);
        if (nhl) {
            *nhl = 0;
        }
        if (attr.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
            fs.type(file_type::symlink);
        }
    }
    if (detail::is_not_found_error(ec)) {
        return file_status(file_type::not_found);
    }
    return ec ? file_status(file_type::none) : fs;
#else
    (void)sz;
    (void)nhl;
    (void)lwt;
    struct ::stat fs;
    auto result = ::lstat(p.c_str(), &fs);
    if (result == 0) {
        ec.clear();
        file_status f_s = detail::file_status_from_st_mode(fs.st_mode);
        return f_s;
    }
    ec = detail::make_system_error();
    if (detail::is_not_found_error(ec)) {
        return file_status(file_type::not_found, perms::unknown);
    }
    return file_status(file_type::none);
#endif
}

GHC_INLINE file_status status_ex(const path& p, std::error_code& ec, file_status* sls = nullptr, uintmax_t* sz = nullptr, uintmax_t* nhl = nullptr, time_t* lwt = nullptr, int recurse_count = 0) noexcept
{
    ec.clear();
#ifdef GHC_OS_WINDOWS
    if (recurse_count > 16) {
        ec = detail::make_system_error(0x2A9 /*ERROR_STOPPED_ON_SYMLINK*/);
        return file_status(file_type::unknown);
    }
    WIN32_FILE_ATTRIBUTE_DATA attr;
    if (!::GetFileAttributesExW(p.wstring().c_str(), GetFileExInfoStandard, &attr)) {
        ec = detail::make_system_error();
    }
    else if (attr.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
        path target = resolveSymlink(p, ec);
        file_status result;
        if (!ec && !target.empty()) {
            if (sls) {
                *sls = status_from_INFO(p, &attr, ec);
            }
            return detail::status_ex(target, ec, nullptr, sz, nhl, lwt, recurse_count + 1);
        }
        return file_status(file_type::unknown);
    }
    if (ec) {
        if (detail::is_not_found_error(ec)) {
            return file_status(file_type::not_found);
        }
        return file_status(file_type::none);
    }
    if (nhl) {
        *nhl = 0;
    }
    return detail::status_from_INFO(p, &attr, ec, sz, lwt);
#else
    (void)recurse_count;
    struct ::stat st;
    auto result = ::lstat(p.c_str(), &st);
    if (result == 0) {
        ec.clear();
        file_status fs = detail::file_status_from_st_mode(st.st_mode);
        if (fs.type() == file_type::symlink) {
            result = ::stat(p.c_str(), &st);
            if (result == 0) {
                if (sls) {
                    *sls = fs;
                }
                fs = detail::file_status_from_st_mode(st.st_mode);
            }
        }
        if (sz) {
            *sz = static_cast<uintmax_t>(st.st_size);
        }
        if (nhl) {
            *nhl = st.st_nlink;
        }
        if (lwt) {
            *lwt = st.st_mtime;
        }
        return fs;
    }
    else {
        ec = detail::make_system_error();
        if (detail::is_not_found_error(ec)) {
            return file_status(file_type::not_found, perms::unknown);
        }
        return file_status(file_type::none);
    }
#endif
}

}  // namespace detail

GHC_INLINE u8arguments::u8arguments(int& argc, char**& argv)
    : _argc(argc)
    , _argv(argv)
    , _refargc(argc)
    , _refargv(argv)
    , _isvalid(false)
{
#ifdef GHC_OS_WINDOWS
    LPWSTR* p;
    p = ::CommandLineToArgvW(::GetCommandLineW(), &argc);
    _args.reserve(static_cast<size_t>(argc));
    _argp.reserve(static_cast<size_t>(argc));
    for (size_t i = 0; i < static_cast<size_t>(argc); ++i) {
        _args.push_back(detail::toUtf8(std::wstring(p[i])));
        _argp.push_back((char*)_args[i].data());
    }
    argv = _argp.data();
    ::LocalFree(p);
    _isvalid = true;
#else
    std::setlocale(LC_ALL, "");
#if defined(__ANDROID__) && __ANDROID_API__ < 26
    _isvalid = true;
#else
    if (detail::equals_simple_insensitive(::nl_langinfo(CODESET), "UTF-8")) {
        _isvalid = true;
    }
#endif
#endif
}

//-----------------------------------------------------------------------------
// 30.10.8.4.1 constructors and destructor

GHC_INLINE path::path() noexcept {}

GHC_INLINE path::path(const path& p)
    : _path(p._path)
{
}

GHC_INLINE path::path(path&& p) noexcept
    : _path(std::move(p._path))
{
}

GHC_INLINE path::path(string_type&& source, format fmt)
#ifdef GHC_USE_WCHAR_T
    : _path(detail::toUtf8(source))
#else
    : _path(std::move(source))
#endif
{
    postprocess_path_with_format(_path, fmt);
}

#endif  // GHC_EXPAND_IMPL

#ifdef GHC_WITH_EXCEPTIONS
template <class Source, typename>
inline path::path(const Source& source, const std::locale& loc, format fmt)
    : path(source, fmt)
{
    std::string locName = loc.name();
    if (!(locName.length() >= 5 && (locName.substr(locName.length() - 5) == "UTF-8" || locName.substr(locName.length() - 5) == "utf-8"))) {
        throw filesystem_error("This implementation only supports UTF-8 locales!", path(_path), detail::make_error_code(detail::portable_error::not_supported));
    }
}

template <class InputIterator>
inline path::path(InputIterator first, InputIterator last, const std::locale& loc, format fmt)
    : path(std::basic_string<typename std::iterator_traits<InputIterator>::value_type>(first, last), fmt)
{
    std::string locName = loc.name();
    if (!(locName.length() >= 5 && (locName.substr(locName.length() - 5) == "UTF-8" || locName.substr(locName.length() - 5) == "utf-8"))) {
        throw filesystem_error("This implementation only supports UTF-8 locales!", path(_path), detail::make_error_code(detail::portable_error::not_supported));
    }
}
#endif

#ifdef GHC_EXPAND_IMPL

GHC_INLINE path::~path() {}

//-----------------------------------------------------------------------------
// 30.10.8.4.2 assignments

GHC_INLINE path& path::operator=(const path& p)
{
    _path = p._path;
    return *this;
}

GHC_INLINE path& path::operator=(path&& p) noexcept
{
    _path = std::move(p._path);
    return *this;
}

GHC_INLINE path& path::operator=(path::string_type&& source)
{
    return assign(source);
}

GHC_INLINE path& path::assign(path::string_type&& source)
{
#ifdef GHC_USE_WCHAR_T
    _path = detail::toUtf8(source);
#else
    _path = std::move(source);
#endif
    postprocess_path_with_format(_path, native_format);
    return *this;
}

#endif  // GHC_EXPAND_IMPL

template <class Source>
inline path& path::operator=(const Source& source)
{
    return assign(source);
}

template <class Source>
inline path& path::assign(const Source& source)
{
    _path.assign(detail::toUtf8(source));
    postprocess_path_with_format(_path, native_format);
    return *this;
}

template <>
inline path& path::assign<path>(const path& source)
{
    _path = source._path;
    return *this;
}

template <class InputIterator>
inline path& path::assign(InputIterator first, InputIterator last)
{
    _path.assign(first, last);
    postprocess_path_with_format(_path, native_format);
    return *this;
}

#ifdef GHC_EXPAND_IMPL

//-----------------------------------------------------------------------------
// 30.10.8.4.3 appends

GHC_INLINE path& path::operator/=(const path& p)
{
    if (p.empty()) {
        // was: if ((!has_root_directory() && is_absolute()) || has_filename())
        if (!_path.empty() && _path[_path.length() - 1] != '/' && _path[_path.length() - 1] != ':') {
            _path += '/';
        }
        return *this;
    }
    if ((p.is_absolute() && (_path != root_name() || p._path != "/")) || (p.has_root_name() && p.root_name() != root_name())) {
        assign(p);
        return *this;
    }
    if (p.has_root_directory()) {
        assign(root_name());
    }
    else if ((!has_root_directory() && is_absolute()) || has_filename()) {
        _path += '/';
    }
    auto iter = p.begin();
    bool first = true;
    if (p.has_root_name()) {
        ++iter;
    }
    while (iter != p.end()) {
        if (!first && !(!_path.empty() && _path[_path.length() - 1] == '/')) {
            _path += '/';
        }
        first = false;
        _path += (*iter++).generic_string();
    }
    return *this;
}

GHC_INLINE void path::append_name(const char* name)
{
    if (_path.empty()) {
        this->operator/=(path(name));
    }
    else {
        if (_path.back() != path::generic_separator) {
            _path.push_back(path::generic_separator);
        }
        _path += name;
    }
}

#endif  // GHC_EXPAND_IMPL

template <class Source>
inline path& path::operator/=(const Source& source)
{
    return append(source);
}

template <class Source>
inline path& path::append(const Source& source)
{
    return this->operator/=(path(detail::toUtf8(source)));
}

template <>
inline path& path::append<path>(const path& p)
{
    return this->operator/=(p);
}

template <class InputIterator>
inline path& path::append(InputIterator first, InputIterator last)
{
    std::basic_string<typename std::iterator_traits<InputIterator>::value_type> part(first, last);
    return append(part);
}

#ifdef GHC_EXPAND_IMPL

//-----------------------------------------------------------------------------
// 30.10.8.4.4 concatenation

GHC_INLINE path& path::operator+=(const path& x)
{
    return concat(x._path);
}

GHC_INLINE path& path::operator+=(const string_type& x)
{
    return concat(x);
}

#ifdef __cpp_lib_string_view
GHC_INLINE path& path::operator+=(std::basic_string_view<value_type> x)
{
    return concat(x);
}
#endif

GHC_INLINE path& path::operator+=(const value_type* x)
{
    return concat(string_type(x));
}

GHC_INLINE path& path::operator+=(value_type x)
{
#ifdef GHC_OS_WINDOWS
    if (x == '\\') {
        x = generic_separator;
    }
#endif
    if (_path.empty() || _path.back() != generic_separator) {
#ifdef GHC_USE_WCHAR_T
        _path += detail::toUtf8(string_type(1, x));
#else
        _path += x;
#endif
    }
    return *this;
}

#endif  // GHC_EXPAND_IMPL

template <class Source>
inline path::path_from_string<Source>& path::operator+=(const Source& x)
{
    return concat(x);
}

template <class EcharT>
inline path::path_type_EcharT<EcharT>& path::operator+=(EcharT x)
{
    std::basic_string<EcharT> part(1, x);
    concat(detail::toUtf8(part));
    return *this;
}

template <class Source>
inline path& path::concat(const Source& x)
{
    path p(x);
    postprocess_path_with_format(p._path, native_format);
    _path += p._path;
    return *this;
}
template <class InputIterator>
inline path& path::concat(InputIterator first, InputIterator last)
{
    _path.append(first, last);
    postprocess_path_with_format(_path, native_format);
    return *this;
}

#ifdef GHC_EXPAND_IMPL

//-----------------------------------------------------------------------------
// 30.10.8.4.5 modifiers
GHC_INLINE void path::clear() noexcept
{
    _path.clear();
}

GHC_INLINE path& path::make_preferred()
{
    // as this filesystem implementation only uses generic_format
    // internally, this must be a no-op
    return *this;
}

GHC_INLINE path& path::remove_filename()
{
    if (has_filename()) {
        _path.erase(_path.size() - filename()._path.size());
    }
    return *this;
}

GHC_INLINE path& path::replace_filename(const path& replacement)
{
    remove_filename();
    return append(replacement);
}

GHC_INLINE path& path::replace_extension(const path& replacement)
{
    if (has_extension()) {
        _path.erase(_path.size() - extension()._path.size());
    }
    if (!replacement.empty() && replacement._path[0] != '.') {
        _path += '.';
    }
    return concat(replacement);
}

GHC_INLINE void path::swap(path& rhs) noexcept
{
    _path.swap(rhs._path);
}

//-----------------------------------------------------------------------------
// 30.10.8.4.6, native format observers
#ifdef GHC_OS_WINDOWS
GHC_INLINE path::impl_string_type path::native_impl() const
{
    impl_string_type result;
    if (is_absolute() && _path.length() > MAX_PATH - 10) {
        // expand long Windows filenames with marker
        if (has_root_name() && _path[0] == '/') {
            result = "\\\\?\\UNC" + _path.substr(1);
        }
        else {
            result = "\\\\?\\" + _path;
        }
    }
    else {
        result = _path;
    }
    /*if (has_root_name() && root_name()._path[0] == '/') {
        return _path;
    }*/
    for (auto& c : result) {
        if (c == '/') {
            c = '\\';
        }
    }
    return result;
}
#else
GHC_INLINE const path::impl_string_type& path::native_impl() const
{
    return _path;
}
#endif

GHC_INLINE const path::string_type& path::native() const
{
#ifdef GHC_OS_WINDOWS
#ifdef GHC_USE_WCHAR_T
    _native_cache = detail::fromUtf8<string_type>(native_impl());
#else
    _native_cache = native_impl();
#endif
    return _native_cache;
#else
    return _path;
#endif
}

GHC_INLINE const path::value_type* path::c_str() const
{
    return native().c_str();
}

GHC_INLINE path::operator path::string_type() const
{
    return native();
}

#endif  // GHC_EXPAND_IMPL

template <class EcharT, class traits, class Allocator>
inline std::basic_string<EcharT, traits, Allocator> path::string(const Allocator& a) const
{
    return detail::fromUtf8<std::basic_string<EcharT, traits, Allocator>>(native_impl(), a);
}

#ifdef GHC_EXPAND_IMPL

GHC_INLINE std::string path::string() const
{
    return native_impl();
}

GHC_INLINE std::wstring path::wstring() const
{
#ifdef GHC_USE_WCHAR_T
    return native();
#else
    return detail::fromUtf8<std::wstring>(native());
#endif
}

GHC_INLINE std::string path::u8string() const
{
    return native_impl();
}

GHC_INLINE std::u16string path::u16string() const
{
    return detail::fromUtf8<std::u16string>(native_impl());
}

GHC_INLINE std::u32string path::u32string() const
{
    return detail::fromUtf8<std::u32string>(native_impl());
}

#endif  // GHC_EXPAND_IMPL

//-----------------------------------------------------------------------------
// 30.10.8.4.7, generic format observers
template <class EcharT, class traits, class Allocator>
inline std::basic_string<EcharT, traits, Allocator> path::generic_string(const Allocator& a) const
{
    return detail::fromUtf8<std::basic_string<EcharT, traits, Allocator>>(_path, a);
}

#ifdef GHC_EXPAND_IMPL

GHC_INLINE const std::string& path::generic_string() const
{
    return _path;
}

GHC_INLINE std::wstring path::generic_wstring() const
{
    return detail::fromUtf8<std::wstring>(_path);
}

GHC_INLINE std::string path::generic_u8string() const
{
    return _path;
}

GHC_INLINE std::u16string path::generic_u16string() const
{
    return detail::fromUtf8<std::u16string>(_path);
}

GHC_INLINE std::u32string path::generic_u32string() const
{
    return detail::fromUtf8<std::u32string>(_path);
}

//-----------------------------------------------------------------------------
// 30.10.8.4.8, compare
GHC_INLINE int path::compare(const path& p) const noexcept
{
    return native().compare(p.native());
}

GHC_INLINE int path::compare(const string_type& s) const
{
    return native().compare(path(s).native());
}

#ifdef __cpp_lib_string_view
GHC_INLINE int path::compare(std::basic_string_view<value_type> s) const
{
    return native().compare(path(s).native());
}
#endif

GHC_INLINE int path::compare(const value_type* s) const
{
    return native().compare(path(s).native());
}

//-----------------------------------------------------------------------------
// 30.10.8.4.9, decomposition
GHC_INLINE path path::root_name() const
{
#ifdef GHC_OS_WINDOWS
    if (_path.length() >= 2 && std::toupper(static_cast<unsigned char>(_path[0])) >= 'A' && std::toupper(static_cast<unsigned char>(_path[0])) <= 'Z' && _path[1] == ':') {
        return path(_path.substr(0, 2));
    }
#endif
    if (_path.length() > 2 && _path[0] == '/' && _path[1] == '/' && _path[2] != '/' && std::isprint(_path[2])) {
        impl_string_type::size_type pos = _path.find_first_of("/\\", 3);
        if (pos == impl_string_type::npos) {
            return path(_path);
        }
        else {
            return path(_path.substr(0, pos));
        }
    }
    return path();
}

GHC_INLINE path path::root_directory() const
{
    path root = root_name();
    if (_path.length() > root._path.length() && _path[root._path.length()] == '/') {
        return path("/");
    }
    return path();
}

GHC_INLINE path path::root_path() const
{
    return root_name().generic_string() + root_directory().generic_string();
}

GHC_INLINE path path::relative_path() const
{
    std::string root = root_path()._path;
    return path(_path.substr((std::min)(root.length(), _path.length())), generic_format);
}

GHC_INLINE path path::parent_path() const
{
    if (has_relative_path()) {
        if (empty() || begin() == --end()) {
            return path();
        }
        else {
            path pp;
            for (string_type s : input_iterator_range<iterator>(begin(), --end())) {
                if (s == "/") {
                    // don't use append to join a path-
                    pp += s;
                }
                else {
                    pp /= s;
                }
            }
            return pp;
        }
    }
    else {
        return *this;
    }
}

GHC_INLINE path path::filename() const
{
    return relative_path().empty() ? path() : path(*--end());
}

GHC_INLINE path path::stem() const
{
    impl_string_type fn = filename().string();
    if (fn != "." && fn != "..") {
        impl_string_type::size_type n = fn.rfind('.');
        if (n != impl_string_type::npos && n != 0) {
            return path{fn.substr(0, n)};
        }
    }
    return path{fn};
}

GHC_INLINE path path::extension() const
{
    impl_string_type fn = filename().string();
    impl_string_type::size_type pos = fn.find_last_of('.');
    if (pos == std::string::npos || pos == 0) {
        return "";
    }
    return fn.substr(pos);
}

//-----------------------------------------------------------------------------
// 30.10.8.4.10, query
GHC_INLINE bool path::empty() const noexcept
{
    return _path.empty();
}

GHC_INLINE bool path::has_root_name() const
{
    return !root_name().empty();
}

GHC_INLINE bool path::has_root_directory() const
{
    return !root_directory().empty();
}

GHC_INLINE bool path::has_root_path() const
{
    return !root_path().empty();
}

GHC_INLINE bool path::has_relative_path() const
{
    return !relative_path().empty();
}

GHC_INLINE bool path::has_parent_path() const
{
    return !parent_path().empty();
}

GHC_INLINE bool path::has_filename() const
{
    return !filename().empty();
}

GHC_INLINE bool path::has_stem() const
{
    return !stem().empty();
}

GHC_INLINE bool path::has_extension() const
{
    return !extension().empty();
}

GHC_INLINE bool path::is_absolute() const
{
#ifdef GHC_OS_WINDOWS
    return has_root_name() && has_root_directory();
#else
    return has_root_directory();
#endif
}

GHC_INLINE bool path::is_relative() const
{
    return !is_absolute();
}

//-----------------------------------------------------------------------------
// 30.10.8.4.11, generation
GHC_INLINE path path::lexically_normal() const
{
    path dest;
    bool lastDotDot = false;
    for (string_type s : *this) {
        if (s == ".") {
            dest /= "";
            continue;
        }
        else if (s == ".." && !dest.empty()) {
            auto root = root_path();
            if (dest == root) {
                continue;
            }
            else if (*(--dest.end()) != "..") {
                if (dest._path.back() == generic_separator) {
                    dest._path.pop_back();
                }
                dest.remove_filename();
                continue;
            }
        }
        if (!(s.empty() && lastDotDot)) {
            dest /= s;
        }
        lastDotDot = s == "..";
    }
    if (dest.empty()) {
        dest = ".";
    }
    return dest;
}

GHC_INLINE path path::lexically_relative(const path& base) const
{
    if (root_name() != base.root_name() || is_absolute() != base.is_absolute() || (!has_root_directory() && base.has_root_directory())) {
        return path();
    }
    const_iterator a = begin(), b = base.begin();
    while (a != end() && b != base.end() && *a == *b) {
        ++a;
        ++b;
    }
    if (a == end() && b == base.end()) {
        return path(".");
    }
    int count = 0;
    for (const auto& element : input_iterator_range<const_iterator>(b, base.end())) {
        if (element != "." && element != "" && element != "..") {
            ++count;
        }
        else if (element == "..") {
            --count;
        }
    }
    if (count < 0) {
        return path();
    }
    path result;
    for (int i = 0; i < count; ++i) {
        result /= "..";
    }
    for (const auto& element : input_iterator_range<const_iterator>(a, end())) {
        result /= element;
    }
    return result;
}

GHC_INLINE path path::lexically_proximate(const path& base) const
{
    path result = lexically_relative(base);
    return result.empty() ? *this : result;
}

//-----------------------------------------------------------------------------
// 30.10.8.5, iterators
GHC_INLINE path::iterator::iterator() {}

GHC_INLINE path::iterator::iterator(const path::impl_string_type::const_iterator& first, const path::impl_string_type::const_iterator& last, const path::impl_string_type::const_iterator& pos)
    : _first(first)
    , _last(last)
    , _iter(pos)
{
    updateCurrent();
    // find the position of a potential root directory slash
#ifdef GHC_OS_WINDOWS
    if (_last - _first >= 3 && std::toupper(static_cast<unsigned char>(*first)) >= 'A' && std::toupper(static_cast<unsigned char>(*first)) <= 'Z' && *(first + 1) == ':' && *(first + 2) == '/') {
        _root = _first + 2;
    }
    else
#endif
    {
        if (_first != _last && *_first == '/') {
            if (_last - _first >= 2 && *(_first + 1) == '/' && !(_last - _first >= 3 && *(_first + 2) == '/')) {
                _root = increment(_first);
            }
            else {
                _root = _first;
            }
        }
        else {
            _root = _last;
        }
    }
}

GHC_INLINE path::impl_string_type::const_iterator path::iterator::increment(const path::impl_string_type::const_iterator& pos) const
{
    path::impl_string_type::const_iterator i = pos;
    bool fromStart = i == _first;
    if (i != _last) {
        // we can only sit on a slash if it is a network name or a root
        if (*i++ == '/') {
            if (i != _last && *i == '/') {
                if (fromStart && !(i + 1 != _last && *(i + 1) == '/')) {
                    // leadind double slashes detected, treat this and the
                    // following until a slash as one unit
                    i = std::find(++i, _last, '/');
                }
                else {
                    // skip redundant slashes
                    while (i != _last && *i == '/') {
                        ++i;
                    }
                }
            }
        }
        else {
            if (fromStart && i != _last && *i == ':') {
                ++i;
            }
            else {
                i = std::find(i, _last, '/');
            }
        }
    }
    return i;
}

GHC_INLINE path::impl_string_type::const_iterator path::iterator::decrement(const path::impl_string_type::const_iterator& pos) const
{
    path::impl_string_type::const_iterator i = pos;
    if (i != _first) {
        --i;
        // if this is now the root slash or the trailing slash, we are done,
        // else check for network name
        if (i != _root && (pos != _last || *i != '/')) {
#ifdef GHC_OS_WINDOWS
            static const std::string seps = "/:";
            i = std::find_first_of(std::reverse_iterator<path::impl_string_type::const_iterator>(i), std::reverse_iterator<path::impl_string_type::const_iterator>(_first), seps.begin(), seps.end()).base();
            if (i > _first && *i == ':') {
                i++;
            }
#else
            i = std::find(std::reverse_iterator<path::impl_string_type::const_iterator>(i), std::reverse_iterator<path::impl_string_type::const_iterator>(_first), '/').base();
#endif
            // Now we have to check if this is a network name
            if (i - _first == 2 && *_first == '/' && *(_first + 1) == '/') {
                i -= 2;
            }
        }
    }
    return i;
}

GHC_INLINE void path::iterator::updateCurrent()
{
    if (_iter != _first && _iter != _last && (*_iter == '/' && _iter != _root) && (_iter + 1 == _last)) {
        _current = "";
    }
    else {
        _current.assign(_iter, increment(_iter));
        if (_current.generic_string().size() > 1 && _current.generic_string()[0] == '/' && _current.generic_string()[_current.generic_string().size() - 1] == '/') {
            // shrink successive slashes to one
            _current = "/";
        }
    }
}

GHC_INLINE path::iterator& path::iterator::operator++()
{
    _iter = increment(_iter);
    while (_iter != _last &&     // we didn't reach the end
           _iter != _root &&     // this is not a root position
           *_iter == '/' &&      // we are on a slash
           (_iter + 1) != _last  // the slash is not the last char
    ) {
        ++_iter;
    }
    updateCurrent();
    return *this;
}

GHC_INLINE path::iterator path::iterator::operator++(int)
{
    path::iterator i{*this};
    ++(*this);
    return i;
}

GHC_INLINE path::iterator& path::iterator::operator--()
{
    _iter = decrement(_iter);
    updateCurrent();
    return *this;
}

GHC_INLINE path::iterator path::iterator::operator--(int)
{
    auto i = *this;
    --(*this);
    return i;
}

GHC_INLINE bool path::iterator::operator==(const path::iterator& other) const
{
    return _iter == other._iter;
}

GHC_INLINE bool path::iterator::operator!=(const path::iterator& other) const
{
    return _iter != other._iter;
}

GHC_INLINE path::iterator::reference path::iterator::operator*() const
{
    return _current;
}

GHC_INLINE path::iterator::pointer path::iterator::operator->() const
{
    return &_current;
}

GHC_INLINE path::iterator path::begin() const
{
    return iterator(_path.begin(), _path.end(), _path.begin());
}

GHC_INLINE path::iterator path::end() const
{
    return iterator(_path.begin(), _path.end(), _path.end());
}

//-----------------------------------------------------------------------------
// 30.10.8.6, path non-member functions
GHC_INLINE void swap(path& lhs, path& rhs) noexcept
{
    swap(lhs._path, rhs._path);
}

GHC_INLINE size_t hash_value(const path& p) noexcept
{
    return std::hash<std::string>()(p.generic_string());
}

GHC_INLINE bool operator==(const path& lhs, const path& rhs) noexcept
{
    return lhs.generic_string() == rhs.generic_string();
}

GHC_INLINE bool operator!=(const path& lhs, const path& rhs) noexcept
{
    return lhs.generic_string() != rhs.generic_string();
}

GHC_INLINE bool operator<(const path& lhs, const path& rhs) noexcept
{
    return lhs.generic_string() < rhs.generic_string();
}

GHC_INLINE bool operator<=(const path& lhs, const path& rhs) noexcept
{
    return lhs.generic_string() <= rhs.generic_string();
}

GHC_INLINE bool operator>(const path& lhs, const path& rhs) noexcept
{
    return lhs.generic_string() > rhs.generic_string();
}

GHC_INLINE bool operator>=(const path& lhs, const path& rhs) noexcept
{
    return lhs.generic_string() >= rhs.generic_string();
}

GHC_INLINE path operator/(const path& lhs, const path& rhs)
{
    path result(lhs);
    result /= rhs;
    return result;
}

#endif  // GHC_EXPAND_IMPL

//-----------------------------------------------------------------------------
// 30.10.8.6.1 path inserter and extractor
template <class charT, class traits>
inline std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& os, const path& p)
{
    os << "\"";
    auto ps = p.string<charT, traits>();
    for (auto c : ps) {
        if (c == '"' || c == '\\') {
            os << '\\';
        }
        os << c;
    }
    os << "\"";
    return os;
}

template <class charT, class traits>
inline std::basic_istream<charT, traits>& operator>>(std::basic_istream<charT, traits>& is, path& p)
{
    std::basic_string<charT, traits> tmp;
    charT c;
    is >> c;
    if (c == '"') {
        auto sf = is.flags();
        is >> std::noskipws;
        while (is) {
            auto c2 = is.get();
            if (is) {
                if (c2 == '\\') {
                    c2 = is.get();
                    if (is) {
                        tmp += static_cast<charT>(c2);
                    }
                }
                else if (c2 == '"') {
                    break;
                }
                else {
                    tmp += static_cast<charT>(c2);
                }
            }
        }
        if ((sf & std::ios_base::skipws) == std::ios_base::skipws) {
            is >> std::skipws;
        }
        p = path(tmp);
    }
    else {
        is >> tmp;
        p = path(static_cast<charT>(c) + tmp);
    }
    return is;
}

#ifdef GHC_EXPAND_IMPL

//-----------------------------------------------------------------------------
// 30.10.9 Class filesystem_error
GHC_INLINE filesystem_error::filesystem_error(const std::string& what_arg, std::error_code ec)
    : std::system_error(ec, what_arg)
    , _what_arg(what_arg)
    , _ec(ec)
{
}

GHC_INLINE filesystem_error::filesystem_error(const std::string& what_arg, const path& p1, std::error_code ec)
    : std::system_error(ec, what_arg)
    , _what_arg(what_arg)
    , _ec(ec)
    , _p1(p1)
{
    if (!_p1.empty()) {
        _what_arg += ": '" + _p1.u8string() + "'";
    }
}

GHC_INLINE filesystem_error::filesystem_error(const std::string& what_arg, const path& p1, const path& p2, std::error_code ec)
    : std::system_error(ec, what_arg)
    , _what_arg(what_arg)
    , _ec(ec)
    , _p1(p1)
    , _p2(p2)
{
    if (!_p1.empty()) {
        _what_arg += ": '" + _p1.u8string() + "'";
    }
    if (!_p2.empty()) {
        _what_arg += ", '" + _p2.u8string() + "'";
    }
}

GHC_INLINE const path& filesystem_error::path1() const noexcept
{
    return _p1;
}

GHC_INLINE const path& filesystem_error::path2() const noexcept
{
    return _p2;
}

GHC_INLINE const char* filesystem_error::what() const noexcept
{
    return _what_arg.c_str();
}

//-----------------------------------------------------------------------------
// 30.10.15, filesystem operations
#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE path absolute(const path& p)
{
    std::error_code ec;
    path result = absolute(p, ec);
    if (ec) {
        throw filesystem_error(detail::systemErrorText(ec.value()), p, ec);
    }
    return result;
}
#endif

GHC_INLINE path absolute(const path& p, std::error_code& ec)
{
    ec.clear();
#ifdef GHC_OS_WINDOWS
    if (p.empty()) {
        return absolute(current_path(ec), ec) / "";
    }
    ULONG size = ::GetFullPathNameW(p.wstring().c_str(), 0, 0, 0);
    if (size) {
        std::vector<wchar_t> buf(size, 0);
        ULONG s2 = GetFullPathNameW(p.wstring().c_str(), size, buf.data(), nullptr);
        if (s2 && s2 < size) {
            path result = path(std::wstring(buf.data(), s2));
            if (p.filename() == ".") {
                result /= ".";
            }
            return result;
        }
    }
    ec = detail::make_system_error();
    return path();
#else
    path base = current_path(ec);
    if (!ec) {
        if (p.empty()) {
            return base / p;
        }
        if (p.has_root_name()) {
            if (p.has_root_directory()) {
                return p;
            }
            else {
                return p.root_name() / base.root_directory() / base.relative_path() / p.relative_path();
            }
        }
        else {
            if (p.has_root_directory()) {
                return base.root_name() / p;
            }
            else {
                return base / p;
            }
        }
    }
    ec = detail::make_system_error();
    return path();
#endif
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE path canonical(const path& p)
{
    std::error_code ec;
    auto result = canonical(p, ec);
    if (ec) {
        throw filesystem_error(detail::systemErrorText(ec.value()), p, ec);
    }
    return result;
}
#endif

GHC_INLINE path canonical(const path& p, std::error_code& ec)
{
    if (p.empty()) {
        ec = detail::make_error_code(detail::portable_error::not_found);
        return path();
    }
    path work = p.is_absolute() ? p : absolute(p, ec);
    path root = work.root_path();
    path result;

    auto fs = status(work, ec);
    if (ec) {
        return path();
    }
    if (fs.type() == file_type::not_found) {
        ec = detail::make_error_code(detail::portable_error::not_found);
        return path();
    }
    bool redo;
    do {
        redo = false;
        result.clear();
        for (auto pe : work) {
            if (pe.empty() || pe == ".") {
                continue;
            }
            else if (pe == "..") {
                result = result.parent_path();
                continue;
            }
            else if ((result / pe).string().length() <= root.string().length()) {
                result /= pe;
                continue;
            }
            auto sls = symlink_status(result / pe, ec);
            if (ec) {
                return path();
            }
            if (is_symlink(sls)) {
                redo = true;
                auto target = read_symlink(result / pe, ec);
                if (ec) {
                    return path();
                }
                if (target.is_absolute()) {
                    result = target;
                    continue;
                }
                else {
                    result /= target;
                    continue;
                }
            }
            else {
                result /= pe;
            }
        }
        work = result;
    } while (redo);
    ec.clear();
    return result;
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE void copy(const path& from, const path& to)
{
    copy(from, to, copy_options::none);
}
#endif

GHC_INLINE void copy(const path& from, const path& to, std::error_code& ec) noexcept
{
    copy(from, to, copy_options::none, ec);
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE void copy(const path& from, const path& to, copy_options options)
{
    std::error_code ec;
    copy(from, to, options, ec);
    if (ec) {
        throw filesystem_error(detail::systemErrorText(ec.value()), from, to, ec);
    }
}
#endif

GHC_INLINE void copy(const path& from, const path& to, copy_options options, std::error_code& ec) noexcept
{
    std::error_code tec;
    file_status fs_from, fs_to;
    ec.clear();
    if ((options & (copy_options::skip_symlinks | copy_options::copy_symlinks | copy_options::create_symlinks)) != copy_options::none) {
        fs_from = symlink_status(from, ec);
    }
    else {
        fs_from = status(from, ec);
    }
    if (!exists(fs_from)) {
        if (!ec) {
            ec = detail::make_error_code(detail::portable_error::not_found);
        }
        return;
    }
    if ((options & (copy_options::skip_symlinks | copy_options::create_symlinks)) != copy_options::none) {
        fs_to = symlink_status(to, tec);
    }
    else {
        fs_to = status(to, tec);
    }
    if (is_other(fs_from) || is_other(fs_to) || (is_directory(fs_from) && is_regular_file(fs_to)) || (exists(fs_to) && equivalent(from, to, ec))) {
        ec = detail::make_error_code(detail::portable_error::invalid_argument);
    }
    else if (is_symlink(fs_from)) {
        if ((options & copy_options::skip_symlinks) == copy_options::none) {
            if (!exists(fs_to) && (options & copy_options::copy_symlinks) != copy_options::none) {
                copy_symlink(from, to, ec);
            }
            else {
                ec = detail::make_error_code(detail::portable_error::invalid_argument);
            }
        }
    }
    else if (is_regular_file(fs_from)) {
        if ((options & copy_options::directories_only) == copy_options::none) {
            if ((options & copy_options::create_symlinks) != copy_options::none) {
                create_symlink(from.is_absolute() ? from : canonical(from, ec), to, ec);
            }
            else if ((options & copy_options::create_hard_links) != copy_options::none) {
                create_hard_link(from, to, ec);
            }
            else if (is_directory(fs_to)) {
                copy_file(from, to / from.filename(), options, ec);
            }
            else {
                copy_file(from, to, options, ec);
            }
        }
    }
#ifdef LWG_2682_BEHAVIOUR
    else if (is_directory(fs_from) && (options & copy_options::create_symlinks) != copy_options::none) {
        ec = detail::make_error_code(detail::portable_error::is_a_directory);
    }
#endif
    else if (is_directory(fs_from) && (options == copy_options::none || (options & copy_options::recursive) != copy_options::none)) {
        if (!exists(fs_to)) {
            create_directory(to, from, ec);
            if (ec) {
                return;
            }
        }
        for (auto iter = directory_iterator(from, ec); iter != directory_iterator(); iter.increment(ec)) {
            if (!ec) {
                copy(iter->path(), to / iter->path().filename(), options | static_cast<copy_options>(0x8000), ec);
            }
            if (ec) {
                return;
            }
        }
    }
    return;
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE bool copy_file(const path& from, const path& to)
{
    return copy_file(from, to, copy_options::none);
}
#endif

GHC_INLINE bool copy_file(const path& from, const path& to, std::error_code& ec) noexcept
{
    return copy_file(from, to, copy_options::none, ec);
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE bool copy_file(const path& from, const path& to, copy_options option)
{
    std::error_code ec;
    auto result = copy_file(from, to, option, ec);
    if (ec) {
        throw filesystem_error(detail::systemErrorText(ec.value()), from, to, ec);
    }
    return result;
}
#endif

GHC_INLINE bool copy_file(const path& from, const path& to, copy_options options, std::error_code& ec) noexcept
{
    std::error_code tecf, tect;
    auto sf = status(from, tecf);
    auto st = status(to, tect);
    bool overwrite = false;
    ec.clear();
    if (!is_regular_file(sf)) {
        ec = tecf;
        return false;
    }
    if (exists(st) && (!is_regular_file(st) || equivalent(from, to, ec) || (options & (copy_options::skip_existing | copy_options::overwrite_existing | copy_options::update_existing)) == copy_options::none)) {
        ec = tect ? tect : detail::make_error_code(detail::portable_error::exists);
        return false;
    }
    if (exists(st)) {
        if ((options & copy_options::update_existing) == copy_options::update_existing) {
            auto from_time = last_write_time(from, ec);
            if (ec) {
                ec = detail::make_system_error();
                return false;
            }
            auto to_time = last_write_time(to, ec);
            if (ec) {
                ec = detail::make_system_error();
                return false;
            }
            if (from_time <= to_time) {
                return false;
            }
        }
        overwrite = true;
    }
#ifdef GHC_OS_WINDOWS
    if (!::CopyFileW(detail::fromUtf8<std::wstring>(from.u8string()).c_str(), detail::fromUtf8<std::wstring>(to.u8string()).c_str(), !overwrite)) {
        ec = detail::make_system_error();
        return false;
    }
    return true;
#else
    std::vector<char> buffer(16384, '\0');
    int in = -1, out = -1;
    if ((in = ::open(from.c_str(), O_RDONLY)) < 0) {
        ec = detail::make_system_error();
        return false;
    }
    std::shared_ptr<void> guard_in(nullptr, [in](void*) { ::close(in); });
    int mode = O_CREAT | O_WRONLY | O_TRUNC;
    if (!overwrite) {
        mode |= O_EXCL;
    }
    if ((out = ::open(to.c_str(), mode, static_cast<int>(sf.permissions() & perms::all))) < 0) {
        ec = detail::make_system_error();
        return false;
    }
    std::shared_ptr<void> guard_out(nullptr, [out](void*) { ::close(out); });
    ssize_t br, bw;
    while ((br = ::read(in, buffer.data(), buffer.size())) > 0) {
        ssize_t offset = 0;
        do {
            if ((bw = ::write(out, buffer.data() + offset, static_cast<size_t>(br))) > 0) {
                br -= bw;
                offset += bw;
            }
            else if (bw < 0) {
                ec = detail::make_system_error();
                return false;
            }
        } while (br);
    }
    return true;
#endif
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE void copy_symlink(const path& existing_symlink, const path& new_symlink)
{
    std::error_code ec;
    copy_symlink(existing_symlink, new_symlink, ec);
    if (ec) {
        throw filesystem_error(detail::systemErrorText(ec.value()), existing_symlink, new_symlink, ec);
    }
}
#endif

GHC_INLINE void copy_symlink(const path& existing_symlink, const path& new_symlink, std::error_code& ec) noexcept
{
    ec.clear();
    auto to = read_symlink(existing_symlink, ec);
    if (!ec) {
        if (exists(to, ec) && is_directory(to, ec)) {
            create_directory_symlink(to, new_symlink, ec);
        }
        else {
            create_symlink(to, new_symlink, ec);
        }
    }
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE bool create_directories(const path& p)
{
    std::error_code ec;
    auto result = create_directories(p, ec);
    if (ec) {
        throw filesystem_error(detail::systemErrorText(ec.value()), p, ec);
    }
    return result;
}
#endif

GHC_INLINE bool create_directories(const path& p, std::error_code& ec) noexcept
{
    path current;
    ec.clear();
    bool didCreate = false;
    for (path::string_type part : p) {
        current /= part;
        if (current != p.root_name() && current != p.root_path()) {
            std::error_code tec;
            auto fs = status(current, tec);
            if (tec && fs.type() != file_type::not_found) {
                ec = tec;
                return false;
            }
            if (!exists(fs)) {
                create_directory(current, ec);
                if (ec) {
                    std::error_code tmp_ec;
                    if (is_directory(current, tmp_ec)) {
                        ec.clear();
                    } else {
                        return false;
                    }
                }
                didCreate = true;
            }
#ifndef LWG_2935_BEHAVIOUR
            else if (!is_directory(fs)) {
                ec = detail::make_error_code(detail::portable_error::exists);
                return false;
            }
#endif
        }
    }
    return didCreate;
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE bool create_directory(const path& p)
{
    std::error_code ec;
    auto result = create_directory(p, path(), ec);
    if (ec) {
        throw filesystem_error(detail::systemErrorText(ec.value()), p, ec);
    }
    return result;
}
#endif

GHC_INLINE bool create_directory(const path& p, std::error_code& ec) noexcept
{
    return create_directory(p, path(), ec);
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE bool create_directory(const path& p, const path& attributes)
{
    std::error_code ec;
    auto result = create_directory(p, attributes, ec);
    if (ec) {
        throw filesystem_error(detail::systemErrorText(ec.value()), p, ec);
    }
    return result;
}
#endif

GHC_INLINE bool create_directory(const path& p, const path& attributes, std::error_code& ec) noexcept
{
    std::error_code tec;
    ec.clear();
    auto fs = status(p, tec);
#ifdef LWG_2935_BEHAVIOUR
    if (status_known(fs) && exists(fs)) {
        return false;
    }
#else
    if (status_known(fs) && exists(fs) && is_directory(fs)) {
        return false;
    }
#endif
#ifdef GHC_OS_WINDOWS
    if (!attributes.empty()) {
        if (!::CreateDirectoryExW(detail::fromUtf8<std::wstring>(attributes.u8string()).c_str(), detail::fromUtf8<std::wstring>(p.u8string()).c_str(), NULL)) {
            ec = detail::make_system_error();
            return false;
        }
    }
    else if (!::CreateDirectoryW(detail::fromUtf8<std::wstring>(p.u8string()).c_str(), NULL)) {
        ec = detail::make_system_error();
        return false;
    }
#else
    ::mode_t attribs = static_cast<mode_t>(perms::all);
    if (!attributes.empty()) {
        struct ::stat fileStat;
        if (::stat(attributes.c_str(), &fileStat) != 0) {
            ec = detail::make_system_error();
            return false;
        }
        attribs = fileStat.st_mode;
    }
    if (::mkdir(p.c_str(), attribs) != 0) {
        ec = detail::make_system_error();
        return false;
    }
#endif
    return true;
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE void create_directory_symlink(const path& to, const path& new_symlink)
{
    std::error_code ec;
    create_directory_symlink(to, new_symlink, ec);
    if (ec) {
        throw filesystem_error(detail::systemErrorText(ec.value()), to, new_symlink, ec);
    }
}
#endif

GHC_INLINE void create_directory_symlink(const path& to, const path& new_symlink, std::error_code& ec) noexcept
{
    detail::create_symlink(to, new_symlink, true, ec);
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE void create_hard_link(const path& to, const path& new_hard_link)
{
    std::error_code ec;
    create_hard_link(to, new_hard_link, ec);
    if (ec) {
        throw filesystem_error(detail::systemErrorText(ec.value()), to, new_hard_link, ec);
    }
}
#endif

GHC_INLINE void create_hard_link(const path& to, const path& new_hard_link, std::error_code& ec) noexcept
{
    detail::create_hardlink(to, new_hard_link, ec);
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE void create_symlink(const path& to, const path& new_symlink)
{
    std::error_code ec;
    create_symlink(to, new_symlink, ec);
    if (ec) {
        throw filesystem_error(detail::systemErrorText(ec.value()), to, new_symlink, ec);
    }
}
#endif

GHC_INLINE void create_symlink(const path& to, const path& new_symlink, std::error_code& ec) noexcept
{
    detail::create_symlink(to, new_symlink, false, ec);
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE path current_path()
{
    std::error_code ec;
    auto result = current_path(ec);
    if (ec) {
        throw filesystem_error(detail::systemErrorText(ec.value()), ec);
    }
    return result;
}
#endif

GHC_INLINE path current_path(std::error_code& ec)
{
    ec.clear();
#ifdef GHC_OS_WINDOWS
    DWORD pathlen = ::GetCurrentDirectoryW(0, 0);
    std::unique_ptr<wchar_t[]> buffer(new wchar_t[size_t(pathlen) + 1]);
    if (::GetCurrentDirectoryW(pathlen, buffer.get()) == 0) {
        ec = detail::make_system_error();
        return path();
    }
    return path(std::wstring(buffer.get()), path::native_format);
#else
    size_t pathlen = static_cast<size_t>(std::max(int(::pathconf(".", _PC_PATH_MAX)), int(PATH_MAX)));
    std::unique_ptr<char[]> buffer(new char[pathlen + 1]);
    if (::getcwd(buffer.get(), pathlen) == nullptr) {
        ec = detail::make_system_error();
        return path();
    }
    return path(buffer.get());
#endif
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE void current_path(const path& p)
{
    std::error_code ec;
    current_path(p, ec);
    if (ec) {
        throw filesystem_error(detail::systemErrorText(ec.value()), p, ec);
    }
}
#endif

GHC_INLINE void current_path(const path& p, std::error_code& ec) noexcept
{
    ec.clear();
#ifdef GHC_OS_WINDOWS
    if (!::SetCurrentDirectoryW(detail::fromUtf8<std::wstring>(p.u8string()).c_str())) {
        ec = detail::make_system_error();
    }
#else
    if (::chdir(p.string().c_str()) == -1) {
        ec = detail::make_system_error();
    }
#endif
}

GHC_INLINE bool exists(file_status s) noexcept
{
    return status_known(s) && s.type() != file_type::not_found;
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE bool exists(const path& p)
{
    return exists(status(p));
}
#endif

GHC_INLINE bool exists(const path& p, std::error_code& ec) noexcept
{
    file_status s = status(p, ec);
    if (status_known(s)) {
        ec.clear();
    }
    return exists(s);
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE bool equivalent(const path& p1, const path& p2)
{
    std::error_code ec;
    bool result = equivalent(p1, p2, ec);
    if (ec) {
        throw filesystem_error(detail::systemErrorText(ec.value()), p1, p2, ec);
    }
    return result;
}
#endif

GHC_INLINE bool equivalent(const path& p1, const path& p2, std::error_code& ec) noexcept
{
    ec.clear();
#ifdef GHC_OS_WINDOWS
    std::shared_ptr<void> file1(::CreateFileW(p1.wstring().c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0), CloseHandle);
    auto e1 = ::GetLastError();
    std::shared_ptr<void> file2(::CreateFileW(p2.wstring().c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0), CloseHandle);
    if (file1.get() == INVALID_HANDLE_VALUE || file2.get() == INVALID_HANDLE_VALUE) {
#ifdef LWG_2937_BEHAVIOUR
        ec = detail::make_system_error(e1 ? e1 : ::GetLastError());
#else
        if (file1 == file2) {
            ec = detail::make_system_error(e1 ? e1 : ::GetLastError());
        }
#endif
        return false;
    }
    BY_HANDLE_FILE_INFORMATION inf1, inf2;
    if (!::GetFileInformationByHandle(file1.get(), &inf1)) {
        ec = detail::make_system_error();
        return false;
    }
    if (!::GetFileInformationByHandle(file2.get(), &inf2)) {
        ec = detail::make_system_error();
        return false;
    }
    return inf1.ftLastWriteTime.dwLowDateTime == inf2.ftLastWriteTime.dwLowDateTime && inf1.ftLastWriteTime.dwHighDateTime == inf2.ftLastWriteTime.dwHighDateTime && inf1.nFileIndexHigh == inf2.nFileIndexHigh && inf1.nFileIndexLow == inf2.nFileIndexLow &&
           inf1.nFileSizeHigh == inf2.nFileSizeHigh && inf1.nFileSizeLow == inf2.nFileSizeLow && inf1.dwVolumeSerialNumber == inf2.dwVolumeSerialNumber;
#else
    struct ::stat s1, s2;
    auto rc1 = ::stat(p1.c_str(), &s1);
    auto e1 = errno;
    auto rc2 = ::stat(p2.c_str(), &s2);
    if (rc1 || rc2) {
#ifdef LWG_2937_BEHAVIOUR
        ec = detail::make_system_error(e1 ? e1 : errno);
#else
        if (rc1 && rc2) {
            ec = detail::make_system_error(e1 ? e1 : errno);
        }
#endif
        return false;
    }
    return s1.st_dev == s2.st_dev && s1.st_ino == s2.st_ino && s1.st_size == s2.st_size && s1.st_mtime == s2.st_mtime;
#endif
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE uintmax_t file_size(const path& p)
{
    std::error_code ec;
    auto result = file_size(p, ec);
    if (ec) {
        throw filesystem_error(detail::systemErrorText(ec.value()), p, ec);
    }
    return result;
}
#endif

GHC_INLINE uintmax_t file_size(const path& p, std::error_code& ec) noexcept
{
    ec.clear();
#ifdef GHC_OS_WINDOWS
    WIN32_FILE_ATTRIBUTE_DATA attr;
    if (!GetFileAttributesExW(detail::fromUtf8<std::wstring>(p.u8string()).c_str(), GetFileExInfoStandard, &attr)) {
        ec = detail::make_system_error();
        return static_cast<uintmax_t>(-1);
    }
    return static_cast<uintmax_t>(attr.nFileSizeHigh) << (sizeof(attr.nFileSizeHigh) * 8) | attr.nFileSizeLow;
#else
    struct ::stat fileStat;
    if (::stat(p.c_str(), &fileStat) == -1) {
        ec = detail::make_system_error();
        return static_cast<uintmax_t>(-1);
    }
    return static_cast<uintmax_t>(fileStat.st_size);
#endif
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE uintmax_t hard_link_count(const path& p)
{
    std::error_code ec;
    auto result = hard_link_count(p, ec);
    if (ec) {
        throw filesystem_error(detail::systemErrorText(ec.value()), p, ec);
    }
    return result;
}
#endif

GHC_INLINE uintmax_t hard_link_count(const path& p, std::error_code& ec) noexcept
{
    ec.clear();
#ifdef GHC_OS_WINDOWS
    uintmax_t result = static_cast<uintmax_t>(-1);
    std::shared_ptr<void> file(::CreateFileW(p.wstring().c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0), CloseHandle);
    BY_HANDLE_FILE_INFORMATION inf;
    if (file.get() == INVALID_HANDLE_VALUE) {
        ec = detail::make_system_error();
    }
    else {
        if (!::GetFileInformationByHandle(file.get(), &inf)) {
            ec = detail::make_system_error();
        }
        else {
            result = inf.nNumberOfLinks;
        }
    }
    return result;
#else
    uintmax_t result = 0;
    file_status fs = detail::status_ex(p, ec, nullptr, nullptr, &result, nullptr);
    if (fs.type() == file_type::not_found) {
        ec = detail::make_error_code(detail::portable_error::not_found);
    }
    return ec ? static_cast<uintmax_t>(-1) : result;
#endif
}

GHC_INLINE bool is_block_file(file_status s) noexcept
{
    return s.type() == file_type::block;
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE bool is_block_file(const path& p)
{
    return is_block_file(status(p));
}
#endif

GHC_INLINE bool is_block_file(const path& p, std::error_code& ec) noexcept
{
    return is_block_file(status(p, ec));
}

GHC_INLINE bool is_character_file(file_status s) noexcept
{
    return s.type() == file_type::character;
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE bool is_character_file(const path& p)
{
    return is_character_file(status(p));
}
#endif

GHC_INLINE bool is_character_file(const path& p, std::error_code& ec) noexcept
{
    return is_character_file(status(p, ec));
}

GHC_INLINE bool is_directory(file_status s) noexcept
{
    return s.type() == file_type::directory;
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE bool is_directory(const path& p)
{
    return is_directory(status(p));
}
#endif

GHC_INLINE bool is_directory(const path& p, std::error_code& ec) noexcept
{
    return is_directory(status(p, ec));
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE bool is_empty(const path& p)
{
    if (is_directory(p)) {
        return directory_iterator(p) == directory_iterator();
    }
    else {
        return file_size(p) == 0;
    }
}
#endif

GHC_INLINE bool is_empty(const path& p, std::error_code& ec) noexcept
{
    auto fs = status(p, ec);
    if (ec) {
        return false;
    }
    if (is_directory(fs)) {
        directory_iterator iter(p, ec);
        if (ec) {
            return false;
        }
        return iter == directory_iterator();
    }
    else {
        auto sz = file_size(p, ec);
        if (ec) {
            return false;
        }
        return sz == 0;
    }
}

GHC_INLINE bool is_fifo(file_status s) noexcept
{
    return s.type() == file_type::fifo;
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE bool is_fifo(const path& p)
{
    return is_fifo(status(p));
}
#endif

GHC_INLINE bool is_fifo(const path& p, std::error_code& ec) noexcept
{
    return is_fifo(status(p, ec));
}

GHC_INLINE bool is_other(file_status s) noexcept
{
    return exists(s) && !is_regular_file(s) && !is_directory(s) && !is_symlink(s);
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE bool is_other(const path& p)
{
    return is_other(status(p));
}
#endif

GHC_INLINE bool is_other(const path& p, std::error_code& ec) noexcept
{
    return is_other(status(p, ec));
}

GHC_INLINE bool is_regular_file(file_status s) noexcept
{
    return s.type() == file_type::regular;
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE bool is_regular_file(const path& p)
{
    return is_regular_file(status(p));
}
#endif

GHC_INLINE bool is_regular_file(const path& p, std::error_code& ec) noexcept
{
    return is_regular_file(status(p, ec));
}

GHC_INLINE bool is_socket(file_status s) noexcept
{
    return s.type() == file_type::socket;
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE bool is_socket(const path& p)
{
    return is_socket(status(p));
}
#endif

GHC_INLINE bool is_socket(const path& p, std::error_code& ec) noexcept
{
    return is_socket(status(p, ec));
}

GHC_INLINE bool is_symlink(file_status s) noexcept
{
    return s.type() == file_type::symlink;
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE bool is_symlink(const path& p)
{
    return is_symlink(symlink_status(p));
}
#endif

GHC_INLINE bool is_symlink(const path& p, std::error_code& ec) noexcept
{
    return is_symlink(symlink_status(p, ec));
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE file_time_type last_write_time(const path& p)
{
    std::error_code ec;
    auto result = last_write_time(p, ec);
    if (ec) {
        throw filesystem_error(detail::systemErrorText(ec.value()), p, ec);
    }
    return result;
}
#endif

GHC_INLINE file_time_type last_write_time(const path& p, std::error_code& ec) noexcept
{
    time_t result = 0;
    ec.clear();
    file_status fs = detail::status_ex(p, ec, nullptr, nullptr, nullptr, &result);
    return ec ? (file_time_type::min)() : std::chrono::system_clock::from_time_t(result);
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE void last_write_time(const path& p, file_time_type new_time)
{
    std::error_code ec;
    last_write_time(p, new_time, ec);
    if (ec) {
        throw filesystem_error(detail::systemErrorText(ec.value()), p, ec);
    }
}
#endif

GHC_INLINE void last_write_time(const path& p, file_time_type new_time, std::error_code& ec) noexcept
{
    ec.clear();
    auto d = new_time.time_since_epoch();
#ifdef GHC_OS_WINDOWS
    std::shared_ptr<void> file(::CreateFileW(p.wstring().c_str(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL), ::CloseHandle);
    FILETIME ft;
    auto tt = std::chrono::duration_cast<std::chrono::microseconds>(d).count() * 10 + 116444736000000000;
    ft.dwLowDateTime = static_cast<DWORD>(tt);
    ft.dwHighDateTime = static_cast<DWORD>(tt >> 32);
    if (!::SetFileTime(file.get(), 0, 0, &ft)) {
        ec = detail::make_system_error();
    }
#elif defined(GHC_OS_MACOS)
#ifdef __MAC_OS_X_VERSION_MIN_REQUIRED
#if __MAC_OS_X_VERSION_MIN_REQUIRED < 101300
    struct ::stat fs;
    if (::stat(p.c_str(), &fs) == 0) {
        struct ::timeval tv[2];
        tv[0].tv_sec = fs.st_atimespec.tv_sec;
        tv[0].tv_usec = static_cast<int>(fs.st_atimespec.tv_nsec / 1000);
        tv[1].tv_sec = std::chrono::duration_cast<std::chrono::seconds>(d).count();
        tv[1].tv_usec = static_cast<int>(std::chrono::duration_cast<std::chrono::microseconds>(d).count() % 1000000);
        if (::utimes(p.c_str(), tv) == 0) {
            return;
        }
    }
    ec = detail::make_system_error();
    return;
#else
    struct ::timespec times[2];
    times[0].tv_sec = 0;
    times[0].tv_nsec = UTIME_OMIT;
    times[1].tv_sec = std::chrono::duration_cast<std::chrono::seconds>(d).count();
    times[1].tv_nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(d).count() % 1000000000;
    if (::utimensat(AT_FDCWD, p.c_str(), times, AT_SYMLINK_NOFOLLOW) != 0) {
        ec = detail::make_system_error();
    }
    return;
#endif
#endif
#else
#ifndef UTIME_OMIT
#define UTIME_OMIT ((1l << 30) - 2l)
#endif
    struct ::timespec times[2];
    times[0].tv_sec = 0;
    times[0].tv_nsec = UTIME_OMIT;
    times[1].tv_sec = static_cast<decltype(times[1].tv_sec)>(std::chrono::duration_cast<std::chrono::seconds>(d).count());
    times[1].tv_nsec = static_cast<decltype(times[1].tv_nsec)>(std::chrono::duration_cast<std::chrono::nanoseconds>(d).count() % 1000000000);
#if defined(__ANDROID_API__) && __ANDROID_API__ < 12
    if (syscall(__NR_utimensat, AT_FDCWD, p.c_str(), times, AT_SYMLINK_NOFOLLOW) != 0) {
#else
    if (::utimensat(AT_FDCWD, p.c_str(), times, AT_SYMLINK_NOFOLLOW) != 0) {
#endif
        ec = detail::make_system_error();
    }
    return;
#endif
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE void permissions(const path& p, perms prms, perm_options opts)
{
    std::error_code ec;
    permissions(p, prms, opts, ec);
    if (ec) {
        throw filesystem_error(detail::systemErrorText(ec.value()), p, ec);
    }
}
#endif

GHC_INLINE void permissions(const path& p, perms prms, std::error_code& ec) noexcept
{
    permissions(p, prms, perm_options::replace, ec);
}

GHC_INLINE void permissions(const path& p, perms prms, perm_options opts, std::error_code& ec)
{
    if (static_cast<int>(opts & (perm_options::replace | perm_options::add | perm_options::remove)) == 0) {
        ec = detail::make_error_code(detail::portable_error::invalid_argument);
        return;
    }
    auto fs = symlink_status(p, ec);
    if ((opts & perm_options::replace) != perm_options::replace) {
        if ((opts & perm_options::add) == perm_options::add) {
            prms = fs.permissions() | prms;
        }
        else {
            prms = fs.permissions() & ~prms;
        }
    }
#ifdef GHC_OS_WINDOWS
#ifdef __GNUC__
    auto oldAttr = GetFileAttributesW(p.wstring().c_str());
    if (oldAttr != INVALID_FILE_ATTRIBUTES) {
        DWORD newAttr = ((prms & perms::owner_write) == perms::owner_write) ? oldAttr & ~(static_cast<DWORD>(FILE_ATTRIBUTE_READONLY)) : oldAttr | FILE_ATTRIBUTE_READONLY;
        if (oldAttr == newAttr || SetFileAttributesW(p.wstring().c_str(), newAttr)) {
            return;
        }
    }
    ec = detail::make_system_error();
#else
    int mode = 0;
    if ((prms & perms::owner_read) == perms::owner_read) {
        mode |= _S_IREAD;
    }
    if ((prms & perms::owner_write) == perms::owner_write) {
        mode |= _S_IWRITE;
    }
    if (::_wchmod(p.wstring().c_str(), mode) != 0) {
        ec = detail::make_system_error();
    }
#endif
#else
    if ((opts & perm_options::nofollow) != perm_options::nofollow) {
        if (::chmod(p.c_str(), static_cast<mode_t>(prms)) != 0) {
            ec = detail::make_system_error();
        }
    }
#endif
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE path proximate(const path& p, std::error_code& ec)
{
    return proximate(p, current_path(), ec);
}
#endif

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE path proximate(const path& p, const path& base)
{
    return weakly_canonical(p).lexically_proximate(weakly_canonical(base));
}
#endif

GHC_INLINE path proximate(const path& p, const path& base, std::error_code& ec)
{
    return weakly_canonical(p, ec).lexically_proximate(weakly_canonical(base, ec));
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE path read_symlink(const path& p)
{
    std::error_code ec;
    auto result = read_symlink(p, ec);
    if (ec) {
        throw filesystem_error(detail::systemErrorText(ec.value()), p, ec);
    }
    return result;
}
#endif

GHC_INLINE path read_symlink(const path& p, std::error_code& ec)
{
    file_status fs = symlink_status(p, ec);
    if (fs.type() != file_type::symlink) {
        ec = detail::make_error_code(detail::portable_error::invalid_argument);
        return path();
    }
    auto result = detail::resolveSymlink(p, ec);
    return ec ? path() : result;
}

GHC_INLINE path relative(const path& p, std::error_code& ec)
{
    return relative(p, current_path(ec), ec);
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE path relative(const path& p, const path& base)
{
    return weakly_canonical(p).lexically_relative(weakly_canonical(base));
}
#endif

GHC_INLINE path relative(const path& p, const path& base, std::error_code& ec)
{
    return weakly_canonical(p, ec).lexically_relative(weakly_canonical(base, ec));
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE bool remove(const path& p)
{
    std::error_code ec;
    auto result = remove(p, ec);
    if (ec) {
        throw filesystem_error(detail::systemErrorText(ec.value()), p, ec);
    }
    return result;
}
#endif

GHC_INLINE bool remove(const path& p, std::error_code& ec) noexcept
{
    ec.clear();
#ifdef GHC_OS_WINDOWS
    std::wstring np = detail::fromUtf8<std::wstring>(p.u8string());
    DWORD attr = GetFileAttributesW(np.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES) {
        auto error = ::GetLastError();
        if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND) {
            return false;
        }
        ec = detail::make_system_error(error);
    }
    if (!ec) {
        if (attr & FILE_ATTRIBUTE_DIRECTORY) {
            if (!RemoveDirectoryW(np.c_str())) {
                ec = detail::make_system_error();
            }
        }
        else {
            if (!DeleteFileW(np.c_str())) {
                ec = detail::make_system_error();
            }
        }
    }
#else
    if (::remove(p.c_str()) == -1) {
        auto error = errno;
        if (error == ENOENT) {
            return false;
        }
        ec = detail::make_system_error();
    }
#endif
    return ec ? false : true;
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE uintmax_t remove_all(const path& p)
{
    std::error_code ec;
    auto result = remove_all(p, ec);
    if (ec) {
        throw filesystem_error(detail::systemErrorText(ec.value()), p, ec);
    }
    return result;
}
#endif

GHC_INLINE uintmax_t remove_all(const path& p, std::error_code& ec) noexcept
{
    ec.clear();
    uintmax_t count = 0;
    if (p == "/") {
        ec = detail::make_error_code(detail::portable_error::not_supported);
        return static_cast<uintmax_t>(-1);
    }
    std::error_code tec;
    auto fs = status(p, tec);
    if (exists(fs) && is_directory(fs)) {
        for (auto iter = directory_iterator(p, ec); iter != directory_iterator(); iter.increment(ec)) {
            if (ec) {
                break;
            }
            bool is_symlink_result = iter->is_symlink(ec);
            if (ec) return static_cast<uintmax_t>(-1);
            bool is_directory_result = iter->is_directory(ec);
            if (ec) return static_cast<uintmax_t>(-1);
            if (!is_symlink_result && is_directory_result) {
                count += remove_all(iter->path(), ec);
                if (ec) {
                    return static_cast<uintmax_t>(-1);
                }
            }
            else {
                remove(iter->path(), ec);
                if (ec) {
                    return static_cast<uintmax_t>(-1);
                }
                ++count;
            }
        }
    }
    if (!ec) {
        if (remove(p, ec)) {
            ++count;
        }
    }
    if (ec) {
        return static_cast<uintmax_t>(-1);
    }
    return count;
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE void rename(const path& from, const path& to)
{
    std::error_code ec;
    rename(from, to, ec);
    if (ec) {
        throw filesystem_error(detail::systemErrorText(ec.value()), from, to, ec);
    }
}
#endif

GHC_INLINE void rename(const path& from, const path& to, std::error_code& ec) noexcept
{
    ec.clear();
#ifdef GHC_OS_WINDOWS
    if (from != to) {
        if (!MoveFileExW(detail::fromUtf8<std::wstring>(from.u8string()).c_str(), detail::fromUtf8<std::wstring>(to.u8string()).c_str(), (DWORD)MOVEFILE_REPLACE_EXISTING)) {
            ec = detail::make_system_error();
        }
    }
#else
    if (from != to) {
        if (::rename(from.c_str(), to.c_str()) != 0) {
            ec = detail::make_system_error();
        }
    }
#endif
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE void resize_file(const path& p, uintmax_t size)
{
    std::error_code ec;
    resize_file(p, size, ec);
    if (ec) {
        throw filesystem_error(detail::systemErrorText(ec.value()), p, ec);
    }
}
#endif

GHC_INLINE void resize_file(const path& p, uintmax_t size, std::error_code& ec) noexcept
{
    ec.clear();
#ifdef GHC_OS_WINDOWS
    LARGE_INTEGER lisize;
    lisize.QuadPart = static_cast<LONGLONG>(size);
    if(lisize.QuadPart < 0) {
#ifdef ERROR_FILE_TOO_LARGE
        ec = detail::make_system_error(ERROR_FILE_TOO_LARGE);
#else
        ec = detail::make_system_error(223);
#endif
        return;
    }
    std::shared_ptr<void> file(CreateFileW(detail::fromUtf8<std::wstring>(p.u8string()).c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL), CloseHandle);
    if (file.get() == INVALID_HANDLE_VALUE) {
        ec = detail::make_system_error();
    }
    else if (SetFilePointerEx(file.get(), lisize, NULL, FILE_BEGIN) == 0 || SetEndOfFile(file.get()) == 0) {
        ec = detail::make_system_error();
    }
#else
    if (::truncate(p.c_str(), static_cast<off_t>(size)) != 0) {
        ec = detail::make_system_error();
    }
#endif
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE space_info space(const path& p)
{
    std::error_code ec;
    auto result = space(p, ec);
    if (ec) {
        throw filesystem_error(detail::systemErrorText(ec.value()), p, ec);
    }
    return result;
}
#endif

GHC_INLINE space_info space(const path& p, std::error_code& ec) noexcept
{
    ec.clear();
#ifdef GHC_OS_WINDOWS
    ULARGE_INTEGER freeBytesAvailableToCaller = {{0, 0}};
    ULARGE_INTEGER totalNumberOfBytes = {{0, 0}};
    ULARGE_INTEGER totalNumberOfFreeBytes = {{0, 0}};
    if (!GetDiskFreeSpaceExW(detail::fromUtf8<std::wstring>(p.u8string()).c_str(), &freeBytesAvailableToCaller, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
        ec = detail::make_system_error();
        return {static_cast<uintmax_t>(-1), static_cast<uintmax_t>(-1), static_cast<uintmax_t>(-1)};
    }
    return {static_cast<uintmax_t>(totalNumberOfBytes.QuadPart), static_cast<uintmax_t>(totalNumberOfFreeBytes.QuadPart), static_cast<uintmax_t>(freeBytesAvailableToCaller.QuadPart)};
#else
    struct ::statvfs sfs;
    if (::statvfs(p.c_str(), &sfs) != 0) {
        ec = detail::make_system_error();
        return {static_cast<uintmax_t>(-1), static_cast<uintmax_t>(-1), static_cast<uintmax_t>(-1)};
    }
    return {static_cast<uintmax_t>(sfs.f_blocks * sfs.f_frsize), static_cast<uintmax_t>(sfs.f_bfree * sfs.f_frsize), static_cast<uintmax_t>(sfs.f_bavail * sfs.f_frsize)};
#endif
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE file_status status(const path& p)
{
    std::error_code ec;
    auto result = status(p, ec);
    if (result.type() == file_type::none) {
        throw filesystem_error(detail::systemErrorText(ec.value()), p, ec);
    }
    return result;
}
#endif

GHC_INLINE file_status status(const path& p, std::error_code& ec) noexcept
{
    return detail::status_ex(p, ec);
}

GHC_INLINE bool status_known(file_status s) noexcept
{
    return s.type() != file_type::none;
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE file_status symlink_status(const path& p)
{
    std::error_code ec;
    auto result = symlink_status(p, ec);
    if (result.type() == file_type::none) {
        throw filesystem_error(detail::systemErrorText(ec.value()), ec);
    }
    return result;
}
#endif

GHC_INLINE file_status symlink_status(const path& p, std::error_code& ec) noexcept
{
    return detail::symlink_status_ex(p, ec);
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE path temp_directory_path()
{
    std::error_code ec;
    path result = temp_directory_path(ec);
    if (ec) {
        throw filesystem_error(detail::systemErrorText(ec.value()), ec);
    }
    return result;
}
#endif

GHC_INLINE path temp_directory_path(std::error_code& ec) noexcept
{
    ec.clear();
#ifdef GHC_OS_WINDOWS
    wchar_t buffer[512];
    auto rc = GetTempPathW(511, buffer);
    if (!rc || rc > 511) {
        ec = detail::make_system_error();
        return path();
    }
    return path(std::wstring(buffer));
#else
    static const char* temp_vars[] = {"TMPDIR", "TMP", "TEMP", "TEMPDIR", nullptr};
    const char* temp_path = nullptr;
    for (auto temp_name = temp_vars; *temp_name != nullptr; ++temp_name) {
        temp_path = std::getenv(*temp_name);
        if (temp_path) {
            return path(temp_path);
        }
    }
    return path("/tmp");
#endif
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE path weakly_canonical(const path& p)
{
    std::error_code ec;
    auto result = weakly_canonical(p, ec);
    if (ec) {
        throw filesystem_error(detail::systemErrorText(ec.value()), p, ec);
    }
    return result;
}
#endif

GHC_INLINE path weakly_canonical(const path& p, std::error_code& ec) noexcept
{
    path result;
    ec.clear();
    bool scan = true;
    for (auto pe : p) {
        if (scan) {
            std::error_code tec;
            if (exists(result / pe, tec)) {
                result /= pe;
            }
            else {
                if (ec) {
                    return path();
                }
                scan = false;
                if (!result.empty()) {
                    result = canonical(result, ec) / pe;
                    if (ec) {
                        break;
                    }
                }
                else {
                    result /= pe;
                }
            }
        }
        else {
            result /= pe;
        }
    }
    if (scan) {
        if (!result.empty()) {
            result = canonical(result, ec);
        }
    }
    return ec ? path() : result.lexically_normal();
}

//-----------------------------------------------------------------------------
// 30.10.11 class file_status
// 30.10.11.1 constructors and destructor
GHC_INLINE file_status::file_status() noexcept
    : file_status(file_type::none)
{
}

GHC_INLINE file_status::file_status(file_type ft, perms prms) noexcept
    : _type(ft)
    , _perms(prms)
{
}

GHC_INLINE file_status::file_status(const file_status& other) noexcept
    : _type(other._type)
    , _perms(other._perms)
{
}

GHC_INLINE file_status::file_status(file_status&& other) noexcept
    : _type(other._type)
    , _perms(other._perms)
{
}

GHC_INLINE file_status::~file_status() {}

// assignments:
GHC_INLINE file_status& file_status::operator=(const file_status& rhs) noexcept
{
    _type = rhs._type;
    _perms = rhs._perms;
    return *this;
}

GHC_INLINE file_status& file_status::operator=(file_status&& rhs) noexcept
{
    _type = rhs._type;
    _perms = rhs._perms;
    return *this;
}

// 30.10.11.3 modifiers
GHC_INLINE void file_status::type(file_type ft) noexcept
{
    _type = ft;
}

GHC_INLINE void file_status::permissions(perms prms) noexcept
{
    _perms = prms;
}

// 30.10.11.2 observers
GHC_INLINE file_type file_status::type() const noexcept
{
    return _type;
}

GHC_INLINE perms file_status::permissions() const noexcept
{
    return _perms;
}

//-----------------------------------------------------------------------------
// 30.10.12 class directory_entry
// 30.10.12.1 constructors and destructor
// directory_entry::directory_entry() noexcept = default;
// directory_entry::directory_entry(const directory_entry&) = default;
// directory_entry::directory_entry(directory_entry&&) noexcept = default;
#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE directory_entry::directory_entry(const filesystem::path& p)
    : _path(p)
    , _file_size(0)
#ifndef GHC_OS_WINDOWS
    , _hard_link_count(0)
#endif
    , _last_write_time(0)
{
    refresh();
}
#endif

GHC_INLINE directory_entry::directory_entry(const filesystem::path& p, std::error_code& ec)
    : _path(p)
    , _file_size(0)
#ifndef GHC_OS_WINDOWS
    , _hard_link_count(0)
#endif
    , _last_write_time(0)
{
    refresh(ec);
}

GHC_INLINE directory_entry::~directory_entry() {}

// assignments:
// directory_entry& directory_entry::operator=(const directory_entry&) = default;
// directory_entry& directory_entry::operator=(directory_entry&&) noexcept = default;

// 30.10.12.2 directory_entry modifiers
#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE void directory_entry::assign(const filesystem::path& p)
{
    _path = p;
    refresh();
}
#endif

GHC_INLINE void directory_entry::assign(const filesystem::path& p, std::error_code& ec)
{
    _path = p;
    refresh(ec);
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE void directory_entry::replace_filename(const filesystem::path& p)
{
    _path.replace_filename(p);
    refresh();
}
#endif

GHC_INLINE void directory_entry::replace_filename(const filesystem::path& p, std::error_code& ec)
{
    _path.replace_filename(p);
    refresh(ec);
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE void directory_entry::refresh()
{
    std::error_code ec;
    refresh(ec);
    if (ec) {
        throw filesystem_error(detail::systemErrorText(ec.value()), _path, ec);
    }
}
#endif

GHC_INLINE void directory_entry::refresh(std::error_code& ec) noexcept
{
#ifdef GHC_OS_WINDOWS
    _status = detail::status_ex(_path, ec, &_symlink_status, &_file_size, nullptr, &_last_write_time);
#else
    _status = detail::status_ex(_path, ec, &_symlink_status, &_file_size, &_hard_link_count, &_last_write_time);
#endif
}

// 30.10.12.3 directory_entry observers
GHC_INLINE const filesystem::path& directory_entry::path() const noexcept
{
    return _path;
}

GHC_INLINE directory_entry::operator const filesystem::path&() const noexcept
{
    return _path;
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE bool directory_entry::exists() const
{
    return filesystem::exists(status());
}
#endif

GHC_INLINE bool directory_entry::exists(std::error_code& ec) const noexcept
{
    return filesystem::exists(status(ec));
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE bool directory_entry::is_block_file() const
{
    return filesystem::is_block_file(status());
}
#endif
GHC_INLINE bool directory_entry::is_block_file(std::error_code& ec) const noexcept
{
    return filesystem::is_block_file(status(ec));
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE bool directory_entry::is_character_file() const
{
    return filesystem::is_character_file(status());
}
#endif

GHC_INLINE bool directory_entry::is_character_file(std::error_code& ec) const noexcept
{
    return filesystem::is_character_file(status(ec));
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE bool directory_entry::is_directory() const
{
    return filesystem::is_directory(status());
}
#endif

GHC_INLINE bool directory_entry::is_directory(std::error_code& ec) const noexcept
{
    return filesystem::is_directory(status(ec));
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE bool directory_entry::is_fifo() const
{
    return filesystem::is_fifo(status());
}
#endif

GHC_INLINE bool directory_entry::is_fifo(std::error_code& ec) const noexcept
{
    return filesystem::is_fifo(status(ec));
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE bool directory_entry::is_other() const
{
    return filesystem::is_other(status());
}
#endif

GHC_INLINE bool directory_entry::is_other(std::error_code& ec) const noexcept
{
    return filesystem::is_other(status(ec));
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE bool directory_entry::is_regular_file() const
{
    return filesystem::is_regular_file(status());
}
#endif

GHC_INLINE bool directory_entry::is_regular_file(std::error_code& ec) const noexcept
{
    return filesystem::is_regular_file(status(ec));
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE bool directory_entry::is_socket() const
{
    return filesystem::is_socket(status());
}
#endif

GHC_INLINE bool directory_entry::is_socket(std::error_code& ec) const noexcept
{
    return filesystem::is_socket(status(ec));
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE bool directory_entry::is_symlink() const
{
    return filesystem::is_symlink(symlink_status());
}
#endif

GHC_INLINE bool directory_entry::is_symlink(std::error_code& ec) const noexcept
{
    return filesystem::is_symlink(symlink_status(ec));
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE uintmax_t directory_entry::file_size() const
{
    if (_status.type() != file_type::none) {
        return _file_size;
    }
    return filesystem::file_size(path());
}
#endif

GHC_INLINE uintmax_t directory_entry::file_size(std::error_code& ec) const noexcept
{
    if (_status.type() != file_type::none) {
        ec.clear();
        return _file_size;
    }
    return filesystem::file_size(path(), ec);
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE uintmax_t directory_entry::hard_link_count() const
{
#ifndef GHC_OS_WINDOWS
    if (_status.type() != file_type::none) {
        return _hard_link_count;
    }
#endif
    return filesystem::hard_link_count(path());
}
#endif

GHC_INLINE uintmax_t directory_entry::hard_link_count(std::error_code& ec) const noexcept
{
#ifndef GHC_OS_WINDOWS
    if (_status.type() != file_type::none) {
        ec.clear();
        return _hard_link_count;
    }
#endif
    return filesystem::hard_link_count(path(), ec);
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE file_time_type directory_entry::last_write_time() const
{
    if (_status.type() != file_type::none) {
        return std::chrono::system_clock::from_time_t(_last_write_time);
    }
    return filesystem::last_write_time(path());
}
#endif

GHC_INLINE file_time_type directory_entry::last_write_time(std::error_code& ec) const noexcept
{
    if (_status.type() != file_type::none) {
        ec.clear();
        return std::chrono::system_clock::from_time_t(_last_write_time);
    }
    return filesystem::last_write_time(path(), ec);
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE file_status directory_entry::status() const
{
    if (_status.type() != file_type::none) {
        return _status;
    }
    return filesystem::status(path());
}
#endif

GHC_INLINE file_status directory_entry::status(std::error_code& ec) const noexcept
{
    if (_status.type() != file_type::none) {
        ec.clear();
        return _status;
    }
    return filesystem::status(path(), ec);
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE file_status directory_entry::symlink_status() const
{
    if (_symlink_status.type() != file_type::none) {
        return _symlink_status;
    }
    return filesystem::symlink_status(path());
}
#endif

GHC_INLINE file_status directory_entry::symlink_status(std::error_code& ec) const noexcept
{
    if (_symlink_status.type() != file_type::none) {
        ec.clear();
        return _symlink_status;
    }
    return filesystem::symlink_status(path(), ec);
}

GHC_INLINE bool directory_entry::operator<(const directory_entry& rhs) const noexcept
{
    return _path < rhs._path;
}

GHC_INLINE bool directory_entry::operator==(const directory_entry& rhs) const noexcept
{
    return _path == rhs._path;
}

GHC_INLINE bool directory_entry::operator!=(const directory_entry& rhs) const noexcept
{
    return _path != rhs._path;
}

GHC_INLINE bool directory_entry::operator<=(const directory_entry& rhs) const noexcept
{
    return _path <= rhs._path;
}

GHC_INLINE bool directory_entry::operator>(const directory_entry& rhs) const noexcept
{
    return _path > rhs._path;
}

GHC_INLINE bool directory_entry::operator>=(const directory_entry& rhs) const noexcept
{
    return _path >= rhs._path;
}

//-----------------------------------------------------------------------------
// 30.10.13 class directory_iterator

#ifdef GHC_OS_WINDOWS
class directory_iterator::impl
{
public:
    impl(const path& p, directory_options options)
        : _base(p)
        , _options(options)
        , _dirHandle(INVALID_HANDLE_VALUE)
    {
        if (!_base.empty()) {
            ZeroMemory(&_findData, sizeof(WIN32_FIND_DATAW));
            if ((_dirHandle = FindFirstFileW(detail::fromUtf8<std::wstring>((_base / "*").u8string()).c_str(), &_findData)) != INVALID_HANDLE_VALUE) {
                if (std::wstring(_findData.cFileName) == L"." || std::wstring(_findData.cFileName) == L"..") {
                    increment(_ec);
                }
                else {
                    _current = _base / std::wstring(_findData.cFileName);
                    copyToDirEntry(_ec);
                }
            }
            else {
                auto error = ::GetLastError();
                _base = filesystem::path();
                if (error != ERROR_ACCESS_DENIED || (options & directory_options::skip_permission_denied) == directory_options::none) {
                    _ec = detail::make_system_error();
                }
            }
        }
    }
    impl(const impl& other) = delete;
    ~impl()
    {
        if (_dirHandle != INVALID_HANDLE_VALUE) {
            FindClose(_dirHandle);
            _dirHandle = INVALID_HANDLE_VALUE;
        }
    }
    void increment(std::error_code& ec)
    {
        if (_dirHandle != INVALID_HANDLE_VALUE) {
            do {
                if (FindNextFileW(_dirHandle, &_findData)) {
                    _current = _base;
#ifdef GHC_RAISE_UNICODE_ERRORS
                    try {
                        _current.append_name(detail::toUtf8(_findData.cFileName).c_str());
                    }
                    catch(filesystem_error& fe) {
                        ec = fe.code();
                        return;
                    }
#else
                    _current.append_name(detail::toUtf8(_findData.cFileName).c_str());
#endif
                    copyToDirEntry(ec);
                }
                else {
                    auto err = ::GetLastError();
                    if(err != ERROR_NO_MORE_FILES) {
                        _ec = ec = detail::make_system_error(err);
                    }
                    FindClose(_dirHandle);
                    _dirHandle = INVALID_HANDLE_VALUE;
                    _current = filesystem::path();
                    break;
                }
            } while (std::wstring(_findData.cFileName) == L"." || std::wstring(_findData.cFileName) == L"..");
        }
        else {
            ec = _ec;
        }
    }
    void copyToDirEntry(std::error_code& ec)
    {
        _dir_entry._path = _current;
        if (_findData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
            _dir_entry._status = detail::status_ex(_current, ec, &_dir_entry._symlink_status, &_dir_entry._file_size, nullptr, &_dir_entry._last_write_time);
        }
        else {
            _dir_entry._status = detail::status_from_INFO(_current, &_findData, ec, &_dir_entry._file_size, &_dir_entry._last_write_time);
            _dir_entry._symlink_status = _dir_entry._status;
        }
        if (ec) {
            if (_dir_entry._status.type() != file_type::none && _dir_entry._symlink_status.type() != file_type::none) {
                ec.clear();
            }
            else {
                _dir_entry._file_size = static_cast<uintmax_t>(-1);
                _dir_entry._last_write_time = 0;
            }
        }
    }
    path _base;
    directory_options _options;
    WIN32_FIND_DATAW _findData;
    HANDLE _dirHandle;
    path _current;
    directory_entry _dir_entry;
    std::error_code _ec;
};
#else
// POSIX implementation
class directory_iterator::impl
{
public:
    impl(const path& path, directory_options options)
        : _base(path)
        , _options(options)
        , _dir(nullptr)
        , _entry(nullptr)
    {
        if (!path.empty()) {
            _dir = ::opendir(path.native().c_str());
        }
        if (!path.empty()) {
            if (!_dir) {
                auto error = errno;
                _base = filesystem::path();
                if (error != EACCES || (options & directory_options::skip_permission_denied) == directory_options::none) {
                    _ec = detail::make_system_error();
                }
            }
            else {
                increment(_ec);
            }
        }
    }
    impl(const impl& other) = delete;
    ~impl()
    {
        if (_dir) {
            ::closedir(_dir);
        }
    }
    void increment(std::error_code& ec)
    {
        if (_dir) {
            do {
                errno = 0;
                _entry = readdir(_dir);
                if (_entry) {
                    _current = _base;
                    _current.append_name(_entry->d_name);
                    _dir_entry = directory_entry(_current, ec);
                }
                else {
                    ::closedir(_dir);
                    _dir = nullptr;
                    _current = path();
                    if (errno) {
                        ec = detail::make_system_error();
                    }
                    break;
                }
            } while (std::strcmp(_entry->d_name, ".") == 0 || std::strcmp(_entry->d_name, "..") == 0);
        }
    }
    path _base;
    directory_options _options;
    path _current;
    DIR* _dir;
    struct ::dirent* _entry;
    directory_entry _dir_entry;
    std::error_code _ec;
};
#endif

// 30.10.13.1 member functions
GHC_INLINE directory_iterator::directory_iterator() noexcept
    : _impl(new impl(path(), directory_options::none))
{
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE directory_iterator::directory_iterator(const path& p)
    : _impl(new impl(p, directory_options::none))
{
    if (_impl->_ec) {
        throw filesystem_error(detail::systemErrorText(_impl->_ec.value()), p, _impl->_ec);
    }
    _impl->_ec.clear();
}

GHC_INLINE directory_iterator::directory_iterator(const path& p, directory_options options)
    : _impl(new impl(p, options))
{
    if (_impl->_ec) {
        throw filesystem_error(detail::systemErrorText(_impl->_ec.value()), p, _impl->_ec);
    }
}
#endif

GHC_INLINE directory_iterator::directory_iterator(const path& p, std::error_code& ec) noexcept
    : _impl(new impl(p, directory_options::none))
{
    if (_impl->_ec) {
        ec = _impl->_ec;
    }
}

GHC_INLINE directory_iterator::directory_iterator(const path& p, directory_options options, std::error_code& ec) noexcept
    : _impl(new impl(p, options))
{
    if (_impl->_ec) {
        ec = _impl->_ec;
    }
}

GHC_INLINE directory_iterator::directory_iterator(const directory_iterator& rhs)
    : _impl(rhs._impl)
{
}

GHC_INLINE directory_iterator::directory_iterator(directory_iterator&& rhs) noexcept
    : _impl(std::move(rhs._impl))
{
}

GHC_INLINE directory_iterator::~directory_iterator() {}

GHC_INLINE directory_iterator& directory_iterator::operator=(const directory_iterator& rhs)
{
    _impl = rhs._impl;
    return *this;
}

GHC_INLINE directory_iterator& directory_iterator::operator=(directory_iterator&& rhs) noexcept
{
    _impl = std::move(rhs._impl);
    return *this;
}

GHC_INLINE const directory_entry& directory_iterator::operator*() const
{
    return _impl->_dir_entry;
}

GHC_INLINE const directory_entry* directory_iterator::operator->() const
{
    return &_impl->_dir_entry;
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE directory_iterator& directory_iterator::operator++()
{
    std::error_code ec;
    _impl->increment(ec);
    if (ec) {
        throw filesystem_error(detail::systemErrorText(ec.value()), _impl->_current, ec);
    }
    return *this;
}
#endif

GHC_INLINE directory_iterator& directory_iterator::increment(std::error_code& ec) noexcept
{
    _impl->increment(ec);
    return *this;
}

GHC_INLINE bool directory_iterator::operator==(const directory_iterator& rhs) const
{
    return _impl->_current == rhs._impl->_current;
}

GHC_INLINE bool directory_iterator::operator!=(const directory_iterator& rhs) const
{
    return _impl->_current != rhs._impl->_current;
}

// 30.10.13.2 directory_iterator non-member functions

GHC_INLINE directory_iterator begin(directory_iterator iter) noexcept
{
    return iter;
}

GHC_INLINE directory_iterator end(const directory_iterator&) noexcept
{
    return directory_iterator();
}

//-----------------------------------------------------------------------------
// 30.10.14 class recursive_directory_iterator

GHC_INLINE recursive_directory_iterator::recursive_directory_iterator() noexcept
    : _impl(new recursive_directory_iterator_impl(directory_options::none, true))
{
    _impl->_dir_iter_stack.push(directory_iterator());
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE recursive_directory_iterator::recursive_directory_iterator(const path& p)
    : _impl(new recursive_directory_iterator_impl(directory_options::none, true))
{
    _impl->_dir_iter_stack.push(directory_iterator(p));
}

GHC_INLINE recursive_directory_iterator::recursive_directory_iterator(const path& p, directory_options options)
    : _impl(new recursive_directory_iterator_impl(options, true))
{
    _impl->_dir_iter_stack.push(directory_iterator(p, options));
}
#endif

GHC_INLINE recursive_directory_iterator::recursive_directory_iterator(const path& p, directory_options options, std::error_code& ec) noexcept
    : _impl(new recursive_directory_iterator_impl(options, true))
{
    _impl->_dir_iter_stack.push(directory_iterator(p, options, ec));
}

GHC_INLINE recursive_directory_iterator::recursive_directory_iterator(const path& p, std::error_code& ec) noexcept
    : _impl(new recursive_directory_iterator_impl(directory_options::none, true))
{
    _impl->_dir_iter_stack.push(directory_iterator(p, ec));
}

GHC_INLINE recursive_directory_iterator::recursive_directory_iterator(const recursive_directory_iterator& rhs)
    : _impl(rhs._impl)
{
}

GHC_INLINE recursive_directory_iterator::recursive_directory_iterator(recursive_directory_iterator&& rhs) noexcept
    : _impl(std::move(rhs._impl))
{
}

GHC_INLINE recursive_directory_iterator::~recursive_directory_iterator() {}

// 30.10.14.1 observers
GHC_INLINE directory_options recursive_directory_iterator::options() const
{
    return _impl->_options;
}

GHC_INLINE int recursive_directory_iterator::depth() const
{
    return static_cast<int>(_impl->_dir_iter_stack.size() - 1);
}

GHC_INLINE bool recursive_directory_iterator::recursion_pending() const
{
    return _impl->_recursion_pending;
}

GHC_INLINE const directory_entry& recursive_directory_iterator::operator*() const
{
    return *(_impl->_dir_iter_stack.top());
}

GHC_INLINE const directory_entry* recursive_directory_iterator::operator->() const
{
    return &(*(_impl->_dir_iter_stack.top()));
}

// 30.10.14.1 modifiers recursive_directory_iterator&
GHC_INLINE recursive_directory_iterator& recursive_directory_iterator::operator=(const recursive_directory_iterator& rhs)
{
    _impl = rhs._impl;
    return *this;
}

GHC_INLINE recursive_directory_iterator& recursive_directory_iterator::operator=(recursive_directory_iterator&& rhs) noexcept
{
    _impl = std::move(rhs._impl);
    return *this;
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE recursive_directory_iterator& recursive_directory_iterator::operator++()
{
    std::error_code ec;
    increment(ec);
    if (ec) {
        throw filesystem_error(detail::systemErrorText(ec.value()), _impl->_dir_iter_stack.empty() ? path() : _impl->_dir_iter_stack.top()->path(), ec);
    }
    return *this;
}
#endif

GHC_INLINE recursive_directory_iterator& recursive_directory_iterator::increment(std::error_code& ec) noexcept
{
    auto status = (*this)->status(ec);
    if (ec) return *this;
    auto symlink_status = (*this)->symlink_status(ec);
    if (ec) return *this;
    if (recursion_pending() && is_directory(status) && (!is_symlink(symlink_status) || (options() & directory_options::follow_directory_symlink) != directory_options::none)) {
        _impl->_dir_iter_stack.push(directory_iterator((*this)->path(), _impl->_options, ec));
    }
    else {
        _impl->_dir_iter_stack.top().increment(ec);
    }
    if (!ec) {
        while (depth() && _impl->_dir_iter_stack.top() == directory_iterator()) {
            _impl->_dir_iter_stack.pop();
            _impl->_dir_iter_stack.top().increment(ec);
        }
    }
    else if (!_impl->_dir_iter_stack.empty()) {
        _impl->_dir_iter_stack.pop();
    }
    _impl->_recursion_pending = true;
    return *this;
}

#ifdef GHC_WITH_EXCEPTIONS
GHC_INLINE void recursive_directory_iterator::pop()
{
    std::error_code ec;
    pop(ec);
    if (ec) {
        throw filesystem_error(detail::systemErrorText(ec.value()), _impl->_dir_iter_stack.empty() ? path() : _impl->_dir_iter_stack.top()->path(), ec);
    }
}
#endif

GHC_INLINE void recursive_directory_iterator::pop(std::error_code& ec)
{
    if (depth() == 0) {
        *this = recursive_directory_iterator();
    }
    else {
        do {
            _impl->_dir_iter_stack.pop();
            _impl->_dir_iter_stack.top().increment(ec);
        } while (depth() && _impl->_dir_iter_stack.top() == directory_iterator());
    }
}

GHC_INLINE void recursive_directory_iterator::disable_recursion_pending()
{
    _impl->_recursion_pending = false;
}

// other members as required by 27.2.3, input iterators
GHC_INLINE bool recursive_directory_iterator::operator==(const recursive_directory_iterator& rhs) const
{
    return _impl->_dir_iter_stack.top() == rhs._impl->_dir_iter_stack.top();
}

GHC_INLINE bool recursive_directory_iterator::operator!=(const recursive_directory_iterator& rhs) const
{
    return _impl->_dir_iter_stack.top() != rhs._impl->_dir_iter_stack.top();
}

// 30.10.14.2 directory_iterator non-member functions
GHC_INLINE recursive_directory_iterator begin(recursive_directory_iterator iter) noexcept
{
    return iter;
}

GHC_INLINE recursive_directory_iterator end(const recursive_directory_iterator&) noexcept
{
    return recursive_directory_iterator();
}

#endif  // GHC_EXPAND_IMPL

}  // namespace filesystem
}  // namespace ghc

// cleanup some macros
#undef GHC_INLINE
#undef GHC_EXPAND_IMPL

#endif  // GHC_FILESYSTEM_H

namespace fs
{
	using namespace ghc::filesystem;
	using ifstream = ghc::filesystem::ifstream;
	using ofstream = ghc::filesystem::ofstream;
	using fstream = ghc::filesystem::fstream;
}

// file: ext/tinyformat/tinyformat.h
// tinyformat.h
// Copyright (C) 2011, Chris Foster [chris42f (at) gmail (d0t) com]
//
// Boost Software License - Version 1.0
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
//
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

//------------------------------------------------------------------------------
// Tinyformat: A minimal type safe printf replacement
//
// tinyformat.h is a type safe printf replacement library in a single C++
// header file.  Design goals include:
//
// * Type safety and extensibility for user defined types.
// * C99 printf() compatibility, to the extent possible using std::wostream
// * Simplicity and minimalism.  A single header file to include and distribute
//   with your projects.
// * Augment rather than replace the standard stream formatting mechanism
// * C++98 support, with optional C++11 niceties
//
//
// Main interface example usage
// ----------------------------
//
// To print a date to std::wcout:
//
//   std::wstring weekday = L"Wednesday";
//   const wchar_t* month = L"July";
//   size_t day = 27;
//   long hour = 14;
//   int min = 44;
//
//   tfm::printf("%s, %s %d, %.2d:%.2d\n", weekday, month, day, hour, min);
//
// The strange types here emphasize the type safety of the interface; it is
// possible to print a std::wstring using the "%s" conversion, and a
// size_t using the "%d" conversion.  A similar result could be achieved
// using either of the tfm::format() functions.  One prints on a user provided
// stream:
//
//   tfm::format(std::cerr, L"%s, %s %d, %.2d:%.2d\n",
//               weekday, month, day, hour, min);
//
// The other returns a std::wstring:
//
//   std::wstring date = tfm::format(L"%s, %s %d, %.2d:%.2d\n",
//                                  weekday, month, day, hour, min);
//   std::wcout << date;
//
// These are the three primary interface functions.  There is also a
// convenience function printfln() which appends a newline to the usual result
// of printf() for super simple logging.
//
//
// User defined format functions
// -----------------------------
//
// Simulating variadic templates in C++98 is pretty painful since it requires
// writing out the same function for each desired number of arguments.  To make
// this bearable tinyformat comes with a set of macros which are used
// internally to generate the API, but which may also be used in user code.
//
// The three macros TINYFORMAT_ARGTYPES(n), TINYFORMAT_VARARGS(n) and
// TINYFORMAT_PASSARGS(n) will generate a list of n argument types,
// type/name pairs and argument names respectively when called with an integer
// n between 1 and 16.  We can use these to define a macro which generates the
// desired user defined function with n arguments.  To generate all 16 user
// defined function bodies, use the macro TINYFORMAT_FOREACH_ARGNUM.  For an
// example, see the implementation of printf() at the end of the source file.
//
// Sometimes it's useful to be able to pass a list of format arguments through
// to a non-template function.  The FormatList class is provided as a way to do
// this by storing the argument list in a type-opaque way.  Continuing the
// example from above, we construct a FormatList using makeFormatList():
//
//   FormatListRef formatList = tfm::makeFormatList(weekday, month, day, hour, min);
//
// The format list can now be passed into any non-template function and used
// via a call to the vformat() function:
//
//   tfm::vformat(std::wcout, L"%s, %s %d, %.2d:%.2d\n", formatList);
//
//
// Additional API information
// --------------------------
//
// Error handling: Define TINYFORMAT_ERROR to customize the error handling for
// format strings which are unsupported or have the wrong number of format
// specifiers (calls assert() by default).
//
// User defined types: Uses operator<< for user defined types by default.
// Overload formatValue() for more control.


#ifndef TINYFORMAT_H_INCLUDED
#define TINYFORMAT_H_INCLUDED

namespace tinyformat {}
//------------------------------------------------------------------------------
// Config section.  Customize to your liking!

// Namespace alias to encourage brevity
namespace tfm = tinyformat;

// Error handling; calls assert() by default.
// #define TINYFORMAT_ERROR(reasonString) your_error_handler(reasonString)

// Define for C++11 variadic templates which make the code shorter & more
// general.  If you don't define this, C++11 support is autodetected below.
// #define TINYFORMAT_USE_VARIADIC_TEMPLATES


//------------------------------------------------------------------------------
// Implementation details.
#include <algorithm>
#include <iostream>
#include <sstream>

#ifndef TINYFORMAT_ASSERT
#   include <cassert>
#   define TINYFORMAT_ASSERT(cond) assert(cond)
#endif

#define TINYFORMAT_ALLOW_WCHAR_STRINGS
#define TINYFORMAT_USE_VARIADIC_TEMPLATES

#ifndef TINYFORMAT_ERROR
#   include <cassert>
#   define TINYFORMAT_ERROR(reason) assert(0 && reason)
#endif

#if !defined(TINYFORMAT_USE_VARIADIC_TEMPLATES) && !defined(TINYFORMAT_NO_VARIADIC_TEMPLATES)
#   ifdef __GXX_EXPERIMENTAL_CXX0X__
#       define TINYFORMAT_USE_VARIADIC_TEMPLATES
#   endif
#endif

#if defined(__GLIBCXX__) && __GLIBCXX__ < 20080201
//  std::showpos is broken on old libstdc++ as provided with OSX.  See
//  http://gcc.gnu.org/ml/libstdc++/2007-11/msg00075.html
#   define TINYFORMAT_OLD_LIBSTDCPLUSPLUS_WORKAROUND
#endif

#ifdef __APPLE__
// Workaround OSX linker warning: xcode uses different default symbol
// visibilities for static libs vs executables (see issue #25)
#   define TINYFORMAT_HIDDEN __attribute__((visibility("hidden")))
#else
#   define TINYFORMAT_HIDDEN
#endif

namespace tinyformat {

//------------------------------------------------------------------------------
namespace detail {

// Test whether type T1 is convertible to type T2
template <typename T1, typename T2>
struct is_convertible
{
    private:
        // two types of different size
        struct fail { wchar_t dummy[2]; };
        struct succeed { wchar_t dummy; };
        // Try to convert a T1 to a T2 by plugging into tryConvert
        static fail tryConvert(...);
        static succeed tryConvert(const T2&);
        static const T1& makeT1();
    public:
#       ifdef _MSC_VER
        // Disable spurious loss of precision warnings in tryConvert(makeT1())
#       pragma warning(push)
#       pragma warning(disable:4244)
#       pragma warning(disable:4267)
#       endif
        // Standard trick: the (...) version of tryConvert will be chosen from
        // the overload set only if the version taking a T2 doesn't match.
        // Then we compare the sizes of the return types to check which
        // function matched.  Very neat, in a disgusting kind of way :)
        static const bool value =
            sizeof(tryConvert(makeT1())) == sizeof(succeed);
#       ifdef _MSC_VER
#       pragma warning(pop)
#       endif
};


// Detect when a type is not a wchar_t string
template<typename T> struct is_wchar { typedef int tinyformat_wchar_is_not_supported; };
template<> struct is_wchar<wchar_t*> {};
template<> struct is_wchar<const wchar_t*> {};
template<int n> struct is_wchar<const wchar_t[n]> {};
template<int n> struct is_wchar<wchar_t[n]> {};


// Format the value by casting to type fmtT.  This default implementation
// should never be called.
template<typename T, typename fmtT, bool convertible = is_convertible<T, fmtT>::value>
struct formatValueAsType
{
    static void invoke(std::wostream& /*out*/, const T& /*value*/) { TINYFORMAT_ASSERT(0); }
};
// Specialized version for types that can actually be converted to fmtT, as
// indicated by the "convertible" template parameter.
template<typename T, typename fmtT>
struct formatValueAsType<T,fmtT,true>
{
    static void invoke(std::wostream& out, const T& value)
        { out << static_cast<fmtT>(value); }
};

#ifdef TINYFORMAT_OLD_LIBSTDCPLUSPLUS_WORKAROUND
template<typename T, bool convertible = is_convertible<T, int>::value>
struct formatZeroIntegerWorkaround
{
    static bool invoke(std::wostream& /**/, const T& /**/) { return false; }
};
template<typename T>
struct formatZeroIntegerWorkaround<T,true>
{
    static bool invoke(std::wostream& out, const T& value)
    {
        if (static_cast<int>(value) == 0 && out.flags() & std::ios::showpos)
        {
            out << "+0";
            return true;
        }
        return false;
    }
};
#endif // TINYFORMAT_OLD_LIBSTDCPLUSPLUS_WORKAROUND

// Convert an arbitrary type to integer.  The version with convertible=false
// throws an error.
template<typename T, bool convertible = is_convertible<T,int>::value>
struct convertToInt
{
    static int invoke(const T& /*value*/)
    {
        TINYFORMAT_ERROR("tinyformat: Cannot convert from argument type to "
                         "integer for use as variable width or precision");
        return 0;
    }
};
// Specialization for convertToInt when conversion is possible
template<typename T>
struct convertToInt<T,true>
{
    static int invoke(const T& value) { return static_cast<int>(value); }
};

// Format at most ntrunc wchar_tacters to the given stream.
template<typename T>
inline void formatTruncated(std::wostream& out, const T& value, int ntrunc)
{
    std::wostringstream tmp;
    tmp << value;
    std::wstring result = tmp.str();
    out.write(result.c_str(), (std::min)(ntrunc, static_cast<int>(result.size())));
}
#define TINYFORMAT_DEFINE_FORMAT_TRUNCATED_CSTR(type)       \
inline void formatTruncated(std::wostream& out, type* value, int ntrunc) \
{                                                           \
    std::streamsize len = 0;                                \
    while(len < ntrunc && value[len] != 0)                  \
        ++len;                                              \
    out.write(value, len);                                  \
}
// Overload for const wchar_t* and wchar_t*.  Could overload for signed & unsigned
// wchar_t too, but these are technically unneeded for printf compatibility.
TINYFORMAT_DEFINE_FORMAT_TRUNCATED_CSTR(const wchar_t)
TINYFORMAT_DEFINE_FORMAT_TRUNCATED_CSTR(wchar_t)
#undef TINYFORMAT_DEFINE_FORMAT_TRUNCATED_CSTR

} // namespace detail


//------------------------------------------------------------------------------
// Variable formatting functions.  May be overridden for user-defined types if
// desired.


/// Format a value into a stream, delegating to operator<< by default.
///
/// Users may override this for their own types.  When this function is called,
/// the stream flags will have been modified according to the format string.
/// The format specification is provided in the range [fmtBegin, fmtEnd).  For
/// truncating conversions, ntrunc is set to the desired maximum number of
/// characters, for example "%.7s" calls formatValue with ntrunc = 7.
///
/// By default, formatValue() uses the usual stream insertion operator
/// operator<< to format the type T, with special cases for the %c and %p
/// conversions.
template<typename T>
inline void formatValue(std::wostream& out, const wchar_t* /*fmtBegin*/,
                        const wchar_t* fmtEnd, int ntrunc, const T& value)
{
#ifndef TINYFORMAT_ALLOW_WCHAR_STRINGS
    // Since we don't support printing of wchar_t using "%ls", make it fail at
    // compile time in preference to printing as a void* at runtime.
    typedef typename detail::is_wchar<T>::tinyformat_wchar_is_not_supported DummyType;
    (void) DummyType(); // avoid unused type warning with gcc-4.8
#endif
    // The mess here is to support the %c and %p conversions: if these
    // conversions are active we try to convert the type to a wchar_t or const
    // void* respectively and format that instead of the value itself.  For the
    // %p conversion it's important to avoid dereferencing the pointer, which
    // could otherwise lead to a crash when printing a dangling (const wchar_t*).
    const bool canConvertToChar = detail::is_convertible<T,wchar_t>::value;
    const bool canConvertToVoidPtr = detail::is_convertible<T, const void*>::value;
    if(canConvertToChar && *(fmtEnd-1) == 'c')
        detail::formatValueAsType<T, wchar_t>::invoke(out, value);
    else if(canConvertToVoidPtr && *(fmtEnd-1) == 'p')
        detail::formatValueAsType<T, const void*>::invoke(out, value);
#ifdef TINYFORMAT_OLD_LIBSTDCPLUSPLUS_WORKAROUND
    else if(detail::formatZeroIntegerWorkaround<T>::invoke(out, value)) /**/;
#endif
    else if(ntrunc >= 0)
    {
        // Take care not to overread C strings in truncating conversions like
        // "%.4s" where at most 4 wchar_tacters may be read.
        detail::formatTruncated(out, value, ntrunc);
    }
    else
        out << value;
}


// Overloaded version for wchar_t types to support printing as an integer
#define TINYFORMAT_DEFINE_FORMATVALUE_CHAR(wchar_tType)                  \
inline void formatValue(std::wostream& out, const wchar_t* /*fmtBegin*/,  \
                        const wchar_t* fmtEnd, int /**/, wchar_tType value) \
{                                                                     \
    switch(*(fmtEnd-1))                                               \
    {                                                                 \
        case 'u': case 'd': case 'i': case 'o': case 'X': case 'x':   \
            out << static_cast<int>(value); break;                    \
        default:                                                      \
            out << value;                   break;                    \
    }                                                                 \
}
// per 3.9.1: char, signed char and unsigned char are all distinct types
TINYFORMAT_DEFINE_FORMATVALUE_CHAR(char)
TINYFORMAT_DEFINE_FORMATVALUE_CHAR(signed char)
TINYFORMAT_DEFINE_FORMATVALUE_CHAR(unsigned char)
#undef TINYFORMAT_DEFINE_FORMATVALUE_CHAR


//------------------------------------------------------------------------------
// Tools for emulating variadic templates in C++98.  The basic idea here is
// stolen from the boost preprocessor metaprogramming library and cut down to
// be just general enough for what we need.

#define TINYFORMAT_ARGTYPES(n) TINYFORMAT_ARGTYPES_ ## n
#define TINYFORMAT_VARARGS(n) TINYFORMAT_VARARGS_ ## n
#define TINYFORMAT_PASSARGS(n) TINYFORMAT_PASSARGS_ ## n
#define TINYFORMAT_PASSARGS_TAIL(n) TINYFORMAT_PASSARGS_TAIL_ ## n

// To keep it as transparent as possible, the macros below have been generated
// using python via the excellent cog.py code generation script.  This avoids
// the need for a bunch of complex (but more general) preprocessor tricks as
// used in boost.preprocessor.
//
// To rerun the code generation in place, use `cog.py -r tinyformat.h`
// (see http://nedbatchelder.com/code/cog).  Alternatively you can just create
// extra versions by hand.

/*[[[cog
maxParams = 16

def makeCommaSepLists(lineTemplate, elemTemplate, startInd=1):
    for j in range(startInd,maxParams+1):
        list = ', '.join([elemTemplate % {'i':i} for i in range(startInd,j+1)])
        cog.outl(lineTemplate % {'j':j, 'list':list})

makeCommaSepLists('#define TINYFORMAT_ARGTYPES_%(j)d %(list)s',
                  'class T%(i)d')

cog.outl()
makeCommaSepLists('#define TINYFORMAT_VARARGS_%(j)d %(list)s',
                  'const T%(i)d& v%(i)d')

cog.outl()
makeCommaSepLists('#define TINYFORMAT_PASSARGS_%(j)d %(list)s', 'v%(i)d')

cog.outl()
cog.outl('#define TINYFORMAT_PASSARGS_TAIL_1')
makeCommaSepLists('#define TINYFORMAT_PASSARGS_TAIL_%(j)d , %(list)s',
                  'v%(i)d', startInd = 2)

cog.outl()
cog.outl('#define TINYFORMAT_FOREACH_ARGNUM(m) \\\n    ' +
         ' '.join(['m(%d)' % (j,) for j in range(1,maxParams+1)]))
]]]*/
#define TINYFORMAT_ARGTYPES_1 class T1
#define TINYFORMAT_ARGTYPES_2 class T1, class T2
#define TINYFORMAT_ARGTYPES_3 class T1, class T2, class T3
#define TINYFORMAT_ARGTYPES_4 class T1, class T2, class T3, class T4
#define TINYFORMAT_ARGTYPES_5 class T1, class T2, class T3, class T4, class T5
#define TINYFORMAT_ARGTYPES_6 class T1, class T2, class T3, class T4, class T5, class T6
#define TINYFORMAT_ARGTYPES_7 class T1, class T2, class T3, class T4, class T5, class T6, class T7
#define TINYFORMAT_ARGTYPES_8 class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8
#define TINYFORMAT_ARGTYPES_9 class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9
#define TINYFORMAT_ARGTYPES_10 class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10
#define TINYFORMAT_ARGTYPES_11 class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11
#define TINYFORMAT_ARGTYPES_12 class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12
#define TINYFORMAT_ARGTYPES_13 class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13
#define TINYFORMAT_ARGTYPES_14 class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14
#define TINYFORMAT_ARGTYPES_15 class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15
#define TINYFORMAT_ARGTYPES_16 class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16

#define TINYFORMAT_VARARGS_1 const T1& v1
#define TINYFORMAT_VARARGS_2 const T1& v1, const T2& v2
#define TINYFORMAT_VARARGS_3 const T1& v1, const T2& v2, const T3& v3
#define TINYFORMAT_VARARGS_4 const T1& v1, const T2& v2, const T3& v3, const T4& v4
#define TINYFORMAT_VARARGS_5 const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5
#define TINYFORMAT_VARARGS_6 const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6
#define TINYFORMAT_VARARGS_7 const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7
#define TINYFORMAT_VARARGS_8 const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7, const T8& v8
#define TINYFORMAT_VARARGS_9 const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7, const T8& v8, const T9& v9
#define TINYFORMAT_VARARGS_10 const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7, const T8& v8, const T9& v9, const T10& v10
#define TINYFORMAT_VARARGS_11 const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7, const T8& v8, const T9& v9, const T10& v10, const T11& v11
#define TINYFORMAT_VARARGS_12 const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7, const T8& v8, const T9& v9, const T10& v10, const T11& v11, const T12& v12
#define TINYFORMAT_VARARGS_13 const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7, const T8& v8, const T9& v9, const T10& v10, const T11& v11, const T12& v12, const T13& v13
#define TINYFORMAT_VARARGS_14 const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7, const T8& v8, const T9& v9, const T10& v10, const T11& v11, const T12& v12, const T13& v13, const T14& v14
#define TINYFORMAT_VARARGS_15 const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7, const T8& v8, const T9& v9, const T10& v10, const T11& v11, const T12& v12, const T13& v13, const T14& v14, const T15& v15
#define TINYFORMAT_VARARGS_16 const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6, const T7& v7, const T8& v8, const T9& v9, const T10& v10, const T11& v11, const T12& v12, const T13& v13, const T14& v14, const T15& v15, const T16& v16

#define TINYFORMAT_PASSARGS_1 v1
#define TINYFORMAT_PASSARGS_2 v1, v2
#define TINYFORMAT_PASSARGS_3 v1, v2, v3
#define TINYFORMAT_PASSARGS_4 v1, v2, v3, v4
#define TINYFORMAT_PASSARGS_5 v1, v2, v3, v4, v5
#define TINYFORMAT_PASSARGS_6 v1, v2, v3, v4, v5, v6
#define TINYFORMAT_PASSARGS_7 v1, v2, v3, v4, v5, v6, v7
#define TINYFORMAT_PASSARGS_8 v1, v2, v3, v4, v5, v6, v7, v8
#define TINYFORMAT_PASSARGS_9 v1, v2, v3, v4, v5, v6, v7, v8, v9
#define TINYFORMAT_PASSARGS_10 v1, v2, v3, v4, v5, v6, v7, v8, v9, v10
#define TINYFORMAT_PASSARGS_11 v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11
#define TINYFORMAT_PASSARGS_12 v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12
#define TINYFORMAT_PASSARGS_13 v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13
#define TINYFORMAT_PASSARGS_14 v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14
#define TINYFORMAT_PASSARGS_15 v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15
#define TINYFORMAT_PASSARGS_16 v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16

#define TINYFORMAT_PASSARGS_TAIL_1
#define TINYFORMAT_PASSARGS_TAIL_2 , v2
#define TINYFORMAT_PASSARGS_TAIL_3 , v2, v3
#define TINYFORMAT_PASSARGS_TAIL_4 , v2, v3, v4
#define TINYFORMAT_PASSARGS_TAIL_5 , v2, v3, v4, v5
#define TINYFORMAT_PASSARGS_TAIL_6 , v2, v3, v4, v5, v6
#define TINYFORMAT_PASSARGS_TAIL_7 , v2, v3, v4, v5, v6, v7
#define TINYFORMAT_PASSARGS_TAIL_8 , v2, v3, v4, v5, v6, v7, v8
#define TINYFORMAT_PASSARGS_TAIL_9 , v2, v3, v4, v5, v6, v7, v8, v9
#define TINYFORMAT_PASSARGS_TAIL_10 , v2, v3, v4, v5, v6, v7, v8, v9, v10
#define TINYFORMAT_PASSARGS_TAIL_11 , v2, v3, v4, v5, v6, v7, v8, v9, v10, v11
#define TINYFORMAT_PASSARGS_TAIL_12 , v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12
#define TINYFORMAT_PASSARGS_TAIL_13 , v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13
#define TINYFORMAT_PASSARGS_TAIL_14 , v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14
#define TINYFORMAT_PASSARGS_TAIL_15 , v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15
#define TINYFORMAT_PASSARGS_TAIL_16 , v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16

#define TINYFORMAT_FOREACH_ARGNUM(m) \
    m(1) m(2) m(3) m(4) m(5) m(6) m(7) m(8) m(9) m(10) m(11) m(12) m(13) m(14) m(15) m(16)
//[[[end]]]



namespace detail {

// Type-opaque holder for an argument to format(), with associated actions on
// the type held as explicit function pointers.  This allows FormatArg's for
// each argument to be allocated as a homogenous array inside FormatList
// whereas a naive implementation based on inheritance does not.
class FormatArg
{
    public:
        FormatArg()
            : m_value(NULL),
            m_formatImpl(NULL),
            m_toIntImpl(NULL)
        { }

        template<typename T>
        FormatArg(const T& value)
            : m_value(static_cast<const void*>(&value)),
            m_formatImpl(&formatImpl<T>),
            m_toIntImpl(&toIntImpl<T>)
        { }

        void format(std::wostream& out, const wchar_t* fmtBegin,
                    const wchar_t* fmtEnd, int ntrunc) const
        {
            TINYFORMAT_ASSERT(m_value);
            TINYFORMAT_ASSERT(m_formatImpl);
            m_formatImpl(out, fmtBegin, fmtEnd, ntrunc, m_value);
        }

        int toInt() const
        {
            TINYFORMAT_ASSERT(m_value);
            TINYFORMAT_ASSERT(m_toIntImpl);
            return m_toIntImpl(m_value);
        }

    private:
        template<typename T>
        TINYFORMAT_HIDDEN static void formatImpl(std::wostream& out, const wchar_t* fmtBegin,
                        const wchar_t* fmtEnd, int ntrunc, const void* value)
        {
            formatValue(out, fmtBegin, fmtEnd, ntrunc, *static_cast<const T*>(value));
        }

        template<typename T>
        TINYFORMAT_HIDDEN static int toIntImpl(const void* value)
        {
            return convertToInt<T>::invoke(*static_cast<const T*>(value));
        }

        const void* m_value;
        void (*m_formatImpl)(std::wostream& out, const wchar_t* fmtBegin,
                             const wchar_t* fmtEnd, int ntrunc, const void* value);
        int (*m_toIntImpl)(const void* value);
};


// Parse and return an integer from the string c, as atoi()
// On return, c is set to one past the end of the integer.
inline int parseIntAndAdvance(const wchar_t*& c)
{
    int i = 0;
    for(;*c >= '0' && *c <= '9'; ++c)
        i = 10*i + (*c - '0');
    return i;
}

// Print literal part of format string and return next format spec
// position.
//
// Skips over any occurrences of '%%', printing a literal '%' to the
// output.  The position of the first % character of the next
// nontrivial format spec is returned, or the end of string.
inline const wchar_t* printFormatStringLiteral(std::wostream& out, const wchar_t* fmt)
{
    const wchar_t* c = fmt;
    for(;; ++c)
    {
        switch(*c)
        {
            case '\0':
                out.write(fmt, c - fmt);
                return c;
            case '%':
                out.write(fmt, c - fmt);
                if(*(c+1) != '%')
                    return c;
                // for "%%", tack trailing % onto next literal section.
                fmt = ++c;
                break;
            default:
                break;
        }
    }
}


// Parse a format string and set the stream state accordingly.
//
// The format mini-language recognized here is meant to be the one from C99,
// with the form "%[flags][width][.precision][length]type".
//
// Formatting options which can't be natively represented using the ostream
// state are returned in spacePadPositive (for space padded positive numbers)
// and ntrunc (for truncating conversions).  argIndex is incremented if
// necessary to pull out variable width and precision .  The function returns a
// pointer to the wchar_tacter after the end of the current format spec.
inline const wchar_t* streamStateFromFormat(std::wostream& out, bool& spacePadPositive,
                                         int& ntrunc, const wchar_t* fmtStart,
                                         const detail::FormatArg* formatters,
                                         int& argIndex, int numFormatters)
{
    if(*fmtStart != '%')
    {
        TINYFORMAT_ERROR("tinyformat: Not enough conversion specifiers in format string");
        return fmtStart;
    }
    // Reset stream state to defaults.
    out.width(0);
    out.precision(6);
    out.fill(' ');
    // Reset most flags; ignore irrelevant unitbuf & skipws.
    out.unsetf(std::ios::adjustfield | std::ios::basefield |
               std::ios::floatfield | std::ios::showbase | std::ios::boolalpha |
               std::ios::showpoint | std::ios::showpos | std::ios::uppercase);
    bool precisionSet = false;
    bool widthSet = false;
    int widthExtra = 0;
    const wchar_t* c = fmtStart + 1;
    // 1) Parse flags
    for(;; ++c)
    {
        switch(*c)
        {
            case '#':
                out.setf(std::ios::showpoint | std::ios::showbase);
                continue;
            case '0':
                // overridden by left alignment ('-' flag)
                if(!(out.flags() & std::ios::left))
                {
                    // Use internal padding so that numeric values are
                    // formatted correctly, eg -00010 rather than 000-10
                    out.fill('0');
                    out.setf(std::ios::internal, std::ios::adjustfield);
                }
                continue;
            case '-':
                out.fill(' ');
                out.setf(std::ios::left, std::ios::adjustfield);
                continue;
            case ' ':
                // overridden by show positive sign, '+' flag.
                if(!(out.flags() & std::ios::showpos))
                    spacePadPositive = true;
                continue;
            case '+':
                out.setf(std::ios::showpos);
                spacePadPositive = false;
                widthExtra = 1;
                continue;
            default:
                break;
        }
        break;
    }
    // 2) Parse width
    if(*c >= '0' && *c <= '9')
    {
        widthSet = true;
        out.width(parseIntAndAdvance(c));
    }
    if(*c == '*')
    {
        widthSet = true;
        int width = 0;
        if(argIndex < numFormatters)
            width = formatters[argIndex++].toInt();
        else
            TINYFORMAT_ERROR("tinyformat: Not enough arguments to read variable width");
        if(width < 0)
        {
            // negative widths correspond to '-' flag set
            out.fill(' ');
            out.setf(std::ios::left, std::ios::adjustfield);
            width = -width;
        }
        out.width(width);
        ++c;
    }
    // 3) Parse precision
    if(*c == '.')
    {
        ++c;
        int precision = 0;
        if(*c == '*')
        {
            ++c;
            if(argIndex < numFormatters)
                precision = formatters[argIndex++].toInt();
            else
                TINYFORMAT_ERROR("tinyformat: Not enough arguments to read variable precision");
        }
        else
        {
            if(*c >= '0' && *c <= '9')
                precision = parseIntAndAdvance(c);
            else if(*c == '-') // negative precisions ignored, treated as zero.
                parseIntAndAdvance(++c);
        }
        out.precision(precision);
        precisionSet = true;
    }
    // 4) Ignore any C99 length modifier
    while(*c == 'l' || *c == 'h' || *c == 'L' ||
          *c == 'j' || *c == 'z' || *c == 't')
        ++c;
    // 5) We're up to the conversion specifier character.
    // Set stream flags based on conversion specifier (thanks to the
    // boost::format class for forging the way here).
    bool intConversion = false;
    switch(*c)
    {
        case 'u': case 'd': case 'i':
            out.setf(std::ios::dec, std::ios::basefield);
            intConversion = true;
            break;
        case 'o':
            out.setf(std::ios::oct, std::ios::basefield);
            intConversion = true;
            break;
        case 'X':
            out.setf(std::ios::uppercase);
            // Falls through
        case 'x': case 'p':
            out.setf(std::ios::hex, std::ios::basefield);
            intConversion = true;
            break;
        case 'E':
            out.setf(std::ios::uppercase);
            // Falls through
        case 'e':
            out.setf(std::ios::scientific, std::ios::floatfield);
            out.setf(std::ios::dec, std::ios::basefield);
            break;
        case 'F':
            out.setf(std::ios::uppercase);
            // Falls through
        case 'f':
            out.setf(std::ios::fixed, std::ios::floatfield);
            break;
        case 'G':
            out.setf(std::ios::uppercase);
            // Falls through
        case 'g':
            out.setf(std::ios::dec, std::ios::basefield);
            // As in boost::format, let stream decide float format.
            out.flags(out.flags() & ~std::ios::floatfield);
            break;
        case 'a': case 'A':
            TINYFORMAT_ERROR("tinyformat: the %a and %A conversion specs "
                             "are not supported");
            break;
        case 'c':
            // Handled as special case inside formatValue()
            break;
        case 's':
            if(precisionSet)
                ntrunc = static_cast<int>(out.precision());
            // Make %s print booleans as "true" and "false"
            out.setf(std::ios::boolalpha);
            break;
        case 'n':
            // Not supported - will cause problems!
            TINYFORMAT_ERROR("tinyformat: %n conversion spec not supported");
            break;
        case '\0':
            TINYFORMAT_ERROR("tinyformat: Conversion spec incorrectly "
                             "terminated by end of string");
            return c;
        default:
            break;
    }
    if(intConversion && precisionSet && !widthSet)
    {
        // "precision" for integers gives the minimum number of digits (to be
        // padded with zeros on the left).  This isn't really supported by the
        // iostreams, but we can approximately simulate it with the width if
        // the width isn't otherwise used.
        out.width(out.precision() + widthExtra);
        out.setf(std::ios::internal, std::ios::adjustfield);
        out.fill('0');
    }
    return c+1;
}


//------------------------------------------------------------------------------
inline void formatImpl(std::wostream& out, const wchar_t* fmt,
                       const detail::FormatArg* formatters,
                       int numFormatters)
{
    // Saved stream state
    std::streamsize origWidth = out.width();
    std::streamsize origPrecision = out.precision();
    std::ios::fmtflags origFlags = out.flags();
    wchar_t origFill = out.fill();

    for (int argIndex = 0; argIndex < numFormatters; ++argIndex)
    {
        // Parse the format string
        fmt = printFormatStringLiteral(out, fmt);
        bool spacePadPositive = false;
        int ntrunc = -1;
        const wchar_t* fmtEnd = streamStateFromFormat(out, spacePadPositive, ntrunc, fmt,
                                                   formatters, argIndex, numFormatters);
        if (argIndex >= numFormatters)
        {
            // Check args remain after reading any variable width/precision
            TINYFORMAT_ERROR("tinyformat: Not enough format arguments");
            return;
        }
        const FormatArg& arg = formatters[argIndex];
        // Format the arg into the stream.
        if(!spacePadPositive)
            arg.format(out, fmt, fmtEnd, ntrunc);
        else
        {
            // The following is a special case with no direct correspondence
            // between stream formatting and the printf() behaviour.  Simulate
            // it crudely by formatting into a temporary string stream and
            // munging the resulting string.
            std::wostringstream tmpStream;
            tmpStream.copyfmt(out);
            tmpStream.setf(std::ios::showpos);
            arg.format(tmpStream, fmt, fmtEnd, ntrunc);
            std::wstring result = tmpStream.str(); // allocates... yuck.
            for(size_t i = 0, iend = result.size(); i < iend; ++i)
                if(result[i] == '+') result[i] = ' ';
            out << result;
        }
        fmt = fmtEnd;
    }

    // Print remaining part of format string.
    fmt = printFormatStringLiteral(out, fmt);
    if(*fmt != '\0')
        TINYFORMAT_ERROR("tinyformat: Too many conversion specifiers in format string");

    // Restore stream state
    out.width(origWidth);
    out.precision(origPrecision);
    out.flags(origFlags);
    out.fill(origFill);
}

} // namespace detail


/// List of template arguments format(), held in a type-opaque way.
///
/// A const reference to FormatList (typedef'd as FormatListRef) may be
/// conveniently used to pass arguments to non-template functions: All type
/// information has been stripped from the arguments, leaving just enough of a
/// common interface to perform formatting as required.
class FormatList
{
    public:
        FormatList(detail::FormatArg* formatters, int N)
            : m_formatters(formatters), m_N(N) { }

        friend void vformat(std::wostream& out, const wchar_t* fmt,
                            const FormatList& list);

    private:
        const detail::FormatArg* m_formatters;
        int m_N;
};

/// Reference to type-opaque format list for passing to vformat()
typedef const FormatList& FormatListRef;


namespace detail {

// Format list subclass with fixed storage to avoid dynamic allocation
template<int N>
class FormatListN : public FormatList
{
    public:
#ifdef TINYFORMAT_USE_VARIADIC_TEMPLATES
        template<typename... Args>
        FormatListN(const Args&... args)
            : FormatList(&m_formatterStore[0], N),
            m_formatterStore { FormatArg(args)... }
        { static_assert(sizeof...(args) == N, "Number of args must be N"); }
#else // C++98 version
        void init(int) {}
#       define TINYFORMAT_MAKE_FORMATLIST_CONSTRUCTOR(n)                \
                                                                        \
        template<TINYFORMAT_ARGTYPES(n)>                                \
        FormatListN(TINYFORMAT_VARARGS(n))                              \
            : FormatList(&m_formatterStore[0], n)                       \
        { TINYFORMAT_ASSERT(n == N); init(0, TINYFORMAT_PASSARGS(n)); } \
                                                                        \
        template<TINYFORMAT_ARGTYPES(n)>                                \
        void init(int i, TINYFORMAT_VARARGS(n))                         \
        {                                                               \
            m_formatterStore[i] = FormatArg(v1);                        \
            init(i+1 TINYFORMAT_PASSARGS_TAIL(n));                      \
        }

        TINYFORMAT_FOREACH_ARGNUM(TINYFORMAT_MAKE_FORMATLIST_CONSTRUCTOR)
#       undef TINYFORMAT_MAKE_FORMATLIST_CONSTRUCTOR
#endif

    private:
        FormatArg m_formatterStore[N];
};

// Special 0-arg version - MSVC says zero-sized C array in struct is nonstandard
template<> class FormatListN<0> : public FormatList
{
    public: FormatListN() : FormatList(0, 0) {}
};

} // namespace detail


//------------------------------------------------------------------------------
// Primary API functions

#ifdef TINYFORMAT_USE_VARIADIC_TEMPLATES

/// Make type-agnostic format list from list of template arguments.
///
/// The exact return type of this function is an implementation detail and
/// shouldn't be relied upon.  Instead it should be stored as a FormatListRef:
///
///   FormatListRef formatList = makeFormatList( /*...*/ );
template<typename... Args>
detail::FormatListN<sizeof...(Args)> makeFormatList(const Args&... args)
{
    return detail::FormatListN<sizeof...(args)>(args...);
}

#else // C++98 version

inline detail::FormatListN<0> makeFormatList()
{
    return detail::FormatListN<0>();
}
#define TINYFORMAT_MAKE_MAKEFORMATLIST(n)                     \
template<TINYFORMAT_ARGTYPES(n)>                              \
detail::FormatListN<n> makeFormatList(TINYFORMAT_VARARGS(n))  \
{                                                             \
    return detail::FormatListN<n>(TINYFORMAT_PASSARGS(n));    \
}
TINYFORMAT_FOREACH_ARGNUM(TINYFORMAT_MAKE_MAKEFORMATLIST)
#undef TINYFORMAT_MAKE_MAKEFORMATLIST

#endif

/// Format list of arguments to the stream according to the given format string.
///
/// The name vformat() is chosen for the semantic similarity to vprintf(): the
/// list of format arguments is held in a single function argument.
inline void vformat(std::wostream& out, const wchar_t* fmt, FormatListRef list)
{
    detail::formatImpl(out, fmt, list.m_formatters, list.m_N);
}


#ifdef TINYFORMAT_USE_VARIADIC_TEMPLATES

/// Format list of arguments to the stream according to given format string.
template<typename... Args>
void format(std::wostream& out, const wchar_t* fmt, const Args&... args)
{
    vformat(out, fmt, makeFormatList(args...));
}

/// Format list of arguments according to the given format string and return
/// the result as a string.
template<typename... Args>
std::wstring format(const wchar_t* fmt, const Args&... args)
{
    std::wostringstream oss;
    format(oss, fmt, args...);
    return oss.str();
}

/// Format list of arguments to std::wcout, according to the given format string
template<typename... Args>
void printf(const wchar_t* fmt, const Args&... args)
{
    format(std::wcout, fmt, args...);
}

template<typename... Args>
void printfln(const wchar_t* fmt, const Args&... args)
{
    format(std::wcout, fmt, args...);
    std::wcout << '\n';
}


#else // C++98 version

inline void format(std::wostream& out, const wchar_t* fmt)
{
    vformat(out, fmt, makeFormatList());
}

inline std::wstring format(const wchar_t* fmt)
{
    std::wostringstream oss;
    format(oss, fmt);
    return oss.str();
}

inline void printf(const wchar_t* fmt)
{
    format(std::wcout, fmt);
}

inline void printfln(const wchar_t* fmt)
{
    format(std::wcout, fmt);
    std::wcout << '\n';
}

#define TINYFORMAT_MAKE_FORMAT_FUNCS(n)                                   \
                                                                          \
template<TINYFORMAT_ARGTYPES(n)>                                          \
void format(std::wostream& out, const wchar_t* fmt, TINYFORMAT_VARARGS(n))    \
{                                                                         \
    vformat(out, fmt, makeFormatList(TINYFORMAT_PASSARGS(n)));            \
}                                                                         \
                                                                          \
template<TINYFORMAT_ARGTYPES(n)>                                          \
std::wstring format(const wchar_t* fmt, TINYFORMAT_VARARGS(n))                \
{                                                                         \
    std::wostringstream oss;                                               \
    format(oss, fmt, TINYFORMAT_PASSARGS(n));                             \
    return oss.str();                                                     \
}                                                                         \
                                                                          \
template<TINYFORMAT_ARGTYPES(n)>                                          \
void printf(const wchar_t* fmt, TINYFORMAT_VARARGS(n))                       \
{                                                                         \
    format(std::wcout, fmt, TINYFORMAT_PASSARGS(n));                       \
}                                                                         \
                                                                          \
template<TINYFORMAT_ARGTYPES(n)>                                          \
void printfln(const wchar_t* fmt, TINYFORMAT_VARARGS(n))                     \
{                                                                         \
    format(std::wcout, fmt, TINYFORMAT_PASSARGS(n));                       \
    std::wcout << '\n';                                                    \
}

TINYFORMAT_FOREACH_ARGNUM(TINYFORMAT_MAKE_FORMAT_FUNCS)
#undef TINYFORMAT_MAKE_FORMAT_FUNCS

#endif


} // namespace tinyformat

#endif // TINYFORMAT_H_INCLUDED

// file: Commands/CAssemblerCommand.h

class TempData;
class SymbolData;

struct ValidateState
{
	bool noFileChange = false;
	const wchar_t *noFileChangeDirective = nullptr;
	int passes = 0;
};

class CAssemblerCommand
{
public:
	CAssemblerCommand();
	virtual ~CAssemblerCommand() { };
	virtual bool Validate(const ValidateState &state) = 0;
	virtual void Encode() const = 0;
	virtual void writeTempData(TempData& tempData) const = 0;
	virtual void writeSymData(SymbolData& symData) const { };
	void applyFileInfo();
	int getSection() { return section; }
	void updateSection(int num) { section = num; }
protected:
	int FileNum;
	int FileLine;
private:
	int section;
};

class DummyCommand: public CAssemblerCommand
{
public:
	bool Validate(const ValidateState &state) override { return false; }
	void Encode() const override { };
	void writeTempData(TempData& tempData) const override { };
	void writeSymData(SymbolData& symData) const override { };
};

class InvalidCommand: public CAssemblerCommand
{
public:
	bool Validate(const ValidateState &state) override { return false; }
	void Encode() const override { };
	void writeTempData(TempData& tempData) const override { };
	void writeSymData(SymbolData& symData) const override { };
};

// file: Core/Expression.h

#include <memory>
#include <string>
#include <vector>

class Label;

struct ExpressionFunctionEntry;
struct ExpressionLabelFunctionEntry;

enum class OperatorType
{
	Invalid,
	Integer,
	Float,
	Identifier,
	String,
	MemoryPos,
	Add,
	Sub,
	Mult,
	Div,
	Mod,
	Neg,
	LogNot,
	BitNot,
	LeftShift,
	RightShift,
	Less,
	Greater,
	LessEqual,
	GreaterEqual,
	Equal,
	NotEqual,
	BitAnd,
	Xor,
	BitOr,
	LogAnd,
	LogOr,
	TertiaryIf,
	ToString,
	FunctionCall
};

enum class ExpressionValueType { Invalid, Integer, Float, String};

struct ExpressionValue
{
	ExpressionValueType type;

	ExpressionValue()
	{
		type = ExpressionValueType::Invalid;
		intValue = 0;
	}

	ExpressionValue(int64_t value)
	{
		type = ExpressionValueType::Integer;
		intValue = value;
	}

	ExpressionValue(double value)
	{
		type = ExpressionValueType::Float;
		floatValue = value;
	}

	ExpressionValue(const std::wstring& value)
	{
		type = ExpressionValueType::String;
		strValue = value;
	}

	bool isFloat() const
	{
		return type == ExpressionValueType::Float;
	}

	bool isInt() const
	{
		return type == ExpressionValueType::Integer;
	}

	bool isString() const
	{
		return type == ExpressionValueType::String;
	}

	bool isValid() const
	{
		return type != ExpressionValueType::Invalid;
	}

	union
	{
		int64_t intValue;
		double floatValue;
	};

	std::wstring strValue;

	ExpressionValue operator!() const;
	ExpressionValue operator~() const;
	bool operator<(const ExpressionValue& other) const;
	bool operator<=(const ExpressionValue& other) const;
	bool operator>(const ExpressionValue& other) const;
	bool operator>=(const ExpressionValue& other) const;
	bool operator==(const ExpressionValue& other) const;
	bool operator!=(const ExpressionValue& other) const;
	ExpressionValue operator+(const ExpressionValue& other) const;
	ExpressionValue operator-(const ExpressionValue& other) const;
	ExpressionValue operator*(const ExpressionValue& other) const;
	ExpressionValue operator/(const ExpressionValue& other) const;
	ExpressionValue operator%(const ExpressionValue& other) const;
	ExpressionValue operator<<(const ExpressionValue& other) const;
	ExpressionValue operator>>(const ExpressionValue& other) const;
	ExpressionValue operator&(const ExpressionValue& other) const;
	ExpressionValue operator|(const ExpressionValue& other) const;
	ExpressionValue operator&&(const ExpressionValue& other) const;
	ExpressionValue operator||(const ExpressionValue& other) const;
	ExpressionValue operator^(const ExpressionValue& other) const;
};

class ExpressionInternal
{
public:
	ExpressionInternal();
	~ExpressionInternal();
	ExpressionInternal(int64_t value);
	ExpressionInternal(double value);
	ExpressionInternal(const std::wstring& value, OperatorType type);
	ExpressionInternal(OperatorType op, ExpressionInternal* a = nullptr,
		ExpressionInternal* b = nullptr, ExpressionInternal* c = nullptr);
	ExpressionInternal(const std::wstring& name, const std::vector<ExpressionInternal*>& parameters);
	ExpressionValue evaluate();
	std::wstring toString();
	bool isIdentifier() { return type == OperatorType::Identifier; }
	std::wstring getStringValue() { return strValue; }
	void replaceMemoryPos(const std::wstring& identifierName);
	bool simplify(bool inUnknownOrFalseBlock);
	unsigned int getFileNum() { return fileNum; }
	unsigned int getSection() { return section; }
private:
	void allocate(size_t count);
	void deallocate();
	std::wstring formatFunctionCall();
	ExpressionValue executeExpressionFunctionCall(const ExpressionFunctionEntry& entry);
	ExpressionValue executeExpressionLabelFunctionCall(const ExpressionLabelFunctionEntry& entry);
	ExpressionValue executeFunctionCall();
	bool checkParameterCount(size_t min, size_t max);

	OperatorType type;
	ExpressionInternal** children;
	size_t childrenCount;

	union
	{
		int64_t intValue;
		double floatValue;
	};
	std::wstring strValue;

	unsigned int fileNum, section;
};

class Expression
{
public:
	Expression();
	ExpressionValue evaluate();
	bool isLoaded() const { return expression != nullptr; }
	void setExpression(ExpressionInternal* exp, bool inUnknownOrFalseBlock);
	void replaceMemoryPos(const std::wstring& identifierName);
	bool isConstExpression() { return constExpression; }

	template<typename T>
	bool evaluateInteger(T& dest)
	{
		if (expression == nullptr)
			return false;

		ExpressionValue value = expression->evaluate();
		if (value.isInt() == false)
			return false;

		dest = (T) value.intValue;
		return true;
	}

	bool evaluateString(std::wstring& dest, bool convert);
	bool evaluateIdentifier(std::wstring& dest);
	std::wstring toString();
private:
	std::shared_ptr<ExpressionInternal> expression;
	std::wstring originalText;
	bool constExpression;
};

Expression createConstExpression(int64_t value);

// file: Parser/Tokenizer.h

#include <list>
#include <string>
#include <vector>

class TextFile;

enum class TokenType
{
	Invalid,
	Identifier,
	Integer,
	String,
	Float,
	LParen,
	RParen,
	Plus,
	Minus,
	Mult,
	Div,
	Mod,
	Caret,
	Tilde,
	LeftShift,
	RightShift,
	Less,
	Greater,
	LessEqual,
	GreaterEqual,
	Equal,
	NotEqual,
	BitAnd,
	BitOr,
	LogAnd,
	LogOr,
	Exclamation,
	Question,
	Colon,
	LBrack,
	RBrack,
	Comma,
	Assign,
	Equ,
	EquValue,
	Hash,
	LBrace,
	RBrace,
	Dollar,
	NumberString,
	Degree,
	Separator
};

struct Token
{
	friend class Tokenizer;

	Token()
	{
	}

	void setOriginalText(const std::wstring& t)
	{
		setOriginalText(t, 0, t.length());
	}

	void setOriginalText(const std::wstring& t, const size_t pos, const size_t len)
	{
		originalText = t.substr(pos, len);
	}

	std::wstring getOriginalText() const
	{
		return originalText;
	}

	void setStringValue(const std::wstring& t)
	{
		setStringValue(t, 0, t.length());
	}

	void setStringValue(const std::wstring& t, const size_t pos, const size_t len)
	{
		stringValue = t.substr(pos, len);
	}

	void setStringAndOriginalValue(const std::wstring& t)
	{
		setStringAndOriginalValue(t, 0, t.length());
	}

	void setStringAndOriginalValue(const std::wstring& t, const size_t pos, const size_t len)
	{
		setStringValue(t, pos, len);
		originalText = stringValue;
	}

	std::wstring getStringValue() const
	{
		return stringValue;
	}

	bool stringValueStartsWith(wchar_t c) const
	{
		return stringValue[0] == c;
	}

	TokenType type;
	size_t line;
	size_t column;

	union
	{
		int64_t intValue;
		double floatValue;
	};

protected:
	std::wstring originalText;
	std::wstring stringValue;

	bool checked = false;
};

typedef std::list<Token> TokenList;

struct TokenizerPosition
{
	friend class Tokenizer;

	TokenizerPosition previous()
	{
		TokenizerPosition pos = *this;
		pos.it--;
		return pos;
	}
private:
	TokenList::iterator it;
};

class Tokenizer
{
public:
	Tokenizer();
	const Token& nextToken();
	const Token& peekToken(int ahead = 0);
	void eatToken() { eatTokens(1); }
	void eatTokens(int num);
	bool atEnd() { return position.it == tokens.end(); }
	TokenizerPosition getPosition() { return position; }
	void setPosition(TokenizerPosition pos) { position = pos; }
	void skipLookahead();
	std::vector<Token> getTokens(TokenizerPosition start, TokenizerPosition end) const;
	void registerReplacement(const std::wstring& identifier, std::vector<Token>& tokens);
	void registerReplacement(const std::wstring& identifier, const std::wstring& newValue);
	void registerReplacementString(const std::wstring& identifier, const std::wstring& newValue);
	void registerReplacementInteger(const std::wstring& identifier, int64_t newValue);
	void registerReplacementFloat(const std::wstring& identifier, double newValue);
	static size_t addEquValue(const std::vector<Token>& tokens);
	static void clearEquValues() { equValues.clear(); }
	void resetLookaheadCheckMarks();
protected:
	void clearTokens() { tokens.clear(); };
	void resetPosition() { position.it = tokens.begin(); }
	void addToken(Token token);
private:
	bool processElement(TokenList::iterator& it);

	TokenList tokens;
	TokenizerPosition position;

	struct Replacement
	{
		std::wstring identifier;
		std::vector<Token> value;
	};

	Token invalidToken;
	std::vector<Replacement> replacements;
	static std::vector<std::vector<Token>> equValues;
};

class FileTokenizer: public Tokenizer
{
public:
	bool init(TextFile* input);
protected:
	Token loadToken();
	bool isInputAtEnd();

	void skipWhitespace();
	void createToken(TokenType type, size_t length);
	void createToken(TokenType type, size_t length, int64_t value);
	void createToken(TokenType type, size_t length, double value);
	void createToken(TokenType type, size_t length, const std::wstring& value);
	void createToken(TokenType type, size_t length, const std::wstring& value, size_t valuePos, size_t valueLength);
	void createTokenCurrentString(TokenType type, size_t length);

	bool convertInteger(size_t start, size_t end, int64_t& result);
	bool convertFloat(size_t start, size_t end, double& result);
	bool parseOperator();

	TextFile* input;
	std::wstring currentLine;
	size_t lineNumber;
	size_t linePos;

	Token token;
	bool equActive;
};

class TokenStreamTokenizer: public Tokenizer
{
public:
	void init(const std::vector<Token>& tokens)
	{
		clearTokens();

		for (const Token &tok: tokens)
			addToken(tok);

		resetPosition();
	}
};


// file: Core/ExpressionFunctions.h

#include <map>
#include <memory>
#include <string>
#include <vector>


class Label;

struct ExpressionValue;

bool getExpFuncParameter(const std::vector<ExpressionValue>& parameters, size_t index, int64_t& dest,
	const std::wstring& funcName, bool optional);

bool getExpFuncParameter(const std::vector<ExpressionValue>& parameters, size_t index, const std::wstring*& dest,
	const std::wstring& funcName, bool optional);

using ExpressionFunction = ExpressionValue (*)(const std::wstring& funcName, const std::vector<ExpressionValue>&);
using ExpressionLabelFunction = ExpressionValue (*)(const std::wstring& funcName, const std::vector<std::shared_ptr<Label>> &);

enum class ExpFuncSafety
{
	// Result may depend entirely on the internal state
	Unsafe,
	// Result is unsafe in conditional blocks, safe otherwise
	ConditionalUnsafe,
	// Result is completely independent of the internal state
	Safe,
};

struct ExpressionFunctionEntry
{
	ExpressionFunction function;
	size_t minParams;
	size_t maxParams;
	ExpFuncSafety safety;
};

struct ExpressionLabelFunctionEntry
{
	ExpressionLabelFunction function;
	size_t minParams;
	size_t maxParams;
	ExpFuncSafety safety;
};

struct UserExpressionFunction
{
	std::wstring name;
	std::vector<std::wstring> parameters;
	std::vector<Token> content;
};

class UserFunctions
{
public:
	static UserFunctions &instance()
	{
		static UserFunctions func;
		return func;
	}

	bool addFunction(const UserExpressionFunction &func)
	{
		_entries.emplace(func.name, func);
		return true;
	}
	const UserExpressionFunction *findFunction(const std::wstring &name) const
	{
		auto it = _entries.find(name);
		return it != _entries.end() ? &it->second : nullptr;
	}

private:
	UserFunctions() = default;

	std::map<std::wstring, UserExpressionFunction> _entries;
};

using ExpressionFunctionMap =  std::map<std::wstring, const ExpressionFunctionEntry>;
using ExpressionLabelFunctionMap =  std::map<std::wstring, const ExpressionLabelFunctionEntry>;

extern const ExpressionFunctionMap expressionFunctions;
extern const ExpressionLabelFunctionMap expressionLabelFunctions;

// file: Core/SymbolData.h


#include <cstdint>
#include <set>
#include <string>
#include <vector>

class AssemblerFile;

struct SymDataSymbol
{
	std::wstring name;
	int64_t address;

	bool operator<(const SymDataSymbol& other) const
	{
		return address < other.address;
	}
};

struct SymDataAddressInfo
{
	int64_t address;
	size_t fileIndex;
	size_t lineNumber;

	bool operator<(const SymDataAddressInfo& other) const
	{
		return address < other.address;
	}
};

struct SymDataFunction
{
	int64_t address;
	size_t size;

	bool operator<(const SymDataFunction& other) const
	{
		return address < other.address;
	}
};

struct SymDataData
{
	int64_t address;
	size_t size;
	int type;

	bool operator<(const SymDataData& other) const
	{
		if (address != other.address)
			return address < other.address;

		if (size != other.size)
			return size < other.size;

		return type < other.type;
	}
};

struct SymDataModule
{
	AssemblerFile* file;
	std::vector<SymDataSymbol> symbols;
	std::vector<SymDataFunction> functions;
	std::set<SymDataData> data;
};

struct SymDataModuleInfo
{
	unsigned int crc32;
};

class SymbolData
{
public:
	enum DataType { Data8, Data16, Data32, Data64, DataAscii };

	SymbolData();
	void clear();
	void setNocashSymFileName(const fs::path& name, int version) { nocashSymFileName = name; nocashSymVersion = version; };
	void write();
	void setEnabled(bool b) { enabled = b; };

	void addLabel(int64_t address, const std::wstring& name);
	void addData(int64_t address, size_t size, DataType type);
	void startModule(AssemblerFile* file);
	void endModule(AssemblerFile* file);
	void startFunction(int64_t address);
	void endFunction(int64_t address);
private:
	void writeNocashSym();
	size_t addFileName(const std::wstring& fileName);

	fs::path nocashSymFileName;
	bool enabled;
	int nocashSymVersion;

	// entry 0 is for data without parent modules
	std::vector<SymDataModule> modules;
	std::vector<std::wstring> files;
	int currentModule;
	int currentFunction;
};

// file: Util/Util.h


#include <string>
#include <vector>

std::wstring convertUtf8ToWString(const char* source);
std::string convertWCharToUtf8(wchar_t character);
std::string convertWStringToUtf8(const std::wstring& source);

std::wstring intToHexString(unsigned int value, int digits, bool prefix = false);
std::wstring intToString(unsigned int value, int digits);
bool stringToInt(const std::wstring& line, size_t start, size_t end, int64_t& result);
int32_t getFloatBits(float value);
float bitsToFloat(int32_t value);
int64_t getDoubleBits(double value);

std::vector<std::wstring> getStringListFromArray(wchar_t** source, int count);
std::vector<std::wstring> splitString(const std::wstring& str, const wchar_t delim, bool skipEmpty);

std::wstring toWLowercase(const std::string& str);
size_t replaceAll(std::wstring& str, const wchar_t* oldValue,const std::wstring& newValue);
bool startsWith(const std::wstring& str, const wchar_t* value, size_t stringPos = 0);

// file: Util/FileClasses.h


#include <list>
#include <vector>

class TextFile
{
public:
	enum Encoding { ASCII, UTF8, UTF16LE, UTF16BE, SJIS, GUESS };
	enum Mode { Read, Write };

	TextFile();
	~TextFile();
	void openMemory(const std::wstring& content);
	bool open(const fs::path& fileName, Mode mode, Encoding defaultEncoding = GUESS);
	bool open(Mode mode, Encoding defaultEncoding = GUESS);
	bool isOpen() { return fromMemory || stream.is_open(); };
	bool atEnd() { return isOpen() && mode == Read && tell() >= size_; };
	long size() { return size_; };
	void close();

	bool hasGuessedEncoding() { return guessedEncoding; };
	bool isFromMemory() { return fromMemory; }
	int getNumLines() { return lineCount; }

	void setFileName(const fs::path& name) { fileName = name; };
	const fs::path& getFileName() { return fileName; };

	wchar_t readCharacter();
	std::wstring readLine();
	std::vector<std::wstring> readAll();
	void writeCharacter(wchar_t character);
	void write(const wchar_t* line);
	void write(const std::wstring& line);
	void write(const char* value);
	void write(const std::string& value);
	void writeLine(const wchar_t* line);
	void writeLine(const std::wstring& line);
	void writeLine(const char* line);
	void writeLine(const std::string& line);
	void writeLines(std::vector<std::wstring>& list);

	template <typename... Args>
	void writeFormat(const wchar_t* text, const Args&... args)
	{
		std::wstring message = tfm::format(text,args...);
		write(message);
	}

	bool hasError() { return errorText.size() != 0 && !errorRetrieved; };
	const std::wstring& getErrorText() { errorRetrieved = true; return errorText; };
private:
	long tell();
	void seek(long pos);

	fs::fstream stream;
	fs::path fileName;
	Encoding encoding;
	Mode mode;
	bool recursion;
	bool guessedEncoding;
	long size_;
	std::wstring errorText;
	bool errorRetrieved;
	bool fromMemory;
	std::wstring content;
	size_t contentPos;
	int lineCount;

	std::string buf;
	size_t bufPos;

	inline unsigned char bufGetChar()
	{
		if (buf.size() <= bufPos)
		{
			bufFillRead();
			if (buf.size() == 0)
				return 0;
		}
		return buf[bufPos++];
	}
	inline unsigned short bufGet16LE()
	{
		unsigned char c1 = bufGetChar();
		unsigned char c2 = bufGetChar();
		return c1 | (c2 << 8);
	}
	inline unsigned short bufGet16BE()
	{
		unsigned char c1 = bufGetChar();
		unsigned char c2 = bufGetChar();
		return c2 | (c1 << 8);
	}

	void bufPut(const void *p, const size_t len);
	void bufPut(const char c);

	void bufFillRead();
	void bufDrainWrite();
};

wchar_t sjisToUnicode(unsigned short);
TextFile::Encoding getEncodingFromString(const std::wstring& str);

// file: Util/ByteArray.h


#include <string>

#include <sys/types.h>

#if defined(_MSC_VER) && !defined(ssize_t)
typedef intptr_t ssize_t;
#endif

typedef unsigned char byte;

enum class Endianness { Big, Little };

class ByteArray
{
public:
	ByteArray();
	ByteArray(const ByteArray& other);
	ByteArray(byte* data, size_t size);
	ByteArray(ByteArray&& other);
	~ByteArray();
	ByteArray& operator=(ByteArray& other);
	ByteArray& operator=(ByteArray&& other);

	size_t append(const ByteArray& other);
	size_t append(void* data, size_t size);
	size_t appendByte(byte b) { return append(&b,1); };
	void replaceByte(size_t pos, byte b) { data_[pos] = b; };
	void replaceBytes(size_t pos, byte* data, size_t size);
	void reserveBytes(size_t count, byte value = 0);
	void alignSize(size_t alignment);

	int getWord(size_t pos, Endianness endianness = Endianness::Little) const
	{
		if (pos+1 >= this->size()) return -1;
		unsigned char* d = (unsigned char*) this->data();

		if (endianness == Endianness::Little)
		{
			return d[pos+0] | (d[pos+1] << 8);
		} else {
			return d[pos+1] | (d[pos+0] << 8);
		}
	}

	int getDoubleWord(size_t pos, Endianness endianness = Endianness::Little) const
	{
		if (pos+3 >= this->size()) return -1;
		unsigned char* d = (unsigned char*) this->data();

		if (endianness == Endianness::Little)
		{
			return d[pos+0] | (d[pos+1] << 8) | (d[pos+2] << 16) | (d[pos+3] << 24);
		} else {
			return d[pos+3] | (d[pos+2] << 8) | (d[pos+1] << 16) | (d[pos+0] << 24);
		}
	}

	void replaceWord(size_t pos, unsigned int w, Endianness endianness = Endianness::Little)
	{
		if (pos+1 >= this->size()) return;
		unsigned char* d = (unsigned char*) this->data();

		if (endianness == Endianness::Little)
		{
			d[pos+0] = w & 0xFF;
			d[pos+1] = (w >> 8) & 0xFF;
		} else {
			d[pos+0] = (w >> 8) & 0xFF;
			d[pos+1] = w & 0xFF;
		}
	}

	void replaceDoubleWord(size_t pos, unsigned int w, Endianness endianness = Endianness::Little)
	{
		if (pos+3 >= this->size()) return;
		unsigned char* d = (unsigned char*) this->data();

		if (endianness == Endianness::Little)
		{
			d[pos+0] = w & 0xFF;
			d[pos+1] = (w >> 8) & 0xFF;
			d[pos+2] = (w >> 16) & 0xFF;
			d[pos+3] = (w >> 24) & 0xFF;
		} else {
			d[pos+0] = (w >> 24) & 0xFF;
			d[pos+1] = (w >> 16) & 0xFF;
			d[pos+2] = (w >> 8) & 0xFF;
			d[pos+3] = w & 0xFF;
		}
	}

	byte& operator [](size_t index)
	{
		return data_[index];
	};

	const byte& operator [](size_t index) const
	{
		return data_[index];
	};

	size_t size() const { return size_; };
	byte* data(size_t pos = 0) const { return &data_[pos]; };
	void clear() { size_ = 0; };
	void resize(size_t newSize);
	ByteArray mid(size_t start, ssize_t length = 0);
	ByteArray left(size_t length) { return mid(0,length); };
	ByteArray right(size_t length) { return mid(size_-length,length); };

	static ByteArray fromFile(const fs::path& fileName, unsigned long start = 0, size_t size = 0);
	bool toFile(const fs::path& fileName);
private:
	void grow(size_t neededSize);
	byte* data_;
	size_t size_;
	size_t allocatedSize_;
};

// file: Core/FileManager.h


#include <memory>
#include <vector>

class SymbolData;

struct SymDataModuleInfo;

class AssemblerFile
{
public:
	virtual ~AssemblerFile() { };

	virtual bool open(bool onlyCheck) = 0;
	virtual void close() = 0;
	virtual bool isOpen() = 0;
	virtual bool write(void* data, size_t length) = 0;
	virtual int64_t getVirtualAddress() = 0;
	virtual int64_t getPhysicalAddress() = 0;
	virtual int64_t getHeaderSize() = 0;
	virtual bool seekVirtual(int64_t virtualAddress) = 0;
	virtual bool seekPhysical(int64_t physicalAddress) = 0;
	virtual bool getModuleInfo(SymDataModuleInfo& info) { return false; };
	virtual bool hasFixedVirtualAddress() { return false; };
	virtual void beginSymData(SymbolData& symData) { };
	virtual void endSymData(SymbolData& symData) { };
	virtual const fs::path& getFileName() = 0;
};

class GenericAssemblerFile: public AssemblerFile
{
public:
	GenericAssemblerFile(const fs::path& fileName, int64_t headerSize, bool overwrite);
	GenericAssemblerFile(const fs::path& fileName, const fs::path& originalFileName, int64_t headerSize);

	virtual bool open(bool onlyCheck);
	virtual void close() { if (stream.is_open()) stream.close(); };
	virtual bool isOpen() { return stream.is_open(); };
	virtual bool write(void* data, size_t length);
	virtual int64_t getVirtualAddress() { return virtualAddress; };
	virtual int64_t getPhysicalAddress() { return virtualAddress-headerSize; };
	virtual int64_t getHeaderSize() { return headerSize; };
	virtual bool seekVirtual(int64_t virtualAddress);
	virtual bool seekPhysical(int64_t physicalAddress);
	virtual bool hasFixedVirtualAddress() { return true; };

	virtual const fs::path& getFileName() { return fileName; };
	const fs::path& getOriginalFileName() { return originalName; };
	int64_t getOriginalHeaderSize() { return originalHeaderSize; };
	void setHeaderSize(int64_t size) { headerSize = size; };

private:
	enum Mode { Open, Create, Copy };

	Mode mode;
	int64_t originalHeaderSize;
	int64_t headerSize;
	int64_t virtualAddress;
	fs::ofstream stream;
	fs::path fileName;
	fs::path originalName;
};


class FileManager
{
public:
	FileManager();
	~FileManager();
	void reset();
	bool openFile(std::shared_ptr<AssemblerFile> file, bool onlyCheck);
	void addFile(std::shared_ptr<AssemblerFile> file);
	bool hasOpenFile() { return activeFile != nullptr; };
	void closeFile();
	bool write(void* data, size_t length);
	bool writeU8(uint8_t data);
	bool writeU16(uint16_t data);
	bool writeU32(uint32_t data);
	bool writeU64(uint64_t data);
	int64_t getVirtualAddress();
	int64_t getPhysicalAddress();
	int64_t getHeaderSize();
	bool seekVirtual(int64_t virtualAddress);
	bool seekPhysical(int64_t physicalAddress);
	bool advanceMemory(size_t bytes);
	std::shared_ptr<AssemblerFile> getOpenFile() { return activeFile; };
	int64_t getOpenFileID();
	void setEndianness(Endianness endianness) { this->endianness = endianness; };
	Endianness getEndianness() { return endianness; }
private:
	bool checkActiveFile();
	std::vector<std::shared_ptr<AssemblerFile>> files;
	std::shared_ptr<AssemblerFile> activeFile;
	Endianness endianness;
	Endianness ownEndianness;
};

// file: Core/ELF/ElfTypes.h

///////////////////////
// ELF Header Constants

// File type
enum ElfType
{
	ET_NONE        =0,
	ET_REL         =1,
	ET_EXEC        =2,
	ET_DYN         =3,
	ET_CORE        =4,
	ET_LOPROC =0xFF00,
	ET_HIPROC =0xFFFF,
};

// Machine/Architecture
enum ElfMachine
{
	EM_NONE  =0,
	EM_MIPS  =8,
	EM_ARM   =40,
};

// File version
#define EV_NONE    0
#define EV_CURRENT 1

// Identification index
#define EI_MAG0    0
#define EI_MAG1    1
#define EI_MAG2    2
#define EI_MAG3    3
#define EI_CLASS   4
#define EI_DATA    5
#define EI_VERSION 6
#define EI_PAD     7
#define EI_NIDENT 16

// Magic number
#define ELFMAG0 0x7F
#define ELFMAG1  'E'
#define ELFMAG2  'L'
#define ELFMAG3  'F'

// File class
#define ELFCLASSNONE 0
#define ELFCLASS32   1
#define ELFCLASS64   2

// Encoding
#define ELFDATANONE 0
#define ELFDATA2LSB 1
#define ELFDATA2MSB 2


/////////////////////
// Sections constants

// Section indexes
#define SHN_UNDEF          0
#define SHN_LORESERVE 0xFF00
#define SHN_LOPROC    0xFF00
#define SHN_HIPROC    0xFF1F
#define SHN_ABS       0xFFF1
#define SHN_COMMON    0xFFF2
#define SHN_HIRESERVE 0xFFFF

// Section types
#define SHT_NULL            0
#define SHT_PROGBITS        1
#define SHT_SYMTAB          2
#define SHT_STRTAB          3
#define SHT_RELA            4
#define SHT_HASH            5
#define SHT_DYNAMIC         6
#define SHT_NOTE            7
#define SHT_NOBITS          8
#define SHT_REL             9
#define SHT_SHLIB          10
#define SHT_DYNSYM         11
#define SHT_INIT_ARRAY     14
#define SHT_LOPROC 0x70000000
#define SHT_HIPROC 0x7FFFFFFF
#define SHT_LOUSER 0x80000000
#define SHT_HIUSER 0xFFFFFFFF

// Custom section types
#define SHT_PSPREL 0x700000a0


// Section flags
enum ElfSectionFlags
{
	SHF_WRITE     =0x1,
	SHF_ALLOC     =0x2,
	SHF_EXECINSTR =0x4,
	SHF_MASKPROC  =0xF0000000,
};

// Symbol binding
#define STB_LOCAL   0
#define STB_GLOBAL  1
#define STB_WEAK    2
#define STB_LOPROC 13
#define STB_HIPROC 15

// Symbol types
#define STT_NOTYPE   0
#define STT_OBJECT   1
#define STT_FUNC     2
#define STT_SECTION  3
#define STT_FILE     4
#define STT_LOPROC  13
#define STT_HIPROC  15

// Undefined name
#define STN_UNDEF 0

// Relocation types
#define R_386_NONE      0
#define R_386_32        1
#define R_386_PC32      2
#define R_386_GOT32     3
#define R_386_PLT32     4
#define R_386_COPY      5
#define R_386_GLOB_DAT  6
#define R_386_JMP_SLOT  7
#define R_386_RELATIVE  8
#define R_386_GOTOFF    9
#define R_386_GOTPC    10

// Segment types
#define PT_NULL             0
#define PT_LOAD             1
#define PT_DYNAMIC          2
#define PT_INTERP           3
#define PT_NOTE             4
#define PT_SHLIB            5
#define PT_PHDR             6
#define PT_LOPROC  0x70000000
#define PT_HIPROC  0x7FFFFFFF

// Segment flags
#define PF_X 1
#define PF_W 2
#define PF_R 4

// Dynamic Array Tags
#define DT_NULL              0
#define DT_NEEDED            1
#define DT_PLTRELSZ          2
#define DT_PLTGOT            3
#define DT_HASH              4
#define DT_STRTAB            5
#define DT_SYMTAB            6
#define DT_RELA              7
#define DT_RELASZ            8
#define DT_RELAENT           9
#define DT_STRSZ            10
#define DT_SYMENT           11
#define DT_INIT             12
#define DT_FINI             13
#define DT_SONAME           14
#define DT_RPATH            15
#define DT_SYMBOLIC         16
#define DT_REL              17
#define DT_RELSZ            18
#define DT_RELENT           19
#define DT_PLTREL           20
#define DT_DEBUG            21
#define DT_TEXTREL          22
#define DT_JMPREL           23
#define DT_LOPROC   0x70000000
#define DT_HIPROC   0x7FFFFFFF

typedef unsigned int Elf32_Addr;
typedef unsigned short Elf32_Half;
typedef unsigned int Elf32_Off;
typedef signed int Elf32_Sword;
typedef unsigned int Elf32_Word;


// ELF file header
struct Elf32_Ehdr
{
	unsigned char e_ident[EI_NIDENT];
	Elf32_Half    e_type;
	Elf32_Half    e_machine;
	Elf32_Word    e_version;
	Elf32_Addr    e_entry;
	Elf32_Off     e_phoff;
	Elf32_Off     e_shoff;
	Elf32_Word    e_flags;
	Elf32_Half    e_ehsize;
	Elf32_Half    e_phentsize;
	Elf32_Half    e_phnum;
	Elf32_Half    e_shentsize;
	Elf32_Half    e_shnum;
	Elf32_Half    e_shstrndx;
};

// Section header
struct Elf32_Shdr
{
	Elf32_Word sh_name;
	Elf32_Word sh_type;
	Elf32_Word sh_flags;
	Elf32_Addr sh_addr;
	Elf32_Off  sh_offset;
	Elf32_Word sh_size;
	Elf32_Word sh_link;
	Elf32_Word sh_info;
	Elf32_Word sh_addralign;
	Elf32_Word sh_entsize;
};

// Segment header
struct Elf32_Phdr
{
	Elf32_Word p_type;
	Elf32_Off  p_offset;
	Elf32_Addr p_vaddr;
	Elf32_Addr p_paddr;
	Elf32_Word p_filesz;
	Elf32_Word p_memsz;
	Elf32_Word p_flags;
	Elf32_Word p_align;
};

// Symbol table entry
struct Elf32_Sym
{
	Elf32_Word    st_name;
	Elf32_Addr    st_value;
	Elf32_Word    st_size;
	unsigned char st_info;
	unsigned char st_other;
	Elf32_Half    st_shndx;
};

#define ELF32_ST_BIND(i)   ((i)>>4)
#define ELF32_ST_TYPE(i)   ((i)&0xf)
#define ELF32_ST_INFO(b,t) (((b)<<4)+((t)&0xf))

// Relocation entries
struct Elf32_Rel
{
	Elf32_Addr r_offset;
	Elf32_Word r_info;

	unsigned char getType()
	{
		return r_info & 0xFF;
	}

	Elf32_Word getSymbolNum()
	{
		return r_info >> 8;
	}
};

struct Elf32_Rela
{
	Elf32_Addr  r_offset;
	Elf32_Word  r_info;
	Elf32_Sword r_addend;
};

#define ELF32_R_SYM(i) ((i)>>8)
#define ELF32_R_TYPE(i) ((unsigned char)(i))
#define ELF32_R_INFO(s,t) (((s)<<8 )+(unsigned char)(t))

// file: Core/ELF/ElfFile.h


#include <vector>

enum ElfPart { ELFPART_SEGMENTTABLE, ELFPART_SECTIONTABLE, ELFPART_SEGMENTS, ELFPART_SEGMENTLESSSECTIONS };

class ElfSegment;
class ElfSection;

class ElfFile
{
public:

	bool load(const fs::path&fileName, bool sort);
	bool load(ByteArray& data, bool sort);
	void save(const fs::path& fileName);

	Elf32_Half getType() { return fileHeader.e_type; };
	Elf32_Half getMachine() { return fileHeader.e_machine; };
	Endianness getEndianness()
	{
		return fileHeader.e_ident[EI_DATA] == ELFDATA2MSB ? Endianness::Big : Endianness::Little;
	}
	size_t getSegmentCount() { return segments.size(); };
	ElfSegment* getSegment(size_t index) { return segments[index]; };

	int findSegmentlessSection(const std::string& name);
	ElfSection* getSegmentlessSection(size_t index) { return segmentlessSections[index]; };
	size_t getSegmentlessSectionCount() { return segmentlessSections.size(); };
	ByteArray& getFileData() { return fileData; }

	int getSymbolCount();
	bool getSymbol(Elf32_Sym& symbol, size_t index);
	const char* getStrTableString(size_t pos);
private:
	void loadElfHeader();
	void writeHeader(ByteArray& data, size_t pos, Endianness endianness);
	void loadProgramHeader(Elf32_Phdr& header, ByteArray& data, size_t pos);
	void loadSectionHeader(Elf32_Shdr& header, ByteArray& data, size_t pos);
	void loadSectionNames();
	void determinePartOrder();

	Elf32_Ehdr fileHeader;
	std::vector<ElfSegment*> segments;
	std::vector<ElfSection*> sections;
	std::vector<ElfSection*> segmentlessSections;
	ByteArray fileData;
	ElfPart partsOrder[4];

	ElfSection* symTab;
	ElfSection* strTab;
};


class ElfSection
{
public:
	ElfSection(Elf32_Shdr header);
	void setName(std::string& name) { this->name = name; };
	const std::string& getName() { return name; };
	void setData(ByteArray& data) { this->data = data; };
	void setOwner(ElfSegment* segment);
	bool hasOwner() { return owner != nullptr; };
	void writeHeader(ByteArray& data, size_t pos, Endianness endianness);
	void writeData(ByteArray& output);
	void setOffsetBase(int base);
	ByteArray& getData() { return data; };

	Elf32_Word getType() { return header.sh_type; };
	Elf32_Off getOffset() { return header.sh_offset; };
	Elf32_Word getSize() { return header.sh_size; };
	Elf32_Word getNameOffset() { return header.sh_name; };
	Elf32_Word getAlignment() { return header.sh_addralign; };
	Elf32_Addr getAddress() { return header.sh_addr; };
	Elf32_Half getInfo() { return header.sh_info; };
	Elf32_Word getFlags() { return header.sh_flags; };
private:
	Elf32_Shdr header;
	std::string name;
	ByteArray data;
	ElfSegment* owner;
};

class ElfSegment
{
public:
	ElfSegment(Elf32_Phdr header, ByteArray& segmentData);
	bool isSectionPartOf(ElfSection* section);
	void addSection(ElfSection* section);
	Elf32_Off getOffset() { return header.p_offset; };
	Elf32_Word getPhysSize() { return header.p_filesz; };
	Elf32_Word getType() { return header.p_type; };
	Elf32_Addr getVirtualAddress() { return header.p_vaddr; };
	size_t getSectionCount() { return sections.size(); };
	void writeHeader(ByteArray& data, size_t pos, Endianness endianness);
	void writeData(ByteArray& output);
	void splitSections();

	int findSection(const std::string& name);
	ElfSection* getSection(size_t index) { return sections[index]; };
	void writeToData(size_t offset, void* data, size_t size);
	void sortSections();
private:
	Elf32_Phdr header;
	ByteArray data;
	std::vector<ElfSection*> sections;
	ElfSection* paddrSection;
};

struct RelocationData
{
	int64_t opcodeOffset;
	int64_t relocationBase;
	uint32_t opcode;

	int64_t symbolAddress;
	int targetSymbolType;
	int targetSymbolInfo;
};

// file: Core/ELF/ElfRelocator.h


#include <memory>

struct ElfRelocatorCtor
{
	std::wstring symbolName;
	size_t size;
};

struct RelocationAction
{
	RelocationAction(int64_t offset, uint32_t newValue) : offset(offset), newValue(newValue) {}
	int64_t offset;
	uint32_t newValue;
};

class CAssemblerCommand;
class Parser;

class IElfRelocator
{
public:
	virtual ~IElfRelocator() {};
	virtual int expectedMachine() const = 0;
	virtual bool isDummyRelocationType(int type) const { return false; }
	virtual bool relocateOpcode(int type, const RelocationData& data, std::vector<RelocationAction>& actions, std::vector<std::wstring>& errors) = 0;
	virtual bool finish(std::vector<RelocationAction>& actions, std::vector<std::wstring>& errors) { return true; }
	virtual void setSymbolAddress(RelocationData& data, int64_t symbolAddress, int symbolType) = 0;

	virtual std::unique_ptr<CAssemblerCommand> generateCtorStub(std::vector<ElfRelocatorCtor>& ctors);
};


class Label;
class SymbolData;

struct ElfRelocatorSection
{
	ElfSection* section;
	size_t index;
	ElfSection* relSection;
	std::shared_ptr<Label> label;
};

struct ElfRelocatorSymbol
{
	std::shared_ptr<Label> label;
	std::wstring name;
	int64_t relativeAddress;
	int64_t relocatedAddress;
	size_t section;
	size_t size;
	int type;
};

struct ElfRelocatorFile
{
	ElfFile* elf;
	std::vector<ElfRelocatorSection> sections;
	std::vector<ElfRelocatorSymbol> symbols;
	std::wstring name;
};

class ElfRelocator
{
public:
	bool init(const fs::path& inputName);
	bool exportSymbols();
	void writeSymbols(SymbolData& symData) const;
	std::unique_ptr<CAssemblerCommand> generateCtor(const std::wstring& ctorName);
	bool relocate(int64_t& memoryAddress);
	bool hasDataChanged() { return dataChanged; };
	const ByteArray& getData() const { return outputData; };
private:
	bool relocateFile(ElfRelocatorFile& file, int64_t& relocationAddress);
	void loadRelocation(Elf32_Rel& rel, ByteArray& data, int offset, Endianness endianness);

	ByteArray outputData;
	std::unique_ptr<IElfRelocator> relocator;
	std::vector<ElfRelocatorFile> files;
	std::vector<ElfRelocatorCtor> ctors;
	bool dataChanged;
};

// file: Archs/Architecture.h


#include <map>
#include <memory>

class IElfRelocator;
class Tokenizer;
class Parser;

struct ExpressionFunctionEntry;

using ExpressionFunctionMap =  std::map<std::wstring, const ExpressionFunctionEntry>;

class CArchitecture
{
public:
	virtual std::unique_ptr<CAssemblerCommand> parseDirective(Parser& parser) { return nullptr; }
	virtual std::unique_ptr<CAssemblerCommand> parseOpcode(Parser& parser) { return nullptr; }
	virtual const ExpressionFunctionMap& getExpressionFunctions();
	virtual void NextSection() = 0;
	virtual void Pass2() = 0;
	virtual void Revalidate() = 0;
	virtual std::unique_ptr<IElfRelocator> getElfRelocator() = 0;
	virtual Endianness getEndianness() = 0;
};

class ArchitectureCommand: public CAssemblerCommand
{
public:
	ArchitectureCommand(const std::wstring& tempText, const std::wstring& symText);
	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
	void writeSymData(SymbolData& symData) const override;
private:
	int64_t position;
	Endianness endianness;
	std::wstring tempText;
	std::wstring symText;
};

class CInvalidArchitecture: public CArchitecture
{
public:
	void NextSection() override;
	void Pass2() override;
	void Revalidate() override;
	std::unique_ptr<IElfRelocator> getElfRelocator() override;
	Endianness getEndianness() override { return Endianness::Little; }
};

extern CInvalidArchitecture InvalidArchitecture;

// file: Archs/MIPS/Mips.h


class Expression;

enum MipsArchType { MARCH_PSX = 0, MARCH_N64, MARCH_PS2, MARCH_PSP, MARCH_RSP, MARCH_INVALID };

class CMipsArchitecture: public CArchitecture
{
public:
	CMipsArchitecture();
	virtual std::unique_ptr<CAssemblerCommand> parseDirective(Parser& parser);
	virtual std::unique_ptr<CAssemblerCommand> parseOpcode(Parser& parser);
	virtual const ExpressionFunctionMap& getExpressionFunctions();
	virtual void NextSection();
	virtual void Pass2() { return; };
	virtual void Revalidate();
	virtual std::unique_ptr<IElfRelocator> getElfRelocator();
	virtual Endianness getEndianness()
	{
		return Version == MARCH_N64 || Version == MARCH_RSP ? Endianness::Big : Endianness::Little;
	};
	void SetLoadDelay(bool Delay, int Register);
	bool GetLoadDelay() { return LoadDelay; };
	int GetLoadDelayRegister() { return LoadDelayRegister; };
	bool GetIgnoreDelay() { return IgnoreLoadDelay; };
	void SetIgnoreDelay(bool b) { IgnoreLoadDelay = b; };
	void SetFixLoadDelay(bool b) { FixLoadDelay = b; };
	bool GetFixLoadDelay() { return FixLoadDelay; };
	void SetVersion(MipsArchType v) { Version = v; };
	MipsArchType GetVersion() { return Version; };
	bool GetDelaySlot() { return DelaySlot; };
	void SetDelaySlot(bool b) {DelaySlot = b; };
	bool hasLoadDelay() { return Version == MARCH_PSX; };
private:
	bool FixLoadDelay;
	bool IgnoreLoadDelay;
	bool LoadDelay;
	int LoadDelayRegister;
	bool DelaySlot;
	MipsArchType Version;
};

typedef struct {
	const char* name;
	short num;
	short len;
} tMipsRegister;

typedef struct {
	char name[5];
	short num;
} MipsRegisterInfo;

enum MipsVfpuType { MIPSVFPU_VECTOR, MIPSVFPU_MATRIX };

struct MipsVFPURegister
{
	MipsVfpuType type;
	int num;
	char name[8];
};

extern const tMipsRegister MipsRegister[];
extern CMipsArchitecture Mips;

bool MipsGetRegister(const char* source, int& RetLen, MipsRegisterInfo& Result);
int MipsGetRegister(const char* source, int& RetLen);
bool MipsGetFloatRegister(const char* source, int& RetLen, MipsRegisterInfo& Result);
bool MipsGetPs2VectorRegister(const char* source, int& RetLen, MipsRegisterInfo& Result);
int MipsGetFloatRegister(const char* source, int& RetLen);
bool MipsCheckImmediate(const char* Source, Expression& Dest, int& RetLen);


// file: Archs/MIPS/MipsOpcodes.h

#include <cstdint>

#define MA_MIPS1		(1 << 0)
#define MA_MIPS2		(1 << 1)
#define MA_MIPS3		(1 << 2)
#define MA_MIPS4		(1 << 3)
#define MA_PSX			(1 << 4)
#define MA_PS2			(1 << 6)
#define MA_PSP			(1 << 7)
#define MA_RSP			(1 << 8)

#define MA_EXPSX		(1 << 12)
#define MA_EXN64		(1 << 13)
#define MA_EXPS2		(1 << 14)
#define MA_EXPSP		(1 << 15)
#define MA_EXRSP		(1 << 16)

#define MO_IPCA							(1 << 0)	// pc >> 2
#define MO_IPCR							(1 << 1)	// PC, -> difference >> 2
#define MO_RSD							(1 << 2)	// rs = rd
#define MO_RST							(1 << 3)	// rs = rt
#define MO_RDT							(1 << 4)	// rd = rt
#define MO_DELAY						(1 << 5)	// delay slot follows
#define MO_NODELAYSLOT					(1 << 6)	// can't be in a delay slot
#define MO_DELAYRT						(1 << 7)	// rt won't be available for one instruction
#define MO_IGNORERTD					(1 << 8)	// don't care for rt delay
#define MO_FRSD							(1 << 9)	// float rs + rd
#define MO_IMMALIGNED					(1 << 10)	// immediate 4 byte aligned
#define MO_NEGIMM						(1 << 11)	// negated immediate (for subi)
#define MO_64BIT						(1 << 12)	// only available on 64 bit cpus
#define MO_FPU							(1 << 13)	// only available with an fpu
#define MO_DFPU							(1 << 14)	// double-precision fpu opcodes

#define MO_VFPU							(1 << 16)	// vfpu type opcode
#define MO_VFPU_MIXED					(1 << 17)	// mixed mode vfpu register
#define MO_VFPU_6BIT					(1 << 18)	// vfpu register can have 6 bits max
#define MO_VFPU_SINGLE					(1 << 19)	// single vfpu reg
#define MO_VFPU_QUAD					(1 << 20)	// quad vfpu reg
#define MO_VFPU_TRANSPOSE_VS			(1 << 21)	// matrix vs has to be transposed
#define MO_VFPU_PAIR					(1 << 22)	// pair vfpu reg
#define MO_VFPU_TRIPLE					(1 << 23)	// triple vfpu reg

#define MO_RSP_VRSD						(1 << 24)	// rsp vector rs + rd
#define MO_RSP_HWOFFSET					(1 << 25) // RSP halfword load/store offset
#define MO_RSP_WOFFSET					(1 << 26) // RSP word load/store offset
#define MO_RSP_DWOFFSET					(1 << 27)	// RSP doubleword load/store offset
#define MO_RSP_QWOFFSET					(1 << 28)	// RSP quadword load/store offset


#define BITFIELD(START,LENGTH,VALUE)	(((VALUE) & ((1 << (LENGTH)) - 1)) << (START))
#define MIPS_FUNC(VALUE)				BITFIELD(0,6,(VALUE))
#define MIPS_SA(VALUE)					BITFIELD(6,5,(VALUE))
#define MIPS_SECFUNC(VALUE)				MIPS_SA((VALUE))
#define MIPS_OP(VALUE)					BITFIELD(26,6,(VALUE))

#define MIPS_RS(VALUE)					BITFIELD(21,5,(VALUE))
#define MIPS_RT(VALUE)					BITFIELD(16,5,(VALUE))
#define MIPS_RD(VALUE)					BITFIELD(11,5,(VALUE))
#define MIPS_FS(VALUE)					MIPS_RD((VALUE))
#define MIPS_FT(VALUE)					MIPS_RT((VALUE))
#define MIPS_FD(VALUE)					MIPS_SA((VALUE))

#define MIPS_SPECIAL(VALUE)				(MIPS_OP(0) | MIPS_FUNC(VALUE))
#define MIPS_REGIMM(VALUE)				(MIPS_OP(1) | MIPS_RT(VALUE))
#define MIPS_COP0(VALUE)				(MIPS_OP(16) | MIPS_RS(VALUE))
#define MIPS_COP0FUNCT(VALUE)			(MIPS_COP0(16) | MIPS_FUNC(VALUE))
#define MIPS_COP1(VALUE)				(MIPS_OP(17) | MIPS_RS(VALUE))
#define MIPS_COP1BC(VALUE)				(MIPS_COP1(8) | MIPS_RT(VALUE))
#define MIPS_COP1S(VALUE)				(MIPS_COP1(16) | MIPS_FUNC(VALUE))
#define MIPS_COP1D(VALUE)				(MIPS_COP1(17) | MIPS_FUNC(VALUE))
#define MIPS_COP1W(VALUE)				(MIPS_COP1(20) | MIPS_FUNC(VALUE))
#define MIPS_COP1L(VALUE)				(MIPS_COP1(21) | MIPS_FUNC(VALUE))

#define MIPS_VFPUSIZE(VALUE)			( (((VALUE) & 1) << 7) | (((VALUE) & 2) << 14) )
#define MIPS_VFPUFUNC(VALUE)			BITFIELD(23, 3, (VALUE))
#define MIPS_COP2(VALUE)				(MIPS_OP(18) | MIPS_RS(VALUE))
#define MIPS_COP2BC(VALUE)				(MIPS_COP2(8) | MIPS_RT(VALUE))
#define MIPS_RSP_COP2(VALUE)			(MIPS_OP(18) | (1 << 25) | MIPS_FUNC(VALUE))
#define MIPS_RSP_LWC2(VALUE)			(MIPS_OP(50) | MIPS_RD(VALUE))
#define MIPS_RSP_SWC2(VALUE)			(MIPS_OP(58) | MIPS_RD(VALUE))
#define MIPS_RSP_VE(VALUE)				BITFIELD(21, 4, (VALUE))
#define MIPS_RSP_VDE(VALUE)				BITFIELD(11, 4, (VALUE))
#define MIPS_RSP_VEALT(VALUE)			BITFIELD(7, 4, (VALUE))
#define MIPS_VFPU0(VALUE)				(MIPS_OP(24) | MIPS_VFPUFUNC(VALUE))
#define MIPS_VFPU1(VALUE)				(MIPS_OP(25) | MIPS_VFPUFUNC(VALUE))
#define MIPS_VFPU3(VALUE)				(MIPS_OP(27) | MIPS_VFPUFUNC(VALUE))
#define MIPS_SPECIAL3(VALUE)			(MIPS_OP(31) | MIPS_FUNC(VALUE))
#define MIPS_ALLEGREX0(VALUE)			(MIPS_SPECIAL3(32) | MIPS_SECFUNC(VALUE))
#define MIPS_VFPU4(VALUE)				(MIPS_OP(52) | MIPS_RS(VALUE))
#define MIPS_VFPU4_11(VALUE)			(MIPS_VFPU4(0) | MIPS_RT(VALUE))
#define MIPS_VFPU4_12(VALUE)			(MIPS_VFPU4(1) | MIPS_RT(VALUE))
#define MIPS_VFPU4_13(VALUE)			(MIPS_VFPU4(2) | MIPS_RT(VALUE))
#define MIPS_VFPU5(VALUE)				(MIPS_OP(55) | MIPS_VFPUFUNC(VALUE))
#define MIPS_VFPU6(VALUE)				(MIPS_OP(60) | MIPS_VFPUFUNC(VALUE))
#define MIPS_VFPU6_1(VALUE)				(MIPS_VFPU6(7) | BITFIELD(20, 3, VALUE))
// This is a bit ugly, VFPU opcodes are encoded strangely.
#define MIPS_VFPU6_1VROT()				(MIPS_VFPU6(7) | BITFIELD(21, 2, 1))
#define MIPS_VFPU6_2(VALUE)				(MIPS_VFPU6_1(0) | MIPS_RT(VALUE))


struct MipsArchDefinition
{
	const char* name;
	int supportSets;
	int excludeMask;
	int flags;
};

extern const MipsArchDefinition mipsArchs[];

struct tMipsOpcode
{
	const char* name;
	const char* encoding;
	int32_t destencoding;
	int archs;
	int flags;
};

extern const tMipsOpcode MipsOpcodes[];

// file: Archs/MIPS/CMipsInstruction.h


enum class MipsRegisterType
{
	Normal,
	Float,
	FpuControl,
	Cop0,
	Ps2Cop2,
	PsxCop2Data,
	PsxCop2Control,
	VfpuVector,
	VfpuMatrix,
	RspCop0,
	RspVector,
	RspVectorControl,
	RspVectorElement,
	RspScalarElement,
	RspOffsetElement
};

enum class MipsImmediateType
{
	None,
	Immediate5,
	Immediate10,
	Immediate16,
	Immediate20,
	Immediate25,
	Immediate26,
	Immediate20_0,
	ImmediateHalfFloat,
	Immediate7,
	CacheOp,
	Ext,
	Ins,
	Cop2BranchType
};

struct MipsRegisterValue
{
	MipsRegisterType type;
	std::wstring name;
	int num;
};

struct MipsRegisterData {
	MipsRegisterValue grs;			// general source reg
	MipsRegisterValue grt;			// general target reg
	MipsRegisterValue grd;			// general dest reg

	MipsRegisterValue frs;			// float source reg
	MipsRegisterValue frt;			// float target reg
	MipsRegisterValue frd;			// float dest reg

	MipsRegisterValue ps2vrs;		// ps2 vector source reg
	MipsRegisterValue ps2vrt;		// ps2 vector target reg
	MipsRegisterValue ps2vrd;		// ps2 vector dest reg

	MipsRegisterValue rspvrs;		// rsp vector source reg
	MipsRegisterValue rspvrt;		// rsp vector target reg
	MipsRegisterValue rspvrd;		// rsp vector dest reg
	MipsRegisterValue rspve;		// rsp vector element reg
	MipsRegisterValue rspvde;		// rsp vector dest element reg
	MipsRegisterValue rspvealt;		// rsp vector element reg (alt. placement)

	MipsRegisterValue vrs;			// vfpu source reg
	MipsRegisterValue vrt;			// vfpu target reg
	MipsRegisterValue vrd;			// vfpu dest reg

	void reset()
	{
		grs.num = grt.num = grd.num = -1;
		frs.num = frt.num = frd.num = -1;
		vrs.num = vrt.num = vrd.num = -1;
		ps2vrs.num = ps2vrt.num = ps2vrd.num = -1;
		rspvrs.num = rspvrt.num = rspvrd.num = -1;
		rspve.num = rspvde.num = rspvealt.num = -1;
	}
};

struct MipsImmediateData
{
	struct
	{
		MipsImmediateType type;
		Expression expression;
		int value;
		int originalValue;
	} primary;

	struct
	{
		MipsImmediateType type;
		Expression expression;
		int value;
		int originalValue;
	} secondary;

	void reset()
	{
		primary.type = MipsImmediateType::None;
		if (primary.expression.isLoaded())
			primary.expression = Expression();

		secondary.type = MipsImmediateType::None;
		if (secondary.expression.isLoaded())
			secondary.expression = Expression();
	}
};

struct MipsOpcodeData
{
	tMipsOpcode opcode;
	int vfpuSize;
	int vectorCondition;

	void reset()
	{
		vfpuSize = vectorCondition = -1;
	}
};

class CMipsInstruction: public CAssemblerCommand
{
public:
	CMipsInstruction(MipsOpcodeData& opcode, MipsImmediateData& immediate, MipsRegisterData& registers);
	~CMipsInstruction();
	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
private:
	void encodeNormal() const;
	void encodeVfpu() const;
	int floatToHalfFloat(int i);

	bool IgnoreLoadDelay;
	int64_t RamPos;
	bool addNop;

	// opcode variables
	MipsOpcodeData opcodeData;
	MipsImmediateData immediateData;
	MipsRegisterData registerData;
};

// file: Util/EncodingTable.h


#include <map>
#include <vector>

class Trie
{
public:
	Trie();
	void insert(const wchar_t* text, size_t value);
	void insert(wchar_t character, size_t value);
	bool findLongestPrefix(const wchar_t* text, size_t& result);
private:
	struct LookupEntry
	{
		size_t node;
		wchar_t input;

		bool operator<(const LookupEntry& other) const
		{
			if (node != other.node)
				return node < other.node;
			return input < other.input;
		}
	};

	struct Node
	{
		size_t index;
		bool hasValue;
		size_t value;
	};

	std::vector<Node> nodes;
	std::map<LookupEntry,size_t> lookup;
};

class EncodingTable
{
public:
	EncodingTable();
	~EncodingTable();
	void clear();
	bool load(const fs::path& fileName, TextFile::Encoding encoding = TextFile::GUESS);
	bool isLoaded() { return entries.size() != 0; };
	void addEntry(unsigned char* hex, size_t hexLength, const std::wstring& value);
	void addEntry(unsigned char* hex, size_t hexLength, wchar_t value);
	void setTerminationEntry(unsigned char* hex, size_t hexLength);
	ByteArray encodeString(const std::wstring& str, bool writeTermination = true);
	ByteArray encodeTermination();
private:
	struct TableEntry
	{
		size_t hexPos;
		size_t hexLen;
		size_t valueLen;
	};

	ByteArray hexData;
	std::vector<TableEntry> entries;
	Trie lookup;
	TableEntry terminationEntry;
};

// file: Core/Misc.h


#include <vector>

class Logger
{
public:
	enum ErrorType { Warning, Error, FatalError, Notice };

	static void clear();
	static void printLine(const std::wstring& text);
	static void printLine(const std::string& text);

	template <typename... Args>
	static void printLine(const wchar_t* text, const Args&... args)
	{
		std::wstring message = tfm::format(text,args...);
		printLine(message);
	}

	static void print(const std::wstring& text);

	template <typename... Args>
	static void print(const wchar_t* text, const Args&... args)
	{
		std::wstring message = tfm::format(text,args...);
		print(message);
	}

	static void printError(ErrorType type, const std::wstring& text);
	static void printError(ErrorType type, const wchar_t* text);
	static void queueError(ErrorType type, const std::wstring& text);
	static void queueError(ErrorType type, const wchar_t* text);

	template <typename... Args>
	static void printError(ErrorType type, const wchar_t* text, const Args&... args)
	{
		std::wstring message = tfm::format(text,args...);
		printError(type,message);
	}

	template <typename... Args>
	static void queueError(ErrorType type, const wchar_t* text, const Args&... args)
	{
		std::wstring message = tfm::format(text,args...);
		queueError(type,message);
	}

	static void printQueue();
	static void clearQueue() { queue.clear(); };
	static std::vector<std::wstring> getErrors() { return errors; };
	static bool hasError() { return error; };
	static bool hasFatalError() { return fatalError; };
	static void setErrorOnWarning(bool b) { errorOnWarning = b; };
	static void setSilent(bool b) { silent = b; };
	static bool isSilent() { return silent; }
	static void suppressErrors() { ++suppressLevel; }
	static void unsuppressErrors() { if (suppressLevel) --suppressLevel; }
private:
	static std::wstring formatError(ErrorType type, const wchar_t* text);
	static void setFlags(ErrorType type);

	struct QueueEntry
	{
		ErrorType type;
		std::wstring text;
	};

	static std::vector<QueueEntry> queue;
	static std::vector<std::wstring> errors;
	static bool error;
	static bool fatalError;
	static bool errorOnWarning;
	static bool silent;
	static int suppressLevel;
};

class TempData
{
public:
	void setFileName(const fs::path& name) { file.setFileName(name); };
	void clear() { file.setFileName({}); }
	void start();
	void end();
	void writeLine(int64_t memoryAddress, const std::wstring& text);
	bool isOpen() { return file.isOpen(); }
private:
	TextFile file;
};

// file: Core/Assembler.h


#include <memory>
#include <string>
#include <vector>

class AssemblerFile;

#define ARMIPS_VERSION_MAJOR    0
#define ARMIPS_VERSION_MINOR    11
#define ARMIPS_VERSION_REVISION 0

enum class ArmipsMode { FILE, MEMORY };

struct LabelDefinition
{
	std::wstring originalName;
	std::wstring name;
	int64_t value;
};

struct EquationDefinition
{
	std::wstring name;
	std::wstring value;
};

struct ArmipsArguments
{
	// common
	ArmipsMode mode;
	int symFileVersion;
	bool errorOnWarning;
	bool silent;
	bool showStats;
	std::vector<std::wstring>* errorsResult;
	std::vector<EquationDefinition> equList;
	std::vector<LabelDefinition> labels;

	// file mode
	fs::path inputFileName;
	fs::path tempFileName;
	fs::path symFileName;
	bool useAbsoluteFileNames;

	// memory mode
	std::shared_ptr<AssemblerFile> memoryFile;
	std::wstring content;

	ArmipsArguments()
	{
		mode = ArmipsMode::FILE;
		symFileVersion = 0;
		errorOnWarning = false;
		silent = false;
		showStats = false;
		errorsResult = nullptr;
		useAbsoluteFileNames = true;
	}
};

bool runArmips(ArmipsArguments& settings);

// file: Core/SymbolTable.h

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

struct LabelDefinition;

struct SymbolKey
{
	std::wstring name;
	int file;
	int section;
};

bool operator<(SymbolKey const& lhs, SymbolKey const& rhs);

class Label
{
public:
	Label(std::wstring name): name(name),defined(false),data(false),updateInfo(true),info(0) { };
	const std::wstring getName() { return name; };
	void setOriginalName(const std::wstring& name) { originalName = name; }
	const std::wstring getOriginalName() { return originalName.empty() ? name : originalName; }
	int64_t getValue() { return value; };
	void setValue(int64_t val) { value = val; };
	bool hasPhysicalValue() { return physicalValueSet; }
	int64_t getPhysicalValue() { return physicalValue; }
	void setPhysicalValue(int64_t val) { physicalValue = val; physicalValueSet = true; }
	bool isDefined() { return defined; };
	void setDefined(bool b) { defined = b; };
	bool isData() { return data; };
	void setIsData(bool b) { data = b; };
	void setInfo(int inf) { info = inf; };
	int getInfo() { return info; };
	void setUpdateInfo(bool b) { updateInfo = b; };
	bool getUpdateInfo() { return updateInfo; };
	void setSection(int num) { section = num; }
	int getSection() { return section; }
private:
	std::wstring name, originalName;
	int64_t value;
	int64_t physicalValue;
	bool physicalValueSet = false;
	bool defined;
	bool data;
	bool updateInfo;
	int info;
	int section;
};

class SymbolTable
{
public:
	SymbolTable();
	~SymbolTable();
	void clear();
	bool symbolExists(const std::wstring& symbol, int file, int section);
	static bool isValidSymbolName(const std::wstring& symbol);
	static bool isValidSymbolCharacter(wchar_t character, bool first = false);
	static bool isLocalSymbol(const std::wstring& symbol, size_t pos = 0) { return symbol.size() >= pos+2 && symbol[pos+0] == '@' && symbol[pos+1] == '@'; };
	static bool isStaticSymbol(const std::wstring& symbol, size_t pos = 0) { return symbol.size() >= pos+1 && symbol[pos+0] == '@'; };
	static bool isGlobalSymbol(const std::wstring& symbol, size_t pos = 0) { return !isLocalSymbol(symbol) && !isStaticSymbol(symbol); };

	std::shared_ptr<Label> getLabel(const std::wstring& symbol, int file, int section);
	bool addEquation(const std::wstring& name, int file, int section, size_t referenceIndex);
	bool findEquation(const std::wstring& name, int file, int section, size_t& dest);
	void addLabels(const std::vector<LabelDefinition>& labels);
	int findSection(int64_t address);

	std::wstring getUniqueLabelName(bool local = false);
	size_t getLabelCount() { return labels.size(); };
	size_t getEquationCount() { return equationsCount; };
	bool isGeneratedLabel(const std::wstring& name) { return generatedLabels.find(name) != generatedLabels.end(); }
private:
	void setFileSectionValues(const std::wstring& symbol, int& file, int& section);

	enum SymbolType { LabelSymbol, EquationSymbol };
	struct SymbolInfo
	{
		SymbolType type;
		size_t index;
	};

	std::map<SymbolKey,SymbolInfo> symbols;
	std::vector<std::shared_ptr<Label>> labels;
	size_t equationsCount;
	size_t uniqueCount;
	std::set<std::wstring> generatedLabels;
};

// file: Core/Common.h


#include <string>
#include <vector>

class AssemblerFile;
class CArchitecture;

class FileList
{
public:
	void add(const fs::path& path);

	const fs::path& path(int fileIndex) const;
	const fs::path& relative_path(int fileIndex) const;
	const std::wstring& wstring(int fileIndex) const;
	const std::wstring& relativeWstring(int fileIndex) const;

	size_t size() const;
	void clear();

private:
	class Entry
	{
	public:
		Entry(const fs::path& path);

		const fs::path& path() const;
		const fs::path& relativePath() const;
		const std::wstring& wstring() const;
		const std::wstring& relativeWstring() const;

	private:
		fs::path _path;
		fs::path _relativePath;
		std::wstring _string; // preconverted for performance
		std::wstring _relativeString; // preconverted for performance
	};

	std::vector<Entry> _entries;
};

typedef struct {
	int FileNum;
	int LineNumber;
	int TotalLineCount;
} tFileInfo;

typedef struct {
	FileList fileList;
	tFileInfo FileInfo;
	SymbolTable symbolTable;
	EncodingTable Table;
	int Section;
	bool nocash;
	bool relativeInclude;
	bool memoryMode;
	std::shared_ptr<AssemblerFile> memoryFile;
	bool multiThreading;
} tGlobal;

extern tGlobal Global;
extern CArchitecture* Arch;

class FileManager;
extern FileManager* g_fileManager;

fs::path getFullPathName(const fs::path& path);

bool checkLabelDefined(const std::wstring& labelName, int section);
bool checkValidLabelName(const std::wstring& labelName);

bool isPowerOfTwo(int64_t n);

// file: Core/Allocations.h

#include <cstdint>
#include <map>

struct AllocationStats
{
	int64_t largestPosition;
	int64_t largestSize;
	int64_t largestUsage;

	int64_t largestFreePosition;
	int64_t largestFreeSize;
	int64_t largestFreeUsage;

	int64_t sharedFreePosition;
	int64_t sharedFreeSize;
	int64_t sharedFreeUsage;

	int64_t totalSize;
	int64_t totalUsage;
	int64_t sharedSize;
	int64_t sharedUsage;

	int64_t largestPoolPosition;
	int64_t largestPoolSize;
	int64_t totalPoolSize;
};

class Allocations
{
public:
	static void clear();
	static void setArea(int64_t fileID, int64_t position, int64_t space, int64_t usage, bool usesFill, bool shared);
	static void forgetArea(int64_t fileID, int64_t position, int64_t space);

	static void setPool(int64_t fileID, int64_t position, int64_t size);
	static void forgetPool(int64_t fileID, int64_t position, int64_t size);

	static void clearSubAreas();
	static bool allocateSubArea(int64_t fileID, int64_t& position, int64_t minRange, int64_t maxRange, int64_t size);
	static int64_t getSubAreaUsage(int64_t fileID, int64_t position);

	static bool canTrimSpace();

	static void validateOverlap();
	static AllocationStats collectStats();

private:
	struct Key
	{
		int64_t fileID;
		int64_t position;

		inline bool operator <(const Allocations::Key &other) const
		{
			return std::tie(fileID, position) < std::tie(other.fileID, other.position);
		}
	};
	struct Usage
	{
		int64_t space;
		int64_t usage;
		bool usesFill;
		bool shared;
	};

	struct SubArea
	{
		int64_t offset;
		int64_t size;
	};

	static void collectAreaStats(AllocationStats &stats);
	static void collectPoolStats(AllocationStats &stats);

	static int64_t getSubAreaUsage(Key key)
	{
		return getSubAreaUsage(key.fileID, key.position);
	}

	static std::map<Key, Usage> allocations;
	static std::map<Key, int64_t> pools;
	static std::multimap<Key, SubArea> subAreas;
	static bool keepPositions;
	static bool nextKeepPositions;
	static bool keptPositions;
};

// file: Core/Allocations.cpp

std::map<Allocations::Key, Allocations::Usage> Allocations::allocations;
std::map<Allocations::Key, int64_t> Allocations::pools;
std::multimap<Allocations::Key, Allocations::SubArea> Allocations::subAreas;

// Are we keeping existing positions this round?  Start with no, since we have none.
bool Allocations::keepPositions = false;
// Should we keep positions next time?  This is set to false on allocation failure.
bool Allocations::nextKeepPositions = true;
// Did we adjust any positions to keep the old?
bool Allocations::keptPositions = false;

void Allocations::clear()
{
	allocations.clear();
	keepPositions = false;
	nextKeepPositions = true;
	keptPositions = false;
}

void Allocations::setArea(int64_t fileID, int64_t position, int64_t space, int64_t usage, bool usesFill, bool shared)
{
	Key key{ fileID, position };
	allocations[key] = Usage{ space, usage, usesFill, shared };
}

void Allocations::forgetArea(int64_t fileID, int64_t position, int64_t space)
{
	Key key{ fileID, position };
	auto it = allocations.find(key);
	if (it != allocations.end() && it->second.space == space)
		allocations.erase(it);

	subAreas.erase(key);
}

void Allocations::setPool(int64_t fileID, int64_t position, int64_t size)
{
	Key key{ fileID, position };
	pools[key] = size;
}

void Allocations::forgetPool(int64_t fileID, int64_t position, int64_t size)
{
	Key key{ fileID, position };
	auto it = pools.find(key);
	if (it != pools.end() && it->second == size)
		pools.erase(it);
}

bool Allocations::allocateSubArea(int64_t fileID, int64_t& position, int64_t minRange, int64_t maxRange, int64_t size)
{
	for (auto it : allocations)
	{
		if (it.first.fileID != fileID || !it.second.shared)
			continue;
		if (minRange != -1 && it.first.position + it.second.space < minRange)
			continue;
		if (maxRange != -1 && it.first.position > maxRange)
			continue;

		int64_t actualUsage = it.second.usage + getSubAreaUsage(it.first);
		int64_t possiblePosition = it.first.position + actualUsage;
		if (minRange != -1 && possiblePosition < minRange)
			continue;
		if (maxRange != -1 && possiblePosition > maxRange)
			continue;

		// Can we use the position it had before?  Nudge up size if so.
		if (keepPositions && position != -1 && position > possiblePosition)
		{
			int64_t offset = position - it.first.position;
			if (it.second.space >= offset + size && offset != actualUsage)
			{
				size += offset - actualUsage;
				keptPositions = true;
				// Fall through to reuse the emplace.
				possiblePosition = position;
			}
		}

		if (it.second.space >= actualUsage + size)
		{
			position = possiblePosition;
			subAreas.emplace(it.first, SubArea{ actualUsage, size });
			return true;
		}
	}

	nextKeepPositions = false;
	return false;
}

void Allocations::clearSubAreas()
{
	subAreas.clear();
	keepPositions = nextKeepPositions;
	nextKeepPositions = true;
	keptPositions = false;
}

bool Allocations::canTrimSpace()
{
	return keptPositions;
}

int64_t Allocations::getSubAreaUsage(int64_t fileID, int64_t position)
{
	Key key{ fileID, position };
	// For safety, just find the end of the last sub area.
	int64_t maxExtent = 0;
	auto range = subAreas.equal_range(key);
	for (auto it = range.first; it != range.second; ++it)
	{
		int64_t extent = it->second.offset + it->second.size;
		maxExtent = std::max(extent, maxExtent);
	}

	// Now subtract out the original usage.
	if (maxExtent <= allocations[key].usage)
		return 0;
	return maxExtent - allocations[key].usage;
}

void Allocations::validateOverlap()
{
	// An easy mistake to make is a "subarea" where the parent area fills, and erases the subarea.
	// Let's detect any sort of area overlap and report a warning.
	Key lastKey{ -1, -1 };
	int64_t lastEndPosition = -1;
	Usage lastUsage{};

	for (auto it : allocations) {
		if (it.first.fileID == lastKey.fileID && it.first.position > lastKey.position && it.first.position < lastEndPosition) {
			// First, the obvious: does the content overlap?
			if (it.first.position < lastKey.position + lastUsage.usage)
				Logger::queueError(Logger::Warning, L"Content of areas %08llX and %08llx overlap", lastKey.position, it.first.position);
			// Next question, does the earlier one fill?
			else if (it.second.usesFill && lastUsage.usesFill)
				Logger::queueError(Logger::Warning, L"Areas %08llX and %08llx overlap and both fill", lastKey.position, it.first.position);

			// If the new area ends before the last, keep it as the last.
			if (lastEndPosition > it.first.position + it.second.space) {
				// But update the usage to the max position.
				int64_t newUsageEnd = it.first.position + it.second.usage + getSubAreaUsage(it.first);
				lastUsage.usage = newUsageEnd - lastKey.position;
				continue;
			}
		}

		lastKey = it.first;
		lastUsage = it.second;
		lastUsage.usage += getSubAreaUsage(lastKey);
		lastEndPosition = it.first.position + it.second.space;
	}
}

AllocationStats Allocations::collectStats()
{
	AllocationStats stats{};
	collectAreaStats(stats);
	collectPoolStats(stats);
	return stats;
}

void Allocations::collectAreaStats(AllocationStats &stats)
{
	// Need to work out overlaps.
	Key lastKey{ -1, -1 };
	int64_t lastEndPosition = -1;
	Usage lastUsage{};

	auto applyUsage = [&stats](int64_t position, const Usage &usage)
	{
		if (usage.space > stats.largestSize)
		{
			stats.largestPosition = position;
			stats.largestSize = usage.space;
			stats.largestUsage = usage.usage;
		}

		if (usage.space - usage.usage > stats.largestFreeSize - stats.largestFreeUsage)
		{
			stats.largestFreePosition = position;
			stats.largestFreeSize = usage.space;
			stats.largestFreeUsage = usage.usage;
		}

		// We assume overlaps agree on sharing, hopefully...
		if (usage.shared && usage.space - usage.usage > stats.sharedFreeSize - stats.sharedFreeUsage)
		{
			stats.sharedFreePosition = position;
			stats.sharedFreeSize = usage.space;
			stats.sharedFreeUsage = usage.usage;
		}

		stats.totalSize += usage.space;
		stats.totalUsage += usage.usage;
		if (usage.shared)
		{
			stats.sharedSize += usage.space;
			stats.sharedUsage += usage.usage;
		}
	};

	for (auto it : allocations)
	{
		if (it.first.fileID == lastKey.fileID && it.first.position > lastKey.position && it.first.position < lastEndPosition)
		{
			// Overlap, merge.
			int64_t lastUsageEnd = lastKey.position + lastUsage.usage;
			int64_t newUsageEnd = it.first.position + it.second.usage + getSubAreaUsage(it.first);

			if (lastUsageEnd >= it.first.position)
				lastUsage.usage += newUsageEnd - lastUsageEnd;
			else
				lastUsage.usage += it.second.usage + getSubAreaUsage(it.first);

			lastEndPosition = it.first.position + it.second.space;
			lastUsage.space = lastEndPosition - lastKey.position;

			continue;
		}

		if (lastKey.position != -1)
			applyUsage(lastKey.position, lastUsage);

		lastKey = it.first;
		lastUsage = it.second;
		lastUsage.usage += getSubAreaUsage(lastKey);
		lastEndPosition = it.first.position + it.second.space;
	}

	if (lastKey.position != -1)
		applyUsage(lastKey.position, lastUsage);
}

void Allocations::collectPoolStats(AllocationStats &stats)
{
	for (auto it : pools)
	{
		if (it.second > stats.largestPoolSize)
		{
			stats.largestPoolPosition = it.first.position;
			stats.largestPoolSize = it.second;
		}

		stats.totalPoolSize += it.second;
	}
}

// file: Parser/DirectivesParser.h

#include <memory>
#include <unordered_map>

class CAssemblerCommand;
class Parser;

using DirectiveFunc = std::unique_ptr<CAssemblerCommand> (*)(Parser&,int);

struct DirectiveEntry {
	DirectiveFunc function;
	int flags;
};

using DirectiveMap = std::unordered_multimap<std::wstring, const DirectiveEntry>;

#define DIRECTIVE_USERMASK			0x0000FFFF

// Global flags
#define DIRECTIVE_NOCASHON			0x00010000
#define DIRECTIVE_NOCASHOFF			0x00020000
#define DIRECTIVE_MIPSRESETDELAY	0x00040000
#define DIRECTIVE_DISABLED			0x00080000
#define DIRECTIVE_NOTINMEMORY		0x00100000
#define DIRECTIVE_MANUALSEPARATOR	0x00200000

// file directive flags
#define DIRECTIVE_POS_PHYSICAL		0x00000001
#define DIRECTIVE_POS_VIRTUAL		0x00000002
#define DIRECTIVE_ALIGN_PHYSICAL	0x00000001
#define DIRECTIVE_ALIGN_VIRTUAL		0x00000002
#define DIRECTIVE_ALIGN_FILL		0x00000004

// conditional directive flags
#define DIRECTIVE_COND_IF			0x00000001
#define DIRECTIVE_COND_IFDEF		0x00000002
#define DIRECTIVE_COND_IFNDEF		0x00000003

// data directive flags
#define DIRECTIVE_DATA_8			0x00000001
#define DIRECTIVE_DATA_16			0x00000002
#define DIRECTIVE_DATA_32			0x00000003
#define DIRECTIVE_DATA_64			0x00000004
#define DIRECTIVE_DATA_ASCII		0x00000005
#define DIRECTIVE_DATA_SJIS			0x00000006
#define DIRECTIVE_DATA_CUSTOM		0x00000007
#define DIRECTIVE_DATA_FLOAT		0x00000008
#define DIRECTIVE_DATA_DOUBLE		0x00000009
#define DIRECTIVE_DATA_TERMINATION	0x00000100

// message directive flags
#define DIRECTIVE_MSG_WARNING		0x00000001
#define DIRECTIVE_MSG_ERROR			0x00000002
#define DIRECTIVE_MSG_NOTICE		0x00000003

// MIPS directive flags
#define DIRECTIVE_MIPS_PSX			0x00000001
#define DIRECTIVE_MIPS_PS2			0x00000002
#define DIRECTIVE_MIPS_PSP			0x00000003
#define DIRECTIVE_MIPS_N64			0x00000004
#define DIRECTIVE_MIPS_RSP			0x00000005

// ARM directive flags
#define DIRECTIVE_ARM_GBA			0x00000001
#define DIRECTIVE_ARM_NDS			0x00000002
#define DIRECTIVE_ARM_3DS			0x00000003
#define DIRECTIVE_ARM_BIG			0x00000004
#define DIRECTIVE_ARM_LITTLE		0x00000005

// Area directive flags
#define DIRECTIVE_AREA_SHARED		0x00000001

extern const DirectiveMap directives;

// file: Archs/MIPS/MipsMacros.h


#include <memory>

struct MipsImmediateData;
struct MipsRegisterData;

#define MIPSM_B						0x00000001
#define MIPSM_BU					0x00000002
#define MIPSM_HW					0x00000003
#define MIPSM_HWU					0x00000004
#define MIPSM_W						0x00000005
#define MIPSM_WU					0x00000006
#define MIPSM_DW					0x00000007
#define MIPSM_LLSCW					0x00000008
#define MIPSM_LLSCDW				0x00000009
#define MIPSM_COP1					0x0000000a
#define MIPSM_COP2					0x0000000b
#define MIPSM_DCOP1					0x0000000c
#define MIPSM_DCOP2					0x0000000d
#define MIPSM_ACCESSMASK			0x0000000f

#define MIPSM_NE					0x00000001
#define MIPSM_LT					0x00000002
#define MIPSM_LTU					0x00000003
#define MIPSM_GE					0x00000004
#define MIPSM_GEU					0x00000005
#define MIPSM_EQ					0x00000006
#define MIPSM_CONDITIONMASK			0x00000007

#define MIPSM_IMM					0x00000200
#define MIPSM_LEFT					0x00000400
#define MIPSM_RIGHT					0x00000800
#define MIPSM_UNALIGNED				0x00001000
#define MIPSM_DONTWARNDELAYSLOT		0x00002000
#define MIPSM_UPPER					0x00004000
#define MIPSM_LOWER					0x00008000
#define MIPSM_LOAD					0x00010000
#define MIPSM_STORE					0x00020000
#define MIPSM_LIKELY				0x00040000
#define MIPSM_REVCMP				0x00080000

class Parser;

using MipsMacroFunc = std::unique_ptr<CAssemblerCommand> (*)(Parser&,MipsRegisterData&,MipsImmediateData&,int);

struct MipsMacroDefinition {
	const wchar_t* name;
	const wchar_t* args;
	MipsMacroFunc function;
	int flags;
};

extern const MipsMacroDefinition mipsMacros[];

class MipsMacroCommand: public CAssemblerCommand
{
public:
	MipsMacroCommand(std::unique_ptr<CAssemblerCommand> content, int macroFlags);
	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
private:
	std::unique_ptr<CAssemblerCommand> content;
	int macroFlags;
	bool IgnoreLoadDelay;
};

// file: Archs/MIPS/MipsParser.h


#include <memory>
#include <string>

class CAssemblerCommand;
class Expression;
class Parser;

struct MipsMacroDefinition;
struct tMipsOpcode;

struct MipsRegisterDescriptor {
	const wchar_t* name;
	int num;
};

class MipsParser
{
public:
	std::unique_ptr<CAssemblerCommand> parseDirective(Parser& parser);
	std::unique_ptr<CMipsInstruction> parseOpcode(Parser& parser);
	std::unique_ptr<CAssemblerCommand> parseMacro(Parser& parser);
private:
	bool parseRegisterNumber(Parser& parser, MipsRegisterValue& dest, int numValues);
	bool parseRegisterTable(Parser& parser, MipsRegisterValue& dest, const MipsRegisterDescriptor* table, size_t count);
	bool parseRegister(Parser& parser, MipsRegisterValue& dest);
	bool parseFpuRegister(Parser& parser, MipsRegisterValue& dest);
	bool parseFpuControlRegister(Parser& parser, MipsRegisterValue& dest);
	bool parseCop0Register(Parser& parser, MipsRegisterValue& dest);
	bool parsePs2Cop2Register(Parser& parser, MipsRegisterValue& dest);
	bool parsePsxCop2DataRegister(Parser& parser, MipsRegisterValue& dest);
	bool parsePsxCop2ControlRegister(Parser& parser, MipsRegisterValue& dest);
	bool parseRspCop0Register(Parser& parser, MipsRegisterValue& dest);
	bool parseRspVectorControlRegister(Parser& parser, MipsRegisterValue& dest);
	bool parseRspVectorRegister(Parser& parser, MipsRegisterValue& dest);
	bool parseRspVectorElement(Parser& parser, MipsRegisterValue& dest);
	bool parseRspScalarElement(Parser& parser, MipsRegisterValue& dest);
	bool parseRspOffsetElement(Parser& parser, MipsRegisterValue& dest);
	bool parseVfpuRegister(Parser& parser, MipsRegisterValue& reg, int size);
	bool parseVfpuControlRegister(Parser& parser, MipsRegisterValue& reg);
	bool parseImmediate(Parser& parser, Expression& dest);
	bool parseVcstParameter(Parser& parser, int& result);
	bool parseVfpuVrot(Parser& parser, int& result, int size);
	bool parseVfpuCondition(Parser& parser, int& result);
	bool parseVpfxsParameter(Parser& parser, int& result);
	bool parseVpfxdParameter(Parser& parser, int& result);
	bool parseCop2BranchCondition(Parser& parser, int& result);
	bool parseWb(Parser& parser);

	bool decodeCop2BranchCondition(const std::wstring& text, size_t& pos, int& result);
	bool decodeVfpuType(const std::wstring& name, size_t& pos, int& dest);
	bool decodeOpcode(const std::wstring& name, const tMipsOpcode& opcode);

	void setOmittedRegisters(const tMipsOpcode& opcode);
	bool matchSymbol(Parser& parser, wchar_t symbol);
	bool parseParameters(Parser& parser, const tMipsOpcode& opcode);
	bool parseMacroParameters(Parser& parser, const MipsMacroDefinition& macro);

	MipsRegisterData registers;
	MipsImmediateData immediate;
	MipsOpcodeData opcodeData;
	bool hasFixedSecondaryImmediate;
};

class MipsOpcodeFormatter
{
public:
	const std::wstring& formatOpcode(const MipsOpcodeData& opData, const MipsRegisterData& regData,
		const MipsImmediateData& immData);
private:
	void handleOpcodeName(const MipsOpcodeData& opData);
	void handleOpcodeParameters(const MipsOpcodeData& opData, const MipsRegisterData& regData,
		const MipsImmediateData& immData);
	void handleImmediate(MipsImmediateType type, unsigned int originalValue, unsigned int opcodeFlags);

	std::wstring buffer;
};

// file: Archs/MIPS/CMipsInstruction.cpp


CMipsInstruction::CMipsInstruction(MipsOpcodeData& opcode, MipsImmediateData& immediate, MipsRegisterData& registers)
{
	this->opcodeData = opcode;
	this->immediateData = immediate;
	this->registerData = registers;

	addNop = false;
	IgnoreLoadDelay = Mips.GetIgnoreDelay();
}

CMipsInstruction::~CMipsInstruction()
{

}

int getImmediateBits(MipsImmediateType type)
{
	switch (type)
	{
	case MipsImmediateType::Immediate5:
		return 5;
	case MipsImmediateType::Immediate7:
		return 7;
	case MipsImmediateType::Immediate10:
		return 10;
	case MipsImmediateType::Immediate16:
	case MipsImmediateType::ImmediateHalfFloat:
		return 16;
	case MipsImmediateType::Immediate20:
	case MipsImmediateType::Immediate20_0:
		return 20;
	case MipsImmediateType::Immediate25:
		return 25;
	case MipsImmediateType::Immediate26:
		return 26;
	default:
		return 0;
	}
}

// http://code.google.com/p/jpcsp/source/browse/trunk/src/jpcsp/Allegrex/VfpuState.java?spec=svn3676&r=3383#1196
int CMipsInstruction::floatToHalfFloat(int i)
{
	int s = ((i >> 16) & 0x00008000); // sign
	int e = ((i >> 23) & 0x000000ff) - (127 - 15); // exponent
	int f = ((i >> 0) & 0x007fffff); // fraction

	// need to handle NaNs and Inf?
	if (e <= 0) {
		if (e < -10) {
			if (s != 0) {
				// handle -0.0
				return 0x8000;
			}
			return 0;
		}
		f = (f | 0x00800000) >> (1 - e);
		return s | (f >> 13);
	} else if (e == 0xff - (127 - 15)) {
		if (f == 0) {
			// Inf
			return s | 0x7c00;
		}
		// NAN
		return s | 0x7fff;
	}

	if (e > 30) {
		// Overflow
		return s | 0x7c00;
	}

	return s | (e << 10) | (f >> 13);
}

bool CMipsInstruction::Validate(const ValidateState &state)
{
	bool Result = false;

	bool previousNop = addNop;
	addNop = false;

	RamPos = g_fileManager->getVirtualAddress();
	if (RamPos % 4)
	{
		Logger::queueError(Logger::Error,L"opcode not aligned to word boundary");
		return false;
	}

	// check immediates
	if (immediateData.primary.type != MipsImmediateType::None)
	{
		if (immediateData.primary.expression.isLoaded())
		{
			if (!immediateData.primary.expression.evaluateInteger(immediateData.primary.value))
			{
				Logger::queueError(Logger::Error, L"Invalid immediate expression");
				return false;
			}

			immediateData.primary.originalValue = immediateData.primary.value;
		}

		if (immediateData.primary.type == MipsImmediateType::ImmediateHalfFloat)
			immediateData.primary.value = floatToHalfFloat(immediateData.primary.originalValue);

		if (opcodeData.opcode.flags & MO_IMMALIGNED)	// immediate must be aligned
		{
			if (immediateData.primary.value % 4)
			{
				Logger::queueError(Logger::Error,L"Immediate must be word aligned");
				return false;
			}
		}

		if (opcodeData.opcode.flags & MO_NEGIMM) 		// negated immediate
		{
			immediateData.primary.value = -immediateData.primary.value;
		} else if (opcodeData.opcode.flags & MO_IPCA)	// absolute value >> 2
		{
			immediateData.primary.value = (immediateData.primary.value >> 2) & 0x3FFFFFF;
		} else if (opcodeData.opcode.flags & MO_IPCR)	// relative 16 bit value
		{
			int num = (int) (immediateData.primary.value-RamPos-4);

			if (num > 0x20000 || num < (-0x20000))
			{
				Logger::queueError(Logger::Error,L"Branch target %08X out of range",immediateData.primary.value);
				return false;
			}
			immediateData.primary.value = num >> 2;
		} else if (opcodeData.opcode.flags & (MO_RSP_HWOFFSET | MO_RSP_WOFFSET | MO_RSP_DWOFFSET | MO_RSP_QWOFFSET))
		{
			int shift = 0;

			if (opcodeData.opcode.flags & MO_RSP_HWOFFSET) shift = 1;
			else if (opcodeData.opcode.flags & MO_RSP_WOFFSET) shift = 2;
			else if (opcodeData.opcode.flags & MO_RSP_DWOFFSET) shift = 3;
			else if (opcodeData.opcode.flags & MO_RSP_QWOFFSET) shift = 4;

			if (immediateData.primary.value & ((1 << shift) - 1))
			{
				Logger::queueError(Logger::Error,L"Offset must be %d-byte aligned",1<<shift);
				return false;
			}
			immediateData.primary.value = immediateData.primary.value >> shift;
		}

		int immediateBits = getImmediateBits(immediateData.primary.type);
		unsigned int mask = (0xFFFFFFFF << (32-immediateBits)) >> (32-immediateBits);
		int digits = (immediateBits+3) / 4;

		if ((unsigned int)std::abs(immediateData.primary.value) > mask)
		{
			Logger::queueError(Logger::Error,L"Immediate value 0x%0*X out of range",digits,immediateData.primary.value);
			return false;
		}

		immediateData.primary.value &= mask;
	}

	if (immediateData.secondary.type != MipsImmediateType::None)
	{
		if (immediateData.secondary.expression.isLoaded())
		{
			if (!immediateData.secondary.expression.evaluateInteger(immediateData.secondary.value))
			{
				Logger::queueError(Logger::Error, L"Invalid immediate expression");
				return false;
			}

			immediateData.secondary.originalValue = immediateData.secondary.value;
		}

		switch (immediateData.secondary.type)
		{
		case MipsImmediateType::CacheOp:
			if ((unsigned int)immediateData.secondary.value > 0x1f)
			{
				Logger::queueError(Logger::Error,L"Immediate value %02X out of range",immediateData.secondary.value);
				return false;
			}
			break;
		case MipsImmediateType::Ext:
		case MipsImmediateType::Ins:
			if (immediateData.secondary.value > 32 || immediateData.secondary.value == 0)
			{
				Logger::queueError(Logger::Error,L"Immediate value %02X out of range",immediateData.secondary.value);
				return false;
			}

			immediateData.secondary.value--;
			if (immediateData.secondary.type == MipsImmediateType::Ins)
				immediateData.secondary.value += immediateData.primary.value;
			break;
		case MipsImmediateType::Cop2BranchType:
		default:
			break;
		}
	}

	// check load delay
	if (Mips.hasLoadDelay() && Mips.GetLoadDelay() && !IgnoreLoadDelay)
	{
		bool fix = false;

		if (registerData.grd.num != -1 && registerData.grd.num == Mips.GetLoadDelayRegister())
		{
			Logger::queueError(Logger::Warning,L"register %S may not be available due to load delay",registerData.grd.name);
			fix = true;
		} else if (registerData.grs.num != -1 && registerData.grs.num == Mips.GetLoadDelayRegister())
		{
			Logger::queueError(Logger::Warning,L"register %S may not be available due to load delay",registerData.grs.name);
			fix = true;
		} else if (registerData.grt.num != -1 && registerData.grt.num == Mips.GetLoadDelayRegister()
			&& !(opcodeData.opcode.flags & MO_IGNORERTD))
		{
			Logger::queueError(Logger::Warning,L"register %S may not be available due to load delay",registerData.grt.name);
			fix = true;
		}

		if (Mips.GetFixLoadDelay() && fix)
		{
			addNop = true;
			Logger::queueError(Logger::Notice,L"added nop to ensure correct behavior");
		}
	}

	if ((opcodeData.opcode.flags & MO_NODELAYSLOT) && Mips.GetDelaySlot() && !IgnoreLoadDelay)
	{
		Logger::queueError(Logger::Error,L"This instruction can't be in a delay slot");
	}

	Mips.SetDelaySlot((opcodeData.opcode.flags & MO_DELAY) != 0);

	// now check if this opcode causes a load delay
	if (Mips.hasLoadDelay())
		Mips.SetLoadDelay((opcodeData.opcode.flags & MO_DELAYRT) != 0,registerData.grt.num);

	if (previousNop != addNop)
		Result = true;

	g_fileManager->advanceMemory(addNop ? 8 : 4);
	return Result;
}

void CMipsInstruction::encodeNormal() const
{
	int32_t encoding = opcodeData.opcode.destencoding;

	if (registerData.grs.num != -1) encoding |= MIPS_RS(registerData.grs.num);	// source reg
	if (registerData.grt.num != -1) encoding |= MIPS_RT(registerData.grt.num);	// target reg
	if (registerData.grd.num != -1) encoding |= MIPS_RD(registerData.grd.num);	// dest reg

	if (registerData.frt.num != -1) encoding |= MIPS_FT(registerData.frt.num);	// float target reg
	if (registerData.frs.num != -1) encoding |= MIPS_FS(registerData.frs.num);	// float source reg
	if (registerData.frd.num != -1) encoding |= MIPS_FD(registerData.frd.num);	// float dest reg

	if (registerData.ps2vrt.num != -1) encoding |= (registerData.ps2vrt.num << 16);	// ps2 vector target reg
	if (registerData.ps2vrs.num != -1) encoding |= (registerData.ps2vrs.num << 21);	// ps2 vector source reg
	if (registerData.ps2vrd.num != -1) encoding |= (registerData.ps2vrd.num << 6);	// ps2 vector dest reg

	if (registerData.rspvrt.num != -1) encoding |= MIPS_FT(registerData.rspvrt.num);	// rsp vector target reg
	if (registerData.rspvrs.num != -1) encoding |= MIPS_FS(registerData.rspvrs.num);	// rsp vector source reg
	if (registerData.rspvrd.num != -1) encoding |= MIPS_FD(registerData.rspvrd.num);	// rsp vector dest reg

	if (registerData.rspve.num != -1) encoding |= MIPS_RSP_VE(registerData.rspve.num);			// rsp element
	if (registerData.rspvde.num != -1) encoding |= MIPS_RSP_VDE(registerData.rspvde.num);		// rsp destination element
	if (registerData.rspvealt.num != -1) encoding |= MIPS_RSP_VEALT(registerData.rspvealt.num);	// rsp element (alt. placement)

	if (!(opcodeData.opcode.flags & MO_VFPU_MIXED) && registerData.vrt.num != -1)			// vfpu rt
		encoding |= registerData.vrt.num << 16;

	switch (immediateData.primary.type)
	{
	case MipsImmediateType::Immediate5:
	case MipsImmediateType::Immediate10:
	case MipsImmediateType::Immediate20:
		encoding |= immediateData.primary.value << 6;
		break;
	case MipsImmediateType::Immediate16:
	case MipsImmediateType::Immediate25:
	case MipsImmediateType::Immediate26:
	case MipsImmediateType::Immediate20_0:
	case MipsImmediateType::Immediate7:
	case MipsImmediateType::ImmediateHalfFloat:
		encoding |= immediateData.primary.value;
		break;
	default:
		// TODO: Assert?
		break;
	}

	switch (immediateData.secondary.type)
	{
	case MipsImmediateType::CacheOp:
		encoding |= immediateData.secondary.value << 16;
		break;
	case MipsImmediateType::Ext:
	case MipsImmediateType::Ins:
		encoding |= immediateData.secondary.value << 11;
		break;
	case MipsImmediateType::Cop2BranchType:
		encoding |= immediateData.secondary.value << 18;
		break;
	default:
		// TODO: Assert?
		break;
	}

	if (opcodeData.opcode.flags & MO_VFPU_MIXED)
	{
		// always vrt
		encoding |= registerData.vrt.num >> 5;
		encoding |= (registerData.vrt.num & 0x1F) << 16;
	}

	g_fileManager->writeU32((uint32_t)encoding);
}

void CMipsInstruction::encodeVfpu() const
{
	int encoding = opcodeData.opcode.destencoding;

	if (opcodeData.vectorCondition != -1) encoding |= (opcodeData.vectorCondition << 0);
	if (registerData.vrd.num != -1) encoding |= (registerData.vrd.num << 0);
	if (registerData.vrs.num != -1) encoding |= (registerData.vrs.num << 8);
	if (registerData.vrt.num != -1) encoding |= (registerData.vrt.num << 16);
	if (opcodeData.vfpuSize != -1 && (opcodeData.opcode.flags & (MO_VFPU_PAIR|MO_VFPU_SINGLE|MO_VFPU_TRIPLE|MO_VFPU_QUAD)) == 0)
	{
		if (opcodeData.vfpuSize & 1) encoding |= (1 << 7);
		if (opcodeData.vfpuSize & 2) encoding |= (1 << 15);
	}

	if (registerData.grt.num != -1) encoding |= (registerData.grt.num << 16);

	switch (immediateData.primary.type)
	{
	case MipsImmediateType::Immediate5:
		encoding |= immediateData.primary.value << 16;
		break;
	case MipsImmediateType::Immediate7:
		encoding |= immediateData.primary.value << 0;
		break;
	default:
		// TODO: Assert?
		break;
	}

	g_fileManager->writeU32((uint32_t)encoding);
}

void CMipsInstruction::Encode() const
{
	if (addNop)
		g_fileManager->writeU32(0);

	if (opcodeData.opcode.flags & MO_VFPU)
		encodeVfpu();
	else
		encodeNormal();
}

void CMipsInstruction::writeTempData(TempData& tempData) const
{
	MipsOpcodeFormatter formatter;
	tempData.writeLine(RamPos,formatter.formatOpcode(opcodeData,registerData,immediateData));
}

// file: Archs/MIPS/MipsExpressionFunctions.h


extern const ExpressionFunctionMap mipsExpressionFunctions;

// file: Archs/MIPS/MipsElfRelocator.h

enum {
	R_MIPS_NONE,
	R_MIPS_16,
	R_MIPS_32,
	R_MIPS_REL32,
	R_MIPS_26,
	R_MIPS_HI16,
	R_MIPS_LO16,
	R_MIPS_GPREL16,
	R_MIPS_LITERAL,
	R_MIPS_GOT16,
	R_MIPS_PC16,
	R_MIPS_CALL16,
	R_MIPS_GPREL32
};

class MipsElfRelocator: public IElfRelocator
{
public:
	int expectedMachine() const override;
	bool relocateOpcode(int type, const RelocationData& data, std::vector<RelocationAction>& actions, std::vector<std::wstring>& errors) override;
	bool finish(std::vector<RelocationAction>& actions, std::vector<std::wstring>& errors) override;
	void setSymbolAddress(RelocationData& data, int64_t symbolAddress, int symbolType) override;
	std::unique_ptr<CAssemblerCommand> generateCtorStub(std::vector<ElfRelocatorCtor>& ctors) override;
private:
	bool processHi16Entries(uint32_t lo16Opcode, int64_t lo16RelocationBase, std::vector<RelocationAction>& actions, std::vector<std::wstring>& errors);

	struct Hi16Entry
	{
		Hi16Entry(int64_t offset, int64_t relocationBase, uint32_t opcode) : offset(offset), relocationBase(relocationBase), opcode(opcode) {}
		int64_t offset;
		int64_t relocationBase;
		uint32_t opcode;
	};

	std::vector<Hi16Entry> hi16Entries;
};

// file: Archs/MIPS/Mips.cpp


CMipsArchitecture Mips;

CMipsArchitecture::CMipsArchitecture()
{
	FixLoadDelay = false;
	IgnoreLoadDelay = false;
	LoadDelay = false;
	LoadDelayRegister = 0;
	DelaySlot = false;
	Version = MARCH_INVALID;
}

std::unique_ptr<CAssemblerCommand> CMipsArchitecture::parseDirective(Parser& parser)
{
	MipsParser mipsParser;
	return mipsParser.parseDirective(parser);
}

std::unique_ptr<CAssemblerCommand> CMipsArchitecture::parseOpcode(Parser& parser)
{
	MipsParser mipsParser;

	std::unique_ptr<CAssemblerCommand> macro = mipsParser.parseMacro(parser);
	if (macro != nullptr)
		return macro;

	return mipsParser.parseOpcode(parser);
}

const ExpressionFunctionMap& CMipsArchitecture::getExpressionFunctions()
{
	return mipsExpressionFunctions;
}

void CMipsArchitecture::NextSection()
{
	LoadDelay = false;
	LoadDelayRegister = 0;
	DelaySlot = false;
}

void CMipsArchitecture::Revalidate()
{
	LoadDelay = false;
	LoadDelayRegister = 0;
	DelaySlot = false;
}

std::unique_ptr<IElfRelocator> CMipsArchitecture::getElfRelocator()
{
	switch (Version)
	{
	case MARCH_PS2:
	case MARCH_PSP:
	case MARCH_N64:
		return std::make_unique<MipsElfRelocator>();
	case MARCH_PSX:
	case MARCH_RSP:
	default:
		return nullptr;
	}
}

void CMipsArchitecture::SetLoadDelay(bool Delay, int Register)
{
	LoadDelay = Delay;
	LoadDelayRegister = Register;
}

// file: Archs/MIPS/MipsElfFile.h


class MipsElfFile: public AssemblerFile
{
public:
	MipsElfFile();
	virtual bool open(bool onlyCheck);
	virtual void close();
	virtual bool isOpen() { return opened; };
	virtual bool write(void* data, size_t length);
	virtual int64_t getVirtualAddress();
	virtual int64_t getPhysicalAddress();
	virtual int64_t getHeaderSize();
	virtual bool seekVirtual(int64_t virtualAddress);
	virtual bool seekPhysical(int64_t physicalAddress);
	virtual bool getModuleInfo(SymDataModuleInfo& info);
	virtual void beginSymData(SymbolData& symData);
	virtual void endSymData(SymbolData& symData);
	virtual const fs::path& getFileName() { return fileName; };

	bool load(const fs::path& fileName, const fs::path& outputFileName);
	void save();
	bool setSection(const std::wstring& name);
private:
	ElfFile elf;
	fs::path fileName;
	fs::path outputFileName;
	bool opened;
	int platform;

	int segment;
	int section;
	size_t sectionOffset;
};


class DirectiveLoadMipsElf: public CAssemblerCommand
{
public:
	DirectiveLoadMipsElf(const fs::path& fileName);
	DirectiveLoadMipsElf(const fs::path& inputName, const fs::path& outputName);
	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
	void writeSymData(SymbolData& symData) const override;
private:
	std::shared_ptr<MipsElfFile> file;
	fs::path inputName;
	fs::path outputName;
};

// file: Util/CRC.h

#include <cstddef>

unsigned short getCrc16(unsigned char* Source, size_t len);
unsigned int getCrc32(unsigned char* Source, size_t len);
unsigned int getChecksum(unsigned char* Source, size_t len);

// file: Archs/MIPS/MipsElfFile.cpp


MipsElfFile::MipsElfFile()
{
	platform = Mips.GetVersion();
	section = segment = -1;
	opened = false;
}

bool MipsElfFile::open(bool onlyCheck)
{
	opened = !onlyCheck;
	return true;
}

void MipsElfFile::close()
{
	if (isOpen())
		save();
}

void MipsElfFile::beginSymData(SymbolData& symData)
{
	symData.startModule(this);
}

void MipsElfFile::endSymData(SymbolData& symData)
{
	symData.endModule(this);
}

int64_t MipsElfFile::getVirtualAddress()
{
	if (segment != -1)
	{
		ElfSegment* seg = elf.getSegment(segment);
		ElfSection* sect = seg->getSection(section);
		int64_t addr = seg->getVirtualAddress() + sect->getOffset();
		return addr+sectionOffset;
	}

	// segmentless sections don't have a virtual address
	Logger::queueError(Logger::Error,L"Not inside a mapped section");
	return -1;
}

int64_t MipsElfFile::getPhysicalAddress()
{
	if (segment != -1)
	{
		ElfSegment* seg = elf.getSegment(segment);
		ElfSection* sect = seg->getSection(section);
		int64_t addr = seg->getOffset() + sect->getOffset();
		return addr;
	}

	if (section != -1)
	{
		ElfSection* sect = elf.getSegmentlessSection(section);
		return sect->getOffset();
	}

	Logger::queueError(Logger::Error,L"Not inside a section");
	return -1;
}

int64_t MipsElfFile::getHeaderSize()
{
	// this method is not used
	Logger::queueError(Logger::Error,L"Unimplemented method");
	return -1;
}

bool MipsElfFile::seekVirtual(int64_t virtualAddress)
{
	// search in segments
	for (size_t i = 0; i < elf.getSegmentCount(); i++)
	{
		ElfSegment* seg = elf.getSegment(i);
		int64_t segStart = seg->getVirtualAddress();
		int64_t segEnd = segStart+seg->getPhysSize();

		if (segStart <= virtualAddress && virtualAddress < segEnd)
		{
			// find section
			for (size_t l = 0; l < seg->getSectionCount(); l++)
			{
				ElfSection* sect = seg->getSection(l);
				int64_t sectStart = segStart+sect->getOffset();
				int64_t sectEnd = sectStart+sect->getSize();

				if (sectStart <= virtualAddress && virtualAddress < sectEnd)
				{
					segment = (int) i;
					section = (int) l;
					sectionOffset = (size_t) (virtualAddress-sectStart);
					return true;
				}
			}

			Logger::queueError(Logger::Error,L"Found segment, but no containing section");
			return false;
		}
	}

	// segmentless sections don't have a virtual address
	Logger::printError(Logger::Error,L"Couldn't find a mapped section");
	return false;
}

bool MipsElfFile::seekPhysical(int64_t physicalAddress)
{
	// search in segments
	for (size_t i = 0; i < elf.getSegmentCount(); i++)
	{
		ElfSegment* seg = elf.getSegment(i);
		int64_t segStart = seg->getOffset();
		int64_t segEnd = segStart+seg->getPhysSize();

		if (segStart <= physicalAddress && physicalAddress < segEnd)
		{
			// find section
			for (size_t l = 0; l < seg->getSectionCount(); l++)
			{
				ElfSection* sect = seg->getSection(l);
				int64_t sectStart = segStart+sect->getOffset();
				int64_t sectEnd = sectStart+sect->getSize();

				if (sectStart <= physicalAddress && physicalAddress < sectEnd)
				{
					segment = (int) i;
					section = (int) l;
					sectionOffset = physicalAddress-sectStart;
					return true;
				}
			}

			Logger::queueError(Logger::Error,L"Found segment, but no containing section");
			return false;
		}
	}

	// search in segmentless sections
	for (size_t i = 0; i < elf.getSegmentlessSectionCount(); i++)
	{
		ElfSection* sect = elf.getSegmentlessSection(i);
		int64_t sectStart = sect->getOffset();
		int64_t sectEnd = sectStart+sect->getSize();

		if (sectStart <= physicalAddress && physicalAddress < sectEnd)
		{
			segment = -1;
			section = (int) i;
			sectionOffset = physicalAddress-sectStart;
			return true;
		}
	}

	segment = -1;
	section = -1;
	Logger::queueError(Logger::Error,L"Couldn't find a section");
	return false;
}

bool MipsElfFile::getModuleInfo(SymDataModuleInfo& info)
{
	info.crc32 = getCrc32(elf.getFileData().data(),elf.getFileData().size());
	return true;
}

bool MipsElfFile::write(void* data, size_t length)
{
	if (segment != -1)
	{
		ElfSegment* seg = elf.getSegment(segment);
		ElfSection* sect = seg->getSection(section);

		int64_t pos = sect->getOffset()+sectionOffset;
		seg->writeToData(pos,data,length);
		sectionOffset += length;
		return true;
	}

	if (section != -1)
	{
		// TODO: segmentless sections
		return false;
	}

	Logger::printError(Logger::Error,L"Not inside a section");
	return false;
}

bool MipsElfFile::load(const fs::path& fileName, const fs::path& outputFileName)
{
	this->outputFileName = outputFileName;

	if (!elf.load(fileName,true))
	{
		Logger::printError(Logger::FatalError,L"Failed to load %s",fileName.wstring());
		return false;
	}

	if (elf.getType() == 0xFFA0)
	{
		Logger::printError(Logger::FatalError,L"Relocatable ELF %s not supported yet",fileName.wstring());
		return false;
	}

	if (elf.getType() != 2)
	{
		Logger::printError(Logger::FatalError,L"Unknown ELF %s type %d",fileName,elf.getType());
		return false;
	}

	if (elf.getSegmentCount() != 0)
		seekVirtual(elf.getSegment(0)->getVirtualAddress());

	return true;
}

bool MipsElfFile::setSection(const std::wstring& name)
{
	std::string utf8Name = convertWStringToUtf8(name);

	// look in segments
	for (size_t i = 0; i < elf.getSegmentCount(); i++)
	{
		ElfSegment* seg = elf.getSegment(i);
		int n = seg->findSection(utf8Name);
		if (n != -1)
		{
			segment = (int) i;
			section = n;
			return true;
		}
	}

	// look in stray sections
	int n = elf.findSegmentlessSection(utf8Name);
	if (n != -1)
	{
		segment = -1;
		section = n;
		return true;
	}

	Logger::queueError(Logger::Warning,L"Section %s not found",name);
	return false;
}

void MipsElfFile::save()
{
	elf.save(outputFileName);
}

//
// DirectiveLoadPspElf
//

DirectiveLoadMipsElf::DirectiveLoadMipsElf(const fs::path& fileName)
{
	file = std::make_shared<MipsElfFile>();

	this->inputName = getFullPathName(fileName);
	if (!file->load(this->inputName,this->inputName))
	{
		file = nullptr;
		return;
	}

	g_fileManager->addFile(file);
}

DirectiveLoadMipsElf::DirectiveLoadMipsElf(const fs::path& inputName, const fs::path& outputName)
{
	file = std::make_shared<MipsElfFile>();

	this->inputName = getFullPathName(inputName);
	this->outputName = getFullPathName(outputName);
	if (!file->load(this->inputName,this->outputName))
	{
		file = nullptr;
		return;
	}

	g_fileManager->addFile(file);
}

bool DirectiveLoadMipsElf::Validate(const ValidateState &state)
{
	Arch->NextSection();
	g_fileManager->openFile(file,true);
	return false;
}

void DirectiveLoadMipsElf::Encode() const
{
	g_fileManager->openFile(file,false);
}

void DirectiveLoadMipsElf::writeTempData(TempData& tempData) const
{
	if (outputName.empty())
	{
		tempData.writeLine(g_fileManager->getVirtualAddress(),tfm::format(L".loadelf \"%s\"",inputName.wstring()));
	} else {
		tempData.writeLine(g_fileManager->getVirtualAddress(),tfm::format(L".loadelf \"%s\",\"%s\"",
			inputName.wstring(),outputName.wstring()));
	}
}

void DirectiveLoadMipsElf::writeSymData(SymbolData& symData) const
{
	file->beginSymData(symData);
}

// file: Commands/CommandSequence.h


#include <memory>
#include <vector>

class Label;

class CommandSequence: public CAssemblerCommand
{
public:
	CommandSequence();
	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
	void writeSymData(SymbolData& symData) const override;
	void addCommand(std::unique_ptr<CAssemblerCommand> cmd) { commands.push_back(std::move(cmd)); }
private:
	std::vector<std::unique_ptr<CAssemblerCommand>> commands;
};

// file: Parser/Parser.h


#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

class CAssemblerCommand;
class Expression;
class TextFile;

struct DirectiveEntry;

using DirectiveMap = std::unordered_multimap<std::wstring, const DirectiveEntry>;

struct AssemblyTemplateArgument
{
	const wchar_t* variableName;
	std::wstring value;
};

struct ParserMacro
{
	std::wstring name;
	std::vector<std::wstring> parameters;
	std::set<std::wstring> labels;
	std::vector<Token> content;
	size_t counter;
};

enum class ConditionalResult { Unknown, True, False };

class Parser
{
public:
	Parser();
	bool atEnd() { return entries.back().tokenizer->atEnd(); }

	void addEquation(const Token& start, const std::wstring& name, const std::wstring& value);

	Expression parseExpression();
	bool parseExpressionList(std::vector<Expression>& list, int min = -1, int max = -1);
	bool parseIdentifier(std::wstring& dest);
	std::unique_ptr<CAssemblerCommand> parseCommand();
	std::unique_ptr<CAssemblerCommand> parseCommandSequence(wchar_t indicator = 0, const std::initializer_list<const wchar_t*> terminators = {});
	std::unique_ptr<CAssemblerCommand> parseFile(TextFile& file, bool virtualFile = false);
	std::unique_ptr<CAssemblerCommand> parseString(const std::wstring& text);
	std::unique_ptr<CAssemblerCommand> parseTemplate(const std::wstring& text, const std::initializer_list<AssemblyTemplateArgument> variables = {});
	std::unique_ptr<CAssemblerCommand> parseDirective(const DirectiveMap &directiveSet);
	bool matchToken(TokenType type, bool optional = false);

	Tokenizer* getTokenizer() { return entries.back().tokenizer; };
	const Token& peekToken(int ahead = 0) { return getTokenizer()->peekToken(ahead); };
	const Token& nextToken() { return getTokenizer()->nextToken(); };
	void eatToken() { getTokenizer()->eatToken(); };
	void eatTokens(int num) { getTokenizer()->eatTokens(num); };

	void pushConditionalResult(ConditionalResult cond);
	void popConditionalResult() { conditionStack.pop_back(); };
	bool isInsideTrueBlock() { return conditionStack.back().inTrueBlock; }
	bool isInsideUnknownBlock() { return conditionStack.back().inUnknownBlock; }

	void printError(const Token &token, const std::wstring &text);

	template <typename... Args>
	void printError(const Token& token, const wchar_t* text, const Args&... args)
	{
		printError(token, tfm::format(text,args...));
	}

	bool hasError() { return error; }
	void updateFileInfo();
protected:
	void clearError() { error = false; }
	std::unique_ptr<CAssemblerCommand> handleError();

	std::unique_ptr<CAssemblerCommand> parse(Tokenizer* tokenizer, bool virtualFile, const fs::path& name = {});
	std::unique_ptr<CAssemblerCommand> parseLabel();
	bool checkEquLabel();
	bool parseFunctionDeclaration(std::wstring& name, std::vector<std::wstring>& parameters);
	bool checkExpFuncDefinition();
	bool checkMacroDefinition();

	std::optional<std::vector<Token>> extractMacroParameter(const Token &macroStart);
	std::unique_ptr<CAssemblerCommand> parseMacroCall();

	struct FileEntry
	{
		Tokenizer* tokenizer;
		bool virtualFile;
		int fileNum;
		int previousCommandLine;
	};

	std::vector<FileEntry> entries;
	std::map<std::wstring,ParserMacro> macros;
	std::set<std::wstring> macroLabels;
	bool initializingMacro;
	bool error;
	size_t errorLine;

	bool overrideFileInfo;
	int overrideFileNum;
	int overrideLineNum;

	struct ConditionInfo
	{
		bool inTrueBlock;
		bool inUnknownBlock;
	};

	std::vector<ConditionInfo> conditionStack;
};

struct TokenSequenceValue
{
	TokenSequenceValue(const wchar_t* text)
	{
		type = TokenType::Identifier;
		textValue = text;
	}

	TokenSequenceValue(int64_t num)
	{
		type = TokenType::Integer;
		intValue = num;
	}

	TokenSequenceValue(double num)
	{
		type = TokenType::Float;
		floatValue = num;
	}


	TokenType type;
	union
	{
		const wchar_t* textValue;
		int64_t intValue;
		double floatValue;
	};
};

using TokenSequence = std::initializer_list<TokenType>;
using TokenValueSequence = std::initializer_list<TokenSequenceValue>;

class TokenSequenceParser
{
public:
	void addEntry(int result, TokenSequence tokens, TokenValueSequence values);
	bool parse(Parser& parser, int& result);
	size_t getEntryCount() { return entries.size(); }
private:
	struct Entry
	{
		std::vector<TokenType> tokens;
		std::vector<TokenSequenceValue> values;
		int result;
	};

	std::vector<Entry> entries;
};

// file: Archs/MIPS/MipsElfRelocator.cpp


int MipsElfRelocator::expectedMachine() const
{
	return EM_MIPS;
}

bool MipsElfRelocator::processHi16Entries(uint32_t lo16Opcode, int64_t lo16RelocationBase, std::vector<RelocationAction>& actions, std::vector<std::wstring>& errors)
{
	bool result = true;

	for (const Hi16Entry &hi16: hi16Entries)
	{
		if (hi16.relocationBase != lo16RelocationBase)
		{
			errors.push_back(tfm::format(L"Mismatched R_MIPS_HI16 with	R_MIPS_LO16 of a different symbol"));
			result = false;
			continue;
		}

		int32_t addend = (int32_t)((hi16.opcode & 0xFFFF) << 16) + (int16_t)(lo16Opcode & 0xFFFF);
		int64_t fullPosition = addend + hi16.relocationBase;
		uint32_t opcode = (hi16.opcode & 0xffff0000) | (((fullPosition >> 16) + ((fullPosition & 0x8000) != 0)) & 0xFFFF);
		actions.emplace_back(hi16.offset, opcode);
	}

	hi16Entries.clear();
	return result;
}

bool MipsElfRelocator::relocateOpcode(int type, const RelocationData& data, std::vector<RelocationAction>& actions, std::vector<std::wstring>& errors)
{
	unsigned int op = data.opcode;
	bool result = true;

	switch (type)
	{
	case R_MIPS_26: //j, jal
		op = (op & 0xFC000000) | (((op&0x03FFFFFF)+(data.relocationBase>>2))&0x03FFFFFF);
		break;
	case R_MIPS_32:
		op += (int) data.relocationBase;
		break;
	case R_MIPS_HI16:
		hi16Entries.emplace_back(data.opcodeOffset, data.relocationBase, data.opcode);
		break;
	case R_MIPS_LO16:
		if (!processHi16Entries(op, data.relocationBase, actions, errors))
			result = false;
		op = (op&0xffff0000) | (((op&0xffff)+data.relocationBase)&0xffff);
		break;
	default:
		errors.emplace_back(tfm::format(L"Unknown MIPS relocation type %d",type));
		return false;
	}

	actions.emplace_back(data.opcodeOffset, op);
	return result;
}

bool MipsElfRelocator::finish(std::vector<RelocationAction>& actions, std::vector<std::wstring>& errors)
{
	// This shouldn't happen. If it does, relocate as if there was no lo16 opcode
	if (!hi16Entries.empty())
		return processHi16Entries(0, hi16Entries.front().relocationBase, actions, errors);
	return true;
}

void MipsElfRelocator::setSymbolAddress(RelocationData& data, int64_t symbolAddress, int symbolType)
{
	data.symbolAddress = symbolAddress;
	data.targetSymbolType = symbolType;
}

const wchar_t* mipsCtorTemplate = LR"(
	addiu	sp,-32
	sw		ra,0(sp)
	sw		s0,4(sp)
	sw		s1,8(sp)
	sw		s2,12(sp)
	sw		s3,16(sp)
	li		s0,%ctorTable%
	li		s1,%ctorTable%+%ctorTableSize%
	%outerLoopLabel%:
	lw		s2,(s0)
	lw		s3,4(s0)
	addiu	s0,8
	%innerLoopLabel%:
	lw		a0,(s2)
	jalr	a0
	addiu	s2,4h
	bne		s2,s3,%innerLoopLabel%
	nop
	bne		s0,s1,%outerLoopLabel%
	nop
	lw		ra,0(sp)
	lw		s0,4(sp)
	lw		s1,8(sp)
	lw		s2,12(sp)
	lw		s3,16(sp)
	jr		ra
	addiu	sp,32
	%ctorTable%:
	.word	%ctorContent%
)";

std::unique_ptr<CAssemblerCommand> MipsElfRelocator::generateCtorStub(std::vector<ElfRelocatorCtor>& ctors)
{
	Parser parser;
	if (ctors.size() != 0)
	{
		// create constructor table
		std::wstring table;
		for (size_t i = 0; i < ctors.size(); i++)
		{
			if (i != 0)
				table += ',';
			table += tfm::format(L"%s,%s+0x%08X",ctors[i].symbolName,ctors[i].symbolName,ctors[i].size);
		}

		return parser.parseTemplate(mipsCtorTemplate,{
			{ L"%ctorTable%",		Global.symbolTable.getUniqueLabelName() },
			{ L"%ctorTableSize%",	tfm::format(L"%d",ctors.size()*8) },
			{ L"%outerLoopLabel%",	Global.symbolTable.getUniqueLabelName() },
			{ L"%innerLoopLabel%",	Global.symbolTable.getUniqueLabelName() },
			{ L"%ctorContent%",		table },
		});
	} else {
		return parser.parseTemplate(L"jr ra :: nop");
	}
}

// file: Archs/MIPS/MipsExpressionFunctions.cpp


#define GET_PARAM(params,index,dest) \
	if (getExpFuncParameter(params,index,dest,funcName,false) == false) \
		return ExpressionValue();

ExpressionValue expFuncHi(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	int64_t value;

	GET_PARAM(parameters,0,value);

	return ExpressionValue((int64_t)((value >> 16) + ((value & 0x8000) != 0)) & 0xFFFF);
}

ExpressionValue expFuncLo(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	int64_t value;

	GET_PARAM(parameters,0,value);

	return ExpressionValue((int64_t)(int16_t)(value & 0xFFFF));
}

const ExpressionFunctionMap mipsExpressionFunctions = {
	{ L"lo",			{ &expFuncLo,				1,	1,	ExpFuncSafety::Safe } },
	{ L"hi",			{ &expFuncHi,				1,	1,	ExpFuncSafety::Safe } },
};

// file: Archs/MIPS/MipsMacros.cpp


MipsMacroCommand::MipsMacroCommand(std::unique_ptr<CAssemblerCommand> content, int macroFlags)
{
	this->content = std::move(content);
	this->macroFlags = macroFlags;
	IgnoreLoadDelay = Mips.GetIgnoreDelay();
}

bool MipsMacroCommand::Validate(const ValidateState &state)
{
	int64_t memoryPos = g_fileManager->getVirtualAddress();
	content->applyFileInfo();
	bool result = content->Validate(state);
	int64_t newMemoryPos = g_fileManager->getVirtualAddress();

	applyFileInfo();

	if (!IgnoreLoadDelay && Mips.GetDelaySlot() && (newMemoryPos-memoryPos) > 4
		&& (macroFlags & MIPSM_DONTWARNDELAYSLOT) == 0)
	{
		Logger::queueError(Logger::Warning,L"Macro with multiple opcodes used inside a delay slot");
	}

	if (newMemoryPos == memoryPos)
		Logger::queueError(Logger::Warning,L"Empty macro content");

	return result;
}

void MipsMacroCommand::Encode() const
{
	content->Encode();
}

void MipsMacroCommand::writeTempData(TempData& tempData) const
{
	content->applyFileInfo();
	content->writeTempData(tempData);
}

std::wstring preprocessMacro(const wchar_t* text, MipsImmediateData& immediates)
{
	// A macro is turned into a sequence of opcodes that are parsed seperately.
	// Any expressions used in the macro may be evaluated at a different memory
	// position, so the '.' operator needs to be replaced by a label at the start
	// of the macro
	std::wstring labelName = Global.symbolTable.getUniqueLabelName(true);
	immediates.primary.expression.replaceMemoryPos(labelName);
	immediates.secondary.expression.replaceMemoryPos(labelName);

	return tfm::format(L"%s: %s",labelName,text);
}

std::unique_ptr<CAssemblerCommand> createMacro(Parser& parser, const std::wstring& text, int flags, std::initializer_list<AssemblyTemplateArgument> variables)
{
	std::unique_ptr<CAssemblerCommand> content = parser.parseTemplate(text,variables);
	return std::make_unique<MipsMacroCommand>(std::move(content),flags);
}

std::unique_ptr<CAssemblerCommand> generateMipsMacroAbs(Parser& parser, MipsRegisterData& registers, MipsImmediateData& immediates, int flags)
{
	const wchar_t* templateAbs = LR"(
		%sraop% 	r1,%rs%,31
		xor 		%rd%,%rs%,r1
		%subop% 	%rd%,%rd%,r1
	)";

	std::wstring sraop, subop;

	switch (flags & MIPSM_ACCESSMASK)
	{
	case MIPSM_W:	sraop = L"sra"; subop = L"subu"; break;
	case MIPSM_DW:	sraop = L"dsra32"; subop = L"dsubu"; break;
	default: return nullptr;
	}

	std::wstring macroText = preprocessMacro(templateAbs,immediates);
	return createMacro(parser,macroText,flags, {
			{ L"%rd%",		registers.grd.name },
			{ L"%rs%",		registers.grs.name },
			{ L"%sraop%",	sraop },
			{ L"%subop%",	subop },
	});
}

std::unique_ptr<CAssemblerCommand> generateMipsMacroLiFloat(Parser& parser, MipsRegisterData& registers, MipsImmediateData& immediates, int flags)
{
	const wchar_t* templateLiFloat = LR"(
		li 		r1,float(%imm%)
		mtc1	r1,%rs%
	)";

	std::wstring sraop, subop;

	std::wstring macroText = preprocessMacro(templateLiFloat,immediates);
	return createMacro(parser,macroText,flags, {
			{ L"%imm%",		immediates.secondary.expression.toString() },
			{ L"%rs%",		registers.frs.name },
	});
}

std::unique_ptr<CAssemblerCommand> generateMipsMacroLi(Parser& parser, MipsRegisterData& registers, MipsImmediateData& immediates, int flags)
{
	const wchar_t* templateLi = LR"(
		.if abs(%imm%) > 0xFFFFFFFF
			.error "Immediate value too big"
		.elseif %imm% & ~0xFFFF
			.if (%imm% & 0xFFFF8000) == 0xFFFF8000
				.if %lower%
					addiu	%rs%,r0, lo(%imm%)
				.endif
			.elseif (%imm% & 0xFFFF) == 0
				.if %upper%
					lui		%rs%, hi(%imm%)
				.elseif %lower%
					nop
				.endif
			.else
				.if %upper%
					lui		%rs%, hi(%imm%)
				.endif
				.if %lower%
					addiu 	%rs%, lo(%imm%)
				.endif
			.endif
		.else
			.if %lower%
				ori		%rs%,r0,%imm%
			.endif
		.endif
	)";

	// floats need to be treated as integers, convert them
	if (immediates.secondary.expression.isConstExpression())
	{
		ExpressionValue value = immediates.secondary.expression.evaluate();
		if (value.isFloat())
		{
			int32_t newValue = getFloatBits((float)value.floatValue);
			immediates.secondary.expression = createConstExpression(newValue);
		}
	}

	std::wstring macroText = preprocessMacro(templateLi,immediates);
	return createMacro(parser,macroText,flags, {
			{ L"%upper%",	(flags & MIPSM_UPPER) ? L"1" : L"0" },
			{ L"%lower%",	(flags & MIPSM_LOWER) ? L"1" : L"0" },
			{ L"%rs%",		registers.grs.name },
			{ L"%imm%",		immediates.secondary.expression.toString() },
	});
}

std::unique_ptr<CAssemblerCommand> generateMipsMacroLoadStore(Parser& parser, MipsRegisterData& registers, MipsImmediateData& immediates, int flags)
{
	const wchar_t* templateLoadStore = LR"(
		.if %imm% & ~0xFFFFFFFF
			.error "Address too big"
		.elseif %imm% < 0x8000 || (%imm% & 0xFFFF8000) == 0xFFFF8000
			.if %lower%
				%op%	%rs%, lo(%imm%)(r0)
			.elseif %upper%
				nop
			.endif
		.else
			.if %upper%
				lui		%temp%, hi(%imm%)
			.endif
			.if %lower%
				%op%	%rs%, lo(%imm%)(%temp%)
			.endif
		.endif
	)";

	const wchar_t* op;
	bool isCop = false;
	switch (flags & (MIPSM_ACCESSMASK|MIPSM_LOAD|MIPSM_STORE))
	{
	case MIPSM_LOAD|MIPSM_B:		op = L"lb"; break;
	case MIPSM_LOAD|MIPSM_BU:		op = L"lbu"; break;
	case MIPSM_LOAD|MIPSM_HW:		op = L"lh"; break;
	case MIPSM_LOAD|MIPSM_HWU:		op = L"lhu"; break;
	case MIPSM_LOAD|MIPSM_W:		op = L"lw"; break;
	case MIPSM_LOAD|MIPSM_WU:		op = L"lwu"; break;
	case MIPSM_LOAD|MIPSM_DW:		op = L"ld"; break;
	case MIPSM_LOAD|MIPSM_LLSCW:	op = L"ll"; break;
	case MIPSM_LOAD|MIPSM_LLSCDW:	op = L"lld"; break;
	case MIPSM_LOAD|MIPSM_COP1:		op = L"lwc1"; isCop = true; break;
	case MIPSM_LOAD|MIPSM_COP2:		op = L"lwc2"; isCop = true; break;
	case MIPSM_LOAD|MIPSM_DCOP1:	op = L"ldc1"; isCop = true; break;
	case MIPSM_LOAD|MIPSM_DCOP2:	op = L"ldc2"; isCop = true; break;
	case MIPSM_STORE|MIPSM_B:		op = L"sb"; break;
	case MIPSM_STORE|MIPSM_HW:		op = L"sh"; break;
	case MIPSM_STORE|MIPSM_W:		op = L"sw"; break;
	case MIPSM_STORE|MIPSM_DW:		op = L"sd"; break;
	case MIPSM_STORE|MIPSM_LLSCW:	op = L"sc"; break;
	case MIPSM_STORE|MIPSM_LLSCDW:	op = L"scd"; break;
	case MIPSM_STORE|MIPSM_COP1:	op = L"swc1"; isCop = true; break;
	case MIPSM_STORE|MIPSM_COP2:	op = L"swc2"; isCop = true; break;
	case MIPSM_STORE|MIPSM_DCOP1:	op = L"sdc1"; isCop = true; break;
	case MIPSM_STORE|MIPSM_DCOP2:	op = L"sdc2"; isCop = true; break;
	default: return nullptr;
	}

	std::wstring macroText = preprocessMacro(templateLoadStore,immediates);

	bool store = (flags & MIPSM_STORE) != 0;
	return createMacro(parser,macroText,flags, {
			{ L"%upper%",	(flags & MIPSM_UPPER) ? L"1" : L"0" },
			{ L"%lower%",	(flags & MIPSM_LOWER) ? L"1" : L"0" },
			{ L"%rs%",		isCop ? registers.frs.name : registers.grs.name },
			{ L"%temp%",	isCop || store ? L"r1" : registers.grs.name },
			{ L"%imm%",		immediates.secondary.expression.toString() },
			{ L"%op%",		op },
	});
}

std::unique_ptr<CAssemblerCommand> generateMipsMacroLoadUnaligned(Parser& parser, MipsRegisterData& registers, MipsImmediateData& immediates, int flags)
{
	const wchar_t* selectedTemplate;

	std::wstring op, size;
	int type = flags & MIPSM_ACCESSMASK;
	if (type == MIPSM_HW || type == MIPSM_HWU)
	{
		const wchar_t* templateHalfword = LR"(
			.if (%off% < 0x8000) && ((%off%+1) >= 0x8000)
				.error "Immediate offset too big"
			.else
				%op%	r1,%off%+1(%rs%)
				%op%	%rd%,%off%(%rs%)
				sll		r1,8
				or		%rd%,r1
			.endif
		)";

		op = type == MIPSM_HWU ? L"lbu" : L"lb";
		selectedTemplate = templateHalfword;
	} else if (type == MIPSM_W || type == MIPSM_DW)
	{
		const wchar_t* templateWord = LR"(
			.if (%off% < 0x8000) && ((%off%+%size%-1) >= 0x8000)
				.error "Immediate offset too big"
			.else
				%op%l	%rd%,%off%+%size%-1(%rs%)
				%op%r	%rd%,%off%(%rs%)
			.endif
		)";

		if (registers.grs.num == registers.grd.num)
		{
			Logger::printError(Logger::Error,L"Cannot use same register as source and destination");
			return std::make_unique<DummyCommand>();
		}

		op = type == MIPSM_W ? L"lw" : L"ld";
		size = type == MIPSM_W ? L"4" : L"8";
		selectedTemplate = templateWord;
	} else {
		return nullptr;
	}

	std::wstring macroText = preprocessMacro(selectedTemplate,immediates);
	return createMacro(parser,macroText,flags, {
			{ L"%rs%",		registers.grs.name },
			{ L"%rd%",		registers.grd.name },
			{ L"%off%",		immediates.primary.expression.toString() },
			{ L"%op%",		op },
			{ L"%size%",    size },
	});
}

std::unique_ptr<CAssemblerCommand> generateMipsMacroStoreUnaligned(Parser& parser, MipsRegisterData& registers, MipsImmediateData& immediates, int flags)
{
	const wchar_t* selectedTemplate;

	std::wstring op, size;
	int type = flags & MIPSM_ACCESSMASK;
	if (type == MIPSM_HW)
	{
		const wchar_t* templateHalfword = LR"(
			.if (%off% < 0x8000) && ((%off%+1) >= 0x8000)
				.error "Immediate offset too big"
			.else
				sb		%rd%,%off%(%rs%)
				srl		r1,%rd%,8
				sb		r1,%off%+1(%rs%)
			.endif
		)";

		selectedTemplate = templateHalfword;
	} else if (type == MIPSM_W || type == MIPSM_DW)
	{
		const wchar_t* templateWord = LR"(
			.if (%off% < 0x8000) && ((%off%+%size%-1) >= 0x8000)
				.error "Immediate offset too big"
			.else
				%op%l	%rd%,%off%+%size%-1(%rs%)
				%op%r	%rd%,%off%(%rs%)
			.endif
		)";

		if (registers.grs.num == registers.grd.num)
		{
			Logger::printError(Logger::Error,L"Cannot use same register as source and destination");
			return std::make_unique<DummyCommand>();
		}

		op = type == MIPSM_W ? L"sw" : L"sd";
		size = type == MIPSM_W ? L"4" : L"8";
		selectedTemplate = templateWord;
	} else {
		return nullptr;
	}

	std::wstring macroText = preprocessMacro(selectedTemplate,immediates);
	return createMacro(parser,macroText,flags, {
			{ L"%rs%",		registers.grs.name },
			{ L"%rd%",		registers.grd.name },
			{ L"%off%",		immediates.primary.expression.toString() },
			{ L"%op%",		op },
			{ L"%size%",	size },
	});
}

std::unique_ptr<CAssemblerCommand> generateMipsMacroBranch(Parser& parser, MipsRegisterData& registers, MipsImmediateData& immediates, int flags)
{
	const wchar_t* selectedTemplate;

	int type = flags & MIPSM_CONDITIONMASK;

	bool bne = type == MIPSM_NE;
	bool beq = type == MIPSM_EQ;
	bool beqz = type == MIPSM_GE || type == MIPSM_GEU;
	bool bnez = type == MIPSM_LT || type == MIPSM_LTU;
	bool unsigned_ = type == MIPSM_GEU || type == MIPSM_LTU;
	bool immediate = (flags & MIPSM_IMM) != 0;
	bool likely = (flags & MIPSM_LIKELY) != 0;
	bool revcmp = (flags & MIPSM_REVCMP) != 0;

	std::wstring op;
	if (bne || beq)
	{
		const wchar_t* templateNeEq = LR"(
			.if %imm% == 0
				%op%	%rs%,r0,%dest%
			.else
				li		r1,%imm%
				%op%	%rs%,r1,%dest%
			.endif
		)";

		selectedTemplate = templateNeEq;
		if(likely)
			op = bne ? L"bnel" : L"beql";
		else
			op = bne ? L"bne" : L"beq";
	} else if (immediate && (beqz || bnez))
	{
		const wchar_t* templateImmediate = LR"(
			.if %revcmp% && %imm% == 0
				slt%u% 	r1,r0,%rs%
			.elseif %revcmp%
				li		r1,%imm%
				slt%u%	r1,r1,%rs%
			.elseif (%imm% < -0x8000) || (%imm% >= 0x8000)
				li		r1,%imm%
				slt%u%	r1,%rs%,r1
			.else
				slti%u%	r1,%rs%,%imm%
			.endif
			%op%	r1,%dest%
		)";

		selectedTemplate = templateImmediate;
		if(likely)
			op = bnez ? L"bnezl" : L"beqzl";
		else
			op = bnez ? L"bnez" : L"beqz";
	} else if (beqz || bnez)
	{
		const wchar_t* templateRegister = LR"(
			.if %revcmp%
				slt%u%	r1,%rt%,%rs%
			.else
				slt%u%	r1,%rs%,%rt%
			.endif
			%op%	r1,%dest%
		)";

		selectedTemplate = templateRegister;
		if(likely)
			op = bnez ? L"bnezl" : L"beqzl";
		else
			op = bnez ? L"bnez" : L"beqz";
	} else {
		return nullptr;
	}

	std::wstring macroText = preprocessMacro(selectedTemplate,immediates);
	return createMacro(parser,macroText,flags, {
			{ L"%op%",		op },
			{ L"%u%",		unsigned_ ? L"u" : L" "},
			{ L"%revcmp%",	revcmp ? L"1" : L"0"},
			{ L"%rs%",		registers.grs.name },
			{ L"%rt%",		registers.grt.name },
			{ L"%imm%",		immediates.primary.expression.toString() },
			{ L"%dest%",	immediates.secondary.expression.toString() },
	});
}

std::unique_ptr<CAssemblerCommand> generateMipsMacroSet(Parser& parser, MipsRegisterData& registers, MipsImmediateData& immediates, int flags)
{
	const wchar_t* selectedTemplate;

	int type = flags & MIPSM_CONDITIONMASK;

	bool ne = type == MIPSM_NE;
	bool eq = type == MIPSM_EQ;
	bool ge = type == MIPSM_GE || type == MIPSM_GEU;
	bool lt = type == MIPSM_LT || type == MIPSM_LTU;
	bool unsigned_ = type == MIPSM_GEU || type == MIPSM_LTU;
	bool immediate = (flags & MIPSM_IMM) != 0;
	bool revcmp = (flags & MIPSM_REVCMP) != 0;

	if (immediate && (ne || eq))
	{
		const wchar_t* templateImmediateEqNe = LR"(
			.if %imm% & ~0xFFFF
				li		%rd%,%imm%
				xor		%rd%,%rs%,%rd%
			.else
				xori	%rd%,%rs%,%imm%
			.endif
			.if %eq%
				sltiu	%rd%,%rd%,1
			.else
				sltu	%rd%,r0,%rd%
			.endif
		)";

		selectedTemplate = templateImmediateEqNe;
	} else if (ne || eq)
	{
		const wchar_t* templateEqNe = LR"(
			xor		%rd%,%rs%,%rt%
			.if %eq%
				sltiu	%rd%,%rd%,1
			.else
				sltu	%rd%,r0,%rd%
			.endif
		)";

		selectedTemplate = templateEqNe;
	} else if (immediate && (ge || lt))
	{
		const wchar_t* templateImmediateGeLt = LR"(
			.if %revcmp% && %imm% == 0
				slt%u%	%rd%,r0,%rs%
			.elseif %revcmp%
				li		%rd%,%imm%
				slt%u%	%rd%,%rd%,%rs%
			.elseif (%imm% < -0x8000) || (%imm% >= 0x8000)
				li		%rd%,%imm%
				slt%u%	%rd%,%rs%,%rd%
			.else
				slti%u%	%rd%,%rs%,%imm%
			.endif
			.if %ge%
				xori	%rd%,%rd%,1
			.endif
		)";

		selectedTemplate = templateImmediateGeLt;
	} else if (ge)
	{
		const wchar_t* templateGe = LR"(
			.if %revcmp%
				slt%u%	%rd%,%rt%,%rs%
			.else
				slt%u%	%rd%,%rs%,%rt%
			.endif
			xori	%rd%,%rd%,1
		)";

		selectedTemplate = templateGe;
	} else
	{
		return nullptr;
	}

	std::wstring macroText = preprocessMacro(selectedTemplate,immediates);
	return createMacro(parser,macroText,flags, {
			{ L"%u%",		unsigned_ ? L"u" : L" "},
			{ L"%eq%",		eq ? L"1" : L"0" },
			{ L"%ge%",		ge ? L"1" : L"0" },
			{ L"%revcmp%",	revcmp ? L"1" : L"0" },
			{ L"%rd%",		registers.grd.name },
			{ L"%rs%",		registers.grs.name },
			{ L"%rt%",		registers.grt.name },
			{ L"%imm%",		immediates.secondary.expression.toString() },
	});
}

std::unique_ptr<CAssemblerCommand> generateMipsMacroRotate(Parser& parser, MipsRegisterData& registers, MipsImmediateData& immediates, int flags)
{
	bool left = (flags & MIPSM_LEFT) != 0;
	bool immediate = (flags & MIPSM_IMM) != 0;
	bool psp = Mips.GetVersion() == MARCH_PSP;

	const wchar_t* selectedTemplate;
	if (psp && immediate)
	{
		const wchar_t* templatePspImmediate = LR"(
			.if %amount% != 0
				.if %left%
					rotr	%rd%,%rs%,-%amount%&31
				.else
					rotr	%rd%,%rs%,%amount%
				.endif
			.else
				move	%rd%,%rs%
			.endif
		)";

		selectedTemplate = templatePspImmediate;
	} else if (psp)
	{
		const wchar_t* templatePspRegister = LR"(
			.if %left%
				negu	r1,%rt%
				rotrv	%rd%,%rs%,r1
			.else
				rotrv	%rd%,%rs%,%rt%
			.endif
		)";

		selectedTemplate = templatePspRegister;
	} else if (immediate)
	{
		const wchar_t* templateImmediate = LR"(
			.if %amount% != 0
				.if %left%
					srl	r1,%rs%,-%amount%&31
					sll	%rd%,%rs%,%amount%
				.else
					sll	r1,%rs%,-%amount%&31
					srl	%rd%,%rs%,%amount%
				.endif
				or		%rd%,%rd%,r1
			.else
				move	%rd%,%rs%
			.endif
		)";

		selectedTemplate = templateImmediate;
	} else {
		const wchar_t* templateRegister = LR"(
			negu	r1,%rt%
			.if %left%
				srlv	r1,%rs%,r1
				sllv	%rd%,%rs%,%rt%
			.else
				sllv	r1,%rs%,r1
				srlv	%rd%,%rs%,%rt%
			.endif
			or	%rd%,%rd%,r1
		)";

		selectedTemplate = templateRegister;
	}

	std::wstring macroText = preprocessMacro(selectedTemplate,immediates);
	return createMacro(parser,macroText,flags, {
			{ L"%left%",	left ? L"1" : L"0" },
			{ L"%rd%",		registers.grd.name },
			{ L"%rs%",		registers.grs.name },
			{ L"%rt%",		registers.grt.name },
			{ L"%amount%",	immediates.primary.expression.toString() },
	});
}

/* Placeholders
	i = i1 = 16 bit immediate
	I = i2 = 32 bit immediate
	s,t,d = registers */
const MipsMacroDefinition mipsMacros[] = {
	{ L"abs",	L"d,s",		&generateMipsMacroAbs,				MIPSM_W },
	{ L"dabs",	L"d,s",		&generateMipsMacroAbs,				MIPSM_DW },

	{ L"li",	L"s,I",		&generateMipsMacroLi,				MIPSM_IMM|MIPSM_UPPER|MIPSM_LOWER },
	{ L"li.u",	L"s,I",		&generateMipsMacroLi,				MIPSM_IMM|MIPSM_UPPER },
	{ L"li.l",	L"s,I",		&generateMipsMacroLi,				MIPSM_IMM|MIPSM_LOWER },
	{ L"la",	L"s,I",		&generateMipsMacroLi,				MIPSM_IMM|MIPSM_UPPER|MIPSM_LOWER },
	{ L"la.u",	L"s,I",		&generateMipsMacroLi,				MIPSM_IMM|MIPSM_UPPER },
	{ L"la.l",	L"s,I",		&generateMipsMacroLi,				MIPSM_IMM|MIPSM_LOWER },

	{ L"li.s",	L"S,I",		&generateMipsMacroLiFloat,			MIPSM_IMM },

	{ L"lb",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_B|MIPSM_UPPER|MIPSM_LOWER },
	{ L"lbu",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_BU|MIPSM_UPPER|MIPSM_LOWER },
	{ L"lh",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_HW|MIPSM_UPPER|MIPSM_LOWER },
	{ L"lhu",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_HWU|MIPSM_UPPER|MIPSM_LOWER },
	{ L"lw",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_W|MIPSM_UPPER|MIPSM_LOWER },
	{ L"lwu",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_WU|MIPSM_UPPER|MIPSM_LOWER },
	{ L"ld",    L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_DW|MIPSM_UPPER|MIPSM_LOWER },
	{ L"ll",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_LLSCW|MIPSM_UPPER|MIPSM_LOWER },
	{ L"lld",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_LLSCDW|MIPSM_UPPER|MIPSM_LOWER },
	{ L"lwc1",	L"S,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_COP1|MIPSM_UPPER|MIPSM_LOWER },
	{ L"l.s",	L"S,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_COP1|MIPSM_UPPER|MIPSM_LOWER },
	{ L"lwc2",	L"S,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_COP2|MIPSM_UPPER|MIPSM_LOWER },
	{ L"ldc1",	L"S,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_DCOP1|MIPSM_UPPER|MIPSM_LOWER },
	{ L"l.d",	L"S,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_DCOP1|MIPSM_UPPER|MIPSM_LOWER },
	{ L"ldc2",	L"S,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_DCOP2|MIPSM_UPPER|MIPSM_LOWER },

	{ L"lb.u",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_B|MIPSM_UPPER },
	{ L"lbu.u",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_BU|MIPSM_UPPER },
	{ L"lh.u",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_HW|MIPSM_UPPER },
	{ L"lhu.u",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_HWU|MIPSM_UPPER },
	{ L"lw.u",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_W|MIPSM_UPPER },
	{ L"lwu.u",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_WU|MIPSM_UPPER },
	{ L"ld.u",  L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_DW|MIPSM_UPPER },
	{ L"ll.u",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_LLSCW|MIPSM_UPPER },
	{ L"lld.u",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_LLSCDW|MIPSM_UPPER },
	{ L"lwc1.u",L"S,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_COP1|MIPSM_UPPER },
	{ L"l.s.u",	L"S,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_COP1|MIPSM_UPPER },
	{ L"lwc2.u",L"S,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_COP2|MIPSM_UPPER },
	{ L"ldc1.u",L"S,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_DCOP1|MIPSM_UPPER },
	{ L"l.d.u",	L"S,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_DCOP1|MIPSM_UPPER },
	{ L"ldc2.u",L"S,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_DCOP2|MIPSM_UPPER },

	{ L"lb.l",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_B|MIPSM_LOWER },
	{ L"lbu.l",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_BU|MIPSM_LOWER },
	{ L"lh.l",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_HW|MIPSM_LOWER },
	{ L"lhu.l",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_HWU|MIPSM_LOWER },
	{ L"lw.l",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_W|MIPSM_LOWER },
	{ L"lwu.l",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_WU|MIPSM_LOWER },
	{ L"ld.l",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_DW|MIPSM_LOWER },
	{ L"ll.l",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_LLSCW|MIPSM_LOWER },
	{ L"lld.l",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_LLSCDW|MIPSM_LOWER },
	{ L"lwc1.l",L"S,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_COP1|MIPSM_LOWER },
	{ L"l.s.l",	L"S,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_COP1|MIPSM_LOWER },
	{ L"lwc2.l",L"S,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_COP2|MIPSM_LOWER },
	{ L"ldc1.l",L"S,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_DCOP1|MIPSM_LOWER },
	{ L"l.d.l",	L"S,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_DCOP1|MIPSM_LOWER },
	{ L"ldc2.l",L"S,I",		&generateMipsMacroLoadStore,		MIPSM_LOAD|MIPSM_DCOP2|MIPSM_LOWER },

	{ L"ulh",	L"d,i(s)",	&generateMipsMacroLoadUnaligned,	MIPSM_HW|MIPSM_IMM },
	{ L"ulh",	L"d,(s)",	&generateMipsMacroLoadUnaligned,	MIPSM_HW },
	{ L"ulhu",	L"d,i(s)",	&generateMipsMacroLoadUnaligned,	MIPSM_HWU|MIPSM_IMM },
	{ L"ulhu",	L"d,(s)",	&generateMipsMacroLoadUnaligned,	MIPSM_HWU },
	{ L"ulw",	L"d,i(s)",	&generateMipsMacroLoadUnaligned,	MIPSM_W|MIPSM_IMM },
	{ L"ulw",	L"d,(s)",	&generateMipsMacroLoadUnaligned,	MIPSM_W },
	{ L"uld",	L"d,i(s)",	&generateMipsMacroLoadUnaligned,	MIPSM_DW|MIPSM_IMM },
	{ L"uld",	L"d,(s)",	&generateMipsMacroLoadUnaligned,	MIPSM_DW },

	{ L"sb",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_B|MIPSM_UPPER|MIPSM_LOWER },
	{ L"sh",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_HW|MIPSM_UPPER|MIPSM_LOWER },
	{ L"sw",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_W|MIPSM_UPPER|MIPSM_LOWER },
	{ L"sd",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_DW|MIPSM_UPPER|MIPSM_LOWER },
	{ L"sc",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_LLSCW|MIPSM_UPPER|MIPSM_LOWER },
	{ L"scd",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_LLSCDW|MIPSM_UPPER|MIPSM_LOWER },
	{ L"swc1",	L"S,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_COP1|MIPSM_UPPER|MIPSM_LOWER },
	{ L"s.s",	L"S,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_COP1|MIPSM_UPPER|MIPSM_LOWER },
	{ L"swc2",	L"S,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_COP2|MIPSM_UPPER|MIPSM_LOWER },
	{ L"sdc1",	L"S,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_DCOP1|MIPSM_UPPER|MIPSM_LOWER },
	{ L"s.d",	L"S,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_DCOP1|MIPSM_UPPER|MIPSM_LOWER },
	{ L"sdc2",	L"S,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_DCOP2|MIPSM_UPPER|MIPSM_LOWER },

	{ L"sb.u",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_B|MIPSM_UPPER },
	{ L"sh.u",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_HW|MIPSM_UPPER },
	{ L"sw.u",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_W|MIPSM_UPPER },
	{ L"sd.u",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_DW|MIPSM_UPPER },
	{ L"sc.u",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_LLSCW|MIPSM_UPPER },
	{ L"scd.u",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_LLSCDW|MIPSM_UPPER },
	{ L"swc1.u",L"S,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_COP1|MIPSM_UPPER },
	{ L"s.s.u",	L"S,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_COP1|MIPSM_UPPER },
	{ L"swc2.u",L"S,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_COP2|MIPSM_UPPER },
	{ L"sdc1.u",L"S,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_DCOP1|MIPSM_UPPER },
	{ L"s.d.u",	L"S,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_DCOP1|MIPSM_UPPER },
	{ L"sdc2.u",L"S,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_DCOP2|MIPSM_UPPER },

	{ L"sb.l",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_B|MIPSM_LOWER },
	{ L"sh.l",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_HW|MIPSM_LOWER },
	{ L"sw.l",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_W|MIPSM_LOWER },
	{ L"sd.l",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_DW|MIPSM_LOWER },
	{ L"sc.l",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_LLSCW|MIPSM_LOWER },
	{ L"scd.l",	L"s,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_LLSCDW|MIPSM_LOWER },
	{ L"swc1.l",L"S,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_COP1|MIPSM_LOWER },
	{ L"s.s.l",	L"S,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_COP1|MIPSM_LOWER },
	{ L"swc2.l",L"S,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_COP2|MIPSM_LOWER },
	{ L"sdc1.l",L"S,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_DCOP1|MIPSM_LOWER },
	{ L"s.d.l",	L"S,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_DCOP1|MIPSM_LOWER },
	{ L"sdc2.l",L"S,I",		&generateMipsMacroLoadStore,		MIPSM_STORE|MIPSM_DCOP2|MIPSM_LOWER },

	{ L"ush",	L"d,i(s)",	&generateMipsMacroStoreUnaligned,	MIPSM_HW|MIPSM_IMM },
	{ L"ush",	L"d,(s)",	&generateMipsMacroStoreUnaligned,	MIPSM_HW },
	{ L"usw",	L"d,i(s)",	&generateMipsMacroStoreUnaligned,	MIPSM_W|MIPSM_IMM },
	{ L"usw",	L"d,(s)",	&generateMipsMacroStoreUnaligned,	MIPSM_W },
	{ L"usd",	L"d,i(s)",	&generateMipsMacroStoreUnaligned,	MIPSM_DW|MIPSM_IMM },
	{ L"usd",	L"d,(s)",	&generateMipsMacroStoreUnaligned,	MIPSM_DW },

	{ L"blt",	L"s,t,I",	&generateMipsMacroBranch,			MIPSM_LT|MIPSM_DONTWARNDELAYSLOT },
	{ L"blt",	L"s,i,I",	&generateMipsMacroBranch,			MIPSM_LT|MIPSM_IMM|MIPSM_DONTWARNDELAYSLOT },
	{ L"bgt",	L"s,t,I",	&generateMipsMacroBranch,			MIPSM_LT|MIPSM_REVCMP|MIPSM_DONTWARNDELAYSLOT },
	{ L"bgt",	L"s,i,I",	&generateMipsMacroBranch,			MIPSM_LT|MIPSM_IMM|MIPSM_REVCMP|MIPSM_DONTWARNDELAYSLOT },
	{ L"bltu",	L"s,t,I",	&generateMipsMacroBranch,			MIPSM_LTU|MIPSM_DONTWARNDELAYSLOT },
	{ L"bltu",	L"s,i,I",	&generateMipsMacroBranch,			MIPSM_LTU|MIPSM_IMM|MIPSM_DONTWARNDELAYSLOT },
	{ L"bgtu",	L"s,t,I",	&generateMipsMacroBranch,			MIPSM_LTU|MIPSM_REVCMP|MIPSM_DONTWARNDELAYSLOT },
	{ L"bgtu",	L"s,i,I",	&generateMipsMacroBranch,			MIPSM_LTU|MIPSM_IMM|MIPSM_REVCMP|MIPSM_DONTWARNDELAYSLOT },
	{ L"bge",	L"s,t,I",	&generateMipsMacroBranch,			MIPSM_GE|MIPSM_DONTWARNDELAYSLOT },
	{ L"bge",	L"s,i,I",	&generateMipsMacroBranch,			MIPSM_GE|MIPSM_IMM|MIPSM_DONTWARNDELAYSLOT },
	{ L"ble",	L"s,t,I",	&generateMipsMacroBranch,			MIPSM_GE|MIPSM_REVCMP|MIPSM_DONTWARNDELAYSLOT },
	{ L"ble",	L"s,i,I",	&generateMipsMacroBranch,			MIPSM_GE|MIPSM_IMM|MIPSM_REVCMP|MIPSM_DONTWARNDELAYSLOT },
	{ L"bgeu",	L"s,t,I",	&generateMipsMacroBranch,			MIPSM_GEU|MIPSM_DONTWARNDELAYSLOT },
	{ L"bgeu",	L"s,i,I",	&generateMipsMacroBranch,			MIPSM_GEU|MIPSM_IMM|MIPSM_DONTWARNDELAYSLOT },
	{ L"bleu",	L"s,t,I",	&generateMipsMacroBranch,			MIPSM_GEU|MIPSM_REVCMP|MIPSM_DONTWARNDELAYSLOT },
	{ L"bleu",	L"s,i,I",	&generateMipsMacroBranch,			MIPSM_GEU|MIPSM_IMM|MIPSM_REVCMP|MIPSM_DONTWARNDELAYSLOT },
	{ L"bne",	L"s,i,I",	&generateMipsMacroBranch,			MIPSM_NE|MIPSM_IMM|MIPSM_DONTWARNDELAYSLOT },
	{ L"beq",	L"s,i,I",	&generateMipsMacroBranch,			MIPSM_EQ|MIPSM_IMM|MIPSM_DONTWARNDELAYSLOT },
	{ L"bltl",	L"s,t,I",	&generateMipsMacroBranch,			MIPSM_LT|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ L"bltl",	L"s,i,I",	&generateMipsMacroBranch,			MIPSM_LT|MIPSM_IMM|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ L"bgtl",	L"s,t,I",	&generateMipsMacroBranch,			MIPSM_LT|MIPSM_REVCMP|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ L"bgtl",	L"s,i,I",	&generateMipsMacroBranch,			MIPSM_LT|MIPSM_IMM|MIPSM_REVCMP|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ L"bltul",	L"s,t,I",	&generateMipsMacroBranch,			MIPSM_LTU|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ L"bltul",	L"s,i,I",	&generateMipsMacroBranch,			MIPSM_LTU|MIPSM_IMM|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ L"bgtul",	L"s,t,I",	&generateMipsMacroBranch,			MIPSM_LTU|MIPSM_REVCMP|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ L"bgtul",	L"s,i,I",	&generateMipsMacroBranch,			MIPSM_LTU|MIPSM_IMM|MIPSM_REVCMP|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ L"bgel",	L"s,t,I",	&generateMipsMacroBranch,			MIPSM_GE|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ L"bgel",	L"s,i,I",	&generateMipsMacroBranch,			MIPSM_GE|MIPSM_IMM|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ L"blel",	L"s,t,I",	&generateMipsMacroBranch,			MIPSM_GE|MIPSM_REVCMP|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ L"blel",	L"s,i,I",	&generateMipsMacroBranch,			MIPSM_GE|MIPSM_IMM|MIPSM_REVCMP|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ L"bgeul",	L"s,t,I",	&generateMipsMacroBranch,			MIPSM_GEU|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ L"bgeul",	L"s,i,I",	&generateMipsMacroBranch,			MIPSM_GEU|MIPSM_IMM|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ L"bleul",	L"s,t,I",	&generateMipsMacroBranch,			MIPSM_GEU|MIPSM_REVCMP|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ L"bleul",	L"s,i,I",	&generateMipsMacroBranch,			MIPSM_GEU|MIPSM_IMM|MIPSM_REVCMP|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ L"bnel",	L"s,i,I",	&generateMipsMacroBranch,			MIPSM_NE|MIPSM_IMM|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },
	{ L"beql",	L"s,i,I",	&generateMipsMacroBranch,			MIPSM_EQ|MIPSM_IMM|MIPSM_DONTWARNDELAYSLOT|MIPSM_LIKELY },

	{ L"slt",	L"d,s,I",	&generateMipsMacroSet,				MIPSM_LT|MIPSM_IMM },
	{ L"sltu",	L"d,s,I",	&generateMipsMacroSet,				MIPSM_LTU|MIPSM_IMM },
	{ L"sgt",	L"d,s,I",	&generateMipsMacroSet,				MIPSM_LT|MIPSM_IMM|MIPSM_REVCMP },
	{ L"sgtu",	L"d,s,I",	&generateMipsMacroSet,				MIPSM_LTU|MIPSM_IMM|MIPSM_REVCMP },
	{ L"sge",	L"d,s,t",	&generateMipsMacroSet,				MIPSM_GE },
	{ L"sge",	L"d,s,I",	&generateMipsMacroSet,				MIPSM_GE|MIPSM_IMM },
	{ L"sle",	L"d,s,t",	&generateMipsMacroSet,				MIPSM_GE|MIPSM_REVCMP },
	{ L"sle",	L"d,s,I",	&generateMipsMacroSet,				MIPSM_GE|MIPSM_IMM|MIPSM_REVCMP },
	{ L"sgeu",	L"d,s,t",	&generateMipsMacroSet,				MIPSM_GEU },
	{ L"sgeu",	L"d,s,I",	&generateMipsMacroSet,				MIPSM_GEU|MIPSM_IMM },
	{ L"sleu",	L"d,s,t",	&generateMipsMacroSet,				MIPSM_GEU|MIPSM_REVCMP },
	{ L"sleu",	L"d,s,I",	&generateMipsMacroSet,				MIPSM_GEU|MIPSM_IMM|MIPSM_REVCMP },
	{ L"sne",	L"d,s,t",	&generateMipsMacroSet,				MIPSM_NE },
	{ L"sne",	L"d,s,I",	&generateMipsMacroSet,				MIPSM_NE|MIPSM_IMM },
	{ L"seq",	L"d,s,t",	&generateMipsMacroSet,				MIPSM_EQ },
	{ L"seq",	L"d,s,I",	&generateMipsMacroSet,				MIPSM_EQ|MIPSM_IMM },

	{ L"rol",	L"d,s,t",	&generateMipsMacroRotate,			MIPSM_LEFT },
	{ L"rol",	L"d,s,i",	&generateMipsMacroRotate,			MIPSM_LEFT|MIPSM_IMM },
	{ L"ror",	L"d,s,t",	&generateMipsMacroRotate,			MIPSM_RIGHT },
	{ L"ror",	L"d,s,i",	&generateMipsMacroRotate,			MIPSM_RIGHT|MIPSM_IMM },

	{ nullptr,	nullptr,	nullptr,							0 }
};

// file: Archs/MIPS/MipsOpcodes.cpp

const tMipsOpcode MipsOpcodes[] = {
//     31---------26---------------------------------------------------0
//     |  opcode   |                                                   |
//     ------6----------------------------------------------------------
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
// 000 | *1    | *2    | J     | JAL   | BEQ   | BNE   | BLEZ  | BGTZ  | 00..07
// 001 | ADDI  | ADDIU | SLTI  | SLTIU | ANDI  | ORI   | XORI  | LUI   | 08..0F
// 010 | *3    | *4    | *5    | ---   | BEQL  | BNEL  | BLEZL | BGTZL | 10..17
// 011 | DADDI | DADDIU| LDL   | LDR   | ---   | ---   | LQ    | SQ    | 18..1F
// 100 | LB    | LH    | LWL   | LW    | LBU   | LHU   | LWR   | LWU   | 20..27
// 101 | SB    | SH    | SWL   | SW    | SDL   | SDR   | SWR   | CACHE | 28..2F
// 110 | LL    | LWC1  | LV.S  | ---   | LLD   | ULV.Q | LV.Q  | LD    | 30..37
// 111 | SC    | SWC1  | SV.S  | ---   | SCD   | USV.Q | SV.Q  | SD    | 38..3F
//  hi |-------|-------|-------|-------|-------|-------|-------|-------|
//		*1 = SPECIAL	*2 = REGIMM		*3 = COP0		*4 = COP1		*5 = COP2
	{ "j",		"i26",				MIPS_OP(0x02), 			MA_MIPS1,	MO_IPCA|MO_DELAY|MO_NODELAYSLOT },
	{ "jal",	"i26",				MIPS_OP(0x03),			MA_MIPS1,	MO_IPCA|MO_DELAY|MO_NODELAYSLOT },
	{ "beq",	"s,t,i16",			MIPS_OP(0x04),			MA_MIPS1,	MO_IPCR|MO_DELAY|MO_NODELAYSLOT },
	{ "beqz",	"s,i16",			MIPS_OP(0x04),			MA_MIPS1,	MO_IPCR|MO_DELAY|MO_NODELAYSLOT },
	{ "b",		"i16",				MIPS_OP(0x04), 			MA_MIPS1,	MO_IPCR|MO_DELAY|MO_NODELAYSLOT },
	{ "bne",	"s,t,i16",			MIPS_OP(0x05),			MA_MIPS1,	MO_IPCR|MO_DELAY|MO_NODELAYSLOT },
	{ "bnez",	"s,i16",			MIPS_OP(0x05),			MA_MIPS1,	MO_IPCR|MO_DELAY|MO_NODELAYSLOT },
	{ "blez",	"s,i16",			MIPS_OP(0x06),			MA_MIPS1,	MO_IPCR|MO_DELAY|MO_NODELAYSLOT },
	{ "bgtz",	"s,i16",			MIPS_OP(0x07),			MA_MIPS1,	MO_IPCR|MO_DELAY|MO_NODELAYSLOT },
	{ "addi",	"t,s,i16",			MIPS_OP(0x08),			MA_MIPS1,	MO_IGNORERTD },
	{ "addi",	"s,i16",			MIPS_OP(0x08),			MA_MIPS1,	MO_RST },
	{ "subi",	"t,s,i16",			MIPS_OP(0x08),			MA_MIPS1,	MO_IGNORERTD|MO_NEGIMM },
	{ "subi",	"s,i16",			MIPS_OP(0x08),			MA_MIPS1,	MO_RST|MO_NEGIMM },
	{ "addiu",	"t,s,i16",			MIPS_OP(0x09),			MA_MIPS1,	MO_IGNORERTD },
	{ "addiu",	"s,i16",			MIPS_OP(0x09),			MA_MIPS1,	MO_RST },
	{ "subiu",	"t,s,i16",			MIPS_OP(0x09),			MA_MIPS1,	MO_IGNORERTD|MO_NEGIMM },
	{ "subiu",	"s,i16",			MIPS_OP(0x09),			MA_MIPS1,	MO_RST|MO_NEGIMM },
	{ "slti",	"t,s,i16",			MIPS_OP(0x0A),			MA_MIPS1,	MO_IGNORERTD },
	{ "slti",	"s,i16",			MIPS_OP(0x0A),			MA_MIPS1,	MO_RST },
	{ "sltiu",	"t,s,i16",			MIPS_OP(0x0B),			MA_MIPS1,	MO_IGNORERTD },
	{ "sltiu",	"s,i16",			MIPS_OP(0x0B),			MA_MIPS1,	MO_RST },
	{ "andi",	"t,s,i16",			MIPS_OP(0x0C),			MA_MIPS1,	MO_IGNORERTD },
	{ "andi",	"s,i16",			MIPS_OP(0x0C),			MA_MIPS1,	MO_RST },
	{ "ori",	"t,s,i16",			MIPS_OP(0x0D),			MA_MIPS1,	MO_IGNORERTD },
	{ "ori",	"s,i16",			MIPS_OP(0x0D),			MA_MIPS1,	MO_RST },
	{ "xori",	"t,s,i16",			MIPS_OP(0x0E),			MA_MIPS1,	MO_IGNORERTD },
	{ "xori",	"s,i16",			MIPS_OP(0x0E),			MA_MIPS1,	MO_RST },
	{ "lui",	"t,i16",			MIPS_OP(0x0F),			MA_MIPS1,	MO_IGNORERTD },
	{ "cop2",	"i25",				MIPS_OP(0x12)|(1<<25), 	MA_PSX,		0 },
	{ "beql",	"s,t,i16",			MIPS_OP(0x14),			MA_MIPS2,	MO_IPCR|MO_DELAY|MO_NODELAYSLOT },
	{ "beqzl",	"s,i16",			MIPS_OP(0x14),			MA_MIPS2,	MO_IPCR|MO_DELAY|MO_NODELAYSLOT },
	{ "bnel",	"s,t,i16",			MIPS_OP(0x15),			MA_MIPS2,	MO_IPCR|MO_DELAY|MO_NODELAYSLOT },
	{ "bnezl",	"s,i16",			MIPS_OP(0x15),			MA_MIPS2,	MO_IPCR|MO_DELAY|MO_NODELAYSLOT },
	{ "blezl",	"s,i16",			MIPS_OP(0x16),			MA_MIPS2,	MO_IPCR|MO_DELAY|MO_NODELAYSLOT },
	{ "bgtzl",	"s,i16",			MIPS_OP(0x17),			MA_MIPS2,	MO_IPCR|MO_DELAY|MO_NODELAYSLOT },
	{ "daddi",	"t,s,i16",			MIPS_OP(0x18),			MA_MIPS3,	MO_64BIT },
	{ "daddi",	"s,i16",			MIPS_OP(0x18),			MA_MIPS3,	MO_64BIT|MO_RST },
	{ "dsubi",	"t,s,i16",			MIPS_OP(0x18),			MA_MIPS3,	MO_64BIT|MO_NEGIMM },
	{ "dsubi",	"s,i16",			MIPS_OP(0x18),			MA_MIPS3,	MO_64BIT|MO_RST|MO_NEGIMM },
	{ "daddiu",	"t,s,i16",			MIPS_OP(0x19),			MA_MIPS3,	MO_64BIT },
	{ "daddiu",	"s,i16",			MIPS_OP(0x19),			MA_MIPS3,	MO_64BIT|MO_RST },
	{ "dsubiu",	"t,s,i16",			MIPS_OP(0x19),			MA_MIPS3,	MO_64BIT|MO_NEGIMM },
	{ "dsubiu",	"s,i16",			MIPS_OP(0x19),			MA_MIPS3,	MO_64BIT|MO_RST|MO_NEGIMM },
	{ "ldl",	"t,i16(s)",			MIPS_OP(0x1A),			MA_MIPS3,	MO_64BIT|MO_DELAYRT|MO_IGNORERTD },
	{ "ldl",	"t,(s)",			MIPS_OP(0x1A),			MA_MIPS3,	MO_64BIT|MO_DELAYRT|MO_IGNORERTD },
	{ "ldr",	"t,i16(s)",			MIPS_OP(0x1B),			MA_MIPS3,	MO_64BIT|MO_DELAYRT|MO_IGNORERTD },
	{ "ldr",	"t,(s)",			MIPS_OP(0x1B),			MA_MIPS3,	MO_64BIT|MO_DELAYRT|MO_IGNORERTD },
	{ "lq",		"t,i16(s)",			MIPS_OP(0x1E),			MA_PS2,		MO_DELAYRT|MO_IGNORERTD },
	{ "lq",		"t,(s)",			MIPS_OP(0x1E),			MA_PS2,		MO_DELAYRT|MO_IGNORERTD },
	{ "sq",		"t,i16(s)",			MIPS_OP(0x1F),			MA_PS2,		MO_DELAYRT|MO_IGNORERTD },
	{ "sq",		"t,(s)",			MIPS_OP(0x1F),			MA_PS2,		MO_DELAYRT|MO_IGNORERTD },
	{ "lb",		"t,i16(s)",			MIPS_OP(0x20),			MA_MIPS1,	MO_DELAYRT|MO_IGNORERTD },
	{ "lb",		"t,(s)",			MIPS_OP(0x20),			MA_MIPS1,	MO_DELAYRT|MO_IGNORERTD },
	{ "lh",		"t,i16(s)",			MIPS_OP(0x21),			MA_MIPS1,	MO_DELAYRT|MO_IGNORERTD },
	{ "lh",		"t,(s)",			MIPS_OP(0x21),			MA_MIPS1,	MO_DELAYRT|MO_IGNORERTD },
	{ "lwl",	"t,i16(s)",			MIPS_OP(0x22),			MA_MIPS1|MA_EXRSP,	MO_DELAYRT|MO_IGNORERTD },
	{ "lwl",	"t,(s)",			MIPS_OP(0x22),			MA_MIPS1|MA_EXRSP,	MO_DELAYRT|MO_IGNORERTD },
	{ "lw",		"t,i16(s)",			MIPS_OP(0x23),			MA_MIPS1,	MO_DELAYRT|MO_IGNORERTD },
	{ "lw",		"t,(s)",			MIPS_OP(0x23),			MA_MIPS1,	MO_DELAYRT|MO_IGNORERTD },
	{ "lbu",	"t,i16(s)",			MIPS_OP(0x24),			MA_MIPS1,	MO_DELAYRT|MO_IGNORERTD },
	{ "lbu",	"t,(s)",			MIPS_OP(0x24),			MA_MIPS1,	MO_DELAYRT|MO_IGNORERTD },
	{ "lhu",	"t,i16(s)",			MIPS_OP(0x25),			MA_MIPS1,	MO_DELAYRT|MO_IGNORERTD },
	{ "lhu",	"t,(s)",			MIPS_OP(0x25),			MA_MIPS1,	MO_DELAYRT|MO_IGNORERTD },
	{ "lwr",	"t,i16(s)",			MIPS_OP(0x26),			MA_MIPS1|MA_EXRSP,	MO_DELAYRT|MO_IGNORERTD },
	{ "lwr",	"t,(s)",			MIPS_OP(0x26),			MA_MIPS1|MA_EXRSP,	MO_DELAYRT|MO_IGNORERTD },
	{ "lwu",	"t,i16(s)",			MIPS_OP(0x27),			MA_MIPS3,	MO_64BIT|MO_DELAYRT },
	{ "lwu",	"t,(s)",			MIPS_OP(0x27),			MA_MIPS3,	MO_64BIT|MO_DELAYRT },
	{ "sb",		"t,i16(s)",			MIPS_OP(0x28),			MA_MIPS1,	0 },
	{ "sb",		"t,(s)",			MIPS_OP(0x28),			MA_MIPS1,	0 },
	{ "sh",		"t,i16(s)",			MIPS_OP(0x29),			MA_MIPS1,	0 },
	{ "sh",		"t,(s)",			MIPS_OP(0x29),			MA_MIPS1,	0 },
	{ "swl",	"t,i16(s)",			MIPS_OP(0x2A),			MA_MIPS1|MA_EXRSP,	0 },
	{ "swl",	"t,(s)",			MIPS_OP(0x2A),			MA_MIPS1|MA_EXRSP,	0 },
	{ "sw",		"t,i16(s)",			MIPS_OP(0x2B),			MA_MIPS1,	0 },
	{ "sw",		"t,(s)",			MIPS_OP(0x2B),			MA_MIPS1,	0 },
	{ "sdl",	"t,i16(s)",			MIPS_OP(0x2C),			MA_MIPS3,	MO_64BIT },
	{ "sdl",	"t,(s)",			MIPS_OP(0x2C),			MA_MIPS3,	MO_64BIT },
	{ "sdr",	"t,i16(s)",			MIPS_OP(0x2D),			MA_MIPS3,	MO_64BIT|MO_IGNORERTD },
	{ "sdr",	"t,(s)",			MIPS_OP(0x2D),			MA_MIPS3,	MO_64BIT|MO_IGNORERTD },
	{ "swr",	"t,i16(s)",			MIPS_OP(0x2E),			MA_MIPS1|MA_EXRSP,	0 },
	{ "swr",	"t,(s)",			MIPS_OP(0x2E),			MA_MIPS1|MA_EXRSP,	0 },
	{ "cache",	"jc,i16(s)",		MIPS_OP(0x2F),			MA_MIPS2,	0 },
	{ "cache",	"jc,(s)",			MIPS_OP(0x2F),			MA_MIPS2,	0 },
	{ "ll",		"t,i16(s)",			MIPS_OP(0x30),			MA_MIPS2,	MO_DELAYRT|MO_IGNORERTD },
	{ "ll",		"t,(s)",			MIPS_OP(0x30),			MA_MIPS2,	MO_DELAYRT|MO_IGNORERTD },
	{ "lwc1",	"T,i16(s)",			MIPS_OP(0x31),			MA_MIPS1,	MO_FPU },
	{ "lwc1",	"T,(s)",			MIPS_OP(0x31),			MA_MIPS1,	MO_FPU },
	{ "l.s",	"T,i16(s)",			MIPS_OP(0x31),			MA_MIPS1,	MO_FPU },
	{ "l.s",	"T,(s)",			MIPS_OP(0x31),			MA_MIPS1,	MO_FPU },
	{ "lwc2",	"gt,i16(s)",		MIPS_OP(0x32),			MA_PSX,		0 },
	{ "lwc2",	"gt,(s)",			MIPS_OP(0x32),			MA_PSX,		0 },
	{ "lv.s",	"vt,i16(s)",		MIPS_OP(0x32),			MA_PSP,		MO_VFPU_SINGLE|MO_VFPU_MIXED|MO_IMMALIGNED },
	{ "lv.s",	"vt,(s)",			MIPS_OP(0x32),			MA_PSP,		MO_VFPU_SINGLE|MO_VFPU_MIXED },
	{ "lld",	"t,i16(s)",			MIPS_OP(0x34),			MA_MIPS3,	MO_64BIT|MO_DELAYRT },
	{ "lld",	"t,(s)",			MIPS_OP(0x34),			MA_MIPS3,	MO_64BIT|MO_DELAYRT },
	{ "ldc1",	"T,i16(s)",			MIPS_OP(0x35),			MA_MIPS2,	MO_DFPU },
	{ "ldc1",	"T,(s)",			MIPS_OP(0x35),			MA_MIPS2,	MO_DFPU },
	{ "l.d",	"T,i16(s)",			MIPS_OP(0x35),			MA_MIPS2,	MO_DFPU },
	{ "l.d",	"T,(s)",			MIPS_OP(0x35),			MA_MIPS2,	MO_DFPU },
	{ "ulv.q",	"vt,i16(s)",		MIPS_OP(0x35),			MA_PSP,		MO_VFPU_QUAD|MO_VFPU_MIXED|MO_IMMALIGNED },
	{ "ulv.q",	"vt,(s)",			MIPS_OP(0x35),			MA_PSP,		MO_VFPU_QUAD|MO_VFPU_MIXED },
	{ "lvl.q",	"vt,i16(s)",		MIPS_OP(0x35),			MA_PSP,		MO_VFPU_QUAD|MO_VFPU_MIXED|MO_VFPU_6BIT|MO_IMMALIGNED },
	{ "lvl.q",	"vt,(s)",			MIPS_OP(0x35),			MA_PSP,		MO_VFPU_QUAD|MO_VFPU_MIXED|MO_VFPU_6BIT },
	{ "lvr.q",	"vt,i16(s)",		MIPS_OP(0x35)|0x02,		MA_PSP,		MO_VFPU_QUAD|MO_VFPU_MIXED|MO_VFPU_6BIT|MO_IMMALIGNED },
	{ "lvr.q",	"vt,(s)",			MIPS_OP(0x35)|0x02,		MA_PSP,		MO_VFPU_QUAD|MO_VFPU_MIXED|MO_VFPU_6BIT },
	{ "lv.q",	"vt,i16(s)",		MIPS_OP(0x36),			MA_PSP,		MO_VFPU_QUAD|MO_VFPU_MIXED|MO_VFPU_6BIT|MO_IMMALIGNED },
	{ "lv.q",	"vt,(s)",			MIPS_OP(0x36),			MA_PSP,		MO_VFPU_QUAD|MO_VFPU_MIXED|MO_VFPU_6BIT },
	{ "lqc2",	"Vt,i16(s)",		MIPS_OP(0x36),			MA_PS2,		MO_DELAYRT },
	{ "ld",		"t,i16(s)",			MIPS_OP(0x37),			MA_MIPS3,	MO_64BIT|MO_DELAYRT },
	{ "ld",		"t,(s)",			MIPS_OP(0x37),			MA_MIPS3,	MO_64BIT|MO_DELAYRT },
	{ "sc",		"t,i16(s)",			MIPS_OP(0x38),			MA_MIPS2,	0 },
	{ "sc",		"t,(s)",			MIPS_OP(0x38),			MA_MIPS2,	0 },
	{ "swc1",	"T,i16(s)",			MIPS_OP(0x39),			MA_MIPS1,	MO_FPU },
	{ "swc1",	"T,(s)",			MIPS_OP(0x39),			MA_MIPS1,	MO_FPU },
	{ "s.s",	"T,i16(s)",			MIPS_OP(0x39),			MA_MIPS1,	MO_FPU },
	{ "s.s",	"T,(s)",			MIPS_OP(0x39),			MA_MIPS1,	MO_FPU },
	{ "swc2",	"gt,i16(s)",		MIPS_OP(0x3A),			MA_PSX,		0 },
	{ "swc2",	"gt,(s)",			MIPS_OP(0x3A),			MA_PSX,		0 },
	{ "sv.s",	"vt,i16(s)",		MIPS_OP(0x3A),			MA_PSP,		MO_VFPU_SINGLE|MO_VFPU_MIXED|MO_IMMALIGNED },
	{ "sv.s",	"vt,(s)",			MIPS_OP(0x3A),			MA_PSP,		MO_VFPU_SINGLE|MO_VFPU_MIXED },
	{ "scd",	"t,i16(s)",			MIPS_OP(0x3C),			MA_MIPS3,	MO_64BIT|MO_DELAYRT },
	{ "scd",	"t,(s)",			MIPS_OP(0x3C),			MA_MIPS3,	MO_64BIT|MO_DELAYRT },
	{ "sdc1",	"T,i16(s)",			MIPS_OP(0x3D),			MA_MIPS2,	MO_DFPU },
	{ "sdc1",	"T,(s)",			MIPS_OP(0x3D),			MA_MIPS2,	MO_DFPU },
	{ "s.d",	"T,i16(s)",			MIPS_OP(0x3D),			MA_MIPS2,	MO_DFPU },
	{ "s.d",	"T,(s)",			MIPS_OP(0x3D),			MA_MIPS2,	MO_DFPU },
	{ "usv.q",	"vt,i16(s)",		MIPS_OP(0x3D),			MA_PSP,		MO_VFPU_QUAD|MO_VFPU_MIXED|MO_IMMALIGNED },
	{ "usv.q",	"vt,(s)",			MIPS_OP(0x3D),			MA_PSP,		MO_VFPU_QUAD|MO_VFPU_MIXED },
	{ "svl.q",	"vt,i16(s)",		MIPS_OP(0x3D),			MA_PSP,		MO_VFPU_QUAD|MO_VFPU_MIXED|MO_VFPU_6BIT|MO_IMMALIGNED },
	{ "svl.q",	"vt,(s)",			MIPS_OP(0x3D),			MA_PSP,		MO_VFPU_QUAD|MO_VFPU_MIXED|MO_VFPU_6BIT },
	{ "svr.q",	"vt,i16(s)",		MIPS_OP(0x3D)|0x02,		MA_PSP,		MO_VFPU_QUAD|MO_VFPU_MIXED|MO_VFPU_6BIT|MO_IMMALIGNED },
	{ "svr.q",	"vt,(s)",			MIPS_OP(0x3D)|0x02,		MA_PSP,		MO_VFPU_QUAD|MO_VFPU_MIXED|MO_VFPU_6BIT },
	{ "sv.q",	"vt,i16(s),w",		MIPS_OP(0x3E)|0x02,		MA_PSP,		MO_VFPU_QUAD|MO_VFPU_MIXED|MO_VFPU_6BIT|MO_IMMALIGNED },
	{ "sv.q",	"vt,(s),w",			MIPS_OP(0x3E)|0x02,		MA_PSP,		MO_VFPU_QUAD|MO_VFPU_MIXED|MO_VFPU_6BIT },
	{ "sv.q",	"vt,i16(s)",		MIPS_OP(0x3E),			MA_PSP,		MO_VFPU_QUAD|MO_VFPU_MIXED|MO_VFPU_6BIT|MO_IMMALIGNED },
	{ "sv.q",	"vt,(s)",			MIPS_OP(0x3E),			MA_PSP,		MO_VFPU_QUAD|MO_VFPU_MIXED|MO_VFPU_6BIT },
	{ "sqc2",	"Vt,i16(s)",		MIPS_OP(0x3E),			MA_PS2,		MO_DELAYRT },
	{ "sd",		"t,i16(s)",			MIPS_OP(0x3F),			MA_MIPS3,	MO_64BIT|MO_DELAYRT },
	{ "sd",		"t,(s)",			MIPS_OP(0x3F),			MA_MIPS3,	MO_64BIT|MO_DELAYRT },

//     31---------26------------------------------------------5--------0
//     |=   SPECIAL|                                         | function|
//     ------6----------------------------------------------------6-----
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
// 000 | SLL   | ---   | SRL*1 | SRA   | SLLV  |  ---  | SRLV*2| SRAV  | 00..07
// 001 | JR    | JALR  | MOVZ  | MOVN  |SYSCALL| BREAK |  ---  | SYNC  | 08..0F
// 010 | MFHI  | MTHI  | MFLO  | MTLO  | DSLLV |  ---  |   *3  |  *4   | 10..17
// 011 | MULT  | MULTU | DIV   | DIVU  | MADD  | MADDU | ----  | ----- | 18..1F
// 100 | ADD   | ADDU  | SUB   | SUBU  | AND   | OR    | XOR   | NOR   | 20..27
// 101 | mfsa  | mtsa  | SLT   | SLTU  |  *5   |  *6   |  *7   |  *8   | 28..2F
// 110 | TGE   | TGEU  | TLT   | TLTU  | TEQ   |  ---  | TNE   |  ---  | 30..37
// 111 | dsll  |  ---  | dsrl  | dsra  |dsll32 |  ---  |dsrl32 |dsra32 | 38..3F
//  hi |-------|-------|-------|-------|-------|-------|-------|-------|
// *1:	rotr when rs = 1 (PSP only)		*2:	rotrv when sa = 1 (PSP only)
// *3:	dsrlv on PS2, clz on PSP		*4:	dsrav on PS2, clo on PSP
// *5:	dadd on PS2, max on PSP			*6:	daddu on PS2, min on PSP
// *7:	dsub on PS2, msub on PSP		*8:	dsubu on PS2, msubu on PSP
	{ "sll",	"d,t,i5",	MIPS_SPECIAL(0x00),				MA_MIPS1,	0 },
	{ "sll",	"d,i5",		MIPS_SPECIAL(0x00),				MA_MIPS1,	MO_RDT },
	{ "nop",	"",			MIPS_SPECIAL(0x00),				MA_MIPS1,	0 },
	{ "srl",	"d,t,i5",	MIPS_SPECIAL(0x02),				MA_MIPS1,	0 },
	{ "srl",	"d,i5",		MIPS_SPECIAL(0x02),				MA_MIPS1,	MO_RDT },
	{ "rotr",	"d,t,i5",	MIPS_SPECIAL(0x02)|MIPS_RS(1),	MA_PSP,		0 },
	{ "rotr",	"d,i5",		MIPS_SPECIAL(0x02)|MIPS_RS(1),	MA_PSP,		MO_RDT },
	{ "sra",	"d,t,i5",	MIPS_SPECIAL(0x03),				MA_MIPS1,	0 },
	{ "sra",	"d,i5",		MIPS_SPECIAL(0x03),				MA_MIPS1,	MO_RDT },
	{ "sllv",	"d,t,s",	MIPS_SPECIAL(0x04),				MA_MIPS1,	0 },
	{ "sllv",	"d,s",		MIPS_SPECIAL(0x04),				MA_MIPS1,	MO_RDT },
	{ "srlv",	"d,t,s",	MIPS_SPECIAL(0x06),				MA_MIPS1,	0 },
	{ "srlv",	"d,s",		MIPS_SPECIAL(0x06),				MA_MIPS1,	MO_RDT },
	{ "rotrv",	"d,t,s",	MIPS_SPECIAL(0x06)|MIPS_SA(1),	MA_PSP,		0 },
	{ "rotrv",	"d,s",		MIPS_SPECIAL(0x06)|MIPS_SA(1),	MA_PSP,		MO_RDT },
	{ "srav",	"d,t,s",	MIPS_SPECIAL(0x07),				MA_MIPS1,	0 },
	{ "srav",	"d,s",		MIPS_SPECIAL(0x07),				MA_MIPS1,	MO_RDT },
	{ "jr",		"s",		MIPS_SPECIAL(0x08),				MA_MIPS1,	MO_DELAY|MO_NODELAYSLOT },
	{ "jalr",	"s,d",		MIPS_SPECIAL(0x09),				MA_MIPS1,	MO_DELAY|MO_NODELAYSLOT },
	{ "jalr",	"s",		MIPS_SPECIAL(0x09)|MIPS_RD(31),	MA_MIPS1,	MO_DELAY|MO_NODELAYSLOT },
	{ "movz",	"d,s,t",	MIPS_SPECIAL(0x0A),				MA_MIPS4|MA_PS2|MA_PSP,	0 },
	{ "movn",	"d,s,t",	MIPS_SPECIAL(0x0B),				MA_MIPS4|MA_PS2|MA_PSP,	0 },
	{ "syscall","i20",		MIPS_SPECIAL(0x0C),				MA_MIPS1|MA_EXRSP,	MO_NODELAYSLOT },
	{ "syscall","",			MIPS_SPECIAL(0x0C),				MA_MIPS1|MA_EXRSP,	MO_NODELAYSLOT },
	{ "break",	"i20",		MIPS_SPECIAL(0x0D),				MA_MIPS1,	MO_NODELAYSLOT },
	{ "break",	"",			MIPS_SPECIAL(0x0D),				MA_MIPS1,	MO_NODELAYSLOT },
	{ "sync",	"",			MIPS_SPECIAL(0x0F),				MA_MIPS2,	0 },
	{ "mfhi",	"d",		MIPS_SPECIAL(0x10),				MA_MIPS1|MA_EXRSP,	0 },
	{ "mthi",	"s",		MIPS_SPECIAL(0x11),				MA_MIPS1|MA_EXRSP,	0 },
	{ "mflo",	"d",		MIPS_SPECIAL(0x12),				MA_MIPS1|MA_EXRSP,	0 },
	{ "mtlo",	"s",		MIPS_SPECIAL(0x13),				MA_MIPS1|MA_EXRSP,	0 },
	{ "dsllv",	"d,t,s",	MIPS_SPECIAL(0x14),				MA_MIPS3,	MO_64BIT },
	{ "dsllv",	"d,s",		MIPS_SPECIAL(0x14),				MA_MIPS3,	MO_64BIT|MO_RDT },
	{ "dsrlv",	"d,t,s",	MIPS_SPECIAL(0x16),				MA_MIPS3,	MO_64BIT },
	{ "dsrlv",	"d,s",		MIPS_SPECIAL(0x16),				MA_MIPS3,	MO_64BIT|MO_RDT },
	{ "clz",	"d,s",		MIPS_SPECIAL(0x16),				MA_PSP,		0 },
	{ "dsrav",	"d,t,s",	MIPS_SPECIAL(0x17),				MA_MIPS3,	MO_64BIT },
	{ "dsrav",	"d,s",		MIPS_SPECIAL(0x17),				MA_MIPS3,	MO_64BIT|MO_RDT },
	{ "clo",	"d,s",		MIPS_SPECIAL(0x17),				MA_PSP,		0 },
	{ "mult",	"d,s,t",	MIPS_SPECIAL(0x18),				MA_PS2,		0 },
	{ "multu",	"d,s,t",	MIPS_SPECIAL(0x19),				MA_PS2,		0 },
	{ "mult",	"s,t",		MIPS_SPECIAL(0x18),				MA_MIPS1|MA_EXRSP,	0 },
	{ "mult",	"r\x0,s,t",	MIPS_SPECIAL(0x18),				MA_MIPS1|MA_EXRSP,	0 },
	{ "multu",	"s,t",		MIPS_SPECIAL(0x19),				MA_MIPS1|MA_EXRSP,	0 },
	{ "multu",	"r\x0,s,t",	MIPS_SPECIAL(0x19),				MA_MIPS1|MA_EXRSP,	0 },
	{ "div",	"s,t",		MIPS_SPECIAL(0x1A),				MA_MIPS1|MA_EXRSP,	0 },
	{ "div",	"r\x0,s,t",	MIPS_SPECIAL(0x1A),				MA_MIPS1|MA_EXRSP,	0 },
	{ "divu",	"s,t",		MIPS_SPECIAL(0x1B),				MA_MIPS1|MA_EXRSP,	0 },
	{ "divu",	"r\x0,s,t",	MIPS_SPECIAL(0x1B),				MA_MIPS1|MA_EXRSP,	0 },
	{ "dmult",	"s,t",		MIPS_SPECIAL(0x1C),				MA_MIPS3|MA_EXPS2,	MO_64BIT },
	{ "dmult",	"r\x0,s,t",	MIPS_SPECIAL(0x1C),				MA_MIPS3|MA_EXPS2,	MO_64BIT },
	{ "madd",	"s,t",		MIPS_SPECIAL(0x1C),				MA_PSP,		0 },
	{ "dmultu",	"s,t",		MIPS_SPECIAL(0x1D),				MA_MIPS3|MA_EXPS2,	MO_64BIT },
	{ "dmultu",	"r\x0,s,t",	MIPS_SPECIAL(0x1D),				MA_MIPS3|MA_EXPS2,	MO_64BIT },
	{ "maddu",	"s,t",		MIPS_SPECIAL(0x1D),				MA_PSP,		0 },
	{ "ddiv",	"s,t",		MIPS_SPECIAL(0x1E),				MA_MIPS3|MA_EXPS2,	MO_64BIT },
	{ "ddiv",	"r\x0,s,t",	MIPS_SPECIAL(0x1E),				MA_MIPS3|MA_EXPS2,	MO_64BIT },
	{ "ddivu",	"s,t",		MIPS_SPECIAL(0x1F),				MA_MIPS3|MA_EXPS2,	MO_64BIT },
	{ "ddivu",	"r\x0,s,t",	MIPS_SPECIAL(0x1F),				MA_MIPS3|MA_EXPS2,	MO_64BIT },
	{ "add",	"d,s,t",	MIPS_SPECIAL(0x20),				MA_MIPS1,	0 },
	{ "add",	"s,t",		MIPS_SPECIAL(0x20),				MA_MIPS1,	MO_RSD },
	{ "addu",	"d,s,t",	MIPS_SPECIAL(0x21),				MA_MIPS1,	0 },
	{ "addu",	"s,t",		MIPS_SPECIAL(0x21),				MA_MIPS1,	MO_RSD },
	{ "move",	"d,s",		MIPS_SPECIAL(0x21),				MA_MIPS1,	0 },
	{ "clear",	"d",		MIPS_SPECIAL(0x21),				MA_MIPS1,	0 },
	{ "sub",	"d,s,t",	MIPS_SPECIAL(0x22),				MA_MIPS1,	0 },
	{ "sub",	"s,t",		MIPS_SPECIAL(0x22),				MA_MIPS1,	MO_RSD },
	{ "neg",	"d,t",		MIPS_SPECIAL(0x22),				MA_MIPS1,	0 },
	{ "subu",	"d,s,t",	MIPS_SPECIAL(0x23),				MA_MIPS1,	0 },
	{ "subu",	"s,t",		MIPS_SPECIAL(0x23),				MA_MIPS1,	MO_RSD },
	{ "negu",	"d,t",		MIPS_SPECIAL(0x23),				MA_MIPS1,	0 },
	{ "and",	"d,s,t",	MIPS_SPECIAL(0x24),				MA_MIPS1,	0 },
	{ "and",	"s,t",		MIPS_SPECIAL(0x24),				MA_MIPS1,	MO_RSD },
	{ "or",		"d,s,t",	MIPS_SPECIAL(0x25),				MA_MIPS1,	0 },
	{ "or",		"s,t",		MIPS_SPECIAL(0x25),				MA_MIPS1,	MO_RSD },
	{ "xor",	"d,s,t",	MIPS_SPECIAL(0x26), 			MA_MIPS1,	0 },
	{ "eor",	"d,s,t",	MIPS_SPECIAL(0x26),				MA_MIPS1,	0 },
	{ "xor",	"s,t",		MIPS_SPECIAL(0x26), 			MA_MIPS1,	MO_RSD },
	{ "eor",	"s,t",		MIPS_SPECIAL(0x26), 			MA_MIPS1,	MO_RSD },
	{ "nor",	"d,s,t",	MIPS_SPECIAL(0x27),				MA_MIPS1,	0 },
	{ "nor",	"s,t",		MIPS_SPECIAL(0x27),				MA_MIPS1,	MO_RSD },
	{ "not",	"d,s",		MIPS_SPECIAL(0x27),				MA_MIPS1,	0 },
	{ "mfsa",	"d",		MIPS_SPECIAL(0x28),				MA_PS2,		0 },
	{ "mtsa",	"s",		MIPS_SPECIAL(0x29),				MA_PS2,		0 },
	{ "slt",	"d,s,t",	MIPS_SPECIAL(0x2A),				MA_MIPS1,	0 },
	{ "slt",	"s,t",		MIPS_SPECIAL(0x2A),				MA_MIPS1,	MO_RSD},
	{ "sgt",	"d,t,s",	MIPS_SPECIAL(0x2A),				MA_MIPS1,	0 },
	{ "sgt",	"d,s",		MIPS_SPECIAL(0x2A),				MA_MIPS1,	MO_RDT},
	{ "sltu",	"d,s,t",	MIPS_SPECIAL(0x2B),				MA_MIPS1,	0 },
	{ "sltu",	"s,t",		MIPS_SPECIAL(0x2B),				MA_MIPS1,	MO_RSD },
	{ "sgtu",	"d,t,s",	MIPS_SPECIAL(0x2B),				MA_MIPS1,	0 },
	{ "sgtu",	"d,s",		MIPS_SPECIAL(0x2B),				MA_MIPS1,	MO_RDT},
	{ "dadd",	"d,s,t",	MIPS_SPECIAL(0x2C),				MA_MIPS3,	MO_64BIT },
	{ "dadd",	"s,t",		MIPS_SPECIAL(0x2C),				MA_MIPS3,	MO_64BIT|MO_RSD },
	{ "max",	"d,s,t",	MIPS_SPECIAL(0x2C),				MA_PSP,		0 },
	{ "daddu",	"d,s,t",	MIPS_SPECIAL(0x2D), 			MA_MIPS3,	MO_64BIT },
	{ "daddu",	"s,t",		MIPS_SPECIAL(0x2D), 			MA_MIPS3,	MO_64BIT|MO_RSD },
	{ "dmove",	"d,s",		MIPS_SPECIAL(0x2D), 			MA_MIPS3,	MO_64BIT },
	{ "min",	"d,s,t",	MIPS_SPECIAL(0x2D), 			MA_PSP,		0 },
	{ "dsub",	"d,s,t",	MIPS_SPECIAL(0x2E), 			MA_MIPS3,	MO_64BIT },
	{ "dsub",	"s,t",		MIPS_SPECIAL(0x2E), 			MA_MIPS3,	MO_64BIT|MO_RSD },
	{ "dneg",	"d,t",		MIPS_SPECIAL(0x2E),				MA_MIPS3,	MO_64BIT },
	{ "msub",	"s,t",		MIPS_SPECIAL(0x2E),				MA_PSP,		0 },
	{ "dsubu",	"d,s,t",	MIPS_SPECIAL(0x2F), 			MA_MIPS3,	MO_64BIT },
	{ "dsubu",	"s,t",		MIPS_SPECIAL(0x2F), 			MA_MIPS3,	MO_64BIT|MO_RSD },
	{ "dnegu",	"d,t",		MIPS_SPECIAL(0x2F),				MA_MIPS3,	MO_64BIT },
	{ "msubu",	"s,t",		MIPS_SPECIAL(0x2F),				MA_PSP,		0 },
	{ "tge",	"s,t,i10",	MIPS_SPECIAL(0x30),				MA_MIPS2,	0 },
	{ "tge",	"s,t",		MIPS_SPECIAL(0x30),				MA_MIPS2,	0 },
	{ "tgeu",	"s,t,i10",	MIPS_SPECIAL(0x31),				MA_MIPS2,	0 },
	{ "tgeu",	"s,t",		MIPS_SPECIAL(0x31),				MA_MIPS2,	0 },
	{ "tlt",	"s,t,i10",	MIPS_SPECIAL(0x32),				MA_MIPS2,	0 },
	{ "tlt",	"s,t",		MIPS_SPECIAL(0x32),				MA_MIPS2,	0 },
	{ "tltu",	"s,t,i10",	MIPS_SPECIAL(0x33),				MA_MIPS2,	0 },
	{ "tltu",	"s,t",		MIPS_SPECIAL(0x33),				MA_MIPS2,	0 },
	{ "teq",	"s,t,i10",	MIPS_SPECIAL(0x34),				MA_MIPS2,	0 },
	{ "teq",	"s,t",		MIPS_SPECIAL(0x34),				MA_MIPS2,	0 },
	{ "tne",	"s,t,i10",	MIPS_SPECIAL(0x36),				MA_MIPS2,	0 },
	{ "tne",	"s,t",		MIPS_SPECIAL(0x36),				MA_MIPS2,	0 },
	{ "dsll",	"d,t,i5",	MIPS_SPECIAL(0x38),				MA_MIPS3,	MO_64BIT },
	{ "dsll",	"d,i5",		MIPS_SPECIAL(0x38),				MA_MIPS3,	MO_64BIT|MO_RDT },
	{ "dsrl",	"d,t,i5",	MIPS_SPECIAL(0x3A),				MA_MIPS3,	MO_64BIT },
	{ "dsrl",	"d,i5",		MIPS_SPECIAL(0x3A),				MA_MIPS3,	MO_64BIT|MO_RDT },
	{ "dsra",	"d,t,i5",	MIPS_SPECIAL(0x3B),				MA_MIPS3,	MO_64BIT },
	{ "dsra",	"d,i5",		MIPS_SPECIAL(0x3B),				MA_MIPS3,	MO_64BIT|MO_RDT },
	{ "dsll32",	"d,t,i5",	MIPS_SPECIAL(0x3C),				MA_MIPS3,	MO_64BIT },
	{ "dsll32",	"d,i5",		MIPS_SPECIAL(0x3C),				MA_MIPS3,	MO_64BIT|MO_RDT },
	{ "dsrl32",	"d,t,i5",	MIPS_SPECIAL(0x3E),				MA_MIPS3,	MO_64BIT },
	{ "dsrl32",	"d,i5",		MIPS_SPECIAL(0x3E),				MA_MIPS3,	MO_64BIT|MO_RDT },
	{ "dsra32",	"d,t,i5",	MIPS_SPECIAL(0x3F),				MA_MIPS3,	MO_64BIT },
	{ "dsra32",	"d,i5",		MIPS_SPECIAL(0x3F),				MA_MIPS3,	MO_64BIT|MO_RDT },

//     REGIMM: encoded by the rt field when opcode field = REGIMM.
//     31---------26----------20-------16------------------------------0
//     |=    REGIMM|          |   rt    |                              |
//     ------6---------------------5------------------------------------
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
//  00 | BLTZ  | BGEZ  | BLTZL | BGEZL |  ---  |  ---  |  ---  |  ---  | 00-07
//  01 | tgei  | tgeiu | tlti  | tltiu | teqi  |  ---  | tnei  |  ---  | 08-0F
//  10 | BLTZAL| BGEZAL|BLTZALL|BGEZALL|  ---  |  ---  |  ---  |  ---  | 10-17
//  11 | mtsab | mtsah |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 18-1F
//  hi |-------|-------|-------|-------|-------|-------|-------|-------|
	{ "bltz",	"s,i16",	MIPS_REGIMM(0x00),				MA_MIPS1,	MO_IPCR|MO_DELAY|MO_NODELAYSLOT },
	{ "bgez",	"s,i16",	MIPS_REGIMM(0x01),				MA_MIPS1,	MO_IPCR|MO_DELAY|MO_NODELAYSLOT },
	{ "bltzl",	"s,i16",	MIPS_REGIMM(0x02),				MA_MIPS2,	MO_IPCR|MO_DELAY|MO_NODELAYSLOT },
	{ "bgezl",	"s,i16",	MIPS_REGIMM(0x03),				MA_MIPS2,	MO_IPCR|MO_DELAY|MO_NODELAYSLOT },
	{ "tgei",	"s,i16",	MIPS_REGIMM(0x08),				MA_MIPS2,	0 },
	{ "tgeiu",	"s,i16",	MIPS_REGIMM(0x09),				MA_MIPS2,	0 },
	{ "tlti",	"s,i16",	MIPS_REGIMM(0x0A),				MA_MIPS2,	0 },
	{ "tltiu",	"s,i16",	MIPS_REGIMM(0x0B),				MA_MIPS2,	0 },
	{ "teqi",	"s,i16",	MIPS_REGIMM(0x0C),				MA_MIPS2,	0 },
	{ "tnei",	"s,i16",	MIPS_REGIMM(0x0E),				MA_MIPS2,	0 },
	{ "bltzal",	"s,i16",	MIPS_REGIMM(0x10),				MA_MIPS1,	MO_IPCR|MO_DELAY|MO_NODELAYSLOT },
	{ "bgezal",	"s,i16",	MIPS_REGIMM(0x11),				MA_MIPS1,	MO_IPCR|MO_DELAY|MO_NODELAYSLOT },
	{ "bal",	"i16",		MIPS_REGIMM(0x11),				MA_MIPS1,	MO_IPCR|MO_DELAY|MO_NODELAYSLOT },
	{ "bltzall","s,i16",	MIPS_REGIMM(0x12),				MA_MIPS2,	MO_IPCR|MO_DELAY|MO_NODELAYSLOT },
	{ "bgezall","s,i16",	MIPS_REGIMM(0x13),				MA_MIPS2,	MO_IPCR|MO_DELAY|MO_NODELAYSLOT },
	{ "mtsab",	"s,i16",	MIPS_REGIMM(0x18),				MA_PS2,		0 },
	{ "mtsah",	"s,i16",	MIPS_REGIMM(0x19),				MA_PS2,		0 },

//     31---------26---------21----------------------------------------0
//     |=      COP0|    rs    |                                        |
//     -----6-------5---------------------------------------------------
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
//  00 |  MFC0 | DMFC0 |  ---  |  ---  |  MTC0 | DMTC0 |  ---  |  ---  | 00..07
//  01 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 08..0F
//  10 |FUNCT* |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 10..17
//  11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 18..1F
//  hi |-------|-------|-------|-------|-------|-------|-------|-------|
	{ "mfc0",	"t,z",		MIPS_COP0(0x00),				MA_MIPS1|MA_EXRSP,	0 },
	{ "mfc0",	"t,Rz",		MIPS_COP0(0x00),				MA_RSP,		0 },
	{ "dmfc0",	"t,z",		MIPS_COP0(0x01),				MA_MIPS3,	MO_64BIT },
	{ "mtc0",	"t,z",		MIPS_COP0(0x04),				MA_MIPS1|MA_EXRSP,	0 },
	{ "mtc0",	"t,Rz",		MIPS_COP0(0x04),				MA_RSP,		0 },
	{ "dmtc0",	"t,z",		MIPS_COP0(0x05),				MA_MIPS3,	MO_64BIT },

//     31--------------------21-------------------------------5--------0
//     |=            COP0FUNCT|                              | function|
//     -----11----------------------------------------------------6-----
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
// 000 |  ---  | TLBR  | TLBWI |  ---  |  ---  |  ---  | TLBWR |  ---  | 00..07
// 001 | TLBP  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 08..0F
// 010 | RFE   |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 10..17
// 011 | ERET  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 18..1F
// 100 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 20..27
// 101 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 28..2F
// 110 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 30..37
// 110 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 38..3F
//  hi |-------|-------|-------|-------|-------|-------|-------|-------|
	{ "tlbr",	"",			MIPS_COP0FUNCT(0x01),			MA_MIPS1|MA_EXRSP,	0 },
	{ "tlbwi",	"",			MIPS_COP0FUNCT(0x02),			MA_MIPS1|MA_EXRSP,	0 },
	{ "tlbwr",	"",			MIPS_COP0FUNCT(0x06),			MA_MIPS1|MA_EXRSP,	0 },
	{ "tlbp",	"",			MIPS_COP0FUNCT(0x08),			MA_MIPS1|MA_EXRSP,	0 },
	{ "rfe",	"",			MIPS_COP0FUNCT(0x10),			MA_PSX,		0 },
	{ "eret",	"",			MIPS_COP0FUNCT(0x18),			MA_MIPS3,	0 },

//     31---------26---------21----------------------------------------0
//     |=      COP1|    rs    |                                        |
//     -----6-------5---------------------------------------------------
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
//  00 |  MFC1 | DMFC1 |  CFC1 |  ---  |  MTC1 | DMTC1 |  CTC1 |  ---  | 00..07
//  01 |  BC*  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 08..0F
//  10 |  S*   |  ---  |  ---  |  ---  |  W*   |  ---  |  ---  |  ---  | 10..17
//  11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 18..1F
//  hi |-------|-------|-------|-------|-------|-------|-------|-------|

	{ "mfc1",	"t,S",		MIPS_COP1(0x00),				MA_MIPS1,	MO_FPU },
	{ "dmfc1",	"t,S",		MIPS_COP1(0x01),				MA_MIPS3,	MO_DFPU|MO_64BIT },
	{ "cfc1",	"t,f",		MIPS_COP1(0x02),				MA_MIPS1,	MO_FPU },
	{ "mtc1",	"t,S",		MIPS_COP1(0x04),				MA_MIPS1,	MO_FPU },
	{ "dmtc1",	"t,S",		MIPS_COP1(0x05),				MA_MIPS3,	MO_DFPU|MO_64BIT },
	{ "ctc1",	"t,f",		MIPS_COP1(0x06),				MA_MIPS1,	MO_FPU },

//     31---------26----------20-------16------------------------------0
//     |=    COP1BC|          |   rt    |                              |
//     ------11--------------5------------------------------------------
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
//  00 |  BC1F | BC1T  | BC1FL | BC1TL |  ---  |  ---  |  ---  |  ---  | 00..07
//  01 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 08..0F
//  10 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 10..17
//  11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 18..1F
//  hi |-------|-------|-------|-------|-------|-------|-------|-------|
	{ "bc1f",	"i16",		MIPS_COP1BC(0x00),				MA_MIPS1,	MO_FPU|MO_IPCR|MO_DELAY|MO_NODELAYSLOT },
	{ "bc1t",	"i16",		MIPS_COP1BC(0x01),				MA_MIPS1,	MO_FPU|MO_IPCR|MO_DELAY|MO_NODELAYSLOT },
	{ "bc1fl",	"i16",		MIPS_COP1BC(0x02),				MA_MIPS2,	MO_FPU|MO_IPCR|MO_DELAY|MO_NODELAYSLOT },
	{ "bc1tl",	"i16",		MIPS_COP1BC(0x03),				MA_MIPS2,	MO_FPU|MO_IPCR|MO_DELAY|MO_NODELAYSLOT },

//     31--------------------21-------------------------------5--------0
//     |=                COP1S|                              | function|
//     -----11----------------------------------------------------6-----
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
// 000 |  add  |  sub  |  mul  |  div  | sqrt  |  abs  |  mov  |  neg  | 00..07
// 001 |round.l|trunc.l|ceil.l |floor.l|round.w|trunc.w|ceil.w |floor.w| 08..0F
// 010 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | rsqrt |  ---  | 10..17
// 011 | adda  | suba  | mula  |  ---  | madd  | msub  | madda | msuba | 18..1F
// 100 |  ---  | cvt.d |  ---  |  ---  | cvt.w | cvt.l |  ---  |  ---  | 20..27
// 101 |  max  |  min  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 28..2F
// 110 |  c.f  | c.un  | c.eq  | c.ueq |c.(o)lt| c.ult |c.(o)le| c.ule | 30..37
// 110 |  c.sf | c.ngle| c.seq | c.ngl | c.lt  | c.nge | c.le  | c.ngt | 38..3F
//  hi |-------|-------|-------|-------|-------|-------|-------|-------|
	{ "add.s",		"D,S,T",	MIPS_COP1S(0x00),			MA_MIPS1,	MO_FPU },
	{ "add.s",		"S,T",		MIPS_COP1S(0x00),			MA_MIPS1,	MO_FPU|MO_FRSD },
	{ "sub.s",		"D,S,T",	MIPS_COP1S(0x01),			MA_MIPS1,	MO_FPU },
	{ "sub.s",		"S,T",		MIPS_COP1S(0x01),			MA_MIPS1,	MO_FPU|MO_FRSD },
	{ "mul.s",		"D,S,T",	MIPS_COP1S(0x02),			MA_MIPS1,	MO_FPU },
	{ "mul.s",		"S,T",		MIPS_COP1S(0x02),			MA_MIPS1,	MO_FPU|MO_FRSD },
	{ "div.s",		"D,S,T",	MIPS_COP1S(0x03),			MA_MIPS1,	MO_FPU },
	{ "div.s",		"S,T",		MIPS_COP1S(0x03),			MA_MIPS1,	MO_FPU|MO_FRSD },
	{ "sqrt.s",		"D,S",		MIPS_COP1S(0x04),			MA_MIPS2,	MO_FPU },
	{ "abs.s",		"D,S",		MIPS_COP1S(0x05),			MA_MIPS1,	MO_FPU },
	{ "mov.s",		"D,S",		MIPS_COP1S(0x06),			MA_MIPS1,	MO_FPU },
	{ "neg.s",		"D,S",		MIPS_COP1S(0x07),			MA_MIPS1,	MO_FPU },
	{ "round.l.s",	"D,S",		MIPS_COP1S(0x08),			MA_MIPS3,	MO_DFPU },
	{ "trunc.l.s",	"D,S",		MIPS_COP1S(0x09),			MA_MIPS3,	MO_DFPU },
	{ "ceil.l.s",	"D,S",		MIPS_COP1S(0x0A),			MA_MIPS3,	MO_DFPU },
	{ "floor.l.s",	"D,S",		MIPS_COP1S(0x0B),			MA_MIPS3,	MO_DFPU },
	{ "round.w.s",	"D,S",		MIPS_COP1S(0x0C),			MA_MIPS2|MA_EXPS2,	MO_FPU },
	{ "trunc.w.s",	"D,S",		MIPS_COP1S(0x0D),			MA_MIPS1|MA_EXPS2,	MO_FPU },
	{ "ceil.w.s",	"D,S",		MIPS_COP1S(0x0E),			MA_MIPS2|MA_EXPS2,	MO_FPU },
	{ "floor.w.s",	"D,S",		MIPS_COP1S(0x0F),			MA_MIPS2|MA_EXPS2,	MO_FPU },
	{ "rsqrt.w.s",	"D,S",		MIPS_COP1S(0x16),			MA_PS2,		0 },
	{ "adda.s",		"S,T",		MIPS_COP1S(0x18),			MA_PS2,		0 },
	{ "suba.s",		"S,T",		MIPS_COP1S(0x19),			MA_PS2,		0 },
	{ "mula.s",		"S,T",		MIPS_COP1S(0x1A),			MA_PS2,		0 },
	{ "madd.s",		"D,S,T",	MIPS_COP1S(0x1C),			MA_PS2,		0 },
	{ "madd.s",		"S,T",		MIPS_COP1S(0x1C),			MA_PS2,		MO_FRSD },
	{ "msub.s",		"D,S,T",	MIPS_COP1S(0x1D),			MA_PS2,		0 },
	{ "msub.s",		"S,T",		MIPS_COP1S(0x1D),			MA_PS2,		MO_FRSD },
	{ "madda.s",	"S,T",		MIPS_COP1S(0x1E),			MA_PS2,		0 },
	{ "msuba.s",	"S,T",		MIPS_COP1S(0x1F),			MA_PS2,		0 },
	{ "cvt.d.s",	"D,S",		MIPS_COP1S(0x21),			MA_MIPS1,	MO_DFPU },
	{ "cvt.w.s",	"D,S",		MIPS_COP1S(0x24),			MA_MIPS1,	MO_FPU },
	{ "cvt.l.s",	"D,S",		MIPS_COP1S(0x25),			MA_MIPS3,	MO_DFPU },
	{ "max.s",		"D,S,T",	MIPS_COP1S(0x28),			MA_PS2,		0 },
	{ "min.s",		"D,S,T",	MIPS_COP1S(0x29),			MA_PS2,		0 },
	{ "c.f.s",		"S,T",		MIPS_COP1S(0x30),			MA_MIPS1,	MO_FPU },
	{ "c.un.s",		"S,T",		MIPS_COP1S(0x31),			MA_MIPS1|MA_EXPS2,	MO_FPU },
	{ "c.eq.s",		"S,T",		MIPS_COP1S(0x32),			MA_MIPS1,	MO_FPU },
	{ "c.ueq.s",	"S,T",		MIPS_COP1S(0x33),			MA_MIPS1|MA_EXPS2,	MO_FPU },
	{ "c.olt.s",	"S,T",		MIPS_COP1S(0x34),			MA_MIPS1|MA_EXPS2,	MO_FPU },
	{ "c.lt.s",		"S,T",		MIPS_COP1S(0x34),			MA_PS2,		0 },
	{ "c.ult.s",	"S,T",		MIPS_COP1S(0x35),			MA_MIPS1|MA_EXPS2,	MO_FPU },
	{ "c.ole.s",	"S,T",		MIPS_COP1S(0x36),			MA_MIPS1|MA_EXPS2,	MO_FPU },
	{ "c.le.s",		"S,T",		MIPS_COP1S(0x36),			MA_PS2,		0 },
	{ "c.ule.s",	"S,T",		MIPS_COP1S(0x37),			MA_MIPS1|MA_EXPS2,	MO_FPU },
	{ "c.sf.s",		"S,T",		MIPS_COP1S(0x38),			MA_MIPS1|MA_EXPS2,	MO_FPU },
	{ "c.ngle.s",	"S,T",		MIPS_COP1S(0x39),			MA_MIPS1|MA_EXPS2,	MO_FPU },
	{ "c.seq.s",	"S,T",		MIPS_COP1S(0x3A),			MA_MIPS1|MA_EXPS2,	MO_FPU },
	{ "c.ngl.s",	"S,T",		MIPS_COP1S(0x3B),			MA_MIPS1|MA_EXPS2,	MO_FPU },
	{ "c.lt.s",		"S,T",		MIPS_COP1S(0x3C),			MA_MIPS1|MA_EXPS2,	MO_FPU },
	{ "c.nge.s",	"S,T",		MIPS_COP1S(0x3D),			MA_MIPS1|MA_EXPS2,	MO_FPU },
	{ "c.le.s",		"S,T",		MIPS_COP1S(0x3E),			MA_MIPS1|MA_EXPS2,	MO_FPU },
	{ "c.ngt.s",	"S,T",		MIPS_COP1S(0x3F),			MA_MIPS1|MA_EXPS2,	MO_FPU },

//     31--------------------21-------------------------------5--------0
//     |=                COP1D|                              | function|
//     -----11----------------------------------------------------6-----
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
// 000 |  add  |  sub  |  mul  |  div  | sqrt  |  abs  |  mov  |  neg  | 00..07
// 001 |round.l|trunc.l|ceil.l |floor.l|round.w|trunc.w|ceil.w |floor.w| 08..0F
// 010 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 10..17
// 011 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 18..1F
// 100 | cvt.s |  ---  |  ---  |  ---  | cvt.w | cvt.l |  ---  |  ---  | 20..27
// 101 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 28..2F
// 110 |  c.f  | c.un  | c.eq  | c.ueq | c.olt | c.ult | c.ole | c.ule | 30..37
// 110 |  c.sf | c.ngle| c.seq | c.ngl | c.lt  | c.nge | c.le  | c.ngt | 38..3F
//  hi |-------|-------|-------|-------|-------|-------|-------|-------|
	{ "add.d",		"D,S,T",	MIPS_COP1D(0x00),			MA_MIPS1,	MO_DFPU },
	{ "add.d",		"S,T",		MIPS_COP1D(0x00),			MA_MIPS1,	MO_DFPU|MO_FRSD },
	{ "sub.d",		"D,S,T",	MIPS_COP1D(0x01),			MA_MIPS1,	MO_DFPU },
	{ "sub.d",		"S,T",		MIPS_COP1D(0x01),			MA_MIPS1,	MO_DFPU|MO_FRSD },
	{ "mul.d",		"D,S,T",	MIPS_COP1D(0x02),			MA_MIPS1,	MO_DFPU },
	{ "mul.d",		"S,T",		MIPS_COP1D(0x02),			MA_MIPS1,	MO_DFPU|MO_FRSD },
	{ "div.d",		"D,S,T",	MIPS_COP1D(0x03),			MA_MIPS1,	MO_DFPU },
	{ "div.d",		"S,T",		MIPS_COP1D(0x03),			MA_MIPS1,	MO_DFPU|MO_FRSD },
	{ "sqrt.d",		"D,S",		MIPS_COP1D(0x04),			MA_MIPS2,	MO_DFPU },
	{ "abs.d",		"D,S",		MIPS_COP1D(0x05),			MA_MIPS1,	MO_DFPU },
	{ "mov.d",		"D,S",		MIPS_COP1D(0x06),			MA_MIPS1,	MO_DFPU },
	{ "neg.d",		"D,S",		MIPS_COP1D(0x07),			MA_MIPS1,	MO_DFPU },
	{ "round.l.d",	"D,S",		MIPS_COP1D(0x08),			MA_MIPS3,	MO_DFPU },
	{ "trunc.l.d",	"D,S",		MIPS_COP1D(0x09),			MA_MIPS3,	MO_DFPU },
	{ "ceil.l.d",	"D,S",		MIPS_COP1D(0x0A),			MA_MIPS3,	MO_DFPU },
	{ "floor.l.d",	"D,S",		MIPS_COP1D(0x0B),			MA_MIPS3,	MO_DFPU },
	{ "round.w.d",	"D,S",		MIPS_COP1D(0x0C),			MA_MIPS2,	MO_DFPU },
	{ "trunc.w.d",	"D,S",		MIPS_COP1D(0x0D),			MA_MIPS1,	MO_DFPU },
	{ "ceil.w.d",	"D,S",		MIPS_COP1D(0x0E),			MA_MIPS2,	MO_DFPU },
	{ "floor.w.d",	"D,S",		MIPS_COP1D(0x0F),			MA_MIPS2,	MO_DFPU },
	{ "cvt.s.d",	"D,S",		MIPS_COP1D(0x20),			MA_MIPS1,	MO_DFPU },
	{ "cvt.w.d",	"D,S",		MIPS_COP1D(0x24),			MA_MIPS1,	MO_DFPU },
	{ "cvt.l.d",	"D,S",		MIPS_COP1D(0x25),			MA_MIPS3,	MO_DFPU },
	{ "c.f.d",		"S,T",		MIPS_COP1D(0x30),			MA_MIPS1,	MO_DFPU },
	{ "c.un.d",		"S,T",		MIPS_COP1D(0x31),			MA_MIPS1,	MO_DFPU },
	{ "c.eq.d",		"S,T",		MIPS_COP1D(0x32),			MA_MIPS1,	MO_DFPU },
	{ "c.ueq.d",	"S,T",		MIPS_COP1D(0x33),			MA_MIPS1,	MO_DFPU },
	{ "c.olt.d",	"S,T",		MIPS_COP1D(0x34),			MA_MIPS1,	MO_DFPU },
	{ "c.ult.d",	"S,T",		MIPS_COP1D(0x35),			MA_MIPS1,	MO_DFPU },
	{ "c.ole.d",	"S,T",		MIPS_COP1D(0x36),			MA_MIPS1,	MO_DFPU },
	{ "c.ule.d",	"S,T",		MIPS_COP1D(0x37),			MA_MIPS1,	MO_DFPU },
	{ "c.sf.d",		"S,T",		MIPS_COP1D(0x38),			MA_MIPS1,	MO_DFPU },
	{ "c.ngle.d",	"S,T",		MIPS_COP1D(0x39),			MA_MIPS1,	MO_DFPU },
	{ "c.seq.d",	"S,T",		MIPS_COP1D(0x3A),			MA_MIPS1,	MO_DFPU },
	{ "c.ngl.d",	"S,T",		MIPS_COP1D(0x3B),			MA_MIPS1,	MO_DFPU },
	{ "c.lt.d",		"S,T",		MIPS_COP1D(0x3C),			MA_MIPS1,	MO_DFPU },
	{ "c.nge.d",	"S,T",		MIPS_COP1D(0x3D),			MA_MIPS1,	MO_DFPU },
	{ "c.le.d",		"S,T",		MIPS_COP1D(0x3E),			MA_MIPS1,	MO_DFPU },
	{ "c.ngt.d",	"S,T",		MIPS_COP1D(0x3F),			MA_MIPS1,	MO_DFPU },

//     31--------------------21-------------------------------5--------0
//     |=                COP1W|                              | function|
//     -----11----------------------------------------------------6-----
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
// 000 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 00..07
// 001 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 08..0F
// 010 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 10..17
// 011 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 18..1F
// 100 | cvt.s | cvt.d |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 20..27
// 101 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 28..2F
// 110 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 30..37
// 110 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 38..3F
//  hi |-------|-------|-------|-------|-------|-------|-------|-------|
	{ "cvt.s.w",	"D,S",		MIPS_COP1W(0x20),			MA_MIPS1,	MO_FPU },
	{ "cvt.d.w",	"D,S",		MIPS_COP1W(0x21),			MA_MIPS1,	MO_DFPU },

//     31--------------------21-------------------------------5--------0
//     |=                COP1L|                              | function|
//     -----11----------------------------------------------------6-----
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
// 000 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 00..07
// 001 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 08..0F
// 010 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 10..17
// 011 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 18..1F
// 100 | cvt.s | cvt.d |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 20..27
// 101 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 28..2F
// 110 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 30..37
// 110 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 38..3F
//  hi |-------|-------|-------|-------|-------|-------|-------|-------|
	{ "cvt.s.l",	"D,S",		MIPS_COP1L(0x20),			MA_MIPS3,	MO_DFPU },
	{ "cvt.d.l",	"D,S",		MIPS_COP1L(0x21),			MA_MIPS3,	MO_DFPU },

//     31---------26---------21----------------------------------------0
//     |=      COP2|    rs    |                                        |
//     -----6-------5---------------------------------------------------
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
//  00 |  MFC2 |  ---  |  CFC2 |  MFV  |  MTC2 |  ---  |  CTC2 |  MTV  |
//  01 |  BC*  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  10 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  hi |-------|-------|-------|-------|-------|-------|-------|-------|

	{ "mfc2",	"t,gs",			MIPS_COP2(0x00),			MA_PSX,		0 },
	{ "mfc2",	"t,RsRo",		MIPS_COP2(0x00),			MA_RSP,		0 },
	{ "cfc2",	"t,gc",			MIPS_COP2(0x02),			MA_PSX,		0 },
	{ "cfc2",	"t,Rc",			MIPS_COP2(0x02),			MA_RSP,		0 },
	{ "mtc2",	"t,gs",			MIPS_COP2(0x04),			MA_PSX,		0 },
	{ "mtc2",	"t,RsRo",		MIPS_COP2(0x04),			MA_RSP,		0 },
	{ "ctc2",	"t,gc",			MIPS_COP2(0x06),			MA_PSX,		0 },
	{ "ctc2",	"t,Rc",			MIPS_COP2(0x06),			MA_RSP,		0 },
	// VVVVVV VVVVV ttttt -------- C DDDDDDD
	{ "mfv",	"t,vd",			MIPS_COP2(0x03),			MA_PSP,		MO_VFPU|MO_VFPU_SINGLE },
	{ "mfvc",	"t,vc",			MIPS_COP2(0x03)|0x80,		MA_PSP,		MO_VFPU },
	{ "mtv",	"t,vd",			MIPS_COP2(0x07),			MA_PSP,		MO_VFPU|MO_VFPU_SINGLE },
	{ "mtvc",	"t,vc",			MIPS_COP2(0x07)|0x80,		MA_PSP,		MO_VFPU },


//     COP2BC: ? indicates any, * indicates all
//     31---------26----------20-------16------------------------------0
//     |=    COP2BC|          |   rt    |                              |
//     ------11---------5-----------------------------------------------
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
//  00 |  BVFx |  BVTx | BVFLx | BVTLx |  BVFy |  BVTy | BVFLy | BVTLy |
//  01 |  BVFz |  BVTz | BVFLz | BVTLz |  BVFw |  BVTw | BVFLw | BVTLw |
//  10 |  BVF? |  BVT? | BVFL? | BVTL? |  BVF* |  BVT* | BVFL* | BVTL* |
//  11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  hi |-------|-------|-------|-------|-------|-------|-------|-------|
	{ "bvf",		"jb,i16",	MIPS_COP2BC(0x00),			MA_PSP,		MO_IPCR|MO_DELAY|MO_NODELAYSLOT },
	{ "bvf.B",		"i16",		MIPS_COP2BC(0x00),			MA_PSP,		MO_IPCR|MO_DELAY|MO_NODELAYSLOT },
	{ "bvt",		"jb,i16",	MIPS_COP2BC(0x01),			MA_PSP,		MO_IPCR|MO_DELAY|MO_NODELAYSLOT },
	{ "bvt.B",		"i16",		MIPS_COP2BC(0x01),			MA_PSP,		MO_IPCR|MO_DELAY|MO_NODELAYSLOT },
	{ "bvfl",		"jb,i16",	MIPS_COP2BC(0x02),			MA_PSP,		MO_IPCR|MO_DELAY|MO_NODELAYSLOT },
	{ "bvfl.B",		"i16",		MIPS_COP2BC(0x02),			MA_PSP,		MO_IPCR|MO_DELAY|MO_NODELAYSLOT },
	{ "bvtl",		"jb,i16",	MIPS_COP2BC(0x03),			MA_PSP,		MO_IPCR|MO_DELAY|MO_NODELAYSLOT },
	{ "bvtl.B",		"i16",		MIPS_COP2BC(0x03),			MA_PSP,		MO_IPCR|MO_DELAY|MO_NODELAYSLOT },

//     31---------26-----23--------------------------------------------0
//     |= VFPU0| VOP | |
//     ------6--------3-------------------------------------------------
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
// 000 | VADD  | VSUB  | VSBN  | ---   | ---   | ---   | ---   | VDIV  | 00..07
//  hi |-------|-------|-------|-------|-------|-------|-------|-------|
	{ "vadd.S",		"vd,vs,vt",	MIPS_VFPU0(0x00),			MA_PSP,		MO_VFPU },
	{ "vsub.S",		"vd,vs,vt",	MIPS_VFPU0(0x01),			MA_PSP,		MO_VFPU },
	{ "vsbn.S",		"vd,vs,vt",	MIPS_VFPU0(0x02),			MA_PSP,		MO_VFPU },
	{ "vdiv.S",		"vd,vs,vt",	MIPS_VFPU0(0x07),			MA_PSP,		MO_VFPU },

//     31-------26-----23----------------------------------------------0
//     |=   VFPU1|  f  |                                               |
//     -----6-------3---------------------------------------------------
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
//     |  VMUL |  VDOT |  VSCL |  ---  |  VHDP |  VDET |  VCRS |  ---  |
//     |-------|-------|-------|-------|-------|-------|-------|-------|
	{ "vmul.S",		"vd,vs,vt",	MIPS_VFPU1(0),				MA_PSP,		MO_VFPU },
	{ "vdot.S",		"vd,vs,vt",	MIPS_VFPU1(1),				MA_PSP,		MO_VFPU },
	{ "vscl.S",		"vd,vs,vt",	MIPS_VFPU1(2),				MA_PSP,		MO_VFPU },
	{ "vhdp.S",		"vd,vs,vt",	MIPS_VFPU1(4),				MA_PSP,		MO_VFPU },
	{ "vdet.S",		"vd,vs,vt",	MIPS_VFPU1(5),				MA_PSP,		MO_VFPU },
	{ "vcrs.S",		"vd,vs,vt",	MIPS_VFPU1(6),				MA_PSP,		MO_VFPU },

//     31-------26-----23----------------------------------------------0
//     |=   VFPU3|  f  |                                               |
//     -----6-------3---------------------------------------------------
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
//     |  VCMP |  ---  |  VMIN |  VMAX |  ---  | VSCMP |  VSGE |  VSLT |
//     |-------|-------|-------|-------|-------|-------|-------|-------|
	// VVVVVV VVV TTTTTTT z SSSSSSS z --- CCCC
	{ "vcmp.S",		"C,vs,vt",	MIPS_VFPU3(0),				MA_PSP,		MO_VFPU },
	{ "vmin.S",		"vd,vs,vt",	MIPS_VFPU3(2),				MA_PSP,		MO_VFPU },
	{ "vmax.S",		"vd,vs,vt",	MIPS_VFPU3(3),				MA_PSP,		MO_VFPU },
	{ "vscmp.S",	"vd,vs,vt",	MIPS_VFPU3(5),				MA_PSP,		MO_VFPU },
	{ "vsge.S",		"vd,vs,vt",	MIPS_VFPU3(6),				MA_PSP,		MO_VFPU },
	{ "vslt.S",		"vd,vs,vt",	MIPS_VFPU3(7),				MA_PSP,		MO_VFPU },

//     31-------26--------------------------------------------5--------0
//     |=SPECIAL3|                                           | function|
//     -----11----------------------------------------------------6-----
//     -----6-------5---------------------------------------------------
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
// 000 |  EXT  |  ---  |  ---  |  ---  |  INS  |  ---  |  ---  |  ---  |
// 001 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
// 010 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
// 011 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
// 100 |ALLEGRE|  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
// 101 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
// 110 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
// 110 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  hi |-------|-------|-------|-------|-------|-------|-------|-------|
	{ "ext",	"t,s,i5,je",	MIPS_SPECIAL3(0),			MA_PSP,		0 },
	{ "ins",	"t,s,i5,ji",	MIPS_SPECIAL3(4),			MA_PSP,		0 },

//     31-------26----------------------------------10--------5--------0
//     |=SPECIAL3|                                 | secfunc |ALLEGREX0|
//     ------11---------5-------------------------------5---------6-----
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
//  00 |  ---  |  ---  | WSBH  | WSBW  |  ---  |  ---  |  ---  |  ---  |
//  01 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  10 |  SEB  |  ---  |  ---  |  ---  |BITREV |  ---  |  ---  |  ---  |
//  11 |  SEH  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  hi |-------|-------|-------|-------|-------|-------|-------|-------|
	// VVVVVV ----- ttttt ddddd VVVVV VVVVVV
	{ "wsbh",	"d,t",			MIPS_ALLEGREX0(0x02),		MA_PSP,		0 },
	{ "wsbh",	"d",			MIPS_ALLEGREX0(0x02),		MA_PSP,		0 },
	{ "wsbw",	"d,t",			MIPS_ALLEGREX0(0x03),		MA_PSP,		0 },
	{ "wsbw",	"d",			MIPS_ALLEGREX0(0x03),		MA_PSP,		0 },
	{ "seb",	"d,t",			MIPS_ALLEGREX0(0x10),		MA_PSP,		0 },
	{ "seb",	"d",			MIPS_ALLEGREX0(0x10),		MA_PSP,		0 },
	{ "bitrev",	"d,t",			MIPS_ALLEGREX0(0x14),		MA_PSP,		0 },
	{ "bitrev",	"d",			MIPS_ALLEGREX0(0x14),		MA_PSP,		0 },
	{ "seh",	"d,t",			MIPS_ALLEGREX0(0x18),		MA_PSP,		0 },
	{ "seh",	"d",			MIPS_ALLEGREX0(0x18),		MA_PSP,		0 },


//     VFPU4: This one is a bit messy.
//     31-------26------21---------------------------------------------0
//     |=   VFPU4|  rs  |                                              |
//     -----6-------5---------------------------------------------------
//  hi |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
//  00 |VF4-1.1|VF4-1.2|VF4-1.3| VCST  |  ---  |  ---  |  ---  |  ---  |
//  01 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  10 | VF2IN | VF2IZ | VF2IU | VF2ID | VI2F  | VCMOV |  ---  |  ---  |
//  11 | VWBN  | VWBN  | VWBN  | VWBN  | VWBN  | VWBN  | VWBN  | VWBN  |
//     |-------|-------|-------|-------|-------|-------|-------|-------|
	// VVVVVV VVVVV iiiii z ------- z DDDDDDD
	// Technically these also have names (as the second arg.)
	{ "vcst.S",		"vd,Wc",	MIPS_VFPU4(0x03),			MA_PSP,		MO_VFPU },
	{ "vf2in.S",	"vd,vs,i5",	MIPS_VFPU4(0x10),			MA_PSP,		MO_VFPU },
	{ "vf2iz.S",	"vd,vs,i5",	MIPS_VFPU4(0x11),			MA_PSP,		MO_VFPU },
	{ "vf2iu.S",	"vd,vs,i5",	MIPS_VFPU4(0x12),			MA_PSP,		MO_VFPU },
	{ "vf2id.S",	"vd,vs,i5",	MIPS_VFPU4(0x13),			MA_PSP,		MO_VFPU },
	{ "vi2f.S",		"vd,vs,i5",	MIPS_VFPU4(0x14),			MA_PSP,		MO_VFPU },
	{ "vcmovt.S",	"vd,vs,i5",	MIPS_VFPU4(0x15)|0,  		MA_PSP,		MO_VFPU },
	{ "vcmovf.S",	"vd,vs,i5",	MIPS_VFPU4(0x15)|(1<<19),	MA_PSP,		MO_VFPU },
	{ "vwbn.S",		"vd,vs,i5",	MIPS_VFPU4(0x18),			MA_PSP,		MO_VFPU },

//     31-------------21-------16--------------------------------------0
//     |= VF4-1.1      |   rt  |                                       |
//     --------11----------5--------------------------------------------
//  hi |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
//  00 | VMOV  | VABS  | VNEG  | VIDT  | vsAT0 | vsAT1 | VZERO | VONE  |
//  01 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  10 | VRCP  | VRSQ  | vsIN  | VCOS  | VEXP2 | VLOG2 | vsQRT | VASIN |
//  11 | VNRCP |  ---  | VNSIN |  ---  |VREXP2 |  ---  |  ---  |  ---  |
//     |-------|-------|-------|-------|-------|-------|-------|-------|
	{ "vmov.S",		"vd,vs",	MIPS_VFPU4_11(0x00),		MA_PSP,		MO_VFPU },
	{ "vabs.S",		"vd,vs",	MIPS_VFPU4_11(0x01),		MA_PSP,		MO_VFPU },
	{ "vneg.S",		"vd,vs",	MIPS_VFPU4_11(0x02), 		MA_PSP,		MO_VFPU },
	{ "vidt.S",		"vd",		MIPS_VFPU4_11(0x03),		MA_PSP,		MO_VFPU },
	{ "vsat0.S",	"vd,vs",	MIPS_VFPU4_11(0x04),		MA_PSP,		MO_VFPU },
	{ "vsat1.S",	"vd,vs",	MIPS_VFPU4_11(0x05),		MA_PSP,		MO_VFPU },
	{ "vzero.S",	"vd",		MIPS_VFPU4_11(0x06),		MA_PSP,		MO_VFPU },
	{ "vone.S",		"vd",		MIPS_VFPU4_11(0x07),		MA_PSP,		MO_VFPU },
	{ "vrcp.S",		"vd,vs",	MIPS_VFPU4_11(0x10),		MA_PSP,		MO_VFPU },
	{ "vrsq.S",		"vd,vs",	MIPS_VFPU4_11(0x11),		MA_PSP,		MO_VFPU },
	{ "vsin.S",		"vd,vs",	MIPS_VFPU4_11(0x12),		MA_PSP,		MO_VFPU },
	{ "vcos.S",		"vd,vs",	MIPS_VFPU4_11(0x13),		MA_PSP,		MO_VFPU },
	{ "vexp2.S",	"vd,vs",	MIPS_VFPU4_11(0x14),		MA_PSP,		MO_VFPU },
	{ "vlog2.S",	"vd,vs",	MIPS_VFPU4_11(0x15),		MA_PSP,		MO_VFPU },
	{ "vsqrt.S",	"vd,vs",	MIPS_VFPU4_11(0x16),		MA_PSP,		MO_VFPU },
	{ "vasin.S",	"vd,vs",	MIPS_VFPU4_11(0x17),		MA_PSP,		MO_VFPU },
	{ "vnrcp.S",	"vd,vs",	MIPS_VFPU4_11(0x18),		MA_PSP,		MO_VFPU },
	{ "vnsin.S",	"vd,vs",	MIPS_VFPU4_11(0x1a),		MA_PSP,		MO_VFPU },
	{ "vrexp2.S",	"vd,vs",	MIPS_VFPU4_11(0x1c),		MA_PSP,		MO_VFPU },

//     VFPU4 1.2: TODO: Unsure where vsBZ goes, no one uses it.
//     31-------------21-------16--------------------------------------0
//     |= VF4-1.2      |   rt  |                                       |
//     --------11----------5--------------------------------------------
//  hi |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
//  00 | VRNDS | VRNDI |VRNDF1 |VRNDF2 |  ---  |  ---  |  ---  |  ---  |
//  01 |  ---  |  ---  |  ---  |  ---  | vsBZ? |  ---  |  ---  |  ---  |
//  10 |  ---  |  ---  | VF2H  | VH2F  |  ---  |  ---  | vsBZ? | VLGB  |
//  11 | VUC2I | VC2I  | VUS2I | vs2I  | VI2UC | VI2C  | VI2US | VI2S  |
//     |-------|-------|-------|-------|-------|-------|-------|-------|
	{ "vrnds.S",	"vd",		MIPS_VFPU4_12(0x00),		MA_PSP,		MO_VFPU },
	{ "vrndi.S",	"vd",		MIPS_VFPU4_12(0x01),		MA_PSP,		MO_VFPU },
	{ "vrndf1.S",	"vd",		MIPS_VFPU4_12(0x02),		MA_PSP,		MO_VFPU },
	{ "vrndf2.S",	"vd",		MIPS_VFPU4_12(0x03),		MA_PSP,		MO_VFPU },
	// TODO: vsBZ?
	{ "vf2h.S",		"vd,vs",	MIPS_VFPU4_12(0x12),		MA_PSP,		MO_VFPU },
	{ "vh2f.S",		"vd,vs",	MIPS_VFPU4_12(0x13),		MA_PSP,		MO_VFPU },
	// TODO: vsBZ?
	{ "vlgb.S",		"vd,vs",	MIPS_VFPU4_12(0x17),		MA_PSP,		MO_VFPU },
	{ "vuc2i.S",	"vd,vs",	MIPS_VFPU4_12(0x18),		MA_PSP,		MO_VFPU },
	{ "vc2i.S",		"vd,vs",	MIPS_VFPU4_12(0x19),		MA_PSP,		MO_VFPU },
	{ "vus2i.S",	"vd,vs",	MIPS_VFPU4_12(0x1a),		MA_PSP,		MO_VFPU },
	{ "vs2i.S",		"vd,vs",	MIPS_VFPU4_12(0x1b),		MA_PSP,		MO_VFPU },
	{ "vi2uc.S",	"vd,vs",	MIPS_VFPU4_12(0x1c),		MA_PSP,		MO_VFPU },
	{ "vi2c.S",		"vd,vs",	MIPS_VFPU4_12(0x1d),		MA_PSP,		MO_VFPU },
	{ "vi2us.S",	"vd,vs",	MIPS_VFPU4_12(0x1e),		MA_PSP,		MO_VFPU },
	{ "vi2s.S",		"vd,vs",	MIPS_VFPU4_12(0x1f),		MA_PSP,		MO_VFPU },

//     31--------------21------16--------------------------------------0
//     |= VF4-1.3      |   rt  |                                       |
//     --------11----------5--------------------------------------------
//  hi |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
//  00 | vsRT1 | vsRT2 | VBFY1 | VBFY2 | VOCP  | vsOCP | VFAD  | VAVG  |
//  01 | vsRT3 | vsRT4 | vsGN  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  10 | VMFVC | VMTVC |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  11 |  ---  |VT4444 |VT5551 |VT5650 |  ---  |  ---  |  ---  |  ---  |
//     |-------|-------|-------|-------|-------|-------|-------|-------|
	{ "vsrt1.S",	"vd,vs",	MIPS_VFPU4_13(0x00),		MA_PSP,		MO_VFPU },
	{ "vsrt2.S",	"vd,vs",	MIPS_VFPU4_13(0x01),		MA_PSP,		MO_VFPU },
	{ "vbfy1.S",	"vd,vs",	MIPS_VFPU4_13(0x02),		MA_PSP,		MO_VFPU },
	{ "vbfy2.S",	"vd,vs",	MIPS_VFPU4_13(0x03),		MA_PSP,		MO_VFPU },
	{ "vocp.S",		"vd,vs",	MIPS_VFPU4_13(0x04),		MA_PSP,		MO_VFPU },
	{ "vsocp.S",	"vd,vs",	MIPS_VFPU4_13(0x05),		MA_PSP,		MO_VFPU },
	{ "vfad.S",		"vd,vs",	MIPS_VFPU4_13(0x06),		MA_PSP,		MO_VFPU },
	{ "vavg.S",		"vd,vs",	MIPS_VFPU4_13(0x07),		MA_PSP,		MO_VFPU },
	{ "vsrt3.S",	"vd,vs",	MIPS_VFPU4_13(0x08),		MA_PSP,		MO_VFPU },
	{ "vsrt4.S",	"vd,vs",	MIPS_VFPU4_13(0x09),		MA_PSP,		MO_VFPU },
	{ "vsgn.S",		"vd,vs",	MIPS_VFPU4_13(0x0a),		MA_PSP,		MO_VFPU },
	{ "vmfv.S",		"vs,i7",	MIPS_VFPU4_13(0x10)|0x00,	MA_PSP,		MO_VFPU },
	{ "vmtv.S",		"vs,i7",	MIPS_VFPU4_13(0x11)|0x00,	MA_PSP,		MO_VFPU },
	{ "vmfvc.S",	"vs,i7",	MIPS_VFPU4_13(0x10)|0x80,	MA_PSP,		MO_VFPU },
	{ "vmtvc.S",	"vs,i7",	MIPS_VFPU4_13(0x11)|0x80,	MA_PSP,		MO_VFPU },
	{ "vt4444.S",	"vd,vs",	MIPS_VFPU4_13(0x19),		MA_PSP,		MO_VFPU },
	{ "vt5551.S",	"vd,vs",	MIPS_VFPU4_13(0x1a),		MA_PSP,		MO_VFPU },
	{ "vt5650.S",	"vd,vs",	MIPS_VFPU4_13(0x1b),		MA_PSP,		MO_VFPU },

//     31-------26-----23----------------------------------------------0
//     |= VFPU5| f     |                                               |
//     -----6-------3---------------------------------------------------
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
//     | VPFXS | VPFXS | VPFXT | VPFXT | VPFXD | VPFXD | VIIM  | VFIM  |
//     |-------|-------|-------|-------|-------|-------|-------|-------|
	{ "vpfxs",		"Ws",		MIPS_VFPU5(0),				MA_PSP,		0 },
	{ "vpfxt",		"Ws",		MIPS_VFPU5(2),				MA_PSP,		0 },
	{ "vpfxd",		"Wd",		MIPS_VFPU5(4),				MA_PSP,		0 },
	{ "viim.s",		"vt,i16",	MIPS_VFPU5(6),				MA_PSP,		MO_VFPU_SINGLE },
	{ "vfim.s",		"vt,ih",	MIPS_VFPU5(7),				MA_PSP,		MO_VFPU_SINGLE },

//     31-------26-----23----------------------------------------------0
//     |= VFPU6| f     |                                               |
//     -----6-------3---------------------------------------------------
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
//     | VMMUL |     V(H)TFM2/3/4      | VMSCL |   *1  |  ---  |VF6-1.1|
//     |-------|-------|-------|-------|-------|-------|-------|-------|
//		*1: vcrsp.t/vqmul.q
	{ "vmmul.S",	"md,ms,mt",		MIPS_VFPU6(0),					MA_PSP,	MO_VFPU|MO_VFPU_TRANSPOSE_VS },
	{ "vtfm2.p",	"vd,ms,vt",		MIPS_VFPU6(1)|MIPS_VFPUSIZE(1),	MA_PSP,	MO_VFPU|MO_VFPU_PAIR },
	{ "vhtfm2.p",	"vd,ms,vt",		MIPS_VFPU6(2)|MIPS_VFPUSIZE(1),	MA_PSP,	MO_VFPU|MO_VFPU_PAIR },
	{ "vtfm3.t",	"vd,ms,vt",		MIPS_VFPU6(2)|MIPS_VFPUSIZE(2),	MA_PSP,	MO_VFPU|MO_VFPU_TRIPLE },
	{ "vhtfm3.t",	"vd,ms,vt",		MIPS_VFPU6(3)|MIPS_VFPUSIZE(2),	MA_PSP,	MO_VFPU|MO_VFPU_TRIPLE },
	{ "vtfm4.q",	"vd,ms,vt",		MIPS_VFPU6(3)|MIPS_VFPUSIZE(3),	MA_PSP,	MO_VFPU|MO_VFPU_QUAD },
	{ "vmscl.S",	"md,ms,vSt",	MIPS_VFPU6(4),					MA_PSP,	MO_VFPU },
	{ "vcrsp.t",	"vd,vs,vt",		MIPS_VFPU6(5)|MIPS_VFPUSIZE(2),	MA_PSP,	MO_VFPU|MO_VFPU_TRIPLE },
	{ "vqmul.q",	"vd,vs,vt",		MIPS_VFPU6(5)|MIPS_VFPUSIZE(3),	MA_PSP,	MO_VFPU|MO_VFPU_QUAD },

//     31--------23----20----------------------------------------------0
//     |= VF6-1.1 |  f |                                               |
//     -----9-------3---------------------------------------------------
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
//     |VF6-1.2|  ---  |     VROT      |  ---  |  ---  |  ---  |  ---  |
//     |-------|-------|-------|-------|-------|-------|-------|-------|
	// VVVVVVVVVVV iiiii z SSSSSSS z DDDDDDD
	{ "vrot.S",		"vd,vSs,Wr",	MIPS_VFPU6_1VROT(),		MA_PSP,		MO_VFPU },

//     31--------20----16----------------------------------------------0
//     |= VF6-1.2 |  f |                                               |
//     -----6-------4---------------------------------------------------
//  hi |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
//   0 | VMMOV |  ---  |  ---  | VMIDT |  ---  |  ---  |VMZERO | VMONE |
//   1 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//     |-------|-------|-------|-------|-------|-------|-------|-------|
	// VVVVVVVVVVVVVVVV z SSSSSSS z DDDDDDD
	{ "vmmov.S",	"md,ms",	MIPS_VFPU6_2(0),			MA_PSP,		MO_VFPU },
	// VVVVVVVVVVVVVVVV z ------- z DDDDDDD
	{ "vmidt.S",	"md",		MIPS_VFPU6_2(3),			MA_PSP,		MO_VFPU },
	{ "vmzero.S",	"md",		MIPS_VFPU6_2(6),			MA_PSP,		MO_VFPU },
	{ "vmone.S",	"md",		MIPS_VFPU6_2(7),			MA_PSP,		MO_VFPU },

//     31---------26------------------------------------------5--------0
//     |=       RSP|                                         | function|
//     ------6----------------------------------------------------6-----
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
// 000 | VMULF | VMULU | VRNDP | VMULQ | VMUDL | VMUDM | VMUDN | VMUDH | 00..07
// 001 | VMACF | VMACU | VRNDN | VMACQ | VMADL | VMADH | VMADN | VMADH | 08..0F
// 010 | VADD  | VSUB  | VSUT  | VABS  | VADDC | VSUBC | VADDB | VSUBB | 10..17
// 011 | VACCB | VSUCB | VSAD  | VSAC  | VSUM  | VSAR  | VACC  | VSUC  | 18..1F
// 100 | VLT   | VEQ   | VNE   | VGE   | VCL   | VCH   | VCR   | VMRG  | 20..27
// 101 | VAND  | VNAND | VOR   | VNOR  | VXOR  | VNXOR |  ---  |  ---  | 28..2F
// 110 | VRCP  | VRCPL | VRCPH | VMOV  | VRSQ  | VRSQL | VRSQH | VNOP  | 30..37
// 111 | VEXTT | VEXTQ | VEXTN |  ---  | VINST | VINSQ | VINSN | VNULL | 38..3F
//  hi |-------|-------|-------|-------|-------|-------|-------|-------|
	{ "vmulf",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x00),		MA_RSP,		0 },
	{ "vmulf",	"Rs,RtRe",		MIPS_RSP_COP2(0x00),		MA_RSP,		MO_RSP_VRSD },
	{ "vmulu",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x01),		MA_RSP,		0 },
	{ "vmulu",	"Rs,RtRe",		MIPS_RSP_COP2(0x01),		MA_RSP,		MO_RSP_VRSD },
	{ "vrndp",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x02),		MA_RSP,		0 },
	{ "vrndp",	"Rs,RtRe",		MIPS_RSP_COP2(0x02),		MA_RSP,		MO_RSP_VRSD },
	{ "vmulq",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x03),		MA_RSP,		0 },
	{ "vmulq",	"Rs,RtRe",		MIPS_RSP_COP2(0x03),		MA_RSP,		MO_RSP_VRSD },
	{ "vmudl",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x04),		MA_RSP,		0 },
	{ "vmudl",	"Rs,RtRe",		MIPS_RSP_COP2(0x04),		MA_RSP,		MO_RSP_VRSD },
	{ "vmudm",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x05),		MA_RSP,		0 },
	{ "vmudm",	"Rs,RtRe",		MIPS_RSP_COP2(0x05),		MA_RSP,		MO_RSP_VRSD },
	{ "vmudn",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x06),		MA_RSP,		0 },
	{ "vmudn",	"Rs,RtRe",		MIPS_RSP_COP2(0x06),		MA_RSP,		MO_RSP_VRSD },
	{ "vmudh",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x07),		MA_RSP,		0 },
	{ "vmudh",	"Rs,RtRe",		MIPS_RSP_COP2(0x07),		MA_RSP,		MO_RSP_VRSD },
	{ "vmacf",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x08),		MA_RSP,		0 },
	{ "vmacf",	"Rs,RtRe",		MIPS_RSP_COP2(0x08),		MA_RSP,		MO_RSP_VRSD },
	{ "vmacu",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x09),		MA_RSP,		0 },
	{ "vmacu",	"Rs,RtRe",		MIPS_RSP_COP2(0x09),		MA_RSP,		MO_RSP_VRSD },
	{ "vrndn",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x0a),		MA_RSP,		0 },
	{ "vrndn",	"Rs,RtRe",		MIPS_RSP_COP2(0x0a),		MA_RSP,		MO_RSP_VRSD },
	{ "vmacq",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x0b),		MA_RSP,		0 },
	{ "vmacq",	"Rs,RtRe",		MIPS_RSP_COP2(0x0b),		MA_RSP,		MO_RSP_VRSD },
	{ "vmadl",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x0c),		MA_RSP,		0 },
	{ "vmadl",	"Rs,RtRe",		MIPS_RSP_COP2(0x0c),		MA_RSP,		MO_RSP_VRSD },
	{ "vmadm",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x0d),		MA_RSP,		0 },
	{ "vmadm",	"Rs,RtRe",		MIPS_RSP_COP2(0x0d),		MA_RSP,		MO_RSP_VRSD },
	{ "vmadn",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x0e),		MA_RSP,		0 },
	{ "vmadn",	"Rs,RtRe",		MIPS_RSP_COP2(0x0e),		MA_RSP,		MO_RSP_VRSD },
	{ "vmadh",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x0f),		MA_RSP,		0 },
	{ "vmadh",	"Rs,RtRe",		MIPS_RSP_COP2(0x0f),		MA_RSP,		MO_RSP_VRSD },
	{ "vadd",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x10),		MA_RSP,		0 },
	{ "vadd",	"Rs,RtRe",		MIPS_RSP_COP2(0x10),		MA_RSP,		MO_RSP_VRSD },
	{ "vsub",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x11),		MA_RSP,		0 },
	{ "vsub",	"Rs,RtRe",		MIPS_RSP_COP2(0x11),		MA_RSP,		MO_RSP_VRSD },
	{ "vsut",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x12),		MA_RSP,		0 },
	{ "vsut",	"Rs,RtRe",		MIPS_RSP_COP2(0x12),		MA_RSP,		MO_RSP_VRSD },
	{ "vabs",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x13),		MA_RSP,		0 },
	{ "vabs",	"Rs,RtRe",		MIPS_RSP_COP2(0x13),		MA_RSP,		MO_RSP_VRSD },
	{ "vaddc",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x14),		MA_RSP,		0 },
	{ "vaddc",	"Rs,RtRe",		MIPS_RSP_COP2(0x14),		MA_RSP,		MO_RSP_VRSD },
	{ "vsubc",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x15),		MA_RSP,		0 },
	{ "vsubc",	"Rs,RtRe",		MIPS_RSP_COP2(0x15),		MA_RSP,		MO_RSP_VRSD },
	{ "vaddb",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x16),		MA_RSP,		0 },
	{ "vaddb",	"Rs,RtRe",		MIPS_RSP_COP2(0x16),		MA_RSP,		MO_RSP_VRSD },
	{ "vsubb",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x17),		MA_RSP,		0 },
	{ "vsubb",	"Rs,RtRe",		MIPS_RSP_COP2(0x17),		MA_RSP,		MO_RSP_VRSD },
	{ "vaccb",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x18),		MA_RSP,		0 },
	{ "vaccb",	"Rs,RtRe",		MIPS_RSP_COP2(0x18),		MA_RSP,		MO_RSP_VRSD },
	{ "vsucb",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x19),		MA_RSP,		0 },
	{ "vsucb",	"Rs,RtRe",		MIPS_RSP_COP2(0x19),		MA_RSP,		MO_RSP_VRSD },
	{ "vsad",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x1a),		MA_RSP,		0 },
	{ "vsad",	"Rs,RtRe",		MIPS_RSP_COP2(0x1a),		MA_RSP,		MO_RSP_VRSD },
	{ "vsac",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x1b),		MA_RSP,		0 },
	{ "vsac",	"Rs,RtRe",		MIPS_RSP_COP2(0x1b),		MA_RSP,		MO_RSP_VRSD },
	{ "vsum",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x1c),		MA_RSP,		0 },
	{ "vsum",	"Rs,RtRe",		MIPS_RSP_COP2(0x1c),		MA_RSP,		MO_RSP_VRSD },
	{ "vsar",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x1d),		MA_RSP,		0 },
	{ "vsar",	"Rs,RtRe",		MIPS_RSP_COP2(0x1d),		MA_RSP,		MO_RSP_VRSD },
	{ "vacc",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x1e),		MA_RSP,		0 },
	{ "vacc",	"Rs,RtRe",		MIPS_RSP_COP2(0x1e),		MA_RSP,		MO_RSP_VRSD },
	{ "vsuc",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x1f),		MA_RSP,		0 },
	{ "vsuc",	"Rs,RtRe",		MIPS_RSP_COP2(0x1f),		MA_RSP,		MO_RSP_VRSD },
	{ "vlt",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x20),		MA_RSP,		0 },
	{ "vlt",	"Rs,RtRe",		MIPS_RSP_COP2(0x20),		MA_RSP,		MO_RSP_VRSD },
	{ "veq",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x21),		MA_RSP,		0 },
	{ "veq",	"Rs,RtRe",		MIPS_RSP_COP2(0x21),		MA_RSP,		MO_RSP_VRSD },
	{ "vne",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x22),		MA_RSP,		0 },
	{ "vne",	"Rs,RtRe",		MIPS_RSP_COP2(0x22),		MA_RSP,		MO_RSP_VRSD },
	{ "vge",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x23),		MA_RSP,		0 },
	{ "vge",	"Rs,RtRe",		MIPS_RSP_COP2(0x23),		MA_RSP,		MO_RSP_VRSD },
	{ "vcl",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x24),		MA_RSP,		0 },
	{ "vcl",	"Rs,RtRe",		MIPS_RSP_COP2(0x24),		MA_RSP,		MO_RSP_VRSD },
	{ "vch",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x25),		MA_RSP,		0 },
	{ "vch",	"Rs,RtRe",		MIPS_RSP_COP2(0x25),		MA_RSP,		MO_RSP_VRSD },
	{ "vcr",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x26),		MA_RSP,		0 },
	{ "vcr",	"Rs,RtRe",		MIPS_RSP_COP2(0x26),		MA_RSP,		MO_RSP_VRSD },
	{ "vmrg",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x27),		MA_RSP,		0 },
	{ "vmrg",	"Rs,RtRe",		MIPS_RSP_COP2(0x27),		MA_RSP,		MO_RSP_VRSD },
	{ "vand",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x28),		MA_RSP,		0 },
	{ "vand",	"Rs,RtRe",		MIPS_RSP_COP2(0x28),		MA_RSP,		MO_RSP_VRSD },
	{ "vnand",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x29),		MA_RSP,		0 },
	{ "vnand",	"Rs,RtRe",		MIPS_RSP_COP2(0x29),		MA_RSP,		MO_RSP_VRSD },
	{ "vor",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x2a),		MA_RSP,		0 },
	{ "vor",	"Rs,RtRe",		MIPS_RSP_COP2(0x2a),		MA_RSP,		MO_RSP_VRSD },
	{ "vnor",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x2b),		MA_RSP,		0 },
	{ "vnor",	"Rs,RtRe",		MIPS_RSP_COP2(0x2b),		MA_RSP,		MO_RSP_VRSD },
	{ "vxor",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x2c),		MA_RSP,		0 },
	{ "vxor",	"Rs,RtRe",		MIPS_RSP_COP2(0x2c),		MA_RSP,		MO_RSP_VRSD },
	{ "vnxor",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x2d),		MA_RSP,		0 },
	{ "vnxor",	"Rs,RtRe",		MIPS_RSP_COP2(0x2d),		MA_RSP,		MO_RSP_VRSD },
	{ "vrcp",	"RdRm,RtRl",	MIPS_RSP_COP2(0x30),		MA_RSP,		0 },
	{ "vrcpl",	"RdRm,RtRl",	MIPS_RSP_COP2(0x31),		MA_RSP,		0 },
	{ "vrcph",	"RdRm,RtRl",	MIPS_RSP_COP2(0x32),		MA_RSP,		0 },
	{ "vmov",	"RdRm,RtRl",	MIPS_RSP_COP2(0x33),		MA_RSP,		0 },
	{ "vrsq",	"RdRm,RtRl",	MIPS_RSP_COP2(0x34),		MA_RSP,		0 },
	{ "vrsql",	"RdRm,RtRl",	MIPS_RSP_COP2(0x35),		MA_RSP,		0 },
	{ "vrsqh",	"RdRm,RtRl",	MIPS_RSP_COP2(0x36),		MA_RSP,		0 },
	{ "vnop",	"",				MIPS_RSP_COP2(0x37),		MA_RSP,		0 },
	{ "vextt",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x38),		MA_RSP,		0 },
	{ "vextt",	"Rs,RtRe",		MIPS_RSP_COP2(0x38),		MA_RSP,		MO_RSP_VRSD },
	{ "vextq",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x39),		MA_RSP,		0 },
	{ "vextq",	"Rs,RtRe",		MIPS_RSP_COP2(0x39),		MA_RSP,		MO_RSP_VRSD },
	{ "vextn",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x3a),		MA_RSP,		0 },
	{ "vextn",	"Rs,RtRe",		MIPS_RSP_COP2(0x3a),		MA_RSP,		MO_RSP_VRSD },
	{ "vinst",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x3c),		MA_RSP,		0 },
	{ "vinst",	"Rs,RtRe",		MIPS_RSP_COP2(0x3c),		MA_RSP,		MO_RSP_VRSD },
	{ "vinsq",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x3d),		MA_RSP,		0 },
	{ "vinsq",	"Rs,RtRe",		MIPS_RSP_COP2(0x3d),		MA_RSP,		MO_RSP_VRSD },
	{ "vinsn",	"Rd,Rs,RtRe",	MIPS_RSP_COP2(0x3e),		MA_RSP,		0 },
	{ "vinsn",	"Rs,RtRe",		MIPS_RSP_COP2(0x3e),		MA_RSP,		MO_RSP_VRSD },
	{ "vnull",	"",				MIPS_RSP_COP2(0x3f),		MA_RSP,		0 },

//     31---------26--------------------15-------11--------------------0
//     |=      LWC2|                    |   rd    |                    |
//     -----6----------------------5------------------------------------
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
//  00 |  LBV  |  LSV  |  LLV  |  LDV  |  LQV  |  LRV  |  LPV  |  LUV  | 00..07
//  01 |  LHV  |  LFV  |  LWV  |  LTV  |  ---  |  ---  |  ---  |  ---  | 08..0F
//  10 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 10..17
//  11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 18..1F
//  hi |-------|-------|-------|-------|-------|-------|-------|-------|
	{"lbv",		"RtRo,i7(s)",	MIPS_RSP_LWC2(0x00),		MA_RSP,		0 },
	{"lbv",		"RtRo,(s)",		MIPS_RSP_LWC2(0x00),		MA_RSP,		0 },
	{"lsv",		"RtRo,i7(s)",	MIPS_RSP_LWC2(0x01),		MA_RSP,		MO_RSP_HWOFFSET },
	{"lsv",		"RtRo,(s)",		MIPS_RSP_LWC2(0x01),		MA_RSP,		MO_RSP_HWOFFSET },
	{"llv",		"RtRo,i7(s)",	MIPS_RSP_LWC2(0x02),		MA_RSP,		MO_RSP_WOFFSET },
	{"llv",		"RtRo,(s)",		MIPS_RSP_LWC2(0x02),		MA_RSP,		MO_RSP_WOFFSET },
	{"ldv",		"RtRo,i7(s)",	MIPS_RSP_LWC2(0x03),		MA_RSP,		MO_RSP_DWOFFSET },
	{"ldv",		"RtRo,(s)",		MIPS_RSP_LWC2(0x03),		MA_RSP,		MO_RSP_DWOFFSET },
	{"lqv",		"RtRo,i7(s)",	MIPS_RSP_LWC2(0x04),		MA_RSP,		MO_RSP_QWOFFSET },
	{"lqv",		"RtRo,(s)",		MIPS_RSP_LWC2(0x04),		MA_RSP,		MO_RSP_QWOFFSET },
	{"lrv",		"RtRo,i7(s)",	MIPS_RSP_LWC2(0x05),		MA_RSP,		MO_RSP_QWOFFSET },
	{"lrv",		"RtRo,(s)",		MIPS_RSP_LWC2(0x05),		MA_RSP,		MO_RSP_QWOFFSET },
	{"lpv",		"RtRo,i7(s)",	MIPS_RSP_LWC2(0x06),		MA_RSP,		MO_RSP_DWOFFSET },
	{"lpv",		"RtRo,(s)",		MIPS_RSP_LWC2(0x06),		MA_RSP,		MO_RSP_DWOFFSET },
	{"luv",		"RtRo,i7(s)",	MIPS_RSP_LWC2(0x07),		MA_RSP,		MO_RSP_DWOFFSET },
	{"luv",		"RtRo,(s)",		MIPS_RSP_LWC2(0x07),		MA_RSP,		MO_RSP_DWOFFSET },
	{"lhv",		"RtRo,i7(s)",	MIPS_RSP_LWC2(0x08),		MA_RSP,		MO_RSP_QWOFFSET },
	{"lhv",		"RtRo,(s)",		MIPS_RSP_LWC2(0x08),		MA_RSP,		MO_RSP_QWOFFSET },
	{"lfv",		"RtRo,i7(s)",	MIPS_RSP_LWC2(0x09),		MA_RSP,		MO_RSP_QWOFFSET },
	{"lfv",		"RtRo,(s)",		MIPS_RSP_LWC2(0x09),		MA_RSP,		MO_RSP_QWOFFSET },
	{"ltv",		"RtRo,i7(s)",	MIPS_RSP_LWC2(0x0b),		MA_RSP,		MO_RSP_QWOFFSET },
	{"ltv",		"RtRo,(s)",		MIPS_RSP_LWC2(0x0b),		MA_RSP,		MO_RSP_QWOFFSET },

//     31---------26--------------------15-------11--------------------0
//     |=      SWC2|                    |   rd    |                    |
//     -----6----------------------5------------------------------------
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
//  00 |  SBV  |  SSV  |  SLV  |  SDV  |  SQV  |  SRV  |  SPV  |  SUV  | 00..07
//  01 |  SHV  |  SFV  |  SWV  |  STV  |  ---  |  ---  |  ---  |  ---  | 08..0F
//  10 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 10..17
//  11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  | 18..1F
//  hi |-------|-------|-------|-------|-------|-------|-------|-------|
	{"sbv",		"RtRo,i7(s)",	MIPS_RSP_SWC2(0x00),		MA_RSP,		0 },
	{"sbv",		"RtRo,(s)",		MIPS_RSP_SWC2(0x00),		MA_RSP,		0 },
	{"ssv",		"RtRo,i7(s)",	MIPS_RSP_SWC2(0x01),		MA_RSP,		MO_RSP_HWOFFSET },
	{"ssv",		"RtRo,(s)",		MIPS_RSP_SWC2(0x01),		MA_RSP,		MO_RSP_HWOFFSET },
	{"slv",		"RtRo,i7(s)",	MIPS_RSP_SWC2(0x02),		MA_RSP,		MO_RSP_WOFFSET },
	{"slv",		"RtRo,(s)",		MIPS_RSP_SWC2(0x02),		MA_RSP,		MO_RSP_WOFFSET },
	{"sdv",		"RtRo,i7(s)",	MIPS_RSP_SWC2(0x03),		MA_RSP,		MO_RSP_DWOFFSET },
	{"sdv",		"RtRo,(s)",		MIPS_RSP_SWC2(0x03),		MA_RSP,		MO_RSP_DWOFFSET },
	{"sqv",		"RtRo,i7(s)",	MIPS_RSP_SWC2(0x04),		MA_RSP,		MO_RSP_QWOFFSET },
	{"sqv",		"RtRo,(s)",		MIPS_RSP_SWC2(0x04),		MA_RSP,		MO_RSP_QWOFFSET },
	{"srv",		"RtRo,i7(s)",	MIPS_RSP_SWC2(0x05),		MA_RSP,		MO_RSP_QWOFFSET },
	{"srv",		"RtRo,(s)",		MIPS_RSP_SWC2(0x05),		MA_RSP,		MO_RSP_QWOFFSET },
	{"spv",		"RtRo,i7(s)",	MIPS_RSP_SWC2(0x06),		MA_RSP,		MO_RSP_DWOFFSET },
	{"spv",		"RtRo,(s)",		MIPS_RSP_SWC2(0x06),		MA_RSP,		MO_RSP_DWOFFSET },
	{"suv",		"RtRo,i7(s)",	MIPS_RSP_SWC2(0x07),		MA_RSP,		MO_RSP_DWOFFSET },
	{"suv",		"RtRo,(s)",		MIPS_RSP_SWC2(0x07),		MA_RSP,		MO_RSP_DWOFFSET },
	{"shv",		"RtRo,i7(s)",	MIPS_RSP_SWC2(0x08),		MA_RSP,		MO_RSP_QWOFFSET },
	{"shv",		"RtRo,(s)",		MIPS_RSP_SWC2(0x08),		MA_RSP,		MO_RSP_QWOFFSET },
	{"sfv",		"RtRo,i7(s)",	MIPS_RSP_SWC2(0x09),		MA_RSP,		MO_RSP_QWOFFSET },
	{"sfv",		"RtRo,(s)",		MIPS_RSP_SWC2(0x09),		MA_RSP,		MO_RSP_QWOFFSET },
	{"swv",		"RtRo,i7(s)",	MIPS_RSP_SWC2(0x0a),		MA_RSP,		MO_RSP_QWOFFSET },
	{"swv",		"RtRo,(s)",		MIPS_RSP_SWC2(0x0a),		MA_RSP,		MO_RSP_QWOFFSET },
	{"stv",		"RtRo,i7(s)",	MIPS_RSP_SWC2(0x0b),		MA_RSP,		MO_RSP_QWOFFSET },
	{"stv",		"RtRo,(s)",		MIPS_RSP_SWC2(0x0b),		MA_RSP,		MO_RSP_QWOFFSET },
	// END
	{ nullptr,	nullptr,		0,							0,			0 },
};

const MipsArchDefinition mipsArchs[] = {
	// MARCH_PSX
	{ "PSX",		MA_MIPS1|MA_PSX,					MA_EXPSX,	0 },
	// MARCH_N64
	{ "N64",		MA_MIPS1|MA_MIPS2|MA_MIPS3,			MA_EXN64,	MO_64BIT|MO_FPU|MO_DFPU },
	// MARCH_PS2
	{ "PS2",		MA_MIPS1|MA_MIPS2|MA_MIPS3|MA_PS2,	MA_EXPS2,	MO_64BIT|MO_FPU },
	// MARCH_PSP
	{ "PSP",		MA_MIPS1|MA_MIPS2|MA_MIPS3|MA_PSP,	MA_EXPSP,	MO_FPU },
	// MARCH_RSP
	{ "RSP",		MA_MIPS1|MA_RSP,					MA_EXRSP,	0 },
	// MARCH_INVALID
	{ "Invalid",	0,									0,			0 },
};

// file: Parser/ExpressionParser.h

class Expression;
class Tokenizer;

Expression parseExpression(Tokenizer& tokenizer, bool inUnknownOrFalseBlock);
void allowFunctionCallExpression(bool allow);

// file: Archs/MIPS/PsxRelocator.h


#include <memory>
#include <string>
#include <vector>

class Label;
class MipsElfRelocator;

enum class PsxRelocationType { WordLiteral, UpperImmediate, LowerImmediate, FunctionCall };
enum class PsxRelocationRefType { SymblId, SegmentOffset };

struct PsxRelocation
{
	PsxRelocationType type;
	PsxRelocationRefType refType;
	int segmentOffset = 0;
	int referenceId = 0;
	int referencePos = 0;
	int relativeOffset = 0;
	int filePos = 0;
};

struct PsxSegment
{
	std::wstring name;
	int id;
	ByteArray data;
	std::vector<PsxRelocation> relocations;
};


enum class PsxSymbolType { Internal, InternalID, External, BSS, Function };

struct PsxSymbol
{
	PsxSymbolType type;
	std::wstring name;
	int segment;
	int offset;
	int id;
	int size;
	std::shared_ptr<Label> label;
};

struct PsxRelocatorFile
{
	std::wstring name;
	std::vector<PsxSegment> segments;
	std::vector<PsxSymbol> symbols;
};

class PsxRelocator
{
public:
	bool init(const fs::path& inputName);
	bool relocate(int& memoryAddress);
	bool hasDataChanged() { return dataChanged; };
	const ByteArray& getData() const { return outputData; };
	void writeSymbols(SymbolData& symData) const;
private:
	size_t loadString(ByteArray& data, size_t pos, std::wstring& dest);
	bool parseObject(ByteArray data, PsxRelocatorFile& dest);
	bool relocateFile(PsxRelocatorFile& file, int& relocationAddress);

	ByteArray outputData;
	std::vector<PsxRelocatorFile> files;
	MipsElfRelocator* reloc;
	bool dataChanged;
};

class DirectivePsxObjImport: public CAssemblerCommand
{
public:
	DirectivePsxObjImport(const fs::path& fileName);
	~DirectivePsxObjImport() { };
	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override { };
	void writeSymData(SymbolData& symData) const override;
private:
	PsxRelocator rel;
};

// file: Commands/CDirectiveFile.h


class AssemblerFile;
class GenericAssemblerFile;

class CDirectiveFile: public CAssemblerCommand
{
public:
	enum class Type { Invalid, Open, Create, Copy, Close };

	CDirectiveFile();
	void initOpen(const fs::path& fileName, int64_t memory);
	void initCreate(const fs::path& fileName, int64_t memory);
	void initCopy(const fs::path& inputName, const fs::path& outputName, int64_t memory);
	void initClose();

	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
	void writeSymData(SymbolData& symData) const override;
private:
	Type type;
	int64_t virtualAddress;
	std::shared_ptr<GenericAssemblerFile> file;
	std::shared_ptr<AssemblerFile> closeFile;
};

class CDirectivePosition: public CAssemblerCommand
{
public:
	enum Type { Physical, Virtual };
	CDirectivePosition(Expression value, Type type);
	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
	void writeSymData(SymbolData& symData) const override { };
private:
	void exec() const;
	Expression expression;
	Type type;
	int64_t position;
	int64_t virtualAddress;
};

class CDirectiveIncbin: public CAssemblerCommand
{
public:
	CDirectiveIncbin(const fs::path& fileName);
	void setStart(Expression& exp) { startExpression = exp; };
	void setSize(Expression& exp) { sizeExpression = exp; };

	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
	void writeSymData(SymbolData& symData) const override;
private:
	fs::path fileName;
	int64_t fileSize;

	Expression startExpression;
	Expression sizeExpression;
	int64_t size;
	int64_t start;
	int64_t virtualAddress;
};

class CDirectiveAlignFill: public CAssemblerCommand
{
public:
	enum Mode { AlignPhysical, AlignVirtual, Fill };

	CDirectiveAlignFill(int64_t value, Mode mode);
	CDirectiveAlignFill(Expression& value, Mode mode);
	CDirectiveAlignFill(Expression& value, Expression& fillValue, Mode mode);
	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
	void writeSymData(SymbolData& symData) const override;
private:
	Mode mode;
	Expression valueExpression;
	Expression fillExpression;
	int64_t value;
	int64_t finalSize;
	int8_t fillByte;
	int64_t virtualAddress;
};

class CDirectiveSkip: public CAssemblerCommand
{
public:
	CDirectiveSkip(Expression& value);
	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
	void writeSymData(SymbolData& symData) const override { };
private:
	Expression expression;
	int64_t value;
	int64_t virtualAddress;
};

class CDirectiveHeaderSize: public CAssemblerCommand
{
public:
	CDirectiveHeaderSize(Expression expression);
	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
	void writeSymData(SymbolData& symData) const override { };
private:
	void exec() const;
	Expression expression;
	int64_t headerSize;
	int64_t virtualAddress;
};

class DirectiveObjImport: public CAssemblerCommand
{
public:
	DirectiveObjImport(const fs::path& inputName);
	DirectiveObjImport(const fs::path& inputName, const std::wstring& ctorName);
	~DirectiveObjImport() { };
	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
	void writeSymData(SymbolData& symData) const override;
private:
	ElfRelocator rel;
	std::unique_ptr<CAssemblerCommand> ctor;
};

// file: Archs/MIPS/MipsParser.cpp


#define CHECK(exp) if (!(exp)) return false;

const MipsRegisterDescriptor mipsRegisters[] = {
	{ L"r0", 0 },		{ L"zero", 0},		{ L"at", 1 },		{ L"r1", 1 },
	{ L"v0", 2 },		{ L"r2", 2 },		{ L"v1", 3 },		{ L"r3", 3 },
	{ L"a0", 4 },		{ L"r4", 4 },		{ L"a1", 5 },		{ L"r5", 5 },
	{ L"a2", 6 },		{ L"r6", 6 },		{ L"a3", 7 },		{ L"r7", 7 },
	{ L"t0", 8 },		{ L"r8", 8 },		{ L"t1", 9 },		{ L"r9", 9 },
	{ L"t2", 10 },		{ L"r10", 10 },		{ L"t3", 11 },		{ L"r11", 11 },
	{ L"t4", 12 },		{ L"r12", 12 },		{ L"t5", 13 },		{ L"r13", 13 },
	{ L"t6", 14 },		{ L"r14", 14 },		{ L"t7", 15 },		{ L"r15", 15 },
	{ L"s0", 16 },		{ L"r16", 16 },		{ L"s1", 17 },		{ L"r17", 17 },
	{ L"s2", 18 },		{ L"r18", 18 },		{ L"s3", 19 },		{ L"r19", 19 },
	{ L"s4", 20 },		{ L"r20", 20 },		{ L"s5", 21 },		{ L"r21", 21 },
	{ L"s6", 22 },		{ L"r22", 22 },		{ L"s7", 23 },		{ L"r23", 23 },
	{ L"t8", 24 },		{ L"r24", 24 },		{ L"t9", 25 },		{ L"r25", 25 },
	{ L"k0", 26 },		{ L"r26", 26 },		{ L"k1", 27 },		{ L"r27", 27 },
	{ L"gp", 28 },		{ L"r28", 28 },		{ L"sp", 29 },		{ L"r29", 29 },
	{ L"fp", 30 },		{ L"r30", 30 },		{ L"ra", 31 },		{ L"r31", 31 },
	{ L"s8", 30 },
};

const MipsRegisterDescriptor mipsFloatRegisters[] = {
	{ L"f0", 0 },		{ L"f1", 1 },		{ L"f2", 2 },		{ L"f3", 3 },
	{ L"f4", 4 },		{ L"f5", 5 },		{ L"f6", 6 },		{ L"f7", 7 },
	{ L"f8", 8 },		{ L"f9", 9 },		{ L"f00", 0 },		{ L"f01", 1 },
	{ L"f02", 2 },		{ L"f03", 3 },		{ L"f04", 4 },		{ L"f05", 5 },
	{ L"f06", 6 },		{ L"f07", 7 },		{ L"f08", 8 },		{ L"f09", 9 },
	{ L"f10", 10 },		{ L"f11", 11 },		{ L"f12", 12 },		{ L"f13", 13 },
	{ L"f14", 14 },		{ L"f15", 15 },		{ L"f16", 16 },		{ L"f17", 17 },
	{ L"f18", 18 },		{ L"f19", 19 },		{ L"f20", 20 },		{ L"f21", 21 },
	{ L"f22", 22 },		{ L"f23", 23 },		{ L"f24", 24 },		{ L"f25", 25 },
	{ L"f26", 26 },		{ L"f27", 27 },		{ L"f28", 28 },		{ L"f29", 29 },
	{ L"f30", 30 },		{ L"f31", 31 },
};

const MipsRegisterDescriptor mipsFpuControlRegisters[] = {
	{ L"fir", 0 },		{ L"fcr0", 0 },		{ L"fcsr", 31 },	{ L"fcr31", 31 },
};

const MipsRegisterDescriptor mipsCop0Registers[] = {
	{ L"index", 0},			{ L"random", 1 }, 		{ L"entrylo", 2 },
	{ L"entrylo0", 2 },		{ L"entrylo1", 3 },		{ L"context", 4 },
	{ L"pagemask", 5 },		{ L"wired", 6 },		{ L"badvaddr", 8 },
	{ L"count", 9 },		{ L"entryhi", 10 },		{ L"compare", 11 },
	{ L"status", 12 },		{ L"sr", 12 },			{ L"cause", 13 },
	{ L"epc", 14 },			{ L"prid", 15 },		{ L"config", 16 },
	{ L"lladdr", 17 },		{ L"watchlo", 18 },		{ L"watchhi", 19 },
	{ L"xcontext", 20 },	{ L"badpaddr", 23 },	{ L"ecc", 26 },
	{ L"perr", 26},			{ L"cacheerr", 27 },	{ L"taglo", 28 },
	{ L"taghi", 29 },		{ L"errorepc", 30 },
};

const MipsRegisterDescriptor mipsPs2Cop2FpRegisters[] = {
	{ L"vf0", 0 },		{ L"vf1", 1 },		{ L"vf2", 2 },		{ L"vf3", 3 },
	{ L"vf4", 4 },		{ L"vf5", 5 },		{ L"vf6", 6 },		{ L"vf7", 7 },
	{ L"vf8", 8 },		{ L"vf9", 9 },		{ L"vf00", 0 },		{ L"vf01", 1 },
	{ L"vf02", 2 },		{ L"vf03", 3 },		{ L"vf04", 4 },		{ L"vf05", 5 },
	{ L"vf06", 6 },		{ L"vf07", 7 },		{ L"vf08", 8 },		{ L"vf09", 9 },
	{ L"vf10", 10 },	{ L"vf11", 11 },	{ L"vf12", 12 },	{ L"vf13", 13 },
	{ L"vf14", 14 },	{ L"vf15", 15 },	{ L"vf16", 16 },	{ L"vf17", 17 },
	{ L"vf18", 18 },	{ L"vf19", 19 },	{ L"vf20", 20 },	{ L"vf21", 21 },
	{ L"vf22", 22 },	{ L"vf23", 23 },	{ L"vf24", 24 },	{ L"vf25", 25 },
	{ L"vf26", 26 },	{ L"vf27", 27 },	{ L"vf28", 28 },	{ L"vf29", 29 },
	{ L"vf30", 30 },	{ L"vf31", 31 },
};

const MipsRegisterDescriptor mipsPsxCop2DataRegisters[] = {
	{ L"vxy0", 0 },		{ L"vz0", 1 },		{ L"vxy1", 2 },		{ L"vz1", 3 },
	{ L"vxy2", 4 },		{ L"vz2", 5 },		{ L"rgbc", 6 },		{ L"otz", 7 },
	{ L"ir0", 8 },		{ L"ir1", 9 },		{ L"ir2", 10 },		{ L"ir3", 11 },
	{ L"sxy0", 12 },	{ L"sxy1", 13 },	{ L"sxy2", 14 },	{ L"sxyp", 15 },
	{ L"sz0", 16 },		{ L"sz1", 17 },		{ L"sz2", 18 },		{ L"sz3", 19 },
	{ L"rgb0", 20 },	{ L"rgb1", 21 },	{ L"rgb2", 22 },	{ L"res1", 23 },
	{ L"mac0", 24 },	{ L"mac1", 25 },	{ L"mac2", 26 },	{ L"mac3", 27 },
	{ L"irgb", 28 },	{ L"orgb", 29 },	{ L"lzcs", 30 },	{ L"lzcr", 31 },
};

const MipsRegisterDescriptor mipsPsxCop2ControlRegisters[] = {
	{ L"rt0", 0 },		{ L"rt1", 1 },		{ L"rt2", 2 },		{ L"rt3", 3 },
	{ L"rt4", 4 },		{ L"trx", 5 },		{ L"try", 6 },		{ L"trz", 7 },
	{ L"llm0", 8 },		{ L"llm1", 9 },		{ L"llm2", 10 },	{ L"llm3", 11 },
	{ L"llm4", 12 },	{ L"rbk", 13 },		{ L"gbk", 14 },		{ L"bbk", 15 },
	{ L"lcm0", 16 },	{ L"lcm1", 17 },	{ L"lcm2", 18 },	{ L"lcm3", 19 },
	{ L"lcm4", 20 },	{ L"rfc", 21 },		{ L"gfc", 22 },		{ L"bfc", 23 },
	{ L"ofx", 24 },		{ L"ofy", 25 },		{ L"h", 26 },		{ L"dqa", 27 },
	{ L"dqb", 28 },		{ L"zsf3", 29 },	{ L"zsf4", 30 },	{ L"flag", 31 },
};

const MipsRegisterDescriptor mipsRspCop0Registers[] = {
	{ L"sp_mem_addr", 0 },	{ L"sp_dram_addr", 1 }, { L"sp_rd_len", 2 },
	{ L"sp_wr_len", 3 },	{ L"sp_status", 4 },	{ L"sp_dma_full", 5 },
	{ L"sp_dma_busy", 6 },	{ L"sp_semaphore", 7 },	{ L"dpc_start", 8 },
	{ L"dpc_end", 9 },		{ L"dpc_current", 10 },	{ L"dpc_status", 11 },
	{ L"dpc_clock", 12 },	{ L"dpc_bufbusy", 13 },	{ L"dpc_pipebusy", 14 },
	{ L"dpc_tmem", 15 },
};

const MipsRegisterDescriptor mipsRspVectorControlRegisters[] = {
	{ L"vco", 0 },		{ L"vcc", 1 }, 		{ L"vce", 2 },
};

const MipsRegisterDescriptor mipsRspVectorRegisters[] = {
	{ L"v0", 0 },		{ L"v1", 1 },		{ L"v2", 2 },		{ L"v3", 3 },
	{ L"v4", 4 },		{ L"v5", 5 },		{ L"v6", 6 },		{ L"v7", 7 },
	{ L"v8", 8 },		{ L"v9", 9 },		{ L"v00", 0 },		{ L"v01", 1 },
	{ L"v02", 2 },		{ L"v03", 3 },		{ L"v04", 4 },		{ L"v05", 5 },
	{ L"v06", 6 },		{ L"v07", 7 },		{ L"v08", 8 },		{ L"v09", 9 },
	{ L"v10", 10 },		{ L"v11", 11 },		{ L"v12", 12 },		{ L"v13", 13 },
	{ L"v14", 14 },		{ L"v15", 15 },		{ L"v16", 16 },		{ L"v17", 17 },
	{ L"v18", 18 },		{ L"v19", 19 },		{ L"v20", 20 },		{ L"v21", 21 },
	{ L"v22", 22 },		{ L"v23", 23 },		{ L"v24", 24 },		{ L"v25", 25 },
	{ L"v26", 26 },		{ L"v27", 27 },		{ L"v28", 28 },		{ L"v29", 29 },
	{ L"v30", 30 },		{ L"v31", 31 },
};

std::unique_ptr<CAssemblerCommand> parseDirectiveResetDelay(Parser& parser, int flags)
{
	Mips.SetIgnoreDelay(true);
	return std::make_unique<DummyCommand>();
}

std::unique_ptr<CAssemblerCommand> parseDirectiveFixLoadDelay(Parser& parser, int flags)
{
	Mips.SetFixLoadDelay(true);
	return std::make_unique<DummyCommand>();
}

std::unique_ptr<CAssemblerCommand> parseDirectiveLoadElf(Parser& parser, int flags)
{
	std::vector<Expression> list;
	if (!parser.parseExpressionList(list,1,2))
		return nullptr;

	std::wstring inputName, outputName;
	if (!list[0].evaluateString(inputName,true))
		return nullptr;

	if (list.size() == 2)
	{
		if (!list[1].evaluateString(outputName,true))
			return nullptr;
		return std::make_unique<DirectiveLoadMipsElf>(inputName,outputName);
	} else {
		return std::make_unique<DirectiveLoadMipsElf>(inputName);
	}
}

std::unique_ptr<CAssemblerCommand> parseDirectiveImportObj(Parser& parser, int flags)
{
	const Token& start = parser.peekToken();

	std::vector<Expression> list;
	if (!parser.parseExpressionList(list,1,2))
		return nullptr;

	std::wstring inputName;
	if (!list[0].evaluateString(inputName,true))
		return nullptr;

	if (list.size() == 2)
	{
		std::wstring ctorName;
		if (!list[1].evaluateIdentifier(ctorName))
			return nullptr;

		if (Mips.GetVersion() == MARCH_PSX)
		{
			parser.printError(start,L"Constructor not supported for PSX libraries");
			return std::make_unique<InvalidCommand>();
		}

		return std::make_unique<DirectiveObjImport>(inputName,ctorName);
	}

	if (Mips.GetVersion() == MARCH_PSX)
		return std::make_unique<DirectivePsxObjImport>(inputName);
	else
		return std::make_unique<DirectiveObjImport>(inputName);
}

const DirectiveMap mipsDirectives = {
	{ L".resetdelay",		{ &parseDirectiveResetDelay,	0 } },
	{ L".fixloaddelay",		{ &parseDirectiveFixLoadDelay,	0 } },
	{ L".loadelf",			{ &parseDirectiveLoadElf,		0 } },
	{ L".importobj",		{ &parseDirectiveImportObj,		0 } },
	{ L".importlib",		{ &parseDirectiveImportObj,		0 } },
};

std::unique_ptr<CAssemblerCommand> MipsParser::parseDirective(Parser& parser)
{
	return parser.parseDirective(mipsDirectives);
}

bool MipsParser::parseRegisterNumber(Parser& parser, MipsRegisterValue& dest, int numValues)
{
	// check for $0 and $1
	if (parser.peekToken().type == TokenType::Dollar)
	{
		const Token& number = parser.peekToken(1);
		if (number.type == TokenType::Integer && number.intValue < numValues)
		{
			dest.name = tfm::format(L"$%d", number.intValue);
			dest.num = (int) number.intValue;

			parser.eatTokens(2);
			return true;
		}
	}

	return false;
}

bool MipsParser::parseRegisterTable(Parser& parser, MipsRegisterValue& dest, const MipsRegisterDescriptor* table, size_t count)
{
	int offset = 0;
	bool hasDollar = parser.peekToken().type == TokenType::Dollar;
	if (hasDollar)
		offset = 1;

	const Token &token = parser.peekToken(offset);

	if (token.type != TokenType::Identifier)
		return false;

	const std::wstring stringValue = token.getStringValue();
	for (size_t i = 0; i < count; i++)
	{
		if (stringValue == table[i].name)
		{
			dest.name = stringValue;
			dest.num = table[i].num;
			parser.eatTokens(hasDollar ? 2 : 1);
			return true;
		}
	}

	return false;
}

bool MipsParser::parseRegister(Parser& parser, MipsRegisterValue& dest)
{
	dest.type = MipsRegisterType::Normal;

	if (parseRegisterNumber(parser, dest, 32))
		return true;

	return parseRegisterTable(parser,dest,mipsRegisters, std::size(mipsRegisters));
}

bool MipsParser::parseFpuRegister(Parser& parser, MipsRegisterValue& dest)
{
	dest.type = MipsRegisterType::Float;

	if (parseRegisterNumber(parser, dest, 32))
		return true;

	return parseRegisterTable(parser,dest,mipsFloatRegisters, std::size(mipsFloatRegisters));
}

bool MipsParser::parseFpuControlRegister(Parser& parser, MipsRegisterValue& dest)
{
	dest.type = MipsRegisterType::FpuControl;

	if (parseRegisterNumber(parser, dest, 32))
		return true;

	return parseRegisterTable(parser,dest,mipsFpuControlRegisters, std::size(mipsFpuControlRegisters));
}

bool MipsParser::parseCop0Register(Parser& parser, MipsRegisterValue& dest)
{
	dest.type = MipsRegisterType::Cop0;

	if (parseRegisterNumber(parser, dest, 32))
		return true;

	return parseRegisterTable(parser,dest,mipsCop0Registers, std::size(mipsCop0Registers));
}

bool MipsParser::parsePs2Cop2Register(Parser& parser, MipsRegisterValue& dest)
{
	dest.type = MipsRegisterType::Ps2Cop2;
	return parseRegisterTable(parser,dest,mipsPs2Cop2FpRegisters, std::size(mipsPs2Cop2FpRegisters));
}

bool MipsParser::parsePsxCop2DataRegister(Parser& parser, MipsRegisterValue& dest)
{
	dest.type = MipsRegisterType::PsxCop2Data;

	if (parseRegisterNumber(parser, dest, 32))
		return true;

	return parseRegisterTable(parser,dest,mipsPsxCop2DataRegisters, std::size(mipsPsxCop2DataRegisters));
}

bool MipsParser::parsePsxCop2ControlRegister(Parser& parser, MipsRegisterValue& dest)
{
	dest.type = MipsRegisterType::PsxCop2Control;

	if (parseRegisterNumber(parser, dest, 32))
		return true;

	return parseRegisterTable(parser,dest,mipsPsxCop2ControlRegisters, std::size(mipsPsxCop2ControlRegisters));
}

bool MipsParser::parseRspCop0Register(Parser& parser, MipsRegisterValue& dest)
{
	dest.type = MipsRegisterType::RspCop0;

	if (parseRegisterNumber(parser, dest, 32))
		return true;

	return parseRegisterTable(parser,dest,mipsRspCop0Registers, std::size(mipsRspCop0Registers));
}

bool MipsParser::parseRspVectorControlRegister(Parser& parser, MipsRegisterValue& dest)
{
	dest.type = MipsRegisterType::RspVectorControl;

	if (parseRegisterNumber(parser, dest, 32))
		return true;

	return parseRegisterTable(parser,dest,mipsRspVectorControlRegisters, std::size(mipsRspVectorControlRegisters));
}

bool MipsParser::parseRspVectorRegister(Parser& parser, MipsRegisterValue& dest)
{
	dest.type = MipsRegisterType::RspVector;
	return parseRegisterTable(parser,dest,mipsRspVectorRegisters, std::size(mipsRspVectorRegisters));
}

bool MipsParser::parseRspVectorElement(Parser& parser, MipsRegisterValue& dest)
{
	dest.type = MipsRegisterType::RspVectorElement;

	if (parser.peekToken().type == TokenType::LBrack)
	{
		static const MipsRegisterDescriptor rspElementNames[] = {
			{ L"0q", 2 },		{ L"1q", 3 },		{ L"0h", 4 },		{ L"1h", 5 },
			{ L"2h", 6 },		{ L"3h", 7 },		{ L"0w", 8 },		{ L"0", 8 },
			{ L"1w", 9 },		{ L"1", 9 },		{ L"2w", 10 },		{ L"2", 10 },
			{ L"3w", 11 },		{ L"3", 11 },		{ L"4w", 12 },		{ L"4", 12 },
			{ L"5w", 13 },		{ L"5", 13 },		{ L"6w", 14 },		{ L"6", 14 },
			{ L"7w", 15 },		{ L"7", 15 },
		};

		parser.eatToken();

		if (parseRegisterNumber(parser, dest, 16))
			return parser.nextToken().type == TokenType::RBrack;

		const Token& token = parser.nextToken();

		if (token.type != TokenType::Integer && token.type != TokenType::NumberString)
			return false;

		//ignore the numerical values, just use the original text as an identifier
		std::wstring stringValue = token.getOriginalText();
		if (std::any_of(stringValue.begin(), stringValue.end(), iswupper))
		{
			std::transform(stringValue.begin(), stringValue.end(), stringValue.begin(), towlower);
		}

		for (size_t i = 0; i < std::size(rspElementNames); i++)
		{
			if (stringValue == rspElementNames[i].name)
			{
				dest.num = rspElementNames[i].num;
				dest.name = rspElementNames[i].name;

				return parser.nextToken().type == TokenType::RBrack;
			}
		}

		return false;
	}

	dest.num = 0;
	dest.name = L"";

	return true;

}

bool MipsParser::parseRspScalarElement(Parser& parser, MipsRegisterValue& dest)
{
	dest.type = MipsRegisterType::RspScalarElement;

	if (parser.nextToken().type != TokenType::LBrack)
		return false;

	const Token &token = parser.nextToken();

	if (token.type != TokenType::Integer || token.intValue >= 8)
		return false;

	dest.name = tfm::format(L"%d", token.intValue);
	dest.num = (int)token.intValue + 8;

	return parser.nextToken().type == TokenType::RBrack;
}

bool MipsParser::parseRspOffsetElement(Parser& parser, MipsRegisterValue& dest)
{
	dest.type = MipsRegisterType::RspOffsetElement;

	if (parser.peekToken().type == TokenType::LBrack)
	{
		parser.eatToken();

		const Token &token = parser.nextToken();

		if (token.type != TokenType::Integer || token.intValue >= 16)
			return false;

		dest.name = tfm::format(L"%d", token.intValue);
		dest.num = (int)token.intValue;

		return parser.nextToken().type == TokenType::RBrack;
	}

	dest.num = 0;
	dest.name = L"";

	return true;
}

static bool decodeDigit(wchar_t digit, int& dest)
{
	if (digit >= '0' && digit <= '9')
	{
		dest = digit-'0';
		return true;
	}
	return false;
}

bool MipsParser::parseVfpuRegister(Parser& parser, MipsRegisterValue& reg, int size)
{
	const Token& token = parser.peekToken();
	const std::wstring stringValue = token.getStringValue();
	if (token.type != TokenType::Identifier || stringValue.size() != 4)
		return false;

	int mtx,col,row;
	if (!decodeDigit(stringValue[1],mtx)) return false;
	if (!decodeDigit(stringValue[2],col)) return false;
	if (!decodeDigit(stringValue[3],row)) return false;
	wchar_t mode = towlower(stringValue[0]);

	if (size < 0 || size > 3)
		return false;

	if (row > 3 || col > 3 || mtx > 7)
		return false;

	reg.num = 0;
	switch (mode)
	{
	case 'r':					// transposed vector
		reg.num |= (1 << 5);
		std::swap(col,row);
		[[fallthrough]];
	case 'c':					// vector
		reg.type = MipsRegisterType::VfpuVector;

		switch (size)
		{
		case 1:	// pair
		case 3: // quad
			if (row & 1)
				return false;
			break;
		case 2:	// triple
			if (row & 2)
				return false;
			row <<= 1;
			break;
		default:
			return false;
		}
		break;
	case 's':					// single
		reg.type = MipsRegisterType::VfpuVector;

		if (size != 0)
			return false;
		break;
	case 'e':					// transposed matrix
		reg.num |= (1 << 5);
		[[fallthrough]];
	case 'm':					// matrix
		reg.type = MipsRegisterType::VfpuMatrix;

		// check size
		switch (size)
		{
		case 1:	// 2x2
		case 3:	// 4x4
			if (row & 1)
				return false;
			break;
		case 2:	// 3x3
			if (row & ~1)
				return false;
			row <<= 1;
			break;
		default:
			return false;
		}
		break;
	default:
		return false;
	}

	reg.num |= mtx << 2;
	reg.num |= col;
	reg.num |= row << 5;

	reg.name = stringValue;
	parser.eatToken();
	return true;
}

bool MipsParser::parseVfpuControlRegister(Parser& parser, MipsRegisterValue& reg)
{
	static const wchar_t* vfpuCtrlNames[16] = {
		L"spfx",	L"tpfx",	L"dpfx",	L"cc",
		L"inf4",	L"rsv5",	L"rsv6",	L"rev",
		L"rcx0",	L"rcx1",	L"rcx2",	L"rcx3",
		L"rcx4",	L"rcx5",	L"rcx6",	L"rcx7",
	};

	const Token& token = parser.peekToken();
	const std::wstring stringValue = token.getStringValue();

	if (token.type == TokenType::Identifier)
	{
		for (int i = 0; i < 16; i++)
		{
			if (stringValue == vfpuCtrlNames[i])
			{
				reg.num = i;
				reg.name = vfpuCtrlNames[i];

				parser.eatToken();
				return true;
			}
		}
	} else if (token.type == TokenType::Integer && token.intValue <= 15)
	{
		reg.num = (int) token.intValue;
		reg.name = vfpuCtrlNames[reg.num];

		parser.eatToken();
		return true;
	}

	return false;
}

bool MipsParser::parseImmediate(Parser& parser, Expression& dest)
{
	// check for (reg) or reg sequence
	TokenizerPosition pos = parser.getTokenizer()->getPosition();

	bool hasParen = parser.peekToken().type == TokenType::LParen;
	if (hasParen)
		parser.eatToken();

	MipsRegisterValue tempValue;
	bool isRegister = parseRegister(parser,tempValue);
	parser.getTokenizer()->setPosition(pos);

	if (isRegister)
		return false;

	dest = parser.parseExpression();
	return dest.isLoaded();
}

bool MipsParser::matchSymbol(Parser& parser, wchar_t symbol)
{
	switch (symbol)
	{
	case '(':
		return parser.matchToken(TokenType::LParen);
	case ')':
		return parser.matchToken(TokenType::RParen);
	case ',':
		return parser.matchToken(TokenType::Comma);
	}

	return false;
}

bool MipsParser::parseVcstParameter(Parser& parser, int& result)
{
	static TokenSequenceParser sequenceParser;

	// initialize on first use
	if (sequenceParser.getEntryCount() == 0)
	{
		// maxfloat
		sequenceParser.addEntry(1,
			{TokenType::Identifier},
			{L"maxfloat"}
		);
		// sqrt(2)
		sequenceParser.addEntry(2,
			{TokenType::Identifier, TokenType::LParen, TokenType::Integer, TokenType::RParen},
			{L"sqrt", INT64_C(2)}
		);
		// sqrt(1/2)
		sequenceParser.addEntry(3,
			{TokenType::Identifier, TokenType::LParen, TokenType::Integer, TokenType::Div, TokenType::Integer, TokenType::RParen},
			{L"sqrt", INT64_C(1), INT64_C(2)}
		);
		// sqrt(0.5)
		sequenceParser.addEntry(3,
			{TokenType::Identifier, TokenType::LParen, TokenType::Float, TokenType::RParen},
			{L"sqrt", 0.5}
		);
		// 2/sqrt(pi)
		sequenceParser.addEntry(4,
			{TokenType::Integer, TokenType::Div, TokenType::Identifier, TokenType::LParen, TokenType::Identifier, TokenType::RParen},
			{INT64_C(2), L"sqrt", L"pi"}
		);
		// 2/pi
		sequenceParser.addEntry(5,
			{TokenType::Integer, TokenType::Div, TokenType::Identifier},
			{INT64_C(2), L"pi"}
		);
		// 1/pi
		sequenceParser.addEntry(6,
			{TokenType::Integer, TokenType::Div, TokenType::Identifier},
			{INT64_C(1), L"pi"}
		);
		// pi/4
		sequenceParser.addEntry(7,
			{TokenType::Identifier, TokenType::Div, TokenType::Integer},
			{L"pi", INT64_C(4)}
		);
		// pi/2
		sequenceParser.addEntry(8,
			{TokenType::Identifier, TokenType::Div, TokenType::Integer},
			{L"pi", INT64_C(2)}
		);
		// pi/6 - early because "pi" is a prefix of it
		sequenceParser.addEntry(16,
			{TokenType::Identifier, TokenType::Div, TokenType::Integer},
			{L"pi", INT64_C(6)}
		);
		// pi
		sequenceParser.addEntry(9,
			{TokenType::Identifier},
			{L"pi"}
		);
		// e
		sequenceParser.addEntry(10,
			{TokenType::Identifier},
			{L"e"}
		);
		// log2(e)
		sequenceParser.addEntry(11,
			{TokenType::Identifier, TokenType::LParen, TokenType::Identifier, TokenType::RParen},
			{L"log2", L"e"}
		);
		// log10(e)
		sequenceParser.addEntry(12,
			{TokenType::Identifier, TokenType::LParen, TokenType::Identifier, TokenType::RParen},
			{L"log10", L"e"}
		);
		// ln(2)
		sequenceParser.addEntry(13,
			{TokenType::Identifier, TokenType::LParen, TokenType::Integer, TokenType::RParen},
			{L"ln", INT64_C(2)}
		);
		// ln(10)
		sequenceParser.addEntry(14,
			{TokenType::Identifier, TokenType::LParen, TokenType::Integer, TokenType::RParen},
			{L"ln", INT64_C(10)}
		);
		// 2*pi
		sequenceParser.addEntry(15,
			{TokenType::Integer, TokenType::Mult, TokenType::Identifier},
			{INT64_C(2), L"pi"}
		);
		// log10(2)
		sequenceParser.addEntry(17,
			{TokenType::Identifier, TokenType::LParen, TokenType::Integer, TokenType::RParen},
			{L"log10", INT64_C(2)}
		);
		// log2(10)
		sequenceParser.addEntry(18,
			{TokenType::Identifier, TokenType::LParen, TokenType::Integer, TokenType::RParen},
			{L"log2", INT64_C(10)}
		);
		// sqrt(3)/2
		sequenceParser.addEntry(19,
			{TokenType::Identifier, TokenType::LParen, TokenType::Integer, TokenType::RParen, TokenType::Div, TokenType::Integer},
			{L"sqrt", INT64_C(3), INT64_C(2)}
		);
	}

	return sequenceParser.parse(parser,result);
}

bool MipsParser::parseVfpuVrot(Parser& parser, int& result, int size)
{
	int sin = -1;
	int cos = -1;
	bool negSine = false;
	int sineCount = 0;

	if (parser.nextToken().type != TokenType::LBrack)
		return false;

	int numElems = size+1;
	for (int i = 0; i < numElems; i++)
	{
		const Token* tokenFinder = &parser.nextToken();

		if (i != 0)
		{
			if (tokenFinder->type != TokenType::Comma)
				return false;

			tokenFinder = &parser.nextToken();
		}

		bool isNeg = tokenFinder->type == TokenType::Minus;
		if (isNeg)
			tokenFinder = &parser.nextToken();

		const Token& token = *tokenFinder;

		const std::wstring stringValue = token.getStringValue();
		if (token.type != TokenType::Identifier || stringValue.size() != 1)
			return false;

		switch (stringValue[0])
		{
		case 's':
			// if one is negative, all have to be
			if ((!isNeg && negSine) || (isNeg && !negSine && sineCount > 0))
				return false;

			negSine = negSine || isNeg;
			sin = i;
			sineCount++;
			break;
		case 'c':
			// can't be negative, or happen twice
			if (isNeg || cos != -1)
				return false;
			cos = i;
			break;
		case '0':
			if (isNeg)
				return false;
			break;
		default:
			return false;
		}
	}

	if (parser.nextToken().type != TokenType::RBrack)
		return false;

	result = negSine ? 0x10 : 0;

	if (sin == -1 && cos == -1)
	{
		return false;
	} else if (sin == -1)
	{
		if (numElems == 4)
			return false;

		result |= cos;
		result |= ((size+1) << 2);
	} else if (cos == -1)
	{
		if (numElems == 4)
			return false;

		if (sineCount == 1)
		{
			result |= (size+1);
			result |= (sin << 2);
		} else if (sineCount == numElems)
		{
			result |= (size+1);
			result |= ((size+1) << 2);
		} else {
			return false;
		}
	} else {
		if (sineCount > 1)
		{
			if (sineCount+1 != numElems)
				return false;

			result |= cos;
			result |= (cos << 2);
		} else {
			result |= cos;
			result |= (sin << 2);
		}
	}

	return true;
}

bool MipsParser::parseVfpuCondition(Parser& parser, int& result)
{
	static const wchar_t* conditions[] = {
		L"fl", L"eq", L"lt", L"le", L"tr", L"ne", L"ge", L"gt",
		L"ez", L"en", L"ei", L"es", L"nz", L"nn", L"ni", L"ns"
	};

	const Token& token = parser.nextToken();
	if (token.type != TokenType::Identifier)
		return false;

	const std::wstring stringValue = token.getStringValue();
	for (size_t i = 0; i <  std::size(conditions); i++)
	{
		if (stringValue == conditions[i])
		{
			result = (int)i;
			return true;
		}
	}

	return false;
}

bool MipsParser::parseVpfxsParameter(Parser& parser, int& result)
{
	static TokenSequenceParser sequenceParser;

	// initialize on first use
	if (sequenceParser.getEntryCount() == 0)
	{
		// 0
		sequenceParser.addEntry(0, {TokenType::Integer}, {INT64_C(0)} );
		// 1
		sequenceParser.addEntry(1, {TokenType::Integer}, {INT64_C(1)} );
		// 2
		sequenceParser.addEntry(2, {TokenType::Integer}, {INT64_C(2)} );
		// 1/2
		sequenceParser.addEntry(3, {TokenType::Integer, TokenType::Div, TokenType::Integer}, {INT64_C(1), INT64_C(2)} );
		// 3
		sequenceParser.addEntry(4, {TokenType::Integer}, {INT64_C(3)} );
		// 1/3
		sequenceParser.addEntry(5, {TokenType::Integer, TokenType::Div, TokenType::Integer}, {INT64_C(1), INT64_C(3)} );
		// 1/4
		sequenceParser.addEntry(6, {TokenType::Integer, TokenType::Div, TokenType::Integer}, {INT64_C(1), INT64_C(4)} );
		// 1/6
		sequenceParser.addEntry(7, {TokenType::Integer, TokenType::Div, TokenType::Integer}, {INT64_C(1), INT64_C(6)} );
	}

	if (parser.nextToken().type != TokenType::LBrack)
		return false;

	for (int i = 0; i < 4; i++)
	{
		const Token *tokenFinder = &parser.nextToken();

		if (i != 0)
		{
			if (tokenFinder->type != TokenType::Comma)
				return false;

			tokenFinder = &parser.nextToken();
		}

		// negation
		if (tokenFinder->type == TokenType::Minus)
		{
			result |= 1 << (16+i);
			tokenFinder = &parser.nextToken();
		}

		// abs
		bool abs = false;
		if (tokenFinder->type == TokenType::BitOr)
		{
			result |= 1 << (8+i);
			abs = true;
			tokenFinder = &parser.nextToken();
		}

		const Token& token = *tokenFinder;

		// check for register
		const wchar_t* reg;
		static const wchar_t* vpfxstRegisters = L"xyzw";
		const std::wstring stringValue = token.getStringValue();
		if (stringValue.size() == 1 && (reg = wcschr(vpfxstRegisters,stringValue[0])) != nullptr)
		{
			result |= (reg-vpfxstRegisters) << (i*2);

			if (abs && parser.nextToken().type != TokenType::BitOr)
				return false;

			continue;
		}

		// abs is invalid with constants
		if (abs)
			return false;

		result |= 1 << (12+i);

		int constNum = -1;
		if (!sequenceParser.parse(parser,constNum))
			return false;

		result |= (constNum & 3) << (i*2);
		if (constNum & 4)
			result |= 1 << (8+i);
	}

	return parser.nextToken().type == TokenType::RBrack;
}

bool MipsParser::parseVpfxdParameter(Parser& parser, int& result)
{
	static TokenSequenceParser sequenceParser;

	// initialize on first use
	if (sequenceParser.getEntryCount() == 0)
	{
		// 0-1
		sequenceParser.addEntry(1,
			{TokenType::Integer, TokenType::Minus, TokenType::Integer},
			{INT64_C(0), INT64_C(1)} );
		// 0-1
		sequenceParser.addEntry(-1,
			{TokenType::Integer, TokenType::Minus, TokenType::NumberString},
			{INT64_C(0), L"1m"} );
		// 0:1
		sequenceParser.addEntry(1,
			{TokenType::Integer, TokenType::Colon, TokenType::Integer},
			{INT64_C(0), INT64_C(1)} );
		// 0:1
		sequenceParser.addEntry(-1,
			{TokenType::Integer, TokenType::Colon, TokenType::NumberString},
			{INT64_C(0), L"1m"} );
		// -1-1
		sequenceParser.addEntry(3,
			{TokenType::Minus, TokenType::Integer, TokenType::Minus, TokenType::Integer},
			{INT64_C(1), INT64_C(1)} );
		// -1-1m
		sequenceParser.addEntry(-3,
			{TokenType::Minus, TokenType::Integer, TokenType::Minus, TokenType::NumberString},
			{INT64_C(1), L"1m"} );
		// -1:1
		sequenceParser.addEntry(3,
			{TokenType::Minus, TokenType::Integer, TokenType::Colon, TokenType::Integer},
			{INT64_C(1), INT64_C(1)} );
		// -1:1m
		sequenceParser.addEntry(-3,
			{TokenType::Minus, TokenType::Integer, TokenType::Colon, TokenType::NumberString},
			{INT64_C(1), L"1m"} );
	}

	for (int i = 0; i < 4; i++)
	{
		if (i != 0)
		{
			if (parser.nextToken().type != TokenType::Comma)
				return false;
		}

		parser.eatToken();

		int num = 0;
		if (!sequenceParser.parse(parser,num))
			return false;

		// m versions
		if (num < 0)
		{
			result |= 1 << (8+i);
			num = abs(num);
		}

		result |= num << (2*i);
	}

	return parser.nextToken().type == TokenType::RBrack;
}


bool MipsParser::decodeCop2BranchCondition(const std::wstring& text, size_t& pos, int& result)
{
	if (pos+3 == text.size())
	{
		if (startsWith(text,L"any",pos))
		{
			result = 4;
			pos += 3;
			return true;
		}
		if (startsWith(text,L"all",pos))
		{
			result = 5;
			pos += 3;
			return true;
		}
	} else if (pos+1 == text.size())
	{
		switch (text[pos++])
		{
		case 'x':
		case '0':
			result = 0;
			return true;
		case 'y':
		case '1':
			result = 1;
			return true;
		case 'z':
		case '2':
			result = 2;
			return true;
		case 'w':
		case '3':
			result = 3;
			return true;
		case '4':
			result = 4;
			return true;
		case '5':
			result = 5;
			return true;
		}

		// didn't match it
		pos--;
	}

	return false;
}

bool MipsParser::parseCop2BranchCondition(Parser& parser, int& result)
{
	const Token& token = parser.nextToken();

	if (token.type == TokenType::Integer)
	{
		result = (int) token.intValue;
		return token.intValue <= 5;
	}

	if (token.type != TokenType::Identifier)
		return false;

	size_t pos = 0;
	return decodeCop2BranchCondition(token.getStringValue(),pos,result);
}

bool MipsParser::parseWb(Parser& parser)
{
	const Token& token = parser.nextToken();
	if (token.type != TokenType::Identifier)
		return false;

	return token.getStringValue() == L"wb";
}

static bool decodeImmediateSize(const char*& encoding, MipsImmediateType& dest)
{
	if (*encoding == 'h')	// half float
	{
		encoding++;
		dest = MipsImmediateType::ImmediateHalfFloat;
	} else {
		int num = 0;
		while (*encoding >= '0' && *encoding <= '9')
		{
			num = num*10 + *encoding-'0';
			encoding++;
		}

		switch (num)
		{
		case 5:
			dest = MipsImmediateType::Immediate5;
			break;
		case 7:
			dest = MipsImmediateType::Immediate7;
			break;
		case 10:
			dest = MipsImmediateType::Immediate10;
			break;
		case 16:
			dest = MipsImmediateType::Immediate16;
			break;
		case 20:
			dest = MipsImmediateType::Immediate20;
			break;
		case 25:
			dest = MipsImmediateType::Immediate25;
			break;
		case 26:
			dest = MipsImmediateType::Immediate26;
			break;
		default:
			return false;
		}
	}

	return true;
}

bool MipsParser::decodeVfpuType(const std::wstring& name, size_t& pos, int& dest)
{
	if (pos >= name.size())
		return false;

	switch (name[pos++])
	{
	case 's':
		dest = 0;
		return true;
	case 'p':
		dest = 1;
		return true;
	case 't':
		dest = 2;
		return true;
	case 'q':
		dest = 3;
		return true;
	}

	pos--;
	return false;
}

bool MipsParser::decodeOpcode(const std::wstring& name, const tMipsOpcode& opcode)
{
	const char* encoding = opcode.name;
	size_t pos = 0;

	registers.reset();
	immediate.reset();
	opcodeData.reset();
	hasFixedSecondaryImmediate = false;

	while (*encoding != 0)
	{
		switch (*encoding++)
		{
		case 'S':
			CHECK(decodeVfpuType(name,pos,opcodeData.vfpuSize));
			break;
		case 'B':
			CHECK(decodeCop2BranchCondition(name,pos,immediate.secondary.originalValue));
			immediate.secondary.type = MipsImmediateType::Cop2BranchType;
			immediate.secondary.value = immediate.secondary.originalValue;
			hasFixedSecondaryImmediate = true;
			break;
		default:
			CHECK(pos < name.size());
			CHECK(*(encoding-1) == name[pos++]);
			break;
		}
	}

	return pos >= name.size();
}

void MipsParser::setOmittedRegisters(const tMipsOpcode& opcode)
{
	// copy over omitted registers
	if (opcode.flags & MO_RSD)
		registers.grd = registers.grs;

	if (opcode.flags & MO_RST)
		registers.grt = registers.grs;

	if (opcode.flags & MO_RDT)
		registers.grt = registers.grd;

	if (opcode.flags & MO_FRSD)
		registers.frd = registers.frs;

	if (opcode.flags & MO_RSP_VRSD)
		registers.rspvrd = registers.rspvrs;
}

bool MipsParser::parseParameters(Parser& parser, const tMipsOpcode& opcode)
{
	const char* encoding = opcode.encoding;

	// initialize opcode variables
	immediate.primary.type = MipsImmediateType::None;
	if (!hasFixedSecondaryImmediate)
		immediate.secondary.type = MipsImmediateType::None;

	if (opcodeData.vfpuSize == -1)
	{
		if (opcode.flags & MO_VFPU_SINGLE)
			opcodeData.vfpuSize = 0;
		else if (opcode.flags & MO_VFPU_PAIR)
			opcodeData.vfpuSize = 1;
		else if (opcode.flags & MO_VFPU_TRIPLE)
			opcodeData.vfpuSize = 2;
		else if (opcode.flags & MO_VFPU_QUAD)
			opcodeData.vfpuSize = 3;
	}

	// parse parameters
	MipsRegisterValue tempRegister;
	int actualSize = opcodeData.vfpuSize;

	while (*encoding != 0)
	{
		switch (*encoding++)
		{
		case 't':	// register
			CHECK(parseRegister(parser,registers.grt));
			break;
		case 'd':	// register
			CHECK(parseRegister(parser,registers.grd));
			break;
		case 's':	// register
			CHECK(parseRegister(parser,registers.grs));
			break;
		case 'T':	// float register
			CHECK(parseFpuRegister(parser,registers.frt));
			break;
		case 'D':	// float register
			CHECK(parseFpuRegister(parser,registers.frd));
			break;
		case 'S':	// float register
			CHECK(parseFpuRegister(parser,registers.frs));
			break;
		case 'f':	// fpu control register
			CHECK(parseFpuControlRegister(parser,registers.frs));
			break;
		case 'z':	// cop0 register
			CHECK(parseCop0Register(parser,registers.grd));
			break;
		case 'v':	// psp vfpu reg
			if (*encoding == 'S')
			{
				encoding++;
				actualSize = 0;
			}

			switch (*encoding++)
			{
			case 's':
				CHECK(parseVfpuRegister(parser,registers.vrs,actualSize));
				CHECK(registers.vrs.type == MipsRegisterType::VfpuVector);
				if (opcode.flags & MO_VFPU_6BIT) CHECK(!(registers.vrs.num & 0x40));
				break;
			case 't':
				CHECK(parseVfpuRegister(parser,registers.vrt,actualSize));
				CHECK(registers.vrt.type == MipsRegisterType::VfpuVector);
				if (opcode.flags & MO_VFPU_6BIT) CHECK(!(registers.vrt.num & 0x40));
				break;
			case 'd':
				CHECK(parseVfpuRegister(parser,registers.vrd,actualSize));
				CHECK(registers.vrd.type == MipsRegisterType::VfpuVector);
				if (opcode.flags & MO_VFPU_6BIT) CHECK(!(registers.vrd.num & 0x40));
				break;
			case 'c':
				CHECK(parseVfpuControlRegister(parser,registers.vrd));
				break;
			default:
				return false;
			}
			break;
		case 'm':	// vfpu matrix register
			switch (*encoding++)
			{
			case 's':
				CHECK(parseVfpuRegister(parser,registers.vrs,opcodeData.vfpuSize));
				CHECK(registers.vrs.type == MipsRegisterType::VfpuMatrix);
				if (opcode.flags & MO_VFPU_TRANSPOSE_VS)
					registers.vrs.num ^= 0x20;
				break;
			case 't':
				CHECK(parseVfpuRegister(parser,registers.vrt,opcodeData.vfpuSize));
				CHECK(registers.vrt.type == MipsRegisterType::VfpuMatrix);
				break;
			case 'd':
				CHECK(parseVfpuRegister(parser,registers.vrd,opcodeData.vfpuSize));
				CHECK(registers.vrd.type == MipsRegisterType::VfpuMatrix);
				break;
			default:
				return false;
			}
			break;
		case 'V':	// ps2 vector reg
			switch (*encoding++)
			{
			case 't':	// register
				CHECK(parsePs2Cop2Register(parser,registers.ps2vrt));
				break;
			case 'd':	// register
				CHECK(parsePs2Cop2Register(parser,registers.ps2vrd));
				break;
			case 's':	// register
				CHECK(parsePs2Cop2Register(parser,registers.ps2vrs));
				break;
			default:
				return false;
			}
			break;
		case 'g':	// psx cop2 reg
			switch (*encoding++)
			{
			case 't':	// gte data register
				CHECK(parsePsxCop2DataRegister(parser,registers.grt));
				break;
			case 's':	// gte data register
				CHECK(parsePsxCop2DataRegister(parser,registers.grd));
				break;
			case 'c':	// gte control register
				CHECK(parsePsxCop2ControlRegister(parser,registers.grd));
				break;
			default:
				return false;
			}
			break;
		case 'r':	// forced register
			CHECK(parseRegister(parser,tempRegister));
			CHECK(tempRegister.num == *encoding++);
			break;
		case 'R':	// rsp register
			switch (*encoding++)
			{
			case 'z':	// cop0 register
				CHECK(parseRspCop0Register(parser,registers.grd));
				break;
			case 'c':	// vector control register
				CHECK(parseRspVectorControlRegister(parser,registers.grd));
				break;
			case 't':	// vector register
				CHECK(parseRspVectorRegister(parser,registers.rspvrt));
				break;
			case 'd':	// vector register
				CHECK(parseRspVectorRegister(parser,registers.rspvrd));
				break;
			case 's':	// vector register
				CHECK(parseRspVectorRegister(parser,registers.rspvrs));
				break;
			case 'e':	// vector element
				CHECK(parseRspVectorElement(parser,registers.rspve));
				break;
			case 'l':	// scalar element
				CHECK(parseRspScalarElement(parser,registers.rspve));
				break;
			case 'm':	// scalar destination element
				CHECK(parseRspScalarElement(parser,registers.rspvde));
				break;
			case 'o':	// byte offset element
				CHECK(parseRspOffsetElement(parser,registers.rspvealt));
				break;
			default:
				return false;
			}
			break;
		case 'i':	// primary immediate
			CHECK(parseImmediate(parser,immediate.primary.expression));
			allowFunctionCallExpression(*encoding != '(');
			CHECK(decodeImmediateSize(encoding,immediate.primary.type));
			allowFunctionCallExpression(true);
			break;
		case 'j':	// secondary immediate
			switch (*encoding++)
			{
			case 'c':
				CHECK(parseImmediate(parser,immediate.secondary.expression));
				immediate.secondary.type = MipsImmediateType::CacheOp;
				break;
			case 'e':
				CHECK(parseImmediate(parser,immediate.secondary.expression));
				immediate.secondary.type = MipsImmediateType::Ext;
				break;
			case 'i':
				CHECK(parseImmediate(parser,immediate.secondary.expression));
				immediate.secondary.type = MipsImmediateType::Ins;
				break;
			case 'b':
				CHECK(parseCop2BranchCondition(parser,immediate.secondary.originalValue));
				immediate.secondary.type = MipsImmediateType::Cop2BranchType;
				immediate.secondary.value = immediate.secondary.originalValue;
				break;
			default:
				return false;
			}
			break;
		case 'C':	// vfpu condition
			CHECK(parseVfpuCondition(parser,opcodeData.vectorCondition));
			break;
		case 'W':	// vfpu argument
			switch (*encoding++)
			{
			case 's':
				CHECK(parseVpfxsParameter(parser,immediate.primary.originalValue));
				immediate.primary.value = immediate.primary.originalValue;
				immediate.primary.type = MipsImmediateType::Immediate20_0;
				break;
			case 'd':
				CHECK(parseVpfxdParameter(parser,immediate.primary.originalValue));
				immediate.primary.value = immediate.primary.originalValue;
				immediate.primary.type = MipsImmediateType::Immediate16;
				break;
			case 'c':
				CHECK(parseVcstParameter(parser,immediate.primary.originalValue));
				immediate.primary.value = immediate.primary.originalValue;
				immediate.primary.type = MipsImmediateType::Immediate5;
				break;
			case 'r':
				CHECK(parseVfpuVrot(parser,immediate.primary.originalValue,opcodeData.vfpuSize));
				immediate.primary.value = immediate.primary.originalValue;
				immediate.primary.type = MipsImmediateType::Immediate5;
				break;
			default:
				return false;
			}
			break;
		case 'w':	// 'wb' characters
			CHECK(parseWb(parser));
			break;
		default:
			CHECK(matchSymbol(parser,*(encoding-1)));
			break;
		}
	}

	opcodeData.opcode = opcode;
	setOmittedRegisters(opcode);

	// the next token has to be a separator, else the parameters aren't
	// completely parsed

	return parser.nextToken().type == TokenType::Separator;

}

std::unique_ptr<CMipsInstruction> MipsParser::parseOpcode(Parser& parser)
{
	if (parser.peekToken().type != TokenType::Identifier)
		return nullptr;

	const Token &token = parser.nextToken();

	bool paramFail = false;
	const MipsArchDefinition& arch = mipsArchs[Mips.GetVersion()];
	const std::wstring stringValue = token.getStringValue();

	for (int z = 0; MipsOpcodes[z].name != nullptr; z++)
	{
		if ((MipsOpcodes[z].archs & arch.supportSets) == 0)
			continue;
		if ((MipsOpcodes[z].archs & arch.excludeMask) != 0)
			continue;

		if ((MipsOpcodes[z].flags & MO_64BIT) && !(arch.flags & MO_64BIT))
			continue;
		if ((MipsOpcodes[z].flags & MO_FPU) && !(arch.flags & MO_FPU))
			continue;
		if ((MipsOpcodes[z].flags & MO_DFPU) && !(arch.flags & MO_DFPU))
			continue;

		if (decodeOpcode(stringValue,MipsOpcodes[z]))
		{
			TokenizerPosition tokenPos = parser.getTokenizer()->getPosition();

			if (parseParameters(parser,MipsOpcodes[z]))
			{
				// success, return opcode
				return std::make_unique<CMipsInstruction>(opcodeData,immediate,registers);
			}

			parser.getTokenizer()->setPosition(tokenPos);
			paramFail = true;
		}
	}

	if (paramFail)
		parser.printError(token,L"MIPS parameter failure");
	else
		parser.printError(token,L"Invalid MIPS opcode '%s'",stringValue);

	return nullptr;
}

bool MipsParser::parseMacroParameters(Parser& parser, const MipsMacroDefinition& macro)
{
	const wchar_t* encoding = (const wchar_t*) macro.args;

	while (*encoding != 0)
	{
		switch (*encoding++)
		{
		case 't':	// register
			CHECK(parseRegister(parser,registers.grt));
			break;
		case 'd':	// register
			CHECK(parseRegister(parser,registers.grd));
			break;
		case 's':	// register
			CHECK(parseRegister(parser,registers.grs));
			break;
		case 'S':	// register
			CHECK(parseFpuRegister(parser,registers.frs));
			break;
		case 'i':	// primary immediate
			allowFunctionCallExpression(*encoding != '(');
			CHECK(parseImmediate(parser,immediate.primary.expression));
			allowFunctionCallExpression(true);
			break;
		case 'I':	// secondary immediate
			allowFunctionCallExpression(*encoding != '(');
			CHECK(parseImmediate(parser,immediate.secondary.expression));
			allowFunctionCallExpression(true);
			break;
		default:
			CHECK(matchSymbol(parser,*(encoding-1)));
			break;
		}
	}

	// lw rx,imm is a prefix of lw rx,imm(ry)
	if (parser.peekToken().type == TokenType::LParen)
		return false;

	// the next token has to be a separator, else the parameters aren't
	// completely parsed
	return parser.nextToken().type == TokenType::Separator;
}

std::unique_ptr<CAssemblerCommand> MipsParser::parseMacro(Parser& parser)
{
	TokenizerPosition startPos = parser.getTokenizer()->getPosition();

	// Cannot be a reference (we eat below.)
	const Token token = parser.peekToken();
	if (token.type != TokenType::Identifier)
		return nullptr;

	parser.eatToken();
	const std::wstring stringValue = token.getStringValue();
	for (int z = 0; mipsMacros[z].name != nullptr; z++)
	{
		if (stringValue == mipsMacros[z].name)
		{
			TokenizerPosition tokenPos = parser.getTokenizer()->getPosition();

			if (parseMacroParameters(parser,mipsMacros[z]))
			{
				return mipsMacros[z].function(parser,registers,immediate,mipsMacros[z].flags);
			}

			parser.getTokenizer()->setPosition(tokenPos);
		}
	}

	// no matching macro found, restore state
	parser.getTokenizer()->setPosition(startPos);
	return nullptr;
}

void MipsOpcodeFormatter::handleOpcodeName(const MipsOpcodeData& opData)
{
	const char* encoding = opData.opcode.name;

	while (*encoding != 0)
	{
		switch (*encoding++)
		{
		case 'S':
			buffer += "sptq"[opData.vfpuSize];
			break;
		case 'B':
			// TODO
			break;
		default:
			buffer += *(encoding-1);
			break;
		}
	}
}

void MipsOpcodeFormatter::handleImmediate(MipsImmediateType type, unsigned int originalValue, unsigned int opcodeFlags)
{
	switch (type)
	{
	case MipsImmediateType::ImmediateHalfFloat:
		buffer += tfm::format(L"%f", bitsToFloat(originalValue));
		break;
	case MipsImmediateType::Immediate16:
		if (!(opcodeFlags & MO_IPCR) && originalValue & 0x8000)
			buffer += tfm::format(L"-0x%X", 0x10000-(originalValue & 0xFFFF));
		else
			buffer += tfm::format(L"0x%X", originalValue);
		break;
	default:
		buffer += tfm::format(L"0x%X", originalValue);
		break;
	}
}

void MipsOpcodeFormatter::handleOpcodeParameters(const MipsOpcodeData& opData, const MipsRegisterData& regData,
	const MipsImmediateData& immData)
{
	const char* encoding = opData.opcode.encoding;

	MipsImmediateType type;
	while (*encoding != 0)
	{
		switch (*encoding++)
		{
		case 'r':	// forced register
			buffer += tfm::format(L"r%d",*encoding);
			encoding += 1;
			break;
		case 's':	// register
			buffer += regData.grs.name;
			break;
		case 'd':	// register
			buffer += regData.grd.name;
			break;
		case 't':	// register
			buffer += regData.grt.name;
			break;
		case 'S':	// fpu register
			buffer += regData.frs.name;
			break;
		case 'D':	// fpu register
			buffer += regData.frd.name;
			break;
		case 'T':	// fpu register
			buffer += regData.frt.name;
			break;
		case 'v':	// psp vfpu reg
		case 'm':	// vfpu matrix register
			switch (*encoding++)
			{
			case 'd':
				buffer += regData.vrd.name;
				break;
			case 's':
				buffer += regData.vrs.name;
				break;
			case 't':
				buffer += regData.vrt.name;
				break;
			}
			break;
		case 'V':	// ps2 vector reg
			switch (*encoding++)
			{
			case 'd':
				buffer += regData.ps2vrd.name;
				break;
			case 's':
				buffer += regData.ps2vrs.name;
				break;
			case 't':
				buffer += regData.ps2vrt.name;
				break;
			}
			break;
		case 'i':	// primary immediate
			decodeImmediateSize(encoding,type);
			handleImmediate(immData.primary.type,immData.primary.originalValue,opData.opcode.flags);
			break;
		case 'j':	// secondary immediate
			handleImmediate(immData.secondary.type,immData.secondary.originalValue, opData.opcode.flags);
			encoding++;
			break;
		case 'C':	// vfpu condition
		case 'W':	// vfpu argument
			// TODO
			break;
		case 'w':	// 'wb' characters
			buffer += L"wb";
			break;
		default:
			buffer += *(encoding-1);
			break;
		}
	}
}

const std::wstring& MipsOpcodeFormatter::formatOpcode(const MipsOpcodeData& opData, const MipsRegisterData& regData,
	const MipsImmediateData& immData)
{
	buffer = L"   ";
	handleOpcodeName(opData);

	while (buffer.size() < 11)
		buffer += ' ';

	handleOpcodeParameters(opData,regData,immData);
	return buffer;
}

// file: Archs/MIPS/PsxRelocator.cpp


#include <cstring>
#include <map>

struct PsxLibEntry
{
	std::wstring name;
	ByteArray data;
};

const unsigned char psxObjectFileMagicNum[6] = { 'L', 'N', 'K', '\x02', '\x2E', '\x07' };

std::vector<PsxLibEntry> loadPsxLibrary(const fs::path& inputName)
{
	ByteArray input = ByteArray::fromFile(inputName);
	std::vector<PsxLibEntry> result;

	if (input.size() == 0)
		return result;

	if (memcmp(input.data(),psxObjectFileMagicNum,sizeof(psxObjectFileMagicNum)) == 0)
	{
		PsxLibEntry entry;
		entry.name = inputName.filename().wstring();
		entry.data = input;
		result.push_back(entry);
		return result;
	}

	if (memcmp(input.data(),"LIB\x01",4) != 0)
		return result;

	size_t pos = 4;
	while (pos < input.size())
	{
		PsxLibEntry entry;

		for (int i = 0; i < 16 && input[pos+i] != ' '; i++)
		{
			entry.name += input[pos+i];
		}

		int size = input.getDoubleWord(pos+16);
		int skip = 20;

		while (input[pos+skip] != 0)
		{
			skip += 1+input[pos+skip];
		}

		skip++;

		entry.data = input.mid(pos+skip,size-skip);
		pos += size;

		result.push_back(entry);
	}

	return result;
}

size_t PsxRelocator::loadString(ByteArray& data, size_t pos, std::wstring& dest)
{
	dest = L"";
	int len = data[pos++];

	for (int i = 0; i < len; i++)
	{
		dest += data[pos++];
	}

	return len+1;
}

bool PsxRelocator::parseObject(ByteArray data, PsxRelocatorFile& dest)
{
	if (memcmp(data.data(),psxObjectFileMagicNum,sizeof(psxObjectFileMagicNum)) != 0)
		return false;

	size_t pos = 6;

	std::vector<PsxSegment>& segments = dest.segments;
	std::vector<PsxSymbol>& syms = dest.symbols;

	int activeSegment = -1;
	int lastSegmentPartStart = -1;
	while (pos < data.size())
	{
		switch (data[pos])
		{
		case 0x10:	// segment definition
			{
				PsxSegment seg;
				seg.id = data.getDoubleWord(pos+1);
				segments.push_back(seg);
				pos += 5;

				if (data[pos] != 8)
					return false;

				std::wstring& name = segments[segments.size()-1].name;
				pos += 1 + loadString(data,pos+1,name);
			}
			break;
		case 0x14:	// group?
			pos += data[pos+4]+5;
			break;
		case 0x1C:	// source file name
			pos += data[pos+3]+4;
			break;

		case 0x06:	// set segment id
			{
				int id = data.getWord(pos+1);
				pos += 3;

				int num = -1;
				for (size_t i = 0; i < segments.size(); i++)
					{
					if (segments[i].id == id)
					{
						num = (int) i;
						break;
					}
				}

				activeSegment = num;
			}
			break;
		case 0x02:	// append to data segment
			{
				int size = data.getWord(pos+1);
				pos += 3;

				ByteArray d = data.mid(pos,size);
				pos += size;

				lastSegmentPartStart = (int) segments[activeSegment].data.size();
				segments[activeSegment].data.append(d);
			}
			break;
		case 0x08:	// append zeroes data segment
			{
				int size = data.getWord(pos+1);
				pos += 3;

				ByteArray d;
				d.reserveBytes(size);
				segments[activeSegment].data.append(d);
			}
			break;
		case 0x0A:	// relocation data
			{
				int type = data[pos+1];
				pos += 2;

				PsxRelocation rel;
				rel.relativeOffset = 0;
				rel.filePos = (int) pos-2;

				switch (type)
				{
				case 0x10:	// 32 bit word
					rel.type = PsxRelocationType::WordLiteral;
					rel.segmentOffset = data.getWord(pos);
					pos += 2;
					break;
				case 0x4A:	// jal
					rel.type = PsxRelocationType::FunctionCall;
					rel.segmentOffset = data.getWord(pos);
					pos += 2;
					break;
				case 0x52:	// upper immerdiate
					rel.type = PsxRelocationType::UpperImmediate;
					rel.segmentOffset = data.getWord(pos);
					pos += 2;
					break;
				case 0x54:	// lower immediate (add)
					rel.type = PsxRelocationType::LowerImmediate;
					rel.segmentOffset = data.getWord(pos);
					pos += 2;
					break;
				default:
					return false;
				}

				rel.segmentOffset += lastSegmentPartStart;
checkothertype:
				int otherType = data[pos++];
				switch (otherType)
				{
				case 0x02:	// reference to symbol with id num
					rel.refType = PsxRelocationRefType::SymblId;
					rel.referenceId = data.getWord(pos);
					pos += 2;
					break;
				case 0x2C:	// ref to other segment?
					rel.refType = PsxRelocationRefType::SegmentOffset;

					switch (data[pos++])
					{
					case 0x00:
						rel.relativeOffset = data.getDoubleWord(pos);
						pos += 4;
						goto checkothertype;
					case 0x04:
						rel.referenceId = data.getWord(pos);	// segment id
						pos += 2;

						if (data[pos++] != 0x00)
						{
							return false;
						}

						rel.referencePos = data.getDoubleWord(pos);
						pos += 4;
						break;
					default:
						return false;
					}
					break;
				case 0x2E:	// negative ref?
					rel.refType = PsxRelocationRefType::SegmentOffset;

					switch (data[pos++])
					{
					case 0x00:
						rel.relativeOffset = -data.getDoubleWord(pos);
						pos += 4;
						goto checkothertype;
					default:
						return false;
					}
					break;
				default:
					return false;
				}

				segments[activeSegment].relocations.push_back(rel);
			}
			break;
		case 0x12:	// internal symbol
			{
				PsxSymbol sym;
				sym.type = PsxSymbolType::Internal;
				sym.segment = data.getWord(pos+1);
				sym.offset = data.getDoubleWord(pos+3);
				pos += 7 + loadString(data,pos+7,sym.name);
				syms.push_back(sym);
			}
			break;
		case 0x0E:	// external symbol
			{
				PsxSymbol sym;
				sym.type = PsxSymbolType::External;
				sym.id = data.getWord(pos+1);
				pos += 3 + loadString(data,pos+3,sym.name);
				syms.push_back(sym);
			}
			break;
		case 0x30:	// bss symbol?
			{
				PsxSymbol sym;
				sym.type = PsxSymbolType::BSS;
				sym.id = data.getWord(pos+1);
				sym.segment = data.getWord(pos+3);
				sym.size = data.getDoubleWord(pos+5);
				pos += 9 + loadString(data,pos+9,sym.name);
				syms.push_back(sym);
			}
			break;
		case 0x0C:	// internal with id
			{
				PsxSymbol sym;
				sym.type = PsxSymbolType::InternalID;
				sym.id = data.getWord(pos+1);
				sym.segment = data.getWord(pos+3);
				sym.offset = data.getDoubleWord(pos+5);
				pos += 9 + loadString(data,pos+9,sym.name);
				syms.push_back(sym);
			}
			break;
		case 0x4A:	// function
			{
				PsxSymbol sym;
				sym.type = PsxSymbolType::Function;
				sym.segment = data.getWord(pos+1);
				sym.offset = data.getDoubleWord(pos+3);
				pos += 0x1D + loadString(data,pos+0x1D,sym.name);
				syms.push_back(sym);
			}
			break;
		case 0x4C:	// function size
			pos += 11;
			break;
		case 0x3C:	// ??
			pos += 3;
			break;
		case 0x00:	// ??
			pos++;
			break;
		case 0x32:	// ??
			pos += 3;
			break;
		case 0x3A:	// ??
			pos += 9;
			break;
		default:
			return false;
		}
	}

	return true;
}

bool PsxRelocator::init(const fs::path& inputName)
{
	auto inputFiles = loadPsxLibrary(inputName);
	if (inputFiles.size() == 0)
	{
		Logger::printError(Logger::Error,L"Could not load library");
		return false;
	}

	reloc = new MipsElfRelocator();

	for (PsxLibEntry& entry: inputFiles)
	{
		PsxRelocatorFile file;
		file.name = entry.name;

		if (!parseObject(entry.data,file))
		{
			Logger::printError(Logger::Error,L"Could not load object file %s",entry.name);
			return false;
		}

		// sort relocations
		for (PsxSegment& seg: file.segments)
		{
			auto sortFunc = [](const PsxRelocation &a, const PsxRelocation &b)
			{
				// Sort in order of...
				// - reference type - symbol or offset
				// - reference id - this groups references to the same symbol/segment/?
				// - referencePos - this ensure references to the same offset are grouped
				// - type - this ensures that HI16 is before LO16
				auto tie = [](const PsxRelocation &rel)
				{
					return std::tie(rel.refType, rel.referenceId, rel.referencePos, rel.type);
				};

				return tie(a) < tie(b);
			};

			std::stable_sort(seg.relocations.begin(), seg.relocations.end(), sortFunc);
		}

		// init symbols
		for (PsxSymbol& sym: file.symbols)
		{
			std::wstring lowered = sym.name;
			std::transform(lowered.begin(), lowered.end(), lowered.begin(), ::towlower);

			sym.label = Global.symbolTable.getLabel(lowered,-1,-1);
			if (sym.label == nullptr)
			{
				Logger::printError(Logger::Error,L"Invalid label name \"%s\"",sym.name);
				continue;
			}

			if (sym.label->isDefined() && sym.type != PsxSymbolType::External)
			{
				Logger::printError(Logger::Error,L"Label \"%s\" already defined",sym.name);
				continue;
			}

			sym.label->setOriginalName(sym.name);
		}

		files.push_back(file);
	}

	return true;
}

bool PsxRelocator::relocateFile(PsxRelocatorFile& file, int& relocationAddress)
{
	std::map<int,int> relocationOffsets;
	std::map<int,int> symbolOffsets;
	int start = relocationAddress;

	// assign addresses to segments
	for (PsxSegment& seg: file.segments)
	{
		int index = seg.id;
		size_t size = seg.data.size();

		relocationOffsets[index] = relocationAddress;
		relocationAddress += (int) size;

		while (relocationAddress % 4)
			relocationAddress++;
	}

	// parse/add/relocate symbols
	bool error = false;
	for (PsxSymbol& sym: file.symbols)
	{
		int pos;
		switch (sym.type)
		{
		case PsxSymbolType::Internal:
		case PsxSymbolType::Function:
			sym.label->setValue(relocationOffsets[sym.segment]+sym.offset);
			sym.label->setDefined(true);
			break;
		case PsxSymbolType::InternalID:
			pos = relocationOffsets[sym.segment]+sym.offset;
			sym.label->setValue(pos);
			sym.label->setDefined(true);
			symbolOffsets[sym.id] = pos;
			break;
		case PsxSymbolType::BSS:
			sym.label->setValue(relocationAddress);
			sym.label->setDefined(true);
			symbolOffsets[sym.id] = relocationAddress;
			relocationAddress += sym.size;

			while (relocationAddress % 4)
				relocationAddress++;
			break;
		case PsxSymbolType::External:
			if (!sym.label->isDefined())
			{
				Logger::queueError(Logger::Error,L"Undefined external symbol %s in file %s",sym.name,file.name);
				error = true;
				continue;
			}

			symbolOffsets[sym.id] = (int) sym.label->getValue();
			break;
		}
	}

	if (error)
		return false;

	size_t dataStart = outputData.size();
	outputData.reserveBytes(relocationAddress-start);

	// load code and data
	for (PsxSegment& seg: file.segments)
	{
		// relocate
		ByteArray sectionData = seg.data;

		std::vector<RelocationAction> relocationActions;
		for (PsxRelocation& rel: seg.relocations)
		{
			RelocationData relData;
			int pos = rel.segmentOffset;
			relData.opcodeOffset = pos;
			relData.opcode = sectionData.getDoubleWord(pos);

			switch (rel.refType)
			{
			case PsxRelocationRefType::SymblId:
				relData.relocationBase = symbolOffsets[rel.referenceId]+rel.relativeOffset;
				break;
			case PsxRelocationRefType::SegmentOffset:
				relData.relocationBase = relocationOffsets[rel.referenceId] + rel.referencePos+rel.relativeOffset;
				break;
			}

			std::vector<std::wstring> errors;
			bool result = false;

			switch (rel.type)
			{
			case PsxRelocationType::WordLiteral:
				result = reloc->relocateOpcode(R_MIPS_32,relData, relocationActions, errors);
				break;
			case PsxRelocationType::UpperImmediate:
				result = reloc->relocateOpcode(R_MIPS_HI16,relData, relocationActions, errors);
				break;
			case PsxRelocationType::LowerImmediate:
				result = reloc->relocateOpcode(R_MIPS_LO16,relData, relocationActions, errors);
				break;
			case PsxRelocationType::FunctionCall:
				result = reloc->relocateOpcode(R_MIPS_26,relData, relocationActions, errors);
				break;
			}

			if (!result)
			{
				for (const std::wstring& error : errors)
				{
					Logger::queueError(Logger::Error, error);
				}
				error = true;
			}
		}

		// finish any dangling relocations
		std::vector<std::wstring> errors;
		if (!reloc->finish(relocationActions, errors))
		{
			for (const std::wstring& error : errors)
			{
				Logger::queueError(Logger::Error, error);
			}
			error = true;
		}

		// now actually write the relocated values
		for (const RelocationAction& action : relocationActions)
		{
			sectionData.replaceDoubleWord(action.offset, action.newValue);
		}

		size_t arrayStart = dataStart+relocationOffsets[seg.id]-start;
		memcpy(outputData.data(arrayStart),sectionData.data(),sectionData.size());
	}

	return !error;
}

bool PsxRelocator::relocate(int& memoryAddress)
{
	int oldCrc = getCrc32(outputData.data(),outputData.size());
	outputData.clear();
	dataChanged = false;

	bool error = false;
	int start = memoryAddress;

	for (PsxRelocatorFile& file: files)
	{
		if (!relocateFile(file,memoryAddress))
			error = true;
	}

	int newCrc = getCrc32(outputData.data(),outputData.size());
	if (oldCrc != newCrc)
		dataChanged = true;

	memoryAddress -= start;
	return !error;
}


void PsxRelocator::writeSymbols(SymbolData& symData) const
{
	for (const PsxRelocatorFile& file: files)
	{
		for (const PsxSymbol& sym: file.symbols)
		{
			if (sym.type != PsxSymbolType::External)
				symData.addLabel(sym.label->getValue(),sym.name.c_str());
		}
	}
}

//
// DirectivePsxObjImport
//

DirectivePsxObjImport::DirectivePsxObjImport(const fs::path& fileName)
{
	if (rel.init(fileName))
	{
	}
}

bool DirectivePsxObjImport::Validate(const ValidateState &state)
{
	int memory = (int) g_fileManager->getVirtualAddress();
	rel.relocate(memory);
	g_fileManager->advanceMemory(memory);
	return rel.hasDataChanged();
}

void DirectivePsxObjImport::Encode() const
{
	const ByteArray& data = rel.getData();
	g_fileManager->write(data.data(),data.size());
}

void DirectivePsxObjImport::writeSymData(SymbolData& symData) const
{
	rel.writeSymbols(symData);
}

// file: Archs/Architecture.cpp


CInvalidArchitecture InvalidArchitecture;

const ExpressionFunctionMap &CArchitecture::getExpressionFunctions()
{
	const static ExpressionFunctionMap emptyMap = {};
	return emptyMap;
}

ArchitectureCommand::ArchitectureCommand(const std::wstring& tempText, const std::wstring& symText)
{
	this->tempText = tempText;
	this->symText = symText;
	this->endianness = Arch->getEndianness();
}

bool ArchitectureCommand::Validate(const ValidateState &state)
{
	position = g_fileManager->getVirtualAddress();
	g_fileManager->setEndianness(endianness);
	return false;
}

void ArchitectureCommand::Encode() const
{
	g_fileManager->setEndianness(endianness);
}

void ArchitectureCommand::writeTempData(TempData& tempData) const
{
	if (tempText.size() != 0)
	{
		std::wstringstream stream(tempText);

		std::wstring line;
		while (std::getline(stream,line,L'\n'))
		{
			if (line.size() != 0)
				tempData.writeLine(position,line);
		}
	}
}

void ArchitectureCommand::writeSymData(SymbolData& symData) const
{
	// TODO: find a less ugly way to check for undefined memory positions
	if (position == -1)
		return;

	if (symText.size() != 0)
		symData.addLabel(position,symText);
}


void CInvalidArchitecture::NextSection()
{
	Logger::printError(Logger::FatalError,L"No architecture specified");
}

void CInvalidArchitecture::Pass2()
{
	Logger::printError(Logger::FatalError,L"No architecture specified");
}

void CInvalidArchitecture::Revalidate()
{
	Logger::printError(Logger::FatalError,L"No architecture specified");
}

std::unique_ptr<IElfRelocator> CInvalidArchitecture::getElfRelocator()
{
	Logger::printError(Logger::FatalError,L"No architecture specified");
	return nullptr;
}

// file: Commands/CAssemblerCommand.cpp


CAssemblerCommand::CAssemblerCommand()
{
	FileNum = Global.FileInfo.FileNum;
	FileLine = Global.FileInfo.LineNumber;
	section = Global.Section;
}

void CAssemblerCommand::applyFileInfo()
{
	Global.FileInfo.FileNum = FileNum;
	Global.FileInfo.LineNumber = FileLine;
}

// file: Commands/CAssemblerLabel.h


class Label;

class CAssemblerLabel: public CAssemblerCommand
{
public:
	CAssemblerLabel(const std::wstring& name, const std::wstring& originalName);
	CAssemblerLabel(const std::wstring& name, const std::wstring& originalName, Expression& value);
	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
	void writeSymData(SymbolData& symData) const override;
private:
	Expression labelValue;
	std::shared_ptr<Label> label;
	bool defined;
};

class CDirectiveFunction: public CAssemblerCommand
{
public:
	CDirectiveFunction(const std::wstring& name, const std::wstring& originalName);
	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
	void writeSymData(SymbolData& symData) const override;
	void setContent(std::unique_ptr<CAssemblerCommand> content) { this->content = std::move(content); }
private:
	std::unique_ptr<CAssemblerLabel> label;
	std::unique_ptr<CAssemblerCommand> content;
	int64_t start, end;
};

// file: Commands/CAssemblerLabel.cpp


CAssemblerLabel::CAssemblerLabel(const std::wstring& name, const std::wstring& originalName)
{
	this->defined = false;
	this->label = nullptr;

	if (!Global.symbolTable.isLocalSymbol(name))
		updateSection(++Global.Section);

	label = Global.symbolTable.getLabel(name, FileNum, getSection());
	if (label == nullptr)
	{
		Logger::printError(Logger::Error, L"Invalid label name \"%s\"", name);
		return;
	}

	label->setOriginalName(originalName);

	// does this need to be in validate?
	if (label->getUpdateInfo())
	{
#ifdef ARMIPS_ARM
		if (Arch == &Arm && Arm.GetThumbMode())
			label->setInfo(1);
		else
#endif
			label->setInfo(0);
	}
}

CAssemblerLabel::CAssemblerLabel(const std::wstring& name, const std::wstring& originalName, Expression& value)
	: CAssemblerLabel(name,originalName)
{
	labelValue = value;
}

bool CAssemblerLabel::Validate(const ValidateState &state)
{
	bool result = false;
	if (!defined)
	{
		if (label->isDefined())
		{
			Logger::queueError(Logger::Error, L"Label \"%s\" already defined", label->getName());
			return false;
		}

		label->setDefined(true);
		defined = true;
		result = true;
	}

	bool hasPhysicalValue = false;
	int64_t virtualValue = 0;
	int64_t physicalValue = 0;

	if (labelValue.isLoaded())
	{
		// label value is given by expression
		if (!labelValue.evaluateInteger(virtualValue))
		{
			Logger::printError(Logger::Error, L"Invalid expression");
			return result;
		}
	} else {
		// label value is given by current address
		virtualValue = g_fileManager->getVirtualAddress();
		physicalValue = g_fileManager->getPhysicalAddress();
		hasPhysicalValue = true;
	}

	if (label->getValue() != virtualValue)
	{
		label->setValue(virtualValue);
		result = true;
	}

	if (hasPhysicalValue && (!label->hasPhysicalValue() || physicalValue != label->getPhysicalValue()))
	{
		label->setPhysicalValue(physicalValue);
		result = true;
	}

	return result;
}

void CAssemblerLabel::Encode() const
{

}

void CAssemblerLabel::writeTempData(TempData& tempData) const
{
	if (!Global.symbolTable.isGeneratedLabel(label->getName()))
		tempData.writeLine(label->getValue(),tfm::format(L"%s:",label->getName()));
}

void CAssemblerLabel::writeSymData(SymbolData& symData) const
{
	// TODO: find a less ugly way to check for undefined memory positions
	if (label->getValue() == -1 || Global.symbolTable.isGeneratedLabel(label->getName()))
		return;

	symData.addLabel(label->getValue(),label->getOriginalName());
}




CDirectiveFunction::CDirectiveFunction(const std::wstring& name, const std::wstring& originalName)
{
	this->label = std::make_unique<CAssemblerLabel>(name,originalName);
	this->content = nullptr;
	this->start = this->end = 0;
}

bool CDirectiveFunction::Validate(const ValidateState &state)
{
	start = g_fileManager->getVirtualAddress();

	label->applyFileInfo();
	bool result = label->Validate(state);

	ValidateState contentValidation = state;
	contentValidation.noFileChange = true;
	contentValidation.noFileChangeDirective = L"function";
	content->applyFileInfo();
	if (content->Validate(contentValidation))
		result = true;

	end = g_fileManager->getVirtualAddress();
	return result;
}

void CDirectiveFunction::Encode() const
{
	label->Encode();
	content->Encode();
}

void CDirectiveFunction::writeTempData(TempData& tempData) const
{
	label->writeTempData(tempData);
	content->applyFileInfo();
	content->writeTempData(tempData);
}

void CDirectiveFunction::writeSymData(SymbolData& symData) const
{
	symData.startFunction(start);
	label->writeSymData(symData);
	content->writeSymData(symData);
	symData.endFunction(end);
}

// file: Commands/CDirectiveArea.h


class CDirectiveArea: public CAssemblerCommand
{
public:
	CDirectiveArea(bool shared, Expression& size);
	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
	void writeSymData(SymbolData& symData) const override;
	void setFillExpression(Expression& exp);
	void setPositionExpression(Expression& exp);
	void setContent(std::unique_ptr<CAssemblerCommand> content) { this->content = std::move(content); }
private:
	bool shared;
	int64_t position;
	Expression sizeExpression;
	int64_t areaSize;
	int64_t contentSize;
	Expression fillExpression;
	int8_t fillValue;
	int64_t fileID = 0;
	Expression positionExpression;
	std::unique_ptr<CAssemblerCommand> content;
};

class CDirectiveAutoRegion : public CAssemblerCommand
{
public:
	CDirectiveAutoRegion();
	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
	void writeSymData(SymbolData& symData) const override;
	void setMinRangeExpression(Expression& exp);
	void setRangeExpressions(Expression& minExp, Expression& maxExp);
	void setContent(std::unique_ptr<CAssemblerCommand> content) { this->content = std::move(content); }

private:
	int64_t resetPosition;
	int64_t position;
	int64_t contentSize;
	int64_t fileID = 0;
	Expression minRangeExpression;
	Expression maxRangeExpression;
	std::unique_ptr<CAssemblerCommand> content;
};

// file: Commands/CDirectiveArea.cpp


#include <algorithm>
#include <cstring>

CDirectiveArea::CDirectiveArea(bool shared, Expression& size)
{
	this->areaSize = 0;
	this->contentSize = 0;
	this->position = 0;
	this->fillValue = 0;

	this->shared = shared;
	this->sizeExpression = size;
	this->content = nullptr;
}

void CDirectiveArea::setFillExpression(Expression& exp)
{
	fillExpression = exp;
}

void CDirectiveArea::setPositionExpression(Expression& exp)
{
	positionExpression = exp;
}

bool CDirectiveArea::Validate(const ValidateState &state)
{
	int64_t oldAreaSize = areaSize;
	int64_t oldContentSize = contentSize;
	int64_t oldPosition = position;

	if (positionExpression.isLoaded())
	{
		if (!positionExpression.evaluateInteger(position))
		{
			Logger::queueError(Logger::Error, L"Invalid position expression");
			return false;
		}
		Arch->NextSection();
		g_fileManager->seekVirtual(position);
	}
	else
		position = g_fileManager->getVirtualAddress();

	if (!sizeExpression.evaluateInteger(areaSize))
	{
		Logger::queueError(Logger::Error,L"Invalid size expression");
		return false;
	}

	if (areaSize < 0)
	{
		Logger::queueError(Logger::Error, L"Negative area size");
		return false;
	}

	if (fillExpression.isLoaded())
	{
		if (!fillExpression.evaluateInteger(fillValue))
		{
			Logger::queueError(Logger::Error,L"Invalid fill expression");
			return false;
		}
	}

	bool result = false;
	if (content)
	{
		ValidateState contentValidation = state;
		contentValidation.noFileChange = true;
		contentValidation.noFileChangeDirective = L"area";
		content->applyFileInfo();
		result = content->Validate(contentValidation);
	}
	contentSize = g_fileManager->getVirtualAddress()-position;

	// restore info of this command
	applyFileInfo();

	if (areaSize < contentSize)
	{
		Logger::queueError(Logger::Error, L"Area at %08x overflowed by %d bytes", position, contentSize - areaSize);
	}

	if (fillExpression.isLoaded() || shared)
		g_fileManager->advanceMemory(areaSize-contentSize);

	if (areaSize != oldAreaSize || contentSize != oldContentSize)
		result = true;

	int64_t oldFileID = fileID;
	fileID = g_fileManager->getOpenFileID();

	if ((oldFileID != fileID || oldPosition != position || areaSize == 0) && oldAreaSize != 0)
		Allocations::forgetArea(oldFileID, oldPosition, oldAreaSize);
	if (areaSize != 0)
		Allocations::setArea(fileID, position, areaSize, contentSize, fillExpression.isLoaded(), shared);

	return result;
}

void CDirectiveArea::Encode() const
{
	if (positionExpression.isLoaded())
	{
		Arch->NextSection();
		g_fileManager->seekVirtual(position);
	}

	if (content)
		content->Encode();

	if (fillExpression.isLoaded())
	{
		int64_t subAreaUsage = Allocations::getSubAreaUsage(fileID, position);
		if (subAreaUsage != 0)
			g_fileManager->advanceMemory(subAreaUsage);

		unsigned char buffer[64];
		memset(buffer,fillValue,64);

		size_t writeSize = areaSize-contentSize-subAreaUsage;
		while (writeSize > 0)
		{
			size_t part = std::min<size_t>(64,writeSize);
			g_fileManager->write(buffer,part);
			writeSize -= part;
		}
	}
	else if (shared)
		g_fileManager->advanceMemory(areaSize-contentSize);
}

void CDirectiveArea::writeTempData(TempData& tempData) const
{
	const wchar_t *directiveType = shared ? L"region" : L"area";
	if (positionExpression.isLoaded())
		tempData.writeLine(position, tfm::format(L".org 0x%08llX", position));
	if (shared && fillExpression.isLoaded())
		tempData.writeLine(position,tfm::format(L".%S 0x%08X,0x%02x",directiveType,areaSize,fillValue));
	else
		tempData.writeLine(position,tfm::format(L".%S 0x%08X",directiveType,areaSize));
	if (content)
	{
		content->applyFileInfo();
		content->writeTempData(tempData);
	}

	if (fillExpression.isLoaded() && !shared)
	{
		int64_t subAreaUsage = Allocations::getSubAreaUsage(fileID, position);
		if (subAreaUsage != 0)
			tempData.writeLine(position+contentSize, tfm::format(L".skip 0x%08llX",subAreaUsage));

		std::wstring fillString = tfm::format(L".fill 0x%08X,0x%02X",areaSize-contentSize-subAreaUsage,fillValue);
		tempData.writeLine(position+contentSize+subAreaUsage,fillString);
		tempData.writeLine(position+areaSize,tfm::format(L".end%S",directiveType));
	} else {
		tempData.writeLine(position+contentSize,tfm::format(L".end%S",directiveType));
	}
}

void CDirectiveArea::writeSymData(SymbolData& symData) const
{
	if (content)
		content->writeSymData(symData);

	if (fillExpression.isLoaded())
	{
		int64_t subAreaUsage = Allocations::getSubAreaUsage(fileID, position);
		symData.addData(position+contentSize+subAreaUsage,areaSize-contentSize-subAreaUsage,SymbolData::Data8);
	}
}

CDirectiveAutoRegion::CDirectiveAutoRegion()
{
	this->contentSize = 0;
	this->resetPosition = 0;
	this->position = -1;

	this->content = nullptr;
}

void CDirectiveAutoRegion::setMinRangeExpression(Expression& exp)
{
	minRangeExpression = exp;
}

void CDirectiveAutoRegion::setRangeExpressions(Expression& minExp, Expression& maxExp)
{
	minRangeExpression = minExp;
	maxRangeExpression = maxExp;
}

bool CDirectiveAutoRegion::Validate(const ValidateState &state)
{
	resetPosition = g_fileManager->getVirtualAddress();

	ValidateState contentValidation = state;
	contentValidation.noFileChange = true;
	contentValidation.noFileChangeDirective = L"region";

	// We need at least one full pass run before we can get an address.
	if (state.passes < 1)
	{
		// Just calculate contentSize.
		position = g_fileManager->getVirtualAddress();
		content->applyFileInfo();
		content->Validate(contentValidation);
		contentSize = g_fileManager->getVirtualAddress() - position;

		g_fileManager->seekVirtual(resetPosition);
		return true;
	}

	int64_t oldPosition = position;
	int64_t oldContentSize = contentSize;

	int64_t minRange = -1;
	int64_t maxRange = -1;
	if (minRangeExpression.isLoaded())
	{
		if (!minRangeExpression.evaluateInteger(minRange))
		{
			Logger::queueError(Logger::Error, L"Invalid range expression for .autoregion");
			return false;
		}
	}
	if (maxRangeExpression.isLoaded())
	{
		if (!maxRangeExpression.evaluateInteger(maxRange))
		{
			Logger::queueError(Logger::Error, L"Invalid range expression for .autoregion");
			return false;
		}
	}

	fileID = g_fileManager->getOpenFileID();
	if (!Allocations::allocateSubArea(fileID, position, minRange, maxRange, contentSize))
	{
		Logger::queueError(Logger::Error, L"No space available for .autoregion of size %d", contentSize);
		// We might be able to do better next time.
		return Allocations::canTrimSpace();
	}

	Arch->NextSection();
	g_fileManager->seekVirtual(position);

	content->applyFileInfo();
	bool result = content->Validate(contentValidation);
	contentSize = g_fileManager->getVirtualAddress() - position;

	// restore info of this command
	applyFileInfo();
	g_fileManager->seekVirtual(resetPosition);

	if (position != oldPosition || contentSize != oldContentSize)
		result = true;

	return result;
}

void CDirectiveAutoRegion::Encode() const
{
	Arch->NextSection();
	g_fileManager->seekVirtual(position);
	content->Encode();
	g_fileManager->seekVirtual(resetPosition);
}

void CDirectiveAutoRegion::writeTempData(TempData& tempData) const
{
	tempData.writeLine(position,tfm::format(L".autoregion 0x%08X",position));
	content->applyFileInfo();
	content->writeTempData(tempData);
	tempData.writeLine(position+contentSize,L".endautoregion");
}

void CDirectiveAutoRegion::writeSymData(SymbolData& symData) const
{
	content->writeSymData(symData);
}

// file: Commands/CDirectiveConditional.h


enum class ConditionType
{
	IF,
	ELSE,
	ELSEIF,
	ENDIF,
	IFDEF,
	IFNDEF,
};

class CDirectiveConditional: public CAssemblerCommand
{
public:
	CDirectiveConditional(ConditionType type);
	CDirectiveConditional(ConditionType type, const std::wstring& name);
	CDirectiveConditional(ConditionType type, const Expression& exp);
	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
	void writeSymData(SymbolData& symData) const override;
	void setContent(std::unique_ptr<CAssemblerCommand> ifBlock, std::unique_ptr<CAssemblerCommand> elseBlock);
private:
	bool evaluate();

	Expression expression;
	std::shared_ptr<Label> label;
	bool previousResult;

	ConditionType type;
	std::unique_ptr<CAssemblerCommand> ifBlock;
	std::unique_ptr<CAssemblerCommand> elseBlock;
};

// file: Commands/CDirectiveConditional.cpp


#ifdef ARMIPS_ARM
extern CArmArchitecture Arm;
#endif

CDirectiveConditional::CDirectiveConditional(ConditionType type)
{
	this->type = type;

	ifBlock = nullptr;
	elseBlock = nullptr;
	previousResult = false;
}

CDirectiveConditional::CDirectiveConditional(ConditionType type, const std::wstring& name)
	: CDirectiveConditional(type)
{
	label = Global.symbolTable.getLabel(name,Global.FileInfo.FileNum,Global.Section);
	if (label == nullptr)
		Logger::printError(Logger::Error,L"Invalid label name \"%s\"",name);
}

CDirectiveConditional::CDirectiveConditional(ConditionType type, const Expression& exp)
	: CDirectiveConditional(type)
{
	this->expression = exp;
}

void CDirectiveConditional::setContent(std::unique_ptr<CAssemblerCommand> ifBlock, std::unique_ptr<CAssemblerCommand> elseBlock)
{
	this->ifBlock = std::move(ifBlock);
	this->elseBlock = std::move(elseBlock);
}

bool CDirectiveConditional::evaluate()
{
	int64_t value = 0;
	if (expression.isLoaded())
	{
		if (!expression.evaluateInteger(value))
		{
			Logger::queueError(Logger::Error,L"Invalid conditional expression");
			return false;
		}
	}

	switch (type)
	{
	case ConditionType::IF:
		return value != 0;
	case ConditionType::IFDEF:
		return label->isDefined();
	case ConditionType::IFNDEF:
		return !label->isDefined();
	default:
		break;
	}

	Logger::queueError(Logger::Error,L"Invalid conditional type");
	return false;
}

bool CDirectiveConditional::Validate(const ValidateState &state)
{
	bool result = evaluate();
	bool returnValue = result != previousResult;
	previousResult = result;

	if (result)
	{
		ifBlock->applyFileInfo();
		if (ifBlock->Validate(state))
			returnValue = true;
	} else if (elseBlock != nullptr)
	{
		elseBlock->applyFileInfo();
		if (elseBlock->Validate(state))
			returnValue = true;
	}

	return returnValue;
}

void CDirectiveConditional::Encode() const
{
	if (previousResult)
	{
		ifBlock->Encode();
	} else if (elseBlock != nullptr)
	{
		elseBlock->Encode();
	}
}

void CDirectiveConditional::writeTempData(TempData& tempData) const
{
	if (previousResult)
	{
		ifBlock->applyFileInfo();
		ifBlock->writeTempData(tempData);
	} else if (elseBlock != nullptr)
	{
		elseBlock->applyFileInfo();
		elseBlock->writeTempData(tempData);
	}
}

void CDirectiveConditional::writeSymData(SymbolData& symData) const
{
	if (previousResult)
	{
		ifBlock->writeSymData(symData);
	} else if (elseBlock != nullptr)
	{
		elseBlock->writeSymData(symData);
	}
}

// file: Commands/CDirectiveData.h


enum class EncodingMode { Invalid, U8, U16, U32, U64, Ascii, Float, Double, Sjis, Custom };

class TableCommand: public CAssemblerCommand
{
public:
	TableCommand(const std::wstring& fileName, TextFile::Encoding encoding);
	bool Validate(const ValidateState &state) override;
	void Encode() const override { };
	void writeTempData(TempData& tempData) const override { };
	void writeSymData(SymbolData& symData) const override { };
private:
	EncodingTable table;
};

class CDirectiveData: public CAssemblerCommand
{
public:
	CDirectiveData();
	~CDirectiveData();
	void setNormal(std::vector<Expression>& entries, size_t unitSize);
	void setFloat(std::vector<Expression>& entries);
	void setDouble(std::vector<Expression>& entries);
	void setAscii(std::vector<Expression>& entries, bool terminate);
	void setSjis(std::vector<Expression>& entries, bool terminate);
	void setCustom(std::vector<Expression>& entries, bool terminate);
	bool Validate(const ValidateState &state) override;
	void Encode() const override;
	void writeTempData(TempData& tempData) const override;
	void writeSymData(SymbolData& symData) const override;
private:
	void encodeCustom(EncodingTable& table);
	void encodeSjis();
	void encodeFloat();
	void encodeDouble();
	void encodeNormal();
	size_t getUnitSize() const;
	size_t getDataSize() const;

	int64_t position;
	EncodingMode mode;
	bool writeTermination;
	std::vector<Expression> entries;
	ByteArray customData;
	std::vector<int64_t> normalData;
	Endianness endianness;
};

// file: Commands/CDirectiveData.cpp


//
// TableCommand
//

TableCommand::TableCommand(const std::wstring& fileName, TextFile::Encoding encoding)
{
	auto fullName = getFullPathName(fileName);

	if (!fs::exists(fullName))
	{
		Logger::printError(Logger::Error,L"Table file \"%s\" does not exist",fileName);
		return;
	}

	if (!table.load(fullName,encoding))
	{
		Logger::printError(Logger::Error,L"Invalid table file \"%s\"",fileName);
		return;
	}
}

bool TableCommand::Validate(const ValidateState &state)
{
	Global.Table = table;
	return false;
}


//
// CDirectiveData
//

CDirectiveData::CDirectiveData()
{
	mode = EncodingMode::Invalid;
	writeTermination = false;
	endianness = Arch->getEndianness();
}

CDirectiveData::~CDirectiveData()
{

}

void CDirectiveData::setNormal(std::vector<Expression>& entries, size_t unitSize)
{
	switch (unitSize)
	{
	case 1:
		this->mode = EncodingMode::U8;
		break;
	case 2:
		this->mode = EncodingMode::U16;
		break;
	case 4:
		this->mode = EncodingMode::U32;
		break;
	case 8:
		this->mode = EncodingMode::U64;
		break;
	default:
		Logger::printError(Logger::Error,L"Invalid data unit size %d",unitSize);
		return;
	}

	this->entries = entries;
	this->writeTermination = false;
	normalData.reserve(entries.size());
}

void CDirectiveData::setFloat(std::vector<Expression>& entries)
{
	this->mode = EncodingMode::Float;
	this->entries = entries;
	this->writeTermination = false;
}

void CDirectiveData::setDouble(std::vector<Expression>& entries)
{
	this->mode = EncodingMode::Double;
	this->entries = entries;
	this->writeTermination = false;
}

void CDirectiveData::setAscii(std::vector<Expression>& entries, bool terminate)
{
	this->mode = EncodingMode::Ascii;
	this->entries = entries;
	this->writeTermination = terminate;
}

void CDirectiveData::setSjis(std::vector<Expression>& entries, bool terminate)
{
	this->mode = EncodingMode::Sjis;
	this->entries = entries;
	this->writeTermination = terminate;
}

void CDirectiveData::setCustom(std::vector<Expression>& entries, bool terminate)
{
	this->mode = EncodingMode::Custom;
	this->entries = entries;
	this->writeTermination = terminate;
}

size_t CDirectiveData::getUnitSize() const
{
	switch (mode)
	{
	case EncodingMode::U8:
	case EncodingMode::Ascii:
	case EncodingMode::Sjis:
	case EncodingMode::Custom:
		return 1;
	case EncodingMode::U16:
		return 2;
	case EncodingMode::U32:
	case EncodingMode::Float:
		return 4;
	case EncodingMode::U64:
	case EncodingMode::Double:
		return 8;
	case EncodingMode::Invalid:
		break;
	}

	return 0;
}

size_t CDirectiveData::getDataSize() const
{
	switch (mode)
	{
	case EncodingMode::Sjis:
	case EncodingMode::Custom:
		return customData.size();
	case EncodingMode::U8:
	case EncodingMode::Ascii:
	case EncodingMode::U16:
	case EncodingMode::U32:
	case EncodingMode::U64:
	case EncodingMode::Float:
	case EncodingMode::Double:
		return normalData.size()*getUnitSize();
	case EncodingMode::Invalid:
		break;
	}

	return 0;
}

void CDirectiveData::encodeCustom(EncodingTable& table)
{
	customData.clear();
	for (size_t i = 0; i < entries.size(); i++)
	{
		ExpressionValue value = entries[i].evaluate();
		if (!value.isValid())
		{
			Logger::queueError(Logger::Error,L"Invalid expression");
			continue;
		}

		if (value.isInt())
		{
			customData.appendByte((byte)value.intValue);
		} else if (value.isString())
		{
			ByteArray encoded = table.encodeString(value.strValue,false);
			if (encoded.size() == 0 && value.strValue.size() > 0)
			{
				Logger::queueError(Logger::Error,L"Failed to encode \"%s\"",value.strValue);
			}
			customData.append(encoded);
		} else {
			Logger::queueError(Logger::Error,L"Invalid expression type");
		}
	}

	if (writeTermination)
	{
		ByteArray encoded = table.encodeTermination();
		customData.append(encoded);
	}
}

void CDirectiveData::encodeSjis()
{
	static EncodingTable sjisTable;
	if (!sjisTable.isLoaded())
	{
		unsigned char hexBuffer[2];

		sjisTable.setTerminationEntry((unsigned char*)"\0",1);

		for (unsigned short SJISValue = 0x0001; SJISValue < 0x0100; SJISValue++)
		{
			wchar_t unicodeValue = sjisToUnicode(SJISValue);
			if (unicodeValue != 0xFFFF)
			{
				hexBuffer[0] = SJISValue & 0xFF;
				sjisTable.addEntry(hexBuffer, 1, unicodeValue);
			}
		}
		for (unsigned short SJISValue = 0x8100; SJISValue < 0xEF00; SJISValue++)
		{
			wchar_t unicodeValue = sjisToUnicode(SJISValue);
			if (unicodeValue != 0xFFFF)
			{
				hexBuffer[0] = (SJISValue >> 8) & 0xFF;
				hexBuffer[1] = SJISValue & 0xFF;
				sjisTable.addEntry(hexBuffer, 2, unicodeValue);
			}
		}
	}

	encodeCustom(sjisTable);
}

void CDirectiveData::encodeFloat()
{
	normalData.clear();
	for (size_t i = 0; i < entries.size(); i++)
	{
		ExpressionValue value = entries[i].evaluate();
		if (!value.isValid())
		{
			Logger::queueError(Logger::Error,L"Invalid expression");
			continue;
		}

		if (value.isInt() && mode == EncodingMode::Float)
		{
			int32_t num = getFloatBits((float)value.intValue);
			normalData.push_back(num);
		} else if (value.isInt() && mode == EncodingMode::Double)
		{
			int64_t num = getDoubleBits((double)value.intValue);
			normalData.push_back(num);
		} else if (value.isFloat() && mode == EncodingMode::Float)
		{
			int32_t num = getFloatBits((float)value.floatValue);
			normalData.push_back(num);
		} else if (value.isFloat() && mode == EncodingMode::Double)
		{
			int64_t num = getDoubleBits((double)value.floatValue);
			normalData.push_back(num);
		} else {
			Logger::queueError(Logger::Error,L"Invalid expression type");
		}
	}
}

void CDirectiveData::encodeNormal()
{
	normalData.clear();
	for (size_t i = 0; i < entries.size(); i++)
	{
		ExpressionValue value = entries[i].evaluate();
		if (!value.isValid())
		{
			Logger::queueError(Logger::Error,L"Invalid expression");
			continue;
		}

		if (value.isString())
		{
			bool hadNonAscii = false;
			for (size_t l = 0; l < value.strValue.size(); l++)
			{
				int64_t num = value.strValue[l];
				normalData.push_back(num);

				if (num >= 0x80 && !hadNonAscii)
				{
					Logger::printError(Logger::Warning,L"Non-ASCII character in data directive. Use .string instead");
					hadNonAscii = true;
				}
			}
		} else if (value.isInt())
		{
			int64_t num = value.intValue;
			normalData.push_back(num);
		} else if (value.isFloat() && mode == EncodingMode::U32)
		{
			int32_t num = getFloatBits((float)value.floatValue);
			normalData.push_back(num);
		} else if(value.isFloat() && mode == EncodingMode::U64) {
			int64_t num = getDoubleBits((double)value.floatValue);
			normalData.push_back(num);
		} else {
			Logger::queueError(Logger::Error,L"Invalid expression type");
		}
	}

	if (writeTermination)
	{
		normalData.push_back(0);
	}
}

bool CDirectiveData::Validate(const ValidateState &state)
{
	position = g_fileManager->getVirtualAddress();

	size_t oldSize = getDataSize();
	switch (mode)
	{
	case EncodingMode::U8:
	case EncodingMode::U16:
	case EncodingMode::U32:
	case EncodingMode::U64:
	case EncodingMode::Ascii:
		encodeNormal();
		break;
	case EncodingMode::Float:
	case EncodingMode::Double:
		encodeFloat();
		break;
	case EncodingMode::Sjis:
		encodeSjis();
		break;
	case EncodingMode::Custom:
		encodeCustom(Global.Table);
		break;
	default:
		Logger::queueError(Logger::Error,L"Invalid encoding type");
		break;
	}

	g_fileManager->advanceMemory(getDataSize());
	return oldSize != getDataSize();
}

void CDirectiveData::Encode() const
{
	switch (mode)
	{
	case EncodingMode::Sjis:
	case EncodingMode::Custom:
		g_fileManager->write(customData.data(),customData.size());
		break;
	case EncodingMode::U8:
	case EncodingMode::Ascii:
		for (auto value: normalData)
		{
			g_fileManager->writeU8((uint8_t)value);
		}
		break;
	case EncodingMode::U16:
		for (auto value: normalData)
		{
			g_fileManager->writeU16((uint16_t)value);
		}
		break;
	case EncodingMode::U32:
	case EncodingMode::Float:
		for (auto value: normalData)
		{
			g_fileManager->writeU32((uint32_t)value);
		}
		break;
	case EncodingMode::U64:
	case EncodingMode::Double:
		for (auto value: normalData)
		{
			g_fileManager->writeU64((uint64_t)value);
		}
		break;
	case EncodingMode::Invalid:
		// TODO: Assert?
		break;
	}
}

void CDirectiveData::writeTempData(TempData& tempData) const
{
	size_t size = (getUnitSize()*2+3)*getDataSize()+20;
	wchar_t* str = new wchar_t[size];
	wchar_t* start = str;

	switch (mode)
	{
	case EncodingMode::Sjis:
	case EncodingMode::Custom:
		str += swprintf(str,20,L".byte ");

		for (size_t i = 0; i < customData.size(); i++)
		{
			str += swprintf(str,20,L"0x%02X,",(uint8_t)customData[i]);
		}
		break;
	case EncodingMode::U8:
	case EncodingMode::Ascii:
		str += swprintf(str,20,L".byte ");

		for (size_t i = 0; i < normalData.size(); i++)
		{
			str += swprintf(str,20,L"0x%02X,",(uint8_t)normalData[i]);
		}
		break;
	case EncodingMode::U16:
		str += swprintf(str,20,L".halfword ");

		for (size_t i = 0; i < normalData.size(); i++)
		{
			str += swprintf(str,20,L"0x%04X,",(uint16_t)normalData[i]);
		}
		break;
	case EncodingMode::U32:
	case EncodingMode::Float:
		str += swprintf(str,20,L".word ");

		for (size_t i = 0; i < normalData.size(); i++)
		{
			str += swprintf(str,20,L"0x%08X,",(uint32_t)normalData[i]);
		}
		break;
	case EncodingMode::U64:
	case EncodingMode::Double:
		str += swprintf(str,20,L".doubleword ");

		for (size_t i = 0; i < normalData.size(); i++)
		{
			str += swprintf(str,20,L"0x%16llX,",(uint64_t)normalData[i]);
		}
		break;
	case EncodingMode::Invalid:
		// TODO: Assert?
		break;
	}

	*(str-1) = 0;
	tempData.writeLine(position,start);
	delete[] start;
}

void CDirectiveData::writeSymData(SymbolData& symData) const
{
	switch (mode)
	{
	case EncodingMode::Ascii:
		symData.addData(position,getDataSize(),SymbolData::DataAscii);
		break;
	case EncodingMode::U8:
	case EncodingMode::Sjis:
	case EncodingMode::Custom:
		symData.addData(position,getDataSize(),SymbolData::Data8);
		break;
	case EncodingMode::U16:
		symData.addData(position,getDataSize(),SymbolData::Data16);
		break;
	case EncodingMode::U32:
	case EncodingMode::Float:
		symData.addData(position,getDataSize(),SymbolData::Data32);
		break;
	case EncodingMode::U64:
	case EncodingMode::Double:
		symData.addData(position,getDataSize(),SymbolData::Data64);
		break;
	case EncodingMode::Invalid:
		// TODO: Assert?
		break;
	}
}

// file: Commands/CDirectiveFile.cpp


#include <cstring>

//
// CDirectiveFile
//

CDirectiveFile::CDirectiveFile()
{
	type = Type::Invalid;
	file = nullptr;
}

void CDirectiveFile::initOpen(const fs::path& fileName, int64_t memory)
{
	type = Type::Open;
	fs::path fullName = getFullPathName(fileName);

	file = std::make_shared<GenericAssemblerFile>(fullName,memory,false);
	g_fileManager->addFile(file);

	updateSection(++Global.Section);
}

void CDirectiveFile::initCreate(const fs::path& fileName, int64_t memory)
{
	type = Type::Create;
	fs::path fullName = getFullPathName(fileName);

	file = std::make_shared<GenericAssemblerFile>(fullName,memory,true);
	g_fileManager->addFile(file);

	updateSection(++Global.Section);
}

void CDirectiveFile::initCopy(const fs::path& inputName, const fs::path& outputName, int64_t memory)
{
	type = Type::Copy;
	fs::path fullInputName = getFullPathName(inputName);
	fs::path fullOutputName = getFullPathName(outputName);

	file = std::make_shared<GenericAssemblerFile>(fullOutputName,fullInputName,memory);
	g_fileManager->addFile(file);

	updateSection(++Global.Section);
}

void CDirectiveFile::initClose()
{
	type = Type::Close;
	updateSection(++Global.Section);
}

bool CDirectiveFile::Validate(const ValidateState &state)
{
	if (state.noFileChange)
	{
		if (type == Type::Close)
			Logger::queueError(Logger::Error, L"Cannot close file within %S", state.noFileChangeDirective);
		else
			Logger::queueError(Logger::Error, L"Cannot open new file within %S", state.noFileChangeDirective);
		return false;
	}

	virtualAddress = g_fileManager->getVirtualAddress();
	Arch->NextSection();

	switch (type)
	{
	case Type::Open:
	case Type::Create:
	case Type::Copy:
		g_fileManager->openFile(file,true);
		return false;
	case Type::Close:
		closeFile = g_fileManager->getOpenFile();
		g_fileManager->closeFile();
		return false;
	case Type::Invalid:
		break;
	}

	return false;
}

void CDirectiveFile::Encode() const
{
	switch (type)
	{
	case Type::Open:
	case Type::Create:
	case Type::Copy:
		g_fileManager->openFile(file,false);
		break;
	case Type::Close:
		g_fileManager->closeFile();
		break;
	case Type::Invalid:
		// TODO: Assert?
		break;
	}
}

void CDirectiveFile::writeTempData(TempData& tempData) const
{
	std::wstring str;

	switch (type)
	{
	case Type::Open:
		str = tfm::format(L".open \"%s\",0x%08X",file->getFileName().wstring(),file->getOriginalHeaderSize());
		break;
	case Type::Create:
		str = tfm::format(L".create \"%s\",0x%08X",file->getFileName().wstring(),file->getOriginalHeaderSize());
		break;
	case Type::Copy:
		str = tfm::format(L".open \"%s\",\"%s\",0x%08X",file->getOriginalFileName().wstring(),
			file->getFileName().wstring(),file->getOriginalHeaderSize());
		break;
	case Type::Close:
		str = L".close";
		break;
	case Type::Invalid:
		// TODO: Assert?
		break;
	}

	tempData.writeLine(virtualAddress,str);
}

void CDirectiveFile::writeSymData(SymbolData& symData) const
{
	switch (type)
	{
	case Type::Open:
	case Type::Create:
	case Type::Copy:
		file->beginSymData(symData);
		break;
	case Type::Close:
		if (closeFile)
			closeFile->endSymData(symData);
		break;
	case Type::Invalid:
		// TODO: Assert?
		break;
	}
}

//
// CDirectivePosition
//

CDirectivePosition::CDirectivePosition(Expression expression, Type type)
	: expression(expression), type(type)
{
	updateSection(++Global.Section);
}

void CDirectivePosition::exec() const
{
	switch (type)
	{
	case Physical:
		g_fileManager->seekPhysical(position);
		break;
	case Virtual:
		g_fileManager->seekVirtual(position);
		break;
	}
}

bool CDirectivePosition::Validate(const ValidateState &state)
{
	virtualAddress = g_fileManager->getVirtualAddress();

	if (!expression.evaluateInteger(position))
	{
		Logger::queueError(Logger::FatalError,L"Invalid position");
		return false;
	}

	Arch->NextSection();
	exec();
	return false;
}

void CDirectivePosition::Encode() const
{
	Arch->NextSection();
	exec();
}

void CDirectivePosition::writeTempData(TempData& tempData) const
{
	switch (type)
	{
	case Physical:
		tempData.writeLine(virtualAddress,tfm::format(L".orga 0x%08X",position));
		break;
	case Virtual:
		tempData.writeLine(virtualAddress,tfm::format(L".org 0x%08X",position));
		break;
	}
}

//
// CDirectiveIncbin
//

CDirectiveIncbin::CDirectiveIncbin(const fs::path& fileName)
	: size(0), start(0)
{
	this->fileName = getFullPathName(fileName);

	if (!fs::exists(this->fileName))
	{
		Logger::printError(Logger::FatalError,L"File %s not found",this->fileName.wstring());
	}

	std::error_code error;
	this->fileSize = static_cast<int64_t>(fs::file_size(fileName, error));
}

bool CDirectiveIncbin::Validate(const ValidateState &state)
{
	virtualAddress = g_fileManager->getVirtualAddress();

	if (startExpression.isLoaded())
	{
		if (!startExpression.evaluateInteger(start))
		{
			Logger::queueError(Logger::Error,L"Invalid position expression");
			return false;
		}

		if (start > fileSize)
		{
			Logger::queueError(Logger::Error,L"Start position past end of file");
			return false;
		}
	} else {
		start = 0;
	}

	if (sizeExpression.isLoaded())
	{
		if (!sizeExpression.evaluateInteger(size))
		{
			Logger::queueError(Logger::Error,L"Invalid size expression");
			return false;
		}
	} else {
		size = fileSize-start;
	}

	if (start+size > fileSize)
	{
		Logger::queueError(Logger::Warning,L"Read size truncated due to file size");
		size = fileSize-start;
	}

	Arch->NextSection();
	g_fileManager->advanceMemory(size);
	return false;
}

void CDirectiveIncbin::Encode() const
{
	if (size != 0)
	{
		ByteArray data = ByteArray::fromFile(fileName,(long)start,size);
		if ((int) data.size() != size)
		{
			Logger::printError(Logger::Error,L"Could not read file \"%s\"",fileName.wstring());
			return;
		}
		g_fileManager->write(data.data(),data.size());
	}
}

void CDirectiveIncbin::writeTempData(TempData& tempData) const
{
	tempData.writeLine(virtualAddress,tfm::format(L".incbin \"%s\"",fileName.wstring()));
}

void CDirectiveIncbin::writeSymData(SymbolData& symData) const
{
	symData.addData(virtualAddress,size,SymbolData::Data8);
}


//
// CDirectiveAlignFill
//

CDirectiveAlignFill::CDirectiveAlignFill(int64_t value, Mode mode)
{
	this->mode = mode;
	this->value = value;
	this->finalSize = 0;
	this->fillByte = 0;
}

CDirectiveAlignFill::CDirectiveAlignFill(Expression& value, Mode mode)
	: CDirectiveAlignFill(0,mode)
{
	valueExpression = value;
}

CDirectiveAlignFill::CDirectiveAlignFill(Expression& value, Expression& fillValue, Mode mode)
	: CDirectiveAlignFill(value,mode)
{
	fillExpression = fillValue;
}

bool CDirectiveAlignFill::Validate(const ValidateState &state)
{
	virtualAddress = g_fileManager->getVirtualAddress();

	if (valueExpression.isLoaded())
	{
		if (!valueExpression.evaluateInteger(value))
		{
			Logger::queueError(Logger::FatalError,L"Invalid %s",mode == Fill ? L"size" : L"alignment");
			return false;
		}
	}

	if (mode != Fill && !isPowerOfTwo(value))
	{
		Logger::queueError(Logger::Error, L"Invalid alignment %d", value);
		return false;
	}

	int64_t oldSize = finalSize;
	int64_t mod;
	switch (mode)
	{
	case AlignVirtual:
		mod = g_fileManager->getVirtualAddress() % value;
		finalSize = mod ? value-mod : 0;
		break;
	case AlignPhysical:
		mod = g_fileManager->getPhysicalAddress() % value;
		finalSize = mod ? value-mod : 0;
		break;
	case Fill:
		finalSize = value;
		break;
	}

	if (fillExpression.isLoaded())
	{
		if (!fillExpression.evaluateInteger(fillByte))
		{
			Logger::printError(Logger::FatalError,L"Invalid fill value");
			return false;
		}
	}

	Arch->NextSection();
	g_fileManager->advanceMemory(finalSize);

	bool result = oldSize != finalSize;
	oldSize = finalSize;
	return result;
}

void CDirectiveAlignFill::Encode() const
{
	unsigned char buffer[128];
	int64_t n = finalSize;

	memset(buffer,fillByte,n > 128 ? 128 : n);
	while (n > 128)
	{
		g_fileManager->write(buffer,128);
		n -= 128;
	}

	g_fileManager->write(buffer,n);
}

void CDirectiveAlignFill::writeTempData(TempData& tempData) const
{
	switch (mode)
	{
	case AlignVirtual:
		tempData.writeLine(virtualAddress,tfm::format(L".align 0x%08X",value));
		break;
	case AlignPhysical:
		tempData.writeLine(virtualAddress, tfm::format(L".aligna 0x%08X", value));
		break;
	case Fill:
		tempData.writeLine(virtualAddress,tfm::format(L".fill 0x%08X,0x%02X",value,fillByte));
		break;
	}
}

void CDirectiveAlignFill::writeSymData(SymbolData& symData) const
{
	switch (mode)
	{
	case AlignVirtual:	// ?
	case AlignPhysical:	// ?
		break;
	case Fill:
		symData.addData(virtualAddress,value,SymbolData::Data8);
		break;
	}
}

//
// CDirectiveSkip
//

CDirectiveSkip::CDirectiveSkip(Expression& expression)
	: expression(expression) {}

bool CDirectiveSkip::Validate(const ValidateState &state)
{
	virtualAddress = g_fileManager->getVirtualAddress();

	if (expression.isLoaded())
	{
		if (!expression.evaluateInteger(value))
		{
			Logger::queueError(Logger::FatalError,L"Invalid skip length");
			return false;
		}
	}

	Arch->NextSection();
	g_fileManager->advanceMemory(value);

	return false;
}

void CDirectiveSkip::Encode() const
{
	Arch->NextSection();
	g_fileManager->advanceMemory(value);
}

void CDirectiveSkip::writeTempData(TempData& tempData) const
{
	tempData.writeLine(virtualAddress,tfm::format(L".skip 0x%08X",value));
}

//
// CDirectiveHeaderSize
//

CDirectiveHeaderSize::CDirectiveHeaderSize(Expression expression)
	: expression(expression) {}

void CDirectiveHeaderSize::exec() const
{
	std::shared_ptr<AssemblerFile> openFile = g_fileManager->getOpenFile();
	if (!openFile->hasFixedVirtualAddress())
	{
		Logger::printError(Logger::Error,L"Header size not applicable for this file");
		return;
	}
	std::shared_ptr<GenericAssemblerFile> file = std::static_pointer_cast<GenericAssemblerFile>(openFile);
	int64_t physicalAddress = file->getPhysicalAddress();
	file->setHeaderSize(headerSize);
	file->seekPhysical(physicalAddress);
}

bool CDirectiveHeaderSize::Validate(const ValidateState &state)
{
	virtualAddress = g_fileManager->getVirtualAddress();

	if (!expression.evaluateInteger(headerSize))
	{
		Logger::queueError(Logger::FatalError,L"Invalid header size");
		return false;
	}

	exec();
	return false;
}

void CDirectiveHeaderSize::Encode() const
{
	exec();
}

void CDirectiveHeaderSize::writeTempData(TempData& tempData) const
{
	tempData.writeLine(virtualAddress,tfm::format(L".headersize %s0x%08X",
		headerSize < 0 ? L"-" : L"", headerSize < 0 ? -headerSize : headerSize));
}


//
// DirectiveObjImport
//

DirectiveObjImport::DirectiveObjImport(const fs::path& inputName)
{
	ctor = nullptr;
	if (rel.init(inputName))
	{
		rel.exportSymbols();
	}
}

DirectiveObjImport::DirectiveObjImport(const fs::path& inputName, const std::wstring& ctorName)
{
	if (rel.init(inputName))
	{
		rel.exportSymbols();
		ctor = rel.generateCtor(ctorName);
	}
}

bool DirectiveObjImport::Validate(const ValidateState &state)
{
	bool result = false;
	if (ctor != nullptr && ctor->Validate(state))
		result = true;

	int64_t memory = g_fileManager->getVirtualAddress();
	rel.relocate(memory);
	g_fileManager->advanceMemory((size_t)memory);

	return rel.hasDataChanged() || result;
}

void DirectiveObjImport::Encode() const
{
	if (ctor != nullptr)
		ctor->Encode();

	const ByteArray& data = rel.getData();
	g_fileManager->write(data.data(),data.size());
}

void DirectiveObjImport::writeTempData(TempData& tempData) const
{
	if (ctor != nullptr)
		ctor->writeTempData(tempData);
}

void DirectiveObjImport::writeSymData(SymbolData& symData) const
{
	if (ctor != nullptr)
		ctor->writeSymData(symData);

	rel.writeSymbols(symData);
}

// file: Commands/CDirectiveMessage.h


class CDirectiveMessage: public CAssemblerCommand
{
public:
	enum class Type { Warning, Error, Notice };
	CDirectiveMessage(Type type, Expression exp);
	bool Validate(const ValidateState &state) override;
	void Encode() const override {};
	void writeTempData(TempData& tempData) const override { };
private:
	Type errorType;
	Expression exp;
};

class CDirectiveSym: public CAssemblerCommand
{
public:
	CDirectiveSym(bool enable) {enabled = enable; };
	bool Validate(const ValidateState &state) override { return false; }
	void Encode() const override { };
	void writeTempData(TempData& tempData) const override { };
	void writeSymData(SymbolData& symData) const override;
private:
	bool enabled;
};

// file: Commands/CDirectiveMessage.cpp


CDirectiveMessage::CDirectiveMessage(Type type, Expression exp)
{
	errorType = type;
	this->exp = exp;
}

bool CDirectiveMessage::Validate(const ValidateState &state)
{
	std::wstring text;
	if (!exp.evaluateString(text,true))
	{
		Logger::queueError(Logger::Error,L"Invalid expression");
		return false;
	}

	switch (errorType)
	{
	case Type::Warning:
		Logger::queueError(Logger::Warning,text);
		break;
	case Type::Error:
		Logger::queueError(Logger::Error,text);
		break;
	case Type::Notice:
		Logger::queueError(Logger::Notice,text);
		break;
	}
	return false;
}

void CDirectiveSym::writeSymData(SymbolData &symData) const
{
	symData.setEnabled(enabled);
}

// file: Commands/CommandSequence.cpp

CommandSequence::CommandSequence()
	: CAssemblerCommand()
{

}

bool CommandSequence::Validate(const ValidateState &state)
{
	bool result = false;

	for (const std::unique_ptr<CAssemblerCommand>& cmd: commands)
	{
		cmd->applyFileInfo();
		if (cmd->Validate(state))
			result = true;
	}

	return result;
}

void CommandSequence::Encode() const
{
	for (const std::unique_ptr<CAssemblerCommand>& cmd: commands)
	{
		cmd->Encode();
	}
}

void CommandSequence::writeTempData(TempData& tempData) const
{
	for (const std::unique_ptr<CAssemblerCommand>& cmd: commands)
	{
		cmd->applyFileInfo();
		cmd->writeTempData(tempData);
	}
}

void CommandSequence::writeSymData(SymbolData& symData) const
{
	for (const std::unique_ptr<CAssemblerCommand>& cmd: commands)
	{
		cmd->writeSymData(symData);
	}
}

// file: Parser/DirectivesParser.cpp


#include <algorithm>
#include <initializer_list>

std::unique_ptr<CAssemblerCommand> parseDirectiveOpen(Parser& parser, int flags)
{
	std::vector<Expression> list;
	if (!parser.parseExpressionList(list,2,3))
		return nullptr;

	int64_t memoryAddress;
	std::wstring inputName, outputName;

	if (!list[0].evaluateString(inputName,false))
		return nullptr;

	if (!list.back().evaluateInteger(memoryAddress))
		return nullptr;

	auto file = std::make_unique<CDirectiveFile>();
	if (list.size() == 3)
	{
		if (!list[1].evaluateString(outputName,false))
			return nullptr;

		file->initCopy(inputName,outputName,memoryAddress);
		return file;
	} else {
		file->initOpen(inputName,memoryAddress);
		return file;
	}
}

std::unique_ptr<CAssemblerCommand> parseDirectiveCreate(Parser& parser, int flags)
{
	std::vector<Expression> list;
	if (!parser.parseExpressionList(list,2,2))
		return nullptr;

	int64_t memoryAddress;
	std::wstring inputName, outputName;

	if (!list[0].evaluateString(inputName,false))
		return nullptr;

	if (!list.back().evaluateInteger(memoryAddress))
		return nullptr;

	auto file = std::make_unique<CDirectiveFile>();
	file->initCreate(inputName,memoryAddress);
	return file;
}

std::unique_ptr<CAssemblerCommand> parseDirectiveClose(Parser& parser, int flags)
{
	auto file = std::make_unique<CDirectiveFile>();
	file->initClose();
	return file;
}

std::unique_ptr<CAssemblerCommand> parseDirectiveIncbin(Parser& parser, int flags)
{
	std::vector<Expression> list;
	if (!parser.parseExpressionList(list,1,3))
		return nullptr;

	std::wstring fileName;
	if (!list[0].evaluateString(fileName,false))
		return nullptr;

	auto incbin = std::make_unique<CDirectiveIncbin>(fileName);
	if (list.size() >= 2)
		incbin->setStart(list[1]);

	if (list.size() == 3)
		incbin->setSize(list[2]);

	return incbin;
}

std::unique_ptr<CAssemblerCommand> parseDirectivePosition(Parser& parser, int flags)
{
	Expression exp = parser.parseExpression();
	if (!exp.isLoaded())
		return nullptr;

	CDirectivePosition::Type type;
	switch (flags & DIRECTIVE_USERMASK)
	{
	case DIRECTIVE_POS_PHYSICAL:
		type = CDirectivePosition::Physical;
		break;
	case DIRECTIVE_POS_VIRTUAL:
		type = CDirectivePosition::Virtual;
		break;
	default:
		return nullptr;
	}

	return std::make_unique<CDirectivePosition>(exp,type);
}

std::unique_ptr<CAssemblerCommand> parseDirectiveAlignFill(Parser& parser, int flags)
{
	CDirectiveAlignFill::Mode mode;
	switch (flags & DIRECTIVE_USERMASK)
	{
	case DIRECTIVE_ALIGN_VIRTUAL:
		mode = CDirectiveAlignFill::AlignVirtual;
		break;
	case DIRECTIVE_ALIGN_PHYSICAL:
		mode = CDirectiveAlignFill::AlignPhysical;
		break;
	case DIRECTIVE_ALIGN_FILL:
		mode = CDirectiveAlignFill::Fill;
		break;
	default:
		return nullptr;
	}

	if (mode != CDirectiveAlignFill::Fill && parser.peekToken().type == TokenType::Separator)
		return std::make_unique<CDirectiveAlignFill>(UINT64_C(4),mode);

	std::vector<Expression> list;
	if (!parser.parseExpressionList(list,1,2))
		return nullptr;

	if (list.size() == 2)
		return std::make_unique<CDirectiveAlignFill>(list[0],list[1],mode);
	else
		return std::make_unique<CDirectiveAlignFill>(list[0],mode);
}

std::unique_ptr<CAssemblerCommand> parseDirectiveSkip(Parser& parser, int flags)
{
	std::vector<Expression> list;
	if (!parser.parseExpressionList(list,1,1))
		return nullptr;

	return std::make_unique<CDirectiveSkip>(list[0]);
}

std::unique_ptr<CAssemblerCommand> parseDirectiveHeaderSize(Parser& parser, int flags)
{
	Expression exp = parser.parseExpression();
	if (!exp.isLoaded())
		return nullptr;

	return std::make_unique<CDirectiveHeaderSize>(exp);
}

std::unique_ptr<CAssemblerCommand> parseDirectiveObjImport(Parser& parser, int flags)
{
	std::vector<Expression> list;
	if (!parser.parseExpressionList(list,1,2))
		return nullptr;

	std::wstring fileName;
	if (!list[0].evaluateString(fileName,true))
		return nullptr;

	if (list.size() == 2)
	{
		std::wstring ctorName;
		if (!list[1].evaluateIdentifier(ctorName))
			return nullptr;

		return std::make_unique<DirectiveObjImport>(fileName,ctorName);
	}

	return std::make_unique<DirectiveObjImport>(fileName);
}

std::unique_ptr<CAssemblerCommand> parseDirectiveConditional(Parser& parser, int flags)
{
	ConditionType type;
	std::wstring name;
	Expression exp;

	const Token& start = parser.peekToken();
	ConditionalResult condResult = ConditionalResult::Unknown;
	switch (flags)
	{
	case DIRECTIVE_COND_IF:
		type = ConditionType::IF;
		exp = parser.parseExpression();
		if (!exp.isLoaded())
		{
			parser.printError(start,L"Invalid condition");
			return std::make_unique<DummyCommand>();
		}

		if (exp.isConstExpression())
		{
			ExpressionValue result = exp.evaluate();
			if (result.isInt())
				condResult = result.intValue != 0 ? ConditionalResult::True : ConditionalResult::False;
		}
		break;
	case DIRECTIVE_COND_IFDEF:
		type = ConditionType::IFDEF;
		if (!parser.parseIdentifier(name))
			return nullptr;
		break;
	case DIRECTIVE_COND_IFNDEF:
		type = ConditionType::IFNDEF;
		if (!parser.parseIdentifier(name))
			return nullptr;
		break;
	}

	if(parser.nextToken().type != TokenType::Separator)
	{
		parser.printError(start,L"Directive not terminated");
		return nullptr;
	}

	parser.pushConditionalResult(condResult);
	std::unique_ptr<CAssemblerCommand> ifBlock = parser.parseCommandSequence(L'.', {L".else", L".elseif", L".elseifdef", L".elseifndef", L".endif"});
	parser.popConditionalResult();

	// update the file info so that else commands get the right line number
	parser.updateFileInfo();

	std::unique_ptr<CAssemblerCommand> elseBlock = nullptr;
	const Token &next = parser.nextToken();
	const std::wstring stringValue = next.getStringValue();

	ConditionalResult elseResult;
	switch (condResult)
	{
	case ConditionalResult::True:
		elseResult = ConditionalResult::False;
		break;
	case ConditionalResult::False:
		elseResult = ConditionalResult::True;
		break;
	case ConditionalResult::Unknown:
		elseResult = condResult;
		break;
	}

	parser.pushConditionalResult(elseResult);
	if (stringValue == L".else")
	{
		elseBlock = parser.parseCommandSequence(L'.', {L".endif"});

		parser.eatToken();	// eat .endif
	} else if (stringValue == L".elseif")
	{
		elseBlock = parseDirectiveConditional(parser,DIRECTIVE_COND_IF);
	} else if (stringValue == L".elseifdef")
	{
		elseBlock = parseDirectiveConditional(parser,DIRECTIVE_COND_IFDEF);
	} else if (stringValue == L".elseifndef")
	{
		elseBlock = parseDirectiveConditional(parser,DIRECTIVE_COND_IFNDEF);
	} else if (stringValue != L".endif")
	{
		parser.popConditionalResult();
		return nullptr;
	}

	parser.popConditionalResult();

	// for true or false blocks, there's no need to create a conditional command
	if (condResult == ConditionalResult::True)
	{
		return ifBlock;
	}

	if (condResult == ConditionalResult::False)
	{
		if (elseBlock != nullptr)
			return elseBlock;
		else
			return std::make_unique<DummyCommand>();
	}

	std::unique_ptr<CDirectiveConditional> cond;
	if (exp.isLoaded())
		cond = std::make_unique<CDirectiveConditional>(type,exp);
	else if (name.size() != 0)
		cond = std::make_unique<CDirectiveConditional>(type,name);
	else
		cond = std::make_unique<CDirectiveConditional>(type);

	cond->setContent(std::move(ifBlock),std::move(elseBlock));
	return cond;
}

std::unique_ptr<CAssemblerCommand> parseDirectiveTable(Parser& parser, int flags)
{
	const Token& start = parser.peekToken();

	std::vector<Expression> list;
	if (!parser.parseExpressionList(list,1,2))
		return nullptr;

	std::wstring fileName;
	if (!list[0].evaluateString(fileName,true))
	{
		parser.printError(start,L"Invalid file name");
		return nullptr;
	}

	TextFile::Encoding encoding = TextFile::GUESS;
	if (list.size() == 2)
	{
		std::wstring encodingName;
		if (!list[1].evaluateString(encodingName,true))
		{
			parser.printError(start,L"Invalid encoding name");
			return nullptr;
		}

		encoding = getEncodingFromString(encodingName);
	}

	return std::make_unique<TableCommand>(fileName,encoding);
}

std::unique_ptr<CAssemblerCommand> parseDirectiveData(Parser& parser, int flags)
{
	bool terminate = false;
	if (flags & DIRECTIVE_DATA_TERMINATION)
	{
		terminate = true;
		flags &= ~DIRECTIVE_DATA_TERMINATION;
	}

	std::vector<Expression> list;
	if (!parser.parseExpressionList(list,1,-1))
		return nullptr;

	auto data = std::make_unique<CDirectiveData>();
	switch (flags & DIRECTIVE_USERMASK)
	{
	case DIRECTIVE_DATA_8:
		data->setNormal(list,1);
		break;
	case DIRECTIVE_DATA_16:
		data->setNormal(list,2);
		break;
	case DIRECTIVE_DATA_32:
		data->setNormal(list,4);
		break;
	case DIRECTIVE_DATA_64:
		data->setNormal(list,8);
		break;
	case DIRECTIVE_DATA_ASCII:
		data->setAscii(list,terminate);
		break;
	case DIRECTIVE_DATA_SJIS:
		data->setSjis(list,terminate);
		break;
	case DIRECTIVE_DATA_CUSTOM:
		data->setCustom(list,terminate);
		break;
	case DIRECTIVE_DATA_FLOAT:
		data->setFloat(list);
		break;
	case DIRECTIVE_DATA_DOUBLE:
		data->setDouble(list);
		break;
	}

	return data;
}

std::unique_ptr<CAssemblerCommand> parseDirectiveMipsArch(Parser& parser, int flags)
{
	Arch = &Mips;
	Mips.SetLoadDelay(false, 0);

	switch (flags)
	{
	case DIRECTIVE_MIPS_PSX:
		Mips.SetVersion(MARCH_PSX);
		return std::make_unique<ArchitectureCommand>(L".psx", L"");
	case DIRECTIVE_MIPS_PS2:
		Mips.SetVersion(MARCH_PS2);
		return std::make_unique<ArchitectureCommand>(L".ps2", L"");
	case DIRECTIVE_MIPS_PSP:
		Mips.SetVersion(MARCH_PSP);
		return std::make_unique<ArchitectureCommand>(L".psp", L"");
	case DIRECTIVE_MIPS_N64:
		Mips.SetVersion(MARCH_N64);
		return std::make_unique<ArchitectureCommand>(L".n64", L"");
	case DIRECTIVE_MIPS_RSP:
		Mips.SetVersion(MARCH_RSP);
		return std::make_unique<ArchitectureCommand>(L".rsp", L"");
	}

	return nullptr;
}

std::unique_ptr<CAssemblerCommand> parseDirectiveArmArch(Parser& parser, int flags)
{
#ifdef ARMIPS_ARM
	Arch = &Arm;

	switch (flags)
	{
	case DIRECTIVE_ARM_GBA:
		Arm.SetThumbMode(true);
		Arm.setVersion(AARCH_GBA);
		return std::make_unique<ArchitectureCommand>(L".gba\n.thumb", L".thumb");
	case DIRECTIVE_ARM_NDS:
		Arm.SetThumbMode(false);

		Arm.setVersion(AARCH_NDS);
		return std::make_unique<ArchitectureCommand>(L".nds\n.arm", L".arm");
	case DIRECTIVE_ARM_3DS:
		Arm.SetThumbMode(false);
		Arm.setVersion(AARCH_3DS);
		return std::make_unique<ArchitectureCommand>(L".3ds\n.arm", L".arm");
	case DIRECTIVE_ARM_BIG:
		Arm.SetThumbMode(false);
		Arm.setVersion(AARCH_BIG);
		return std::make_unique<ArchitectureCommand>(L".arm.big\n.arm", L".arm");
	case DIRECTIVE_ARM_LITTLE:
		Arm.SetThumbMode(false);
		Arm.setVersion(AARCH_LITTLE);
		return std::make_unique<ArchitectureCommand>(L".arm.little\n.arm", L".arm");
	}
#endif
	return nullptr;
}

std::unique_ptr<CAssemblerCommand> parseDirectiveArea(Parser& parser, int flags)
{
	std::vector<Expression> parameters;
	if (!parser.parseExpressionList(parameters,1,2))
		return nullptr;

	bool shared = (flags & DIRECTIVE_AREA_SHARED) != 0;
	auto area = std::make_unique<CDirectiveArea>(shared, parameters[0]);
	if (parameters.size() == 2)
		area->setFillExpression(parameters[1]);

	std::unique_ptr<CAssemblerCommand> content = parser.parseCommandSequence(L'.', { L".endarea", L".endregion" });
	parser.eatToken();

	area->setContent(std::move(content));
	return area;
}

std::unique_ptr<CAssemblerCommand> parseDirectiveDefineArea(Parser& parser, int flags)
{
	std::vector<Expression> parameters;
	if (!parser.parseExpressionList(parameters,2,3))
		return nullptr;

	bool shared = (flags & DIRECTIVE_AREA_SHARED) != 0;
	auto area = std::make_unique<CDirectiveArea>(shared, parameters[1]);
	area->setPositionExpression(parameters[0]);
	if (parameters.size() == 3)
		area->setFillExpression(parameters[2]);

	return area;
}

std::unique_ptr<CAssemblerCommand> parseDirectiveAutoRegion(Parser& parser, int flags)
{
	std::vector<Expression> parameters;
	if (parser.peekToken().type != TokenType::Separator)
	{
		if (!parser.parseExpressionList(parameters, 0, 2))
			return nullptr;
	}

	auto area = std::make_unique<CDirectiveAutoRegion>();
	if (parameters.size() == 1)
		area->setMinRangeExpression(parameters[0]);
	else if (parameters.size() == 2)
		area->setRangeExpressions(parameters[0], parameters[1]);

	std::unique_ptr<CAssemblerCommand> content = parser.parseCommandSequence(L'.', {L".endautoregion"});
	parser.eatToken();

	area->setContent(std::move(content));
	return area;
}

std::unique_ptr<CAssemblerCommand> parseDirectiveErrorWarning(Parser& parser, int flags)
{
	const Token &tok = parser.nextToken();

	if (tok.type != TokenType::Identifier && tok.type != TokenType::String)
		return nullptr;

	std::wstring stringValue = tok.getStringValue();
	std::transform(stringValue.begin(),stringValue.end(),stringValue.begin(),::towlower);

	if (stringValue == L"on")
	{
		Logger::setErrorOnWarning(true);
		return std::make_unique<DummyCommand>();
	} else if (stringValue == L"off")
	{
		Logger::setErrorOnWarning(false);
		return std::make_unique<DummyCommand>();
	}

	return nullptr;
}

std::unique_ptr<CAssemblerCommand> parseDirectiveRelativeInclude(Parser& parser, int flags)
{
	const Token &tok = parser.nextToken();

	if (tok.type != TokenType::Identifier && tok.type != TokenType::String)
		return nullptr;

	std::wstring stringValue = tok.getStringValue();
	std::transform(stringValue.begin(),stringValue.end(),stringValue.begin(),::towlower);

	if (stringValue == L"on")
	{
		Global.relativeInclude = true;
		return std::make_unique<DummyCommand>();
	} else if (stringValue == L"off")
	{
		Global.relativeInclude = false;
		return std::make_unique<DummyCommand>();
	}

	return nullptr;
}

std::unique_ptr<CAssemblerCommand> parseDirectiveNocash(Parser& parser, int flags)
{
	const Token &tok = parser.nextToken();

	if (tok.type != TokenType::Identifier && tok.type != TokenType::String)
		return nullptr;

	std::wstring stringValue = tok.getStringValue();
	std::transform(stringValue.begin(),stringValue.end(),stringValue.begin(),::towlower);

	if (stringValue == L"on")
	{
		Global.nocash = true;
		return std::make_unique<DummyCommand>();
	} else if (stringValue == L"off")
	{
		Global.nocash = false;
		return std::make_unique<DummyCommand>();
	}

	return nullptr;
}

std::unique_ptr<CAssemblerCommand> parseDirectiveSym(Parser& parser, int flags)
{
	const Token &tok = parser.nextToken();

	if (tok.type != TokenType::Identifier && tok.type != TokenType::String)
		return nullptr;

	std::wstring stringValue = tok.getStringValue();
	std::transform(stringValue.begin(),stringValue.end(),stringValue.begin(),::towlower);

	if (stringValue == L"on")
		return std::make_unique<CDirectiveSym>(true);
	else if (stringValue == L"off")
		return std::make_unique<CDirectiveSym>(false);
	else
		return nullptr;
}

std::unique_ptr<CAssemblerCommand> parseDirectiveDefineLabel(Parser& parser, int flags)
{
	const Token& tok = parser.nextToken();
	if (tok.type != TokenType::Identifier)
		return nullptr;

	if (parser.nextToken().type != TokenType::Comma)
		return nullptr;

	Expression value = parser.parseExpression();
	if (!value.isLoaded())
		return nullptr;

	const std::wstring stringValue = tok.getStringValue();
	if (!Global.symbolTable.isValidSymbolName(stringValue))
	{
		parser.printError(tok,L"Invalid label name \"%s\"",stringValue);
		return nullptr;
	}

	return std::make_unique<CAssemblerLabel>(stringValue,tok.getOriginalText(),value);
}

std::unique_ptr<CAssemblerCommand> parseDirectiveFunction(Parser& parser, int flags)
{
	const Token& tok = parser.nextToken();
	if (tok.type != TokenType::Identifier)
		return nullptr;

	if (parser.nextToken().type != TokenType::Separator)
	{
		parser.printError(tok,L"Directive not terminated");
		return nullptr;
	}

	auto func = std::make_unique<CDirectiveFunction>(tok.getStringValue(),tok.getOriginalText());
	std::unique_ptr<CAssemblerCommand> seq = parser.parseCommandSequence(L'.', {L".endfunc",L".endfunction",L".func",L".function"});

	const std::wstring stringValue = parser.peekToken().getStringValue();
	if (stringValue == L".endfunc" ||
		stringValue == L".endfunction")
	{
		parser.eatToken();
		if(parser.nextToken().type != TokenType::Separator)
		{
			parser.printError(tok,L"Directive not terminated");
			return nullptr;
		}
	}

	func->setContent(std::move(seq));
	return func;
}

std::unique_ptr<CAssemblerCommand> parseDirectiveMessage(Parser& parser, int flags)
{
	Expression exp = parser.parseExpression();

	switch (flags)
	{
	case DIRECTIVE_MSG_WARNING:
		return std::make_unique<CDirectiveMessage>(CDirectiveMessage::Type::Warning,exp);
	case DIRECTIVE_MSG_ERROR:
		return std::make_unique<CDirectiveMessage>(CDirectiveMessage::Type::Error,exp);
	case DIRECTIVE_MSG_NOTICE:
		return std::make_unique<CDirectiveMessage>(CDirectiveMessage::Type::Notice,exp);
	}

	return nullptr;
}

std::unique_ptr<CAssemblerCommand> parseDirectiveInclude(Parser& parser, int flags)
{
	const Token& start = parser.peekToken();

	std::vector<Expression> parameters;
	if (!parser.parseExpressionList(parameters,1,2))
		return nullptr;

	std::wstring fileNameParameter;
	if (!parameters[0].evaluateString(fileNameParameter,true))
		return nullptr;

	auto fileName = getFullPathName(fileNameParameter);

	TextFile::Encoding encoding = TextFile::GUESS;
	if (parameters.size() == 2)
	{
		std::wstring encodingName;
		if (!parameters[1].evaluateString(encodingName,true)
			&& !parameters[1].evaluateIdentifier(encodingName))
			return nullptr;

		encoding = getEncodingFromString(encodingName);
	}

	// don't include the file if it's inside a false block
	if (!parser.isInsideTrueBlock())
		return std::make_unique<DummyCommand>();

	if (!fs::exists(fileName))
	{
		parser.printError(start,L"Included file \"%s\" does not exist",fileName.wstring());
		return nullptr;
	}

	TextFile f;
	if (!f.open(fileName,TextFile::Read,encoding))
	{
		parser.printError(start,L"Could not open included file \"%s\"",fileName.wstring());
		return nullptr;
	}

	return parser.parseFile(f);
}

const DirectiveMap directives = {
	{ L".open",				{ &parseDirectiveOpen,				DIRECTIVE_NOTINMEMORY } },
	{ L".openfile",			{ &parseDirectiveOpen,				DIRECTIVE_NOTINMEMORY } },
	{ L".create",			{ &parseDirectiveCreate,			DIRECTIVE_NOTINMEMORY } },
	{ L".createfile",		{ &parseDirectiveCreate,			DIRECTIVE_NOTINMEMORY } },
	{ L".close",			{ &parseDirectiveClose,				DIRECTIVE_NOTINMEMORY } },
	{ L".closefile",		{ &parseDirectiveClose,				DIRECTIVE_NOTINMEMORY } },
	{ L".incbin",			{ &parseDirectiveIncbin,			0 } },
	{ L".import",			{ &parseDirectiveIncbin,			0 } },
	{ L".org",				{ &parseDirectivePosition,			DIRECTIVE_POS_VIRTUAL } },
	{ L"org",				{ &parseDirectivePosition,			DIRECTIVE_POS_VIRTUAL } },
	{ L".orga",				{ &parseDirectivePosition,			DIRECTIVE_POS_PHYSICAL } },
	{ L"orga",				{ &parseDirectivePosition,			DIRECTIVE_POS_PHYSICAL } },
	{ L".headersize",		{ &parseDirectiveHeaderSize,		0 } },
	{ L".align",			{ &parseDirectiveAlignFill,			DIRECTIVE_ALIGN_VIRTUAL } },
	{ L".aligna",			{ &parseDirectiveAlignFill,			DIRECTIVE_ALIGN_PHYSICAL } },
	{ L".fill",				{ &parseDirectiveAlignFill,			DIRECTIVE_ALIGN_FILL } },
	{ L"defs",				{ &parseDirectiveAlignFill,			DIRECTIVE_ALIGN_FILL } },
	{ L".skip",				{ &parseDirectiveSkip,				0 } },

	{ L".if",				{ &parseDirectiveConditional,		DIRECTIVE_COND_IF } },
	{ L".ifdef",			{ &parseDirectiveConditional,		DIRECTIVE_COND_IFDEF } },
	{ L".ifndef",			{ &parseDirectiveConditional,		DIRECTIVE_COND_IFNDEF } },

	{ L".loadtable",		{ &parseDirectiveTable,				0 } },
	{ L".table",			{ &parseDirectiveTable,				0 } },
	{ L".byte",				{ &parseDirectiveData,				DIRECTIVE_DATA_8 } },
	{ L".halfword",			{ &parseDirectiveData,				DIRECTIVE_DATA_16 } },
	{ L".word",				{ &parseDirectiveData,				DIRECTIVE_DATA_32 } },
	{ L".doubleword",		{ &parseDirectiveData,				DIRECTIVE_DATA_64 } },
	{ L".db",				{ &parseDirectiveData,				DIRECTIVE_DATA_8 } },
	{ L".dh",				{ &parseDirectiveData,				DIRECTIVE_DATA_16|DIRECTIVE_NOCASHOFF } },
	{ L".dw",				{ &parseDirectiveData,				DIRECTIVE_DATA_32|DIRECTIVE_NOCASHOFF } },
	{ L".dd",				{ &parseDirectiveData,				DIRECTIVE_DATA_64|DIRECTIVE_NOCASHOFF } },
	{ L".dw",				{ &parseDirectiveData,				DIRECTIVE_DATA_16|DIRECTIVE_NOCASHON } },
	{ L".dd",				{ &parseDirectiveData,				DIRECTIVE_DATA_32|DIRECTIVE_NOCASHON } },
	{ L".dcb",				{ &parseDirectiveData,				DIRECTIVE_DATA_8 } },
	{ L".dcw",				{ &parseDirectiveData,				DIRECTIVE_DATA_16 } },
	{ L".dcd",				{ &parseDirectiveData,				DIRECTIVE_DATA_32 } },
	{ L".dcq",				{ &parseDirectiveData,				DIRECTIVE_DATA_64 } },
	{ L"db",				{ &parseDirectiveData,				DIRECTIVE_DATA_8 } },
	{ L"dh",				{ &parseDirectiveData,				DIRECTIVE_DATA_16|DIRECTIVE_NOCASHOFF } },
	{ L"dw",				{ &parseDirectiveData,				DIRECTIVE_DATA_32|DIRECTIVE_NOCASHOFF } },
	{ L"dd",				{ &parseDirectiveData,				DIRECTIVE_DATA_64|DIRECTIVE_NOCASHOFF } },
	{ L"dw",				{ &parseDirectiveData,				DIRECTIVE_DATA_16|DIRECTIVE_NOCASHON } },
	{ L"dd",				{ &parseDirectiveData,				DIRECTIVE_DATA_32|DIRECTIVE_NOCASHON } },
	{ L"dcb",				{ &parseDirectiveData,				DIRECTIVE_DATA_8 } },
	{ L"dcw",				{ &parseDirectiveData,				DIRECTIVE_DATA_16 } },
	{ L"dcd",				{ &parseDirectiveData,				DIRECTIVE_DATA_32 } },
	{ L"dcq",				{ &parseDirectiveData,				DIRECTIVE_DATA_64 } },
	{ L".float",			{ &parseDirectiveData,				DIRECTIVE_DATA_FLOAT } },
	{ L".double",			{ &parseDirectiveData,				DIRECTIVE_DATA_DOUBLE } },
	{ L".ascii",			{ &parseDirectiveData,				DIRECTIVE_DATA_ASCII } },
	{ L".asciiz",			{ &parseDirectiveData,				DIRECTIVE_DATA_ASCII|DIRECTIVE_DATA_TERMINATION } },
	{ L".string",			{ &parseDirectiveData,				DIRECTIVE_DATA_CUSTOM|DIRECTIVE_DATA_TERMINATION } },
	{ L".str",				{ &parseDirectiveData,				DIRECTIVE_DATA_CUSTOM|DIRECTIVE_DATA_TERMINATION } },
	{ L".stringn",			{ &parseDirectiveData,				DIRECTIVE_DATA_CUSTOM } },
	{ L".strn",				{ &parseDirectiveData,				DIRECTIVE_DATA_CUSTOM } },
	{ L".sjis",				{ &parseDirectiveData,				DIRECTIVE_DATA_SJIS|DIRECTIVE_DATA_TERMINATION } },
	{ L".sjisn",			{ &parseDirectiveData,				DIRECTIVE_DATA_SJIS } },

	{ L".psx",				{ &parseDirectiveMipsArch,			DIRECTIVE_MIPS_PSX } },
	{ L".ps2",				{ &parseDirectiveMipsArch,			DIRECTIVE_MIPS_PS2 } },
	{ L".psp",				{ &parseDirectiveMipsArch,			DIRECTIVE_MIPS_PSP } },
	{ L".n64",				{ &parseDirectiveMipsArch,			DIRECTIVE_MIPS_N64 } },
	{ L".rsp",				{ &parseDirectiveMipsArch,			DIRECTIVE_MIPS_RSP } },

	{ L".gba",				{ &parseDirectiveArmArch,			DIRECTIVE_ARM_GBA } },
	{ L".nds",				{ &parseDirectiveArmArch,			DIRECTIVE_ARM_NDS } },
	{ L".3ds",				{ &parseDirectiveArmArch,			DIRECTIVE_ARM_3DS } },
	{ L".arm.big",			{ &parseDirectiveArmArch,			DIRECTIVE_ARM_BIG } },
	{ L".arm.little",		{ &parseDirectiveArmArch,			DIRECTIVE_ARM_LITTLE } },

	{ L".area",				{ &parseDirectiveArea,				0 } },
	{ L".autoregion",		{ &parseDirectiveAutoRegion,		0 } },
	{ L".region",			{ &parseDirectiveArea,				DIRECTIVE_AREA_SHARED } },
	{ L".defineregion",		{ &parseDirectiveDefineArea,		DIRECTIVE_AREA_SHARED } },

	{ L".importobj",		{ &parseDirectiveObjImport,			0 } },
	{ L".importlib",		{ &parseDirectiveObjImport,			0 } },

	{ L".erroronwarning",	{ &parseDirectiveErrorWarning,		0 } },
	{ L".relativeinclude",	{ &parseDirectiveRelativeInclude,	0 } },
	{ L".nocash",			{ &parseDirectiveNocash,			0 } },
	{ L".sym",				{ &parseDirectiveSym,				0 } },

	{ L".definelabel",		{ &parseDirectiveDefineLabel,		0 } },
	{ L".function",			{ &parseDirectiveFunction,			DIRECTIVE_MANUALSEPARATOR } },
	{ L".func",				{ &parseDirectiveFunction,			DIRECTIVE_MANUALSEPARATOR } },

	{ L".warning",			{ &parseDirectiveMessage,			DIRECTIVE_MSG_WARNING } },
	{ L".error",			{ &parseDirectiveMessage,			DIRECTIVE_MSG_ERROR } },
	{ L".notice",			{ &parseDirectiveMessage,			DIRECTIVE_MSG_NOTICE } },

	{ L".include",			{ &parseDirectiveInclude,			0 } },
};

// file: Parser/ExpressionParser.cpp

static ExpressionInternal* expression(Tokenizer& tokenizer);

static bool allowFunctionCall = true;

void allowFunctionCallExpression(bool allow)
{
	allowFunctionCall = allow;
}

static ExpressionInternal* primaryExpression(Tokenizer& tokenizer)
{
	const Token &tok = tokenizer.peekToken();

	switch (tok.type)
	{
	case TokenType::Float:
		tokenizer.eatToken();
		return new ExpressionInternal(tok.floatValue);
	case TokenType::Identifier:
		{
			const std::wstring stringValue = tok.getStringValue();
			tokenizer.eatToken();
			if (stringValue == L".")
				return new ExpressionInternal(OperatorType::MemoryPos);
			else
				return new ExpressionInternal(stringValue,OperatorType::Identifier);
		}
	case TokenType::String:
		tokenizer.eatToken();
		return new ExpressionInternal(tok.getStringValue(),OperatorType::String);
	case TokenType::Integer:
		tokenizer.eatToken();
		return new ExpressionInternal(tok.intValue);
	case TokenType::LParen:
		{
			tokenizer.eatToken();
			ExpressionInternal* exp = expression(tokenizer);

			if (tokenizer.nextToken().type != TokenType::RParen)
			{
				delete exp;
				return nullptr;
			}

			return exp;
		}
	case TokenType::Invalid:
	default:
		break;
	}

	return nullptr;
}

static ExpressionInternal* postfixExpression(Tokenizer& tokenizer)
{
	if (allowFunctionCall &&
		tokenizer.peekToken(0).type == TokenType::Identifier &&
		tokenizer.peekToken(1).type == TokenType::LParen)
	{
		const std::wstring functionName = tokenizer.nextToken().getStringValue();
		tokenizer.eatToken();

		std::vector<ExpressionInternal*> parameters;
		while (tokenizer.peekToken().type != TokenType::RParen)
		{
			if (parameters.size() != 0 && tokenizer.nextToken().type != TokenType::Comma)
			{
				for (ExpressionInternal* exp: parameters)
					delete exp;
				return nullptr;
			}

			ExpressionInternal* exp = expression(tokenizer);
			if (exp == nullptr)
			{
				for (ExpressionInternal* exp: parameters)
					delete exp;
				return nullptr;
			}

			parameters.push_back(exp);
		}

		tokenizer.eatToken();

		return new ExpressionInternal(functionName,parameters);
	}

	return primaryExpression(tokenizer);
}

static ExpressionInternal* unaryExpression(Tokenizer& tokenizer)
{
	ExpressionInternal* exp = postfixExpression(tokenizer);
	if (exp != nullptr)
		return exp;

	const TokenType opType = tokenizer.nextToken().type;
	exp = postfixExpression(tokenizer);
	if (exp == nullptr)
		return nullptr;

	switch (opType)
	{
	case TokenType::Plus:
		return exp;
	case TokenType::Minus:
		return new ExpressionInternal(OperatorType::Neg,exp);
	case TokenType::Tilde:
		return new ExpressionInternal(OperatorType::BitNot,exp);
	case TokenType::Exclamation:
		return new ExpressionInternal(OperatorType::LogNot,exp);
	case TokenType::Degree:
		return new ExpressionInternal(OperatorType::ToString,exp);
	default:
		delete exp;
		return nullptr;
	}
}

static ExpressionInternal* multiplicativeExpression(Tokenizer& tokenizer)
{
	ExpressionInternal* exp = unaryExpression(tokenizer);
	if (exp ==  nullptr)
		return nullptr;

	while (true)
	{
		OperatorType op = OperatorType::Invalid;
		switch (tokenizer.peekToken().type)
		{
		case TokenType::Mult:
			op = OperatorType::Mult;
			break;
		case TokenType::Div:
			op = OperatorType::Div;
			break;
		case TokenType::Mod:
			op = OperatorType::Mod;
			break;
		default:
			break;
		}

		if (op == OperatorType::Invalid)
			break;

		tokenizer.eatToken();

		ExpressionInternal* exp2 = unaryExpression(tokenizer);
		if (exp2 == nullptr)
		{
			delete exp;
			return nullptr;
		}

		exp = new ExpressionInternal(op,exp,exp2);
	}

	return exp;
}

static ExpressionInternal* additiveExpression(Tokenizer& tokenizer)
{
	ExpressionInternal* exp = multiplicativeExpression(tokenizer);
	if (exp == nullptr)
		return nullptr;

	while (true)
	{
		OperatorType op = OperatorType::Invalid;
		switch (tokenizer.peekToken().type)
		{
		case TokenType::Plus:
			op = OperatorType::Add;
			break;
		case TokenType::Minus:
			op = OperatorType::Sub;
			break;
		default:
			break;
		}

		if (op == OperatorType::Invalid)
			break;

		tokenizer.eatToken();

		ExpressionInternal* exp2 = multiplicativeExpression(tokenizer);
		if (exp2 == nullptr)
		{
			delete exp;
			return nullptr;
		}

		exp = new ExpressionInternal(op,exp,exp2);
	}

	return exp;
}

static ExpressionInternal* shiftExpression(Tokenizer& tokenizer)
{
	ExpressionInternal* exp = additiveExpression(tokenizer);
	if (exp == nullptr)
		return nullptr;

	while (true)
	{
		OperatorType op = OperatorType::Invalid;
		switch (tokenizer.peekToken().type)
		{
		case TokenType::LeftShift:
			op = OperatorType::LeftShift;
			break;
		case TokenType::RightShift:
			op = OperatorType::RightShift;
			break;
		default:
			break;
		}

		if (op == OperatorType::Invalid)
			break;

		tokenizer.eatToken();

		ExpressionInternal* exp2 = additiveExpression(tokenizer);
		if (exp2 == nullptr)
		{
			delete exp;
			return nullptr;
		}

		exp = new ExpressionInternal(op,exp,exp2);
	}

	return exp;
}

static ExpressionInternal* relationalExpression(Tokenizer& tokenizer)
{
	ExpressionInternal* exp = shiftExpression(tokenizer);
	if (exp == nullptr)
		return nullptr;

	while (true)
	{
		OperatorType op = OperatorType::Invalid;
		switch (tokenizer.peekToken().type)
		{
		case TokenType::Less:
			op = OperatorType::Less;
			break;
		case TokenType::LessEqual:
			op = OperatorType::LessEqual;
			break;
		case TokenType::Greater:
			op = OperatorType::Greater;
			break;
		case TokenType::GreaterEqual:
			op = OperatorType::GreaterEqual;
			break;
		default:
			break;
		}

		if (op == OperatorType::Invalid)
			break;

		tokenizer.eatToken();

		ExpressionInternal* exp2 = shiftExpression(tokenizer);
		if (exp2 == nullptr)
		{
			delete exp;
			return nullptr;
		}

		exp = new ExpressionInternal(op,exp,exp2);
	}

	return exp;
}

static ExpressionInternal* equalityExpression(Tokenizer& tokenizer)
{
	ExpressionInternal* exp = relationalExpression(tokenizer);
	if (exp == nullptr)
		return nullptr;

	while (true)
	{
		OperatorType op = OperatorType::Invalid;
		switch (tokenizer.peekToken().type)
		{
		case TokenType::Equal:
			op = OperatorType::Equal;
			break;
		case TokenType::NotEqual:
			op = OperatorType::NotEqual;
			break;
		default:
			break;
		}

		if (op == OperatorType::Invalid)
			break;

		tokenizer.eatToken();

		ExpressionInternal* exp2 = relationalExpression(tokenizer);
		if (exp2 == nullptr)
		{
			delete exp;
			return nullptr;
		}

		exp = new ExpressionInternal(op,exp,exp2);
	}

	return exp;
}

static ExpressionInternal* andExpression(Tokenizer& tokenizer)
{
	ExpressionInternal* exp = equalityExpression(tokenizer);
	if (exp == nullptr)
		return nullptr;

	while (tokenizer.peekToken().type == TokenType::BitAnd)
	{
		tokenizer.eatToken();

		ExpressionInternal* exp2 = equalityExpression(tokenizer);
		if (exp2 == nullptr)
		{
			delete exp;
			return nullptr;
		}

		exp = new ExpressionInternal(OperatorType::BitAnd,exp,exp2);
	}

	return exp;
}

static ExpressionInternal* exclusiveOrExpression(Tokenizer& tokenizer)
{
	ExpressionInternal* exp = andExpression(tokenizer);
	if (exp == nullptr)
		return nullptr;

	while (tokenizer.peekToken().type == TokenType::Caret)
	{
		tokenizer.eatToken();

		ExpressionInternal* exp2 = andExpression(tokenizer);
		if (exp2 == nullptr)
		{
			delete exp;
			return nullptr;
		}

		exp = new ExpressionInternal(OperatorType::Xor,exp,exp2);
	}

	return exp;
}

static ExpressionInternal* inclusiveOrExpression(Tokenizer& tokenizer)
{
	ExpressionInternal* exp = exclusiveOrExpression(tokenizer);
	if (exp == nullptr)
		return nullptr;

	while (tokenizer.peekToken().type == TokenType::BitOr)
	{
		tokenizer.eatToken();

		ExpressionInternal* exp2 = exclusiveOrExpression(tokenizer);
		if (exp2 == nullptr)
		{
			delete exp;
			return nullptr;
		}

		exp = new ExpressionInternal(OperatorType::BitOr,exp,exp2);
	}

	return exp;
}

static ExpressionInternal* logicalAndExpression(Tokenizer& tokenizer)
{
	ExpressionInternal* exp = inclusiveOrExpression(tokenizer);
	if (exp == nullptr)
		return nullptr;

	while (tokenizer.peekToken().type == TokenType::LogAnd)
	{
		tokenizer.eatToken();

		ExpressionInternal* exp2 = inclusiveOrExpression(tokenizer);
		if (exp2 == nullptr)
		{
			delete exp;
			return nullptr;
		}

		exp = new ExpressionInternal(OperatorType::LogAnd,exp,exp2);
	}

	return exp;
}

static ExpressionInternal* logicalOrExpression(Tokenizer& tokenizer)
{
	ExpressionInternal* exp = logicalAndExpression(tokenizer);
	if (exp == nullptr)
		return nullptr;

	while (tokenizer.peekToken().type == TokenType::LogOr)
	{
		tokenizer.eatToken();

		ExpressionInternal* exp2 = logicalAndExpression(tokenizer);
		if (exp2 == nullptr)
		{
			delete exp;
			return nullptr;
		}

		exp = new ExpressionInternal(OperatorType::LogOr,exp,exp2);
	}

	return exp;
}

static ExpressionInternal* conditionalExpression(Tokenizer& tokenizer)
{
	ExpressionInternal* exp = logicalOrExpression(tokenizer);
	if (exp == nullptr)
		return nullptr;

	// check a ? b : c
	if (tokenizer.peekToken().type != TokenType::Question)
		return exp;

	tokenizer.eatToken();
	ExpressionInternal* second = expression(tokenizer);

	if (second != nullptr && tokenizer.nextToken().type == TokenType::Colon)
	{
		ExpressionInternal* third = expression(tokenizer);
		if (third != nullptr)
			return new ExpressionInternal(OperatorType::TertiaryIf,exp,second,third);

		delete third;
	}

	delete second;
	delete exp;
	return nullptr;
}

static ExpressionInternal* expression(Tokenizer& tokenizer)
{
	return conditionalExpression(tokenizer);
}

Expression parseExpression(Tokenizer& tokenizer, bool inUnknownOrFalseBlock)
{
	TokenizerPosition pos = tokenizer.getPosition();

	// parse expression, revert tokenizer to previous position
	// if it failed
	ExpressionInternal* exp = expression(tokenizer);
	if (exp == nullptr)
		tokenizer.setPosition(pos);

	Expression result;
	result.setExpression(exp, inUnknownOrFalseBlock);
	return result;
}

// file: Parser/Parser.cpp


inline bool isPartOfList(const std::wstring& value, const std::initializer_list<const wchar_t*>& terminators)
{
	for (const wchar_t* term: terminators)
	{
		if (value == term)
			return true;
	}

	return false;
}

Parser::Parser()
{
	initializingMacro = false;
	overrideFileInfo = false;
	conditionStack.push_back({true,false});
	clearError();
}

void Parser::pushConditionalResult(ConditionalResult cond)
{
	ConditionInfo info = conditionStack.back();
	info.inTrueBlock = info.inTrueBlock && cond != ConditionalResult::False;
	info.inUnknownBlock = info.inUnknownBlock || cond == ConditionalResult::Unknown;
	conditionStack.push_back(info);
}

void Parser::printError(const Token &token, const std::wstring &text)
{
	errorLine = token.line;
	Global.FileInfo.LineNumber = (int) token.line;
	Logger::printError(Logger::Error, text);
	error = true;
}

Expression Parser::parseExpression()
{
	return ::parseExpression(*getTokenizer(), !isInsideTrueBlock() || isInsideUnknownBlock());
}

bool Parser::parseExpressionList(std::vector<Expression>& list, int min, int max)
{
	bool valid = true;
	list.clear();
	list.reserve(max >= 0 ? max : 32);

	const Token& start = peekToken();

	Expression exp = parseExpression();
	list.push_back(exp);

	if (!exp.isLoaded())
	{
		printError(start,L"Parameter failure");
		getTokenizer()->skipLookahead();
		valid = false;
	}

	while (peekToken().type == TokenType::Comma)
	{
		eatToken();

		exp = parseExpression();
		list.push_back(exp);

		if (!exp.isLoaded())
		{
			printError(start,L"Parameter failure");
			getTokenizer()->skipLookahead();
			valid = false;
		}
	}

	if (list.size() < (size_t) min)
	{
		printError(start,L"Not enough parameters (min %d)",min);
		return false;
	}

	if (max != -1 && (size_t) max < list.size())
	{
		printError(start,L"Too many parameters (max %d)",max);
		return false;
	}

	return valid;
}

bool Parser::parseIdentifier(std::wstring& dest)
{
	const Token& tok = nextToken();
	if (tok.type != TokenType::Identifier)
		return false;

	dest = tok.getStringValue();
	return true;
}

std::unique_ptr<CAssemblerCommand> Parser::parseCommandSequence(wchar_t indicator, const std::initializer_list<const wchar_t*> terminators)
{
	auto sequence = std::make_unique<CommandSequence>();

	bool foundTermination = false;
	while (!atEnd())
	{
		const Token &next = peekToken();

		if(next.type == TokenType::Separator)
		{
			eatToken();
			continue;
		}

		if (next.stringValueStartsWith(indicator) && isPartOfList(next.getStringValue(), terminators))
		{
			foundTermination = true;
			break;
		}

		bool foundSomething = false;
		while (checkEquLabel() || checkMacroDefinition() || checkExpFuncDefinition())
		{
			// do nothing, just parse all the equs and macros there are
			if (hasError())
				sequence->addCommand(handleError());

			foundSomething = true;
		}

		if (foundSomething)
			continue;

		std::unique_ptr<CAssemblerCommand> cmd = parseCommand();

		// omit commands inside blocks that are trivially false
		if (!isInsideTrueBlock())
		{
			continue;
		}

		sequence->addCommand(std::move(cmd));
	}

	if (!foundTermination && terminators.size())
	{
		std::wstring expected;
		for (const wchar_t* terminator : terminators)
		{
			if (!expected.empty())
				expected += L", ";
			expected += terminator;
		}

		Logger::printError(Logger::Error, L"Unterminated command sequence, expected any of %s.", expected);
	}

	return sequence;
}

std::unique_ptr<CAssemblerCommand> Parser::parseFile(TextFile& file, bool virtualFile)
{
	FileTokenizer tokenizer;
	if (!tokenizer.init(&file))
		return nullptr;

	std::unique_ptr<CAssemblerCommand> result = parse(&tokenizer,virtualFile,file.getFileName());

	if (!file.isFromMemory())
		Global.FileInfo.TotalLineCount += file.getNumLines();

	return result;
}

std::unique_ptr<CAssemblerCommand> Parser::parseString(const std::wstring& text)
{
	TextFile file;
	file.openMemory(text);
	return parseFile(file,true);
}

std::unique_ptr<CAssemblerCommand> Parser::parseTemplate(const std::wstring& text, std::initializer_list<AssemblyTemplateArgument> variables)
{
	std::wstring fullText = text;

	overrideFileInfo = true;
	overrideFileNum = Global.FileInfo.FileNum;
	overrideLineNum = Global.FileInfo.LineNumber;

	for (auto& arg: variables)
	{
		size_t count = replaceAll(fullText,arg.variableName,arg.value);
		(void)count;
#ifdef _DEBUG
		if (count != 0 && arg.value.empty())
			Logger::printError(Logger::Warning,L"Empty replacement for %s",arg.variableName);
#endif
	}

	std::unique_ptr<CAssemblerCommand> result = parseString(fullText);
	overrideFileInfo = false;

	return result;
}

std::unique_ptr<CAssemblerCommand> Parser::parseDirective(const DirectiveMap &directiveSet)
{
	const Token &tok = peekToken();
	if (tok.type != TokenType::Identifier)
		return nullptr;

	const std::wstring stringValue = tok.getStringValue();

	auto matchRange = directiveSet.equal_range(stringValue);
	for (auto it = matchRange.first; it != matchRange.second; ++it)
	{
		const DirectiveEntry &directive = it->second;

		if (directive.flags & DIRECTIVE_DISABLED)
			continue;
		if ((directive.flags & DIRECTIVE_NOCASHOFF) && Global.nocash)
			continue;
		if ((directive.flags & DIRECTIVE_NOCASHON) && !Global.nocash)
			continue;
		if ((directive.flags & DIRECTIVE_NOTINMEMORY) && Global.memoryMode)
			continue;

		if (directive.flags & DIRECTIVE_MIPSRESETDELAY)
			Arch->NextSection();

		eatToken();
		std::unique_ptr<CAssemblerCommand> result = directive.function(*this,directive.flags);
		if (result == nullptr)
		{
			if (!hasError())
				printError(tok,L"Directive parameter failure");
			return nullptr;
		} else if (!(directive.flags & DIRECTIVE_MANUALSEPARATOR) && nextToken().type != TokenType::Separator)
		{
			printError(tok,L"Directive not terminated");
			return nullptr;
		}

		return result;
	}

	return nullptr;
}

bool Parser::matchToken(TokenType type, bool optional)
{
	if (optional)
	{
		const Token& token = peekToken();
		if (token.type == type)
			eatToken();
		return true;
	}

	return nextToken().type == type;
}

std::unique_ptr<CAssemblerCommand> Parser::parse(Tokenizer* tokenizer, bool virtualFile, const fs::path& name)
{
	if (entries.size() >= 150)
	{
		Logger::queueError(Logger::Error, L"Max include/recursion depth reached");
		return nullptr;
	}

	FileEntry entry;
	entry.tokenizer = tokenizer;
	entry.virtualFile = virtualFile;

	if (!virtualFile && !name.empty())
	{
		entry.fileNum = (int) Global.fileList.size();
		Global.fileList.add(name);
	} else {
		entry.fileNum = -1;
	}

	entries.push_back(entry);

	std::unique_ptr<CAssemblerCommand> sequence = parseCommandSequence();
	entries.pop_back();

	return sequence;
}

void Parser::addEquation(const Token& startToken, const std::wstring& name, const std::wstring& value)
{
	// parse value string
	TextFile f;
	f.openMemory(value);

	FileTokenizer tok;
	tok.init(&f);

	TokenizerPosition start = tok.getPosition();
	while (!tok.atEnd() && tok.peekToken().type != TokenType::Separator)
	{
		const Token& token = tok.nextToken();
		if (token.type == TokenType::Identifier && token.getStringValue() == name)
		{
			printError(startToken,L"Recursive equ definition for \"%s\" not allowed",name);
			return;
		}

		if (token.type == TokenType::Equ)
		{
			printError(startToken,L"equ value must not contain another equ instance");
			return;
		}
	}

	// extract tokens
	TokenizerPosition end = tok.getPosition();
	std::vector<Token> tokens = tok.getTokens(start, end);
	size_t index = Tokenizer::addEquValue(tokens);

	for (FileEntry& entry : entries)
		entry.tokenizer->resetLookaheadCheckMarks();

	// register equation
	Global.symbolTable.addEquation(name, Global.FileInfo.FileNum, Global.Section, index);
}

bool Parser::checkEquLabel()
{
	updateFileInfo();

	const Token& start = peekToken();
	if (start.type == TokenType::Identifier)
	{
		int pos = 1;
		if (peekToken(pos).type == TokenType::Colon)
			pos++;

		if (peekToken(pos).type == TokenType::Equ &&
			peekToken(pos+1).type == TokenType::EquValue)
		{
			std::wstring name = peekToken(0).getStringValue();
			std::wstring value = peekToken(pos+1).getStringValue();
			eatTokens(pos+2);

			// skip the equ if it's inside a false conditional block
			if (!isInsideTrueBlock())
				return true;

			// equs can't be inside blocks whose condition can only be
			// evaluated during validation
			if (isInsideUnknownBlock())
			{
				printError(start,L"equ not allowed inside of block with non-trivial condition");
				return true;
			}

			// equs are not allowed in macros
			if (initializingMacro)
			{
				printError(start,L"equ not allowed in macro");
				return true;
			}

			if (!Global.symbolTable.isValidSymbolName(name))
			{
				printError(start,L"Invalid equation name \"%s\"",name);
				return true;
			}

			if (Global.symbolTable.symbolExists(name,Global.FileInfo.FileNum,Global.Section))
			{
				printError(start,L"Equation name \"%s\" already defined",name);
				return true;
			}

			addEquation(start,name,value);
			return true;
		}
	}

	return false;
}

bool Parser::parseFunctionDeclaration(std::wstring& name, std::vector<std::wstring>& parameters)
{
	const Token& first = peekToken();
	if (first.type != TokenType::Identifier)
		return false;

	name = nextToken().getStringValue();

	if (nextToken().type != TokenType::LParen)
		return false;

	parameters.clear();
	while (!atEnd() && peekToken().type != TokenType::RParen)
	{
		if (!parameters.empty() && peekToken().type == TokenType::Comma)
			eatToken();

		const Token& token = nextToken();
		if (token.type != TokenType::Identifier)
			return false;

		parameters.emplace_back(token.getStringValue());
	}

	return !atEnd() && nextToken().type == TokenType::RParen;
}

bool Parser::checkExpFuncDefinition()
{
	const Token& first = peekToken();
	if (first.type != TokenType::Identifier)
		return false;

	if (!first.stringValueStartsWith(L'.') || first.getStringValue() != L".expfunc")
		return false;

	eatToken();

	UserExpressionFunction func;

	// load declarationn
	if (!parseFunctionDeclaration(func.name, func.parameters))
	{
		printError(first, L"Invalid expression function declaration");
		return false;
	}

	if (nextToken().type != TokenType::Comma)
	{
		printError(first, L"Invalid expression function declaration");
		return false;
	}

	// load definition
	TokenizerPosition start = getTokenizer()->getPosition();

	Expression exp = parseExpression();
	if (!exp.isLoaded())
	{
		printError(first, L"Invalid expression function declaration");
		return false;
	}

	TokenizerPosition end = getTokenizer()->getPosition();
	func.content = getTokenizer()->getTokens(start,end);

	// checks

	// Expression functions have to be defined at parse time, so they can't be defined in blocks
	// with non-trivial conditions
	if (isInsideUnknownBlock())
	{
		printError(first, L"Expression function definition not allowed inside of block with non-trivial condition");
		return false;
	}

	// if we are in a known false block, don't define the function
	if (!isInsideTrueBlock())
		return true;

	if(nextToken().type != TokenType::Separator)
	{
		printError(first, L".expfunc directive not terminated");
		return false;
	}

	// duplicate check
	if (UserFunctions::instance().findFunction(func.name))
	{
		printError(first, L"Expression function \"%s\" already declared", func.name);
		return false;
	}

	// register function
	UserFunctions::instance().addFunction(func);
	return true;
}

bool Parser::checkMacroDefinition()
{
	const Token& first = peekToken();
	if (first.type != TokenType::Identifier)
		return false;

	if (!first.stringValueStartsWith(L'.') || first.getStringValue() != L".macro")
		return false;

	eatToken();

	// nested macro definitions are not allowed
	if (initializingMacro)
	{
		printError(first,L"Nested macro definitions not allowed");
		while (!atEnd())
		{
			const Token& token = nextToken();
			if (token.type == TokenType::Identifier && token.getStringValue() == L".endmacro")
				break;
		}

		return true;
	}

	std::vector<Expression> parameters;
	if (!parseExpressionList(parameters,1,-1))
		return false;

	ParserMacro macro;
	macro.counter = 0;

	// load name
	if (!parameters[0].evaluateIdentifier(macro.name))
		return false;

	// load parameters
	for (size_t i = 1; i < parameters.size(); i++)
	{
		std::wstring name;
		if (!parameters[i].evaluateIdentifier(name))
			return false;

		macro.parameters.push_back(name);
	}

	if(nextToken().type != TokenType::Separator)
	{
		printError(first,L"Macro directive not terminated");
		return false;
	}

	// load macro content

	TokenizerPosition start = getTokenizer()->getPosition();
	bool valid = false;
	while (!atEnd())
	{
		const Token& tok = nextToken();
		if (tok.type == TokenType::Identifier && tok.getStringValue() == L".endmacro")
		{
			valid = true;
			break;
		}
	}

	// Macros have to be defined at parse time, so they can't be defined in blocks
	// with non-trivial conditions
	if (isInsideUnknownBlock())
	{
		printError(first, L"Macro definition not allowed inside of block with non-trivial condition");
		return false;
	}

	// if we are in a known false block, don't define the macro
	if (!isInsideTrueBlock())
		return true;

	// duplicate check
	if (macros.find(macro.name) != macros.end())
	{
		printError(first, L"Macro \"%s\" already defined", macro.name);
		return false;
	}

	// no .endmacro, not valid
	if (!valid)
	{
		printError(first, L"Macro \"%s\" not terminated", macro.name);
		return true;
	}

	// get content
	TokenizerPosition end = getTokenizer()->getPosition().previous();
	macro.content = getTokenizer()->getTokens(start,end);

	if(nextToken().type != TokenType::Separator)
	{
		printError(first,L"Endmacro directive not terminated");
		return false;
	}

	macros[macro.name] = macro;
	return true;
}

std::optional<std::vector<Token>> Parser::extractMacroParameter(const Token &macroStart)
{
	TokenizerPosition startPos = getTokenizer()->getPosition();

	// Find the end of the parameter. The parameter may contain expressions with function calls,
	// so keep track of the current parenthesis depth level
	int parenCount = 0;
	int braceCount = 0;
	int bracketCount = 0;

	while (peekToken().type != TokenType::Separator)
	{
		// if the next token is a comma, only exit the loop if parentheses are balanced
		auto type = peekToken().type;
		if (type == TokenType::Comma && parenCount == 0 && braceCount == 0 && bracketCount == 0)
			break;

		// keep track of parenthesis depth
		switch (type)
		{
		case TokenType::LParen:
			++parenCount;
			break;
		case TokenType::RParen:
			--parenCount;
			break;
		case TokenType::LBrace:
			++braceCount;
			break;
		case TokenType::RBrace:
			--braceCount;
			break;
		case TokenType::LBrack:
			++bracketCount;
			break;
		case TokenType::RBrack:
			--bracketCount;
			break;
		default:
			break;
		}

		eatToken();
	}

	if (parenCount != 0)
	{
		printError(macroStart, L"Unbalanced parentheses in macro parameter");
		return std::nullopt;
	}

	TokenizerPosition endPos = getTokenizer()->getPosition();
	std::vector<Token> tokens = getTokenizer()->getTokens(startPos,endPos);
	if (tokens.size() == 0)
	{
		printError(macroStart, L"Empty macro argument");
		return std::nullopt;
	}

	return tokens;
}

std::unique_ptr<CAssemblerCommand> Parser::parseMacroCall()
{
	const Token& start = peekToken();
	if (start.type != TokenType::Identifier)
		return nullptr;

	auto it = macros.find(start.getStringValue());
	if (it == macros.end())
		return nullptr;

	ParserMacro& macro = it->second;
	eatToken();

	// create a token stream for the macro content,
	// registering replacements for parameter values
	TokenStreamTokenizer macroTokenizer;

	std::set<std::wstring> identifierParameters;
	for (size_t i = 0; i < macro.parameters.size(); i++)
	{
		if (peekToken().type == TokenType::Separator)
		{
			printError(start,L"Too few macro arguments (%d vs %d)",i,macro.parameters.size());
			return nullptr;
		}

		if (i != 0)
		{
			if (nextToken().type != TokenType::Comma)
			{
				printError(start,L"Macro arguments not comma-separated");
				return nullptr;
			}
		}

		auto tokens = extractMacroParameter(start);
		if (!tokens)
			return nullptr;

		// remember any single identifier parameters for the label replacement
		if (tokens->size() == 1 && tokens->front().type == TokenType::Identifier)
			identifierParameters.insert(tokens->front().getStringValue());

		// give them as a replacement to new tokenizer
		macroTokenizer.registerReplacement(macro.parameters[i], *tokens);
	}

	if (peekToken().type == TokenType::Comma)
	{
		size_t count = macro.parameters.size();
		while (peekToken().type == TokenType::Comma)
		{
			// skip comma
			eatToken();

			// skip parameter value
			extractMacroParameter(start);
			++count;
		}

		printError(start,L"Too many macro arguments (%d vs %d)",count,macro.parameters.size());
		return nullptr;
	}

	if(nextToken().type != TokenType::Separator)
	{
		printError(start,L"Macro call not terminated");
		return nullptr;
	}

	// skip macro instantiation in known false blocks
	if (!isInsideUnknownBlock() && !isInsideTrueBlock())
		return std::make_unique<DummyCommand>();

	// a macro is fully parsed once when it's loaded
	// to gather all labels. it's not necessary to
	// instantiate other macros at that time
	if (initializingMacro)
		return std::make_unique<DummyCommand>();

	// the first time a macro is instantiated, it needs to be analyzed
	// for labels
	if (macro.counter == 0)
	{
		initializingMacro = true;

		// parse the short lived next command
		macroTokenizer.init(macro.content);
		Logger::suppressErrors();
		std::unique_ptr<CAssemblerCommand> command =  parse(&macroTokenizer,true);
		Logger::unsuppressErrors();

		macro.labels = macroLabels;
		macroLabels.clear();

		initializingMacro = false;
	}

	// register labels and replacements
	for (const std::wstring& label: macro.labels)
	{
		// check if the label is using the name of a parameter
		// in that case, don't register a unique replacement
		if (identifierParameters.find(label) != identifierParameters.end())
			continue;

		// otherwise make sure the name is unique
		std::wstring fullName;
		if (Global.symbolTable.isLocalSymbol(label))
			fullName = tfm::format(L"@@%s_%s_%08X",macro.name,label.substr(2),macro.counter);
		else if (Global.symbolTable.isStaticSymbol(label))
			fullName = tfm::format(L"@%s_%s_%08X",macro.name,label.substr(1),macro.counter);
		else
			fullName = tfm::format(L"%s_%s_%08X",macro.name,label,macro.counter);

		macroTokenizer.registerReplacement(label,fullName);
	}

	macroTokenizer.init(macro.content);
	macro.counter++;

	return parse(&macroTokenizer,true);

}

std::unique_ptr<CAssemblerCommand> Parser::parseLabel()
{
	updateFileInfo();

	const Token& start = peekToken(0);

	if (peekToken(0).type == TokenType::Identifier &&
		peekToken(1).type == TokenType::Colon)
	{
		const std::wstring name = start.getStringValue();
		eatTokens(2);

		if (initializingMacro)
			macroLabels.insert(name);

		if (!Global.symbolTable.isValidSymbolName(name))
		{
			printError(start,L"Invalid label name \"%s\"",name);
			return nullptr;
		}

		return std::make_unique<CAssemblerLabel>(name,start.getOriginalText());
	}

	return nullptr;
}

std::unique_ptr<CAssemblerCommand> Parser::handleError()
{
	// skip the rest of the statement
	while (!atEnd() && nextToken().type != TokenType::Separator);

	clearError();
	return std::make_unique<InvalidCommand>();
}


void Parser::updateFileInfo()
{
	if (overrideFileInfo)
	{
		Global.FileInfo.FileNum = overrideFileNum;
		Global.FileInfo.LineNumber = overrideLineNum;
		return;
	}

	for (size_t i = entries.size(); i > 0; i--)
	{
		size_t index = i-1;

		if (!entries[index].virtualFile && entries[index].fileNum != -1)
		{
			Global.FileInfo.FileNum = entries[index].fileNum;

			// if it's not the topmost file, then the command to instantiate the
			// following files was already parsed -> take the previous command's line
			if (index != entries.size() - 1)
				Global.FileInfo.LineNumber = entries[index].previousCommandLine;
			else
			{
				Global.FileInfo.LineNumber = (int)entries[index].tokenizer->peekToken().line;
				entries[index].previousCommandLine = Global.FileInfo.LineNumber;
			}
			return;
		}
	}
}

std::unique_ptr<CAssemblerCommand> Parser::parseCommand()
{
	std::unique_ptr<CAssemblerCommand> command;

	updateFileInfo();

	if (atEnd())
		return std::make_unique<DummyCommand>();

	if ((command = parseLabel()) != nullptr)
		return command;
	if (hasError())
		return handleError();

	if ((command = parseMacroCall()) != nullptr)
		return command;
	if (hasError())
		return handleError();

	if ((command = Arch->parseDirective(*this)) != nullptr)
		return command;
	if (hasError())
		return handleError();

	if ((command = parseDirective(directives)) != nullptr)
		return command;
	if (hasError())
		return handleError();

	if ((command = Arch->parseOpcode(*this)) != nullptr)
		return command;
	if (hasError())
		return handleError();

	const Token& token = peekToken();
	printError(token,L"Parse error '%s'",token.getOriginalText());
	return handleError();
}

void TokenSequenceParser::addEntry(int result, TokenSequence tokens, TokenValueSequence values)
{
	Entry entry = { tokens, values, result };
	entries.push_back(entry);
}

bool TokenSequenceParser::parse(Parser& parser, int& result)
{
	for (Entry& entry: entries)
	{
		TokenizerPosition pos = parser.getTokenizer()->getPosition();
		auto values = entry.values.begin();

		bool valid = true;
		for (TokenType type: entry.tokens)
		{
			// check of token type matches
			const Token& token = parser.nextToken();
			if (token.type != type)
			{
				valid = false;
				break;
			}

			// if necessary, check if the value of the token also matches
			if (type == TokenType::Identifier)
			{
				if (values == entry.values.end() || values->textValue != token.getStringValue())
				{
					valid = false;
					break;
				}

				values++;
			} else if (type == TokenType::Integer)
			{
				if (values == entry.values.end() || values->intValue != token.intValue)
				{
					valid = false;
					break;
				}

				values++;
			}
		}

		if (valid && values == entry.values.end())
		{
			result = entry.result;
			return true;
		}

		parser.getTokenizer()->setPosition(pos);
	}

	return false;
}

// file: Parser/Tokenizer.cpp


#include <algorithm>

//
// Tokenizer
//

std::vector<std::vector<Token>> Tokenizer::equValues;

Tokenizer::Tokenizer()
{
	position.it = tokens.begin();
	invalidToken.type = TokenType::Invalid;
	invalidToken.setOriginalText(L"Unexpected end of token stream");
}

bool Tokenizer::processElement(TokenList::iterator& it)
{
	if (it == tokens.end())
		return false;

	while (!(*it).checked)
	{
		bool replaced = false;
		if ((*it).type == TokenType::Identifier)
		{
			const std::wstring stringValue = (*it).getStringValue();
			for (const Replacement& replacement: replacements)
			{
				// if the identifier matches, add all of its tokens
				if (replacement.identifier == stringValue)
				{
					TokenList::iterator insertIt = it;
					insertIt++;

					// replace old token with the new tokens
					// replace the first token manually so that any iterators
					// are still guaranteed to be valid
					(*it) = replacement.value[0];
					tokens.insert(insertIt,replacement.value.begin()+1, replacement.value.end());

					// If the value at this position didn't change, then just keep going.
					// Otherwise we'd be stuck in an endless replace loop
					if (stringValue != (*it).getStringValue())
						replaced = true;
					break;
				}
			}

			if (replaced)
				continue;

			// check for equs
			size_t index;
			if (Global.symbolTable.findEquation(stringValue,Global.FileInfo.FileNum,Global.Section,index))
			{
				TokenList::iterator nextIt = it;
				std::advance(nextIt, 1);

				// check if this is another equ with the same name.
				// if so, keep equ redefinitions for later error handling
				if (nextIt != tokens.end() && nextIt->type == TokenType::Equ)
					break;

				// make room for the replacement tokens
				const std::vector<Token>& replacement = equValues[index];
				tokens.insert(nextIt, replacement.size()-1, {});

				// insert replacement tokens, while keeping the file info of the original token
				Token originalToken = *it;

				TokenList::iterator insertIt = it;
				for (const Token& token: replacement)
				{
					(*insertIt) = token;
					insertIt->line = originalToken.line;
					insertIt->column = originalToken.column;
					std::advance(insertIt, 1);
				}

				replaced = true;
				continue;
			}
		}

		if (!replaced)
			(*it).checked = true;
	}

	return true;
}

const Token& Tokenizer::nextToken()
{
	if (!processElement(position.it))
		return invalidToken;

	return *position.it++;
}

const Token& Tokenizer::peekToken(int ahead)
{
	auto it = position.it;
	for (int i = 0; i < ahead; i++)
	{
		if (!processElement(it))
			return invalidToken;

		it++;
	}

	if (!processElement(it))
		return invalidToken;

	return *it;
}

void Tokenizer::eatTokens(int num)
{
	for (int i = 0; i < num; i++)
	{
		if (!processElement(position.it))
			break;
		position.it++;
	}
}

void Tokenizer::skipLookahead()
{
	//position.index = tokens.size();
}

std::vector<Token> Tokenizer::getTokens(TokenizerPosition start, TokenizerPosition end) const
{
	std::vector<Token> result;

	for (auto it = start.it; it != end.it; it++)
	{
		Token tok = *it;
		tok.checked = false;
		result.push_back(tok);
	}

	return result;
}

void Tokenizer::registerReplacement(const std::wstring& identifier, std::vector<Token>& tokens)
{
	Replacement replacement { identifier, tokens };
	replacements.push_back(replacement);
}

void Tokenizer::registerReplacement(const std::wstring& identifier, const std::wstring& newValue)
{
	// Ensure the new identifier is lower case as it would be as a normally parsed string
	std::wstring lowerCase = newValue;
	std::transform(lowerCase.begin(), lowerCase.end(), lowerCase.begin(), ::towlower);

	Token tok;
	tok.type = TokenType::Identifier;
	tok.setStringValue(lowerCase);
	tok.setOriginalText(newValue);

	Replacement replacement;
	replacement.identifier = identifier;
	replacement.value.push_back(tok);

	replacements.push_back(replacement);
}

void Tokenizer::registerReplacementString(const std::wstring& identifier, const std::wstring& newValue)
{
	Token tok;
	tok.type = TokenType::String;
	tok.setStringValue(newValue);
	tok.setOriginalText(newValue);

	Replacement replacement;
	replacement.identifier = identifier;
	replacement.value.push_back(tok);

	replacements.push_back(replacement);
}

void Tokenizer::registerReplacementInteger(const std::wstring& identifier, int64_t newValue)
{
	Token tok;
	tok.type = TokenType::Integer;
	tok.intValue = newValue;
	tok.setOriginalText(tfm::format(L"%d", newValue).c_str());

	Replacement replacement;
	replacement.identifier = identifier;
	replacement.value.push_back(tok);

	replacements.push_back(replacement);
}

void Tokenizer::registerReplacementFloat(const std::wstring& identifier, double newValue)
{
	Token tok;
	tok.type = TokenType::Float;
	tok.floatValue = newValue;
	tok.setOriginalText(tfm::format(L"%g", newValue).c_str());

	Replacement replacement;
	replacement.identifier = identifier;
	replacement.value.push_back(tok);

	replacements.push_back(replacement);
}

void Tokenizer::addToken(Token token)
{
	tokens.push_back(std::move(token));
}

size_t Tokenizer::addEquValue(const std::vector<Token>& tokens)
{
	size_t index = equValues.size();
	equValues.push_back(tokens);
	return index;
}

void Tokenizer::resetLookaheadCheckMarks()
{
	auto it = position.it;
	while (it != tokens.end() && it->checked)
	{
		it->checked = false;
		it++;
	}
}

//
// FileTokenizer
//

inline bool isWhitespace(const std::wstring& text, size_t pos)
{
	if (pos >= text.size())
		return false;

	return text[pos] == ' ' || text[pos] == '\t';
}

inline bool isComment(const std::wstring& text, size_t pos)
{
	if (pos < text.size() && text[pos] == ';')
		return true;

	if (pos+1 < text.size() && text[pos+0] == '/' && text[pos+1] == '/')
		return true;

	return false;
}

inline bool isContinuation(const std::wstring& text, size_t pos)
{
	if (pos >= text.size())
		return false;

	return text[pos] == '\\';
}

inline bool isBlockComment(const std::wstring& text, size_t pos){
	return pos+1 < text.size() && text[pos+0] == '/' && text[pos+1] == '*';
}

inline bool isBlockCommentEnd(const std::wstring& text, size_t pos){
	return pos+1 < text.size() && text[pos+0] == '*' && text[pos+1] == '/';
}

void FileTokenizer::skipWhitespace()
{
	while (true)
	{
		if (isWhitespace(currentLine,linePos))
		{
			do { linePos++; } while (isWhitespace(currentLine,linePos));
		} else if (isComment(currentLine,linePos))
		{
			linePos = currentLine.size();
		} else if (isBlockComment(currentLine,linePos))
		{
			linePos += 2;
			while(!isBlockCommentEnd(currentLine,linePos))
			{
				linePos++;
				if (linePos >= currentLine.size())
				{
					if (isInputAtEnd())
					{
						createToken(TokenType::Invalid,linePos,L"Unexpected end of file in block comment");
						addToken(token);
						return;
					}
					currentLine = input->readLine();
					linePos = 0;
					lineNumber++;
				}
			}
			linePos += 2;
		} else
		{
			break;
		}
	}
}

void FileTokenizer::createToken(TokenType type, size_t length)
{
	token.type = type;
	token.line = lineNumber;
	token.column = linePos+1;
	token.setOriginalText(currentLine,linePos,length);

	linePos += length;
}

void FileTokenizer::createToken(TokenType type, size_t length, int64_t value)
{
	token.type = type;
	token.line = lineNumber;
	token.column = linePos+1;
	token.setOriginalText(currentLine,linePos,length);
	token.intValue = value;

	linePos += length;
}

void FileTokenizer::createToken(TokenType type, size_t length, double value)
{
	token.type = type;
	token.line = lineNumber;
	token.column = linePos+1;
	token.setOriginalText(currentLine,linePos,length);
	token.floatValue = value;

	linePos += length;
}

void FileTokenizer::createToken(TokenType type, size_t length, const std::wstring& value)
{
	createToken(type, length, value, 0, value.length());
}

void FileTokenizer::createToken(TokenType type, size_t length, const std::wstring& value, size_t valuePos, size_t valueLength)
{
	token.type = type;
	token.line = lineNumber;
	token.column = linePos+1;
	token.setOriginalText(currentLine,linePos,length);
	token.setStringValue(value,valuePos,valueLength);

	linePos += length;
}

void FileTokenizer::createTokenCurrentString(TokenType type, size_t length)
{
	token.type = type;
	token.line = lineNumber;
	token.column = linePos+1;
	token.setStringAndOriginalValue(currentLine,linePos,length);

	linePos += length;
}

bool FileTokenizer::parseOperator()
{
	wchar_t first = currentLine[linePos];
	wchar_t second = linePos+1 >= currentLine.size() ? '\0' : currentLine[linePos+1];

	switch (first)
	{
	case '(':
		createToken(TokenType::LParen,1);
		return true;
	case ')':
		createToken(TokenType::RParen,1);
		return true;
	case '+':
		createToken(TokenType::Plus,1);
		return true;
	case '-':
		createToken(TokenType::Minus,1);
		return true;
	case '*':
		createToken(TokenType::Mult,1);
		return true;
	case '/':
		createToken(TokenType::Div,1);
		return true;
	case '%':
		createToken(TokenType::Mod,1);
		return true;
	case '^':
		createToken(TokenType::Caret,1);
		return true;
	case '~':
		createToken(TokenType::Tilde,1);
		return true;
	case '<':
		if (second == '<')
			createToken(TokenType::LeftShift,2);
		else if (second == '=')
			createToken(TokenType::LessEqual,2);
		else
			createToken(TokenType::Less,1);
		return true;
	case '>':
		if (second == '>')
			createToken(TokenType::RightShift,2);
		else if (second == '=')
			createToken(TokenType::GreaterEqual,2);
		else
			createToken(TokenType::Greater,1);
		return true;
	case '=':
		if (second == '=')
			createToken(TokenType::Equal,2);
		else
			createToken(TokenType::Assign,1);
		return true;
	case '!':
		if (second == '=')
			createToken(TokenType::NotEqual,2);
		else
			createToken(TokenType::Exclamation,1);
		return true;
	case '&':
		if (second == '&')
			createToken(TokenType::LogAnd,2);
		else
			createToken(TokenType::BitAnd,1);
		return true;
	case '|':
		if (second == '|')
			createToken(TokenType::LogOr,2);
		else
			createToken(TokenType::BitOr,1);
		return true;
	case '?':
		createToken(TokenType::Question,1);
		return true;
	case ':':
		if (second == ':')
			createToken(TokenType::Separator,2);
		else
			createToken(TokenType::Colon,1);
		return true;
	case ',':
		createToken(TokenType::Comma,1);
		return true;
	case '[':
		createToken(TokenType::LBrack,1);
		return true;
	case ']':
		createToken(TokenType::RBrack,1);
		return true;
	case '#':
		createToken(TokenType::Hash,1);
		return true;
	case '{':
		createToken(TokenType::LBrace,1);
		return true;
	case '}':
		createToken(TokenType::RBrace,1);
		return true;
	case '$':
		createToken(TokenType::Dollar,1);
		return true;
	case L'\U000000B0':	// degree sign
		createToken(TokenType::Degree,1);
		return true;
	}

	return false;
}

bool FileTokenizer::convertInteger(size_t start, size_t end, int64_t& result)
{
	return stringToInt(currentLine, start, end, result);
}

bool FileTokenizer::convertFloat(size_t start, size_t end, double& result)
{
	std::wstring str = currentLine.substr(start, end - start);
	wchar_t* end_ptr;

	result = wcstod(str.c_str(), &end_ptr);
	return str.c_str() + str.size() == end_ptr;
}

Token FileTokenizer::loadToken()
{
	if (isInputAtEnd())
	{
		createToken(TokenType::Invalid,0);
		return std::move(token);
	}

	size_t pos = linePos;

	if (equActive)
	{
		while (pos < currentLine.size() && !isComment(currentLine,pos))
			pos++;

		createTokenCurrentString(TokenType::EquValue,pos-linePos);

		equActive = false;
		return std::move(token);
	}

	if (parseOperator())
		return std::move(token);

	wchar_t first = currentLine[pos];

	// character constants
	if (first == '\'' && pos+2 < currentLine.size() && currentLine[pos+2] == '\'')
	{
		createToken(TokenType::Integer,3,(int64_t)currentLine[pos+1]);
		return std::move(token);
	}

	// strings
	if (first == '"')
	{
		std::wstring text;
		pos++;

		bool valid = false;
		while (pos < currentLine.size())
		{
			if (pos+1 < currentLine.size() && currentLine[pos] == '\\')
			{
				if (currentLine[pos+1] == '"')
				{
					text += '"';
					pos += 2;
					continue;
				}

				if (currentLine[pos+1] == '\\')
				{
					text += '\\';
					pos += 2;
					continue;
				}
			}

			if (currentLine[pos] == '"')
			{
				pos++;
				valid = true;
				break;
			}

			text += currentLine[pos++];
		}

		if (!valid)
		{
			createToken(TokenType::Invalid,pos-linePos,L"Unexpected end of line in string constant");
			return std::move(token);
		}

		createToken(TokenType::String,pos-linePos,text);
		return std::move(token);
	}

	// numbers
	if (first >= '0' && first <= '9')
	{
		// find end of number
		size_t start = pos;
		size_t end = pos;
		bool isValid = true;
		bool foundPoint = false;
		bool foundExp = false;
		bool foundExpSign = false;
		bool isHex = start+1 < currentLine.size() && currentLine[start] == '0' && towlower(currentLine[start+1]) == 'x';

		while (end < currentLine.size() && (iswalnum(currentLine[end]) || currentLine[end] == '.'))
		{
			if (currentLine[end] == '.')
			{
				if (foundExp || foundPoint)
					isValid = false;
				foundPoint = true;
			} else if (towlower(currentLine[end]) == 'h' && !foundExpSign) {
				isHex = true;
			} else if (towlower(currentLine[end]) == 'e' && !isHex)
			{
				if (foundExp)
				{
					isValid = false;
				} else if (end+1 < currentLine.size() && (currentLine[end+1] == '+' || currentLine[end+1] == '-')){
					end++;
					if (end+1 >= currentLine.size() || !iswalnum(currentLine[end+1]))
						isValid = false;
					foundExpSign = true;
				}
				foundExp = true;
			}

			end++;
		}

		bool isFloat = foundPoint || (foundExp && !isHex);

		if (!isFloat)
		{
			int64_t value;
			if (!convertInteger(start,end,value))
			{
				createTokenCurrentString(TokenType::NumberString,end-start);
				return std::move(token);
			}

			createToken(TokenType::Integer,end-start,value);
		} else { // isFloat
			double value;
			if (!isValid)
			{
				createToken(TokenType::Invalid,end-start,L"Invalid floating point number");
				return std::move(token);
			}

			if (!convertFloat(start,end,value))
			{
				createTokenCurrentString(TokenType::NumberString,end-start);
				return std::move(token);
			}

			createToken(TokenType::Float,end-start,value);
		}

		return std::move(token);
	}

	// identifiers
	bool isFirst = true;
	while (pos < currentLine.size() && Global.symbolTable.isValidSymbolCharacter(currentLine[pos],isFirst))
	{
		pos++;
		isFirst = false;
	}

	if (pos == linePos)
	{
		std::wstring text = tfm::format(L"Invalid input '%c'",currentLine[pos]);
		createToken(TokenType::Invalid,1,text);
		return std::move(token);
	}

	std::wstring text = currentLine.substr(linePos,pos-linePos);
	bool textLowered = false;
	// Lowercase is common, let's try to avoid a copy.
	if (std::any_of(text.begin(), text.end(), ::iswupper))
	{
		std::transform(text.begin(), text.end(), text.begin(), ::towlower);
		textLowered = true;
	}

	if (text == L"equ")
	{
		createToken(TokenType::Equ,pos-linePos);
		equActive = true;
	} else if (textLowered) {
		createToken(TokenType::Identifier,pos-linePos,text);
	} else {
		createTokenCurrentString(TokenType::Identifier,pos-linePos);
	}

	return std::move(token);
}

bool FileTokenizer::isInputAtEnd()
{
	return linePos >= currentLine.size() && input->atEnd();
}

bool FileTokenizer::init(TextFile* input)
{
	clearTokens();

	lineNumber = 1;
	linePos = 0;
	equActive = false;
	currentLine = input->readLine();

	this->input = input;
	if (input != nullptr && input->isOpen())
	{
		while (!isInputAtEnd())
		{
			bool addSeparator = true;

			skipWhitespace();
			if (isContinuation(currentLine, linePos))
			{
				linePos++;
				skipWhitespace();
				if (linePos < currentLine.size())
				{
					createToken(TokenType::Invalid,0,
						L"Unexpected character after line continuation character");
					addToken(token);
				}

				addSeparator = false;
			} else if(linePos < currentLine.size())
			{
				addToken(loadToken());
			}

			if (linePos >= currentLine.size())
			{
				if (addSeparator)
				{
					createToken(TokenType::Separator,0);
					addToken(token);
				}

				if (input->atEnd())
					break;

				currentLine = input->readLine();
				linePos = 0;
				lineNumber++;
			}
		}

		resetPosition();
		return true;
	}

	return false;
}

// file: Util/ByteArray.cpp


#include <cstring>

ByteArray::ByteArray()
{
	data_ = nullptr;
	size_ = allocatedSize_ = 0;
}

ByteArray::ByteArray(const ByteArray& other)
{
	data_ = nullptr;
	size_ = allocatedSize_ = 0;
	append(other);
}

ByteArray::ByteArray(byte* data, size_t size)
{
	data_ = nullptr;
	size_ = allocatedSize_ = 0;
	append(data,size);
}

ByteArray::ByteArray(ByteArray&& other)
{
	data_ = other.data_;
	size_ = other.size_;
	allocatedSize_ = other.allocatedSize_;
	other.data_ = nullptr;
	other.allocatedSize_ = other.size_ = 0;
}

ByteArray::~ByteArray()
{
	free(data_);
}

ByteArray& ByteArray::operator=(ByteArray& other)
{
	free(data_);
	data_ = nullptr;
	size_ = allocatedSize_ = 0;
	append(other);

	return *this;
}

ByteArray& ByteArray::operator=(ByteArray&& other)
{
	data_ = other.data_;
	size_ = other.size_;
	allocatedSize_ = other.allocatedSize_;
	other.data_ = nullptr;
	other.allocatedSize_ = other.size_ = 0;
	return *this;
}

void ByteArray::grow(size_t neededSize)
{
	if (neededSize < allocatedSize_) return;

	// align to next 0.5kb... it's a start
	allocatedSize_ = ((neededSize+511)/512)*512;
	if (data_ == nullptr)
	{
		data_ = (byte*) malloc(allocatedSize_);
	} else {
		data_ = (byte*) realloc(data_,allocatedSize_);
	}
}

size_t ByteArray::append(const ByteArray& other)
{
	size_t oldSize = size();
	size_t otherSize = other.size();
	grow(size()+otherSize);
	memcpy(&data_[size_],other.data(),otherSize);
	size_ += otherSize;
	return oldSize;
}

size_t ByteArray::append(void* data, size_t size)
{
	size_t oldSize = this->size();
	grow(this->size()+size);
	memcpy(&data_[size_],data,size);
	this->size_ += size;
	return oldSize;
}

void ByteArray::replaceBytes(size_t pos, byte* data, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		replaceByte(pos+i,data[i]);
	}
}

void ByteArray::reserveBytes(size_t count, byte value)
{
	grow(this->size()+count);
	memset(&data_[size_],value,count);
	size_ += count;
}

void ByteArray::alignSize(size_t alignment)
{
	if (alignment <= 0) return;

	while (size_ % alignment)
	{
		appendByte(0);
	}
}

void ByteArray::resize(size_t newSize)
{
	grow(newSize);
	size_ = newSize;
}

ByteArray ByteArray::mid(size_t start, ssize_t length)
{
	ByteArray ret;

	if (length < 0)
		length = size_-start;

	if (start >= size_)
		return ret;

	ret.grow(length);
	ret.size_ = length;
	memcpy(ret.data_,&data_[start],length);
	return ret;
}

ByteArray ByteArray::fromFile(const fs::path& fileName, unsigned long start, size_t size)
{
	fs::ifstream stream(fileName, fs::fstream::in | fs::fstream::binary);
	if (!stream.is_open())
		return {};

	auto fileSize = fs::file_size(fileName);
	if (start >= fileSize)
		return {};

	if (size == 0 || start+(long)size > fileSize)
		size = fileSize-start;

	stream.seekg(start);

	ByteArray ret;
	ret.grow(size);

	stream.read(reinterpret_cast<char *>(ret.data()), size);
	ret.size_ = stream.gcount();

	return ret;
}

bool ByteArray::toFile(const fs::path& fileName)
{
	fs::ofstream stream(fileName, fs::fstream::out | fs::fstream::binary | fs::fstream::trunc);
	if (!stream.is_open())
		return {};

	stream.write(reinterpret_cast<const char *>(data_), size_);
	return !stream.fail();
}

// file: Util/CRC.cpp

#include <stdio.h>

const unsigned short Crc16Table[] = /* CRC lookup table */
{
	0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
	0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
	0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
	0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
	0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
	0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
	0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
	0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
	0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
	0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
	0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
	0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
	0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
	0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
	0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
	0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
	0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
	0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
	0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
	0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
	0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
	0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
	0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
	0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
	0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
	0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
	0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
	0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
	0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
	0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
	0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
	0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

const unsigned int Crc32Table[256] = {
	0x00000000,0x77073096,0xEE0E612C,0x990951BA,0x076DC419,0x706AF48F,0xE963A535,0x9E6495A3,
	0x0EDB8832,0x79DCB8A4,0xE0D5E91E,0x97D2D988,0x09B64C2B,0x7EB17CBD,0xE7B82D07,0x90BF1D91,
	0x1DB71064,0x6AB020F2,0xF3B97148,0x84BE41DE,0x1ADAD47D,0x6DDDE4EB,0xF4D4B551,0x83D385C7,
	0x136C9856,0x646BA8C0,0xFD62F97A,0x8A65C9EC,0x14015C4F,0x63066CD9,0xFA0F3D63,0x8D080DF5,
	0x3B6E20C8,0x4C69105E,0xD56041E4,0xA2677172,0x3C03E4D1,0x4B04D447,0xD20D85FD,0xA50AB56B,
	0x35B5A8FA,0x42B2986C,0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,0xDCD60DCF,0xABD13D59,
	0x26D930AC,0x51DE003A,0xC8D75180,0xBFD06116,0x21B4F4B5,0x56B3C423,0xCFBA9599,0xB8BDA50F,
	0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,0x2F6F7C87,0x58684C11,0xC1611DAB,0xB6662D3D,
	0x76DC4190,0x01DB7106,0x98D220BC,0xEFD5102A,0x71B18589,0x06B6B51F,0x9FBFE4A5,0xE8B8D433,
	0x7807C9A2,0x0F00F934,0x9609A88E,0xE10E9818,0x7F6A0DBB,0x086D3D2D,0x91646C97,0xE6635C01,
	0x6B6B51F4,0x1C6C6162,0x856530D8,0xF262004E,0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,
	0x65B0D9C6,0x12B7E950,0x8BBEB8EA,0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xFBD44C65,
	0x4DB26158,0x3AB551CE,0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,0xA4D1C46D,0xD3D6F4FB,
	0x4369E96A,0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,
	0x5005713C,0x270241AA,0xBE0B1010,0xC90C2086,0x5768B525,0x206F85B3,0xB966D409,0xCE61E49F,
	0x5EDEF90E,0x29D9C998,0xB0D09822,0xC7D7A8B4,0x59B33D17,0x2EB40D81,0xB7BD5C3B,0xC0BA6CAD,
	0xEDB88320,0x9ABFB3B6,0x03B6E20C,0x74B1D29A,0xEAD54739,0x9DD277AF,0x04DB2615,0x73DC1683,
	0xE3630B12,0x94643B84,0x0D6D6A3E,0x7A6A5AA8,0xE40ECF0B,0x9309FF9D,0x0A00AE27,0x7D079EB1,
	0xF00F9344,0x8708A3D2,0x1E01F268,0x6906C2FE,0xF762575D,0x806567CB,0x196C3671,0x6E6B06E7,
	0xFED41B76,0x89D32BE0,0x10DA7A5A,0x67DD4ACC,0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,
	0xD6D6A3E8,0xA1D1937E,0x38D8C2C4,0x4FDFF252,0xD1BB67F1,0xA6BC5767,0x3FB506DD,0x48B2364B,
	0xD80D2BDA,0xAF0A1B4C,0x36034AF6,0x41047A60,0xDF60EFC3,0xA867DF55,0x316E8EEF,0x4669BE79,
	0xCB61B38C,0xBC66831A,0x256FD2A0,0x5268E236,0xCC0C7795,0xBB0B4703,0x220216B9,0x5505262F,
	0xC5BA3BBE,0xB2BD0B28,0x2BB45A92,0x5CB36A04,0xC2D7FFA7,0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,
	0x9B64C2B0,0xEC63F226,0x756AA39C,0x026D930A,0x9C0906A9,0xEB0E363F,0x72076785,0x05005713,
	0x95BF4A82,0xE2B87A14,0x7BB12BAE,0x0CB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0x0BDBDF21,
	0x86D3D2D4,0xF1D4E242,0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,
	0x88085AE6,0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,0xF862AE69,0x616BFFD3,0x166CCF45,
	0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3C2,0xA7672661,0xD06016F7,0x4969474D,0x3E6E77DB,
	0xAED16A4A,0xD9D65ADC,0x40DF0B66,0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,0x47B2CF7F,0x30B5FFE9,
	0xBDBDF21C,0xCABAC28A,0x53B39330,0x24B4A3A6,0xBAD03605,0xCDD70693,0x54DE5729,0x23D967BF,
	0xB3667A2E,0xC4614AB8,0x5D681B02,0x2A6F2B94,0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2D02EF8D
};

unsigned short getCrc16(unsigned char* Source, size_t len)
{
	unsigned short crc = 0xFFFF;

	while (len--)
	{
		crc = (crc >> 8) ^ Crc16Table[(crc ^ *Source++) & 0xFF];
	}
	return crc;
}

unsigned int getCrc32(unsigned char* Source, size_t len)
{
	unsigned int crc = 0xFFFFFFFF;

	while (len--)
	{
		crc = (crc >> 8) ^ Crc32Table[(crc & 0xFF) ^ *Source++];
	}

	return crc ^ 0xffffffff;
}


unsigned int getChecksum(unsigned char* Source, size_t len)
{
	int checksum = 0;
	for (size_t i = 0; i < len; i++)
		checksum += *Source++;
	return checksum;
}

// file: Util/EncodingTable.cpp


#define MAXHEXLENGTH 32

Trie::Trie()
{
	Node root { 0, false, 0 };
	nodes.push_back(root);
}

void Trie::insert(const wchar_t* text, size_t value)
{
	size_t node = 0;	// root node

	// traverse existing nodes
	while (*text != 0)
	{
		LookupEntry lookupEntry { node, *text };
		auto it = lookup.find(lookupEntry);
		if (it == lookup.end())
			break;

		node = it->second;
		text++;
	}

	// add new nodes as necessary
	while (*text != 0)
	{
		Node newNode { nodes.size(), false, 0 };
		nodes.push_back(newNode);

		LookupEntry lookupEntry { node, *text };
		lookup[lookupEntry] = newNode.index;
		node = newNode.index;
		text++;
	}

	// set value
	nodes[node].hasValue = true;
	nodes[node].value = value;
}

void Trie::insert(wchar_t character, size_t value)
{
	wchar_t str[2];
	str[0] = character;
	str[1] = 0;
	insert(str,value);
}

bool Trie::findLongestPrefix(const wchar_t* text, size_t& result)
{
	size_t node = 0;		// root node
	size_t valueNode = 0;	// remember last node that had a value

	while (*text != 0)
	{
		if (nodes[node].hasValue)
			valueNode = node;

		LookupEntry lookupEntry { node, *text++ };
		auto it = lookup.find(lookupEntry);

		if (it == lookup.end())
			break;

		node = it->second;
	}

	if (nodes[node].hasValue)
		valueNode = node;

	result = nodes[valueNode].value;
	return nodes[valueNode].hasValue;
}

EncodingTable::EncodingTable()
{

}

EncodingTable::~EncodingTable()
{

}

void EncodingTable::clear()
{
	hexData.clear();
	entries.clear();
}

int parseHexString(std::wstring& hex, unsigned char* dest)
{
	for (size_t i = 0; i < hex.size(); i++)
	{
		wchar_t source = towlower(hex[i]);
		int value;

		if (source >= 'a' && source <= 'f')
		{
			value = source-'a'+10;
		} else if (source >= '0' && source <= '9')
		{
			value = source-'0';
		} else {
			return -1;
		}

		size_t index = i/2;
		if (i % 2)
			dest[index] = (dest[index] << 4) | value;
		else
			dest[index] = value;
	}

	return (int) hex.size()/2;
}

bool EncodingTable::load(const fs::path& fileName, TextFile::Encoding encoding)
{
	unsigned char hexBuffer[MAXHEXLENGTH];

	TextFile input;
	if (!input.open(fileName,TextFile::Read,encoding))
		return false;

	hexData.clear();
	entries.clear();
	setTerminationEntry((unsigned char*)"\0",1);

	while (!input.atEnd())
	{
		std::wstring line = input.readLine();
		if (line.empty() || line[0] == '*') continue;

		if (line[0] == '/')
		{
			std::wstring hex = line.substr(1);
			if (hex.empty() || hex.length() > 2*MAXHEXLENGTH)
			{
				// error
				continue;
			}

			int length = parseHexString(hex,hexBuffer);
			if (length == -1)
			{
				// error
				continue;
			}

			setTerminationEntry(hexBuffer,length);
		} else {
			size_t pos = line.find(L'=');
			std::wstring hex = line.substr(0,pos);
			std::wstring value = line.substr(pos+1);

			if (hex.empty() || value.empty() || hex.length() > 2*MAXHEXLENGTH)
			{
				// error
				continue;
			}

			int length = parseHexString(hex,hexBuffer);
			if (length == -1)
			{
				// error
				continue;
			}

			addEntry(hexBuffer,length,value);
		}
	}

	return true;
}

void EncodingTable::addEntry(unsigned char* hex, size_t hexLength, const std::wstring& value)
{
	if (value.size() == 0)
		return;

	// insert into trie
	size_t index = entries.size();
	lookup.insert(value.c_str(),index);

	// add entry
	TableEntry entry;
	entry.hexPos = hexData.append(hex,hexLength);
	entry.hexLen = hexLength;
	entry.valueLen = value.size();

	entries.push_back(entry);
}

void EncodingTable::addEntry(unsigned char* hex, size_t hexLength, wchar_t value)
{
	if (value == '\0')
		return;

	// insert into trie
	size_t index = entries.size();
	lookup.insert(value,index);

	// add entry
	TableEntry entry;
	entry.hexPos = hexData.append(hex,hexLength);
	entry.hexLen = hexLength;
	entry.valueLen = 1;

	entries.push_back(entry);

}

void EncodingTable::setTerminationEntry(unsigned char* hex, size_t hexLength)
{
	terminationEntry.hexPos = hexData.append(hex,hexLength);
	terminationEntry.hexLen = hexLength;
	terminationEntry.valueLen = 0;
}

ByteArray EncodingTable::encodeString(const std::wstring& str, bool writeTermination)
{
	ByteArray result;

	size_t pos = 0;
	while (pos < str.size())
	{
		size_t index;
		if (!lookup.findLongestPrefix(str.c_str()+pos,index))
		{
			// error
			return ByteArray();
		}

		TableEntry& entry = entries[index];
		for (size_t i = 0; i < entry.hexLen; i++)
		{
			result.appendByte(hexData[entry.hexPos+i]);
		}

		pos += entry.valueLen;
	}

	if (writeTermination)
	{
		TableEntry& entry = terminationEntry;
		for (size_t i = 0; i < entry.hexLen; i++)
		{
			result.appendByte(hexData[entry.hexPos+i]);
		}
	}

	return result;
}

ByteArray EncodingTable::encodeTermination()
{
	ByteArray result;

	TableEntry& entry = terminationEntry;
	for (size_t i = 0; i < entry.hexLen; i++)
	{
		result.appendByte(hexData[entry.hexPos+i]);
	}

	return result;
}

// file: Util/FileClasses.cpp


#include <cassert>
#include <cstring>

const wchar_t SjisToUnicodeTable1[] =
{
	// 0X0080 to 0X00FF
	0x0080, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFF61, 0xFF62, 0xFF63, 0xFF64, 0xFF65, 0xFF66, 0xFF67, 0xFF68, 0xFF69, 0xFF6A, 0xFF6B, 0xFF6C, 0xFF6D, 0xFF6E, 0xFF6F,
	0xFF70, 0xFF71, 0xFF72, 0xFF73, 0xFF74, 0xFF75, 0xFF76, 0xFF77, 0xFF78, 0xFF79, 0xFF7A, 0xFF7B, 0xFF7C, 0xFF7D, 0xFF7E, 0xFF7F,
	0xFF80, 0xFF81, 0xFF82, 0xFF83, 0xFF84, 0xFF85, 0xFF86, 0xFF87, 0xFF88, 0xFF89, 0xFF8A, 0xFF8B, 0xFF8C, 0xFF8D, 0xFF8E, 0xFF8F,
	0xFF90, 0xFF91, 0xFF92, 0xFF93, 0xFF94, 0xFF95, 0xFF96, 0xFF97, 0xFF98, 0xFF99, 0xFF9A, 0xFF9B, 0xFF9C, 0xFF9D, 0xFF9E, 0xFF9F,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
};

const wchar_t SjisToUnicodeTable2[] =
{
	// 0X8100 to 0X81FF
	0x3000, 0x3001, 0x3002, 0xFF0C, 0xFF0E, 0x30FB, 0xFF1A, 0xFF1B, 0xFF1F, 0xFF01, 0x309B, 0x309C, 0x00B4, 0xFF40, 0x00A8, 0xFF3E,
	0xFFE3, 0xFF3F, 0x30FD, 0x30FE, 0x309D, 0x309E, 0x3003, 0x4EDD, 0x3005, 0x3006, 0x3007, 0x30FC, 0x2015, 0x2010, 0xFF0F, 0xFF3C,
	0xFF5E, 0x2225, 0xFF5C, 0x2026, 0x2025, 0x2018, 0x2019, 0x201C, 0x201D, 0xFF08, 0xFF09, 0x3014, 0x3015, 0xFF3B, 0xFF3D, 0xFF5B,
	0xFF5D, 0x3008, 0x3009, 0x300A, 0x300B, 0x300C, 0x300D, 0x300E, 0x300F, 0x3010, 0x3011, 0xFF0B, 0xFF0D, 0x00B1, 0x00D7, 0xFFFF,
	0x00F7, 0xFF1D, 0x2260, 0xFF1C, 0xFF1E, 0x2266, 0x2267, 0x221E, 0x2234, 0x2642, 0x2640, 0x00B0, 0x2032, 0x2033, 0x2103, 0xFFE5,
	0xFF04, 0xFFE0, 0xFFE1, 0xFF05, 0xFF03, 0xFF06, 0xFF0A, 0xFF20, 0x00A7, 0x2606, 0x2605, 0x25CB, 0x25CF, 0x25CE, 0x25C7, 0x25C6,
	0x25A1, 0x25A0, 0x25B3, 0x25B2, 0x25BD, 0x25BC, 0x203B, 0x3012, 0x2192, 0x2190, 0x2191, 0x2193, 0x3013, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x2208, 0x220B, 0x2286, 0x2287, 0x2282, 0x2283, 0x222A, 0x2229,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x2227, 0x2228, 0xFFE2, 0x21D2, 0x21D4, 0x2200, 0x2203, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x2220, 0x22A5, 0x2312, 0x2202, 0x2207, 0x2261,
	0x2252, 0x226A, 0x226B, 0x221A, 0x223D, 0x221D, 0x2235, 0x222B, 0x222C, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x212B, 0x2030, 0x266F, 0x266D, 0x266A, 0x2020, 0x2021, 0x00B6, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x25EF, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0X8200 to 0X82FF
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFF10,
	0xFF11, 0xFF12, 0xFF13, 0xFF14, 0xFF15, 0xFF16, 0xFF17, 0xFF18, 0xFF19, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFF21, 0xFF22, 0xFF23, 0xFF24, 0xFF25, 0xFF26, 0xFF27, 0xFF28, 0xFF29, 0xFF2A, 0xFF2B, 0xFF2C, 0xFF2D, 0xFF2E, 0xFF2F, 0xFF30,
	0xFF31, 0xFF32, 0xFF33, 0xFF34, 0xFF35, 0xFF36, 0xFF37, 0xFF38, 0xFF39, 0xFF3A, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFF41, 0xFF42, 0xFF43, 0xFF44, 0xFF45, 0xFF46, 0xFF47, 0xFF48, 0xFF49, 0xFF4A, 0xFF4B, 0xFF4C, 0xFF4D, 0xFF4E, 0xFF4F,
	0xFF50, 0xFF51, 0xFF52, 0xFF53, 0xFF54, 0xFF55, 0xFF56, 0xFF57, 0xFF58, 0xFF59, 0xFF5A, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x3041,
	0x3042, 0x3043, 0x3044, 0x3045, 0x3046, 0x3047, 0x3048, 0x3049, 0x304A, 0x304B, 0x304C, 0x304D, 0x304E, 0x304F, 0x3050, 0x3051,
	0x3052, 0x3053, 0x3054, 0x3055, 0x3056, 0x3057, 0x3058, 0x3059, 0x305A, 0x305B, 0x305C, 0x305D, 0x305E, 0x305F, 0x3060, 0x3061,
	0x3062, 0x3063, 0x3064, 0x3065, 0x3066, 0x3067, 0x3068, 0x3069, 0x306A, 0x306B, 0x306C, 0x306D, 0x306E, 0x306F, 0x3070, 0x3071,
	0x3072, 0x3073, 0x3074, 0x3075, 0x3076, 0x3077, 0x3078, 0x3079, 0x307A, 0x307B, 0x307C, 0x307D, 0x307E, 0x307F, 0x3080, 0x3081,
	0x3082, 0x3083, 0x3084, 0x3085, 0x3086, 0x3087, 0x3088, 0x3089, 0x308A, 0x308B, 0x308C, 0x308D, 0x308E, 0x308F, 0x3090, 0x3091,
	0x3092, 0x3093, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0X8300 to 0X83FF
	0x30A1, 0x30A2, 0x30A3, 0x30A4, 0x30A5, 0x30A6, 0x30A7, 0x30A8, 0x30A9, 0x30AA, 0x30AB, 0x30AC, 0x30AD, 0x30AE, 0x30AF, 0x30B0,
	0x30B1, 0x30B2, 0x30B3, 0x30B4, 0x30B5, 0x30B6, 0x30B7, 0x30B8, 0x30B9, 0x30BA, 0x30BB, 0x30BC, 0x30BD, 0x30BE, 0x30BF, 0x30C0,
	0x30C1, 0x30C2, 0x30C3, 0x30C4, 0x30C5, 0x30C6, 0x30C7, 0x30C8, 0x30C9, 0x30CA, 0x30CB, 0x30CC, 0x30CD, 0x30CE, 0x30CF, 0x30D0,
	0x30D1, 0x30D2, 0x30D3, 0x30D4, 0x30D5, 0x30D6, 0x30D7, 0x30D8, 0x30D9, 0x30DA, 0x30DB, 0x30DC, 0x30DD, 0x30DE, 0x30DF, 0xFFFF,
	0x30E0, 0x30E1, 0x30E2, 0x30E3, 0x30E4, 0x30E5, 0x30E6, 0x30E7, 0x30E8, 0x30E9, 0x30EA, 0x30EB, 0x30EC, 0x30ED, 0x30EE, 0x30EF,
	0x30F0, 0x30F1, 0x30F2, 0x30F3, 0x30F4, 0x30F5, 0x30F6, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0391,
	0x0392, 0x0393, 0x0394, 0x0395, 0x0396, 0x0397, 0x0398, 0x0399, 0x039A, 0x039B, 0x039C, 0x039D, 0x039E, 0x039F, 0x03A0, 0x03A1,
	0x03A3, 0x03A4, 0x03A5, 0x03A6, 0x03A7, 0x03A8, 0x03A9, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x03B1,
	0x03B2, 0x03B3, 0x03B4, 0x03B5, 0x03B6, 0x03B7, 0x03B8, 0x03B9, 0x03BA, 0x03BB, 0x03BC, 0x03BD, 0x03BE, 0x03BF, 0x03C0, 0x03C1,
	0x03C3, 0x03C4, 0x03C5, 0x03C6, 0x03C7, 0x03C8, 0x03C9, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0X8400 to 0X84FF
	0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0401, 0x0416, 0x0417, 0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E,
	0x041F, 0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427, 0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E,
	0x042F, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0451, 0x0436, 0x0437, 0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0xFFFF,
	0x043E, 0x043F, 0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447, 0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D,
	0x044E, 0x044F, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x2500,
	0x2502, 0x250C, 0x2510, 0x2518, 0x2514, 0x251C, 0x252C, 0x2524, 0x2534, 0x253C, 0x2501, 0x2503, 0x250F, 0x2513, 0x251B, 0x2517,
	0x2523, 0x2533, 0x252B, 0x253B, 0x254B, 0x2520, 0x252F, 0x2528, 0x2537, 0x253F, 0x251D, 0x2530, 0x2525, 0x2538, 0x2542, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
};

const wchar_t SjisToUnicodeTable3[] =
{
	// 0X8700 to 0X87FF
	0x2460, 0x2461, 0x2462, 0x2463, 0x2464, 0x2465, 0x2466, 0x2467, 0x2468, 0x2469, 0x246A, 0x246B, 0x246C, 0x246D, 0x246E, 0x246F,
	0x2470, 0x2471, 0x2472, 0x2473, 0x2160, 0x2161, 0x2162, 0x2163, 0x2164, 0x2165, 0x2166, 0x2167, 0x2168, 0x2169, 0xFFFF, 0x3349,
	0x3314, 0x3322, 0x334D, 0x3318, 0x3327, 0x3303, 0x3336, 0x3351, 0x3357, 0x330D, 0x3326, 0x3323, 0x332B, 0x334A, 0x333B, 0x339C,
	0x339D, 0x339E, 0x338E, 0x338F, 0x33C4, 0x33A1, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x337B, 0xFFFF,
	0x301D, 0x301F, 0x2116, 0x33CD, 0x2121, 0x32A4, 0x32A5, 0x32A6, 0x32A7, 0x32A8, 0x3231, 0x3232, 0x3239, 0x337E, 0x337D, 0x337C,
	0x2252, 0x2261, 0x222B, 0x222E, 0x2211, 0x221A, 0x22A5, 0x2220, 0x221F, 0x22BF, 0x2235, 0x2229, 0x222A, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0X8800 to 0X88FF
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x4E9C,
	0x5516, 0x5A03, 0x963F, 0x54C0, 0x611B, 0x6328, 0x59F6, 0x9022, 0x8475, 0x831C, 0x7A50, 0x60AA, 0x63E1, 0x6E25, 0x65ED, 0x8466,
	0x82A6, 0x9BF5, 0x6893, 0x5727, 0x65A1, 0x6271, 0x5B9B, 0x59D0, 0x867B, 0x98F4, 0x7D62, 0x7DBE, 0x9B8E, 0x6216, 0x7C9F, 0x88B7,
	0x5B89, 0x5EB5, 0x6309, 0x6697, 0x6848, 0x95C7, 0x978D, 0x674F, 0x4EE5, 0x4F0A, 0x4F4D, 0x4F9D, 0x5049, 0x56F2, 0x5937, 0x59D4,
	0x5A01, 0x5C09, 0x60DF, 0x610F, 0x6170, 0x6613, 0x6905, 0x70BA, 0x754F, 0x7570, 0x79FB, 0x7DAD, 0x7DEF, 0x80C3, 0x840E, 0x8863,
	0x8B02, 0x9055, 0x907A, 0x533B, 0x4E95, 0x4EA5, 0x57DF, 0x80B2, 0x90C1, 0x78EF, 0x4E00, 0x58F1, 0x6EA2, 0x9038, 0x7A32, 0x8328,
	0x828B, 0x9C2F, 0x5141, 0x5370, 0x54BD, 0x54E1, 0x56E0, 0x59FB, 0x5F15, 0x98F2, 0x6DEB, 0x80E4, 0x852D, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0X8900 to 0X89FF
	0x9662, 0x9670, 0x96A0, 0x97FB, 0x540B, 0x53F3, 0x5B87, 0x70CF, 0x7FBD, 0x8FC2, 0x96E8, 0x536F, 0x9D5C, 0x7ABA, 0x4E11, 0x7893,
	0x81FC, 0x6E26, 0x5618, 0x5504, 0x6B1D, 0x851A, 0x9C3B, 0x59E5, 0x53A9, 0x6D66, 0x74DC, 0x958F, 0x5642, 0x4E91, 0x904B, 0x96F2,
	0x834F, 0x990C, 0x53E1, 0x55B6, 0x5B30, 0x5F71, 0x6620, 0x66F3, 0x6804, 0x6C38, 0x6CF3, 0x6D29, 0x745B, 0x76C8, 0x7A4E, 0x9834,
	0x82F1, 0x885B, 0x8A60, 0x92ED, 0x6DB2, 0x75AB, 0x76CA, 0x99C5, 0x60A6, 0x8B01, 0x8D8A, 0x95B2, 0x698E, 0x53AD, 0x5186, 0xFFFF,
	0x5712, 0x5830, 0x5944, 0x5BB4, 0x5EF6, 0x6028, 0x63A9, 0x63F4, 0x6CBF, 0x6F14, 0x708E, 0x7114, 0x7159, 0x71D5, 0x733F, 0x7E01,
	0x8276, 0x82D1, 0x8597, 0x9060, 0x925B, 0x9D1B, 0x5869, 0x65BC, 0x6C5A, 0x7525, 0x51F9, 0x592E, 0x5965, 0x5F80, 0x5FDC, 0x62BC,
	0x65FA, 0x6A2A, 0x6B27, 0x6BB4, 0x738B, 0x7FC1, 0x8956, 0x9D2C, 0x9D0E, 0x9EC4, 0x5CA1, 0x6C96, 0x837B, 0x5104, 0x5C4B, 0x61B6,
	0x81C6, 0x6876, 0x7261, 0x4E59, 0x4FFA, 0x5378, 0x6069, 0x6E29, 0x7A4F, 0x97F3, 0x4E0B, 0x5316, 0x4EEE, 0x4F55, 0x4F3D, 0x4FA1,
	0x4F73, 0x52A0, 0x53EF, 0x5609, 0x590F, 0x5AC1, 0x5BB6, 0x5BE1, 0x79D1, 0x6687, 0x679C, 0x67B6, 0x6B4C, 0x6CB3, 0x706B, 0x73C2,
	0x798D, 0x79BE, 0x7A3C, 0x7B87, 0x82B1, 0x82DB, 0x8304, 0x8377, 0x83EF, 0x83D3, 0x8766, 0x8AB2, 0x5629, 0x8CA8, 0x8FE6, 0x904E,
	0x971E, 0x868A, 0x4FC4, 0x5CE8, 0x6211, 0x7259, 0x753B, 0x81E5, 0x82BD, 0x86FE, 0x8CC0, 0x96C5, 0x9913, 0x99D5, 0x4ECB, 0x4F1A,
	0x89E3, 0x56DE, 0x584A, 0x58CA, 0x5EFB, 0x5FEB, 0x602A, 0x6094, 0x6062, 0x61D0, 0x6212, 0x62D0, 0x6539, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0X8A00 to 0X8AFF
	0x9B41, 0x6666, 0x68B0, 0x6D77, 0x7070, 0x754C, 0x7686, 0x7D75, 0x82A5, 0x87F9, 0x958B, 0x968E, 0x8C9D, 0x51F1, 0x52BE, 0x5916,
	0x54B3, 0x5BB3, 0x5D16, 0x6168, 0x6982, 0x6DAF, 0x788D, 0x84CB, 0x8857, 0x8A72, 0x93A7, 0x9AB8, 0x6D6C, 0x99A8, 0x86D9, 0x57A3,
	0x67FF, 0x86CE, 0x920E, 0x5283, 0x5687, 0x5404, 0x5ED3, 0x62E1, 0x64B9, 0x683C, 0x6838, 0x6BBB, 0x7372, 0x78BA, 0x7A6B, 0x899A,
	0x89D2, 0x8D6B, 0x8F03, 0x90ED, 0x95A3, 0x9694, 0x9769, 0x5B66, 0x5CB3, 0x697D, 0x984D, 0x984E, 0x639B, 0x7B20, 0x6A2B, 0xFFFF,
	0x6A7F, 0x68B6, 0x9C0D, 0x6F5F, 0x5272, 0x559D, 0x6070, 0x62EC, 0x6D3B, 0x6E07, 0x6ED1, 0x845B, 0x8910, 0x8F44, 0x4E14, 0x9C39,
	0x53F6, 0x691B, 0x6A3A, 0x9784, 0x682A, 0x515C, 0x7AC3, 0x84B2, 0x91DC, 0x938C, 0x565B, 0x9D28, 0x6822, 0x8305, 0x8431, 0x7CA5,
	0x5208, 0x82C5, 0x74E6, 0x4E7E, 0x4F83, 0x51A0, 0x5BD2, 0x520A, 0x52D8, 0x52E7, 0x5DFB, 0x559A, 0x582A, 0x59E6, 0x5B8C, 0x5B98,
	0x5BDB, 0x5E72, 0x5E79, 0x60A3, 0x611F, 0x6163, 0x61BE, 0x63DB, 0x6562, 0x67D1, 0x6853, 0x68FA, 0x6B3E, 0x6B53, 0x6C57, 0x6F22,
	0x6F97, 0x6F45, 0x74B0, 0x7518, 0x76E3, 0x770B, 0x7AFF, 0x7BA1, 0x7C21, 0x7DE9, 0x7F36, 0x7FF0, 0x809D, 0x8266, 0x839E, 0x89B3,
	0x8ACC, 0x8CAB, 0x9084, 0x9451, 0x9593, 0x9591, 0x95A2, 0x9665, 0x97D3, 0x9928, 0x8218, 0x4E38, 0x542B, 0x5CB8, 0x5DCC, 0x73A9,
	0x764C, 0x773C, 0x5CA9, 0x7FEB, 0x8D0B, 0x96C1, 0x9811, 0x9854, 0x9858, 0x4F01, 0x4F0E, 0x5371, 0x559C, 0x5668, 0x57FA, 0x5947,
	0x5B09, 0x5BC4, 0x5C90, 0x5E0C, 0x5E7E, 0x5FCC, 0x63EE, 0x673A, 0x65D7, 0x65E2, 0x671F, 0x68CB, 0x68C4, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0X8B00 to 0X8BFF
	0x6A5F, 0x5E30, 0x6BC5, 0x6C17, 0x6C7D, 0x757F, 0x7948, 0x5B63, 0x7A00, 0x7D00, 0x5FBD, 0x898F, 0x8A18, 0x8CB4, 0x8D77, 0x8ECC,
	0x8F1D, 0x98E2, 0x9A0E, 0x9B3C, 0x4E80, 0x507D, 0x5100, 0x5993, 0x5B9C, 0x622F, 0x6280, 0x64EC, 0x6B3A, 0x72A0, 0x7591, 0x7947,
	0x7FA9, 0x87FB, 0x8ABC, 0x8B70, 0x63AC, 0x83CA, 0x97A0, 0x5409, 0x5403, 0x55AB, 0x6854, 0x6A58, 0x8A70, 0x7827, 0x6775, 0x9ECD,
	0x5374, 0x5BA2, 0x811A, 0x8650, 0x9006, 0x4E18, 0x4E45, 0x4EC7, 0x4F11, 0x53CA, 0x5438, 0x5BAE, 0x5F13, 0x6025, 0x6551, 0xFFFF,
	0x673D, 0x6C42, 0x6C72, 0x6CE3, 0x7078, 0x7403, 0x7A76, 0x7AAE, 0x7B08, 0x7D1A, 0x7CFE, 0x7D66, 0x65E7, 0x725B, 0x53BB, 0x5C45,
	0x5DE8, 0x62D2, 0x62E0, 0x6319, 0x6E20, 0x865A, 0x8A31, 0x8DDD, 0x92F8, 0x6F01, 0x79A6, 0x9B5A, 0x4EA8, 0x4EAB, 0x4EAC, 0x4F9B,
	0x4FA0, 0x50D1, 0x5147, 0x7AF6, 0x5171, 0x51F6, 0x5354, 0x5321, 0x537F, 0x53EB, 0x55AC, 0x5883, 0x5CE1, 0x5F37, 0x5F4A, 0x602F,
	0x6050, 0x606D, 0x631F, 0x6559, 0x6A4B, 0x6CC1, 0x72C2, 0x72ED, 0x77EF, 0x80F8, 0x8105, 0x8208, 0x854E, 0x90F7, 0x93E1, 0x97FF,
	0x9957, 0x9A5A, 0x4EF0, 0x51DD, 0x5C2D, 0x6681, 0x696D, 0x5C40, 0x66F2, 0x6975, 0x7389, 0x6850, 0x7C81, 0x50C5, 0x52E4, 0x5747,
	0x5DFE, 0x9326, 0x65A4, 0x6B23, 0x6B3D, 0x7434, 0x7981, 0x79BD, 0x7B4B, 0x7DCA, 0x82B9, 0x83CC, 0x887F, 0x895F, 0x8B39, 0x8FD1,
	0x91D1, 0x541F, 0x9280, 0x4E5D, 0x5036, 0x53E5, 0x533A, 0x72D7, 0x7396, 0x77E9, 0x82E6, 0x8EAF, 0x99C6, 0x99C8, 0x99D2, 0x5177,
	0x611A, 0x865E, 0x55B0, 0x7A7A, 0x5076, 0x5BD3, 0x9047, 0x9685, 0x4E32, 0x6ADB, 0x91E7, 0x5C51, 0x5C48, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0X8C00 to 0X8CFF
	0x6398, 0x7A9F, 0x6C93, 0x9774, 0x8F61, 0x7AAA, 0x718A, 0x9688, 0x7C82, 0x6817, 0x7E70, 0x6851, 0x936C, 0x52F2, 0x541B, 0x85AB,
	0x8A13, 0x7FA4, 0x8ECD, 0x90E1, 0x5366, 0x8888, 0x7941, 0x4FC2, 0x50BE, 0x5211, 0x5144, 0x5553, 0x572D, 0x73EA, 0x578B, 0x5951,
	0x5F62, 0x5F84, 0x6075, 0x6176, 0x6167, 0x61A9, 0x63B2, 0x643A, 0x656C, 0x666F, 0x6842, 0x6E13, 0x7566, 0x7A3D, 0x7CFB, 0x7D4C,
	0x7D99, 0x7E4B, 0x7F6B, 0x830E, 0x834A, 0x86CD, 0x8A08, 0x8A63, 0x8B66, 0x8EFD, 0x981A, 0x9D8F, 0x82B8, 0x8FCE, 0x9BE8, 0xFFFF,
	0x5287, 0x621F, 0x6483, 0x6FC0, 0x9699, 0x6841, 0x5091, 0x6B20, 0x6C7A, 0x6F54, 0x7A74, 0x7D50, 0x8840, 0x8A23, 0x6708, 0x4EF6,
	0x5039, 0x5026, 0x5065, 0x517C, 0x5238, 0x5263, 0x55A7, 0x570F, 0x5805, 0x5ACC, 0x5EFA, 0x61B2, 0x61F8, 0x62F3, 0x6372, 0x691C,
	0x6A29, 0x727D, 0x72AC, 0x732E, 0x7814, 0x786F, 0x7D79, 0x770C, 0x80A9, 0x898B, 0x8B19, 0x8CE2, 0x8ED2, 0x9063, 0x9375, 0x967A,
	0x9855, 0x9A13, 0x9E78, 0x5143, 0x539F, 0x53B3, 0x5E7B, 0x5F26, 0x6E1B, 0x6E90, 0x7384, 0x73FE, 0x7D43, 0x8237, 0x8A00, 0x8AFA,
	0x9650, 0x4E4E, 0x500B, 0x53E4, 0x547C, 0x56FA, 0x59D1, 0x5B64, 0x5DF1, 0x5EAB, 0x5F27, 0x6238, 0x6545, 0x67AF, 0x6E56, 0x72D0,
	0x7CCA, 0x88B4, 0x80A1, 0x80E1, 0x83F0, 0x864E, 0x8A87, 0x8DE8, 0x9237, 0x96C7, 0x9867, 0x9F13, 0x4E94, 0x4E92, 0x4F0D, 0x5348,
	0x5449, 0x543E, 0x5A2F, 0x5F8C, 0x5FA1, 0x609F, 0x68A7, 0x6A8E, 0x745A, 0x7881, 0x8A9E, 0x8AA4, 0x8B77, 0x9190, 0x4E5E, 0x9BC9,
	0x4EA4, 0x4F7C, 0x4FAF, 0x5019, 0x5016, 0x5149, 0x516C, 0x529F, 0x52B9, 0x52FE, 0x539A, 0x53E3, 0x5411, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0X8D00 to 0X8DFF
	0x540E, 0x5589, 0x5751, 0x57A2, 0x597D, 0x5B54, 0x5B5D, 0x5B8F, 0x5DE5, 0x5DE7, 0x5DF7, 0x5E78, 0x5E83, 0x5E9A, 0x5EB7, 0x5F18,
	0x6052, 0x614C, 0x6297, 0x62D8, 0x63A7, 0x653B, 0x6602, 0x6643, 0x66F4, 0x676D, 0x6821, 0x6897, 0x69CB, 0x6C5F, 0x6D2A, 0x6D69,
	0x6E2F, 0x6E9D, 0x7532, 0x7687, 0x786C, 0x7A3F, 0x7CE0, 0x7D05, 0x7D18, 0x7D5E, 0x7DB1, 0x8015, 0x8003, 0x80AF, 0x80B1, 0x8154,
	0x818F, 0x822A, 0x8352, 0x884C, 0x8861, 0x8B1B, 0x8CA2, 0x8CFC, 0x90CA, 0x9175, 0x9271, 0x783F, 0x92FC, 0x95A4, 0x964D, 0xFFFF,
	0x9805, 0x9999, 0x9AD8, 0x9D3B, 0x525B, 0x52AB, 0x53F7, 0x5408, 0x58D5, 0x62F7, 0x6FE0, 0x8C6A, 0x8F5F, 0x9EB9, 0x514B, 0x523B,
	0x544A, 0x56FD, 0x7A40, 0x9177, 0x9D60, 0x9ED2, 0x7344, 0x6F09, 0x8170, 0x7511, 0x5FFD, 0x60DA, 0x9AA8, 0x72DB, 0x8FBC, 0x6B64,
	0x9803, 0x4ECA, 0x56F0, 0x5764, 0x58BE, 0x5A5A, 0x6068, 0x61C7, 0x660F, 0x6606, 0x6839, 0x68B1, 0x6DF7, 0x75D5, 0x7D3A, 0x826E,
	0x9B42, 0x4E9B, 0x4F50, 0x53C9, 0x5506, 0x5D6F, 0x5DE6, 0x5DEE, 0x67FB, 0x6C99, 0x7473, 0x7802, 0x8A50, 0x9396, 0x88DF, 0x5750,
	0x5EA7, 0x632B, 0x50B5, 0x50AC, 0x518D, 0x6700, 0x54C9, 0x585E, 0x59BB, 0x5BB0, 0x5F69, 0x624D, 0x63A1, 0x683D, 0x6B73, 0x6E08,
	0x707D, 0x91C7, 0x7280, 0x7815, 0x7826, 0x796D, 0x658E, 0x7D30, 0x83DC, 0x88C1, 0x8F09, 0x969B, 0x5264, 0x5728, 0x6750, 0x7F6A,
	0x8CA1, 0x51B4, 0x5742, 0x962A, 0x583A, 0x698A, 0x80B4, 0x54B2, 0x5D0E, 0x57FC, 0x7895, 0x9DFA, 0x4F5C, 0x524A, 0x548B, 0x643E,
	0x6628, 0x6714, 0x67F5, 0x7A84, 0x7B56, 0x7D22, 0x932F, 0x685C, 0x9BAD, 0x7B39, 0x5319, 0x518A, 0x5237, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0X8E00 to 0X8EFF
	0x5BDF, 0x62F6, 0x64AE, 0x64E6, 0x672D, 0x6BBA, 0x85A9, 0x96D1, 0x7690, 0x9BD6, 0x634C, 0x9306, 0x9BAB, 0x76BF, 0x6652, 0x4E09,
	0x5098, 0x53C2, 0x5C71, 0x60E8, 0x6492, 0x6563, 0x685F, 0x71E6, 0x73CA, 0x7523, 0x7B97, 0x7E82, 0x8695, 0x8B83, 0x8CDB, 0x9178,
	0x9910, 0x65AC, 0x66AB, 0x6B8B, 0x4ED5, 0x4ED4, 0x4F3A, 0x4F7F, 0x523A, 0x53F8, 0x53F2, 0x55E3, 0x56DB, 0x58EB, 0x59CB, 0x59C9,
	0x59FF, 0x5B50, 0x5C4D, 0x5E02, 0x5E2B, 0x5FD7, 0x601D, 0x6307, 0x652F, 0x5B5C, 0x65AF, 0x65BD, 0x65E8, 0x679D, 0x6B62, 0xFFFF,
	0x6B7B, 0x6C0F, 0x7345, 0x7949, 0x79C1, 0x7CF8, 0x7D19, 0x7D2B, 0x80A2, 0x8102, 0x81F3, 0x8996, 0x8A5E, 0x8A69, 0x8A66, 0x8A8C,
	0x8AEE, 0x8CC7, 0x8CDC, 0x96CC, 0x98FC, 0x6B6F, 0x4E8B, 0x4F3C, 0x4F8D, 0x5150, 0x5B57, 0x5BFA, 0x6148, 0x6301, 0x6642, 0x6B21,
	0x6ECB, 0x6CBB, 0x723E, 0x74BD, 0x75D4, 0x78C1, 0x793A, 0x800C, 0x8033, 0x81EA, 0x8494, 0x8F9E, 0x6C50, 0x9E7F, 0x5F0F, 0x8B58,
	0x9D2B, 0x7AFA, 0x8EF8, 0x5B8D, 0x96EB, 0x4E03, 0x53F1, 0x57F7, 0x5931, 0x5AC9, 0x5BA4, 0x6089, 0x6E7F, 0x6F06, 0x75BE, 0x8CEA,
	0x5B9F, 0x8500, 0x7BE0, 0x5072, 0x67F4, 0x829D, 0x5C61, 0x854A, 0x7E1E, 0x820E, 0x5199, 0x5C04, 0x6368, 0x8D66, 0x659C, 0x716E,
	0x793E, 0x7D17, 0x8005, 0x8B1D, 0x8ECA, 0x906E, 0x86C7, 0x90AA, 0x501F, 0x52FA, 0x5C3A, 0x6753, 0x707C, 0x7235, 0x914C, 0x91C8,
	0x932B, 0x82E5, 0x5BC2, 0x5F31, 0x60F9, 0x4E3B, 0x53D6, 0x5B88, 0x624B, 0x6731, 0x6B8A, 0x72E9, 0x73E0, 0x7A2E, 0x816B, 0x8DA3,
	0x9152, 0x9996, 0x5112, 0x53D7, 0x546A, 0x5BFF, 0x6388, 0x6A39, 0x7DAC, 0x9700, 0x56DA, 0x53CE, 0x5468, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0X8F00 to 0X8FFF
	0x5B97, 0x5C31, 0x5DDE, 0x4FEE, 0x6101, 0x62FE, 0x6D32, 0x79C0, 0x79CB, 0x7D42, 0x7E4D, 0x7FD2, 0x81ED, 0x821F, 0x8490, 0x8846,
	0x8972, 0x8B90, 0x8E74, 0x8F2F, 0x9031, 0x914B, 0x916C, 0x96C6, 0x919C, 0x4EC0, 0x4F4F, 0x5145, 0x5341, 0x5F93, 0x620E, 0x67D4,
	0x6C41, 0x6E0B, 0x7363, 0x7E26, 0x91CD, 0x9283, 0x53D4, 0x5919, 0x5BBF, 0x6DD1, 0x795D, 0x7E2E, 0x7C9B, 0x587E, 0x719F, 0x51FA,
	0x8853, 0x8FF0, 0x4FCA, 0x5CFB, 0x6625, 0x77AC, 0x7AE3, 0x821C, 0x99FF, 0x51C6, 0x5FAA, 0x65EC, 0x696F, 0x6B89, 0x6DF3, 0xFFFF,
	0x6E96, 0x6F64, 0x76FE, 0x7D14, 0x5DE1, 0x9075, 0x9187, 0x9806, 0x51E6, 0x521D, 0x6240, 0x6691, 0x66D9, 0x6E1A, 0x5EB6, 0x7DD2,
	0x7F72, 0x66F8, 0x85AF, 0x85F7, 0x8AF8, 0x52A9, 0x53D9, 0x5973, 0x5E8F, 0x5F90, 0x6055, 0x92E4, 0x9664, 0x50B7, 0x511F, 0x52DD,
	0x5320, 0x5347, 0x53EC, 0x54E8, 0x5546, 0x5531, 0x5617, 0x5968, 0x59BE, 0x5A3C, 0x5BB5, 0x5C06, 0x5C0F, 0x5C11, 0x5C1A, 0x5E84,
	0x5E8A, 0x5EE0, 0x5F70, 0x627F, 0x6284, 0x62DB, 0x638C, 0x6377, 0x6607, 0x660C, 0x662D, 0x6676, 0x677E, 0x68A2, 0x6A1F, 0x6A35,
	0x6CBC, 0x6D88, 0x6E09, 0x6E58, 0x713C, 0x7126, 0x7167, 0x75C7, 0x7701, 0x785D, 0x7901, 0x7965, 0x79F0, 0x7AE0, 0x7B11, 0x7CA7,
	0x7D39, 0x8096, 0x83D6, 0x848B, 0x8549, 0x885D, 0x88F3, 0x8A1F, 0x8A3C, 0x8A54, 0x8A73, 0x8C61, 0x8CDE, 0x91A4, 0x9266, 0x937E,
	0x9418, 0x969C, 0x9798, 0x4E0A, 0x4E08, 0x4E1E, 0x4E57, 0x5197, 0x5270, 0x57CE, 0x5834, 0x58CC, 0x5B22, 0x5E38, 0x60C5, 0x64FE,
	0x6761, 0x6756, 0x6D44, 0x72B6, 0x7573, 0x7A63, 0x84B8, 0x8B72, 0x91B8, 0x9320, 0x5631, 0x57F4, 0x98FE, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0X9000 to 0X90FF
	0x62ED, 0x690D, 0x6B96, 0x71ED, 0x7E54, 0x8077, 0x8272, 0x89E6, 0x98DF, 0x8755, 0x8FB1, 0x5C3B, 0x4F38, 0x4FE1, 0x4FB5, 0x5507,
	0x5A20, 0x5BDD, 0x5BE9, 0x5FC3, 0x614E, 0x632F, 0x65B0, 0x664B, 0x68EE, 0x699B, 0x6D78, 0x6DF1, 0x7533, 0x75B9, 0x771F, 0x795E,
	0x79E6, 0x7D33, 0x81E3, 0x82AF, 0x85AA, 0x89AA, 0x8A3A, 0x8EAB, 0x8F9B, 0x9032, 0x91DD, 0x9707, 0x4EBA, 0x4EC1, 0x5203, 0x5875,
	0x58EC, 0x5C0B, 0x751A, 0x5C3D, 0x814E, 0x8A0A, 0x8FC5, 0x9663, 0x976D, 0x7B25, 0x8ACF, 0x9808, 0x9162, 0x56F3, 0x53A8, 0xFFFF,
	0x9017, 0x5439, 0x5782, 0x5E25, 0x63A8, 0x6C34, 0x708A, 0x7761, 0x7C8B, 0x7FE0, 0x8870, 0x9042, 0x9154, 0x9310, 0x9318, 0x968F,
	0x745E, 0x9AC4, 0x5D07, 0x5D69, 0x6570, 0x67A2, 0x8DA8, 0x96DB, 0x636E, 0x6749, 0x6919, 0x83C5, 0x9817, 0x96C0, 0x88FE, 0x6F84,
	0x647A, 0x5BF8, 0x4E16, 0x702C, 0x755D, 0x662F, 0x51C4, 0x5236, 0x52E2, 0x59D3, 0x5F81, 0x6027, 0x6210, 0x653F, 0x6574, 0x661F,
	0x6674, 0x68F2, 0x6816, 0x6B63, 0x6E05, 0x7272, 0x751F, 0x76DB, 0x7CBE, 0x8056, 0x58F0, 0x88FD, 0x897F, 0x8AA0, 0x8A93, 0x8ACB,
	0x901D, 0x9192, 0x9752, 0x9759, 0x6589, 0x7A0E, 0x8106, 0x96BB, 0x5E2D, 0x60DC, 0x621A, 0x65A5, 0x6614, 0x6790, 0x77F3, 0x7A4D,
	0x7C4D, 0x7E3E, 0x810A, 0x8CAC, 0x8D64, 0x8DE1, 0x8E5F, 0x78A9, 0x5207, 0x62D9, 0x63A5, 0x6442, 0x6298, 0x8A2D, 0x7A83, 0x7BC0,
	0x8AAC, 0x96EA, 0x7D76, 0x820C, 0x8749, 0x4ED9, 0x5148, 0x5343, 0x5360, 0x5BA3, 0x5C02, 0x5C16, 0x5DDD, 0x6226, 0x6247, 0x64B0,
	0x6813, 0x6834, 0x6CC9, 0x6D45, 0x6D17, 0x67D3, 0x6F5C, 0x714E, 0x717D, 0x65CB, 0x7A7F, 0x7BAD, 0x7DDA, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0X9100 to 0X91FF
	0x7E4A, 0x7FA8, 0x817A, 0x821B, 0x8239, 0x85A6, 0x8A6E, 0x8CCE, 0x8DF5, 0x9078, 0x9077, 0x92AD, 0x9291, 0x9583, 0x9BAE, 0x524D,
	0x5584, 0x6F38, 0x7136, 0x5168, 0x7985, 0x7E55, 0x81B3, 0x7CCE, 0x564C, 0x5851, 0x5CA8, 0x63AA, 0x66FE, 0x66FD, 0x695A, 0x72D9,
	0x758F, 0x758E, 0x790E, 0x7956, 0x79DF, 0x7C97, 0x7D20, 0x7D44, 0x8607, 0x8A34, 0x963B, 0x9061, 0x9F20, 0x50E7, 0x5275, 0x53CC,
	0x53E2, 0x5009, 0x55AA, 0x58EE, 0x594F, 0x723D, 0x5B8B, 0x5C64, 0x531D, 0x60E3, 0x60F3, 0x635C, 0x6383, 0x633F, 0x63BB, 0xFFFF,
	0x64CD, 0x65E9, 0x66F9, 0x5DE3, 0x69CD, 0x69FD, 0x6F15, 0x71E5, 0x4E89, 0x75E9, 0x76F8, 0x7A93, 0x7CDF, 0x7DCF, 0x7D9C, 0x8061,
	0x8349, 0x8358, 0x846C, 0x84BC, 0x85FB, 0x88C5, 0x8D70, 0x9001, 0x906D, 0x9397, 0x971C, 0x9A12, 0x50CF, 0x5897, 0x618E, 0x81D3,
	0x8535, 0x8D08, 0x9020, 0x4FC3, 0x5074, 0x5247, 0x5373, 0x606F, 0x6349, 0x675F, 0x6E2C, 0x8DB3, 0x901F, 0x4FD7, 0x5C5E, 0x8CCA,
	0x65CF, 0x7D9A, 0x5352, 0x8896, 0x5176, 0x63C3, 0x5B58, 0x5B6B, 0x5C0A, 0x640D, 0x6751, 0x905C, 0x4ED6, 0x591A, 0x592A, 0x6C70,
	0x8A51, 0x553E, 0x5815, 0x59A5, 0x60F0, 0x6253, 0x67C1, 0x8235, 0x6955, 0x9640, 0x99C4, 0x9A28, 0x4F53, 0x5806, 0x5BFE, 0x8010,
	0x5CB1, 0x5E2F, 0x5F85, 0x6020, 0x614B, 0x6234, 0x66FF, 0x6CF0, 0x6EDE, 0x80CE, 0x817F, 0x82D4, 0x888B, 0x8CB8, 0x9000, 0x902E,
	0x968A, 0x9EDB, 0x9BDB, 0x4EE3, 0x53F0, 0x5927, 0x7B2C, 0x918D, 0x984C, 0x9DF9, 0x6EDD, 0x7027, 0x5353, 0x5544, 0x5B85, 0x6258,
	0x629E, 0x62D3, 0x6CA2, 0x6FEF, 0x7422, 0x8A17, 0x9438, 0x6FC1, 0x8AFE, 0x8338, 0x51E7, 0x86F8, 0x53EA, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0X9200 to 0X92FF
	0x53E9, 0x4F46, 0x9054, 0x8FB0, 0x596A, 0x8131, 0x5DFD, 0x7AEA, 0x8FBF, 0x68DA, 0x8C37, 0x72F8, 0x9C48, 0x6A3D, 0x8AB0, 0x4E39,
	0x5358, 0x5606, 0x5766, 0x62C5, 0x63A2, 0x65E6, 0x6B4E, 0x6DE1, 0x6E5B, 0x70AD, 0x77ED, 0x7AEF, 0x7BAA, 0x7DBB, 0x803D, 0x80C6,
	0x86CB, 0x8A95, 0x935B, 0x56E3, 0x58C7, 0x5F3E, 0x65AD, 0x6696, 0x6A80, 0x6BB5, 0x7537, 0x8AC7, 0x5024, 0x77E5, 0x5730, 0x5F1B,
	0x6065, 0x667A, 0x6C60, 0x75F4, 0x7A1A, 0x7F6E, 0x81F4, 0x8718, 0x9045, 0x99B3, 0x7BC9, 0x755C, 0x7AF9, 0x7B51, 0x84C4, 0xFFFF,
	0x9010, 0x79E9, 0x7A92, 0x8336, 0x5AE1, 0x7740, 0x4E2D, 0x4EF2, 0x5B99, 0x5FE0, 0x62BD, 0x663C, 0x67F1, 0x6CE8, 0x866B, 0x8877,
	0x8A3B, 0x914E, 0x92F3, 0x99D0, 0x6A17, 0x7026, 0x732A, 0x82E7, 0x8457, 0x8CAF, 0x4E01, 0x5146, 0x51CB, 0x558B, 0x5BF5, 0x5E16,
	0x5E33, 0x5E81, 0x5F14, 0x5F35, 0x5F6B, 0x5FB4, 0x61F2, 0x6311, 0x66A2, 0x671D, 0x6F6E, 0x7252, 0x753A, 0x773A, 0x8074, 0x8139,
	0x8178, 0x8776, 0x8ABF, 0x8ADC, 0x8D85, 0x8DF3, 0x929A, 0x9577, 0x9802, 0x9CE5, 0x52C5, 0x6357, 0x76F4, 0x6715, 0x6C88, 0x73CD,
	0x8CC3, 0x93AE, 0x9673, 0x6D25, 0x589C, 0x690E, 0x69CC, 0x8FFD, 0x939A, 0x75DB, 0x901A, 0x585A, 0x6802, 0x63B4, 0x69FB, 0x4F43,
	0x6F2C, 0x67D8, 0x8FBB, 0x8526, 0x7DB4, 0x9354, 0x693F, 0x6F70, 0x576A, 0x58F7, 0x5B2C, 0x7D2C, 0x722A, 0x540A, 0x91E3, 0x9DB4,
	0x4EAD, 0x4F4E, 0x505C, 0x5075, 0x5243, 0x8C9E, 0x5448, 0x5824, 0x5B9A, 0x5E1D, 0x5E95, 0x5EAD, 0x5EF7, 0x5F1F, 0x608C, 0x62B5,
	0x633A, 0x63D0, 0x68AF, 0x6C40, 0x7887, 0x798E, 0x7A0B, 0x7DE0, 0x8247, 0x8A02, 0x8AE6, 0x8E44, 0x9013, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0X9300 to 0X93FF
	0x90B8, 0x912D, 0x91D8, 0x9F0E, 0x6CE5, 0x6458, 0x64E2, 0x6575, 0x6EF4, 0x7684, 0x7B1B, 0x9069, 0x93D1, 0x6EBA, 0x54F2, 0x5FB9,
	0x64A4, 0x8F4D, 0x8FED, 0x9244, 0x5178, 0x586B, 0x5929, 0x5C55, 0x5E97, 0x6DFB, 0x7E8F, 0x751C, 0x8CBC, 0x8EE2, 0x985B, 0x70B9,
	0x4F1D, 0x6BBF, 0x6FB1, 0x7530, 0x96FB, 0x514E, 0x5410, 0x5835, 0x5857, 0x59AC, 0x5C60, 0x5F92, 0x6597, 0x675C, 0x6E21, 0x767B,
	0x83DF, 0x8CED, 0x9014, 0x90FD, 0x934D, 0x7825, 0x783A, 0x52AA, 0x5EA6, 0x571F, 0x5974, 0x6012, 0x5012, 0x515A, 0x51AC, 0xFFFF,
	0x51CD, 0x5200, 0x5510, 0x5854, 0x5858, 0x5957, 0x5B95, 0x5CF6, 0x5D8B, 0x60BC, 0x6295, 0x642D, 0x6771, 0x6843, 0x68BC, 0x68DF,
	0x76D7, 0x6DD8, 0x6E6F, 0x6D9B, 0x706F, 0x71C8, 0x5F53, 0x75D8, 0x7977, 0x7B49, 0x7B54, 0x7B52, 0x7CD6, 0x7D71, 0x5230, 0x8463,
	0x8569, 0x85E4, 0x8A0E, 0x8B04, 0x8C46, 0x8E0F, 0x9003, 0x900F, 0x9419, 0x9676, 0x982D, 0x9A30, 0x95D8, 0x50CD, 0x52D5, 0x540C,
	0x5802, 0x5C0E, 0x61A7, 0x649E, 0x6D1E, 0x77B3, 0x7AE5, 0x80F4, 0x8404, 0x9053, 0x9285, 0x5CE0, 0x9D07, 0x533F, 0x5F97, 0x5FB3,
	0x6D9C, 0x7279, 0x7763, 0x79BF, 0x7BE4, 0x6BD2, 0x72EC, 0x8AAD, 0x6803, 0x6A61, 0x51F8, 0x7A81, 0x6934, 0x5C4A, 0x9CF6, 0x82EB,
	0x5BC5, 0x9149, 0x701E, 0x5678, 0x5C6F, 0x60C7, 0x6566, 0x6C8C, 0x8C5A, 0x9041, 0x9813, 0x5451, 0x66C7, 0x920D, 0x5948, 0x90A3,
	0x5185, 0x4E4D, 0x51EA, 0x8599, 0x8B0E, 0x7058, 0x637A, 0x934B, 0x6962, 0x99B4, 0x7E04, 0x7577, 0x5357, 0x6960, 0x8EDF, 0x96E3,
	0x6C5D, 0x4E8C, 0x5C3C, 0x5F10, 0x8FE9, 0x5302, 0x8CD1, 0x8089, 0x8679, 0x5EFF, 0x65E5, 0x4E73, 0x5165, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0X9400 to 0X94FF
	0x5982, 0x5C3F, 0x97EE, 0x4EFB, 0x598A, 0x5FCD, 0x8A8D, 0x6FE1, 0x79B0, 0x7962, 0x5BE7, 0x8471, 0x732B, 0x71B1, 0x5E74, 0x5FF5,
	0x637B, 0x649A, 0x71C3, 0x7C98, 0x4E43, 0x5EFC, 0x4E4B, 0x57DC, 0x56A2, 0x60A9, 0x6FC3, 0x7D0D, 0x80FD, 0x8133, 0x81BF, 0x8FB2,
	0x8997, 0x86A4, 0x5DF4, 0x628A, 0x64AD, 0x8987, 0x6777, 0x6CE2, 0x6D3E, 0x7436, 0x7834, 0x5A46, 0x7F75, 0x82AD, 0x99AC, 0x4FF3,
	0x5EC3, 0x62DD, 0x6392, 0x6557, 0x676F, 0x76C3, 0x724C, 0x80CC, 0x80BA, 0x8F29, 0x914D, 0x500D, 0x57F9, 0x5A92, 0x6885, 0xFFFF,
	0x6973, 0x7164, 0x72FD, 0x8CB7, 0x58F2, 0x8CE0, 0x966A, 0x9019, 0x877F, 0x79E4, 0x77E7, 0x8429, 0x4F2F, 0x5265, 0x535A, 0x62CD,
	0x67CF, 0x6CCA, 0x767D, 0x7B94, 0x7C95, 0x8236, 0x8584, 0x8FEB, 0x66DD, 0x6F20, 0x7206, 0x7E1B, 0x83AB, 0x99C1, 0x9EA6, 0x51FD,
	0x7BB1, 0x7872, 0x7BB8, 0x8087, 0x7B48, 0x6AE8, 0x5E61, 0x808C, 0x7551, 0x7560, 0x516B, 0x9262, 0x6E8C, 0x767A, 0x9197, 0x9AEA,
	0x4F10, 0x7F70, 0x629C, 0x7B4F, 0x95A5, 0x9CE9, 0x567A, 0x5859, 0x86E4, 0x96BC, 0x4F34, 0x5224, 0x534A, 0x53CD, 0x53DB, 0x5E06,
	0x642C, 0x6591, 0x677F, 0x6C3E, 0x6C4E, 0x7248, 0x72AF, 0x73ED, 0x7554, 0x7E41, 0x822C, 0x85E9, 0x8CA9, 0x7BC4, 0x91C6, 0x7169,
	0x9812, 0x98EF, 0x633D, 0x6669, 0x756A, 0x76E4, 0x78D0, 0x8543, 0x86EE, 0x532A, 0x5351, 0x5426, 0x5983, 0x5E87, 0x5F7C, 0x60B2,
	0x6249, 0x6279, 0x62AB, 0x6590, 0x6BD4, 0x6CCC, 0x75B2, 0x76AE, 0x7891, 0x79D8, 0x7DCB, 0x7F77, 0x80A5, 0x88AB, 0x8AB9, 0x8CBB,
	0x907F, 0x975E, 0x98DB, 0x6A0B, 0x7C38, 0x5099, 0x5C3E, 0x5FAE, 0x6787, 0x6BD8, 0x7435, 0x7709, 0x7F8E, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0X9500 to 0X95FF
	0x9F3B, 0x67CA, 0x7A17, 0x5339, 0x758B, 0x9AED, 0x5F66, 0x819D, 0x83F1, 0x8098, 0x5F3C, 0x5FC5, 0x7562, 0x7B46, 0x903C, 0x6867,
	0x59EB, 0x5A9B, 0x7D10, 0x767E, 0x8B2C, 0x4FF5, 0x5F6A, 0x6A19, 0x6C37, 0x6F02, 0x74E2, 0x7968, 0x8868, 0x8A55, 0x8C79, 0x5EDF,
	0x63CF, 0x75C5, 0x79D2, 0x82D7, 0x9328, 0x92F2, 0x849C, 0x86ED, 0x9C2D, 0x54C1, 0x5F6C, 0x658C, 0x6D5C, 0x7015, 0x8CA7, 0x8CD3,
	0x983B, 0x654F, 0x74F6, 0x4E0D, 0x4ED8, 0x57E0, 0x592B, 0x5A66, 0x5BCC, 0x51A8, 0x5E03, 0x5E9C, 0x6016, 0x6276, 0x6577, 0xFFFF,
	0x65A7, 0x666E, 0x6D6E, 0x7236, 0x7B26, 0x8150, 0x819A, 0x8299, 0x8B5C, 0x8CA0, 0x8CE6, 0x8D74, 0x961C, 0x9644, 0x4FAE, 0x64AB,
	0x6B66, 0x821E, 0x8461, 0x856A, 0x90E8, 0x5C01, 0x6953, 0x98A8, 0x847A, 0x8557, 0x4F0F, 0x526F, 0x5FA9, 0x5E45, 0x670D, 0x798F,
	0x8179, 0x8907, 0x8986, 0x6DF5, 0x5F17, 0x6255, 0x6CB8, 0x4ECF, 0x7269, 0x9B92, 0x5206, 0x543B, 0x5674, 0x58B3, 0x61A4, 0x626E,
	0x711A, 0x596E, 0x7C89, 0x7CDE, 0x7D1B, 0x96F0, 0x6587, 0x805E, 0x4E19, 0x4F75, 0x5175, 0x5840, 0x5E63, 0x5E73, 0x5F0A, 0x67C4,
	0x4E26, 0x853D, 0x9589, 0x965B, 0x7C73, 0x9801, 0x50FB, 0x58C1, 0x7656, 0x78A7, 0x5225, 0x77A5, 0x8511, 0x7B86, 0x504F, 0x5909,
	0x7247, 0x7BC7, 0x7DE8, 0x8FBA, 0x8FD4, 0x904D, 0x4FBF, 0x52C9, 0x5A29, 0x5F01, 0x97AD, 0x4FDD, 0x8217, 0x92EA, 0x5703, 0x6355,
	0x6B69, 0x752B, 0x88DC, 0x8F14, 0x7A42, 0x52DF, 0x5893, 0x6155, 0x620A, 0x66AE, 0x6BCD, 0x7C3F, 0x83E9, 0x5023, 0x4FF8, 0x5305,
	0x5446, 0x5831, 0x5949, 0x5B9D, 0x5CF0, 0x5CEF, 0x5D29, 0x5E96, 0x62B1, 0x6367, 0x653E, 0x65B9, 0x670B, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0X9600 to 0X96FF
	0x6CD5, 0x6CE1, 0x70F9, 0x7832, 0x7E2B, 0x80DE, 0x82B3, 0x840C, 0x84EC, 0x8702, 0x8912, 0x8A2A, 0x8C4A, 0x90A6, 0x92D2, 0x98FD,
	0x9CF3, 0x9D6C, 0x4E4F, 0x4EA1, 0x508D, 0x5256, 0x574A, 0x59A8, 0x5E3D, 0x5FD8, 0x5FD9, 0x623F, 0x66B4, 0x671B, 0x67D0, 0x68D2,
	0x5192, 0x7D21, 0x80AA, 0x81A8, 0x8B00, 0x8C8C, 0x8CBF, 0x927E, 0x9632, 0x5420, 0x982C, 0x5317, 0x50D5, 0x535C, 0x58A8, 0x64B2,
	0x6734, 0x7267, 0x7766, 0x7A46, 0x91E6, 0x52C3, 0x6CA1, 0x6B86, 0x5800, 0x5E4C, 0x5954, 0x672C, 0x7FFB, 0x51E1, 0x76C6, 0xFFFF,
	0x6469, 0x78E8, 0x9B54, 0x9EBB, 0x57CB, 0x59B9, 0x6627, 0x679A, 0x6BCE, 0x54E9, 0x69D9, 0x5E55, 0x819C, 0x6795, 0x9BAA, 0x67FE,
	0x9C52, 0x685D, 0x4EA6, 0x4FE3, 0x53C8, 0x62B9, 0x672B, 0x6CAB, 0x8FC4, 0x4FAD, 0x7E6D, 0x9EBF, 0x4E07, 0x6162, 0x6E80, 0x6F2B,
	0x8513, 0x5473, 0x672A, 0x9B45, 0x5DF3, 0x7B95, 0x5CAC, 0x5BC6, 0x871C, 0x6E4A, 0x84D1, 0x7A14, 0x8108, 0x5999, 0x7C8D, 0x6C11,
	0x7720, 0x52D9, 0x5922, 0x7121, 0x725F, 0x77DB, 0x9727, 0x9D61, 0x690B, 0x5A7F, 0x5A18, 0x51A5, 0x540D, 0x547D, 0x660E, 0x76DF,
	0x8FF7, 0x9298, 0x9CF4, 0x59EA, 0x725D, 0x6EC5, 0x514D, 0x68C9, 0x7DBF, 0x7DEC, 0x9762, 0x9EBA, 0x6478, 0x6A21, 0x8302, 0x5984,
	0x5B5F, 0x6BDB, 0x731B, 0x76F2, 0x7DB2, 0x8017, 0x8499, 0x5132, 0x6728, 0x9ED9, 0x76EE, 0x6762, 0x52FF, 0x9905, 0x5C24, 0x623B,
	0x7C7E, 0x8CB0, 0x554F, 0x60B6, 0x7D0B, 0x9580, 0x5301, 0x4E5F, 0x51B6, 0x591C, 0x723A, 0x8036, 0x91CE, 0x5F25, 0x77E2, 0x5384,
	0x5F79, 0x7D04, 0x85AC, 0x8A33, 0x8E8D, 0x9756, 0x67F3, 0x85AE, 0x9453, 0x6109, 0x6108, 0x6CB9, 0x7652, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0X9700 to 0X97FF
	0x8AED, 0x8F38, 0x552F, 0x4F51, 0x512A, 0x52C7, 0x53CB, 0x5BA5, 0x5E7D, 0x60A0, 0x6182, 0x63D6, 0x6709, 0x67DA, 0x6E67, 0x6D8C,
	0x7336, 0x7337, 0x7531, 0x7950, 0x88D5, 0x8A98, 0x904A, 0x9091, 0x90F5, 0x96C4, 0x878D, 0x5915, 0x4E88, 0x4F59, 0x4E0E, 0x8A89,
	0x8F3F, 0x9810, 0x50AD, 0x5E7C, 0x5996, 0x5BB9, 0x5EB8, 0x63DA, 0x63FA, 0x64C1, 0x66DC, 0x694A, 0x69D8, 0x6D0B, 0x6EB6, 0x7194,
	0x7528, 0x7AAF, 0x7F8A, 0x8000, 0x8449, 0x84C9, 0x8981, 0x8B21, 0x8E0A, 0x9065, 0x967D, 0x990A, 0x617E, 0x6291, 0x6B32, 0xFFFF,
	0x6C83, 0x6D74, 0x7FCC, 0x7FFC, 0x6DC0, 0x7F85, 0x87BA, 0x88F8, 0x6765, 0x83B1, 0x983C, 0x96F7, 0x6D1B, 0x7D61, 0x843D, 0x916A,
	0x4E71, 0x5375, 0x5D50, 0x6B04, 0x6FEB, 0x85CD, 0x862D, 0x89A7, 0x5229, 0x540F, 0x5C65, 0x674E, 0x68A8, 0x7406, 0x7483, 0x75E2,
	0x88CF, 0x88E1, 0x91CC, 0x96E2, 0x9678, 0x5F8B, 0x7387, 0x7ACB, 0x844E, 0x63A0, 0x7565, 0x5289, 0x6D41, 0x6E9C, 0x7409, 0x7559,
	0x786B, 0x7C92, 0x9686, 0x7ADC, 0x9F8D, 0x4FB6, 0x616E, 0x65C5, 0x865C, 0x4E86, 0x4EAE, 0x50DA, 0x4E21, 0x51CC, 0x5BEE, 0x6599,
	0x6881, 0x6DBC, 0x731F, 0x7642, 0x77AD, 0x7A1C, 0x7CE7, 0x826F, 0x8AD2, 0x907C, 0x91CF, 0x9675, 0x9818, 0x529B, 0x7DD1, 0x502B,
	0x5398, 0x6797, 0x6DCB, 0x71D0, 0x7433, 0x81E8, 0x8F2A, 0x96A3, 0x9C57, 0x9E9F, 0x7460, 0x5841, 0x6D99, 0x7D2F, 0x985E, 0x4EE4,
	0x4F36, 0x4F8B, 0x51B7, 0x52B1, 0x5DBA, 0x601C, 0x73B2, 0x793C, 0x82D3, 0x9234, 0x96B7, 0x96F6, 0x970A, 0x9E97, 0x9F62, 0x66A6,
	0x6B74, 0x5217, 0x52A3, 0x70C8, 0x88C2, 0x5EC9, 0x604B, 0x6190, 0x6F23, 0x7149, 0x7C3E, 0x7DF4, 0x806F, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0X9800 to 0X98FF
	0x84EE, 0x9023, 0x932C, 0x5442, 0x9B6F, 0x6AD3, 0x7089, 0x8CC2, 0x8DEF, 0x9732, 0x52B4, 0x5A41, 0x5ECA, 0x5F04, 0x6717, 0x697C,
	0x6994, 0x6D6A, 0x6F0F, 0x7262, 0x72FC, 0x7BED, 0x8001, 0x807E, 0x874B, 0x90CE, 0x516D, 0x9E93, 0x7984, 0x808B, 0x9332, 0x8AD6,
	0x502D, 0x548C, 0x8A71, 0x6B6A, 0x8CC4, 0x8107, 0x60D1, 0x67A0, 0x9DF2, 0x4E99, 0x4E98, 0x9C10, 0x8A6B, 0x85C1, 0x8568, 0x6900,
	0x6E7E, 0x7897, 0x8155, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x5F0C,
	0x4E10, 0x4E15, 0x4E2A, 0x4E31, 0x4E36, 0x4E3C, 0x4E3F, 0x4E42, 0x4E56, 0x4E58, 0x4E82, 0x4E85, 0x8C6B, 0x4E8A, 0x8212, 0x5F0D,
	0x4E8E, 0x4E9E, 0x4E9F, 0x4EA0, 0x4EA2, 0x4EB0, 0x4EB3, 0x4EB6, 0x4ECE, 0x4ECD, 0x4EC4, 0x4EC6, 0x4EC2, 0x4ED7, 0x4EDE, 0x4EED,
	0x4EDF, 0x4EF7, 0x4F09, 0x4F5A, 0x4F30, 0x4F5B, 0x4F5D, 0x4F57, 0x4F47, 0x4F76, 0x4F88, 0x4F8F, 0x4F98, 0x4F7B, 0x4F69, 0x4F70,
	0x4F91, 0x4F6F, 0x4F86, 0x4F96, 0x5118, 0x4FD4, 0x4FDF, 0x4FCE, 0x4FD8, 0x4FDB, 0x4FD1, 0x4FDA, 0x4FD0, 0x4FE4, 0x4FE5, 0x501A,
	0x5028, 0x5014, 0x502A, 0x5025, 0x5005, 0x4F1C, 0x4FF6, 0x5021, 0x5029, 0x502C, 0x4FFE, 0x4FEF, 0x5011, 0x5006, 0x5043, 0x5047,
	0x6703, 0x5055, 0x5050, 0x5048, 0x505A, 0x5056, 0x506C, 0x5078, 0x5080, 0x509A, 0x5085, 0x50B4, 0x50B2, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0X9900 to 0X99FF
	0x50C9, 0x50CA, 0x50B3, 0x50C2, 0x50D6, 0x50DE, 0x50E5, 0x50ED, 0x50E3, 0x50EE, 0x50F9, 0x50F5, 0x5109, 0x5101, 0x5102, 0x5116,
	0x5115, 0x5114, 0x511A, 0x5121, 0x513A, 0x5137, 0x513C, 0x513B, 0x513F, 0x5140, 0x5152, 0x514C, 0x5154, 0x5162, 0x7AF8, 0x5169,
	0x516A, 0x516E, 0x5180, 0x5182, 0x56D8, 0x518C, 0x5189, 0x518F, 0x5191, 0x5193, 0x5195, 0x5196, 0x51A4, 0x51A6, 0x51A2, 0x51A9,
	0x51AA, 0x51AB, 0x51B3, 0x51B1, 0x51B2, 0x51B0, 0x51B5, 0x51BD, 0x51C5, 0x51C9, 0x51DB, 0x51E0, 0x8655, 0x51E9, 0x51ED, 0xFFFF,
	0x51F0, 0x51F5, 0x51FE, 0x5204, 0x520B, 0x5214, 0x520E, 0x5227, 0x522A, 0x522E, 0x5233, 0x5239, 0x524F, 0x5244, 0x524B, 0x524C,
	0x525E, 0x5254, 0x526A, 0x5274, 0x5269, 0x5273, 0x527F, 0x527D, 0x528D, 0x5294, 0x5292, 0x5271, 0x5288, 0x5291, 0x8FA8, 0x8FA7,
	0x52AC, 0x52AD, 0x52BC, 0x52B5, 0x52C1, 0x52CD, 0x52D7, 0x52DE, 0x52E3, 0x52E6, 0x98ED, 0x52E0, 0x52F3, 0x52F5, 0x52F8, 0x52F9,
	0x5306, 0x5308, 0x7538, 0x530D, 0x5310, 0x530F, 0x5315, 0x531A, 0x5323, 0x532F, 0x5331, 0x5333, 0x5338, 0x5340, 0x5346, 0x5345,
	0x4E17, 0x5349, 0x534D, 0x51D6, 0x535E, 0x5369, 0x536E, 0x5918, 0x537B, 0x5377, 0x5382, 0x5396, 0x53A0, 0x53A6, 0x53A5, 0x53AE,
	0x53B0, 0x53B6, 0x53C3, 0x7C12, 0x96D9, 0x53DF, 0x66FC, 0x71EE, 0x53EE, 0x53E8, 0x53ED, 0x53FA, 0x5401, 0x543D, 0x5440, 0x542C,
	0x542D, 0x543C, 0x542E, 0x5436, 0x5429, 0x541D, 0x544E, 0x548F, 0x5475, 0x548E, 0x545F, 0x5471, 0x5477, 0x5470, 0x5492, 0x547B,
	0x5480, 0x5476, 0x5484, 0x5490, 0x5486, 0x54C7, 0x54A2, 0x54B8, 0x54A5, 0x54AC, 0x54C4, 0x54C8, 0x54A8, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0X9A00 to 0X9AFF
	0x54AB, 0x54C2, 0x54A4, 0x54BE, 0x54BC, 0x54D8, 0x54E5, 0x54E6, 0x550F, 0x5514, 0x54FD, 0x54EE, 0x54ED, 0x54FA, 0x54E2, 0x5539,
	0x5540, 0x5563, 0x554C, 0x552E, 0x555C, 0x5545, 0x5556, 0x5557, 0x5538, 0x5533, 0x555D, 0x5599, 0x5580, 0x54AF, 0x558A, 0x559F,
	0x557B, 0x557E, 0x5598, 0x559E, 0x55AE, 0x557C, 0x5583, 0x55A9, 0x5587, 0x55A8, 0x55DA, 0x55C5, 0x55DF, 0x55C4, 0x55DC, 0x55E4,
	0x55D4, 0x5614, 0x55F7, 0x5616, 0x55FE, 0x55FD, 0x561B, 0x55F9, 0x564E, 0x5650, 0x71DF, 0x5634, 0x5636, 0x5632, 0x5638, 0xFFFF,
	0x566B, 0x5664, 0x562F, 0x566C, 0x566A, 0x5686, 0x5680, 0x568A, 0x56A0, 0x5694, 0x568F, 0x56A5, 0x56AE, 0x56B6, 0x56B4, 0x56C2,
	0x56BC, 0x56C1, 0x56C3, 0x56C0, 0x56C8, 0x56CE, 0x56D1, 0x56D3, 0x56D7, 0x56EE, 0x56F9, 0x5700, 0x56FF, 0x5704, 0x5709, 0x5708,
	0x570B, 0x570D, 0x5713, 0x5718, 0x5716, 0x55C7, 0x571C, 0x5726, 0x5737, 0x5738, 0x574E, 0x573B, 0x5740, 0x574F, 0x5769, 0x57C0,
	0x5788, 0x5761, 0x577F, 0x5789, 0x5793, 0x57A0, 0x57B3, 0x57A4, 0x57AA, 0x57B0, 0x57C3, 0x57C6, 0x57D4, 0x57D2, 0x57D3, 0x580A,
	0x57D6, 0x57E3, 0x580B, 0x5819, 0x581D, 0x5872, 0x5821, 0x5862, 0x584B, 0x5870, 0x6BC0, 0x5852, 0x583D, 0x5879, 0x5885, 0x58B9,
	0x589F, 0x58AB, 0x58BA, 0x58DE, 0x58BB, 0x58B8, 0x58AE, 0x58C5, 0x58D3, 0x58D1, 0x58D7, 0x58D9, 0x58D8, 0x58E5, 0x58DC, 0x58E4,
	0x58DF, 0x58EF, 0x58FA, 0x58F9, 0x58FB, 0x58FC, 0x58FD, 0x5902, 0x590A, 0x5910, 0x591B, 0x68A6, 0x5925, 0x592C, 0x592D, 0x5932,
	0x5938, 0x593E, 0x7AD2, 0x5955, 0x5950, 0x594E, 0x595A, 0x5958, 0x5962, 0x5960, 0x5967, 0x596C, 0x5969, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0X9B00 to 0X9BFF
	0x5978, 0x5981, 0x599D, 0x4F5E, 0x4FAB, 0x59A3, 0x59B2, 0x59C6, 0x59E8, 0x59DC, 0x598D, 0x59D9, 0x59DA, 0x5A25, 0x5A1F, 0x5A11,
	0x5A1C, 0x5A09, 0x5A1A, 0x5A40, 0x5A6C, 0x5A49, 0x5A35, 0x5A36, 0x5A62, 0x5A6A, 0x5A9A, 0x5ABC, 0x5ABE, 0x5ACB, 0x5AC2, 0x5ABD,
	0x5AE3, 0x5AD7, 0x5AE6, 0x5AE9, 0x5AD6, 0x5AFA, 0x5AFB, 0x5B0C, 0x5B0B, 0x5B16, 0x5B32, 0x5AD0, 0x5B2A, 0x5B36, 0x5B3E, 0x5B43,
	0x5B45, 0x5B40, 0x5B51, 0x5B55, 0x5B5A, 0x5B5B, 0x5B65, 0x5B69, 0x5B70, 0x5B73, 0x5B75, 0x5B78, 0x6588, 0x5B7A, 0x5B80, 0xFFFF,
	0x5B83, 0x5BA6, 0x5BB8, 0x5BC3, 0x5BC7, 0x5BC9, 0x5BD4, 0x5BD0, 0x5BE4, 0x5BE6, 0x5BE2, 0x5BDE, 0x5BE5, 0x5BEB, 0x5BF0, 0x5BF6,
	0x5BF3, 0x5C05, 0x5C07, 0x5C08, 0x5C0D, 0x5C13, 0x5C20, 0x5C22, 0x5C28, 0x5C38, 0x5C39, 0x5C41, 0x5C46, 0x5C4E, 0x5C53, 0x5C50,
	0x5C4F, 0x5B71, 0x5C6C, 0x5C6E, 0x4E62, 0x5C76, 0x5C79, 0x5C8C, 0x5C91, 0x5C94, 0x599B, 0x5CAB, 0x5CBB, 0x5CB6, 0x5CBC, 0x5CB7,
	0x5CC5, 0x5CBE, 0x5CC7, 0x5CD9, 0x5CE9, 0x5CFD, 0x5CFA, 0x5CED, 0x5D8C, 0x5CEA, 0x5D0B, 0x5D15, 0x5D17, 0x5D5C, 0x5D1F, 0x5D1B,
	0x5D11, 0x5D14, 0x5D22, 0x5D1A, 0x5D19, 0x5D18, 0x5D4C, 0x5D52, 0x5D4E, 0x5D4B, 0x5D6C, 0x5D73, 0x5D76, 0x5D87, 0x5D84, 0x5D82,
	0x5DA2, 0x5D9D, 0x5DAC, 0x5DAE, 0x5DBD, 0x5D90, 0x5DB7, 0x5DBC, 0x5DC9, 0x5DCD, 0x5DD3, 0x5DD2, 0x5DD6, 0x5DDB, 0x5DEB, 0x5DF2,
	0x5DF5, 0x5E0B, 0x5E1A, 0x5E19, 0x5E11, 0x5E1B, 0x5E36, 0x5E37, 0x5E44, 0x5E43, 0x5E40, 0x5E4E, 0x5E57, 0x5E54, 0x5E5F, 0x5E62,
	0x5E64, 0x5E47, 0x5E75, 0x5E76, 0x5E7A, 0x9EBC, 0x5E7F, 0x5EA0, 0x5EC1, 0x5EC2, 0x5EC8, 0x5ED0, 0x5ECF, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0X9C00 to 0X9CFF
	0x5ED6, 0x5EE3, 0x5EDD, 0x5EDA, 0x5EDB, 0x5EE2, 0x5EE1, 0x5EE8, 0x5EE9, 0x5EEC, 0x5EF1, 0x5EF3, 0x5EF0, 0x5EF4, 0x5EF8, 0x5EFE,
	0x5F03, 0x5F09, 0x5F5D, 0x5F5C, 0x5F0B, 0x5F11, 0x5F16, 0x5F29, 0x5F2D, 0x5F38, 0x5F41, 0x5F48, 0x5F4C, 0x5F4E, 0x5F2F, 0x5F51,
	0x5F56, 0x5F57, 0x5F59, 0x5F61, 0x5F6D, 0x5F73, 0x5F77, 0x5F83, 0x5F82, 0x5F7F, 0x5F8A, 0x5F88, 0x5F91, 0x5F87, 0x5F9E, 0x5F99,
	0x5F98, 0x5FA0, 0x5FA8, 0x5FAD, 0x5FBC, 0x5FD6, 0x5FFB, 0x5FE4, 0x5FF8, 0x5FF1, 0x5FDD, 0x60B3, 0x5FFF, 0x6021, 0x6060, 0xFFFF,
	0x6019, 0x6010, 0x6029, 0x600E, 0x6031, 0x601B, 0x6015, 0x602B, 0x6026, 0x600F, 0x603A, 0x605A, 0x6041, 0x606A, 0x6077, 0x605F,
	0x604A, 0x6046, 0x604D, 0x6063, 0x6043, 0x6064, 0x6042, 0x606C, 0x606B, 0x6059, 0x6081, 0x608D, 0x60E7, 0x6083, 0x609A, 0x6084,
	0x609B, 0x6096, 0x6097, 0x6092, 0x60A7, 0x608B, 0x60E1, 0x60B8, 0x60E0, 0x60D3, 0x60B4, 0x5FF0, 0x60BD, 0x60C6, 0x60B5, 0x60D8,
	0x614D, 0x6115, 0x6106, 0x60F6, 0x60F7, 0x6100, 0x60F4, 0x60FA, 0x6103, 0x6121, 0x60FB, 0x60F1, 0x610D, 0x610E, 0x6147, 0x613E,
	0x6128, 0x6127, 0x614A, 0x613F, 0x613C, 0x612C, 0x6134, 0x613D, 0x6142, 0x6144, 0x6173, 0x6177, 0x6158, 0x6159, 0x615A, 0x616B,
	0x6174, 0x616F, 0x6165, 0x6171, 0x615F, 0x615D, 0x6153, 0x6175, 0x6199, 0x6196, 0x6187, 0x61AC, 0x6194, 0x619A, 0x618A, 0x6191,
	0x61AB, 0x61AE, 0x61CC, 0x61CA, 0x61C9, 0x61F7, 0x61C8, 0x61C3, 0x61C6, 0x61BA, 0x61CB, 0x7F79, 0x61CD, 0x61E6, 0x61E3, 0x61F6,
	0x61FA, 0x61F4, 0x61FF, 0x61FD, 0x61FC, 0x61FE, 0x6200, 0x6208, 0x6209, 0x620D, 0x620C, 0x6214, 0x621B, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0X9D00 to 0X9DFF
	0x621E, 0x6221, 0x622A, 0x622E, 0x6230, 0x6232, 0x6233, 0x6241, 0x624E, 0x625E, 0x6263, 0x625B, 0x6260, 0x6268, 0x627C, 0x6282,
	0x6289, 0x627E, 0x6292, 0x6293, 0x6296, 0x62D4, 0x6283, 0x6294, 0x62D7, 0x62D1, 0x62BB, 0x62CF, 0x62FF, 0x62C6, 0x64D4, 0x62C8,
	0x62DC, 0x62CC, 0x62CA, 0x62C2, 0x62C7, 0x629B, 0x62C9, 0x630C, 0x62EE, 0x62F1, 0x6327, 0x6302, 0x6308, 0x62EF, 0x62F5, 0x6350,
	0x633E, 0x634D, 0x641C, 0x634F, 0x6396, 0x638E, 0x6380, 0x63AB, 0x6376, 0x63A3, 0x638F, 0x6389, 0x639F, 0x63B5, 0x636B, 0xFFFF,
	0x6369, 0x63BE, 0x63E9, 0x63C0, 0x63C6, 0x63E3, 0x63C9, 0x63D2, 0x63F6, 0x63C4, 0x6416, 0x6434, 0x6406, 0x6413, 0x6426, 0x6436,
	0x651D, 0x6417, 0x6428, 0x640F, 0x6467, 0x646F, 0x6476, 0x644E, 0x652A, 0x6495, 0x6493, 0x64A5, 0x64A9, 0x6488, 0x64BC, 0x64DA,
	0x64D2, 0x64C5, 0x64C7, 0x64BB, 0x64D8, 0x64C2, 0x64F1, 0x64E7, 0x8209, 0x64E0, 0x64E1, 0x62AC, 0x64E3, 0x64EF, 0x652C, 0x64F6,
	0x64F4, 0x64F2, 0x64FA, 0x6500, 0x64FD, 0x6518, 0x651C, 0x6505, 0x6524, 0x6523, 0x652B, 0x6534, 0x6535, 0x6537, 0x6536, 0x6538,
	0x754B, 0x6548, 0x6556, 0x6555, 0x654D, 0x6558, 0x655E, 0x655D, 0x6572, 0x6578, 0x6582, 0x6583, 0x8B8A, 0x659B, 0x659F, 0x65AB,
	0x65B7, 0x65C3, 0x65C6, 0x65C1, 0x65C4, 0x65CC, 0x65D2, 0x65DB, 0x65D9, 0x65E0, 0x65E1, 0x65F1, 0x6772, 0x660A, 0x6603, 0x65FB,
	0x6773, 0x6635, 0x6636, 0x6634, 0x661C, 0x664F, 0x6644, 0x6649, 0x6641, 0x665E, 0x665D, 0x6664, 0x6667, 0x6668, 0x665F, 0x6662,
	0x6670, 0x6683, 0x6688, 0x668E, 0x6689, 0x6684, 0x6698, 0x669D, 0x66C1, 0x66B9, 0x66C9, 0x66BE, 0x66BC, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0X9E00 to 0X9EFF
	0x66C4, 0x66B8, 0x66D6, 0x66DA, 0x66E0, 0x663F, 0x66E6, 0x66E9, 0x66F0, 0x66F5, 0x66F7, 0x670F, 0x6716, 0x671E, 0x6726, 0x6727,
	0x9738, 0x672E, 0x673F, 0x6736, 0x6741, 0x6738, 0x6737, 0x6746, 0x675E, 0x6760, 0x6759, 0x6763, 0x6764, 0x6789, 0x6770, 0x67A9,
	0x677C, 0x676A, 0x678C, 0x678B, 0x67A6, 0x67A1, 0x6785, 0x67B7, 0x67EF, 0x67B4, 0x67EC, 0x67B3, 0x67E9, 0x67B8, 0x67E4, 0x67DE,
	0x67DD, 0x67E2, 0x67EE, 0x67B9, 0x67CE, 0x67C6, 0x67E7, 0x6A9C, 0x681E, 0x6846, 0x6829, 0x6840, 0x684D, 0x6832, 0x684E, 0xFFFF,
	0x68B3, 0x682B, 0x6859, 0x6863, 0x6877, 0x687F, 0x689F, 0x688F, 0x68AD, 0x6894, 0x689D, 0x689B, 0x6883, 0x6AAE, 0x68B9, 0x6874,
	0x68B5, 0x68A0, 0x68BA, 0x690F, 0x688D, 0x687E, 0x6901, 0x68CA, 0x6908, 0x68D8, 0x6922, 0x6926, 0x68E1, 0x690C, 0x68CD, 0x68D4,
	0x68E7, 0x68D5, 0x6936, 0x6912, 0x6904, 0x68D7, 0x68E3, 0x6925, 0x68F9, 0x68E0, 0x68EF, 0x6928, 0x692A, 0x691A, 0x6923, 0x6921,
	0x68C6, 0x6979, 0x6977, 0x695C, 0x6978, 0x696B, 0x6954, 0x697E, 0x696E, 0x6939, 0x6974, 0x693D, 0x6959, 0x6930, 0x6961, 0x695E,
	0x695D, 0x6981, 0x696A, 0x69B2, 0x69AE, 0x69D0, 0x69BF, 0x69C1, 0x69D3, 0x69BE, 0x69CE, 0x5BE8, 0x69CA, 0x69DD, 0x69BB, 0x69C3,
	0x69A7, 0x6A2E, 0x6991, 0x69A0, 0x699C, 0x6995, 0x69B4, 0x69DE, 0x69E8, 0x6A02, 0x6A1B, 0x69FF, 0x6B0A, 0x69F9, 0x69F2, 0x69E7,
	0x6A05, 0x69B1, 0x6A1E, 0x69ED, 0x6A14, 0x69EB, 0x6A0A, 0x6A12, 0x6AC1, 0x6A23, 0x6A13, 0x6A44, 0x6A0C, 0x6A72, 0x6A36, 0x6A78,
	0x6A47, 0x6A62, 0x6A59, 0x6A66, 0x6A48, 0x6A38, 0x6A22, 0x6A90, 0x6A8D, 0x6AA0, 0x6A84, 0x6AA2, 0x6AA3, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0X9F00 to 0X9FFF
	0x6A97, 0x8617, 0x6ABB, 0x6AC3, 0x6AC2, 0x6AB8, 0x6AB3, 0x6AAC, 0x6ADE, 0x6AD1, 0x6ADF, 0x6AAA, 0x6ADA, 0x6AEA, 0x6AFB, 0x6B05,
	0x8616, 0x6AFA, 0x6B12, 0x6B16, 0x9B31, 0x6B1F, 0x6B38, 0x6B37, 0x76DC, 0x6B39, 0x98EE, 0x6B47, 0x6B43, 0x6B49, 0x6B50, 0x6B59,
	0x6B54, 0x6B5B, 0x6B5F, 0x6B61, 0x6B78, 0x6B79, 0x6B7F, 0x6B80, 0x6B84, 0x6B83, 0x6B8D, 0x6B98, 0x6B95, 0x6B9E, 0x6BA4, 0x6BAA,
	0x6BAB, 0x6BAF, 0x6BB2, 0x6BB1, 0x6BB3, 0x6BB7, 0x6BBC, 0x6BC6, 0x6BCB, 0x6BD3, 0x6BDF, 0x6BEC, 0x6BEB, 0x6BF3, 0x6BEF, 0xFFFF,
	0x9EBE, 0x6C08, 0x6C13, 0x6C14, 0x6C1B, 0x6C24, 0x6C23, 0x6C5E, 0x6C55, 0x6C62, 0x6C6A, 0x6C82, 0x6C8D, 0x6C9A, 0x6C81, 0x6C9B,
	0x6C7E, 0x6C68, 0x6C73, 0x6C92, 0x6C90, 0x6CC4, 0x6CF1, 0x6CD3, 0x6CBD, 0x6CD7, 0x6CC5, 0x6CDD, 0x6CAE, 0x6CB1, 0x6CBE, 0x6CBA,
	0x6CDB, 0x6CEF, 0x6CD9, 0x6CEA, 0x6D1F, 0x884D, 0x6D36, 0x6D2B, 0x6D3D, 0x6D38, 0x6D19, 0x6D35, 0x6D33, 0x6D12, 0x6D0C, 0x6D63,
	0x6D93, 0x6D64, 0x6D5A, 0x6D79, 0x6D59, 0x6D8E, 0x6D95, 0x6FE4, 0x6D85, 0x6DF9, 0x6E15, 0x6E0A, 0x6DB5, 0x6DC7, 0x6DE6, 0x6DB8,
	0x6DC6, 0x6DEC, 0x6DDE, 0x6DCC, 0x6DE8, 0x6DD2, 0x6DC5, 0x6DFA, 0x6DD9, 0x6DE4, 0x6DD5, 0x6DEA, 0x6DEE, 0x6E2D, 0x6E6E, 0x6E2E,
	0x6E19, 0x6E72, 0x6E5F, 0x6E3E, 0x6E23, 0x6E6B, 0x6E2B, 0x6E76, 0x6E4D, 0x6E1F, 0x6E43, 0x6E3A, 0x6E4E, 0x6E24, 0x6EFF, 0x6E1D,
	0x6E38, 0x6E82, 0x6EAA, 0x6E98, 0x6EC9, 0x6EB7, 0x6ED3, 0x6EBD, 0x6EAF, 0x6EC4, 0x6EB2, 0x6ED4, 0x6ED5, 0x6E8F, 0x6EA5, 0x6EC2,
	0x6E9F, 0x6F41, 0x6F11, 0x704C, 0x6EEC, 0x6EF8, 0x6EFE, 0x6F3F, 0x6EF2, 0x6F31, 0x6EEF, 0x6F32, 0x6ECC, 0xFFFF, 0xFFFF, 0xFFFF,
};

const wchar_t SjisToUnicodeTable4[] =
{
	// 0XE000 to 0XE0FF
	0x6F3E, 0x6F13, 0x6EF7, 0x6F86, 0x6F7A, 0x6F78, 0x6F81, 0x6F80, 0x6F6F, 0x6F5B, 0x6FF3, 0x6F6D, 0x6F82, 0x6F7C, 0x6F58, 0x6F8E,
	0x6F91, 0x6FC2, 0x6F66, 0x6FB3, 0x6FA3, 0x6FA1, 0x6FA4, 0x6FB9, 0x6FC6, 0x6FAA, 0x6FDF, 0x6FD5, 0x6FEC, 0x6FD4, 0x6FD8, 0x6FF1,
	0x6FEE, 0x6FDB, 0x7009, 0x700B, 0x6FFA, 0x7011, 0x7001, 0x700F, 0x6FFE, 0x701B, 0x701A, 0x6F74, 0x701D, 0x7018, 0x701F, 0x7030,
	0x703E, 0x7032, 0x7051, 0x7063, 0x7099, 0x7092, 0x70AF, 0x70F1, 0x70AC, 0x70B8, 0x70B3, 0x70AE, 0x70DF, 0x70CB, 0x70DD, 0xFFFF,
	0x70D9, 0x7109, 0x70FD, 0x711C, 0x7119, 0x7165, 0x7155, 0x7188, 0x7166, 0x7162, 0x714C, 0x7156, 0x716C, 0x718F, 0x71FB, 0x7184,
	0x7195, 0x71A8, 0x71AC, 0x71D7, 0x71B9, 0x71BE, 0x71D2, 0x71C9, 0x71D4, 0x71CE, 0x71E0, 0x71EC, 0x71E7, 0x71F5, 0x71FC, 0x71F9,
	0x71FF, 0x720D, 0x7210, 0x721B, 0x7228, 0x722D, 0x722C, 0x7230, 0x7232, 0x723B, 0x723C, 0x723F, 0x7240, 0x7246, 0x724B, 0x7258,
	0x7274, 0x727E, 0x7282, 0x7281, 0x7287, 0x7292, 0x7296, 0x72A2, 0x72A7, 0x72B9, 0x72B2, 0x72C3, 0x72C6, 0x72C4, 0x72CE, 0x72D2,
	0x72E2, 0x72E0, 0x72E1, 0x72F9, 0x72F7, 0x500F, 0x7317, 0x730A, 0x731C, 0x7316, 0x731D, 0x7334, 0x732F, 0x7329, 0x7325, 0x733E,
	0x734E, 0x734F, 0x9ED8, 0x7357, 0x736A, 0x7368, 0x7370, 0x7378, 0x7375, 0x737B, 0x737A, 0x73C8, 0x73B3, 0x73CE, 0x73BB, 0x73C0,
	0x73E5, 0x73EE, 0x73DE, 0x74A2, 0x7405, 0x746F, 0x7425, 0x73F8, 0x7432, 0x743A, 0x7455, 0x743F, 0x745F, 0x7459, 0x7441, 0x745C,
	0x7469, 0x7470, 0x7463, 0x746A, 0x7476, 0x747E, 0x748B, 0x749E, 0x74A7, 0x74CA, 0x74CF, 0x74D4, 0x73F1, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0XE100 to 0XE1FF
	0x74E0, 0x74E3, 0x74E7, 0x74E9, 0x74EE, 0x74F2, 0x74F0, 0x74F1, 0x74F8, 0x74F7, 0x7504, 0x7503, 0x7505, 0x750C, 0x750E, 0x750D,
	0x7515, 0x7513, 0x751E, 0x7526, 0x752C, 0x753C, 0x7544, 0x754D, 0x754A, 0x7549, 0x755B, 0x7546, 0x755A, 0x7569, 0x7564, 0x7567,
	0x756B, 0x756D, 0x7578, 0x7576, 0x7586, 0x7587, 0x7574, 0x758A, 0x7589, 0x7582, 0x7594, 0x759A, 0x759D, 0x75A5, 0x75A3, 0x75C2,
	0x75B3, 0x75C3, 0x75B5, 0x75BD, 0x75B8, 0x75BC, 0x75B1, 0x75CD, 0x75CA, 0x75D2, 0x75D9, 0x75E3, 0x75DE, 0x75FE, 0x75FF, 0xFFFF,
	0x75FC, 0x7601, 0x75F0, 0x75FA, 0x75F2, 0x75F3, 0x760B, 0x760D, 0x7609, 0x761F, 0x7627, 0x7620, 0x7621, 0x7622, 0x7624, 0x7634,
	0x7630, 0x763B, 0x7647, 0x7648, 0x7646, 0x765C, 0x7658, 0x7661, 0x7662, 0x7668, 0x7669, 0x766A, 0x7667, 0x766C, 0x7670, 0x7672,
	0x7676, 0x7678, 0x767C, 0x7680, 0x7683, 0x7688, 0x768B, 0x768E, 0x7696, 0x7693, 0x7699, 0x769A, 0x76B0, 0x76B4, 0x76B8, 0x76B9,
	0x76BA, 0x76C2, 0x76CD, 0x76D6, 0x76D2, 0x76DE, 0x76E1, 0x76E5, 0x76E7, 0x76EA, 0x862F, 0x76FB, 0x7708, 0x7707, 0x7704, 0x7729,
	0x7724, 0x771E, 0x7725, 0x7726, 0x771B, 0x7737, 0x7738, 0x7747, 0x775A, 0x7768, 0x776B, 0x775B, 0x7765, 0x777F, 0x777E, 0x7779,
	0x778E, 0x778B, 0x7791, 0x77A0, 0x779E, 0x77B0, 0x77B6, 0x77B9, 0x77BF, 0x77BC, 0x77BD, 0x77BB, 0x77C7, 0x77CD, 0x77D7, 0x77DA,
	0x77DC, 0x77E3, 0x77EE, 0x77FC, 0x780C, 0x7812, 0x7926, 0x7820, 0x792A, 0x7845, 0x788E, 0x7874, 0x7886, 0x787C, 0x789A, 0x788C,
	0x78A3, 0x78B5, 0x78AA, 0x78AF, 0x78D1, 0x78C6, 0x78CB, 0x78D4, 0x78BE, 0x78BC, 0x78C5, 0x78CA, 0x78EC, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0XE200 to 0XE2FF
	0x78E7, 0x78DA, 0x78FD, 0x78F4, 0x7907, 0x7912, 0x7911, 0x7919, 0x792C, 0x792B, 0x7940, 0x7960, 0x7957, 0x795F, 0x795A, 0x7955,
	0x7953, 0x797A, 0x797F, 0x798A, 0x799D, 0x79A7, 0x9F4B, 0x79AA, 0x79AE, 0x79B3, 0x79B9, 0x79BA, 0x79C9, 0x79D5, 0x79E7, 0x79EC,
	0x79E1, 0x79E3, 0x7A08, 0x7A0D, 0x7A18, 0x7A19, 0x7A20, 0x7A1F, 0x7980, 0x7A31, 0x7A3B, 0x7A3E, 0x7A37, 0x7A43, 0x7A57, 0x7A49,
	0x7A61, 0x7A62, 0x7A69, 0x9F9D, 0x7A70, 0x7A79, 0x7A7D, 0x7A88, 0x7A97, 0x7A95, 0x7A98, 0x7A96, 0x7AA9, 0x7AC8, 0x7AB0, 0xFFFF,
	0x7AB6, 0x7AC5, 0x7AC4, 0x7ABF, 0x9083, 0x7AC7, 0x7ACA, 0x7ACD, 0x7ACF, 0x7AD5, 0x7AD3, 0x7AD9, 0x7ADA, 0x7ADD, 0x7AE1, 0x7AE2,
	0x7AE6, 0x7AED, 0x7AF0, 0x7B02, 0x7B0F, 0x7B0A, 0x7B06, 0x7B33, 0x7B18, 0x7B19, 0x7B1E, 0x7B35, 0x7B28, 0x7B36, 0x7B50, 0x7B7A,
	0x7B04, 0x7B4D, 0x7B0B, 0x7B4C, 0x7B45, 0x7B75, 0x7B65, 0x7B74, 0x7B67, 0x7B70, 0x7B71, 0x7B6C, 0x7B6E, 0x7B9D, 0x7B98, 0x7B9F,
	0x7B8D, 0x7B9C, 0x7B9A, 0x7B8B, 0x7B92, 0x7B8F, 0x7B5D, 0x7B99, 0x7BCB, 0x7BC1, 0x7BCC, 0x7BCF, 0x7BB4, 0x7BC6, 0x7BDD, 0x7BE9,
	0x7C11, 0x7C14, 0x7BE6, 0x7BE5, 0x7C60, 0x7C00, 0x7C07, 0x7C13, 0x7BF3, 0x7BF7, 0x7C17, 0x7C0D, 0x7BF6, 0x7C23, 0x7C27, 0x7C2A,
	0x7C1F, 0x7C37, 0x7C2B, 0x7C3D, 0x7C4C, 0x7C43, 0x7C54, 0x7C4F, 0x7C40, 0x7C50, 0x7C58, 0x7C5F, 0x7C64, 0x7C56, 0x7C65, 0x7C6C,
	0x7C75, 0x7C83, 0x7C90, 0x7CA4, 0x7CAD, 0x7CA2, 0x7CAB, 0x7CA1, 0x7CA8, 0x7CB3, 0x7CB2, 0x7CB1, 0x7CAE, 0x7CB9, 0x7CBD, 0x7CC0,
	0x7CC5, 0x7CC2, 0x7CD8, 0x7CD2, 0x7CDC, 0x7CE2, 0x9B3B, 0x7CEF, 0x7CF2, 0x7CF4, 0x7CF6, 0x7CFA, 0x7D06, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0XE300 to 0XE3FF
	0x7D02, 0x7D1C, 0x7D15, 0x7D0A, 0x7D45, 0x7D4B, 0x7D2E, 0x7D32, 0x7D3F, 0x7D35, 0x7D46, 0x7D73, 0x7D56, 0x7D4E, 0x7D72, 0x7D68,
	0x7D6E, 0x7D4F, 0x7D63, 0x7D93, 0x7D89, 0x7D5B, 0x7D8F, 0x7D7D, 0x7D9B, 0x7DBA, 0x7DAE, 0x7DA3, 0x7DB5, 0x7DC7, 0x7DBD, 0x7DAB,
	0x7E3D, 0x7DA2, 0x7DAF, 0x7DDC, 0x7DB8, 0x7D9F, 0x7DB0, 0x7DD8, 0x7DDD, 0x7DE4, 0x7DDE, 0x7DFB, 0x7DF2, 0x7DE1, 0x7E05, 0x7E0A,
	0x7E23, 0x7E21, 0x7E12, 0x7E31, 0x7E1F, 0x7E09, 0x7E0B, 0x7E22, 0x7E46, 0x7E66, 0x7E3B, 0x7E35, 0x7E39, 0x7E43, 0x7E37, 0xFFFF,
	0x7E32, 0x7E3A, 0x7E67, 0x7E5D, 0x7E56, 0x7E5E, 0x7E59, 0x7E5A, 0x7E79, 0x7E6A, 0x7E69, 0x7E7C, 0x7E7B, 0x7E83, 0x7DD5, 0x7E7D,
	0x8FAE, 0x7E7F, 0x7E88, 0x7E89, 0x7E8C, 0x7E92, 0x7E90, 0x7E93, 0x7E94, 0x7E96, 0x7E8E, 0x7E9B, 0x7E9C, 0x7F38, 0x7F3A, 0x7F45,
	0x7F4C, 0x7F4D, 0x7F4E, 0x7F50, 0x7F51, 0x7F55, 0x7F54, 0x7F58, 0x7F5F, 0x7F60, 0x7F68, 0x7F69, 0x7F67, 0x7F78, 0x7F82, 0x7F86,
	0x7F83, 0x7F88, 0x7F87, 0x7F8C, 0x7F94, 0x7F9E, 0x7F9D, 0x7F9A, 0x7FA3, 0x7FAF, 0x7FB2, 0x7FB9, 0x7FAE, 0x7FB6, 0x7FB8, 0x8B71,
	0x7FC5, 0x7FC6, 0x7FCA, 0x7FD5, 0x7FD4, 0x7FE1, 0x7FE6, 0x7FE9, 0x7FF3, 0x7FF9, 0x98DC, 0x8006, 0x8004, 0x800B, 0x8012, 0x8018,
	0x8019, 0x801C, 0x8021, 0x8028, 0x803F, 0x803B, 0x804A, 0x8046, 0x8052, 0x8058, 0x805A, 0x805F, 0x8062, 0x8068, 0x8073, 0x8072,
	0x8070, 0x8076, 0x8079, 0x807D, 0x807F, 0x8084, 0x8086, 0x8085, 0x809B, 0x8093, 0x809A, 0x80AD, 0x5190, 0x80AC, 0x80DB, 0x80E5,
	0x80D9, 0x80DD, 0x80C4, 0x80DA, 0x80D6, 0x8109, 0x80EF, 0x80F1, 0x811B, 0x8129, 0x8123, 0x812F, 0x814B, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0XE400 to 0XE4FF
	0x968B, 0x8146, 0x813E, 0x8153, 0x8151, 0x80FC, 0x8171, 0x816E, 0x8165, 0x8166, 0x8174, 0x8183, 0x8188, 0x818A, 0x8180, 0x8182,
	0x81A0, 0x8195, 0x81A4, 0x81A3, 0x815F, 0x8193, 0x81A9, 0x81B0, 0x81B5, 0x81BE, 0x81B8, 0x81BD, 0x81C0, 0x81C2, 0x81BA, 0x81C9,
	0x81CD, 0x81D1, 0x81D9, 0x81D8, 0x81C8, 0x81DA, 0x81DF, 0x81E0, 0x81E7, 0x81FA, 0x81FB, 0x81FE, 0x8201, 0x8202, 0x8205, 0x8207,
	0x820A, 0x820D, 0x8210, 0x8216, 0x8229, 0x822B, 0x8238, 0x8233, 0x8240, 0x8259, 0x8258, 0x825D, 0x825A, 0x825F, 0x8264, 0xFFFF,
	0x8262, 0x8268, 0x826A, 0x826B, 0x822E, 0x8271, 0x8277, 0x8278, 0x827E, 0x828D, 0x8292, 0x82AB, 0x829F, 0x82BB, 0x82AC, 0x82E1,
	0x82E3, 0x82DF, 0x82D2, 0x82F4, 0x82F3, 0x82FA, 0x8393, 0x8303, 0x82FB, 0x82F9, 0x82DE, 0x8306, 0x82DC, 0x8309, 0x82D9, 0x8335,
	0x8334, 0x8316, 0x8332, 0x8331, 0x8340, 0x8339, 0x8350, 0x8345, 0x832F, 0x832B, 0x8317, 0x8318, 0x8385, 0x839A, 0x83AA, 0x839F,
	0x83A2, 0x8396, 0x8323, 0x838E, 0x8387, 0x838A, 0x837C, 0x83B5, 0x8373, 0x8375, 0x83A0, 0x8389, 0x83A8, 0x83F4, 0x8413, 0x83EB,
	0x83CE, 0x83FD, 0x8403, 0x83D8, 0x840B, 0x83C1, 0x83F7, 0x8407, 0x83E0, 0x83F2, 0x840D, 0x8422, 0x8420, 0x83BD, 0x8438, 0x8506,
	0x83FB, 0x846D, 0x842A, 0x843C, 0x855A, 0x8484, 0x8477, 0x846B, 0x84AD, 0x846E, 0x8482, 0x8469, 0x8446, 0x842C, 0x846F, 0x8479,
	0x8435, 0x84CA, 0x8462, 0x84B9, 0x84BF, 0x849F, 0x84D9, 0x84CD, 0x84BB, 0x84DA, 0x84D0, 0x84C1, 0x84C6, 0x84D6, 0x84A1, 0x8521,
	0x84FF, 0x84F4, 0x8517, 0x8518, 0x852C, 0x851F, 0x8515, 0x8514, 0x84FC, 0x8540, 0x8563, 0x8558, 0x8548, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0XE500 to 0XE5FF
	0x8541, 0x8602, 0x854B, 0x8555, 0x8580, 0x85A4, 0x8588, 0x8591, 0x858A, 0x85A8, 0x856D, 0x8594, 0x859B, 0x85EA, 0x8587, 0x859C,
	0x8577, 0x857E, 0x8590, 0x85C9, 0x85BA, 0x85CF, 0x85B9, 0x85D0, 0x85D5, 0x85DD, 0x85E5, 0x85DC, 0x85F9, 0x860A, 0x8613, 0x860B,
	0x85FE, 0x85FA, 0x8606, 0x8622, 0x861A, 0x8630, 0x863F, 0x864D, 0x4E55, 0x8654, 0x865F, 0x8667, 0x8671, 0x8693, 0x86A3, 0x86A9,
	0x86AA, 0x868B, 0x868C, 0x86B6, 0x86AF, 0x86C4, 0x86C6, 0x86B0, 0x86C9, 0x8823, 0x86AB, 0x86D4, 0x86DE, 0x86E9, 0x86EC, 0xFFFF,
	0x86DF, 0x86DB, 0x86EF, 0x8712, 0x8706, 0x8708, 0x8700, 0x8703, 0x86FB, 0x8711, 0x8709, 0x870D, 0x86F9, 0x870A, 0x8734, 0x873F,
	0x8737, 0x873B, 0x8725, 0x8729, 0x871A, 0x8760, 0x875F, 0x8778, 0x874C, 0x874E, 0x8774, 0x8757, 0x8768, 0x876E, 0x8759, 0x8753,
	0x8763, 0x876A, 0x8805, 0x87A2, 0x879F, 0x8782, 0x87AF, 0x87CB, 0x87BD, 0x87C0, 0x87D0, 0x96D6, 0x87AB, 0x87C4, 0x87B3, 0x87C7,
	0x87C6, 0x87BB, 0x87EF, 0x87F2, 0x87E0, 0x880F, 0x880D, 0x87FE, 0x87F6, 0x87F7, 0x880E, 0x87D2, 0x8811, 0x8816, 0x8815, 0x8822,
	0x8821, 0x8831, 0x8836, 0x8839, 0x8827, 0x883B, 0x8844, 0x8842, 0x8852, 0x8859, 0x885E, 0x8862, 0x886B, 0x8881, 0x887E, 0x889E,
	0x8875, 0x887D, 0x88B5, 0x8872, 0x8882, 0x8897, 0x8892, 0x88AE, 0x8899, 0x88A2, 0x888D, 0x88A4, 0x88B0, 0x88BF, 0x88B1, 0x88C3,
	0x88C4, 0x88D4, 0x88D8, 0x88D9, 0x88DD, 0x88F9, 0x8902, 0x88FC, 0x88F4, 0x88E8, 0x88F2, 0x8904, 0x890C, 0x890A, 0x8913, 0x8943,
	0x891E, 0x8925, 0x892A, 0x892B, 0x8941, 0x8944, 0x893B, 0x8936, 0x8938, 0x894C, 0x891D, 0x8960, 0x895E, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0XE600 to 0XE6FF
	0x8966, 0x8964, 0x896D, 0x896A, 0x896F, 0x8974, 0x8977, 0x897E, 0x8983, 0x8988, 0x898A, 0x8993, 0x8998, 0x89A1, 0x89A9, 0x89A6,
	0x89AC, 0x89AF, 0x89B2, 0x89BA, 0x89BD, 0x89BF, 0x89C0, 0x89DA, 0x89DC, 0x89DD, 0x89E7, 0x89F4, 0x89F8, 0x8A03, 0x8A16, 0x8A10,
	0x8A0C, 0x8A1B, 0x8A1D, 0x8A25, 0x8A36, 0x8A41, 0x8A5B, 0x8A52, 0x8A46, 0x8A48, 0x8A7C, 0x8A6D, 0x8A6C, 0x8A62, 0x8A85, 0x8A82,
	0x8A84, 0x8AA8, 0x8AA1, 0x8A91, 0x8AA5, 0x8AA6, 0x8A9A, 0x8AA3, 0x8AC4, 0x8ACD, 0x8AC2, 0x8ADA, 0x8AEB, 0x8AF3, 0x8AE7, 0xFFFF,
	0x8AE4, 0x8AF1, 0x8B14, 0x8AE0, 0x8AE2, 0x8AF7, 0x8ADE, 0x8ADB, 0x8B0C, 0x8B07, 0x8B1A, 0x8AE1, 0x8B16, 0x8B10, 0x8B17, 0x8B20,
	0x8B33, 0x97AB, 0x8B26, 0x8B2B, 0x8B3E, 0x8B28, 0x8B41, 0x8B4C, 0x8B4F, 0x8B4E, 0x8B49, 0x8B56, 0x8B5B, 0x8B5A, 0x8B6B, 0x8B5F,
	0x8B6C, 0x8B6F, 0x8B74, 0x8B7D, 0x8B80, 0x8B8C, 0x8B8E, 0x8B92, 0x8B93, 0x8B96, 0x8B99, 0x8B9A, 0x8C3A, 0x8C41, 0x8C3F, 0x8C48,
	0x8C4C, 0x8C4E, 0x8C50, 0x8C55, 0x8C62, 0x8C6C, 0x8C78, 0x8C7A, 0x8C82, 0x8C89, 0x8C85, 0x8C8A, 0x8C8D, 0x8C8E, 0x8C94, 0x8C7C,
	0x8C98, 0x621D, 0x8CAD, 0x8CAA, 0x8CBD, 0x8CB2, 0x8CB3, 0x8CAE, 0x8CB6, 0x8CC8, 0x8CC1, 0x8CE4, 0x8CE3, 0x8CDA, 0x8CFD, 0x8CFA,
	0x8CFB, 0x8D04, 0x8D05, 0x8D0A, 0x8D07, 0x8D0F, 0x8D0D, 0x8D10, 0x9F4E, 0x8D13, 0x8CCD, 0x8D14, 0x8D16, 0x8D67, 0x8D6D, 0x8D71,
	0x8D73, 0x8D81, 0x8D99, 0x8DC2, 0x8DBE, 0x8DBA, 0x8DCF, 0x8DDA, 0x8DD6, 0x8DCC, 0x8DDB, 0x8DCB, 0x8DEA, 0x8DEB, 0x8DDF, 0x8DE3,
	0x8DFC, 0x8E08, 0x8E09, 0x8DFF, 0x8E1D, 0x8E1E, 0x8E10, 0x8E1F, 0x8E42, 0x8E35, 0x8E30, 0x8E34, 0x8E4A, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0XE700 to 0XE7FF
	0x8E47, 0x8E49, 0x8E4C, 0x8E50, 0x8E48, 0x8E59, 0x8E64, 0x8E60, 0x8E2A, 0x8E63, 0x8E55, 0x8E76, 0x8E72, 0x8E7C, 0x8E81, 0x8E87,
	0x8E85, 0x8E84, 0x8E8B, 0x8E8A, 0x8E93, 0x8E91, 0x8E94, 0x8E99, 0x8EAA, 0x8EA1, 0x8EAC, 0x8EB0, 0x8EC6, 0x8EB1, 0x8EBE, 0x8EC5,
	0x8EC8, 0x8ECB, 0x8EDB, 0x8EE3, 0x8EFC, 0x8EFB, 0x8EEB, 0x8EFE, 0x8F0A, 0x8F05, 0x8F15, 0x8F12, 0x8F19, 0x8F13, 0x8F1C, 0x8F1F,
	0x8F1B, 0x8F0C, 0x8F26, 0x8F33, 0x8F3B, 0x8F39, 0x8F45, 0x8F42, 0x8F3E, 0x8F4C, 0x8F49, 0x8F46, 0x8F4E, 0x8F57, 0x8F5C, 0xFFFF,
	0x8F62, 0x8F63, 0x8F64, 0x8F9C, 0x8F9F, 0x8FA3, 0x8FAD, 0x8FAF, 0x8FB7, 0x8FDA, 0x8FE5, 0x8FE2, 0x8FEA, 0x8FEF, 0x9087, 0x8FF4,
	0x9005, 0x8FF9, 0x8FFA, 0x9011, 0x9015, 0x9021, 0x900D, 0x901E, 0x9016, 0x900B, 0x9027, 0x9036, 0x9035, 0x9039, 0x8FF8, 0x904F,
	0x9050, 0x9051, 0x9052, 0x900E, 0x9049, 0x903E, 0x9056, 0x9058, 0x905E, 0x9068, 0x906F, 0x9076, 0x96A8, 0x9072, 0x9082, 0x907D,
	0x9081, 0x9080, 0x908A, 0x9089, 0x908F, 0x90A8, 0x90AF, 0x90B1, 0x90B5, 0x90E2, 0x90E4, 0x6248, 0x90DB, 0x9102, 0x9112, 0x9119,
	0x9132, 0x9130, 0x914A, 0x9156, 0x9158, 0x9163, 0x9165, 0x9169, 0x9173, 0x9172, 0x918B, 0x9189, 0x9182, 0x91A2, 0x91AB, 0x91AF,
	0x91AA, 0x91B5, 0x91B4, 0x91BA, 0x91C0, 0x91C1, 0x91C9, 0x91CB, 0x91D0, 0x91D6, 0x91DF, 0x91E1, 0x91DB, 0x91FC, 0x91F5, 0x91F6,
	0x921E, 0x91FF, 0x9214, 0x922C, 0x9215, 0x9211, 0x925E, 0x9257, 0x9245, 0x9249, 0x9264, 0x9248, 0x9295, 0x923F, 0x924B, 0x9250,
	0x929C, 0x9296, 0x9293, 0x929B, 0x925A, 0x92CF, 0x92B9, 0x92B7, 0x92E9, 0x930F, 0x92FA, 0x9344, 0x932E, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0XE800 to 0XE8FF
	0x9319, 0x9322, 0x931A, 0x9323, 0x933A, 0x9335, 0x933B, 0x935C, 0x9360, 0x937C, 0x936E, 0x9356, 0x93B0, 0x93AC, 0x93AD, 0x9394,
	0x93B9, 0x93D6, 0x93D7, 0x93E8, 0x93E5, 0x93D8, 0x93C3, 0x93DD, 0x93D0, 0x93C8, 0x93E4, 0x941A, 0x9414, 0x9413, 0x9403, 0x9407,
	0x9410, 0x9436, 0x942B, 0x9435, 0x9421, 0x943A, 0x9441, 0x9452, 0x9444, 0x945B, 0x9460, 0x9462, 0x945E, 0x946A, 0x9229, 0x9470,
	0x9475, 0x9477, 0x947D, 0x945A, 0x947C, 0x947E, 0x9481, 0x947F, 0x9582, 0x9587, 0x958A, 0x9594, 0x9596, 0x9598, 0x9599, 0xFFFF,
	0x95A0, 0x95A8, 0x95A7, 0x95AD, 0x95BC, 0x95BB, 0x95B9, 0x95BE, 0x95CA, 0x6FF6, 0x95C3, 0x95CD, 0x95CC, 0x95D5, 0x95D4, 0x95D6,
	0x95DC, 0x95E1, 0x95E5, 0x95E2, 0x9621, 0x9628, 0x962E, 0x962F, 0x9642, 0x964C, 0x964F, 0x964B, 0x9677, 0x965C, 0x965E, 0x965D,
	0x965F, 0x9666, 0x9672, 0x966C, 0x968D, 0x9698, 0x9695, 0x9697, 0x96AA, 0x96A7, 0x96B1, 0x96B2, 0x96B0, 0x96B4, 0x96B6, 0x96B8,
	0x96B9, 0x96CE, 0x96CB, 0x96C9, 0x96CD, 0x894D, 0x96DC, 0x970D, 0x96D5, 0x96F9, 0x9704, 0x9706, 0x9708, 0x9713, 0x970E, 0x9711,
	0x970F, 0x9716, 0x9719, 0x9724, 0x972A, 0x9730, 0x9739, 0x973D, 0x973E, 0x9744, 0x9746, 0x9748, 0x9742, 0x9749, 0x975C, 0x9760,
	0x9764, 0x9766, 0x9768, 0x52D2, 0x976B, 0x9771, 0x9779, 0x9785, 0x977C, 0x9781, 0x977A, 0x9786, 0x978B, 0x978F, 0x9790, 0x979C,
	0x97A8, 0x97A6, 0x97A3, 0x97B3, 0x97B4, 0x97C3, 0x97C6, 0x97C8, 0x97CB, 0x97DC, 0x97ED, 0x9F4F, 0x97F2, 0x7ADF, 0x97F6, 0x97F5,
	0x980F, 0x980C, 0x9838, 0x9824, 0x9821, 0x9837, 0x983D, 0x9846, 0x984F, 0x984B, 0x986B, 0x986F, 0x9870, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0XE900 to 0XE9FF
	0x9871, 0x9874, 0x9873, 0x98AA, 0x98AF, 0x98B1, 0x98B6, 0x98C4, 0x98C3, 0x98C6, 0x98E9, 0x98EB, 0x9903, 0x9909, 0x9912, 0x9914,
	0x9918, 0x9921, 0x991D, 0x991E, 0x9924, 0x9920, 0x992C, 0x992E, 0x993D, 0x993E, 0x9942, 0x9949, 0x9945, 0x9950, 0x994B, 0x9951,
	0x9952, 0x994C, 0x9955, 0x9997, 0x9998, 0x99A5, 0x99AD, 0x99AE, 0x99BC, 0x99DF, 0x99DB, 0x99DD, 0x99D8, 0x99D1, 0x99ED, 0x99EE,
	0x99F1, 0x99F2, 0x99FB, 0x99F8, 0x9A01, 0x9A0F, 0x9A05, 0x99E2, 0x9A19, 0x9A2B, 0x9A37, 0x9A45, 0x9A42, 0x9A40, 0x9A43, 0xFFFF,
	0x9A3E, 0x9A55, 0x9A4D, 0x9A5B, 0x9A57, 0x9A5F, 0x9A62, 0x9A65, 0x9A64, 0x9A69, 0x9A6B, 0x9A6A, 0x9AAD, 0x9AB0, 0x9ABC, 0x9AC0,
	0x9ACF, 0x9AD1, 0x9AD3, 0x9AD4, 0x9ADE, 0x9ADF, 0x9AE2, 0x9AE3, 0x9AE6, 0x9AEF, 0x9AEB, 0x9AEE, 0x9AF4, 0x9AF1, 0x9AF7, 0x9AFB,
	0x9B06, 0x9B18, 0x9B1A, 0x9B1F, 0x9B22, 0x9B23, 0x9B25, 0x9B27, 0x9B28, 0x9B29, 0x9B2A, 0x9B2E, 0x9B2F, 0x9B32, 0x9B44, 0x9B43,
	0x9B4F, 0x9B4D, 0x9B4E, 0x9B51, 0x9B58, 0x9B74, 0x9B93, 0x9B83, 0x9B91, 0x9B96, 0x9B97, 0x9B9F, 0x9BA0, 0x9BA8, 0x9BB4, 0x9BC0,
	0x9BCA, 0x9BB9, 0x9BC6, 0x9BCF, 0x9BD1, 0x9BD2, 0x9BE3, 0x9BE2, 0x9BE4, 0x9BD4, 0x9BE1, 0x9C3A, 0x9BF2, 0x9BF1, 0x9BF0, 0x9C15,
	0x9C14, 0x9C09, 0x9C13, 0x9C0C, 0x9C06, 0x9C08, 0x9C12, 0x9C0A, 0x9C04, 0x9C2E, 0x9C1B, 0x9C25, 0x9C24, 0x9C21, 0x9C30, 0x9C47,
	0x9C32, 0x9C46, 0x9C3E, 0x9C5A, 0x9C60, 0x9C67, 0x9C76, 0x9C78, 0x9CE7, 0x9CEC, 0x9CF0, 0x9D09, 0x9D08, 0x9CEB, 0x9D03, 0x9D06,
	0x9D2A, 0x9D26, 0x9DAF, 0x9D23, 0x9D1F, 0x9D44, 0x9D15, 0x9D12, 0x9D41, 0x9D3F, 0x9D3E, 0x9D46, 0x9D48, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0XEA00 to 0XEAFF
	0x9D5D, 0x9D5E, 0x9D64, 0x9D51, 0x9D50, 0x9D59, 0x9D72, 0x9D89, 0x9D87, 0x9DAB, 0x9D6F, 0x9D7A, 0x9D9A, 0x9DA4, 0x9DA9, 0x9DB2,
	0x9DC4, 0x9DC1, 0x9DBB, 0x9DB8, 0x9DBA, 0x9DC6, 0x9DCF, 0x9DC2, 0x9DD9, 0x9DD3, 0x9DF8, 0x9DE6, 0x9DED, 0x9DEF, 0x9DFD, 0x9E1A,
	0x9E1B, 0x9E1E, 0x9E75, 0x9E79, 0x9E7D, 0x9E81, 0x9E88, 0x9E8B, 0x9E8C, 0x9E92, 0x9E95, 0x9E91, 0x9E9D, 0x9EA5, 0x9EA9, 0x9EB8,
	0x9EAA, 0x9EAD, 0x9761, 0x9ECC, 0x9ECE, 0x9ECF, 0x9ED0, 0x9ED4, 0x9EDC, 0x9EDE, 0x9EDD, 0x9EE0, 0x9EE5, 0x9EE8, 0x9EEF, 0xFFFF,
	0x9EF4, 0x9EF6, 0x9EF7, 0x9EF9, 0x9EFB, 0x9EFC, 0x9EFD, 0x9F07, 0x9F08, 0x76B7, 0x9F15, 0x9F21, 0x9F2C, 0x9F3E, 0x9F4A, 0x9F52,
	0x9F54, 0x9F63, 0x9F5F, 0x9F60, 0x9F61, 0x9F66, 0x9F67, 0x9F6C, 0x9F6A, 0x9F77, 0x9F72, 0x9F76, 0x9F95, 0x9F9C, 0x9FA0, 0x582F,
	0x69C7, 0x9059, 0x7464, 0x51DC, 0x7199, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
};

const wchar_t SjisToUnicodeTable5[] =
{
	// 0XED00 to 0XEDFF
	0x7E8A, 0x891C, 0x9348, 0x9288, 0x84DC, 0x4FC9, 0x70BB, 0x6631, 0x68C8, 0x92F9, 0x66FB, 0x5F45, 0x4E28, 0x4EE1, 0x4EFC, 0x4F00,
	0x4F03, 0x4F39, 0x4F56, 0x4F92, 0x4F8A, 0x4F9A, 0x4F94, 0x4FCD, 0x5040, 0x5022, 0x4FFF, 0x501E, 0x5046, 0x5070, 0x5042, 0x5094,
	0x50F4, 0x50D8, 0x514A, 0x5164, 0x519D, 0x51BE, 0x51EC, 0x5215, 0x529C, 0x52A6, 0x52C0, 0x52DB, 0x5300, 0x5307, 0x5324, 0x5372,
	0x5393, 0x53B2, 0x53DD, 0xFA0E, 0x549C, 0x548A, 0x54A9, 0x54FF, 0x5586, 0x5759, 0x5765, 0x57AC, 0x57C8, 0x57C7, 0xFA0F, 0xFFFF,
	0xFA10, 0x589E, 0x58B2, 0x590B, 0x5953, 0x595B, 0x595D, 0x5963, 0x59A4, 0x59BA, 0x5B56, 0x5BC0, 0x752F, 0x5BD8, 0x5BEC, 0x5C1E,
	0x5CA6, 0x5CBA, 0x5CF5, 0x5D27, 0x5D53, 0xFA11, 0x5D42, 0x5D6D, 0x5DB8, 0x5DB9, 0x5DD0, 0x5F21, 0x5F34, 0x5F67, 0x5FB7, 0x5FDE,
	0x605D, 0x6085, 0x608A, 0x60DE, 0x60D5, 0x6120, 0x60F2, 0x6111, 0x6137, 0x6130, 0x6198, 0x6213, 0x62A6, 0x63F5, 0x6460, 0x649D,
	0x64CE, 0x654E, 0x6600, 0x6615, 0x663B, 0x6609, 0x662E, 0x661E, 0x6624, 0x6665, 0x6657, 0x6659, 0xFA12, 0x6673, 0x6699, 0x66A0,
	0x66B2, 0x66BF, 0x66FA, 0x670E, 0xF929, 0x6766, 0x67BB, 0x6852, 0x67C0, 0x6801, 0x6844, 0x68CF, 0xFA13, 0x6968, 0xFA14, 0x6998,
	0x69E2, 0x6A30, 0x6A6B, 0x6A46, 0x6A73, 0x6A7E, 0x6AE2, 0x6AE4, 0x6BD6, 0x6C3F, 0x6C5C, 0x6C86, 0x6C6F, 0x6CDA, 0x6D04, 0x6D87,
	0x6D6F, 0x6D96, 0x6DAC, 0x6DCF, 0x6DF8, 0x6DF2, 0x6DFC, 0x6E39, 0x6E5C, 0x6E27, 0x6E3C, 0x6EBF, 0x6F88, 0x6FB5, 0x6FF5, 0x7005,
	0x7007, 0x7028, 0x7085, 0x70AB, 0x710F, 0x7104, 0x715C, 0x7146, 0x7147, 0xFA15, 0x71C1, 0x71FE, 0x72B1, 0xFFFF, 0xFFFF, 0xFFFF,
	// 0XEE00 to 0XEEFF
	0x72BE, 0x7324, 0xFA16, 0x7377, 0x73BD, 0x73C9, 0x73D6, 0x73E3, 0x73D2, 0x7407, 0x73F5, 0x7426, 0x742A, 0x7429, 0x742E, 0x7462,
	0x7489, 0x749F, 0x7501, 0x756F, 0x7682, 0x769C, 0x769E, 0x769B, 0x76A6, 0xFA17, 0x7746, 0x52AF, 0x7821, 0x784E, 0x7864, 0x787A,
	0x7930, 0xFA18, 0xFA19, 0xFA1A, 0x7994, 0xFA1B, 0x799B, 0x7AD1, 0x7AE7, 0xFA1C, 0x7AEB, 0x7B9E, 0xFA1D, 0x7D48, 0x7D5C, 0x7DB7,
	0x7DA0, 0x7DD6, 0x7E52, 0x7F47, 0x7FA1, 0xFA1E, 0x8301, 0x8362, 0x837F, 0x83C7, 0x83F6, 0x8448, 0x84B4, 0x8553, 0x8559, 0xFFFF,
	0x856B, 0xFA1F, 0x85B0, 0xFA20, 0xFA21, 0x8807, 0x88F5, 0x8A12, 0x8A37, 0x8A79, 0x8AA7, 0x8ABE, 0x8ADF, 0xFA22, 0x8AF6, 0x8B53,
	0x8B7F, 0x8CF0, 0x8CF4, 0x8D12, 0x8D76, 0xFA23, 0x8ECF, 0xFA24, 0xFA25, 0x9067, 0x90DE, 0xFA26, 0x9115, 0x9127, 0x91DA, 0x91D7,
	0x91DE, 0x91ED, 0x91EE, 0x91E4, 0x91E5, 0x9206, 0x9210, 0x920A, 0x923A, 0x9240, 0x923C, 0x924E, 0x9259, 0x9251, 0x9239, 0x9267,
	0x92A7, 0x9277, 0x9278, 0x92E7, 0x92D7, 0x92D9, 0x92D0, 0xFA27, 0x92D5, 0x92E0, 0x92D3, 0x9325, 0x9321, 0x92FB, 0xFA28, 0x931E,
	0x92FF, 0x931D, 0x9302, 0x9370, 0x9357, 0x93A4, 0x93C6, 0x93DE, 0x93F8, 0x9431, 0x9445, 0x9448, 0x9592, 0xF9DC, 0xFA29, 0x969D,
	0x96AF, 0x9733, 0x973B, 0x9743, 0x974D, 0x974F, 0x9751, 0x9755, 0x9857, 0x9865, 0xFA2A, 0xFA2B, 0x9927, 0xFA2C, 0x999E, 0x9A4E,
	0x9AD9, 0x9ADC, 0x9B75, 0x9B72, 0x9B8F, 0x9BB1, 0x9BBB, 0x9C00, 0x9D70, 0x9D6B, 0xFA2D, 0x9E19, 0x9ED1, 0xFFFF, 0xFFFF, 0x2170,
	0x2171, 0x2172, 0x2173, 0x2174, 0x2175, 0x2176, 0x2177, 0x2178, 0x2179, 0xFFE2, 0xFFE4, 0xFF07, 0xFF02, 0xFFFF, 0xFFFF, 0xFFFF,
};

wchar_t sjisToUnicode(unsigned short SjisCharacter)
{
	if (SjisCharacter < 0x80)
	{
		return SjisCharacter;
	} else if (SjisCharacter < 0x100)
	{
		return SjisToUnicodeTable1[SjisCharacter-0x80];
	}

	if ((SjisCharacter & 0xFF) < 0x40) return 0xFFFF;

	if (SjisCharacter >= 0x8100 && SjisCharacter < 0x8500)
	{
		SjisCharacter -= 0x8140;
		SjisCharacter -= (SjisCharacter >> 8) * 0x40;
		return SjisToUnicodeTable2[SjisCharacter];
	} else if (SjisCharacter >= 0x8700 && SjisCharacter < 0xA000)
	{
		SjisCharacter -= 0x8740;
		SjisCharacter -= (SjisCharacter >> 8) * 0x40;
		return SjisToUnicodeTable3[SjisCharacter];
	} else if (SjisCharacter >= 0xE000 && SjisCharacter < 0xEB00)
	{
		SjisCharacter -= 0xE040;
		SjisCharacter -= (SjisCharacter >> 8) * 0x40;
		return SjisToUnicodeTable4[SjisCharacter];
	} else if (SjisCharacter >= 0xED00 && SjisCharacter < 0xEF00)
	{
		SjisCharacter -= 0xED40;
		SjisCharacter -= (SjisCharacter >> 8) * 0x40;
		return SjisToUnicodeTable5[SjisCharacter];
	} else {
		return 0xFFFF;
	}
}

const size_t TEXTFILE_BUF_MAX_SIZE = 4096;

TextFile::TextFile()
{
	recursion = false;
	errorRetrieved = false;
	fromMemory = false;
	bufPos = 0;
	lineCount = 0;
}

TextFile::~TextFile()
{
	close();
}

void TextFile::openMemory(const std::wstring& content)
{
	fromMemory = true;
	this->content = content;
	contentPos = 0;
	size_ = (long) content.size();
	encoding = UTF16LE;
	mode = Read;
	lineCount = 0;
}

bool TextFile::open(const fs::path& fileName, Mode mode, Encoding defaultEncoding)
{
	setFileName(fileName);
	return open(mode,defaultEncoding);
}

bool TextFile::open(Mode mode, Encoding defaultEncoding)
{
	if (fileName.empty())
		return false;

	if (isOpen())
		close();

	fromMemory = false;
	guessedEncoding = false;
	encoding = defaultEncoding;
	this->mode = mode;

	// open all files as binary due to unicode
	switch (mode)
	{
	case Read:
		stream.open(fileName, fs::fstream::in | fs::fstream::binary);

		if (!stream.is_open())
			return false;
		break;
	case Write:
		stream.open(fileName, fs::fstream::out | fs::fstream::binary | fs::fstream::trunc);

		if (!stream.is_open())
			return false;

		buf.resize(TEXTFILE_BUF_MAX_SIZE);
		if (encoding != ASCII)
		{
			encoding = UTF8;
			writeCharacter(0xFEFF);
		}
		break;
	}

	// detect encoding
	unsigned char numBuffer[3] = {0};
	contentPos = 0;

	if (mode == Read)
	{
		size_ = fs::file_size(fileName);

		stream.read(reinterpret_cast<char *>(numBuffer), 3);
		switch (numBuffer[0] | (numBuffer[1] << 8))
		{
		case 0xFFFE:
			encoding = UTF16BE;
			stream.seekg(2);
			contentPos = 2;
			break;
		case 0xFEFF:
			encoding = UTF16LE;
			stream.seekg(2);
			contentPos = 2;
			break;
		case 0xBBEF:
			if (numBuffer[2] == 0xBF)
			{
				encoding = UTF8;
				contentPos = 3;
				break;
			}
			[[fallthrough]];
		default:
			if (defaultEncoding == GUESS)
			{
				encoding = UTF8;
				guessedEncoding = true;
			}
			stream.seekg(0);
			break;
		}
	} else {
		if (defaultEncoding == GUESS)
		{
			encoding = UTF8;
			guessedEncoding = true;
		}
	}

	return true;
}

void TextFile::close()
{
	if (isOpen() && !fromMemory)
	{
		bufDrainWrite();
		stream.close();
	}
	bufPos = 0;
}

long TextFile::tell()
{
	return (long) contentPos;
}

void TextFile::seek(long pos)
{
	if (fromMemory)
		contentPos = pos;
	else
		stream.seekg(pos);
}

void TextFile::bufFillRead()
{
	assert(mode == Read);

	buf.resize(TEXTFILE_BUF_MAX_SIZE);
	stream.read(&buf[0], TEXTFILE_BUF_MAX_SIZE);
	buf.resize(stream.gcount());
	bufPos = 0;
}

wchar_t TextFile::readCharacter()
{
	wchar_t value = 0;

	switch (encoding)
	{
	case UTF8:
		{
			value = bufGetChar();
			contentPos++;

			int extraBytes = 0;
			if ((value & 0xE0) == 0xC0)
			{
				extraBytes = 1;
				value &= 0x1F;
			} else if ((value & 0xF0) == 0xE0)
			{
				extraBytes = 2;
				value &= 0x0F;
			} else if (value > 0x7F)
			{
				errorText = tfm::format(L"One or more invalid UTF-8 characters in this file");
			}

			for (int i = 0; i < extraBytes; i++)
			{
				int b = bufGetChar();
				contentPos++;

				if ((b & 0xC0) != 0x80)
				{
					errorText = tfm::format(L"One or more invalid UTF-8 characters in this file");
				}

				value = (value << 6) | (b & 0x3F);
			}
		}
		break;
	case UTF16LE:
		if (fromMemory)
		{
			value = content[contentPos++];
		} else {
			value = bufGet16LE();
			contentPos += 2;
		}
		break;
	case UTF16BE:
		value = bufGet16BE();
		contentPos += 2;
		break;
	case SJIS:
		{
			unsigned short sjis = bufGetChar();
			contentPos++;
			if (sjis >= 0x80)
			{
				sjis = (sjis << 8) | bufGetChar();
				contentPos++;
			}
			value = sjisToUnicode(sjis);
			if (value == (wchar_t)-1)
			{
				errorText = tfm::format(L"One or more invalid Shift-JIS characters in this file");
			}
		}
		break;
	case ASCII:
		value = bufGetChar();
		contentPos++;
		break;

	case GUESS:
		errorText = tfm::format(L"Cannot read from GUESS encoding");
		break;
	}

	// convert \r\n to \n
	if (value == L'\r' && !recursion && !atEnd())
	{
		recursion = true;
		long pos = tell();
		wchar_t nextValue = readCharacter();
		recursion = false;

		if (nextValue == L'\n')
			return nextValue;
		seek(pos);
	}

	return value;
}

std::wstring TextFile::readLine()
{
	std::wstring result;
	wchar_t value;

	if (isOpen())
	{
		while (tell() < size() && (value = readCharacter()) != L'\n')
		{
			result += value;
		}
	}

	lineCount++;
	return result;
}

std::vector<std::wstring> TextFile::readAll()
{
	std::vector<std::wstring> result;
	while (!atEnd())
	{
		result.push_back(readLine());
	}

	return result;
}

void TextFile::bufPut(const void *p, const size_t len)
{
	assert(mode == Write);

	if (len > TEXTFILE_BUF_MAX_SIZE)
	{
		// Lots of data.  Let's write directly.
		bufDrainWrite();
		stream.write(reinterpret_cast<const char*>(p), len);
	}
	else
	{
		if (bufPos + len > TEXTFILE_BUF_MAX_SIZE)
			bufDrainWrite();

		memcpy(&buf[bufPos], p, len);
		bufPos += len;
	}
}

void TextFile::bufPut(const char c)
{
	assert(mode == Write);

	if (bufPos >= TEXTFILE_BUF_MAX_SIZE)
		bufDrainWrite();

	buf[bufPos++] = c;
}

void TextFile::bufDrainWrite()
{
	stream.write(buf.c_str(), bufPos);
	bufPos = 0;
}

void TextFile::writeCharacter(wchar_t character)
{
	if (mode != Write) return;

	// only support utf8 for now
	if (character < 0x80)
	{
#ifdef _WIN32
		if (character == L'\n')
		{
			bufPut('\r');
		}
#endif
		bufPut(character & 0x7F);
	} else if (encoding != ASCII)
	{
		if (character < 0x800)
		{
			bufPut(0xC0 | ((character >> 6) & 0x1F));
			bufPut(0x80 | (character & 0x3F));
		} else {
			bufPut(0xE0 | ((character >> 12) & 0xF));
			bufPut(0x80 | ((character >> 6) & 0x3F));
			bufPut(0x80 | (character & 0x3F));
		}
	}
}

void TextFile::write(const wchar_t* line)
{
	if (mode != Write) return;
	while (*line != 0)
	{
		writeCharacter(*line);
		line++;
	}
}

void TextFile::write(const std::wstring& line)
{
	write(line.c_str());
}

void TextFile::write(const char* line)
{
	if (mode != Write) return;
	while (*line != 0)
	{
		writeCharacter(*line);
		line++;
	}
}

void TextFile::write(const std::string& line)
{
	write(line.c_str());
}

void TextFile::writeLine(const wchar_t* line)
{
	if (mode != Write) return;
	write(line);
	writeCharacter(L'\n');
}

void TextFile::writeLine(const std::wstring& line)
{
	writeLine(line.c_str());
}

void TextFile::writeLine(const char* line)
{
	if (mode != Write) return;
	write(line);
	writeCharacter(L'\n');
}

void TextFile::writeLine(const std::string& line)
{
	writeLine(line.c_str());
}

void TextFile::writeLines(std::vector<std::wstring>& list)
{
	for (size_t i = 0; i < list.size(); i++)
	{
		writeLine(list[i]);
	}
}

struct EncodingValue
{
	const wchar_t* name;
	TextFile::Encoding value;
};

const EncodingValue encodingValues[] = {
	{ L"sjis",			TextFile::SJIS },
	{ L"shift-jis",		TextFile::SJIS },
	{ L"utf8",			TextFile::UTF8 },
	{ L"utf-8",			TextFile::UTF8 },
	{ L"utf16",			TextFile::UTF16LE },
	{ L"utf-16",		TextFile::UTF16LE },
	{ L"utf16-be",		TextFile::UTF16BE },
	{ L"utf-16-be",		TextFile::UTF16BE },
	{ L"ascii",			TextFile::ASCII },
};

TextFile::Encoding getEncodingFromString(const std::wstring& str)
{
	auto lowerCase = str;
	std::transform(lowerCase.begin(), lowerCase.end(), lowerCase.begin(), &::towlower);

	for (size_t i = 0; i < sizeof(encodingValues)/sizeof(EncodingValue); i++)
	{
		if (lowerCase.compare(encodingValues[i].name) == 0)
			return encodingValues[i].value;
	}

	return TextFile::GUESS;
}

// file: Util/Util.cpp

#include <sstream>

std::wstring convertUtf8ToWString(const char* source)
{
	std::wstring result;

	int index = 0;
	while (source[index] != 0)
	{
		int extraBytes = 0;
		int value = source[index++];

		if ((value & 0xE0) == 0xC0)
		{
			extraBytes = 1;
			value &= 0x1F;
		} else if ((value & 0xF0) == 0xE0)
		{
			extraBytes = 2;
			value &= 0x0F;
		} else if (value > 0x7F)
		{
			// error
			return std::wstring();
		}

		for (int i = 0; i < extraBytes; i++)
		{
			int b = source[index++];
			if ((b & 0xC0) != 0x80)
			{
			// error
			return std::wstring();
			}

			value = (value << 6) | (b & 0x3F);
		}

		result += value;
	}

	return result;
}

std::string convertWCharToUtf8(wchar_t character)
{
	std::string result;

	if (character < 0x80)
	{
		result += character & 0x7F;
	} else if (character < 0x800)
	{
		result += 0xC0 | ((character >> 6) & 0x1F);
		result += (0x80 | (character & 0x3F));
	} else {
		result += 0xE0 | ((character >> 12) & 0xF);
		result += 0x80 | ((character >> 6) & 0x3F);
		result += 0x80 | (character & 0x3F);
	}

	return result;
}

std::string convertWStringToUtf8(const std::wstring& source)
{
	std::string result;

	for (size_t i = 0; i < source.size(); i++)
	{
		wchar_t character = source[i];
		if (character < 0x80)
		{
			result += character & 0x7F;
		} else if (character < 0x800)
		{
			result += 0xC0 | ((character >> 6) & 0x1F);
			result += (0x80 | (character & 0x3F));
		} else {
			result += 0xE0 | ((character >> 12) & 0xF);
			result += 0x80 | ((character >> 6) & 0x3F);
			result += 0x80 | (character & 0x3F);
		}
	}

	return result;
}

std::wstring intToHexString(unsigned int value, int digits, bool prefix)
{
	std::wstring result;
	result.reserve((digits+prefix) ? 2 : 0);

	if (prefix)
	{
		result += '0';
		result += 'x';
	}

	while (digits > 8)
	{
		result += '0';
		digits--;
	}

	wchar_t buf[9];
	swprintf(buf,9,L"%0*X",digits,value);
	result += buf;

	return result;
}

std::wstring intToString(unsigned int value, int digits)
{
	std::wstring result;
	result.reserve(digits);

	while (digits > 8)
	{
		result += ' ';
		digits--;
	}

	wchar_t buf[9];
	swprintf(buf,9,L"%*d",digits,value);
	result += buf;

	return result;
}

bool stringToInt(const std::wstring& line, size_t start, size_t end, int64_t& result)
{
	// find base of number
	int32_t base = 10;
	if (line[start] == '0')
	{
		if (towlower(line[start+1]) == 'x')
		{
			base = 16;
			start += 2;
		} else if (towlower(line[start+1]) == 'o')
		{
			base = 8;
			start += 2;
		} else if (towlower(line[start+1]) == 'b' && towlower(line[end-1]) != 'h')
		{
			base = 2;
			start += 2;
		}
	}

	if (base == 10)
	{
		if (towlower(line[end-1]) == 'h')
		{
			base = 16;
			end--;
		} else if (towlower(line[end-1]) == 'b')
		{
			base = 2;
			end--;
		} else if (towlower(line[end-1]) == 'o')
		{
			base = 8;
			end--;
		}
	}

	// convert number
	result = 0;
	while (start < end)
	{
		wchar_t c = towlower(line[start++]);

		int32_t value = c >= 'a' ? c-'a'+10 : c-'0';

		if (value >= base)
			return false;

		result = (result*base) + value;
	}

	return true;
}

int32_t getFloatBits(float value)
{
	union { float f; int32_t i; } u;
	u.f = value;
	return u.i;
}

float bitsToFloat(int32_t value)
{
	union { float f; int32_t i; } u;
	u.i = value;
	return u.f;
}

int64_t getDoubleBits(double value)
{
	union { double f; int64_t i; } u;
	u.f = value;
	return u.i;
}

std::vector<std::wstring> getStringListFromArray(wchar_t** source, int count)
{
	std::vector<std::wstring> result;
	for (int i = 0; i < count; i++)
	{
		result.push_back(std::wstring(source[i]));
	}

	return result;
}

std::vector<std::wstring> splitString(const std::wstring& str, const wchar_t delim, bool skipEmpty)
{
	std::vector<std::wstring> result;
	std::wstringstream stream(str);
	std::wstring arg;
	while (std::getline(stream,arg,delim))
	{
		if (arg.empty() && skipEmpty) continue;
		result.push_back(arg);
	}

	return result;
}

std::wstring toWLowercase(const std::string& str)
{
	std::wstring result;
	for (size_t i = 0; i < str.size(); i++)
	{
		result += tolower(str[i]);
	}

	return result;
}

size_t replaceAll(std::wstring& str, const wchar_t* oldValue,const std::wstring& newValue)
{
	size_t pos = 0;
	size_t len = wcslen(oldValue);

	size_t count = 0;
	while ((pos = str.find(oldValue, pos)) != std::string::npos)
	{
		str.replace(pos,len,newValue);
		pos += newValue.length();
		count++;
	}

	return count;
}

bool startsWith(const std::wstring& str, const wchar_t* value, size_t stringPos)
{
	while (*value != 0 && stringPos < str.size())
	{
		if (str[stringPos++] != *value++)
			return false;
	}

	return *value == 0;
}

// file: Main/CommandLineInterface.h


int runFromCommandLine(const std::vector<std::wstring>& arguments, ArmipsArguments settings = {});

// file: Main/CommandLineInterface.cpp


static void printUsage(std::wstring executableName)
{
	Logger::printLine(L"armips assembler v%d.%d.%d (%s %s) by Kingcom",
		ARMIPS_VERSION_MAJOR, ARMIPS_VERSION_MINOR, ARMIPS_VERSION_REVISION, __DATE__, __TIME__);
	Logger::printLine(L"Usage: %s [optional parameters] <FILE>", executableName);
	Logger::printLine(L"");
	Logger::printLine(L"Optional parameters:");
	Logger::printLine(L" -temp <TEMP>              Output temporary assembly data to <TEMP> file");
	Logger::printLine(L" -sym  <SYM>               Output symbol data in the sym format to <SYM> file");
	Logger::printLine(L" -sym2 <SYM2>              Output symbol data in the sym2 format to <SYM2> file");
	Logger::printLine(L" -root <ROOT>              Use <ROOT> as working directory during execution");
	Logger::printLine(L" -equ  <NAME> <VAL>        Equivalent to \'<NAME> equ <VAL>\' in code");
	Logger::printLine(L" -strequ <NAME> <VAL>      Equivalent to \'<NAME> equ \"<VAL>\"\' in code");
	Logger::printLine(L" -definelabel <NAME> <VAL> Equivalent to \'.definelabel <NAME>, <VAL>\' in code");
	Logger::printLine(L" -erroronwarning           Treat all warnings like errors");
	Logger::printLine(L" -stat                     Show area usage statistics");
	Logger::printLine(L"");
	Logger::printLine(L"File arguments:");
	Logger::printLine(L" <FILE>                    Main assembly code file");
}

static bool parseArguments(const std::vector<std::wstring>& arguments, ArmipsArguments& settings)
{
	size_t argpos = 1;
	bool readflags = true;
	while (argpos < arguments.size())
	{
		if (readflags && arguments[argpos][0] == L'-')
		{
			if (arguments[argpos] == L"--")
			{
				readflags = false;
				argpos += 1;
			}
			else if (arguments[argpos] == L"-temp" && argpos + 1 < arguments.size())
			{
				settings.tempFileName = arguments[argpos + 1];
				argpos += 2;
			}
			else if (arguments[argpos] == L"-sym" && argpos + 1 < arguments.size())
			{
				settings.symFileName = arguments[argpos + 1];
				settings.symFileVersion = 1;
				argpos += 2;
			}
			else if (arguments[argpos] == L"-sym2" && argpos + 1 < arguments.size())
			{
				settings.symFileName = arguments[argpos + 1];
				settings.symFileVersion = 2;
				argpos += 2;
			}
			else if (arguments[argpos] == L"-erroronwarning")
			{
				settings.errorOnWarning = true;
				argpos += 1;
			}
			else if (arguments[argpos] == L"-stat")
			{
				settings.showStats = true;
				argpos += 1;
			}
			else if (arguments[argpos] == L"-equ" && argpos + 2 < arguments.size())
			{
				EquationDefinition def;

				def.name = arguments[argpos+1];
				std::transform(def.name.begin(), def.name.end(), def.name.begin(), ::towlower);

				if (!checkValidLabelName(def.name))
				{
					Logger::printError(Logger::Error, L"Invalid equation name \"%s\"", def.name);
					return false;
				}

				auto it = std::find_if(settings.equList.begin(), settings.equList.end(),
						[&def](EquationDefinition x) -> bool {return def.name == x.name;});
				if(it != settings.equList.end())
				{
					Logger::printError(Logger::Error, L"Equation name \"%s\" already defined", def.name);
					return false;
				}

				def.value = arguments[argpos + 2];
				settings.equList.push_back(def);
				argpos += 3;
			}
			else if (arguments[argpos] == L"-strequ" && argpos + 2 < arguments.size())
			{
				EquationDefinition def;

				def.name = arguments[argpos+1];
				std::transform(def.name.begin(), def.name.end(), def.name.begin(), ::towlower);

				if (!checkValidLabelName(def.name))
				{
					Logger::printError(Logger::Error, L"Invalid equation name \"%s\"", def.name);
					return false;
				}

				auto it = std::find_if(settings.equList.begin(), settings.equList.end(),
						[&def](EquationDefinition x) -> bool {return def.name == x.name;});
				if(it != settings.equList.end())
				{
					Logger::printError(Logger::Error, L"Equation name \"%s\" already defined", def.name);
					return false;
				}

				def.value = tfm::format(L"\"%s\"", arguments[argpos + 2]);
				settings.equList.push_back(def);
				argpos += 3;
			}
			else if (arguments[argpos] == L"-time")
			{
				Logger::printError(Logger::Warning, L"-time flag is deprecated");
				argpos += 1;
			}
			else if (arguments[argpos] == L"-root" && argpos + 1 < arguments.size())
			{
				std::error_code errorCode;
				fs::current_path(arguments[argpos + 1], errorCode);

				if (errorCode)
				{
					Logger::printError(Logger::Error, L"Could not open directory \"%s\"", arguments[argpos + 1]);
					return false;
				}
				argpos += 2;
			}
			else if (arguments[argpos] == L"-definelabel" && argpos + 2 < arguments.size())
			{
				LabelDefinition def;

				def.originalName = arguments[argpos + 1];
				def.name = def.originalName;
				std::transform(def.name.begin(), def.name.end(), def.name.begin(), ::towlower);

				if (!checkValidLabelName(def.name))
				{
					Logger::printError(Logger::Error, L"Invalid label name \"%s\"", def.name);
					return false;
				}

				auto it = std::find_if(settings.labels.begin(), settings.labels.end(),
						[&def](LabelDefinition x) -> bool {return def.name == x.name;});
				if(it != settings.labels.end())
				{
					Logger::printError(Logger::Error, L"Label name \"%s\" already defined", def.name);
					return false;
				}

				int64_t value;
				if (!stringToInt(arguments[argpos + 2], 0, arguments[argpos + 2].size(), value))
				{
					Logger::printError(Logger::Error, L"Invalid label value \"%s\"", arguments[argpos + 2]);
					return false;
				}
				def.value = value;

				settings.labels.push_back(def);
				argpos += 3;
			}
			else {
				Logger::printError(Logger::Error, L"Invalid command line argument \"%s\"\n", arguments[argpos]);
				printUsage(arguments[0]);
				return false;
			}
		}
		else {
			// only allow one input filename
			if (settings.inputFileName.empty())
			{
				settings.inputFileName = arguments[argpos];
				argpos++;
			}
			else {
				Logger::printError(Logger::Error, L"Multiple input assembly files specified\n");
				printUsage(arguments[0]);
				return false;
			}
		}
	}

	// ensure input file was specified
	if (settings.inputFileName.empty())
	{
		if (arguments.size() > 1)
			Logger::printError(Logger::Error, L"Missing input assembly file\n");

		printUsage(arguments[0]);
		return false;
	}

	// turn input filename into an absolute path
	if (settings.useAbsoluteFileNames)
		settings.inputFileName = fs::absolute(settings.inputFileName).lexically_normal();

	if (!fs::exists(settings.inputFileName))
	{
		Logger::printError(Logger::Error, L"File \"%s\" not found", settings.inputFileName.wstring());
		return false;
	}
	return true;
}

int runFromCommandLine(const std::vector<std::wstring>& arguments, ArmipsArguments settings)
{
	if (!parseArguments(arguments, settings))
	{
		if (arguments.size() > 1 && !settings.silent)
			Logger::printLine(L"Cannot parse arguments; aborting.");

		return 1;
	}

	if (!runArmips(settings))
	{
		if (!settings.silent)
			Logger::printLine(L"Aborting.");

		return 1;
	}

	return 0;
}

// file: Core/ELF/ElfFile.cpp


#include <algorithm>
#include <cctype>
#include <cstring>
#include <vector>

static bool stringEqualInsensitive(const std::string& a, const std::string& b)
{
	if (a.size() != b.size())
		return false;

	auto compare = [](char c1, char c2)
	{
		return std::tolower(c1) == std::tolower(c2);
	};

	return std::equal(a.begin(), a.end(), b.begin(), compare);
}

bool compareSection(ElfSection* a, ElfSection* b)
{
	return a->getOffset() < b->getOffset();
}

ElfSection::ElfSection(Elf32_Shdr header): header(header)
{
	owner = nullptr;
}

void ElfSection::setOwner(ElfSegment* segment)
{
	header.sh_offset -= segment->getOffset();
	owner = segment;
}

void ElfSection::writeHeader(ByteArray& data, size_t pos, Endianness endianness)
{
	data.replaceDoubleWord(pos + 0x00, header.sh_name, endianness);
	data.replaceDoubleWord(pos + 0x04, header.sh_type, endianness);
	data.replaceDoubleWord(pos + 0x08, header.sh_flags, endianness);
	data.replaceDoubleWord(pos + 0x0C, header.sh_addr, endianness);
	data.replaceDoubleWord(pos + 0x10, header.sh_offset, endianness);
	data.replaceDoubleWord(pos + 0x14, header.sh_size, endianness);
	data.replaceDoubleWord(pos + 0x18, header.sh_link, endianness);
	data.replaceDoubleWord(pos + 0x1C, header.sh_info, endianness);
	data.replaceDoubleWord(pos + 0x20, header.sh_addralign, endianness);
	data.replaceDoubleWord(pos + 0x24, header.sh_entsize, endianness);
}

// only called for segmentless sections
void ElfSection::writeData(ByteArray& output)
{
	if (header.sh_type == SHT_NULL) return;

	// nobits sections still get a provisional file address
	if (header.sh_type == SHT_NOBITS)
	{
		header.sh_offset = (Elf32_Off) output.size();
	}

	if (header.sh_addralign != (unsigned) -1)
		output.alignSize(header.sh_addralign);
	header.sh_offset = (Elf32_Off) output.size();
	output.append(data);
}

void ElfSection::setOffsetBase(int base)
{
	header.sh_offset += base;
}

ElfSegment::ElfSegment(Elf32_Phdr header, ByteArray& segmentData): header(header)
{
	data = segmentData;
	paddrSection = nullptr;
}

bool ElfSegment::isSectionPartOf(ElfSection* section)
{
	int sectionStart = section->getOffset();
	int sectionSize = section->getType() == SHT_NOBITS ? 0 : section->getSize();
	int sectionEnd = sectionStart+sectionSize;

	int segmentStart = header.p_offset;
	int segmentEnd = segmentStart+header.p_filesz;

	// exclusive > in case the size is 0
	if (sectionStart < (int)header.p_offset || sectionStart > segmentEnd) return false;

	// does an empty section belong to this or the next segment? hm...
	if (sectionStart == segmentEnd) return sectionSize == 0;

	// the start is inside the section and the size is not 0, so the end should be in here too
	if (sectionEnd > segmentEnd)
	{
		Logger::printError(Logger::Error,L"Section partially contained in segment");
		return false;
	}

	return true;
}

void ElfSegment::addSection(ElfSection* section)
{
	if (header.p_paddr != 0)
	{
		if (section->getOffset() == header.p_paddr)
		{
			paddrSection = section;
		}
	}

	section->setOwner(this);
	sections.push_back(section);
}

void ElfSegment::writeData(ByteArray& output)
{
	if (sections.size() == 0)
	{
		output.alignSize(header.p_align);
		if (header.p_offset == header.p_paddr)
			header.p_paddr = (Elf32_Addr) output.size();

		header.p_offset = (Elf32_Off) output.size();
		return;
	}

	// align segment to alignment of first section
	int align = std::max<int>(sections[0]->getAlignment(),16);
	output.alignSize(align);

	header.p_offset = (Elf32_Off) output.size();
	for (int i = 0; i < (int)sections.size(); i++)
	{
		sections[i]->setOffsetBase(header.p_offset);
	}

	if (paddrSection)
	{
		header.p_paddr = paddrSection->getOffset();
	}

	output.append(data);
}

void ElfSegment::writeHeader(ByteArray& data, size_t pos, Endianness endianness)
{
	data.replaceDoubleWord(pos + 0x00, header.p_type, endianness);
	data.replaceDoubleWord(pos + 0x04, header.p_offset, endianness);
	data.replaceDoubleWord(pos + 0x08, header.p_vaddr, endianness);
	data.replaceDoubleWord(pos + 0x0C, header.p_paddr, endianness);
	data.replaceDoubleWord(pos + 0x10, header.p_filesz, endianness);
	data.replaceDoubleWord(pos + 0x14, header.p_memsz, endianness);
	data.replaceDoubleWord(pos + 0x18, header.p_flags, endianness);
	data.replaceDoubleWord(pos + 0x1C, header.p_align, endianness);
}

void ElfSegment::splitSections()
{

}

int ElfSegment::findSection(const std::string& name)
{
	for (size_t i = 0; i < sections.size(); i++)
	{
		if (stringEqualInsensitive(name,sections[i]->getName()))
			return (int)i;
	}

	return -1;
}

void ElfSegment::writeToData(size_t offset, void* src, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		data[offset+i] = ((byte*)src)[i];
	}
}

void ElfSegment::sortSections()
{
	std::sort(sections.begin(),sections.end(),compareSection);
}

void ElfFile::loadSectionNames()
{
	if (fileHeader.e_shstrndx == SHN_UNDEF) return;

	// check if the string table is actually a string table
	// sometimes it gives the wrong section id
	size_t strTablePos = sections[fileHeader.e_shstrndx]->getOffset();
	size_t strTableSize = sections[fileHeader.e_shstrndx]->getSize();
	for (size_t i = 0; i < strTableSize; i++)
	{
		if (fileData[strTablePos+i] != 0 && fileData[strTablePos+i] < 0x20)
			return;
		if (fileData[strTablePos+i] > 0x7F)
			return;
	}

	for (size_t i = 0; i < sections.size(); i++)
	{
		ElfSection* section = sections[i];
		if (section->getType() == SHT_NULL) continue;

		int strTablePos = sections[fileHeader.e_shstrndx]->getOffset();
		int offset = strTablePos+section->getNameOffset();

		char* name = (char*) fileData.data(offset);
		std::string strName = name;
		section->setName(strName);
	}
}

void ElfFile::determinePartOrder()
{
	size_t segmentTable = fileHeader.e_phoff;
	size_t sectionTable = fileHeader.e_shoff;

	// segments
	size_t firstSegmentStart = fileData.size(), lastSegmentEnd = 0;
	for (size_t i = 0; i < fileHeader.e_phnum; i++)
	{
		size_t pos = fileHeader.e_phoff+i*fileHeader.e_phentsize;

		Elf32_Phdr segmentHeader;
		loadProgramHeader(segmentHeader, fileData, pos);
		size_t end = segmentHeader.p_offset + segmentHeader.p_filesz;

		if (segmentHeader.p_offset < firstSegmentStart) firstSegmentStart = segmentHeader.p_offset;
		if (lastSegmentEnd < end) lastSegmentEnd = end;
	}

	// segmentless sections
	size_t firstSectionStart = fileData.size(), lastSectionEnd = 0;
	for (size_t i = 0; i < segmentlessSections.size(); i++)
	{
		if (segmentlessSections[i]->getType() == SHT_NULL) continue;

		size_t start = segmentlessSections[i]->getOffset();
		size_t end = start+segmentlessSections[i]->getSize();

		if (start == 0 && end == 0)
			continue;
		if (start < firstSectionStart) firstSectionStart = start;
		if (lastSectionEnd < end) lastSectionEnd = end;
	}

	struct PartsSort {
		size_t offset;
		ElfPart type;
		bool operator<(const PartsSort& other) const { return offset < other.offset; };
	};

	PartsSort temp[4] = {
		{ segmentTable,				ELFPART_SEGMENTTABLE },
		{ sectionTable,				ELFPART_SECTIONTABLE },
		{ firstSegmentStart,		ELFPART_SEGMENTS },
		{ firstSectionStart,		ELFPART_SEGMENTLESSSECTIONS },
	};

	std::sort(&temp[0],&temp[4]);

	for (size_t i = 0; i < 4; i++)
	{
		partsOrder[i] = temp[i].type;
	}
}

int ElfFile::findSegmentlessSection(const std::string& name)
{
	for (size_t i = 0; i < segmentlessSections.size(); i++)
	{
		if (stringEqualInsensitive(name,segmentlessSections[i]->getName()))
			return (int)i;
	}

	return -1;
}

void ElfFile::loadElfHeader()
{
	memcpy(fileHeader.e_ident, &fileData[0], sizeof(fileHeader.e_ident));
	Endianness endianness = getEndianness();
	fileHeader.e_type = fileData.getWord(0x10, endianness);
	fileHeader.e_machine = fileData.getWord(0x12, endianness);
	fileHeader.e_version = fileData.getDoubleWord(0x14, endianness);
	fileHeader.e_entry = fileData.getDoubleWord(0x18, endianness);
	fileHeader.e_phoff = fileData.getDoubleWord(0x1C, endianness);
	fileHeader.e_shoff = fileData.getDoubleWord(0x20, endianness);
	fileHeader.e_flags = fileData.getDoubleWord(0x24, endianness);
	fileHeader.e_ehsize = fileData.getWord(0x28, endianness);
	fileHeader.e_phentsize = fileData.getWord(0x2A, endianness);
	fileHeader.e_phnum = fileData.getWord(0x2C, endianness);
	fileHeader.e_shentsize = fileData.getWord(0x2E, endianness);
	fileHeader.e_shnum = fileData.getWord(0x30, endianness);
	fileHeader.e_shstrndx = fileData.getWord(0x32, endianness);
}

void ElfFile::writeHeader(ByteArray& data, size_t pos, Endianness endianness)
{
	memcpy(&fileData[0], fileHeader.e_ident, sizeof(fileHeader.e_ident));
	data.replaceWord(pos + 0x10, fileHeader.e_type, endianness);
	data.replaceWord(pos + 0x12, fileHeader.e_machine, endianness);
	data.replaceDoubleWord(pos + 0x14, fileHeader.e_version, endianness);
	data.replaceDoubleWord(pos + 0x18, fileHeader.e_entry, endianness);
	data.replaceDoubleWord(pos + 0x1C, fileHeader.e_phoff, endianness);
	data.replaceDoubleWord(pos + 0x20, fileHeader.e_shoff, endianness);
	data.replaceDoubleWord(pos + 0x24, fileHeader.e_flags, endianness);
	data.replaceWord(pos + 0x28, fileHeader.e_ehsize, endianness);
	data.replaceWord(pos + 0x2A, fileHeader.e_phentsize, endianness);
	data.replaceWord(pos + 0x2C, fileHeader.e_phnum, endianness);
	data.replaceWord(pos + 0x2E, fileHeader.e_shentsize, endianness);
	data.replaceWord(pos + 0x30, fileHeader.e_shnum, endianness);
	data.replaceWord(pos + 0x32, fileHeader.e_shstrndx, endianness);
}

void ElfFile::loadProgramHeader(Elf32_Phdr& header, ByteArray& data, size_t pos)
{
	Endianness endianness = getEndianness();
	header.p_type   = data.getDoubleWord(pos + 0x00, endianness);
	header.p_offset = data.getDoubleWord(pos + 0x04, endianness);
	header.p_vaddr  = data.getDoubleWord(pos + 0x08, endianness);
	header.p_paddr  = data.getDoubleWord(pos + 0x0C, endianness);
	header.p_filesz = data.getDoubleWord(pos + 0x10, endianness);
	header.p_memsz  = data.getDoubleWord(pos + 0x14, endianness);
	header.p_flags  = data.getDoubleWord(pos + 0x18, endianness);
	header.p_align  = data.getDoubleWord(pos + 0x1C, endianness);
}

void ElfFile::loadSectionHeader(Elf32_Shdr& header, ByteArray& data, size_t pos)
{
	Endianness endianness = getEndianness();
	header.sh_name      = data.getDoubleWord(pos + 0x00, endianness);
	header.sh_type      = data.getDoubleWord(pos + 0x04, endianness);
	header.sh_flags     = data.getDoubleWord(pos + 0x08, endianness);
	header.sh_addr      = data.getDoubleWord(pos + 0x0C, endianness);
	header.sh_offset    = data.getDoubleWord(pos + 0x10, endianness);
	header.sh_size      = data.getDoubleWord(pos + 0x14, endianness);
	header.sh_link      = data.getDoubleWord(pos + 0x18, endianness);
	header.sh_info      = data.getDoubleWord(pos + 0x1C, endianness);
	header.sh_addralign = data.getDoubleWord(pos + 0x20, endianness);
	header.sh_entsize   = data.getDoubleWord(pos + 0x24, endianness);
}

bool ElfFile::load(const fs::path& fileName, bool sort)
{
	ByteArray data = ByteArray::fromFile(fileName);
	if (data.size() == 0)
		return false;
	return load(data,sort);
}

bool ElfFile::load(ByteArray& data, bool sort)
{
	fileData = data;

	loadElfHeader();
	symTab = nullptr;
	strTab = nullptr;

	// load segments
	for (size_t i = 0; i < fileHeader.e_phnum; i++)
	{
		size_t pos = fileHeader.e_phoff+i*fileHeader.e_phentsize;

		Elf32_Phdr sectionHeader;
		loadProgramHeader(sectionHeader, fileData, pos);

		ByteArray segmentData = fileData.mid(sectionHeader.p_offset,sectionHeader.p_filesz);
		ElfSegment* segment = new ElfSegment(sectionHeader,segmentData);
		segments.push_back(segment);
	}

	// load sections and assign them to segments
	for (int i = 0; i < fileHeader.e_shnum; i++)
	{
		size_t pos = fileHeader.e_shoff+i*fileHeader.e_shentsize;

		Elf32_Shdr sectionHeader;
		loadSectionHeader(sectionHeader, fileData, pos);

		ElfSection* section = new ElfSection(sectionHeader);
		sections.push_back(section);

		// check if the section belongs to a segment
		ElfSegment* owner = nullptr;
		for (int k = 0; k < (int)segments.size(); k++)
		{
			if (segments[k]->isSectionPartOf(section))
			{
				owner = segments[k];
				break;
			}
		}

		if (owner != nullptr)
		{
			owner->addSection(section);
		} else {
			if (section->getType() != SHT_NOBITS && section->getType() != SHT_NULL)
			{
				ByteArray data = fileData.mid(section->getOffset(),section->getSize());
				section->setData(data);
			}

			switch (section->getType())
			{
			case SHT_SYMTAB:
				symTab = section;
				break;
			case SHT_STRTAB:
				if (!strTab || i != fileHeader.e_shstrndx)
				{
					strTab = section;
				}
				break;
			}

			segmentlessSections.push_back(section);
		}
	}

	determinePartOrder();
	loadSectionNames();

	if (sort)
	{
		std::sort(segmentlessSections.begin(),segmentlessSections.end(),compareSection);

		for (int i = 0; i < (int)segments.size(); i++)
		{
			segments[i]->sortSections();
		}
	}

	return true;
}

void ElfFile::save(const fs::path& fileName)
{
	fileData.clear();

	// reserve space for header and table data
	fileData.reserveBytes(sizeof(Elf32_Ehdr));

	for (size_t i = 0; i < 4; i++)
	{
		switch (partsOrder[i])
		{
		case ELFPART_SEGMENTTABLE:
			fileData.alignSize(4);
			fileHeader.e_phoff = (Elf32_Off) fileData.size();
			fileData.reserveBytes(segments.size()*fileHeader.e_phentsize);
			break;
		case ELFPART_SECTIONTABLE:
			fileData.alignSize(4);
			fileHeader.e_shoff = (Elf32_Off) fileData.size();
			fileData.reserveBytes(sections.size()*fileHeader.e_shentsize);
			break;
		case ELFPART_SEGMENTS:
			for (size_t i = 0; i < segments.size(); i++)
			{
				segments[i]->writeData(fileData);
			}
			break;
		case ELFPART_SEGMENTLESSSECTIONS:
			for (size_t i = 0; i < segmentlessSections.size(); i++)
			{
				segmentlessSections[i]->writeData(fileData);
			}
			break;
		}
	}

	// copy data to the tables
	Endianness endianness = getEndianness();
	writeHeader(fileData, 0, endianness);
	for (size_t i = 0; i < segments.size(); i++)
	{
		size_t pos = fileHeader.e_phoff+i*fileHeader.e_phentsize;
		segments[i]->writeHeader(fileData, pos, endianness);
	}

	for (size_t i = 0; i < sections.size(); i++)
	{
		size_t pos = fileHeader.e_shoff+i*fileHeader.e_shentsize;
		sections[i]->writeHeader(fileData, pos, endianness);
	}

	fileData.toFile(fileName);
}

int ElfFile::getSymbolCount()
{
	if (symTab == nullptr)
		return 0;

	return symTab->getSize()/sizeof(Elf32_Sym);
}

bool ElfFile::getSymbol(Elf32_Sym& symbol, size_t index)
{
	if (symTab == nullptr)
		return false;

	ByteArray &data = symTab->getData();
	size_t pos = index*sizeof(Elf32_Sym);
	Endianness endianness = getEndianness();
	symbol.st_name  = data.getDoubleWord(pos + 0x00, endianness);
	symbol.st_value = data.getDoubleWord(pos + 0x04, endianness);
	symbol.st_size  = data.getDoubleWord(pos + 0x08, endianness);
	symbol.st_info  = data[pos + 0x0C];
	symbol.st_other = data[pos + 0x0D];
	symbol.st_shndx = data.getWord(pos + 0x0E, endianness);

	return true;
}

const char* ElfFile::getStrTableString(size_t pos)
{
	if (strTab == nullptr)
		return nullptr;

	return (const char*) &strTab->getData()[pos];
}

// file: Core/ELF/ElfRelocator.cpp


#include <cstring>

struct ArFileHeader
{
	char fileName[16];
	char modifactionTime[12];
	char ownerId[6];
	char groupId[6];
	char fileMode[8];
	char fileSize[10];
	char magic[2];
};

struct ArFileEntry
{
	std::wstring name;
	ByteArray data;
};

std::vector<ArFileEntry> loadArArchive(const fs::path& inputName)
{
	ByteArray input = ByteArray::fromFile(inputName);
	std::vector<ArFileEntry> result;

	if (input.size() < 8 || memcmp(input.data(),"!<arch>\n",8) != 0)
	{
		if (input.size() < 4 || memcmp(input.data(),"\x7F""ELF",4) != 0)
			return result;

		ArFileEntry entry;
		entry.name = inputName.filename().wstring();
		entry.data = input;
		result.push_back(entry);
		return result;
	}

	size_t pos = 8;
	while (pos < input.size())
	{
		ArFileHeader* header = (ArFileHeader*) input.data(pos);
		pos += sizeof(ArFileHeader);

		// get file size
		int size = 0;
		for (int i = 0; i < 10; i++)
		{
			if (header->fileSize[i] == ' ')
				break;

			size = size*10;
			size += (header->fileSize[i]-'0');
		}

		// only ELF files are actually interesting
		if (memcmp(input.data(pos),"\x7F""ELF",4) == 0)
		{
			// get file name
			char fileName[17];
			fileName[16] = 0;
			for (int i = 0; i < 16; i++)
			{
				if (header->fileName[i] == ' ')
				{
					// remove trailing slashes of file names
					if (i > 0 && fileName[i-1] == '/')
						i--;
					fileName[i] = 0;
					break;
				}

				fileName[i] = header->fileName[i];
			}

			ArFileEntry entry;
			entry.name = convertUtf8ToWString(fileName);
			entry.data = input.mid(pos,size);
			result.push_back(entry);
		}

		pos += size;
		if (pos % 2)
			pos++;
	}

	return result;
}

bool ElfRelocator::init(const fs::path& inputName)
{
	relocator = Arch->getElfRelocator();
	if (relocator == nullptr)
	{
		Logger::printError(Logger::Error,L"Object importing not supported for this architecture");
		return false;
	}

	auto inputFiles = loadArArchive(inputName);
	if (inputFiles.size() == 0)
	{
		Logger::printError(Logger::Error,L"Could not load library");
		return false;
	}

	for (ArFileEntry& entry: inputFiles)
	{
		ElfRelocatorFile file;

		ElfFile* elf = new ElfFile();
		if (!elf->load(entry.data,false))
		{
			Logger::printError(Logger::Error,L"Could not load object file %s",entry.name);
			return false;
		}

		if (elf->getType() != ET_REL)
		{
			Logger::printError(Logger::Error,L"Unexpected ELF type %d in object file %s",elf->getType(),entry.name);
			return false;
		}

		if (elf->getMachine() != relocator->expectedMachine())
		{
			Logger::printError(Logger::Error,L"Unexpected ELF machine %d in object file %s",elf->getMachine(),entry.name);
			return false;
		}

		if (elf->getEndianness() != Arch->getEndianness())
		{
			Logger::printError(Logger::Error,L"Incorrect endianness in object file %s",entry.name);
			return false;
		}

		if (elf->getSegmentCount() != 0)
		{
			Logger::printError(Logger::Error,L"Unexpected segment count %d in object file %s",elf->getSegmentCount(),entry.name);
			return false;
		}


		// load all relevant sections of this file
		for (size_t s = 0; s < elf->getSegmentlessSectionCount(); s++)
		{
			ElfSection* sec = elf->getSegmentlessSection(s);
			if (!(sec->getFlags() & SHF_ALLOC))
				continue;

			if (sec->getType() == SHT_PROGBITS || sec->getType() == SHT_NOBITS || sec->getType() == SHT_INIT_ARRAY)
			{
				ElfRelocatorSection sectionEntry;
				sectionEntry.section = sec;
				sectionEntry.index = s;
				sectionEntry.relSection = nullptr;
				sectionEntry.label = nullptr;

				// search relocation section
				for (size_t k = 0; k < elf->getSegmentlessSectionCount(); k++)
				{
					ElfSection* relSection = elf->getSegmentlessSection(k);
					if (relSection->getType() != SHT_REL)
						continue;
					if (relSection->getInfo() != s)
						continue;

					// got it
					sectionEntry.relSection = relSection;
					break;
				}

				// keep track of constructor sections
				if (sec->getName() == ".ctors" || sec->getName() == ".init_array")
				{
					ElfRelocatorCtor ctor;
					ctor.symbolName = Global.symbolTable.getUniqueLabelName();
					ctor.size = sec->getSize();

					sectionEntry.label = Global.symbolTable.getLabel(ctor.symbolName,-1,-1);
					sectionEntry.label->setDefined(true);

					ctors.push_back(ctor);
				}

				file.sections.push_back(sectionEntry);
			}
		}

		// init exportable symbols
		for (int i = 0; i < elf->getSymbolCount(); i++)
		{
			Elf32_Sym symbol;
			elf->getSymbol(symbol, i);

			if (ELF32_ST_BIND(symbol.st_info) == STB_GLOBAL && symbol.st_shndx != 0)
			{
				ElfRelocatorSymbol symEntry;
				symEntry.type = ELF32_ST_TYPE(symbol.st_info);
				symEntry.name = convertUtf8ToWString(elf->getStrTableString(symbol.st_name));
				symEntry.relativeAddress = symbol.st_value;
				symEntry.section = symbol.st_shndx;
				symEntry.size = symbol.st_size;
				symEntry.label = nullptr;

				file.symbols.push_back(symEntry);
			}
		}

		file.elf = elf;
		file.name = entry.name;
		files.push_back(file);
	}

	return true;
}

bool ElfRelocator::exportSymbols()
{
	bool error = false;

	for (ElfRelocatorFile& file: files)
	{
		for (ElfRelocatorSymbol& sym: file.symbols)
		{
			if (sym.label != nullptr)
				continue;

			std::wstring lowered = sym.name;
			std::transform(lowered.begin(), lowered.end(), lowered.begin(), ::towlower);

			sym.label = Global.symbolTable.getLabel(lowered,-1,-1);
			if (sym.label == nullptr)
			{
				Logger::printError(Logger::Error,L"Invalid label name \"%s\"",sym.name);
				error = true;
				continue;
			}

			if (sym.label->isDefined())
			{
				Logger::printError(Logger::Error,L"Label \"%s\" already defined",sym.name);
				error = true;
				continue;
			}

			RelocationData data;
			data.symbolAddress = sym.relativeAddress;
			relocator->setSymbolAddress(data,sym.relativeAddress,sym.type);

			sym.relativeAddress = data.symbolAddress;
			sym.label->setInfo(data.targetSymbolInfo);
			sym.label->setIsData(sym.type == STT_OBJECT);
			sym.label->setUpdateInfo(false);

			sym.label->setValue(0);
			sym.label->setDefined(true);
			sym.label->setOriginalName(sym.name);
		}
	}

	return !error;
}

std::unique_ptr<CAssemblerCommand> ElfRelocator::generateCtor(const std::wstring& ctorName)
{
	std::unique_ptr<CAssemblerCommand> content = relocator->generateCtorStub(ctors);

	auto func = std::make_unique<CDirectiveFunction>(ctorName,ctorName);
	func->setContent(std::move(content));
	return func;
}

void ElfRelocator::loadRelocation(Elf32_Rel& rel, ByteArray& data, int offset, Endianness endianness)
{
	rel.r_offset = data.getDoubleWord(offset + 0x00, endianness);
	rel.r_info   = data.getDoubleWord(offset + 0x04, endianness);
}

bool ElfRelocator::relocateFile(ElfRelocatorFile& file, int64_t& relocationAddress)
{
	ElfFile* elf = file.elf;
	int64_t start = relocationAddress;

	// calculate address for each section
	std::map<int64_t,int64_t> relocationOffsets;
	for (ElfRelocatorSection& entry: file.sections)
	{
		ElfSection* section = entry.section;
		size_t index = entry.index;
		int size = section->getSize();

		while (relocationAddress % section->getAlignment())
			relocationAddress++;

		if (entry.label != nullptr)
			entry.label->setValue(relocationAddress);

		relocationOffsets[index] = relocationAddress;
		relocationAddress += size;
	}

	size_t dataStart = outputData.size();
	outputData.reserveBytes((size_t)(relocationAddress-start));

	// load sections
	bool error = false;
	for (ElfRelocatorSection& entry: file.sections)
	{
		ElfSection* section = entry.section;
		size_t index = entry.index;

		if (section->getType() == SHT_NOBITS)
		{
			// reserveBytes initialized the data to 0 already
			continue;
		}

		ByteArray sectionData = section->getData();

		// relocate if necessary
		ElfSection* relSection = entry.relSection;
		if (relSection != nullptr)
		{
			std::vector<RelocationAction> relocationActions;
			for (unsigned int relOffset = 0; relOffset < relSection->getSize(); relOffset += sizeof(Elf32_Rel))
			{
				Elf32_Rel rel;
				loadRelocation(rel, relSection->getData(), relOffset, elf->getEndianness());
				int pos = rel.r_offset;

				if (relocator->isDummyRelocationType(rel.getType()))
					continue;

				int symNum = rel.getSymbolNum();
				if (symNum <= 0)
				{
					Logger::queueError(Logger::Warning,L"Invalid symbol num %06X",symNum);
					error = true;
					continue;
				}

				Elf32_Sym sym;
				elf->getSymbol(sym, symNum);
				int symSection = sym.st_shndx;

				RelocationData relData;
				relData.opcode = sectionData.getDoubleWord(pos, elf->getEndianness());
				relData.opcodeOffset = pos+relocationOffsets[index];
				relocator->setSymbolAddress(relData,sym.st_value,sym.st_info & 0xF);

				// externs?
				if (sym.st_shndx == 0)
				{
					if (sym.st_name == 0)
					{
						Logger::queueError(Logger::Error, L"Symbol without a name");
						error = true;
						continue;
					}

					std::wstring symName = toWLowercase(elf->getStrTableString(sym.st_name));

					std::shared_ptr<Label> label = Global.symbolTable.getLabel(symName,-1,-1);
					if (label == nullptr)
					{
						Logger::queueError(Logger::Error,L"Invalid external symbol %s",symName);
						error = true;
						continue;
					}
					if (!label->isDefined())
					{
						Logger::queueError(Logger::Error,L"Undefined external symbol %s in file %s",symName,file.name);
						error = true;
						continue;
					}

					relData.relocationBase = (unsigned int) label->getValue();
					relData.targetSymbolType = label->isData() ? STT_OBJECT : STT_FUNC;
					relData.targetSymbolInfo = label->getInfo();
				} else {
					relData.relocationBase = relocationOffsets[symSection]+relData.symbolAddress;
				}

				std::vector<std::wstring> errors;
				if (!relocator->relocateOpcode(rel.getType(),relData, relocationActions, errors))
				{
					for (const std::wstring& error : errors)
					{
						Logger::queueError(Logger::Error, error);
					}
					error = true;
					continue;
				}
			}

			// finish any dangling relocations
			std::vector<std::wstring> errors;
			if (!relocator->finish(relocationActions, errors))
			{
				for (const std::wstring& error : errors)
				{
					Logger::queueError(Logger::Error, error);
				}
				error = true;
			}

			// now actually write the relocated values
			for (const RelocationAction& action : relocationActions)
			{
				sectionData.replaceDoubleWord(action.offset-relocationOffsets[index], action.newValue, elf->getEndianness());
			}
		}

		size_t arrayStart = (size_t) (dataStart+relocationOffsets[index]-start);
		memcpy(outputData.data(arrayStart),sectionData.data(),sectionData.size());
	}

	// now update symbols
	for (ElfRelocatorSymbol& sym: file.symbols)
	{
		int64_t oldAddress = sym.relocatedAddress;

		switch (sym.section)
		{
		case SHN_ABS:		// address does not change
			sym.relocatedAddress = sym.relativeAddress;
			break;
		case SHN_COMMON:	// needs to be allocated. relativeAddress gives alignment constraint
			{
				int64_t start = relocationAddress;

				while (relocationAddress % sym.relativeAddress)
					relocationAddress++;

				sym.relocatedAddress = relocationAddress;
				relocationAddress += sym.size;
				outputData.reserveBytes((size_t)(relocationAddress-start));
			}
			break;
		default:			// normal relocated symbol
			sym.relocatedAddress = sym.relativeAddress+relocationOffsets[sym.section];
			break;
		}

		if (sym.label != nullptr)
			sym.label->setValue(sym.relocatedAddress);

		if (oldAddress != sym.relocatedAddress)
			dataChanged = true;
	}

	return !error;
}

bool ElfRelocator::relocate(int64_t& memoryAddress)
{
	int oldCrc = getCrc32(outputData.data(),outputData.size());
	outputData.clear();
	dataChanged = false;

	bool error = false;
	int64_t start = memoryAddress;

	for (ElfRelocatorFile& file: files)
	{
		if (!relocateFile(file,memoryAddress))
			error = true;
	}

	int newCrc = getCrc32(outputData.data(),outputData.size());
	if (oldCrc != newCrc)
		dataChanged = true;

	memoryAddress -= start;
	return !error;
}

void ElfRelocator::writeSymbols(SymbolData& symData) const
{
	for (const ElfRelocatorFile& file: files)
	{
		for (const ElfRelocatorSymbol& sym: file.symbols)
		{
			symData.addLabel(sym.relocatedAddress,sym.name);

			switch (sym.type)
			{
			case STT_OBJECT:
				symData.addData(sym.relocatedAddress,sym.size,SymbolData::Data8);
				break;
			case STT_FUNC:
				symData.startFunction(sym.relocatedAddress);
				symData.endFunction(sym.relocatedAddress+sym.size);
				break;
			}
		}
	}
}

std::unique_ptr<CAssemblerCommand> IElfRelocator::generateCtorStub(std::vector<ElfRelocatorCtor> &ctors)
{
	return nullptr;
}

// file: Core/Assembler.cpp


#include <thread>

void AddFileName(const std::wstring& FileName)
{
	Global.FileInfo.FileNum = (int) Global.fileList.size();
	Global.fileList.add(FileName);
	Global.FileInfo.LineNumber = 0;
}

bool encodeAssembly(std::unique_ptr<CAssemblerCommand> content, SymbolData& symData, TempData& tempData)
{
	bool Revalidate;

#ifdef ARMIPS_ARM
	Arm.Pass2();
#endif
	Mips.Pass2();

	ValidateState validation;
	do	// loop until everything is constant
	{
		Logger::clearQueue();
		Revalidate = false;

		if (validation.passes >= 100)
		{
			Logger::queueError(Logger::Error,L"Stuck in infinite validation loop");
			break;
		}

		g_fileManager->reset();
		Allocations::clearSubAreas();

#ifdef _DEBUG
		if (!Logger::isSilent())
			printf("Validate %d...\n",validation.passes);
#endif

		if (Global.memoryMode)
			g_fileManager->openFile(Global.memoryFile,true);

		Revalidate = content->Validate(validation);

#ifdef ARMIPS_ARM
		Arm.Revalidate();
#endif
		Mips.Revalidate();

		if (Global.memoryMode)
			g_fileManager->closeFile();

		validation.passes++;
	} while (Revalidate);

	Allocations::validateOverlap();

	Logger::printQueue();
	if (Logger::hasError())
	{
		return false;
	}

#ifdef _DEBUG
	if (!Logger::isSilent())
		printf("Encode...\n");
#endif

	// and finally encode
	if (Global.memoryMode)
		g_fileManager->openFile(Global.memoryFile,false);

	auto writeTempData = [&]()
	{
		tempData.start();
		if (tempData.isOpen())
			content->writeTempData(tempData);
		tempData.end();
	};

	auto writeSymData = [&]()
	{
		content->writeSymData(symData);
		symData.write();
	};

	// writeTempData, writeSymData and encode all access the same
	// memory but never change, so they can run in parallel
	if (Global.multiThreading)
	{
		std::thread tempThread(writeTempData);
		std::thread symThread(writeSymData);

		content->Encode();

		tempThread.join();
		symThread.join();
	} else {
		writeTempData();
		writeSymData();
		content->Encode();
	}

	if (g_fileManager->hasOpenFile())
	{
		if (!Global.memoryMode)
			Logger::printError(Logger::Warning,L"File not closed");
		g_fileManager->closeFile();
	}

	return true;
}

static void printStats(const AllocationStats &stats)
{
	Logger::printLine(L"Total areas and regions: %lld / %lld", stats.totalUsage, stats.totalSize);
	Logger::printLine(L"Total regions: %lld / %lld", stats.sharedUsage, stats.sharedSize);
	Logger::printLine(L"Largest area or region: 0x%08llX, %lld / %lld", stats.largestPosition, stats.largestUsage, stats.largestSize);

	int64_t startFreePosition = stats.largestFreePosition + stats.largestFreeUsage;
	Logger::printLine(L"Most free area or region: 0x%08llX, %lld / %lld (free at 0x%08llX)", stats.largestFreePosition, stats.largestFreeUsage, stats.largestFreeSize, startFreePosition);
	int64_t startSharedFreePosition = stats.sharedFreePosition + stats.sharedFreeUsage;
	Logger::printLine(L"Most free region: 0x%08llX, %lld / %lld (free at 0x%08llX)", stats.sharedFreePosition, stats.sharedFreeUsage, stats.sharedFreeSize, startSharedFreePosition);

	if (stats.totalPoolSize != 0)
	{
		Logger::printLine(L"Total pool size: %lld", stats.totalPoolSize);
		Logger::printLine(L"Largest pool: 0x%08llX, %lld", stats.largestPoolPosition, stats.largestPoolSize);
	}
}

bool runArmips(ArmipsArguments& settings)
{
	// initialize and reset global data
	Global.Section = 0;
	Global.nocash = false;
	Global.FileInfo.TotalLineCount = 0;
	Global.relativeInclude = false;
	Global.multiThreading = true;
	Arch = &InvalidArchitecture;

	Tokenizer::clearEquValues();
	Logger::clear();
	Allocations::clear();
	Global.Table.clear();
	Global.symbolTable.clear();

	Global.fileList.clear();
	Global.FileInfo.TotalLineCount = 0;
	Global.FileInfo.LineNumber = 0;
	Global.FileInfo.FileNum = 0;

#ifdef ARMIPS_ARM
	Arm.clear();
#endif

	// process settings
	Parser parser;
	SymbolData symData;
	TempData tempData;

	Logger::setSilent(settings.silent);
	Logger::setErrorOnWarning(settings.errorOnWarning);

	if (!settings.symFileName.empty())
		symData.setNocashSymFileName(settings.symFileName, settings.symFileVersion);

	if (!settings.tempFileName.empty())
		tempData.setFileName(settings.tempFileName);

	Token token;
	for (size_t i = 0; i < settings.equList.size(); i++)
	{
		parser.addEquation(token, settings.equList[i].name, settings.equList[i].value);
	}

	Global.symbolTable.addLabels(settings.labels);
	for (const LabelDefinition& label : settings.labels)
	{
		symData.addLabel(label.value, label.originalName);
	}

	if (Logger::hasError())
		return false;

	// run assembler
	TextFile input;
	switch (settings.mode)
	{
	case ArmipsMode::FILE:
		Global.memoryMode = false;
		if (!input.open(settings.inputFileName,TextFile::Read))
		{
			Logger::printError(Logger::Error,L"Could not open file");
			return false;
		}
		break;
	case ArmipsMode::MEMORY:
		Global.memoryMode = true;
		Global.memoryFile = settings.memoryFile;
		input.openMemory(settings.content);
		break;
	}

	std::unique_ptr<CAssemblerCommand> content = parser.parseFile(input);
	Logger::printQueue();

	bool result = !Logger::hasError();
	if (result && content != nullptr)
		result = encodeAssembly(std::move(content), symData, tempData);

	if (g_fileManager->hasOpenFile())
	{
		if (!Global.memoryMode)
			Logger::printError(Logger::Warning,L"File not closed");
		g_fileManager->closeFile();
	}

	// return errors
	if (settings.errorsResult != nullptr)
	{
		std::vector<std::wstring> errors = Logger::getErrors();
		for (size_t i = 0; i < errors.size(); i++)
			settings.errorsResult->push_back(errors[i]);
	}

	if (settings.showStats)
		printStats(Allocations::collectStats());

	return result;
}

// file: Core/Common.cpp


#include <sys/stat.h>

FileManager fileManager;
FileManager* g_fileManager = &fileManager;

tGlobal Global;
CArchitecture* Arch;

void FileList::add(const fs::path &path)
{
	_entries.emplace_back(path);
}

const fs::path &FileList::path(int fileIndex) const
{
	return _entries[size_t(fileIndex)].path();
}

const fs::path &FileList::relative_path(int fileIndex) const
{
	return _entries[size_t(fileIndex)].relativePath();
}

const std::wstring &FileList::wstring(int fileIndex) const
{
	return _entries[size_t(fileIndex)].wstring();
}

const std::wstring &FileList::relativeWstring(int fileIndex) const
{
	return _entries[size_t(fileIndex)].relativeWstring();
}

size_t FileList::size() const
{
	return _entries.size();
}

void FileList::clear()
{
	_entries.clear();
}

FileList::Entry::Entry(const fs::path &path) :
	_path(path),
	_relativePath(path.lexically_proximate(fs::current_path())),
	_string(_path.wstring()),
	_relativeString(_relativePath.generic_wstring())
{
}

const fs::path &FileList::Entry::path() const
{
	return _path;
}

const fs::path &FileList::Entry::relativePath() const
{
	return _relativePath;
}

const std::wstring &FileList::Entry::wstring() const
{
	return _string;
}

const std::wstring &FileList::Entry::relativeWstring() const
{
	return _relativeString;
}

fs::path getFullPathName(const fs::path& path)
{
	if (Global.relativeInclude && !path.is_absolute())
	{
		const fs::path &source = Global.fileList.path(Global.FileInfo.FileNum);
		return fs::absolute(source.parent_path() / path).lexically_normal();
	}
	else
	{
		return fs::absolute(path).lexically_normal();
	}
}

bool checkLabelDefined(const std::wstring& labelName, int section)
{
	std::shared_ptr<Label> label = Global.symbolTable.getLabel(labelName,Global.FileInfo.FileNum,section);
	return label->isDefined();
}

bool checkValidLabelName(const std::wstring& labelName)
{
	return Global.symbolTable.isValidSymbolName(labelName);
}

bool isPowerOfTwo(int64_t n)
{
	if (n == 0) return false;
	return !(n & (n - 1));
}

// file: Core/Expression.cpp


namespace
{
	std::wstring to_wstring(int64_t value)
	{
		return tfm::format(L"%d", value);
	}

	std::wstring to_wstring(double value)
	{
		return tfm::format(L"%#.17g", value);
	}
}

enum class ExpressionValueCombination
{
	II = (int(ExpressionValueType::Integer) << 2) | (int(ExpressionValueType::Integer) << 0),
	IF = (int(ExpressionValueType::Integer) << 2) | (int(ExpressionValueType::Float)   << 0),
	FI = (int(ExpressionValueType::Float)   << 2) | (int(ExpressionValueType::Integer) << 0),
	FF = (int(ExpressionValueType::Float)   << 2) | (int(ExpressionValueType::Float)   << 0),
	IS = (int(ExpressionValueType::Integer) << 2) | (int(ExpressionValueType::String)  << 0),
	FS = (int(ExpressionValueType::Float)   << 2) | (int(ExpressionValueType::String)  << 0),
	SI = (int(ExpressionValueType::String)  << 2) | (int(ExpressionValueType::Integer) << 0),
	SF = (int(ExpressionValueType::String)  << 2) | (int(ExpressionValueType::Float)   << 0),
	SS = (int(ExpressionValueType::String)  << 2) | (int(ExpressionValueType::String)  << 0),
};

ExpressionValueCombination getValueCombination(ExpressionValueType a, ExpressionValueType b)
{
	return (ExpressionValueCombination) ((int(a) << 2) | (int(b) << 0));
}

ExpressionValue ExpressionValue::operator+(const ExpressionValue& other) const
{
	ExpressionValue result;
	switch (getValueCombination(type,other.type))
	{
	case ExpressionValueCombination::II:
		result.type = ExpressionValueType::Integer;
		result.intValue = intValue + other.intValue;
		break;
	case ExpressionValueCombination::FI:
		result.type = ExpressionValueType::Float;
		result.floatValue = floatValue + other.intValue;
		break;
	case ExpressionValueCombination::IF:
		result.type = ExpressionValueType::Float;
		result.floatValue = intValue + other.floatValue;
		break;
	case ExpressionValueCombination::FF:
		result.type = ExpressionValueType::Float;
		result.floatValue = floatValue + other.floatValue;
		break;
	case ExpressionValueCombination::IS:
		result.type = ExpressionValueType::String;
		result.strValue = to_wstring(intValue) + other.strValue;
		break;
	case ExpressionValueCombination::FS:
		result.type = ExpressionValueType::String;
		result.strValue = to_wstring(floatValue) + other.strValue;
		break;
	case ExpressionValueCombination::SI:
		result.type = ExpressionValueType::String;
		result.strValue = strValue + to_wstring(other.intValue);
		break;
	case ExpressionValueCombination::SF:
		result.type = ExpressionValueType::String;
		result.strValue = strValue + to_wstring(other.floatValue);
		break;
	case ExpressionValueCombination::SS:
		result.type = ExpressionValueType::String;
		result.strValue = strValue + other.strValue;
		break;
	}

	return result;
}

ExpressionValue ExpressionValue::operator-(const ExpressionValue& other) const
{
	ExpressionValue result;
	switch (getValueCombination(type,other.type))
	{
	case ExpressionValueCombination::II:
		result.type = ExpressionValueType::Integer;
		result.intValue = intValue - other.intValue;
		break;
	case ExpressionValueCombination::FI:
		result.type = ExpressionValueType::Float;
		result.floatValue = floatValue - other.intValue;
		break;
	case ExpressionValueCombination::IF:
		result.type = ExpressionValueType::Float;
		result.floatValue = intValue - other.floatValue;
		break;
	case ExpressionValueCombination::FF:
		result.type = ExpressionValueType::Float;
		result.floatValue = floatValue - other.floatValue;
		break;
	default:
		break;
	}

	return result;
}

ExpressionValue ExpressionValue::operator*(const ExpressionValue& other) const
{
	ExpressionValue result;
	switch (getValueCombination(type,other.type))
	{
	case ExpressionValueCombination::II:
		result.type = ExpressionValueType::Integer;
		result.intValue = intValue * other.intValue;
		break;
	case ExpressionValueCombination::FI:
		result.type = ExpressionValueType::Float;
		result.floatValue = floatValue * other.intValue;
		break;
	case ExpressionValueCombination::IF:
		result.type = ExpressionValueType::Float;
		result.floatValue = intValue * other.floatValue;
		break;
	case ExpressionValueCombination::FF:
		result.type = ExpressionValueType::Float;
		result.floatValue = floatValue * other.floatValue;
		break;
	default:
		break;
	}

	return result;
}

ExpressionValue ExpressionValue::operator/(const ExpressionValue& other) const
{
	ExpressionValue result;
	switch (getValueCombination(type,other.type))
	{
	case ExpressionValueCombination::II:
		result.type = ExpressionValueType::Integer;
		if (intValue == INT64_MIN && other.intValue == -1){
			result.intValue = INT64_MIN;
			Logger::queueError(Logger::Warning,L"Division overflow in expression");
			return result;
		}
		if (other.intValue == 0)
		{
			result.intValue = ~0;
			Logger::queueError(Logger::Warning,L"Integer division by zero in expression");
			return result;
		}
		result.intValue = intValue / other.intValue;
		break;
	case ExpressionValueCombination::FI:
		result.type = ExpressionValueType::Float;
		result.floatValue = floatValue / other.intValue;
		break;
	case ExpressionValueCombination::IF:
		result.type = ExpressionValueType::Float;
		result.floatValue = intValue / other.floatValue;
		break;
	case ExpressionValueCombination::FF:
		result.type = ExpressionValueType::Float;
		result.floatValue = floatValue / other.floatValue;
		break;
	default:
		break;
	}

	return result;
}

ExpressionValue ExpressionValue::operator%(const ExpressionValue& other) const
{
	ExpressionValue result;
	switch (getValueCombination(type,other.type))
	{
	case ExpressionValueCombination::II:
		result.type = ExpressionValueType::Integer;
		if (intValue == INT64_MIN && other.intValue == -1){
			result.intValue = 0;
			Logger::queueError(Logger::Warning,L"Division overflow in expression");
			return result;
		}
		if (other.intValue == 0)
		{
			result.intValue = intValue;
			Logger::queueError(Logger::Warning,L"Integer division by zero in expression");
			return result;
		}
		result.intValue = intValue % other.intValue;
		break;
	default:
		break;
	}

	return result;
}

ExpressionValue ExpressionValue::operator!() const
{
	ExpressionValue result;
	result.type = ExpressionValueType::Integer;

	if (isFloat())
		result.intValue = !floatValue;
	else
		result.intValue = !intValue;

	return result;
}

ExpressionValue ExpressionValue::operator~() const
{
	ExpressionValue result;

	if (isInt())
	{
		result.type = ExpressionValueType::Integer;
		result.intValue = ~intValue;
	}

	return result;
}

ExpressionValue ExpressionValue::operator<<(const ExpressionValue& other) const
{
	ExpressionValue result;
	switch (getValueCombination(type,other.type))
	{
	case ExpressionValueCombination::II:
		result.type = ExpressionValueType::Integer;
		result.intValue = ((uint64_t) intValue) << other.intValue;
		break;
	default:
		break;
	}

	return result;
}

ExpressionValue ExpressionValue::operator>>(const ExpressionValue& other) const
{
	ExpressionValue result;
	switch (getValueCombination(type,other.type))
	{
	case ExpressionValueCombination::II:
		result.type = ExpressionValueType::Integer;
		result.intValue = ((uint64_t) intValue) >> other.intValue;
		break;
	default:
		break;
	}

	return result;
}

bool ExpressionValue::operator<(const ExpressionValue& other) const
{
	switch (getValueCombination(type,other.type))
	{
	case ExpressionValueCombination::II:
		return intValue < other.intValue;
	case ExpressionValueCombination::FI:
		return floatValue < other.intValue;
	case ExpressionValueCombination::IF:
		return intValue < other.floatValue;
	case ExpressionValueCombination::FF:
		return floatValue < other.floatValue;
	case ExpressionValueCombination::SS:
		return strValue < other.strValue;
	default:
		break;
	}

	return false;
}

bool ExpressionValue::operator<=(const ExpressionValue& other) const
{
	switch (getValueCombination(type,other.type))
	{
	case ExpressionValueCombination::II:
		return intValue <= other.intValue;
	case ExpressionValueCombination::FI:
		return floatValue <= other.intValue;
	case ExpressionValueCombination::IF:
		return intValue <= other.floatValue;
	case ExpressionValueCombination::FF:
		return floatValue <= other.floatValue;
	case ExpressionValueCombination::SS:
		return strValue <= other.strValue;
	default:
		break;
	}

	return false;
}

bool ExpressionValue::operator>(const ExpressionValue& other) const
{
	return other < *this;
}

bool ExpressionValue::operator>=(const ExpressionValue& other) const
{
	return other <= *this;
}

bool ExpressionValue::operator==(const ExpressionValue& other) const
{
	switch (getValueCombination(type,other.type))
	{
	case ExpressionValueCombination::II:
		return intValue == other.intValue;
	case ExpressionValueCombination::FI:
		return floatValue == other.intValue;
	case ExpressionValueCombination::IF:
		return intValue == other.floatValue;
	case ExpressionValueCombination::FF:
		return floatValue == other.floatValue;
	case ExpressionValueCombination::IS:
		return to_wstring(intValue) == other.strValue;
	case ExpressionValueCombination::FS:
		return to_wstring(floatValue) == other.strValue;
	case ExpressionValueCombination::SI:
		return strValue == to_wstring(other.intValue);
	case ExpressionValueCombination::SF:
		return strValue == to_wstring(other.floatValue);
	case ExpressionValueCombination::SS:
		return strValue == other.strValue;
	}

	return false;
}

bool ExpressionValue::operator!=(const ExpressionValue& other) const
{
	return !(*this == other);
}

ExpressionValue ExpressionValue::operator&(const ExpressionValue& other) const
{
	ExpressionValue result;
	switch (getValueCombination(type,other.type))
	{
	case ExpressionValueCombination::II:
		result.type = ExpressionValueType::Integer;
		result.intValue = intValue & other.intValue;
		break;
	default:
		break;
	}

	return result;
}

ExpressionValue ExpressionValue::operator|(const ExpressionValue& other) const
{
	ExpressionValue result;
	switch (getValueCombination(type,other.type))
	{
	case ExpressionValueCombination::II:
		result.type = ExpressionValueType::Integer;
		result.intValue = intValue | other.intValue;
		break;
	default:
		break;
	}

	return result;
}

ExpressionValue ExpressionValue::operator^(const ExpressionValue& other) const
{
	ExpressionValue result;
	switch (getValueCombination(type,other.type))
	{
	case ExpressionValueCombination::II:
		result.type = ExpressionValueType::Integer;
		result.intValue = intValue ^ other.intValue;
		break;
	default:
		break;
	}

	return result;
}

ExpressionValue ExpressionValue::operator&&(const ExpressionValue& other) const
{
	ExpressionValue result;
	result.type = ExpressionValueType::Integer;

	switch (getValueCombination(type,other.type))
	{
	case ExpressionValueCombination::II:
		result.intValue = intValue && other.intValue;
		break;
	case ExpressionValueCombination::FI:
		result.floatValue = floatValue && other.intValue;
		break;
	case ExpressionValueCombination::IF:
		result.floatValue = intValue && other.floatValue;
		break;
	case ExpressionValueCombination::FF:
		result.floatValue = floatValue && other.floatValue;
		break;
	default:
		break;
	}

	return result;
}

ExpressionValue ExpressionValue::operator||(const ExpressionValue& other) const
{
	ExpressionValue result;
	result.type = ExpressionValueType::Integer;

	switch (getValueCombination(type,other.type))
	{
	case ExpressionValueCombination::II:
		result.intValue = intValue || other.intValue;
		break;
	case ExpressionValueCombination::FI:
		result.floatValue = floatValue || other.intValue;
		break;
	case ExpressionValueCombination::IF:
		result.floatValue = intValue || other.floatValue;
		break;
	case ExpressionValueCombination::FF:
		result.floatValue = floatValue || other.floatValue;
		break;
	default:
		break;
	}

	return result;
}

ExpressionInternal::ExpressionInternal()
{
	children = nullptr;
	childrenCount = 0;
}

ExpressionInternal::~ExpressionInternal()
{
	deallocate();
}

ExpressionInternal::ExpressionInternal(int64_t value)
	: ExpressionInternal()
{
	type = OperatorType::Integer;
	intValue = value;
}

ExpressionInternal::ExpressionInternal(double value)
	: ExpressionInternal()
{
	type = OperatorType::Float;
	floatValue = value;
}

ExpressionInternal::ExpressionInternal(const std::wstring& value, OperatorType type)
	: ExpressionInternal()
{
	this->type = type;
	strValue = value;

	switch (type)
	{
	case OperatorType::Identifier:
		fileNum = Global.FileInfo.FileNum;
		section = Global.Section;
		break;
	case OperatorType::String:
		break;
	default:
		break;
	}
}

ExpressionInternal::ExpressionInternal(OperatorType op, ExpressionInternal* a,
	ExpressionInternal* b, ExpressionInternal* c)
	: ExpressionInternal()
{
	type = op;
	allocate(3);

	children[0] = a;
	children[1] = b;
	children[2] = c;
}

ExpressionInternal::ExpressionInternal(const std::wstring& name, const std::vector<ExpressionInternal*>& parameters)
	: ExpressionInternal()
{
	type = OperatorType::FunctionCall;
	allocate(parameters.size());

	strValue = name;
	for (size_t i = 0; i < parameters.size(); i++)
	{
		children[i] = parameters[i];
	}
}

void ExpressionInternal::allocate(size_t count)
{
	deallocate();

	children = new ExpressionInternal*[count];
	childrenCount = count;
}

void ExpressionInternal::deallocate()
{
	for (size_t i = 0; i < childrenCount; i++)
	{
		delete children[i];
	}

	delete[] children;
	children = nullptr;
	childrenCount = 0;
}

void ExpressionInternal::replaceMemoryPos(const std::wstring& identifierName)
{
	for (size_t i = 0; i < childrenCount; i++)
	{
		if (children[i] != nullptr)
		{
			children[i]->replaceMemoryPos(identifierName);
		}
	}

	if (type == OperatorType::MemoryPos)
	{
		type = OperatorType::Identifier;
		strValue = identifierName;
		fileNum = Global.FileInfo.FileNum;
		section = Global.Section;
	}
}

bool ExpressionInternal::checkParameterCount(size_t minParams, size_t maxParams)
{
	if (minParams > childrenCount)
	{
		Logger::queueError(Logger::Error,L"Not enough parameters for \"%s\" (min %d)",strValue,minParams);
		return false;
	}

	if (maxParams < childrenCount)
	{
		Logger::queueError(Logger::Error,L"Too many parameters for \"%s\" (min %d)",strValue,maxParams);
		return false;
	}

	return true;
}

ExpressionValue ExpressionInternal::executeExpressionFunctionCall(const ExpressionFunctionEntry& entry)
{
	// check parameters
	if (!checkParameterCount(entry.minParams, entry.maxParams))
		return {};

	// evaluate parameters
	std::vector<ExpressionValue> params;
	params.reserve(childrenCount);

	for (size_t i = 0; i < childrenCount; i++)
	{
		ExpressionValue result = children[i]->evaluate();
		if (!result.isValid())
		{
			Logger::queueError(Logger::Error,L"%s: Invalid parameter %d", strValue, i+1);
			return result;
		}

		params.push_back(result);
	}

	// execute
	return entry.function(strValue, params);
}

ExpressionValue ExpressionInternal::executeExpressionLabelFunctionCall(const ExpressionLabelFunctionEntry& entry)
{
	// check parameters
	if (!checkParameterCount(entry.minParams, entry.maxParams))
		return {};

	// evaluate parameters
	std::vector<std::shared_ptr<Label>> params;
	params.reserve(childrenCount);

	for (size_t i = 0; i < childrenCount; i++)
	{
		ExpressionInternal *exp = children[i];
		if (!exp || !exp->isIdentifier())
		{
			Logger::queueError(Logger::Error,L"%s: Invalid parameter %d, expecting identifier", strValue, i+1);
			return {};
		}

		const std::wstring& name = exp->getStringValue();
		std::shared_ptr<Label> label = Global.symbolTable.getLabel(name,exp->getFileNum(),exp->getSection());
		params.push_back(label);
	}

	// execute
	return entry.function(strValue, params);
}

ExpressionValue ExpressionInternal::executeFunctionCall()
{
	// try expression functions
	auto expFuncIt = expressionFunctions.find(strValue);
	if (expFuncIt != expressionFunctions.end())
		return executeExpressionFunctionCall(expFuncIt->second);

	// try expression label functions
	auto expLabelFuncIt = expressionLabelFunctions.find(strValue);
	if (expLabelFuncIt != expressionLabelFunctions.end())
		return executeExpressionLabelFunctionCall(expLabelFuncIt->second);

	// try architecture specific expression functions
	auto& archExpressionFunctions = Arch->getExpressionFunctions();
	expFuncIt = archExpressionFunctions.find(strValue);
	if (expFuncIt != archExpressionFunctions.end())
		return executeExpressionFunctionCall(expFuncIt->second);

	// try user defined expression functions
	auto *userFunc = UserFunctions::instance().findFunction(strValue);
	if (userFunc)
	{
		if (!checkParameterCount(userFunc->parameters.size(), userFunc->parameters.size()))
			return {};

		// evaluate parameters
		std::vector<ExpressionValue> params;
		params.reserve(childrenCount);

		for (size_t i = 0; i < childrenCount; i++)
		{
			ExpressionValue result = children[i]->evaluate();
			if (!result.isValid())
			{
				Logger::queueError(Logger::Error,L"%s: Invalid parameter %d", strValue, i+1);
				return result;
			}

			params.push_back(result);
		}

		// instantiate
		TokenStreamTokenizer tok;
		tok.init(userFunc->content);

		for (size_t i = 0; i < childrenCount; ++i)
		{
			const auto &paramName = userFunc->parameters[i];
			const auto &paramValue = params[i];

			switch (paramValue.type)
			{
			case ExpressionValueType::Float:
				tok.registerReplacementFloat(paramName, paramValue.floatValue);
				break;
			case ExpressionValueType::String:
				tok.registerReplacementString(paramName, paramValue.strValue);
				break;
			case ExpressionValueType::Integer:
				tok.registerReplacementInteger(paramName, paramValue.intValue);
				break;
			case ExpressionValueType::Invalid: // will not occur, invalid results are caught above
				break;
			}
		}

		Expression result = parseExpression(tok, false);
		if (!result.isLoaded())
		{
			Logger::queueError(Logger::Error,L"%s: Failed to parse user function expression", strValue);
			return {};
		}

		if (!tok.atEnd())
		{
			Logger::queueError(Logger::Error,L"%s: Unconsumed tokens after parsing user function expresion", strValue);
			return {};
		}

		// evaluate expression
		return result.evaluate();
	}

	// error
	Logger::queueError(Logger::Error, L"Unknown function \"%s\"", strValue);
	return {};
}

bool isExpressionFunctionSafe(const std::wstring& name, bool inUnknownOrFalseBlock)
{
	// expression functions may be unsafe, others are safe
	ExpFuncSafety safety = ExpFuncSafety::Unsafe;
	bool found = false;

	auto it = expressionFunctions.find(name);
	if (it != expressionFunctions.end())
	{
		safety = it->second.safety;
		found = true;
	}

	if (!found)
	{
		auto labelIt = expressionLabelFunctions.find(name);
		if (labelIt != expressionLabelFunctions.end())
		{
			safety = labelIt->second.safety;
			found = true;
		}
	}

	if (!found)
	{
		auto& archExpressionFunctions = Arch->getExpressionFunctions();
		it = archExpressionFunctions.find(name);
		if (it != archExpressionFunctions.end())
		{
			safety = it->second.safety;
			found = true;
		}
	}

	if (inUnknownOrFalseBlock && safety == ExpFuncSafety::ConditionalUnsafe)
		return false;

	return safety != ExpFuncSafety::Unsafe;
}

bool ExpressionInternal::simplify(bool inUnknownOrFalseBlock)
{
	// check if this expression can actually be simplified
	// without causing side effects
	switch (type)
	{
	case OperatorType::Identifier:
	case OperatorType::MemoryPos:
	case OperatorType::ToString:
		return false;
	case OperatorType::FunctionCall:
		if (!isExpressionFunctionSafe(strValue, inUnknownOrFalseBlock))
			return false;
		break;
	default:
		break;
	}

	// check if the same applies to all children
	bool canSimplify = true;
	for (size_t i = 0; i < childrenCount; i++)
	{
		if (children[i] != nullptr && !children[i]->simplify(inUnknownOrFalseBlock))
			canSimplify = false;
	}

	// if so, this expression can be evaluated into a constant
	if (canSimplify)
	{
		ExpressionValue value = evaluate();

		switch (value.type)
		{
		case ExpressionValueType::Integer:
			type = OperatorType::Integer;
			intValue = value.intValue;
			break;
		case ExpressionValueType::Float:
			type = OperatorType::Float;
			floatValue = value.floatValue;
			break;
		case ExpressionValueType::String:
			type = OperatorType::String;
			strValue = value.strValue;
			break;
		default:
			type = OperatorType::Invalid;
			break;
		}

		deallocate();
	}

	return canSimplify;
}

ExpressionValue ExpressionInternal::evaluate()
{
	ExpressionValue val;

	std::shared_ptr<Label> label;
	switch (type)
	{
	case OperatorType::Integer:
		val.type = ExpressionValueType::Integer;
		val.intValue = intValue;
		return val;
	case OperatorType::Float:
		val.type = ExpressionValueType::Float;
		val.floatValue = floatValue;
		return val;
	case OperatorType::Identifier:
		label = Global.symbolTable.getLabel(strValue,fileNum,section);
		if (label == nullptr)
		{
			Logger::queueError(Logger::Error,L"Invalid label name \"%s\"",strValue);
			return val;
		}

		if (!label->isDefined())
		{
			Logger::queueError(Logger::Error,L"Undefined label \"%s\"",label->getName());
			return val;
		}

		val.type = ExpressionValueType::Integer;
		val.intValue = label->getValue();
		return val;
	case OperatorType::String:
		val.type = ExpressionValueType::String;
		val.strValue = strValue;
		return val;
	case OperatorType::MemoryPos:
		val.type = ExpressionValueType::Integer;
		val.intValue = g_fileManager->getVirtualAddress();
		return val;
	case OperatorType::ToString:
		val.type = ExpressionValueType::String;
		val.strValue = children[0]->toString();
		return val;
	case OperatorType::Add:
		return children[0]->evaluate() + children[1]->evaluate();
	case OperatorType::Sub:
		return children[0]->evaluate() - children[1]->evaluate();
	case OperatorType::Mult:
		return children[0]->evaluate() * children[1]->evaluate();
	case OperatorType::Div:
		return children[0]->evaluate() / children[1]->evaluate();
	case OperatorType::Mod:
		return children[0]->evaluate() % children[1]->evaluate();
	case OperatorType::Neg:
		val.type = ExpressionValueType::Integer;
		val.intValue = 0;
		return val - children[0]->evaluate();
	case OperatorType::LogNot:
		return !children[0]->evaluate();
	case OperatorType::BitNot:
		return ~children[0]->evaluate();
	case OperatorType::LeftShift:
		return children[0]->evaluate() << children[1]->evaluate();
	case OperatorType::RightShift:
		return children[0]->evaluate() >> children[1]->evaluate();
	case OperatorType::Less:
		val.type = ExpressionValueType::Integer;
		val.intValue = children[0]->evaluate() < children[1]->evaluate();
		return val;
	case OperatorType::Greater:
		val.type = ExpressionValueType::Integer;
		val.intValue = children[0]->evaluate() > children[1]->evaluate();
		return val;
	case OperatorType::LessEqual:
		val.type = ExpressionValueType::Integer;
		val.intValue = children[0]->evaluate() <= children[1]->evaluate();
		return val;
	case OperatorType::GreaterEqual:
		val.type = ExpressionValueType::Integer;
		val.intValue = children[0]->evaluate() >= children[1]->evaluate();
		return val;
	case OperatorType::Equal:
		val.type = ExpressionValueType::Integer;
		val.intValue = children[0]->evaluate() == children[1]->evaluate();
		return val;
	case OperatorType::NotEqual:
		val.type = ExpressionValueType::Integer;
		val.intValue = children[0]->evaluate() != children[1]->evaluate();
		return val;
	case OperatorType::BitAnd:
		return children[0]->evaluate() & children[1]->evaluate();
	case OperatorType::BitOr:
		return children[0]->evaluate() | children[1]->evaluate();
	case OperatorType::LogAnd:
		return children[0]->evaluate() && children[1]->evaluate();
	case OperatorType::LogOr:
		return children[0]->evaluate() || children[1]->evaluate();
	case OperatorType::Xor:
		return children[0]->evaluate() ^ children[1]->evaluate();
	case OperatorType::TertiaryIf:
		val.type = ExpressionValueType::Integer;
		val.intValue = 0;
		if (children[0]->evaluate() == val)
			return children[2]->evaluate();
		else
			return children[1]->evaluate();
	case OperatorType::FunctionCall:
		return executeFunctionCall();
	default:
		return val;
	}
}

static std::wstring escapeString(const std::wstring& text)
{
	std::wstring result = text;
	replaceAll(result,LR"(\)",LR"(\\)");
	replaceAll(result,LR"(")",LR"(\")");

	return tfm::format(LR"("%s")",text);
}

std::wstring ExpressionInternal::formatFunctionCall()
{
	std::wstring text = strValue + L"(";

	for (size_t i = 0; i < childrenCount; i++)
	{
		if (i != 0)
			text += L",";
		text += children[i]->toString();
	}

	return text + L")";
}

std::wstring ExpressionInternal::toString()
{
	switch (type)
	{
	case OperatorType::Integer:
		return tfm::format(L"%d",intValue);
	case OperatorType::Float:
		return tfm::format(L"%g",floatValue);
	case OperatorType::Identifier:
		return strValue;
	case OperatorType::String:
		return escapeString(strValue);
	case OperatorType::MemoryPos:
		return L".";
	case OperatorType::Add:
		return tfm::format(L"(%s + %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Sub:
		return tfm::format(L"(%s - %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Mult:
		return tfm::format(L"(%s * %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Div:
		return tfm::format(L"(%s / %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Mod:
		return tfm::format(L"(%s %% %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Neg:
		return tfm::format(L"(-%s)",children[0]->toString());
	case OperatorType::LogNot:
		return tfm::format(L"(!%s)",children[0]->toString());
	case OperatorType::BitNot:
		return tfm::format(L"(~%s)",children[0]->toString());
	case OperatorType::LeftShift:
		return tfm::format(L"(%s << %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::RightShift:
		return tfm::format(L"(%s >> %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Less:
		return tfm::format(L"(%s < %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Greater:
		return tfm::format(L"(%s > %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::LessEqual:
		return tfm::format(L"(%s <= %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::GreaterEqual:
		return tfm::format(L"(%s >= %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Equal:
		return tfm::format(L"(%s == %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::NotEqual:
		return tfm::format(L"(%s != %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::BitAnd:
		return tfm::format(L"(%s & %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::BitOr:
		return tfm::format(L"(%s | %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::LogAnd:
		return tfm::format(L"(%s && %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::LogOr:
		return tfm::format(L"(%s || %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::Xor:
		return tfm::format(L"(%s ^ %s)",children[0]->toString(),children[1]->toString());
	case OperatorType::TertiaryIf:
		return tfm::format(L"(%s ? %s : %s)",children[0]->toString(),children[1]->toString(),children[2]->toString());
	case OperatorType::ToString:
		return tfm::format(L"(%c%s)",L'\U000000B0',children[0]->toString());
	case OperatorType::FunctionCall:
		return formatFunctionCall();
	default:
		return L"";
	}
}

Expression::Expression()
{
	expression = nullptr;
	constExpression = true;
}

void Expression::setExpression(ExpressionInternal* exp, bool inUnknownOrFalseBlock)
{
	expression = std::shared_ptr<ExpressionInternal>(exp);
	if (exp != nullptr)
		constExpression = expression->simplify(inUnknownOrFalseBlock);
	else
		constExpression = true;
}

ExpressionValue Expression::evaluate()
{
	if (expression == nullptr)
	{
		ExpressionValue invalid;
		return invalid;
	}

	return expression->evaluate();
}

void Expression::replaceMemoryPos(const std::wstring& identifierName)
{
	if (expression != nullptr)
		expression->replaceMemoryPos(identifierName);
}

bool Expression::evaluateString(std::wstring &dest, bool convert)
{
	if (expression == nullptr)
		return false;

	ExpressionValue value = expression->evaluate();
	if (convert && value.isInt())
	{
		dest = to_wstring(value.intValue);
		return true;
	}

	if (convert && value.isFloat())
	{
		dest = to_wstring(value.floatValue);
		return true;
	}

	if (!value.isString())
		return false;

	dest = value.strValue;
	return true;
}

bool Expression::evaluateIdentifier(std::wstring &dest)
{
	if (expression == nullptr || !expression->isIdentifier())
		return false;

	dest = expression->getStringValue();
	return true;
}

std::wstring Expression::toString()
{
	return expression != nullptr ? expression->toString() : L"";
}

Expression createConstExpression(int64_t value)
{
	Expression exp;
	ExpressionInternal* num = new ExpressionInternal(value);
	exp.setExpression(num,false);
	return exp;
}

// file: Core/ExpressionFunctions.cpp


#include <cmath>

#if ARMIPS_REGEXP
#include <regex>
#endif

#if defined(__clang__)
#if __has_feature(cxx_exceptions)
#define ARMIPS_EXCEPTIONS 1
#else
#define ARMIPS_EXCEPTIONS 0
#endif
#elif defined(_MSC_VER) && defined(_CPPUNWIND)
#define ARMIPS_EXCEPTIONS 1
#elif defined(__EXCEPTIONS) || defined(__cpp_exceptions)
#define ARMIPS_EXCEPTIONS 1
#else
#define ARMIPS_EXCEPTIONS 0
#endif

bool getExpFuncParameter(const std::vector<ExpressionValue>& parameters, size_t index, int64_t& dest,
	const std::wstring& funcName, bool optional)
{
	if (optional && index >= parameters.size())
		return true;

	if (index >= parameters.size() || !parameters[index].isInt())
	{
		Logger::queueError(Logger::Error,L"Invalid parameter %d for %s: expecting integer",index+1,funcName);
		return false;
	}

	dest = parameters[index].intValue;
	return true;
}

bool getExpFuncParameter(const std::vector<ExpressionValue>& parameters, size_t index, const std::wstring*& dest,
	const std::wstring& funcName, bool optional)
{
	if (optional && index >= parameters.size())
		return true;

	if (index >= parameters.size() || !parameters[index].isString())
	{
		Logger::queueError(Logger::Error,L"Invalid parameter %d for %s: expecting string",index+1,funcName);
		return false;
	}

	dest = &parameters[index].strValue;
	return true;
}

#define GET_PARAM(params,index,dest) \
	if (getExpFuncParameter(params,index,dest,funcName,false) == false) \
		return ExpressionValue();
#define GET_OPTIONAL_PARAM(params,index,dest,defaultValue) \
	dest = defaultValue; \
	if (getExpFuncParameter(params,index,dest,funcName,true) == false) \
		return ExpressionValue();


ExpressionValue expFuncVersion(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	int64_t value = ARMIPS_VERSION_MAJOR*1000 + ARMIPS_VERSION_MINOR*10 + ARMIPS_VERSION_REVISION;
	return ExpressionValue(value);
}

ExpressionValue expFuncEndianness(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	ExpressionValue result;
	result.type = ExpressionValueType::String;

	switch (g_fileManager->getEndianness())
	{
	case Endianness::Little:
		return ExpressionValue(L"little");
	case Endianness::Big:
		return ExpressionValue(L"big");
	}

	return ExpressionValue();
}

ExpressionValue expFuncOutputName(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	std::shared_ptr<AssemblerFile> file = g_fileManager->getOpenFile();
	if (file == nullptr)
	{
		Logger::queueError(Logger::Error,L"outputName: no file opened");
		return ExpressionValue();
	}

	std::wstring value = file->getFileName().wstring();
	return ExpressionValue(value);
}

ExpressionValue expFuncFileExists(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	const std::wstring* fileName;
	GET_PARAM(parameters,0,fileName);

	auto fullName = getFullPathName(*fileName);
	return ExpressionValue(fs::exists(fullName) ? INT64_C(1) : INT64_C(0));
}

ExpressionValue expFuncFileSize(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	const std::wstring* fileName;
	GET_PARAM(parameters,0,fileName);

	auto fullName = getFullPathName(*fileName);

	std::error_code error;
	return ExpressionValue(static_cast<int64_t>(fs::file_size(fullName, error)));
}

ExpressionValue expFuncToString(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	ExpressionValue result;

	switch (parameters[0].type)
	{
	case ExpressionValueType::String:
		result.strValue = parameters[0].strValue;
		break;
	case ExpressionValueType::Integer:
		result.strValue = tfm::format(L"%d",parameters[0].intValue);
		break;
	case ExpressionValueType::Float:
		result.strValue = tfm::format(L"%#.17g",parameters[0].floatValue);
		break;
	default:
		return result;
	}

	result.type = ExpressionValueType::String;
	return result;
}

ExpressionValue expFuncToHex(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	int64_t value, digits;
	GET_PARAM(parameters,0,value);
	GET_OPTIONAL_PARAM(parameters,1,digits,8);

	return ExpressionValue(tfm::format(L"%0*X",digits,value));
}

ExpressionValue expFuncInt(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	ExpressionValue result;

	switch (parameters[0].type)
	{
	case ExpressionValueType::Integer:
		result.intValue = parameters[0].intValue;
		break;
	case ExpressionValueType::Float:
		result.intValue = (int64_t) parameters[0].floatValue;
		break;
	default:
		return result;
	}

	result.type = ExpressionValueType::Integer;
	return result;
}

ExpressionValue expFuncRound(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	ExpressionValue result;

	switch (parameters[0].type)
	{
	case ExpressionValueType::Integer:
		result.intValue = parameters[0].intValue;
		break;
	case ExpressionValueType::Float:
		result.intValue = llround(parameters[0].floatValue);
		break;
	default:
		return result;
	}

	result.type = ExpressionValueType::Integer;
	return result;
}

ExpressionValue expFuncFloat(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	ExpressionValue result;

	switch (parameters[0].type)
	{
	case ExpressionValueType::Integer:
		result.floatValue = (double) parameters[0].intValue;
		break;
	case ExpressionValueType::Float:
		result.floatValue = parameters[0].floatValue;
		break;
	default:
		return result;
	}

	result.type = ExpressionValueType::Float;
	return result;
}

ExpressionValue expFuncFrac(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	ExpressionValue result;
	double intPart;

	switch (parameters[0].type)
	{
	case ExpressionValueType::Float:
		result.floatValue = modf(parameters[0].floatValue,&intPart);
		break;
	default:
		return result;
	}

	result.type = ExpressionValueType::Float;
	return result;
}

ExpressionValue expFuncMin(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	ExpressionValue result;
	double floatMin, floatCur;
	int64_t intMin, intCur;

	floatCur = floatMin = std::numeric_limits<double>::max();
	intCur = intMin = std::numeric_limits<int64_t>::max();
	bool isInt = true;

	for (size_t i = 0; i < parameters.size(); i++)
	{
		switch (parameters[i].type)
		{
		case ExpressionValueType::Integer:
			intCur = parameters[i].intValue;
			floatCur = (double)parameters[i].intValue;
			break;
		case ExpressionValueType::Float:
			floatCur = parameters[i].floatValue;
			isInt = false;
			break;
		default:
			return result;
		}

		if (intCur < intMin)
			intMin = intCur;
		if (floatCur < floatMin)
			floatMin = floatCur;
	}

	if (isInt)
	{
		result.intValue = intMin;
		result.type = ExpressionValueType::Integer;
	}
	else
	{
		result.floatValue = floatMin;
		result.type = ExpressionValueType::Float;
	}

	return result;
}

ExpressionValue expFuncMax(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	ExpressionValue result;
	double floatMax, floatCur;
	int64_t intMax, intCur;

	floatCur = floatMax = std::numeric_limits<double>::min();
	intCur = intMax = std::numeric_limits<int64_t>::min();
	bool isInt = true;

	for (size_t i = 0; i < parameters.size(); i++)
	{
		switch (parameters[i].type)
		{
		case ExpressionValueType::Integer:
			intCur = parameters[i].intValue;
			floatCur = (double)parameters[i].intValue;
			break;
		case ExpressionValueType::Float:
			floatCur = parameters[i].floatValue;
			isInt = false;
			break;
		default:
			return result;
		}

		if (intCur > intMax)
			intMax = intCur;
		if (floatCur > floatMax)
			floatMax = floatCur;
	}

	if (isInt)
	{
		result.intValue = intMax;
		result.type = ExpressionValueType::Integer;
	}
	else
	{
		result.floatValue = floatMax;
		result.type = ExpressionValueType::Float;
	}

	return result;
}

ExpressionValue expFuncAbs(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	ExpressionValue result;

	switch (parameters[0].type)
	{
	case ExpressionValueType::Float:
		result.type = ExpressionValueType::Float;
		result.floatValue = fabs(parameters[0].floatValue);
		break;
	case ExpressionValueType::Integer:
		result.type = ExpressionValueType::Integer;
		result.intValue = parameters[0].intValue >= 0 ?
			parameters[0].intValue : -parameters[0].intValue;
		break;
	default:
		break;
	}

	return result;
}

ExpressionValue expFuncStrlen(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	const std::wstring* source;
	GET_PARAM(parameters,0,source);

	return ExpressionValue((int64_t)source->size());
}

ExpressionValue expFuncSubstr(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	int64_t start, count;
	const std::wstring* source;

	GET_PARAM(parameters,0,source);
	GET_PARAM(parameters,1,start);
	GET_PARAM(parameters,2,count);

	return ExpressionValue(source->substr((size_t)start,(size_t)count));
}

#if ARMIPS_REGEXP
ExpressionValue expFuncRegExMatch(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	const std::wstring* source;
	const std::wstring* regexString;

	GET_PARAM(parameters,0,source);
	GET_PARAM(parameters,1,regexString);

#if ARMIPS_EXCEPTIONS
	try
	{
#endif
		std::wregex regex(*regexString);
		bool found = std::regex_match(*source,regex);
		return ExpressionValue(found ? INT64_C(1) : INT64_C(0));
#if ARMIPS_EXCEPTIONS
	} catch (std::regex_error&)
	{
		Logger::queueError(Logger::Error,L"Invalid regular expression");
		return ExpressionValue();
	}
#endif
}

ExpressionValue expFuncRegExSearch(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	const std::wstring* source;
	const std::wstring* regexString;

	GET_PARAM(parameters,0,source);
	GET_PARAM(parameters,1,regexString);

#if ARMIPS_EXCEPTIONS
	try
	{
#endif
		std::wregex regex(*regexString);
		bool found = std::regex_search(*source,regex);
		return ExpressionValue(found ? INT64_C(1) : INT64_C(0));
#if ARMIPS_EXCEPTIONS
	} catch (std::regex_error&)
	{
		Logger::queueError(Logger::Error,L"Invalid regular expression");
		return ExpressionValue();
	}
#endif
}

ExpressionValue expFuncRegExExtract(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	const std::wstring* source;
	const std::wstring* regexString;
	int64_t matchIndex;

	GET_PARAM(parameters,0,source);
	GET_PARAM(parameters,1,regexString);
	GET_OPTIONAL_PARAM(parameters,2,matchIndex,0);

#if ARMIPS_EXCEPTIONS
	try
	{
#endif
		std::wregex regex(*regexString);
		std::wsmatch result;
		bool found = std::regex_search(*source,result,regex);
		if (!found || (size_t)matchIndex >= result.size())
		{
			Logger::queueError(Logger::Error,L"Capture group index %d does not exist",matchIndex);
			return ExpressionValue();
		}

		return ExpressionValue(result[(size_t)matchIndex].str());
#if ARMIPS_EXCEPTIONS
	} catch (std::regex_error&)
	{
		Logger::queueError(Logger::Error,L"Invalid regular expression");
		return ExpressionValue();
	}
#endif
}
#endif

ExpressionValue expFuncFind(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	int64_t start;
	const std::wstring* source;
	const std::wstring* value;

	GET_PARAM(parameters,0,source);
	GET_PARAM(parameters,1,value);
	GET_OPTIONAL_PARAM(parameters,2,start,0);

	size_t pos = source->find(*value,(size_t)start);
	return ExpressionValue(pos == std::wstring::npos ? INT64_C(-1) : (int64_t) pos);
}

ExpressionValue expFuncRFind(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	int64_t start;
	const std::wstring* source;
	const std::wstring* value;

	GET_PARAM(parameters,0,source);
	GET_PARAM(parameters,1,value);
	GET_OPTIONAL_PARAM(parameters,2,start,std::wstring::npos);

	size_t pos = source->rfind(*value,(size_t)start);
	return ExpressionValue(pos == std::wstring::npos ? INT64_C(-1) : (int64_t) pos);
}


template<typename T>
ExpressionValue expFuncRead(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	const std::wstring* fileName;
	int64_t pos;

	GET_PARAM(parameters,0,fileName);
	GET_OPTIONAL_PARAM(parameters,1,pos,0);

	auto fullName = getFullPathName(*fileName);

	fs::ifstream file(fullName, fs::ifstream::in | fs::ifstream::binary);
	if (!file.is_open())
	{
		Logger::queueError(Logger::Error, L"Could not open %s",*fileName);
		return ExpressionValue();
	}

	file.seekg(pos);
	if (file.eof() || file.fail())
	{
		Logger::queueError(Logger::Error, L"Invalid offset 0x%08X of %s", pos, *fileName);
		return ExpressionValue();
	}

	T buffer;
	file.read(reinterpret_cast<char*>(&buffer), sizeof(T));

	if (file.fail())
	{
		Logger::queueError(Logger::Error, L"Failed to read %d byte(s) from offset 0x%08X of %s", sizeof(T), pos, *fileName);
		return ExpressionValue();
	}

	return ExpressionValue((int64_t) buffer);
}

ExpressionValue expFuncReadAscii(const std::wstring& funcName, const std::vector<ExpressionValue>& parameters)
{
	const std::wstring* fileName;
	int64_t start;
	int64_t length;

	GET_PARAM(parameters,0,fileName);
	GET_OPTIONAL_PARAM(parameters,1,start,0);
	GET_OPTIONAL_PARAM(parameters,2,length,0);

	auto fullName = getFullPathName(*fileName);

	std::error_code error;
	int64_t totalSize = static_cast<int64_t>(fs::file_size(fullName, error));

	if (length == 0 || start+length > totalSize)
		length = totalSize-start;

	fs::ifstream file(fullName, fs::ifstream::in | fs::ifstream::binary);
	if (!file.is_open())
	{
		Logger::queueError(Logger::Error, L"Could not open %s",*fileName);
		return ExpressionValue();
	}

	file.seekg(start);
	if (file.eof() || file.fail())
	{
		Logger::queueError(Logger::Error, L"Invalid offset 0x%08X of %s", start, *fileName);
		return ExpressionValue();
	}

	char buffer[1024];
	bool stringTerminated = false;
	std::wstring result;

	for (int64_t progress = 0; !stringTerminated && progress < length; progress += (int64_t) sizeof(buffer))
	{
		auto bytesToRead = (size_t) std::min((int64_t) sizeof(buffer), length - progress);

		file.read(buffer, bytesToRead);
		if (file.fail())
		{
			Logger::queueError(Logger::Error, L"Failed to read %d byte(s) from offset 0x%08X of %s", bytesToRead, *fileName);
			return ExpressionValue();
		}

		for (auto i = 0; i < file.gcount(); i++)
		{
			if (buffer[i] == 0x00)
			{
				stringTerminated = true;
				break;
			}

			if (buffer[i] < 0x20)
			{
				Logger::printError(Logger::Warning, L"%s: Non-ASCII character", funcName);
				return ExpressionValue();
			}

			result += (wchar_t) buffer[i];
		}
	}

	return ExpressionValue(result);
}

ExpressionValue expLabelFuncDefined(const std::wstring &funcName, const std::vector<std::shared_ptr<Label>> &parameters)
{
	if (parameters.empty() || !parameters.front())
	{
		Logger::queueError(Logger::Error,L"%s: Invalid parameters", funcName);
		return ExpressionValue();
	}

	return ExpressionValue(parameters.front()->isDefined() ? INT64_C(1) : INT64_C(0));
}

ExpressionValue expLabelFuncOrg(const std::wstring& funcName, const std::vector<std::shared_ptr<Label>>& parameters)
{
	// return physical address of label parameter
	if (parameters.size())
	{
		Label* label = parameters.front().get();
		if (!label)
			return ExpressionValue();

		return ExpressionValue(parameters.front()->getValue());
	}

	if(!g_fileManager->hasOpenFile())
	{
		Logger::queueError(Logger::Error,L"%s: no file opened", funcName);
		return ExpressionValue();
	}
	return ExpressionValue(g_fileManager->getVirtualAddress());
}

ExpressionValue expLabelFuncOrga(const std::wstring& funcName, const std::vector<std::shared_ptr<Label>>& parameters)
{
	// return physical address of label parameter
	if (parameters.size())
	{
		Label* label = parameters.front().get();
		if (!label)
			return ExpressionValue();

		if (!label->hasPhysicalValue())
		{
			Logger::queueError(Logger::Error,L"%s: parameter %s has no physical address", funcName, label->getName() );
			return ExpressionValue();
		}

		return ExpressionValue(parameters.front()->getPhysicalValue());
	}

	// return current physical address otherwise
	if(!g_fileManager->hasOpenFile())
	{
		Logger::queueError(Logger::Error,L"%s: no file opened", funcName);
		return ExpressionValue();
	}
	return ExpressionValue(g_fileManager->getPhysicalAddress());
}

ExpressionValue expLabelFuncHeaderSize(const std::wstring& funcName, const std::vector<std::shared_ptr<Label>>& parameters)
{
	// return difference between physical and virtual address of label parameter
	if (parameters.size())
	{
		Label* label = parameters.front().get();
		if (!label)
			return ExpressionValue();

		if (!label->hasPhysicalValue())
		{
			Logger::queueError(Logger::Error,L"%s: parameter %s has no physical address", funcName, label->getName() );
			return ExpressionValue();
		}

		return ExpressionValue(label->getValue() - label->getPhysicalValue());
	}

	if(!g_fileManager->hasOpenFile())
	{
		Logger::queueError(Logger::Error,L"headersize: no file opened");
		return ExpressionValue();
	}
	return ExpressionValue(g_fileManager->getHeaderSize());
}

const ExpressionFunctionMap expressionFunctions = {
	{ L"version",		{ &expFuncVersion,			0,	0,	ExpFuncSafety::Safe } },
	{ L"endianness",	{ &expFuncEndianness,		0,	0,	ExpFuncSafety::Unsafe } },
	{ L"outputname",	{ &expFuncOutputName,		0,	0,	ExpFuncSafety::Unsafe } },
	{ L"fileexists",	{ &expFuncFileExists,		1,	1,	ExpFuncSafety::Safe } },
	{ L"filesize",		{ &expFuncFileSize,			1,	1,	ExpFuncSafety::ConditionalUnsafe } },
	{ L"tostring",		{ &expFuncToString,			1,	1,	ExpFuncSafety::Safe } },
	{ L"tohex",			{ &expFuncToHex,			1,	2,	ExpFuncSafety::Safe } },

	{ L"int",			{ &expFuncInt,				1,	1,	ExpFuncSafety::Safe } },
	{ L"float",			{ &expFuncFloat,			1,	1,	ExpFuncSafety::Safe } },
	{ L"frac",			{ &expFuncFrac,				1,	1,	ExpFuncSafety::Safe } },
	{ L"abs",			{ &expFuncAbs,				1,	1,	ExpFuncSafety::Safe } },
	{ L"round",			{ &expFuncRound,			1,	1,	ExpFuncSafety::Safe } },
	{ L"min",			{ &expFuncMin,				1,	std::numeric_limits<size_t>::max(),	ExpFuncSafety::Safe } },
	{ L"max",			{ &expFuncMax,				1,	std::numeric_limits<size_t>::max(),	ExpFuncSafety::Safe } },

	{ L"strlen",		{ &expFuncStrlen,			1,	1,	ExpFuncSafety::Safe } },
	{ L"substr",		{ &expFuncSubstr,			3,	3,	ExpFuncSafety::Safe } },
#if ARMIPS_REGEXP
	{ L"regex_match",	{ &expFuncRegExMatch,		2,	2,	ExpFuncSafety::Safe } },
	{ L"regex_search",	{ &expFuncRegExSearch,		2,	2,	ExpFuncSafety::Safe } },
	{ L"regex_extract",	{ &expFuncRegExExtract,		2,	3,	ExpFuncSafety::Safe } },
#endif
	{ L"find",			{ &expFuncFind,				2,	3,	ExpFuncSafety::Safe } },
	{ L"rfind",			{ &expFuncRFind,			2,	3,	ExpFuncSafety::Safe } },

	{ L"readbyte",		{ &expFuncRead<uint8_t>,	1,	2,	ExpFuncSafety::ConditionalUnsafe } },
	{ L"readu8",		{ &expFuncRead<uint8_t>,	1,	2,	ExpFuncSafety::ConditionalUnsafe } },
	{ L"readu16",		{ &expFuncRead<uint16_t>,	1,	2,	ExpFuncSafety::ConditionalUnsafe } },
	{ L"readu32",		{ &expFuncRead<uint32_t>,	1,	2,	ExpFuncSafety::ConditionalUnsafe } },
	{ L"readu64",		{ &expFuncRead<uint64_t>,	1,	2,	ExpFuncSafety::ConditionalUnsafe } },
	{ L"reads8",		{ &expFuncRead<int8_t>,		1,	2,	ExpFuncSafety::ConditionalUnsafe } },
	{ L"reads16",		{ &expFuncRead<int16_t>,	1,	2,	ExpFuncSafety::ConditionalUnsafe } },
	{ L"reads32",		{ &expFuncRead<int32_t>,	1,	2,	ExpFuncSafety::ConditionalUnsafe } },
	{ L"reads64",		{ &expFuncRead<int64_t>,	1,	2,	ExpFuncSafety::ConditionalUnsafe } },
	{ L"readascii",		{ &expFuncReadAscii,		1,	3,	ExpFuncSafety::ConditionalUnsafe } },
};

extern const ExpressionLabelFunctionMap expressionLabelFunctions =
{
	{ L"defined",    { &expLabelFuncDefined,      1, 1, ExpFuncSafety::Unsafe } },
	{ L"org",        { &expLabelFuncOrg,          0, 1, ExpFuncSafety::Unsafe } },
	{ L"orga",       { &expLabelFuncOrga,         0, 1, ExpFuncSafety::Unsafe } },
	{ L"headersize", { &expLabelFuncHeaderSize,   0, 1, ExpFuncSafety::Unsafe } },
};

// file: Core/FileManager.cpp


inline uint64_t swapEndianness64(uint64_t value)
{
	return ((value & 0xFF) << 56) | ((value & 0xFF00) << 40) | ((value & 0xFF0000) << 24) | ((value & 0xFF000000) << 8) |
	((value & 0xFF00000000) >> 8) | ((value & 0xFF0000000000) >> 24) |
	((value & 0xFF000000000000) >> 40) | ((value & 0xFF00000000000000) >> 56);
}

inline uint32_t swapEndianness32(uint32_t value)
{
	return ((value & 0xFF) << 24) | ((value & 0xFF00) << 8) | ((value & 0xFF0000) >> 8) | ((value & 0xFF000000) >> 24);
}

inline uint16_t swapEndianness16(uint16_t value)
{
	return ((value & 0xFF) << 8) | ((value & 0xFF00) >> 8);
}


GenericAssemblerFile::GenericAssemblerFile(const fs::path& fileName, int64_t headerSize, bool overwrite)
{
	this->fileName = fileName;
	this->headerSize = headerSize;
	this->originalHeaderSize = headerSize;
	this->seekPhysical(0);
	mode = overwrite ? Create : Open;
}

GenericAssemblerFile::GenericAssemblerFile(const fs::path& fileName, const fs::path& originalFileName, int64_t headerSize)
{
	this->fileName = fileName;
	this->originalName = originalFileName;
	this->headerSize = headerSize;
	this->originalHeaderSize = headerSize;
	this->seekPhysical(0);
	mode = Copy;
}

bool GenericAssemblerFile::open(bool onlyCheck)
{
	std::error_code errorCode;

	headerSize = originalHeaderSize;
	virtualAddress = headerSize;

	auto flagsOpenExisting = fs::ofstream::in | fs::ofstream::out | fs::ofstream::binary;
	auto flagsOverwrite = fs::ofstream::out | fs::ofstream::trunc | fs::ofstream::binary;

	if (!onlyCheck)
	{
		// actually open the file
		switch (mode)
		{
		case Open:
			stream.open(fileName, flagsOpenExisting);
			if (!stream.is_open())
			{
				Logger::printError(Logger::FatalError,L"Could not open file %s",fileName.wstring());
				return false;
			}
			return true;

		case Create:
			stream.open(fileName, flagsOverwrite);
			if (!stream.is_open())
			{
				Logger::printError(Logger::FatalError,L"Could not create file %s",fileName.wstring());
				return false;
			}
			return true;

		case Copy:
			if (!fs::copy_file(originalName, fileName, fs::copy_options::overwrite_existing, errorCode))
			{
				Logger::printError(Logger::FatalError,L"Could not copy file %s",originalName.wstring());
				return false;
			}

			stream.open(fileName, flagsOpenExisting);
			if (!stream.is_open())
			{
				Logger::printError(Logger::FatalError,L"Could not create file %s",fileName.wstring());
				return false;
			}
			return true;
		}
	}

	// else only check if it can be done, don't actually do it permanently
	bool exists = false;
	fs::ofstream temp;
	switch (mode)
	{
	case Open:
		temp.open(fileName, flagsOpenExisting);
		if (!temp.is_open())
		{
			Logger::queueError(Logger::FatalError,L"Could not open file %s",fileName.wstring());
			return false;
		}
		temp.close();
		return true;

	case Create:
		// open file with writee access. if it didn't exist before, remove it afterwards
		exists = fs::exists(fileName);

		temp.open(fileName, exists ? flagsOpenExisting : flagsOverwrite);
		if (!temp.is_open())
		{
			Logger::queueError(Logger::FatalError,L"Could not create file %s",fileName.wstring());
			return false;
		}
		temp.close();

		if (!exists)
			fs::remove(fileName, errorCode);

		return true;

	case Copy:
		// check original file
		temp.open(originalName, flagsOpenExisting);
		if (!temp.is_open())
		{
			Logger::queueError(Logger::FatalError,L"Could not open file %s",originalName.wstring());
			return false;
		}
		temp.close();

		// check new file, same as create
		exists = fs::exists(fileName);

		temp.open(fileName, exists ? flagsOpenExisting : flagsOverwrite);
		if (!temp.is_open())
		{
			Logger::queueError(Logger::FatalError,L"Could not create file %s",fileName.wstring());
			return false;
		}
		temp.close();

		if (!exists)
			fs::remove(fileName, errorCode);
		return true;
	}

	return false;
}

bool GenericAssemblerFile::write(void* data, size_t length)
{
	if (!isOpen())
		return false;

	stream.write(reinterpret_cast<const char *>( data ), length);
	virtualAddress += length;
	return !stream.fail();
}

bool GenericAssemblerFile::seekVirtual(int64_t virtualAddress)
{
	if (virtualAddress - headerSize < 0)
	{
		Logger::queueError(Logger::Error,L"Seeking to virtual address with negative physical address");
		return false;
	}
	if (virtualAddress < 0)
		Logger::queueError(Logger::Warning,L"Seeking to negative virtual address");

	this->virtualAddress = virtualAddress;
	int64_t physicalAddress = virtualAddress-headerSize;

	if (isOpen())
		stream.seekp(physicalAddress);

	return true;
}

bool GenericAssemblerFile::seekPhysical(int64_t physicalAddress)
{
	if (physicalAddress < 0)
	{
		Logger::queueError(Logger::Error,L"Seeking to negative physical address");
		return false;
	}
	if (physicalAddress + headerSize < 0)
		Logger::queueError(Logger::Warning,L"Seeking to physical address with negative virtual address");

	virtualAddress = physicalAddress+headerSize;

	if (isOpen())
		stream.seekp(physicalAddress);

	return true;
}



FileManager::FileManager()
{
	// detect own endianness
	volatile union
	{
		uint32_t i;
		uint8_t c[4];
	} u;
	u.c[3] = 0xAA;
	u.c[2] = 0xBB;
	u.c[1] = 0xCC;
	u.c[0] = 0xDD;

	if (u.i == 0xDDCCBBAA)
		ownEndianness = Endianness::Big;
	else if (u.i == 0xAABBCCDD)
		ownEndianness = Endianness::Little;
	else
		Logger::printError(Logger::Error,L"Running on unknown endianness");

	reset();
}

FileManager::~FileManager()
{

}

void FileManager::reset()
{
	activeFile = nullptr;
	setEndianness(Endianness::Little);
}

bool FileManager::checkActiveFile()
{
	if (activeFile == nullptr)
	{
		Logger::queueError(Logger::Error,L"No file opened");
		return false;
	}
	return true;
}

bool FileManager::openFile(std::shared_ptr<AssemblerFile> file, bool onlyCheck)
{
	if (activeFile != nullptr)
	{
		Logger::queueError(Logger::Warning,L"File not closed before opening a new one");
		activeFile->close();
	}

	activeFile = file;
	return activeFile->open(onlyCheck);
}

void FileManager::addFile(std::shared_ptr<AssemblerFile> file)
{
	files.push_back(file);
}

void FileManager::closeFile()
{
	if (activeFile == nullptr)
	{
		Logger::queueError(Logger::Warning,L"No file opened");
		return;
	}

	activeFile->close();
	activeFile = nullptr;
}

bool FileManager::write(void* data, size_t length)
{
	if (!checkActiveFile())
		return false;

	if (!activeFile->isOpen())
	{
		Logger::queueError(Logger::Error,L"No file opened");
		return false;
	}

	return activeFile->write(data,length);
}

bool FileManager::writeU8(uint8_t data)
{
	return write(&data,1);
}

bool FileManager::writeU16(uint16_t data)
{
	if (endianness != ownEndianness)
		data = swapEndianness16(data);

	return write(&data,2);
}

bool FileManager::writeU32(uint32_t data)
{
	if (endianness != ownEndianness)
		data = swapEndianness32(data);

	return write(&data,4);
}

bool FileManager::writeU64(uint64_t data)
{
	if (endianness != ownEndianness)
		data = swapEndianness64(data);

	return write(&data,8);
}

int64_t FileManager::getVirtualAddress()
{
	if (activeFile == nullptr)
		return -1;
	return activeFile->getVirtualAddress();
}

int64_t FileManager::getPhysicalAddress()
{
	if (activeFile == nullptr)
		return -1;
	return activeFile->getPhysicalAddress();
}

int64_t FileManager::getHeaderSize()
{
	if (activeFile == nullptr)
		return -1;
	return activeFile->getHeaderSize();
}

bool FileManager::seekVirtual(int64_t virtualAddress)
{
	if (!checkActiveFile())
		return false;

	bool result = activeFile->seekVirtual(virtualAddress);
	if (result && Global.memoryMode)
	{
		int sec = Global.symbolTable.findSection(virtualAddress);
		if (sec != -1)
			Global.Section = sec;
	}

	return result;
}

bool FileManager::seekPhysical(int64_t virtualAddress)
{
	if (!checkActiveFile())
		return false;
	return activeFile->seekPhysical(virtualAddress);
}

bool FileManager::advanceMemory(size_t bytes)
{
	if (!checkActiveFile())
		return false;

	int64_t pos = activeFile->getVirtualAddress();
	return activeFile->seekVirtual(pos+bytes);
}

int64_t FileManager::getOpenFileID()
{
	if (!checkActiveFile())
		return 0;

	static_assert(sizeof(int64_t) >= sizeof(intptr_t), "Assumes pointers are <= 64 bit");
	return (int64_t)(intptr_t)activeFile.get();
}

// file: Core/Misc.cpp


#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

std::vector<Logger::QueueEntry> Logger::queue;
std::vector<std::wstring> Logger::errors;
bool Logger::error = false;
bool Logger::fatalError = false;
bool Logger::errorOnWarning = false;
bool Logger::silent = false;
int Logger::suppressLevel = 0;

std::wstring Logger::formatError(ErrorType type, const wchar_t* text)
{
	std::wstring position;

	if (!Global.memoryMode && Global.fileList.size() > 0)
	{
		const auto& fileName = Global.fileList.relativeWstring(Global.FileInfo.FileNum);
		position = tfm::format(L"%s(%d) ", fileName, Global.FileInfo.LineNumber);
	}

	switch (type)
	{
	case Warning:
		return tfm::format(L"%swarning: %s",position,text);
	case Error:
		return tfm::format(L"%serror: %s",position,text);
	case FatalError:
		return tfm::format(L"%sfatal error: %s",position,text);
	case Notice:
		return tfm::format(L"%snotice: %s",position,text);
	}

	return L"";
}

void Logger::setFlags(ErrorType type)
{
	switch (type)
	{
	case Warning:
		if (errorOnWarning)
			error = true;
		break;
	case Error:
		error = true;
		break;
	case FatalError:
		error = true;
		fatalError = true;
		break;
	case Notice:
		break;
	}
}

void Logger::clear()
{
	queue.clear();
	errors.clear();
	error = false;
	fatalError = false;
	errorOnWarning = false;
	silent = false;
}

void Logger::printLine(const std::wstring& text)
{
	if (suppressLevel)
		return;

	std::wcout << text << std::endl;

#if defined(_MSC_VER) && defined(_DEBUG)
	OutputDebugStringW(text.c_str());
	OutputDebugStringW(L"\n");
#endif
}

void Logger::printLine(const std::string& text)
{
	if (suppressLevel)
		return;

	std::cout << text << std::endl;

#if defined(_MSC_VER) && defined(_DEBUG)
	OutputDebugStringA(text.c_str());
	OutputDebugStringA("\n");
#endif
}

void Logger::print(const std::wstring& text)
{
	if (suppressLevel)
		return;

	std::wcout << text;

#if defined(_MSC_VER) && defined(_DEBUG)
	OutputDebugStringW(text.c_str());
#endif
}

void Logger::printError(ErrorType type, const std::wstring& text)
{
	if (suppressLevel)
		return;

	std::wstring errorText = formatError(type,text.c_str());
	errors.push_back(errorText);

	if (!silent)
		printLine(errorText);

	setFlags(type);
}

void Logger::printError(ErrorType type, const wchar_t* text)
{
	if (suppressLevel)
		return;

	std::wstring errorText = formatError(type,text);
	errors.push_back(errorText);

	if (!silent)
		printLine(errorText);

	setFlags(type);
}

void Logger::queueError(ErrorType type, const std::wstring& text)
{
	if (suppressLevel)
		return;

	QueueEntry entry;
	entry.type = type;
	entry.text = formatError(type,text.c_str());
	queue.push_back(entry);
}

void Logger::queueError(ErrorType type, const wchar_t* text)
{
	if (suppressLevel)
		return;

	QueueEntry entry;
	entry.type = type;
	entry.text = formatError(type,text);
	queue.push_back(entry);
}

void Logger::printQueue()
{
	for (size_t i = 0; i < queue.size(); i++)
	{
		errors.push_back(queue[i].text);

		if (!silent)
			printLine(queue[i].text);

		setFlags(queue[i].type);
	}
}

void TempData::start()
{
	if (!file.getFileName().empty())
	{
		if (!file.open(TextFile::Write))
		{
			Logger::printError(Logger::Error,L"Could not open temp file %s.",file.getFileName().wstring());
			return;
		}

		size_t fileCount = Global.fileList.size();
		size_t lineCount = Global.FileInfo.TotalLineCount;
		size_t labelCount = Global.symbolTable.getLabelCount();
		size_t equCount = Global.symbolTable.getEquationCount();

		file.writeFormat(L"; %d %S included\n",fileCount,fileCount == 1 ? "file" : "files");
		file.writeFormat(L"; %d %S\n",lineCount,lineCount == 1 ? "line" : "lines");
		file.writeFormat(L"; %d %S\n",labelCount,labelCount == 1 ? "label" : "labels");
		file.writeFormat(L"; %d %S\n\n",equCount,equCount == 1 ? "equation" : "equations");
		for (size_t i = 0; i < fileCount; i++)
		{
			file.writeFormat(L"; %S\n",Global.fileList.wstring(i));
		}
		file.writeLine("");
	}
}

void TempData::end()
{
	if (file.isOpen())
		file.close();
}

void TempData::writeLine(int64_t memoryAddress, const std::wstring& text)
{
	if (file.isOpen())
	{
		wchar_t hexbuf[10] = {0};
		swprintf(hexbuf, 10, L"%08X ", (int32_t) memoryAddress);
		std::wstring str = hexbuf + text;
		while (str.size() < 70)
			str += ' ';

		str += tfm::format(L"; %S line %d",
			Global.fileList.wstring(Global.FileInfo.FileNum),Global.FileInfo.LineNumber);

		file.writeLine(str);
	}
}

// file: Core/SymbolData.cpp


#include <algorithm>

SymbolData::SymbolData()
{
	clear();
}

void SymbolData::clear()
{
	enabled = true;
	nocashSymFileName.clear();
	modules.clear();
	files.clear();
	currentModule = 0;
	currentFunction = -1;

	SymDataModule defaultModule;
	defaultModule.file = nullptr;
	modules.push_back(defaultModule);
}

struct NocashSymEntry
{
	int64_t address;
	std::wstring text;

	bool operator<(const NocashSymEntry& other) const
	{
		if (address != other.address)
			return address < other.address;
		return text < other.text;
	}
};

void SymbolData::writeNocashSym()
{
	if (nocashSymFileName.empty())
		return;

	std::vector<NocashSymEntry> entries;
	for (size_t k = 0; k < modules.size(); k++)
	{
		SymDataModule& module = modules[k];
		for (size_t i = 0; i < module.symbols.size(); i++)
		{
			SymDataSymbol& sym = module.symbols[i];

			size_t size = 0;
			for (size_t f = 0; f < module.functions.size(); f++)
			{
				if (module.functions[f].address == sym.address)
				{
					size = module.functions[f].size;
					break;
				}
			}

			NocashSymEntry entry;
			entry.address = sym.address;

			if (size != 0 && nocashSymVersion >= 2)
				entry.text = tfm::format(L"%s,%08X",sym.name,size);
			else
				entry.text = sym.name;

			if (nocashSymVersion == 1)
				std::transform(entry.text.begin(), entry.text.end(), entry.text.begin(), ::towlower);

			entries.push_back(entry);
		}

		for (const SymDataData& data: module.data)
		{
			NocashSymEntry entry;
			entry.address = data.address;

			switch (data.type)
			{
			case Data8:
				entry.text = tfm::format(L".byt:%04X",data.size);
				break;
			case Data16:
				entry.text = tfm::format(L".wrd:%04X",data.size);
				break;
			case Data32:
				entry.text = tfm::format(L".dbl:%04X",data.size);
				break;
			case Data64:
				entry.text = tfm::format(L".dbl:%04X",data.size);
				break;
			case DataAscii:
				entry.text = tfm::format(L".asc:%04X",data.size);
				break;
			}

			entries.push_back(entry);
		}
	}

	std::sort(entries.begin(),entries.end());

	TextFile file;
	if (!file.open(nocashSymFileName,TextFile::Write,TextFile::ASCII))
	{
		Logger::printError(Logger::Error,L"Could not open sym file %s.",file.getFileName().wstring());
		return;
	}
	file.writeLine(L"00000000 0");

	for (size_t i = 0; i < entries.size(); i++)
	{
		file.writeFormat(L"%08X %s\n",entries[i].address,entries[i].text);
	}

	file.write("\x1A");
	file.close();
}

void SymbolData::write()
{
	writeNocashSym();
}

void SymbolData::addLabel(int64_t memoryAddress, const std::wstring& name)
{
	if (!enabled)
		return;

	SymDataSymbol sym;
	sym.address = memoryAddress;
	sym.name = name;

	for (SymDataSymbol& symbol: modules[currentModule].symbols)
	{
		if (symbol.address == sym.address && symbol.name == sym.name)
			return;
	}

	modules[currentModule].symbols.push_back(sym);
}

void SymbolData::addData(int64_t address, size_t size, DataType type)
{
	if (!enabled)
		return;

	SymDataData data;
	data.address = address;
	data.size = size;
	data.type = type;
	modules[currentModule].data.insert(data);
}

size_t SymbolData::addFileName(const std::wstring& fileName)
{
	for (size_t i = 0; i < files.size(); i++)
	{
		if (files[i] == fileName)
			return i;
	}

	files.push_back(fileName);
	return files.size()-1;
}

void SymbolData::startModule(AssemblerFile* file)
{
	for (size_t i = 0; i < modules.size(); i++)
	{
		if (modules[i].file == file)
		{
			currentModule = (int)i;
			return;
		}
	}

	SymDataModule module;
	module.file = file;
	modules.push_back(module);
	currentModule = (int)modules.size()-1;
}

void SymbolData::endModule(AssemblerFile* file)
{
	if (modules[currentModule].file != file)
		return;

	if (currentModule == 0)
	{
		Logger::printError(Logger::Error,L"No module opened");
		return;
	}

	if (currentFunction != -1)
	{
		Logger::printError(Logger::Error,L"Module closed before function end");
		currentFunction = -1;
	}

	currentModule = 0;
}

void SymbolData::startFunction(int64_t address)
{
	if (currentFunction != -1)
	{
		endFunction(address);
	}

	currentFunction = (int)modules[currentModule].functions.size();

	SymDataFunction func;
	func.address = address;
	func.size = 0;
	modules[currentModule].functions.push_back(func);
}

void SymbolData::endFunction(int64_t address)
{
	if (currentFunction == -1)
	{
		Logger::printError(Logger::Error,L"Not inside a function");
		return;
	}

	SymDataFunction& func = modules[currentModule].functions[currentFunction];
	func.size = (size_t) (address-func.address);
	currentFunction = -1;
}

// file: Core/SymbolTable.cpp


const wchar_t validSymbolCharacters[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_.";

bool operator<(SymbolKey const& lhs, SymbolKey const& rhs)
{
	if (lhs.file != rhs.file)
		return lhs.file < rhs.file;
	if (lhs.section != rhs.section)
		return lhs.section < rhs.section;
	return lhs.name.compare(rhs.name) < 0;
}

SymbolTable::SymbolTable()
{
	uniqueCount = 0;
}

SymbolTable::~SymbolTable()
{
	clear();
}

void SymbolTable::clear()
{
	symbols.clear();
	labels.clear();
	equationsCount = 0;
	uniqueCount = 0;
}

void SymbolTable::setFileSectionValues(const std::wstring& symbol, int& file, int& section)
{
	if (symbol[0] == '@')
	{
		if (symbol[1] != '@')
		{
			// static label, @. the section doesn't matter
			section = -1;
		} else {
			// local label, @@. the file doesn't matter
			file = -1;
		}
	} else {
		// global label. neither file nor section matters
		file = section = -1;
	}
}

std::shared_ptr<Label> SymbolTable::getLabel(const std::wstring& symbol, int file, int section)
{
	if (!isValidSymbolName(symbol))
		return nullptr;

	int actualSection = section;
	setFileSectionValues(symbol,file,section);
	SymbolKey key = { symbol, file, section };

	// find label, create new one if it doesn't exist
	auto it = symbols.find(key);
	if (it == symbols.end())
	{
		SymbolInfo value = { LabelSymbol, labels.size() };
		symbols[key] = value;

		std::shared_ptr<Label> result = std::make_shared<Label>(symbol);
		if (section == actualSection)
			result->setSection(section);			// local, set section of parent
		else
			result->setSection(actualSection+1);	// global, set section of children
		labels.push_back(result);
		return result;
	}

	// make sure not to match symbols that aren't labels
	if (it->second.type != LabelSymbol)
		return nullptr;

	return labels[it->second.index];
}

bool SymbolTable::symbolExists(const std::wstring& symbol, int file, int section)
{
	if (!isValidSymbolName(symbol))
		return false;

	setFileSectionValues(symbol,file,section);

	SymbolKey key = { symbol, file, section };
	auto it = symbols.find(key);
	return it != symbols.end();
}

bool SymbolTable::isValidSymbolName(const std::wstring& symbol)
{
	size_t size = symbol.size();
	size_t start = 0;

	// don't match empty names
	if (size == 0 || symbol.compare(L"@") == 0 || symbol.compare(L"@@") == 0)
		return false;

	if (symbol[0] == '@')
	{
		start++;
		if (size > 1 && symbol[1] == '@')
			start++;
	}

	if (symbol[start] >= '0' && symbol[start] <= '9')
		return false;

	for (size_t i = start; i < size; i++)
	{
		if (wcschr(validSymbolCharacters,symbol[i]) == nullptr)
			return false;
	}

	return true;
}

bool SymbolTable::isValidSymbolCharacter(wchar_t character, bool first)
{
	if ((character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z')) return true;
	if (!first && (character >= '0' && character <= '9')) return true;
	if (character == '_' || character == '.') return true;
	if (character == '@') return true;
	return false;
}

bool SymbolTable::addEquation(const std::wstring& name, int file, int section, size_t referenceIndex)
{
	if (!isValidSymbolName(name))
		return false;

	if (symbolExists(name,file,section))
		return false;

	setFileSectionValues(name,file,section);

	SymbolKey key = { name, file, section };
	SymbolInfo value = { EquationSymbol, referenceIndex };
	symbols[key] = value;

	equationsCount++;
	return true;
}

bool SymbolTable::findEquation(const std::wstring& name, int file, int section, size_t& dest)
{
	setFileSectionValues(name,file,section);

	SymbolKey key = { name, file, section };
	auto it = symbols.find(key);
	if (it == symbols.end() || it->second.type != EquationSymbol)
		return false;

	dest = it->second.index;
	return true;
}

// TODO: better
std::wstring SymbolTable::getUniqueLabelName(bool local)
{
	std::wstring name = tfm::format(L"__armips_label_%08x__",uniqueCount++);
	if (local)
		name = L"@@" + name;

	generatedLabels.insert(name);
	return name;
}

void SymbolTable::addLabels(const std::vector<LabelDefinition>& labels)
{
	for (const LabelDefinition& def: labels)
	{
		if (!isValidSymbolName(def.name))
			continue;

		std::shared_ptr<Label> label = getLabel(def.name,Global.FileInfo.FileNum,Global.Section);
		if (label == nullptr)
			continue;

		label->setOriginalName(def.originalName);

		if (!isLocalSymbol(def.name))
			Global.Section++;

		label->setDefined(true);
		label->setValue(def.value);
	}
}

int SymbolTable::findSection(int64_t address)
{
	int64_t smallestBefore = -1;
	int64_t smallestDiff = 0x7FFFFFFF;

	for (auto& lab: labels)
	{
		int64_t diff = address-lab->getValue();
		if (diff >= 0 && diff < smallestDiff)
		{
			smallestDiff = diff;
			smallestBefore = lab->getSection();
		}
	}

	return smallestBefore;
}

// file: Main/main.cpp

#include <clocale>

int wmain(int argc, wchar_t* argv[])
{
	std::setlocale(LC_CTYPE,"");

#ifdef ARMIPS_TESTS
	std::wstring name;

	if (argc < 2)
		return !runTests(L"Tests", argv[0]);
	else
		return !runTests(argv[1], argv[0]);
#endif

	std::vector<std::wstring> arguments = getStringListFromArray(argv,argc);

	return runFromCommandLine(arguments);
}

#ifndef _WIN32

int main(int argc, char* argv[])
{
	// convert input to wstring
	std::vector<std::wstring> wideStrings;
	for (int i = 0; i < argc; i++)
	{
		std::wstring str = convertUtf8ToWString(argv[i]);
		wideStrings.push_back(str);
	}

	// create argv replacement
	wchar_t** wargv = new wchar_t*[argc];
	for (int i = 0; i < argc; i++)
	{
		wargv[i] = (wchar_t*) wideStrings[i].c_str();
	}

	int result = wmain(argc,wargv);

	delete[] wargv;
	return result;
}

#endif
