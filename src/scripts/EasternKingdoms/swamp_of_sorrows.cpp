/*
 * This file is part of the BlizzLikeCore Project.
 * See CREDITS and LICENSE files for Copyright information.
 */

/* ScriptData
Name: Swamp_of_Sorrows
Complete(%): 100
Comment: Quest support: 1393
Category: Swap of Sorrows
EndScriptData */

/* ContentData
npc_galen_goodward
EndContentData */

/*#########
##npc_galen_goodward
#########*/

#include "ScriptPCH.h"
#include "ScriptedEscortAI.h"

enum eGalen
{
    GILAN_SAY_START_1         = -1000582,
    GILAN_SAY_START_2         = -1000583,
    GILAN_SAY_UNDER_ATTACK_1  = -1000584,
    GILAN_SAY_UNDER_ATTACK_2  = -1000585,
    GILAN_SAY_END             = -1000586,
    GILAN_EMOTE_END_1         = -1000587,
    GILAN_EMOTE_END_2         = -1000588,

    QUEST_GALENS_ESCAPE       = 1393,
    GO_GALENS_CAGE            = 37118
};

struct npc_galen_goodwardAI : public npc_escortAI
{
    npc_galen_goodwardAI(Creature* pCreature) : npc_escortAI(pCreature) { }

    uint32 m_uiPostEventTimer;

    void Reset()
    {
        m_uiPostEventTimer = 0;
    }

    void EnterCombat(Unit* )
    {
        DoScriptText(RAND(GILAN_SAY_UNDER_ATTACK_1,GILAN_SAY_UNDER_ATTACK_2), me);
    }

    void WaypointReached(uint32 uiPointId)
    {
        Player* pPlayer = GetPlayerForEscort();
        if (!pPlayer)
            return;

        switch (uiPointId)
        {
        case 1:
            DoScriptText(GILAN_SAY_START_2, me);
            break;
        case 16:
            m_uiPostEventTimer = 10000;
            DoScriptText(GILAN_SAY_END, me, pPlayer);
            SetRun(true);
            if (Player* pPlayer = GetPlayerForEscort())
                pPlayer->GroupEventHappens(QUEST_GALENS_ESCAPE, me);
            break;
        case 17:
            DoScriptText(GILAN_EMOTE_END_2, me, pPlayer);
            break;
        }
    }

    void UpdateEscortAI(const uint32 uiDiff)
    {
        if (!UpdateVictim())
        {
            if (m_uiPostEventTimer && m_uiPostEventTimer <= uiDiff)
            {
                if (!me->getVictim() && me->isAlive())
                {
                    Player* pPlayer = GetPlayerForEscort();

                    DoScriptText(GILAN_EMOTE_END_1, me, pPlayer);
                    Reset();
                    return;
                }

            } else m_uiPostEventTimer -= uiDiff;

            return;
        }

        DoMeleeAttackIfReady();
    }

};

bool QuestAccept_npc_galen_goodward(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_GALENS_ESCAPE)
    {
        pCreature->setFaction(113);
        DoScriptText(GILAN_SAY_START_1, pCreature);
        pCreature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_OOC_NOT_ATTACKABLE);

        if (GameObject* pGo = pCreature->FindNearestGameObject(GO_GALENS_CAGE, INTERACTION_DISTANCE))
            pGo->UseDoorOrButton();

        if (npc_galen_goodwardAI* pEscortAI = CAST_AI(npc_galen_goodwardAI,pCreature->AI()))
            pEscortAI->Start(false, false, pPlayer->GetGUID(), pQuest);
    }
    return true;
}
CreatureAI* GetAI_npc_galen_goodward(Creature* pCreature)
{
    return new npc_galen_goodwardAI(pCreature);
}

void AddSC_swamp_of_sorrows()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_galen_goodward";
    newscript->GetAI = &GetAI_npc_galen_goodward;
    newscript->pQuestAccept = &QuestAccept_npc_galen_goodward;
    newscript->RegisterSelf();
}