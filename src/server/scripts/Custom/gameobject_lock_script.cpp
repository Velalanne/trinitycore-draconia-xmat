#include "Player.h"
#include "ScriptMgr.h"
#include "GameObject.h"
#include "GameObjectAI.h"
#include "Group.h"
#include "ObjectGuid.h"
#include "DatabaseEnv.h"
#include "DatabaseEnvFwd.h"
#include "Log.h"
#include "AllPackets.h"
#include "ChatPackets.h"
#include "GameTime.h"
#include <unordered_set>
#include <vector>

struct gameobject_lock_entry
{
    uint32 id;
    uint32 gob_guid;
    uint32 char_entry;
    uint8 lock_id;
    float x;
    float y;
    float z;
    float orientation;
    uint32 map_id;
    uint32 duration_open;
};

class gameobject_lock_script : public GameObjectScript
{
public:
    gameobject_lock_script() : GameObjectScript("gameobject_lock_script") {}

    struct id_lockAI : public GameObjectAI
    {
        id_lockAI(GameObject* go) : GameObjectAI(go) { }

    private:
        uint8 const is_group = 1 << 0;
        uint8 const is_open = 1 << 1;
        uint8 const is_buy = 1 << 2;
        uint8 const can_expire = 1 << 3;

        std::unordered_set<uint32> read_group_members(Player* player)
        {
            auto members = std::unordered_set<uint32>();
            members.insert(player->GetGUID().GetCounter());
            auto group = player->GetGroup();

            if (group)
            {
                auto member = group->GetFirstMember();

                members.insert(member->GetSource()->GetGUID().GetCounter());
                while (member->hasNext())
                {
                    member = member->next();
                    members.insert(member->GetSource()->GetGUID().GetCounter());
                }
            }

            return members;
        }

        std::vector<gameobject_lock_entry> read_id_lock_entries()
        {
            auto gob_guid = me->GetSpawnId();
            auto stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_GAMEOBJECT_LOCK);
            stmt->setUInt32(0, gob_guid);
            auto result = WorldDatabase.Query(stmt);
            auto locks = std::vector<gameobject_lock_entry>();

            if (!result)
            {
                return locks;
            }

            do
            {
                auto field = result->Fetch();

                auto lock = gameobject_lock_entry();
                lock.id = field[0].GetUInt32();
                lock.gob_guid = field[1].GetUInt32();
                lock.char_entry = field[2].GetUInt32();
                lock.lock_id = field[3].GetUInt8();;
                lock.x = field[4].GetFloat();
                lock.y = field[5].GetFloat();
                lock.z = field[6].GetFloat();
                lock.orientation = field[7].GetFloat();
                lock.map_id = field[8].GetUInt32();
                lock.duration_open = field[9].GetUInt32();
                locks.push_back(lock);

            } while (result->NextRow());

            return locks;
        }

        bool Open(gameobject_lock_entry const& entry)
        {
            me->UseDoorOrButton(entry.duration_open);
            return true;
        }

        bool Teleport(gameobject_lock_entry const& entry, Player* player)
        {
            player->TeleportTo(entry.map_id, entry.x, entry.y, entry.z, entry.orientation);
            return true;
        }

        bool Deny(Player* player)
        {
            WorldPackets::Chat::ChatServerMessage packet;
            packet.MessageID = int32(3);
            packet.StringParam = "You cannot use that.";

            player->SendDirectMessage(packet.Write());

            return true;
        }

        bool Act(gameobject_lock_entry const& entry, Player* player)
        {
            if (is_open & entry.lock_id)
            {
                return Open(entry);
            }
            else
            {
                return Teleport(entry, player);
            }
        }

    public:

        bool OnGossipHello(Player* player) override
        {
            auto members = read_group_members(player);
            auto locks = read_id_lock_entries();

            for (auto&& lock : locks)
            {
                if (lock.char_entry == 0)
                {
                    return Act(lock, player);
                }
                else if (!(is_group & lock.lock_id) && (player->GetGuildId() == lock.char_entry))
                {
                    return Act(lock, player);
                }
                else if ((is_group & lock.lock_id) && (members.find(lock.char_entry) != members.end()))
                {
                    return Act(lock, player);
                }
            }

            return Deny(player);
        }
    };

    GameObjectAI* GetAI(GameObject* go) const override
    {
        return new id_lockAI(go);
    }
};

void AddSC_gameobject_lock_script()
{
    new gameobject_lock_script();
}
