#include "ScriptMgr.h"
#include "DatabaseEnv.h"
#include "Player.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"


class npc_managed_guild : public CreatureScript
{
public:
    npc_managed_guild() : CreatureScript("npc_managed_guild") { }

    struct npc_managed_guildAI : public ScriptedAI
    {

        npc_managed_guildAI(Creature* creature) : ScriptedAI(creature)
        {
            
        }

        void OnQuestReward(Player* player, Quest const* quest, uint32 /*opt*/) override
        {
            // check if there is guild invite associated
            WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_MANAGED_GUILD_QUEST_INVITE);
            stmt->setUInt32(0, quest->GetQuestId());
            PreparedQueryResult result = WorldDatabase.Query(stmt);

            // if the quest is "INVITE QUEST"
            if (result)
            {
                if(player->GetGuild()) // check if the player has no guild
                    return; 

                Field* fields  = result->Fetch();
                uint32 guildId = fields[0].GetUInt32();
                uint32 rankId  = fields[1].GetUInt32();

                Guild* targetGuild = sGuildMgr->GetGuildById(guildId);
                if (!targetGuild)
                    return;

                // player's guild membership checked in AddMember before add
                CharacterDatabaseTransaction trans(nullptr);
                targetGuild->AddMember(trans, player->GetGUID(), rankId);

                return;
            }

            WorldDatabasePreparedStatement* stmtLeave = WorldDatabase.GetPreparedStatement(WORLD_SEL_MANAGED_GUILD_QUEST_LEAVE);
            stmtLeave->setUInt32(0, quest->GetQuestId());
            PreparedQueryResult resultLeave = WorldDatabase.Query(stmtLeave);

            // if the quest is "LEAVE QUEST"
            if (resultLeave)
            {
                if(!player->GetGuild()) // check if the player has no guild
                    return; 

                Field* fields          = resultLeave->Fetch();
                uint32 guildId              = fields[0].GetUInt32();

                Guild* targetGuild = sGuildMgr->GetGuildById(guildId);
                if (!targetGuild)
                    return;

                // player's guild membership checked in AddMember before add
                CharacterDatabaseTransaction trans(nullptr);
                targetGuild->DeleteMember(trans, player->GetGUID(), false, false, true);
            }
                
           
        }

    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_managed_guildAI(creature);
    }
};

void AddSC_npc_managed_guild()
{
    new npc_managed_guild();
}
