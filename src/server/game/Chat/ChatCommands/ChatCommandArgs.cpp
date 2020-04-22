/*
* Copyright (C) 2016+     AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-GPL2
* Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
* Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
*/

#include "ChatCommandArgs.h"
#include "AchievementMgr.h"
#include "ChatCommand.h"
#include "ObjectMgr.h"

using namespace acore::ChatCommands;

struct AchievementVisitor
{
    using value_type = AchievementEntry const*;
    value_type operator()(Hyperlink<achievement> achData) const { return achData->Achievement; }
    value_type operator()(uint32 achId) const { return sAchievementMgr->GetAchievement(achId); }
};
char const* acore::ChatCommands::ArgInfo<AchievementEntry const*>::TryConsume(AchievementEntry const*& data, char const* args)
{
    Variant <Hyperlink<achievement>, uint32> val;
    if ((args = CommandArgsConsumerSingle<decltype(val)>::TryConsumeTo(val, args)))
        data = std::visit(AchievementVisitor(), val);
    return args;
}

struct GameTeleVisitor
{
    using value_type = GameTele const*;
    value_type operator()(Hyperlink<tele> tele) const { return sObjectMgr->GetGameTele(tele); }
    value_type operator()(std::string const& tele) const { return sObjectMgr->GetGameTele(tele); }
};
char const* acore::ChatCommands::ArgInfo<GameTele const*>::TryConsume(GameTele const*& data, char const* args)
{
    Variant<Hyperlink<tele>, std::string> val;
    if ((args = CommandArgsConsumerSingle<decltype(val)>::TryConsumeTo(val, args)))
        data = std::visit(GameTeleVisitor(), val);
    return args;
}

struct BoolVisitor
{
    using value_type = bool;
    value_type operator()(uint32 i) const { return !!i; }
    value_type operator()(ExactSequence<'o', 'n'>) const { return true; }
    value_type operator()(ExactSequence<'o', 'f', 'f'>) const { return false; }
};
char const* acore::ChatCommands::ArgInfo<bool>::TryConsume(bool& data, char const* args)
{
    Variant<uint32, ExactSequence<'o', 'n'>, ExactSequence<'o', 'f', 'f'>> val;
    if ((args = CommandArgsConsumerSingle<decltype(val)>::TryConsumeTo(val, args)))
        data = std::visit(BoolVisitor(), val);
    return args;
}
