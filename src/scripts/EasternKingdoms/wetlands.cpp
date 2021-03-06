/*
 * This file is part of the BlizzLikeCore Project.
 * See CREDITS and LICENSE files for Copyright information.
 */

/* ScriptData
Name: Wetlands
Complete(%): 100
Comment: Quest support: 1249
Category: Wetlands
EndScriptData */

/* ContentData
npc_mikhail
npc_tapoke_slim_jahn
EndContentData */

#include "ScriptPCH.h"
#include "ScriptedEscortAI.h"

/*######
## npc_tapoke_slim_jahn
######*/

enum eTapokeSlim
{
    QUEST_MISSING_DIPLO_PT11    = 1249,
    FACTION_ENEMY               = 168,
    SPELL_STEALTH               = 1785,
    SPELL_CALL_FRIENDS          = 16457,                    //summons 1x friend
    NPC_SLIMS_FRIEND            = 4971,
    NPC_TAPOKE_SLIM_JAHN        = 4962
};

struct npc_tapoke_slim_jahnAI : public npc_escortAI
{
    npc_tapoke_slim_jahnAI(Creature* pCreature) : npc_escortAI(pCreature) { }

    bool m_bFriendSummoned;

    void Reset()
    {
        if (!HasEscortState(STATE_ESCORT_ESCORTING))
            m_bFriendSummoned = false;
    }

    void WaypointReached(uint32 uiPointId)
    {
        switch (uiPointId)
        {
            case 2:
                if (me->HasStealthAura())
                    me->RemoveAurasDueToSpell(SPELL_AURA_MOD_STEALTH);

                SetRun();
                me->setFaction(FACTION_ENEMY);
                break;
        }
    }

    void EnterCombat(Unit* /*pWho*/)
    {
        Player* pPlayer = GetPlayerForEscort();

        if (HasEscortState(STATE_ESCORT_ESCORTING) && !m_bFriendSummoned && pPlayer)
        {
            for (uint8 i = 0; i < 3; ++i)
                DoCast(me, SPELL_CALL_FRIENDS, true);

            m_bFriendSummoned = true;
        }
    }

    void JustSummoned(Creature* pSummoned)
    {
        if (Player* pPlayer = GetPlayerForEscort())
            pSummoned->AI()->AttackStart(pPlayer);
    }

    void AttackedBy(Unit* pAttacker)
    {
        if (me->getVictim())
            return;

        if (me->IsFriendlyTo(pAttacker))
            return;

        AttackStart(pAttacker);
    }

    void DamageTaken(Unit* /*pDoneBy*/, uint32& uiDamage)
    {
        if (me->GetHealth()*100 < me->GetMaxHealth()*20)
        {
            if (Player* pPlayer = GetPlayerForEscort())
            {
                if (pPlayer->GetTypeId() == TYPEID_PLAYER)
                    CAST_PLR(pPlayer)->GroupEventHappens(QUEST_MISSING_DIPLO_PT11, me);

                uiDamage = 0;

                me->RestoreFaction();
                me->RemoveAllAuras();
                me->DeleteThreatList();
                me->CombatStop(true);

                SetRun(false);
            }
        }
    }
};

CreatureAI* GetAI_npc_tapoke_slim_jahn(Creature* pCreature)
{
    return new npc_tapoke_slim_jahnAI(pCreature);
}

/*######
## npc_mikhail
######*/

bool QuestAccept_npc_mikhail(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_MISSING_DIPLO_PT11)
    {
        Creature* pSlim = pCreature->FindNearestCreature(NPC_TAPOKE_SLIM_JAHN, 25.0f);

        if (!pSlim)
            return false;

        if (!pSlim->HasStealthAura())
            pSlim->CastSpell(pSlim, SPELL_STEALTH, true);

        if (npc_tapoke_slim_jahnAI* pEscortAI = CAST_AI(npc_tapoke_slim_jahnAI, pSlim->AI()))
            pEscortAI->Start(false, false, pPlayer->GetGUID(), pQuest);
    }
    return false;
}

/*######
## AddSC
######*/

void AddSC_wetlands()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_tapoke_slim_jahn";
    newscript->GetAI = &GetAI_npc_tapoke_slim_jahn;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_mikhail";
    newscript->pQuestAccept = &QuestAccept_npc_mikhail;
    newscript->RegisterSelf();
}
