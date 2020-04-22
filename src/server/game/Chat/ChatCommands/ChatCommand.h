/*
* Copyright (C) 2016+     AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-GPL2
* Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
* Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
*/

#ifndef ACORE_CHATCOMMAND_H
#define ACORE_CHATCOMMAND_H

#include "ChatCommandArgs.h"
#include "ChatCommandTags.h"
#include "Define.h"
#include "Errors.h"
#include <std::optional>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <vector>

class ChatHandler;
class CommandArgs;

template <typename T>
struct CommandArgsConsumerSingle
{
    using arginfo = acore::ChatCommands::ArgInfo<T>;
    static char const* TryConsumeTo(T& val, char const* args)
    {
        return arginfo::TryConsume(val, args);
    }
};

struct CommandArgsVariantConsumer
{
    template <typename V, typename T1, typename T2, typename... Ts>
    static char const* TryConsumeTo(V& val, char const* args)
    {
        T1 v;
        if (char const* next = CommandArgsConsumerSingle<T1>::TryConsumeTo(v, args))
        {
            val = std::move(v);
            return next;
        }
        else
            return TryConsumeTo<V, T2, Ts...>(val, args);
    }

    template <typename V, typename T1>
    static char const* TryConsumeTo(V& val, char const* args)
    {
        T1 v;
        if (char const* next = CommandArgsConsumerSingle<T1>::TryConsumeTo(v, args))
        {
            val = std::move(v);
            return next;
        }
        else
            return nullptr;
    }
};

template <typename... Ts>
struct CommandArgsConsumerSingle<acore::ChatCommands::Variant<Ts...>>
{
    static char const* TryConsumeTo(acore::ChatCommands::Variant<Ts...>& val, char const* args)
    {
        return CommandArgsVariantConsumer::TryConsumeTo<acore::ChatCommands::Variant<Ts...>, Ts...>(val, args);
    }
};

template <typename T>
struct CommandArgsConsumerSingle<std::vector<T>>
{
    static char const* TryConsumeTo(std::vector<T>& val, char const* args)
    {
        char const* last;
        val.clear();

        do val.emplace_back();
        while ((args = CommandArgsConsumerSingle<T>::TryConsumeTo(val.back(), (last = args))));

        val.pop_back();
        return last;
    }
};

template <>
struct CommandArgsConsumerSingle<CommandArgs*>
{
    static char const* TryConsumeTo(CommandArgs*&, char const* args) { return args; }
};

template <>
struct CommandArgsConsumerSingle<char const*>
{
    static char const* TryConsumeTo(char const*&, char const* args) { return args; }
};

template <typename T, size_t offset>
struct CommandArgsConsumerNext;

template <typename Tuple, typename NextType, size_t offset>
struct CommandArgsConsumerMulti
{
    static char const* TryConsumeTo(Tuple& tuple, char const* args)
    {
        if (char const* next = CommandArgsConsumerSingle<NextType>::TryConsumeTo(std::get<offset>(tuple), args))
            return CommandArgsConsumerNext<Tuple, offset+1>::GoNext(tuple, next);
        else
            return nullptr;
    }
};

template <typename Tuple, typename NestedNextType, size_t offset>
struct CommandArgsConsumerMulti<Tuple, std::optional<NestedNextType>, offset>
{
    static char const* TryConsumeTo(Tuple& tuple, char const* args)
    {
        // try with the argument
        auto& myArg = std::get<offset>(tuple);
        myArg.emplace();
        if (char const* next = CommandArgsConsumerSingle<NestedNextType>::TryConsumeTo(*(myArg.get_ptr()), args))
            if ((next = CommandArgsConsumerNext<Tuple, offset+1>::GoNext(tuple, next)))
                return next;
        // try again omitting the argument
        myArg = std::nullopt;
        if (char const* next = CommandArgsConsumerNext<Tuple, offset+1>::GoNext(tuple, args))
            return next;
        return nullptr;
    }
};

template <size_t offset, typename... Ts>
struct CommandArgsConsumerNext<std::tuple<Ts...>, offset>
{
    using tuple_type = std::tuple<Ts...>;

    template <bool C = (offset < sizeof...(Ts))>
    static std::enable_if_t<C, char const*> GoNext(tuple_type& tuple, char const* args)
    {
        return CommandArgsConsumerMulti<tuple_type, std::tuple_element_t<offset, tuple_type>, offset>::TryConsumeTo(tuple, args);
    }

    template <bool C = (offset < sizeof...(Ts))>
    static std::enable_if_t<!C, char const*> GoNext(tuple_type&, char const* args)
    {
        return args;
    }
};

class CommandArgs
{
    public:
        CommandArgs(char const* args) : _original(args), _args(args) {}

        template <typename T1, typename T2, typename... Ts>
        auto TryConsume()
        {
            std::optional<std::tuple<std::remove_cvref_t<T1>, std::remove_cvref_t<T2>, std::remove_cvref_t<Ts>...>> rv;
            rv.emplace();
            if (!TryConsumeToTuple<0>(*(rv.get_ptr())))
                rv = std::nullopt;
            return rv;
        }

        template <typename T1>
        auto TryConsume()
        {
            using T = std::remove_cvref_t<T1>;
            std::optional<T> rv;
            rv.emplace();
            if (char const* next = CommandArgsConsumerSingle<T>::TryConsumeTo(*(rv.get_ptr()), _args))
                _args = next;
            else
                rv = std::nullopt;
            return rv;
        }

        template <size_t offset = 0, typename T>
        bool TryConsumeToTuple(T& tuple)
        {
            if (char const* next = CommandArgsConsumerNext<T, offset>::GoNext(tuple, _args))
            {
                _args = next;
                return true;
            }
            else
                return false;
        }

        void Reset() { _args = _original; }

        char const* GetFullArgs() const { return _original; }
        char const* GetRemainingArgs() const { return _args; }

        bool IsEmpty() const { return !!*_args; }
        explicit operator bool() const { return IsEmpty(); }

    private:
        char const* const _original;
        char const* _args;
};

template <typename T> struct ChatCommandHandlerToTuple { static_assert(!std::is_same_v<T,T>, "Invalid command handler signature"); };
template <typename... Ts> struct ChatCommandHandlerToTuple<bool(*)(ChatHandler*, Ts...)> { using type = std::tuple<ChatHandler*, std::remove_cvref_t<Ts>...>; };

template <typename T> struct ChatCommandStoreLastArg { static void store(T&, CommandArgs&) {} };
template <> struct ChatCommandStoreLastArg<char const*> { static void store(char const*& arg, CommandArgs& args) { arg = args.GetRemainingArgs(); } };
template <> struct ChatCommandStoreLastArg<CommandArgs*> { static void store(CommandArgs*& arg, CommandArgs& args) { arg = &args; } };

class  ChatCommand
{
    using wrapper_func = bool(void*, ChatHandler*, char const*);

    public:
        template <typename TypedHandler>
        ChatCommand(char const* name, uint32 permission, bool allowConsole, TypedHandler handler, std::string help)
            : Name(ASSERT_NOTNULL(name)), Permission(permission), AllowConsole(allowConsole), Help(std::move(help)), ChildCommands({})
        {
            _wrapper = [](void* handler, ChatHandler* chatHandler, char const* argsStr)
            {
                using tuple_type = typename ChatCommandHandlerToTuple<TypedHandler>::type;

                tuple_type arguments;
                std::get<0>(arguments) = chatHandler;

                CommandArgs args(argsStr);
                if (args.TryConsumeToTuple<1>(arguments))
                {
                    auto& last = std::get<std::tuple_size_v<tuple_type>-1>(arguments);
                    ChatCommandStoreLastArg<std::remove_cvref_t<decltype(last)>>::store(last, args);
                    return std::apply(reinterpret_cast<TypedHandler>(handler), std::move(arguments));
                }
                else
                    return false;
            };
            _handler = reinterpret_cast<void*>(handler);
        }

        ChatCommand(char const* name, uint32 permission, bool allowConsole, std::nullptr_t, std::string help, std::vector<ChatCommand> childCommands = {})
            : Name(ASSERT_NOTNULL(name)), Permission(permission), AllowConsole(allowConsole), Help(std::move(help)), ChildCommands(std::move(childCommands))
        {
            _wrapper = nullptr;
            _handler = nullptr;
        }

        bool operator()(ChatHandler* chatHandler, char const* args) const
        {
            ASSERT(_wrapper && _handler);
            return _wrapper(_handler, chatHandler, args);
        }

        bool HasHandler() const { return !!_handler; }

        char const* Name;
        uint32 Permission;                   // function pointer required correct align (use uint32)
        bool AllowConsole;
        std::string Help;
        std::vector<ChatCommand> ChildCommands;

    private:
        wrapper_func* _wrapper;
        void* _handler;
};

#endif
