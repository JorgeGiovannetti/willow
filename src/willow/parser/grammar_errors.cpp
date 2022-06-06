#include <tao/pegtl.hpp>
#include "grammar.cpp"

using namespace tao::pegtl;

namespace willow::parser
{

    // clang-format off

    template< typename > inline constexpr const char* error_message = nullptr;

    template<> inline constexpr auto error_message<t_semicolon> = "expected a ';'";
    template<> inline constexpr auto error_message<a_eof> = "unexpected character before end of file";

    // clang-format on

    // As must_if<> can not take error_message as a template parameter directly, we need to wrap it.
    struct error
    {
        template <typename Rule>
        static constexpr auto message = error_message<Rule>;
    };

    template <typename Rule>
    using control = must_if<error>::control<Rule>;
}