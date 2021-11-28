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
#include <tuple>
#include <vector>
#include <memory>

#if TRINITY_COMPILER == TRINITY_COMPILER_GNU
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

struct dnd_proficiency
{
    uint32 id;
    uint32 class_id;
    uint32 level;
    uint32 strength;
    uint32 dexterity;
    uint32 constitution;
    uint32 intelligence;
    uint32 wisdom;
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
    uint32 strength;
    uint32 dexterity;
    uint32 constitution;
    uint32 intelligence;
    uint32 wisdom;
};

struct dnd_item
{
    uint32 id;
    uint32 melee_hit;
    uint32 ranged_hit;
    uint32 spell_hit;
    uint32 strength;
    uint32 dexterity;
    uint32 constitution;
    uint32 intelligence;
    uint32 wisdom;
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
    uint32 melee_hit;
    uint32 ranged_hit;
    uint32 spell_hit;
    std::pair<uint32, uint32> strength; //stat, prof
    std::pair<uint32, uint32> dexterity; //stat, prof
    std::pair<uint32, uint32> constitution; //stat, prof
    std::pair<uint32, uint32> intelligence; //stat, prof
    std::pair<uint32, uint32> wisdom; //stat, prof
};

class dnd_commandscript : public CommandScript
{
public:
    dnd_commandscript() : CommandScript("dnd_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> dndRollCommandTable =
        {
            //{ "melee",   rbac::RBAC_PERM_DND_ROLL_MELEE,  false, &HandleModifyArenaCommand, "" },
            //{ "ranged",  rbac::RBAC_PERM_DND_ROLL_RANGED, false, &HandleModifyArenaCommand, "" },
            //{ "spell",   rbac::RBAC_PERM_DND_ROLL_SPELL,  false, &HandleModifyArenaCommand, "" },
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
        //TODO
    }

    static std::unique_ptr<dnd_race> GetRace(uint32 dnd_race_id)
    {
        //TODO
    }

    static std::unique_ptr<dnd_proficiency> GetProficiency(uint32 dnd_class_id, uint32 dnd_level)
    {
        //TODO
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
        //TODO
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

    static std::unique_ptr<dnd_bonus_table> Calculate(std::unique_ptr<dnd_proficiency>&& proficiency, std::unique_ptr<dnd_race>&& race, std::vector<std::unique_ptr<dnd_item>> items)
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
        }

        strength += race->strength;
        dexterity += race->dexterity;
        constitution += race->constitution;
        intelligence += race->intelligence;
        wisdom += race->wisdom;

        table->melee_hit    = melee_hit;
        table->ranged_hit   = ranged_hit;
        table->spell_hit    = spell_hit;
        table->strength     = std::make_pair(strength, proficiency->strength);
        table->dexterity    = std::make_pair(dexterity, proficiency->dexterity);
        table->constitution = std::make_pair(constitution, proficiency->constitution);
        table->intelligence = std::make_pair(intelligence, proficiency->intelligence);
        table->wisdom       = std::make_pair(wisdom, proficiency->wisdom);

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
        return Calculate(std::move(proficiency), std::move(race), items);
    }

    static bool HandleDndRollStatCommand(ChatHandler* handler, char const* args)
    {
        auto player = handler->GetSession()->GetPlayer();
        auto table = GetBonusTable(player);

        if (table == nullptr)
        {
            //TODO
        }
        //TODO
    }
};

void AddSC_dnd_commandscript()
{
    new dnd_commandscript();
}
