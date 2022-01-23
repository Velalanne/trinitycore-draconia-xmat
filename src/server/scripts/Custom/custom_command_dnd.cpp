#include "ScriptMgr.h"
#include "Chat.h"
#include "DBCStores.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "Pet.h"
#include "Player.h"
#include "RBAC.h"
#include "ReputationMgr.h"
#include "WorldSession.h"
#include "Item.h"
#include "DatabaseEnv.h"
#include "DatabaseEnvFwd.h"
#include "Group.h"
#include <tuple>
#include <vector>
#include <memory>
#include <stdexcept>
#include <unordered_set>

#if TRINITY_COMPILER == TRINITY_COMPILER_GNU
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

struct dnd_bonus_table
{
    int32 melee_hit;
    int32 ranged_hit;
    int32 spell_hit;
    std::pair<int32, int32> strength; //stat, prof
    std::pair<int32, int32> agility; //stat, prof
    std::pair<int32, int32> stamina; //stat, prof
    std::pair<int32, int32> intellect; //stat, prof
    std::pair<int32, int32> spirit; //stat, prof
};

class dnd_commandscript : public CommandScript
{
private:
    enum class Hit : char {Melee, Ranged, Spell};
    enum class Stat : char { Strength, Agility, Stamina, Intellect, Spirit, Nothing };

    static std::unordered_set<std::string> strength;
    static std::unordered_set<std::string> agility;
    static std::unordered_set<std::string> stamina;
    static std::unordered_set<std::string> intellect;
    static std::unordered_set<std::string> spirit;

public:

    dnd_commandscript() : CommandScript("dnd_commandscript") {}

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> dndRollCommandTable =
        {
            { "melee",   rbac::RBAC_PERM_DND_ROLL_MELEE,  false, &HandleDndRollMeleeCommand, "" },
            { "ranged",  rbac::RBAC_PERM_DND_ROLL_RANGED, false, &HandleDndRollRangedCommand, "" },
            { "spell",   rbac::RBAC_PERM_DND_ROLL_SPELL,  false, &HandleDndRollSpellCommand, "" },
            { "stat",    rbac::RBAC_PERM_DND_ROLL_STAT,   false, &HandleDndRollStatCommand, "" },
            { "dice",    rbac::RBAC_PERM_DND_ROLL_DICE,   false, &HandleDndRollDiceCommand, "" },
        };
        static std::vector<ChatCommand> dndCommandTable =
        {
            { "roll",    rbac::RBAC_PERM_DND_ROLL,        false, nullptr,                   "", dndRollCommandTable },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "dnd",     rbac::RBAC_PERM_DND,             false, nullptr,                   "", dndCommandTable },
        };
        return commandTable;
    }

private:

    static std::size_t FindFirstNotWhitespace(std::string const& text, std::size_t start)
    {
        while ((text.size() > start) && std::isspace(text[start]))
        {
            start++;
        }
        return start;
    }

    static std::pair<std::string, std::size_t> SplitByWhitespace(std::string const& text, std::size_t start)
    {
        for (std::size_t i = start; i < text.size(); ++i)
        {
            if (std::isspace(text[i]))
            {
                auto result = text.substr(start, i - start);
                return std::pair(result, FindFirstNotWhitespace(text, i));
            }
        }

        auto result = text.substr(start);
        return std::pair(result, text.size());
    }

    static std::pair<int, int> ParseDiceDescription(std::string const& dice)
    {
        auto d_position = dice.find_first_of('d');
        if ((d_position == std::string::npos) || (d_position == 0)){
            return std::pair(0, 0);
        }
        int dices = 0;
        int faces = 0;
        try
        {
            auto dices_str = dice.substr(0, d_position);
            dices = std::strtoimax(dices_str.c_str(), nullptr, 10);
            if ((dices < 1) || (dices > 256) || (d_position + 1 >= dice.size()))
            {
                return std::pair(0, 0);
            }
            auto faces_str = dice.substr(d_position + 1);
            faces = std::strtoimax(faces_str.c_str(), nullptr, 10);
        }
        catch (...)
        {
            return std::pair(0, 0);
        }

        return std::pair(dices, faces);
    }

    static std::tuple<int, int, int> ParseDiceCommand(char const* args)
    {
        auto text = std::string(args);
        if (text.empty())
        {
            return std::tuple(0, 0, 0);
        }
        auto dice_pair = SplitByWhitespace(text, 0);
        auto dice = ParseDiceDescription(dice_pair.first);
        if ((dice.first == 0) || (dice.second == 0))
        {
            return std::tuple(0, 0, 0);
        }

        if (dice_pair.second >= text.size())
        {
            return std::tuple(dice.first, dice.second, 0);
        }
        auto bonus_pair = SplitByWhitespace(text, dice_pair.second);
        int bonus = 0;
        try
        {
            bonus = std::strtoimax(bonus_pair.first.c_str(), nullptr, 10);
            if ((bonus < -254) || (bonus > 255))
            {
                return std::tuple(0, 0, 0);
            }
        }
        catch (...)
        {
            return std::tuple(0, 0, 0);
        }
        return std::tuple(dice.first, dice.second, bonus);
    }

    static bool HandleDndRollDiceCommand(ChatHandler* handler, char const* args)
    {
        auto dice = ParseDiceCommand(args);
        if ((std::get<0>(dice) == 0) || (std::get<1>(dice) == 0))
        {
            handler->PSendSysMessage(LANG_COMMAND_DND_ROLL_DICE_ERROR);
            handler->SetSentErrorMessage(true);
            return false;
        }
        auto rolled = 0;
        for (std::size_t i = 0; i < std::get<0>(dice); ++i)
        {
            rolled += irand(1, std::get<1>(dice));
        }

        auto bonus = std::get<2>(dice);
        auto result = rolled + bonus;

        auto player = handler->GetSession()->GetPlayer();
        auto players = GetGroupPlayers(player);
        auto message = BuildPacket(handler, LANG_COMMAND_DND_ROLL_DICE, handler->GetNameLink(player).c_str(), std::get<0>(dice), std::get<1>(dice), result, rolled, bonus);
        for (auto&& item : players)
        {
            item->GetSession()->SendPacket(&message);
        }

        return true;
    }

    static std::vector<Item*> EquippedItems(Player* player)
    {
        auto result = std::vector<Item*>();

        for (auto i = (uint16)EQUIPMENT_SLOT_START; i <= (uint16)EQUIPMENT_SLOT_END; ++i)
        {
            auto item = player->GetItemByPos(i);
            if (item != nullptr)
            {
                result.push_back(item);
            }
        }

        return result;
    }

    static std::unique_ptr<dnd_bonus_table> GetBonusTable(Player* player)
    {
        auto table = std::make_unique<dnd_bonus_table>();
        uint32 melee_hit = player->GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + CombatRating::CR_HIT_MELEE);
        uint32 ranged_hit = player->GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + CombatRating::CR_HIT_RANGED);
        uint32 spell_hit = player->GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + CombatRating::CR_HIT_SPELL);

        uint32 strength = player->GetTotalStatValue(Stats::STAT_STRENGTH);
        uint32 agility = player->GetTotalStatValue(Stats::STAT_AGILITY);
        uint32 stamina = player->GetTotalStatValue(Stats::STAT_STAMINA);
        uint32 intellect = player->GetTotalStatValue(Stats::STAT_INTELLECT);
        uint32 spirit = player->GetTotalStatValue(Stats::STAT_SPIRIT);

        table->melee_hit = melee_hit;
        table->ranged_hit = ranged_hit;
        table->spell_hit = spell_hit;
        table->strength = std::make_pair(strength,   0);
        table->agility = std::make_pair(agility,     0);
        table->stamina = std::make_pair(stamina,     0);
        table->intellect = std::make_pair(intellect, 0);
        table->spirit = std::make_pair(spirit,       0);

        return table;
    }

    static Stat ParseStat(std::string&& stat)
    {
        if (strength.find(stat) != strength.end())
        {
            return dnd_commandscript::Stat::Strength;
        }
        else if (agility.find(stat) != agility.end())
        {
            return dnd_commandscript::Stat::Agility;
        }
        else if (stamina.find(stat) != stamina.end())
        {
            return dnd_commandscript::Stat::Stamina;
        }
        else if (intellect.find(stat) != intellect.end())
        {
            return dnd_commandscript::Stat::Intellect;
        }
        else if (spirit.find(stat) != spirit.end())
        {
            return dnd_commandscript::Stat::Spirit;
        }
        else
        {
            return dnd_commandscript::Stat::Nothing;
        }
    }

    static std::pair<int32, int32> GetStatAndProf(std::unique_ptr<dnd_bonus_table> table, Stat stat)
    {
        switch (stat)
        {
        case dnd_commandscript::Stat::Strength:
            return table->strength;
        case dnd_commandscript::Stat::Agility:
            return table->agility;
        case dnd_commandscript::Stat::Stamina:
            return table->stamina;
        case dnd_commandscript::Stat::Intellect:
            return table->intellect;
        case dnd_commandscript::Stat::Spirit:
            return table->spirit;
        case dnd_commandscript::Stat::Nothing:
            return std::make_pair(0, 0);
        default:
            return std::make_pair(0, 0);
        }
    }

    static std::string PrintStat(Stat stat)
    {
        switch (stat)
        {
        case dnd_commandscript::Stat::Strength:
            return "strength";
        case dnd_commandscript::Stat::Agility:
            return "agility";
        case dnd_commandscript::Stat::Stamina:
            return "stamina";
        case dnd_commandscript::Stat::Intellect:
            return "intellect";
        case dnd_commandscript::Stat::Spirit:
            return "spirit";
        case dnd_commandscript::Stat::Nothing:
            return "nothing";
        default:
            return "nothing";
        }
    }

    static std::string ToLower(std::string&& text)
    {
        auto result = std::string();

        for (auto&& item : text)
        {
            result.push_back(std::tolower(item));
        }

        return result;
    }

    static std::pair<int32, int32> XMod(std::pair<int32, int32>&& statAndProf)
    {
        return std::make_pair(std::max((statAndProf.first - 50) / 5, 0), std::move(statAndProf.second));
    }

    static std::vector<Player*> GetGroupPlayers(Player* player) {
        auto group = player->GetGroup();
        auto result = std::vector<Player*>();
        result.push_back(player);

        if (group == nullptr)
        {
            return result;
        }

        auto member = group->GetFirstMember();
        result.push_back(member->GetSource());
        while (member->hasNext())
        {
            member = member->next();
            result.push_back(member->GetSource());
        }

        return result;
    }

    template<typename... Args>
    static WorldPacket BuildPacket(ChatHandler* handler, uint32 entry, Args&&... args)
    {
        std::string message{ handler->PGetParseString(entry, std::forward<Args>(args)...).c_str() };
        WorldPacket data;
        handler->BuildChatPacket(data, CHAT_MSG_SYSTEM, LANG_UNIVERSAL, nullptr, nullptr, message);
        return data;
    }

    static bool HandleDndRollStatCommand(ChatHandler* handler, char const* args)
    {
        auto player = handler->GetSession()->GetPlayer();
        auto table = GetBonusTable(player);

        if (table == nullptr)
        {
            handler->PSendSysMessage(LANG_COMMAND_DND_ROLL_STAT_ERROR);
            handler->SetSentErrorMessage(true);
            return false;
        }

        auto stat = ParseStat(ToLower(std::string(args)));
        if (stat == dnd_commandscript::Stat::Nothing)
        {
            handler->PSendSysMessage(LANG_COMMAND_DND_ROLL_STAT_ERROR);
            handler->SetSentErrorMessage(true);
            return false;
        }

        auto rolled = irand(1, 20);
        auto values = XMod(GetStatAndProf(std::move(table), stat));
        auto total = rolled + values.first;
        auto stat_print = PrintStat(stat);

        auto players = GetGroupPlayers(player);
        auto message = BuildPacket(handler, LANG_COMMAND_DND_ROLL_STAT, handler->GetNameLink(player).c_str(), stat_print.c_str(), total, rolled, values.first);
        for (auto&& item : players)
        {
            item->GetSession()->SendPacket(&message);
        }

        return true;
    }

    static int32 GetHit(dnd_bonus_table const& table, Hit hit)
    {
        switch (hit)
        {
        case dnd_commandscript::Hit::Melee:
            return table.melee_hit;
        case dnd_commandscript::Hit::Ranged:
            return table.ranged_hit;
        case dnd_commandscript::Hit::Spell:
            return table.spell_hit;
        default:
            throw new std::logic_error("Using dnd_commandscript::Hit that was not implemented.");
        }
    }

    static bool HandleDndRollHitCommand(ChatHandler* handler, char const* args, Hit hit)
    {
        auto player = handler->GetSession()->GetPlayer();
        auto table = GetBonusTable(player);

        if (table == nullptr)
        {
            handler->PSendSysMessage(LANG_COMMAND_DND_ROLL_STAT_ERROR);
            handler->SetSentErrorMessage(true);
            return false;
        }

        auto stat = ParseStat(ToLower(std::string(args)));
        if (stat == dnd_commandscript::Stat::Nothing)
        {
            handler->PSendSysMessage(LANG_COMMAND_DND_ROLL_STAT_ERROR);
            handler->SetSentErrorMessage(true);
            return false;
        }

        auto rolled = irand(1, 20);
        auto hit_bonus = GetHit(*table, hit);
        auto values = XMod(GetStatAndProf(std::move(table), stat));
        auto total = rolled + values.first;
        auto stat_print = PrintStat(stat);

        auto players = GetGroupPlayers(player);
        auto message = BuildPacket(handler, LANG_COMMAND_DND_ROLL_STAT_HIT, handler->GetNameLink(player).c_str(), stat_print.c_str(), total, rolled, values.first, hit_bonus);
        for (auto&& item : players)
        {
            item->GetSession()->SendPacket(&message);
        }

        return true;
    }

    static bool HandleDndRollMeleeCommand(ChatHandler* handler, char const* args)
    {
        return HandleDndRollHitCommand(handler, args, Hit::Melee);
    }

    static bool HandleDndRollRangedCommand(ChatHandler* handler, char const* args)
    {
        return HandleDndRollHitCommand(handler, args, Hit::Ranged);
    }

    static bool HandleDndRollSpellCommand(ChatHandler* handler, char const* args)
    {
        return HandleDndRollHitCommand(handler, args, Hit::Spell);
    }
};

std::unordered_set<std::string> dnd_commandscript::strength = std::unordered_set<std::string>{ "str", "strength" };
std::unordered_set<std::string> dnd_commandscript::agility = std::unordered_set<std::string>{ "a", "ag", "agi", "agility" };
std::unordered_set<std::string> dnd_commandscript::stamina = std::unordered_set<std::string>{ "sta", "stam", "stamina" };
std::unordered_set<std::string> dnd_commandscript::intellect = std::unordered_set<std::string>{ "i", "int", "intellect" };
std::unordered_set<std::string> dnd_commandscript::spirit = std::unordered_set<std::string>{ "sp", "spi", "spir", "spirit" };

void AddSC_dnd_commandscript()
{
    new dnd_commandscript();
}
