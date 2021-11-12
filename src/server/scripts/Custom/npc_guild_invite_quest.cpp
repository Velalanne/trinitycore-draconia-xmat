#include "ScriptMgr.h"
#include "DatabaseEnv.h"
#include "Player.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"


class npc_guild_invite_quest : public CreatureScript
{
public:
    npc_guild_invite_quest() : CreatureScript("npc_guild_invite_quest") { }

    struct npc_guild_invite_questAI : public ScriptedAI
    {

        npc_guild_invite_questAI(Creature* creature) : ScriptedAI(creature)
        {
           
        }

        void OnQuestReward(Player* player, Quest const* quest, uint32 /*opt*/) override
        {
            // check if there is guild invite associated
            WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_QUEST_GUILD_INVITE);
            stmt->setUInt32(0, quest->GetQuestId());
            PreparedQueryResult result = WorldDatabase.Query(stmt);
            if (!result)
            {
                return;
            }
                
            Field* fields  = result->Fetch();
            uint32 guildId = fields[0].GetUInt32();
            uint32 rankId  = fields[1].GetUInt32();


            Guild* targetGuild = sGuildMgr->GetGuildById(guildId);
            if (!targetGuild)
                return;

            // player's guild membership checked in AddMember before add
            CharacterDatabaseTransaction trans(nullptr);
            targetGuild->AddMember(trans, player->GetGUID(), rankId);
        }

    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_guild_invite_questAI(creature);
    }
};

void AddSC_npc_guild_invite_quest()
{
    new npc_guild_invite_quest();
}
