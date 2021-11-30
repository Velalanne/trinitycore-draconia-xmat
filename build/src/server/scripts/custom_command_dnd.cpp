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
#include <tuple>
#include <vector>
#include <memory>
#include <stdexcept>

#if TRINITY_COMPILER == TRINITY_COMPILER_GNU
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

struct dnd_proficiency
{
    uint32 id;
    uint32 class_id;
    uint32 level;
    int32 strength;
    int32 dexterity;
    int32 constitution;
    int32 intelligence;
    int32 wisdom;
    int32 charisma;
};

struct dnd_class
{
    uint32 id;
    std::string name;
};

struct dnd_race
{
    uint32 id;
    std::string name;
    int32 strength;
    int32 dexterity;
    int32 constitution;
    int32 intelligence;
    int32 wisdom;
    int32 charisma;
};

struct dnd_item
{
    uint32 id;
    int32 melee_hit;
    int32 ranged_hit;
    int32 spell_hit;
    int32 strength;
    int32 dexterity;
    int32 constitution;
    int32 intelligence;
    int32 wisdom;
    int32 charisma;
};

struct dnd_character
{
    uint32 id;
    uint32 race_id;
    uint32 class_id;
    uint32 level;
};

struct dnd_bonus_table
{
    int32 melee_hit;
    int32 ranged_hit;
    int32 spell_hit;
    std::pair<int32, int32> strength; //stat, prof
    std::pair<int32, int32> dexterity; //stat, prof
    std::pair<int32, int32> constitution; //stat, prof
    std::pair<int32, int32> intelligence; //stat, prof
    std::pair<int32, int32> wisdom; //stat, prof
    std::pair<int32, int32> charisma;
};

class dnd_commandscript : public CommandScript
{
private:
    enum class Hit : char {Melee, Ranged, Spell};
    enum class Stat : char { Strength, Dexterity, Constitution, Intelligence, Wisdom, Charisma, Nothing };

    static std::unordered_set<std::string> strength;
    static std::unordered_set<std::string> dexterity;
    static std::unordered_set<std::string> constitution;
    static std::unordered_set<std::string> intelligence;
    static std::unordered_set<std::string> wisdom;
    static std::unordered_set<std::string> charisma;

public:
    dnd_commandscript() : CommandScript("dnd_commandscript")
    {
        strength = std::unordered_set<std::string>{ "s", "str", "strength" };
        dexterity = std::unordered_set<std::string>{ "d", "dex", "dexterity" };
        constitution = std::unordered_set<std::string>{ "co", "con", "constitution" };
        intelligence = std::unordered_set<std::string>{ "i", "int", "intelligence" };
        wisdom = std::unordered_set<std::string>{ "w", "wis", "wisdom" };
        charisma = std::unordered_set<std::string>{ "ch", "cha", "char", "charisma" };
    }

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
            if (std::isspace(text[start]))
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
        auto dices_str = dice.substr(0, d_position - 1);
        auto dices = std::stoi(dices_str);
        auto faces_str = dice.substr(d_position);
        auto faces = std::stoi(faces_str);
        return std::pair(dices, faces);
    }

    static std::tuple<int, int, int> ParseDiceCommand(char const* args)
    {
        auto text = std::string(args);
        auto dice_pair = SplitByWhitespace(text, 0);
        auto dice = ParseDiceDescription(dice_pair.first);
        auto bonus_pair = SplitByWhitespace(text, dice_pair.second);
        auto bonus = std::stoi(bonus_pair.first);

        return std::tuple(dice.first, dice.second, bonus);
    }

    static bool HandleDndRollDiceCommand(ChatHandler* handler, char const* args)
    {
        auto dice = ParseDiceCommand(args);
        auto rolled = 0;
        for (std::size_t i = 0; i < std::get<0>(dice); ++i)
        {
            rolled = irand(1, std::get<1>(dice));
        }

        auto bonus = std::get<2>(dice);
        auto result = rolled + bonus;

        auto player = handler->GetSession()->GetPlayer();

        handler->PSendSysMessage(LANG_COMMAND_DND_ROLL_DICE, handler->GetNameLink(player).c_str(), result, bonus);
        return true;
    }

    static std::unique_ptr<dnd_character> GetCharacter(uint32 character_id)
    {
        auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_DND_CHARACTER);
        stmt->setUInt32(0, character_id);
        auto result = CharacterDatabase.Query(stmt);

        if (!result)
        {
            return nullptr;
        }

        do
        {
            auto field = result->Fetch();

            auto character = std::make_unique<dnd_character>();
            character->id = field[0].GetUInt32();
            character->race_id = field[1].GetUInt32();
            character->class_id = field[2].GetUInt32();
            character->level = field[3].GetUInt32();
            return character;

        } while (result->NextRow());

        return nullptr;
    }

    static std::unique_ptr<dnd_race> GetRace(uint32 dnd_race_id)
    {
        auto stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_DND_RACE);
        stmt->setUInt32(0, dnd_race_id);
        auto result = WorldDatabase.Query(stmt);

        if (!result)
        {
            return nullptr;
        }

        do
        {
            auto field = result->Fetch();

            auto race = std::make_unique<dnd_race>();
            race->id = field[0].GetUInt32();
            race->name = field[1].GetString();
            race->strength = field[2].GetUInt32();
            race->dexterity = field[3].GetUInt32();
            race->constitution = field[4].GetUInt32();
            race->intelligence = field[5].GetUInt32();
            race->wisdom = field[6].GetUInt32();
            race->charisma = field[7].GetUInt32();
            return race;

        } while (result->NextRow());

        return nullptr;
    }

    static std::unique_ptr<dnd_proficiency> GetProficiency(uint32 dnd_class_id, uint32 dnd_level)
    {
        auto stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_DND_PROFICIENCY);
        stmt->setUInt32(0, dnd_class_id);
        auto result = WorldDatabase.Query(stmt);

        if (!result)
        {
            return nullptr;
        }

        do
        {
            auto field = result->Fetch();

            auto proficiency = std::make_unique<dnd_proficiency>();
            proficiency->id = field[0].GetUInt32();
            proficiency->class_id = field[1].GetUInt32();
            proficiency->level = field[2].GetUInt32();
            proficiency->strength = field[3].GetUInt32();
            proficiency->dexterity = field[4].GetUInt32();
            proficiency->constitution = field[5].GetUInt32();
            proficiency->intelligence = field[6].GetUInt32();
            proficiency->wisdom = field[7].GetUInt32();
            proficiency->charisma = field[8].GetUInt32();

            if (proficiency->level == dnd_level)
            {
                return proficiency;
            }

        } while (result->NextRow());

        return nullptr;
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

    static std::unique_ptr<dnd_item> GetItem(uint32 item_id)
    {
        auto stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_DND_ITEM);
        stmt->setUInt32(0, item_id);
        auto result = WorldDatabase.Query(stmt);

        if (!result)
        {
            return nullptr;
        }

        do
        {
            auto field = result->Fetch();

            auto item = std::make_unique<dnd_item>();
            item->id = field[0].GetUInt32();
            item->melee_hit = field[1].GetUInt32();
            item->ranged_hit = field[2].GetUInt32();
            item->spell_hit = field[3].GetUInt32();
            item->strength = field[4].GetUInt32();
            item->dexterity = field[5].GetUInt32();
            item->constitution = field[6].GetUInt32();
            item->intelligence = field[7].GetUInt32();
            item->wisdom = field[8].GetUInt32();
            item->charisma = field[9].GetUInt32();

            return item;

        } while (result->NextRow());

        return nullptr;
    }

    static std::vector<std::unique_ptr<dnd_item>> GetItems(Player* player)
    {
        auto result = std::vector<std::unique_ptr<dnd_item>>();
        auto items = EquippedItems(player);

        for (auto&& item : items)
        {
            auto dnd = GetItem(item->GetGUID().GetCounter());
            if (dnd != nullptr)
            {
                result.push_back(std::move(dnd));
            }
        }

        return result;
    }

    static std::unique_ptr<dnd_bonus_table> Calculate(std::unique_ptr<dnd_proficiency>&& proficiency, std::unique_ptr<dnd_race>&& race, std::vector<std::unique_ptr<dnd_item>>&& items)
    {
        auto table = std::make_unique<dnd_bonus_table>();
        uint32 melee_hit    = 0;
        uint32 ranged_hit   = 0;
        uint32 spell_hit    = 0;
        uint32 strength     = 0;
        uint32 dexterity    = 0;
        uint32 constitution = 0;
        uint32 intelligence = 0;
        uint32 wisdom       = 0;
        uint32 charisma     = 0;

        for (auto&& item : items)
        {
            melee_hit += item->melee_hit;
            ranged_hit += item->ranged_hit;
            spell_hit += item->spell_hit;
            strength += item->strength;
            dexterity += item->dexterity;
            constitution += item->constitution;
            intelligence += item->intelligence;
            wisdom += item->wisdom;
            charisma += item->charisma;
        }

        strength += race->strength;
        dexterity += race->dexterity;
        constitution += race->constitution;
        intelligence += race->intelligence;
        wisdom += race->wisdom;
        charisma += race->charisma;

        table->melee_hit    = melee_hit;
        table->ranged_hit   = ranged_hit;
        table->spell_hit    = spell_hit;
        table->strength     = std::make_pair(strength, proficiency->strength);
        table->dexterity    = std::make_pair(dexterity, proficiency->dexterity);
        table->constitution = std::make_pair(constitution, proficiency->constitution);
        table->intelligence = std::make_pair(intelligence, proficiency->intelligence);
        table->wisdom       = std::make_pair(wisdom, proficiency->wisdom);
        table->charisma     = std::make_pair(charisma, proficiency->charisma);

        return table;
    }

    static std::unique_ptr<dnd_bonus_table> GetBonusTable(Player* player)
    {
        auto character = GetCharacter(player->GetGUID().GetCounter());
        if (character == nullptr)
        {
            return nullptr;
        }

        auto race = GetRace(character->race_id);
        if (race == nullptr)
        {
            return nullptr;
        }

        auto proficiency = GetProficiency(character->class_id, character->level);
        if (proficiency == nullptr)
        {
            return nullptr;
        }

        auto items = GetItems(player);
        return Calculate(std::move(proficiency), std::move(race), std::move(items));
    }

    static Stat ParseStat(std::string&& stat)
    {
        if (strength.find(stat) != strength.end())
        {
            return dnd_commandscript::Stat::Strength;
        }
        else if (dexterity.find(stat) != dexterity.end())
        {
            return dnd_commandscript::Stat::Dexterity;
        }
        else if (constitution.find(stat) != constitution.end())
        {
            return dnd_commandscript::Stat::Constitution;
        }
        else if (intelligence.find(stat) != intelligence.end())
        {
            return dnd_commandscript::Stat::Intelligence;
        }
        else if (wisdom.find(stat) != wisdom.end())
        {
            return dnd_commandscript::Stat::Wisdom;
        }
        else if (charisma.find(stat) != charisma.end())
        {
            return dnd_commandscript::Stat::Charisma;
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
        case dnd_commandscript::Stat::Dexterity:
            return table->dexterity;
        case dnd_commandscript::Stat::Constitution:
            return table->constitution;
        case dnd_commandscript::Stat::Intelligence:
            return table->intelligence;
        case dnd_commandscript::Stat::Wisdom:
            return table->wisdom;
        case dnd_commandscript::Stat::Charisma:
            return table->charisma;
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
        case dnd_commandscript::Stat::Dexterity:
            return "dexterity";
        case dnd_commandscript::Stat::Constitution:
            return "sconstitution";
        case dnd_commandscript::Stat::Intelligence:
            return "intelligence";
        case dnd_commandscript::Stat::Wisdom:
            return "wisdom";
        case dnd_commandscript::Stat::Charisma:
            return "charisma";
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
        return std::make_pair((statAndProf.first - 10) / 2, std::move(statAndProf.second));
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

        handler->PSendSysMessage(LANG_COMMAND_DND_ROLL_STAT, handler->GetNameLink(player).c_str(), stat_print, total, rolled, values.first, values.second);
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
        auto hit = GetHit(*table, hit);
        auto values = XMod(GetStatAndProf(std::move(table), stat));
        auto total = rolled + values.first;
        auto stat_print = PrintStat(stat);

        handler->PSendSysMessage(LANG_COMMAND_DND_ROLL_STAT, handler->GetNameLink(player).c_str(), stat_print, total, rolled, values.first, values.second, hit);
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

void AddSC_dnd_commandscript()
{
    new dnd_commandscript();
}
