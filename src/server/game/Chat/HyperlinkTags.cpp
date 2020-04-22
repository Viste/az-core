/*
* Copyright (C) 2016+     AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-GPL2
* Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
* Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
*/

#include "Hyperlinks.h"
#include "AchievementMgr.h"
#include "ObjectMgr.h"
#include "SpellInfo.h"
#include "SpellMgr.h"

static constexpr char HYPERLINK_DATA_DELIMITER = ':';

class HyperlinkDataTokenizer
{
    public:
    HyperlinkDataTokenizer(char const* pos, size_t len) : _pos(pos), _len(len), _empty(false) {}

    template <typename T>
    bool TryConsumeTo(T& val)
    {
        if (_empty)
            return false;

        char const* firstPos = _pos;
        size_t thisLen = 0;
        // find next delimiter
        for (; _len && *_pos != HYPERLINK_DATA_DELIMITER; --_len, ++_pos, ++thisLen);
        if (_len)
            --_len, ++_pos; // skip the delimiter
        else
            _empty = true;

        return acore::Hyperlinks::LinkTags::base_tag::StoreTo(val, firstPos, thisLen);
    }

    bool IsEmpty() { return _empty; }

    private:
    char const* _pos;
    size_t _len;
    bool _empty;
};

bool acore::Hyperlinks::LinkTags::achievement::StoreTo(AchievementLinkData& val, char const* pos, size_t len)
{
    HyperlinkDataTokenizer t(pos, len);
    uint32 achievementId;
    if (!t.TryConsumeTo(achievementId))
        return false;
    val.Achievement = sAchievementMgr->GetAchievement(achievementId);
    return val.Achievement && t.TryConsumeTo(val.CharacterId) && t.TryConsumeTo(val.IsFinished) &&
        t.TryConsumeTo(val.Month) && t.TryConsumeTo(val.Day) && t.TryConsumeTo(val.Year) && t.TryConsumeTo(val.Criteria[0]) &&
        t.TryConsumeTo(val.Criteria[1]) && t.TryConsumeTo(val.Criteria[2]) && t.TryConsumeTo(val.Criteria[3]) && t.IsEmpty();
}

bool acore::Hyperlinks::LinkTags::enchant::StoreTo(SpellInfo const*& val, char const* pos, size_t len)
{
    HyperlinkDataTokenizer t(pos, len);
    uint32 spellId;
    if (!(t.TryConsumeTo(spellId) && t.IsEmpty()))
        return false;
    return (val = sSpellMgr->GetSpellInfo(spellId)) && val->HasAttribute(SPELL_ATTR0_TRADESPELL);
}

bool acore::Hyperlinks::LinkTags::glyph::StoreTo(GlyphLinkData& val, char const* pos, size_t len)
{
    HyperlinkDataTokenizer t(pos, len);
    uint32 slot, prop;
    if (!(t.TryConsumeTo(slot) && t.TryConsumeTo(prop) && t.IsEmpty()))
        return false;
    if (!(val.Slot = sGlyphSlotStore.LookupEntry(slot)))
        return false;
    if (!(val.Glyph = sGlyphPropertiesStore.LookupEntry(prop)))
        return false;
    return true;
}

bool acore::Hyperlinks::LinkTags::item::StoreTo(ItemLinkData& val, char const* pos, size_t len)
{
    HyperlinkDataTokenizer t(pos, len);
    uint32 itemId, dummy;
    if (!t.TryConsumeTo(itemId))
        return false;
    val.Item = sObjectMgr->GetItemTemplate(itemId);
    return val.Item && t.TryConsumeTo(val.EnchantId) && t.TryConsumeTo(val.GemEnchantId[0]) && t.TryConsumeTo(val.GemEnchantId[1]) &&
        t.TryConsumeTo(val.GemEnchantId[2]) && t.TryConsumeTo(dummy) && t.TryConsumeTo(val.RandomPropertyId) && t.TryConsumeTo(val.RandomPropertySeed) &&
        t.TryConsumeTo(val.RenderLevel) && t.IsEmpty() && !dummy;
}

bool acore::Hyperlinks::LinkTags::quest::StoreTo(QuestLinkData& val, char const* pos, size_t len)
{
    HyperlinkDataTokenizer t(pos, len);
    uint32 questId;
    if (!t.TryConsumeTo(questId))
        return false;
    return (val.Quest = sObjectMgr->GetQuestTemplate(questId)) && t.TryConsumeTo(val.QuestLevel) && t.IsEmpty();
}

bool acore::Hyperlinks::LinkTags::spell::StoreTo(SpellInfo const*& val, char const* pos, size_t len)
{
    HyperlinkDataTokenizer t(pos, len);
    uint32 spellId;
    if (!(t.TryConsumeTo(spellId) && t.IsEmpty()))
        return false;
    return !!(val = sSpellMgr->GetSpellInfo(spellId));
}

bool acore::Hyperlinks::LinkTags::talent::StoreTo(TalentLinkData& val, char const* pos, size_t len)
{
    HyperlinkDataTokenizer t(pos, len);
    uint32 talentId;
    if (!(t.TryConsumeTo(talentId) && t.TryConsumeTo(val.Rank) && t.IsEmpty()))
        return false;
    if (!(val.Talent = sTalentStore.LookupEntry(talentId)))
        return false;
    if (!val.Talent->RankID[val.Rank-1])
        return false;
    return true;
}

bool acore::Hyperlinks::LinkTags::trade::StoreTo(TradeskillLinkData& val, char const* pos, size_t len)
{
    HyperlinkDataTokenizer t(pos, len);
    uint32 spellId;
    uint64 guid;
    if (!t.TryConsumeTo(spellId))
        return false;
    val.Spell = sSpellMgr->GetSpellInfo(spellId);
    if (!(val.Spell && val.Spell->Effects[0].Effect == SPELL_EFFECT_TRADE_SKILL && t.TryConsumeTo(val.CurValue) &&
      t.TryConsumeTo(val.MaxValue) && t.TryConsumeTo(guid) && t.TryConsumeTo(val.KnownRecipes) && t.IsEmpty()))
        return false;
    val.Owner.Set(guid);
    return true;
}
