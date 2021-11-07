/***********************************************************************************
*                                                                                  *
*                              Created by,                                         *
*                              Matej Lyran Kucera                                  *
*                                                                                  *
************************************************************************************/
#include "ScriptMgr.h"


class xray_on_login : public PlayerScript
{
    public:
        xray_on_login() : PlayerScript("xray_on_login") { }

    void OnMapChanged(Player* player)
    {
            // xray spell id = 54844
            player->CastSpell(player, 54844, false);
            player->RemoveAurasDueToSpell(54844);
    }
};

void AddSC_xray_on_login()
{
        new xray_on_login();
}
