/*
* Copyright (C) 2016+     AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-GPL2
* Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
* Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
*/

#ifndef ACORE_CHATCOMMANDHELPERS_H
#define ACORE_CHATCOMMANDHELPERS_H

#include <type_traits>

namespace acore
{
namespace ChatCommands
{

static constexpr char COMMAND_DELIMITER = ' ';

/***************** HELPERS *************************\
|* These really aren't for outside use...          *|
\***************************************************/
inline size_t tokenize(char const*& end)
{
    size_t len = 0;
    for (; *end && *end != COMMAND_DELIMITER; ++end, ++len);
    for (; *end && *end == COMMAND_DELIMITER; ++end);
    return len;
}

template <typename T, typename = void>
struct tag_base
{
    using type = T;
};

template <typename T>
using tag_base_t = typename tag_base<T>::type;

template <typename...>
struct are_all_assignable
{
    static constexpr bool value = true;
};

template <typename T1, typename T2, typename... Ts>
struct are_all_assignable<T1, T2, Ts...>
{
    static constexpr bool value = std::is_assignable_v<T1&, T2> && are_all_assignable<T1, Ts...>::value;
};

template <size_t index, typename T1, typename... Ts>
struct get_nth : get_nth<index-1, Ts...> { };

template <typename T1, typename... Ts>
struct get_nth<0, T1, Ts...>
{
    using type = T1;
};

template <size_t index, typename... Ts>
using get_nth_t = typename get_nth<index, Ts...>::type;

}
}

#endif
