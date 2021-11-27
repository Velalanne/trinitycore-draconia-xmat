/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

 /* ScriptData
 Name: modify_commandscript
 %Complete: 100
 Comment: All modify related commands
 Category: commandscripts
 EndScriptData */

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
#include <tuple>;
#include <vector>;

#if TRINITY_COMPILER == TRINITY_COMPILER_GNU
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

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
            //{ "stat",    rbac::RBAC_PERM_DND_ROLL_STAT,   false, &HandleDndRollStatCommand, "" },
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
};

void AddSC_dnd_commandscript()
{
    new dnd_commandscript();
}
