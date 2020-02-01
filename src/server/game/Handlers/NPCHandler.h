/*
 * Copyright (C) 2016+     AzerothCore <www.azerothcore.org>
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 */

#ifndef __NPCHANDLER_H
#define __NPCHANDLER_H

struct QEmote
{
    uint32 _Emote;
    uint32 _Delay;
};

#define MAX_GOSSIP_TEXT_EMOTES 3

struct GossipTextOption
{
    std::string Text_0;
    std::string Text_1;
    uint32 BroadcastTextID;
    uint32 Language;
    float Probability;
    QEmote Emotes[MAX_GOSSIP_TEXT_EMOTES];
};

#define MAX_GOSSIP_TEXT_OPTIONS 8

struct GossipText
{
    GossipTextOption Options[MAX_GOSSIP_TEXT_OPTIONS];
};

#endif
