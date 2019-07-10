/***************************************************************************
 *   Copyright (C) 2009 by Andrey Afletdinov <fheroes2@gmail.com>          *
 *                                                                         *
 *   Part of the Free Heroes2 Engine:                                      *
 *   http://sourceforge.net/projects/fheroes2                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#pragma once

#include <vector>
#include "ByteVectorReader.h"
#include "ByteVectorWriter.h"
#include "interface_itemsbar.h"
#include "display.h"
#include "sprites.h"

class Spell;

class Heroes;

class Artifact
{
public:
    enum level_t
    {
        ART_NONE = 0,
        ART_LEVEL1 = 0x01,
        ART_LEVEL2 = 0x02,
        ART_LEVEL3 = 0x04,
        ART_LEVEL123 = ART_LEVEL1 | ART_LEVEL2 | ART_LEVEL3,
        ART_ULTIMATE = 0x08,
        ART_LOYALTY = 0x10,
        ART_NORANDOM = 0x20
    };

    enum type_t
    {
        ULTIMATE_BOOK,
        ULTIMATE_SWORD,
        ULTIMATE_CLOAK,
        ULTIMATE_WAND,
        ULTIMATE_SHIELD,
        ULTIMATE_STAFF,
        ULTIMATE_CROWN,
        GOLDEN_GOOSE,
        ARCANE_NECKLACE,
        CASTER_BRACELET,
        MAGE_RING,
        WITCHES_BROACH,
        MEDAL_VALOR,
        MEDAL_COURAGE,
        MEDAL_HONOR,
        MEDAL_DISTINCTION,
        FIZBIN_MISFORTUNE,
        THUNDER_MACE,
        ARMORED_GAUNTLETS,
        DEFENDER_HELM,
        GIANT_FLAIL,
        BALLISTA,
        STEALTH_SHIELD,
        DRAGON_SWORD,
        POWER_AXE,
        DIVINE_BREASTPLATE,
        MINOR_SCROLL,
        MAJOR_SCROLL,
        SUPERIOR_SCROLL,
        FOREMOST_SCROLL,
        ENDLESS_SACK_GOLD,
        ENDLESS_BAG_GOLD,
        ENDLESS_PURSE_GOLD,
        NOMAD_BOOTS_MOBILITY,
        TRAVELER_BOOTS_MOBILITY,
        RABBIT_FOOT,
        GOLDEN_HORSESHOE,
        GAMBLER_LUCKY_COIN,
        FOUR_LEAF_CLOVER,
        TRUE_COMPASS_MOBILITY,
        SAILORS_ASTROLABE_MOBILITY,
        EVIL_EYE,
        ENCHANTED_HOURGLASS,
        GOLD_WATCH,
        SKULLCAP,
        ICE_CLOAK,
        FIRE_CLOAK,
        LIGHTNING_HELM,
        EVERCOLD_ICICLE,
        EVERHOT_LAVA_ROCK,
        LIGHTNING_ROD,
        SNAKE_RING,
        ANKH,
        BOOK_ELEMENTS,
        ELEMENTAL_RING,
        HOLY_PENDANT,
        PENDANT_FREE_WILL,
        PENDANT_LIFE,
        SERENITY_PENDANT,
        SEEING_EYE_PENDANT,
        KINETIC_PENDANT,
        PENDANT_DEATH,
        WAND_NEGATION,
        GOLDEN_BOW,
        TELESCOPE,
        STATESMAN_QUILL,
        WIZARD_HAT,
        POWER_RING,
        AMMO_CART,
        TAX_LIEN,
        HIDEOUS_MASK,
        ENDLESS_POUCH_SULFUR,
        ENDLESS_VIAL_MERCURY,
        ENDLESS_POUCH_GEMS,
        ENDLESS_CORD_WOOD,
        ENDLESS_CART_ORE,
        ENDLESS_POUCH_CRYSTAL,
        SPIKED_HELM,
        SPIKED_SHIELD,
        WHITE_PEARL,
        BLACK_PEARL,

        MAGIC_BOOK,

        DUMMY1,
        DUMMY2,
        DUMMY3,
        DUMMY4,

        SPELL_SCROLL,
        ARM_MARTYR,
        BREASTPLATE_ANDURAN,
        BROACH_SHIELDING,
        BATTLE_GARB,
        CRYSTAL_BALL,
        HEART_FIRE,
        HEART_ICE,
        HELMET_ANDURAN,
        HOLY_HAMMER,
        LEGENDARY_SCEPTER,
        MASTHEAD,
        SPHERE_NEGATION,
        STAFF_WIZARDRY,
        SWORD_BREAKER,
        SWORD_ANDURAN,
        SPADE_NECROMANCY,

        UNKNOWN
    };

    Artifact(int = UNKNOWN);

    bool operator==(const Spell&) const;

    bool operator==(const Artifact&) const;

    bool operator!=(const Artifact&) const;

    int operator()() const;

    int GetID() const;

    bool isUltimate() const;

    bool isAlchemistRemove() const;

    bool IsValid() const;

    void Reset();

    uint32_t ExtraValue() const;

    int Level() const;

    int LoyaltyLevel() const;

    int Type() const;

    /* objnarti.icn */
    uint32_t IndexSprite() const;

    /* artfx.icn */
    uint32_t IndexSprite32() const;

    /* artifact.icn */
    uint32_t IndexSprite64() const;

    void SetSpell(int);

    int GetSpell() const;

    std::string GetName() const;

    string GetDescription() const;

    static int Rand(level_t);

    static Artifact FromMP2IndexSprite(uint32_t);

    static std::string GetScenario(const Artifact&);

    static void UpdateStats(const string&);

    void ReadFrom(ByteVectorReader& msg);

private:
    friend ByteVectorWriter& operator<<(ByteVectorWriter&, const Artifact&);

    friend ByteVectorReader& operator>>(ByteVectorReader&, Artifact&);

    int id;
    int ext;
};

ByteVectorWriter& operator<<(ByteVectorWriter&, const Artifact&);

ByteVectorReader& operator>>(ByteVectorReader&, Artifact&);

uint32_t GoldInsteadArtifact(int);

class BagArtifacts
{
public:
    vector<Artifact> _items;
    BagArtifacts();

    bool ContainSpell(const Spell&) const;

    bool isPresentArtifact(const Artifact&) const;

    bool PushArtifact(const Artifact&);

    bool isFull() const;

    bool MakeBattleGarb();

    bool ContainUltimateArtifact() const;

    void RemoveArtifact(const Artifact&);

    void RemoveScroll(const Artifact&);

    uint32_t CountArtifacts() const;

    uint32_t Count(const Artifact&) const;

    string String() const;
};


class ArtifactsBar : public Interface::ItemsActionBar<Artifact>
{
public:
    ArtifactsBar(const Heroes*, bool mini, bool ro, bool change = false);

    void RedrawBackground(const Rect&, Surface&) override;

    void RedrawItem(Artifact&, const Rect&, bool, Surface&) override;

    void ResetSelected() override;

    void Redraw(Surface& dstsf = Display::Get()) override;

    bool ActionBarSingleClick(const Point&, Artifact&, const Rect&) override;

    bool ActionBarSingleClick(const Point&, Artifact&, const Rect&, Artifact&, const Rect&) override;

    bool ActionBarDoubleClick(const Point&, Artifact&, const Rect&) override;

    bool ActionBarPressRight(const Point&, Artifact&, const Rect&) override;

    bool QueueEventProcessing(string* = nullptr);

    bool QueueEventProcessing(ArtifactsBar&, string* = nullptr);

    bool ActionBarCursor(const Point&, Artifact&, const Rect&) override;

    bool ActionBarCursor(const Point&, Artifact&, const Rect&, Artifact&, const Rect&) override;

protected:
    const Heroes* hero;
    Surface backsf;
    SpriteMove spcursor;
    bool use_mini_sprite;
    bool read_only;
    bool can_change;
    string msg;
};
