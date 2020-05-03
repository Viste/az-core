/*
 * Copyright (C) since 2020 Andrei Guluaev (Winfidonarleyan/Kargatum) https://github.com/Winfidonarleyan
 * Licence MIT https://opensource.org/MIT
 */

#include "Log.h"
#include "ScriptMgr.h"
#include "GameConfig.h"
#include "Chat.h"
#include "Player.h"
#include "Battleground.h"

class BGReward_Player : public BGScript
{
public:
    BGReward_Player() : BGScript("BGReward_Player") { }

    void OnBattlegroundEndReward(Battleground* bg, Player* player, TeamId winnerTeamId) override
    {
        if (!CONF_GET_BOOL("BGReward.Enable"))
            return;

        // Not reward on end arena
        if (bg->isArena())
            return;
       
        TeamId bgTeamId = player->GetBgTeamId();
        uint32 RewardCount = 0;

        bgTeamId == winnerTeamId ? RewardCount = CONF_GET_INT("BGReward.WinnerTeam.Count") : RewardCount = CONF_GET_INT("BGReward.LoserTeam.Count");

        switch (player->GetZoneId())
        {
        case 3277: // Warsong Gulch
            player->AddItem(CONF_GET_INT("BGReward.ItemID.WSG"), RewardCount);
            break;
        case 3358: // Arathi Basin
            player->AddItem(CONF_GET_INT("BGReward.ItemID.Arathi"), RewardCount);
            break;
        case 3820: // Eye of the Storm
            player->AddItem(CONF_GET_INT("BGReward.ItemID.Eye"), RewardCount);
            break;
        case 4710: // Isle of Conquest
            player->AddItem(CONF_GET_INT("BGReward.ItemID.Isle"), RewardCount);
            break;
        case 4384: // Strand of the Ancients
            player->AddItem(CONF_GET_INT("BGReward.ItemID.Ancients"), RewardCount);
            break;
        case 2597: // Alterac Valley
            player->AddItem(CONF_GET_INT("BGReward.ItemID.Alterac"), RewardCount);
            break;
        default:
            break;
        }
    }
};

class BGReward_World : public WorldScript
{
public:
    BGReward_World() : WorldScript("BGReward_World") { }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        sGameConfig->AddBoolConfig("BGReward.Enable");
        sGameConfig->AddIntConfig("BGReward.ItemID.WSG", ITEM_WS_MARK_OF_HONOR);
        sGameConfig->AddIntConfig("BGReward.ItemID.Arathi", ITEM_AB_MARK_OF_HONOR);
        sGameConfig->AddIntConfig("BGReward.ItemID.Alterac", ITEM_AV_MARK_OF_HONOR);
        sGameConfig->AddIntConfig("BGReward.ItemID.Isle", ITEM_IC_MARK_OF_HONOR);
        sGameConfig->AddIntConfig("BGReward.ItemID.Ancients", ITEM_SA_MARK_OF_HONOR);
        sGameConfig->AddIntConfig("BGReward.ItemID.Eye", ITEM_EY_MARK_OF_HONOR);
        sGameConfig->AddIntConfig("BGReward.WinnerTeam.Count", 3);
        sGameConfig->AddIntConfig("BGReward.LoserTeam.Count", 1);
    }
};

// Group all custom scripts
void AddSC_BGReward()
{
    new BGReward_Player();
    new BGReward_World();
}
