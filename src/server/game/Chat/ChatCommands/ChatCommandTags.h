/*
* Copyright (C) 2016+     AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-GPL2
* Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
* Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
*/

#ifndef ACORE_CHATCOMMANDTAGS_H
#define ACORE_CHATCOMMANDTAGS_H

#include "ChatCommandHelpers.h"
#include "Hyperlinks.h"
#include <cmath>
#include <cstring>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <optional>

namespace acore
{
namespace ChatCommands
{
/************************** CONTAINER TAGS **********************************************\
|* Simple holder classes to differentiate between extraction methods                    *|
|* Should inherit from ContainerTag for template identification                         *|
|* Must implement the following:                                                        *|
|* - TryConsume: char const* -> char const*                                             *|
|*   returns nullptr if no match, otherwise pointer to first character of next token    *|
|* - typedef value_type of type that is contained within the tag                        *|
|* - cast operator to value_type                                                        *|
|*                                                                                      *|
\****************************************************************************************/
struct ContainerTag {};
template <typename T>
struct tag_base<T, std::enable_if_t<std::is_base_of_v<ContainerTag, T>>>
{
    using type = typename T::value_type;
};

template <char c1, char... chars>
struct ExactSequence : public ContainerTag
{
    using value_type = void;

    static constexpr bool isSingleChar = !sizeof...(chars);

    template <bool C = isSingleChar>
    static typename std::enable_if_t<!C, char const*> _TryConsume(char const* pos)
    {
        if (*(pos++) == c1)
            return ExactSequence<chars...>::_TryConsume(pos);
        else
            return nullptr;
    }

    template <bool C = isSingleChar>
    static typename std::enable_if_t<C, char const*> _TryConsume(char const* pos)
    {
        if (*(pos++) != c1)
            return nullptr;
        // if more of string is left, tokenize should return 0 (otherwise we didn't reach end of token yet)
        return *pos && tokenize(pos) ? nullptr : pos;
    }

    char const* TryConsume(char const* pos) const { return ExactSequence::_TryConsume(pos); }
};

template <typename linktag>
struct Hyperlink : public ContainerTag
{
    typedef typename linktag::value_type value_type;
    typedef std::remove_cvref_t<value_type> storage_type;

    public:
        operator value_type() const { return val; }
        value_type operator*() const { return val; }
        storage_type const* operator->() const { return &val; }

        char const* TryConsume(char const* pos)
        {
            acore::Hyperlinks::HyperlinkInfo info = acore::Hyperlinks::ParseHyperlink(pos);
            // invalid hyperlinks cannot be consumed
            if (!info)
                return nullptr;

            // check if we got the right tag
            if (info.tag.second != strlen(linktag::tag()))
                return nullptr;
            if (strncmp(info.tag.first, linktag::tag(), strlen(linktag::tag())) != 0)
                return nullptr;

            // store value
            if (!linktag::StoreTo(val, info.data.first, info.data.second))
                return nullptr;

            // finally, skip to end of token
            pos = info.next;
            tokenize(pos);

            // return final pos
            return pos;
        }

    private:
        storage_type val;
};

// pull in link tags for user convenience
using namespace ::acore::Hyperlinks::LinkTags;

/************************** VARIANT TAG LOGIC *********************************\
|* This has some special handling over in ChatCommand.h                       *|
\******************************************************************************/

template <typename T>
struct CastToVisitor {
    using result_type = T;

    template <typename U>
    T operator()(U const& v) const { return v; }
};

template <typename T1, typename... Ts>
struct Variant : public std::variant<T1, Ts...>
{
    using first_type = tag_base_t<T1>;
    static constexpr bool have_operators = are_all_assignable<first_type, tag_base_t<Ts>...>::value;

    template <bool C = have_operators>
    operator std::enable_if_t<C, first_type>()
    {
        return operator*();
    }

    template <bool C = have_operators>
    std::enable_if_t<C, first_type> operator*()
    {
        return std::visit(CastToVisitor<first_type>(), *this);
    }

    template <bool C = have_operators>
    std::enable_if_t<C, first_type const&> operator*() const
    {
        return std::visit(CastToVisitor<first_type const&>(), *this);
    }

    template <typename T>
    Variant& operator=(T&& arg) { std::variant<T1, Ts...>::operator=(std::forward<T>(arg)); return *this; }

    template <size_t index>
    decltype(auto) get() const { return std::get<get_nth_t<index, T1, Ts...>>(*this); }
};

}
}

#endif
