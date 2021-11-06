#include "Player.h"
#include "ScriptMgr.h"
#include "GameObject.h"
#include "GameObjectAI.h"
#include "Group.h"
#include "ObjectGuid.h"
#include "DatabaseEnv.h"
#include "DatabaseEnvFwd.h"
#include "Log.h"
#include "WorldPacket.h"
#include <unordered_set>
#include <vector>

struct id_lock_entry
{
    uint32 guid;
    uint32 gob_entry;
    uint32 char_entry;
    uint8 lock_id;
    float x;
    float y;
    float z;
    float orientation;
    uint32 map;
    uint32 price;
    uint32 duration_open;
    uint32 duration_owner;
    uint64 obtained;
};

class id_lock_script : public GameObjectScript
{
public:
    id_lock_script() : GameObjectScript("id_lock_script") {}

    struct id_lockAI : public GameObjectAI
    {
        id_lockAI(GameObject* go) : GameObjectAI(go) { }

    private:
        uint8 const is_group = 1 << 0;
        uint8 const is_open = 1 << 1;
        uint8 const is_buy = 1 << 2;

        std::unordered_set<uint32> read_group_members(Player* player)
        {
            auto members = std::unordered_set<uint32>();
            auto group = player->GetGroup();
            auto member = group->GetFirstMember();

            members.insert(member->GetSource()->GetGUID().GetEntry());
            while (member->hasNext())
            {
                member = member->next();
                members.insert(member->GetSource()->GetGUID().GetEntry());
            }

            return members;
        }

        std::vector<id_lock_entry> read_id_lock_entries()
        {
            auto gob_guid = me->GetGUID().GetEntry();
            auto stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_ID_LOCK);
            stmt->setUInt32(0, gob_guid);
            auto result = WorldDatabase.Query(stmt);
            auto locks = std::vector<id_lock_entry>();

            if (!result)
            {
                return locks;
            }

            do
            {
                auto field = result->Fetch();

                auto lock = id_lock_entry();
                lock.guid = field[0].GetUInt32();
                lock.gob_entry = field[1].GetUInt32();
                lock.char_entry = field[2].GetUInt32();
                lock.lock_id = field[3].GetUInt8();;
                lock.x = field[4].GetFloat();
                lock.y = field[5].GetFloat();
                lock.z = field[6].GetFloat();
                lock.orientation = field[7].GetFloat();
                lock.map = field[8].GetUInt32();
                lock.price = field[9].GetUInt32();
                lock.duration_open = field[10].GetUInt32();
                lock.duration_owner = field[11].GetUInt32();
                lock.obtained = field[12].GetUInt64();;
                locks.push_back(lock);

            } while (result->NextRow());

            return locks;
        }

        bool is_expired(id_lock_entry const& entry)
        {

        }

        bool Open(id_lock_entry const& entry)
        {
            me->UseDoorOrButton(entry.duration_open);
            return false;
        }

        bool Teleport(id_lock_entry const& entry, Player* player)
        {
            player->TeleportTo(entry.map, entry.x, entry.y, entry.z, entry.orientation);
            return false;
        }

        bool Buy(Player* player)
        {
        }

        bool Act(id_lock_entry const& entry, Player* player)
        {
            if (!is_expired(entry))
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
            else if (is_buy & entry.lock_id)
            {
                return Buy(player);
            }
            else
            {
                return Deny(player);
            }
        }

        bool Deny(Player* player)
        {
            player->SendLootError(me->GetGUID(), LootError::LOOT_ERROR_LOCKED);
            return false;
        }

    public:

        bool OnGossipHello(Player* player) override
        {
            auto members = read_group_members(player);
            auto guild = player->GetGuildId();
            auto locks = read_id_lock_entries();

            for (auto&& lock : locks)
            {
                if (members.find(lock.char_entry) != members.end())
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

void AddSC_id_lock_script()
{
    new id_lock_script();
}
