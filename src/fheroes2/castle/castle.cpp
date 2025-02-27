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

#include <algorithm>
#include <sstream>
#include <iostream>

#include "agg.h"
#include "difficulty.h"
#include "settings.h"
#include "payment.h"
#include "world.h"
#include "luck.h"
#include "race.h"
#include "text.h"
#include "game.h"
#include "dialog.h"
#include "ground.h"
#include "morale.h"
#include "game_static.h"
#include "ai.h"
#include "battle_tower.h"
#include "profit.h"
#include "m82.h"
#include "rand.h"
#include "icn.h"


Castle::Castle() : race(Race::NONE), building(0), captain(*this), army(nullptr)
{
    fill(dwelling, dwelling + CASTLEMAXMONSTER, 0);
    army.SetCommander(&captain);
}

Castle::Castle(s32 cx, s32 cy, int rc) : MapPosition(Point(cx, cy)), race(rc), building(0), captain(*this),
                                         army(nullptr)
{
    fill(dwelling, dwelling + CASTLEMAXMONSTER, 0);
    army.SetCommander(&captain);
}

void Castle::LoadFromMP2(ByteVectorReader& st)
{
    switch (st.get())
    {
    case 0:
        SetColor(Color::BLUE);
        break;
    case 1:
        SetColor(Color::GREEN);
        break;
    case 2:
        SetColor(Color::RED);
        break;
    case 3:
        SetColor(Color::YELLOW);
        break;
    case 4:
        SetColor(Color::ORANGE);
        break;
    case 5:
        SetColor(Color::PURPLE);
        break;
    default:
        SetColor(Color::NONE);
        break;
    }

    // custom building
    if (st.get())
    {
        // building
        int build = st.getLE16();
        if (0x0002 & build) building |= BUILD_THIEVESGUILD;
        if (0x0004 & build) building |= BUILD_TAVERN;
        if (0x0008 & build) building |= BUILD_SHIPYARD;
        if (0x0010 & build) building |= BUILD_WELL;
        if (0x0080 & build) building |= BUILD_STATUE;
        if (0x0100 & build) building |= BUILD_LEFTTURRET;
        if (0x0200 & build) building |= BUILD_RIGHTTURRET;
        if (0x0400 & build) building |= BUILD_MARKETPLACE;
        if (0x1000 & build) building |= BUILD_MOAT;
        if (0x0800 & build) building |= BUILD_WEL2;
        if (0x2000 & build) building |= BUILD_SPEC;

        // dwelling
        int dwell = st.getLE16();
        if (0x0008 & dwell) building |= DWELLING_MONSTER1;
        if (0x0010 & dwell) building |= DWELLING_MONSTER2;
        if (0x0020 & dwell) building |= DWELLING_MONSTER3;
        if (0x0040 & dwell) building |= DWELLING_MONSTER4;
        if (0x0080 & dwell) building |= DWELLING_MONSTER5;
        if (0x0100 & dwell) building |= DWELLING_MONSTER6;
        if (0x0200 & dwell) building |= DWELLING_UPGRADE2 | DWELLING_MONSTER2;
        if (0x0400 & dwell) building |= DWELLING_UPGRADE3 | DWELLING_MONSTER3;
        if (0x0800 & dwell) building |= DWELLING_UPGRADE4 | DWELLING_MONSTER4;
        if (0x1000 & dwell) building |= DWELLING_UPGRADE5 | DWELLING_MONSTER5;
        if (0x2000 & dwell) building |= DWELLING_UPGRADE6 | DWELLING_MONSTER6;

        // magic tower
        int level = st.get();
        if (0 < level) building |= BUILD_MAGEGUILD1;
        if (1 < level) building |= BUILD_MAGEGUILD2;
        if (2 < level) building |= BUILD_MAGEGUILD3;
        if (3 < level) building |= BUILD_MAGEGUILD4;
        if (4 < level) building |= BUILD_MAGEGUILD5;
    }
    else
    {
        st.skip(5);

        // default building
        building |= DWELLING_MONSTER1;
        uint32_t dwelling2 = 0;
        switch ((DifficultyEnum)Settings::Get().GameDifficulty())
        {
        case DifficultyEnum::EASY:
            dwelling2 = 75;
            break;
        case DifficultyEnum::NORMAL:
            dwelling2 = 50;
            break;
        case DifficultyEnum::HARD:
            dwelling2 = 25;
            break;
        case DifficultyEnum::EXPERT:
            dwelling2 = 10;
            break;
        default:
            break;
        }
        if (dwelling2 && dwelling2 >= Rand::Get(1, 100)) building |= DWELLING_MONSTER2;
    }

    // custom troops
    bool custom_troops = st.get();
    if (custom_troops)
    {
        Troop troops[5];

        // set monster id
        for (auto& troop : troops)
            troop.SetMonster(st.get() + 1);

        // set count
        for (auto& troop : troops)
            troop.SetCount(st.getLE16());

        army.m_troops.Assign(troops, ARRAY_COUNT_END(troops));
        SetModes(CUSTOMARMY);
    }
    else
        st.skip(15);

    // captain
    if (st.get()) building |= BUILD_CAPTAIN;

    // custom name
    st.skip(1);
    name = Game::GetEncodeString(st.toString(13));

    // race
    uint32_t kingdom_race = Players::GetPlayerRace(GetColor());
    switch (st.get())
    {
    case 0:
        race = Race::KNGT;
        break;
    case 1:
        race = Race::BARB;
        break;
    case 2:
        race = Race::SORC;
        break;
    case 3:
        race = Race::WRLK;
        break;
    case 4:
        race = Race::WZRD;
        break;
    case 5:
        race = Race::NECR;
        break;
    default:
        race = Color::NONE != GetColor() && Race::ALL & kingdom_race ? kingdom_race : Race::Rand();
        break;
    }

    // castle
    building |= st.get() ? BUILD_CASTLE : BUILD_TENT;

    // allow upgrade to castle (0 - true, 1 - false)
    if (st.get())
        ResetModes(ALLOWCASTLE);
    else
        SetModes(ALLOWCASTLE);

    // unknown 29 byte
    //

    PostLoad();
}

void Castle::PostLoad()
{
    // dwelling pack
    if (building & DWELLING_MONSTER1) dwelling[0] = Monster(race, DWELLING_MONSTER1).GetGrown();
    if (building & DWELLING_MONSTER2) dwelling[1] = Monster(race, DWELLING_MONSTER2).GetGrown();
    if (building & DWELLING_UPGRADE2) dwelling[1] = Monster(race, DWELLING_UPGRADE2).GetGrown();
    if (building & DWELLING_MONSTER3) dwelling[2] = Monster(race, DWELLING_MONSTER3).GetGrown();
    if (building & DWELLING_UPGRADE3) dwelling[2] = Monster(race, DWELLING_UPGRADE3).GetGrown();
    if (building & DWELLING_MONSTER4) dwelling[3] = Monster(race, DWELLING_MONSTER4).GetGrown();
    if (building & DWELLING_UPGRADE4) dwelling[3] = Monster(race, DWELLING_UPGRADE4).GetGrown();
    if (building & DWELLING_MONSTER5) dwelling[4] = Monster(race, DWELLING_MONSTER5).GetGrown();
    if (building & DWELLING_UPGRADE5) dwelling[4] = Monster(race, DWELLING_UPGRADE5).GetGrown();
    if (building & DWELLING_MONSTER6) dwelling[5] = Monster(race, DWELLING_MONSTER6).GetGrown();
    if (building & DWELLING_UPGRADE6) dwelling[5] = Monster(race, DWELLING_UPGRADE6).GetGrown();
    if (building & DWELLING_UPGRADE7) dwelling[5] = Monster(race, DWELLING_UPGRADE7).GetGrown();

    // fix upgrade dwelling dependend from race
    switch (race)
    {
    case Race::BARB:
        building &= ~(DWELLING_UPGRADE3 | DWELLING_UPGRADE6);
        break;
    case Race::SORC:
        building &= ~(DWELLING_UPGRADE5 | DWELLING_UPGRADE6);
        break;
    case Race::WRLK:
        building &= ~(DWELLING_UPGRADE2 | DWELLING_UPGRADE3 | DWELLING_UPGRADE5);
        break;
    case Race::WZRD:
        building &= ~(DWELLING_UPGRADE2 | DWELLING_UPGRADE4);
        break;
    case Race::NECR:
        building &= ~DWELLING_UPGRADE6;
        break;
    default:
        break;
    }

    army.SetColor(GetColor());

    // fix captain
    if (building & BUILD_CAPTAIN)
        captain.LoadDefaults(HeroBase::CAPTAIN, race);

    // MageGuild
    mageguild.Builds(race, HaveLibraryCapability());
    // educate heroes and captain
    EducateHeroes();

    // AI troops auto pack for gray towns
    if (Color::NONE == GetColor() &&
        !Modes(CUSTOMARMY))
        JoinRNDArmy();

    // fix shipyard
    if (!HaveNearlySea()) building &= ~BUILD_SHIPYARD;

    // remove tavern from necromancer castle
    if (Race::NECR == race && building & BUILD_TAVERN)
    {
        building &= ~BUILD_TAVERN;
        if (Settings::Get().PriceLoyaltyVersion())
            building |= BUILD_SHRINE;
    }

    SetModes(ALLOWBUILD);

    // end
}

Captain& Castle::GetCaptain()
{
    return captain;
}

const Captain& Castle::GetCaptain() const
{
    return captain;
}

bool Castle::isCastle() const
{
    return building & BUILD_CASTLE;
}

bool Castle::isCapital() const
{
    return Modes(CAPITAL);
}

uint32_t Castle::CountBuildings() const
{
    const uint32_t tavern = race == Race::NECR
                                ? (Settings::Get().PriceLoyaltyVersion() ? BUILD_SHRINE : 0)
                                : BUILD_TAVERN;

    return CountBits(building & (BUILD_THIEVESGUILD | tavern | BUILD_SHIPYARD | BUILD_WELL |
        BUILD_STATUE | BUILD_LEFTTURRET | BUILD_RIGHTTURRET |
        BUILD_MARKETPLACE | BUILD_WEL2 | BUILD_MOAT | BUILD_SPEC |
        BUILD_CAPTAIN | BUILD_CASTLE | BUILD_MAGEGUILD1 | DWELLING_MONSTER1 |
        DWELLING_MONSTER2 | DWELLING_MONSTER3 | DWELLING_MONSTER4 |
        DWELLING_MONSTER5 | DWELLING_MONSTER6));
}

bool Castle::isPosition(const Point& pt) const
{
    const Point& mp = GetCenter();

    /*
              -
             ---
            -+++-
            ++X++
    */

    return (pt.x >= mp.x - 1 && pt.x <= mp.x + 1 && (pt.y == mp.y - 1 || pt.y == mp.y)) ||
        ((pt.x == mp.x - 2 || pt.x == mp.x + 2) && pt.y == mp.y);
}

void Castle::EducateHeroes()
{
    // for learns new spells need 1 day
    if (!GetLevelMageGuild())
        return;
    CastleHeroes heroes = world.GetHeroes(*this);

    if (heroes.FullHouse())
    {
        MageGuildEducateHero(*heroes.Guest());
        MageGuildEducateHero(*heroes.Guard());
    }
    else if (heroes.IsValid())
        MageGuildEducateHero(*heroes.GuestFirst());

    // captain
    if (captain.isValid()) MageGuildEducateHero(captain);
}

void Castle::ActionNewDay()
{
    EducateHeroes();

    SetModes(ALLOWBUILD);
}

uint32_t* Castle::GetDwelling(uint32_t dw)
{
    if (!isBuild(dw))
        return nullptr;
    switch (dw)
    {
    case DWELLING_MONSTER1:
        return &dwelling[0];
    case DWELLING_MONSTER2:
    case DWELLING_UPGRADE2:
        return &dwelling[1];
    case DWELLING_MONSTER3:
    case DWELLING_UPGRADE3:
        return &dwelling[2];
    case DWELLING_MONSTER4:
    case DWELLING_UPGRADE4:
        return &dwelling[3];
    case DWELLING_MONSTER5:
    case DWELLING_UPGRADE5:
        return &dwelling[4];
    case DWELLING_MONSTER6:
    case DWELLING_UPGRADE6:
    case DWELLING_UPGRADE7:
        return &dwelling[5];
    default:
        break;
    }
    return nullptr;
}

void Castle::ActionNewWeek()
{
    ResetModes(DISABLEHIRES);

    // increase population
    if (world.GetWeekType().GetType() != Week::PLAGUE)
    {
        const uint32_t dwellings1[] = {
            DWELLING_MONSTER1, DWELLING_MONSTER2, DWELLING_MONSTER3, DWELLING_MONSTER4,
            DWELLING_MONSTER5, DWELLING_MONSTER6, 0
        };
        uint32_t* dw = nullptr;

        // simple growth
        for (uint32_t ii = 0; dwellings1[ii]; ++ii)
            if (nullptr != (dw = GetDwelling(dwellings1[ii])))
            {
                uint32_t growth = Monster(race, GetActualDwelling(dwellings1[ii])).GetGrown();

                // well build
                if (building & BUILD_WELL) growth += GetGrownWell();

                // wel2 extras
                if (dwellings1[ii] == DWELLING_MONSTER1 && building & BUILD_WEL2) growth += GetGrownWel2();

                // neutral town: half population (normal for begin month)
                if (GetColor() == Color::NONE && !world.BeginMonth())
                    growth /= 2;

                *dw += growth;
            }

        // Week Of
        if (world.GetWeekType().GetType() == Week::MONSTERS && !world.BeginMonth())
        {
            const uint32_t dwellings2[] = {
                DWELLING_MONSTER1, DWELLING_UPGRADE2, DWELLING_UPGRADE3, DWELLING_UPGRADE4,
                DWELLING_UPGRADE5, DWELLING_MONSTER2, DWELLING_MONSTER3, DWELLING_MONSTER4,
                DWELLING_MONSTER5, 0
            };

            for (uint32_t ii = 0; dwellings2[ii]; ++ii)
                if (nullptr != (dw = GetDwelling(dwellings2[ii])))
                {
                    const Monster mons(race, dwellings2[ii]);
                    if (mons.IsValid() && mons() == world.GetWeekType().GetMonster())
                    {
                        *dw += GetGrownWeekOf(mons);
                        break;
                    }
                }
        }

        // neutral town: small increase garrisons (random)
        if (Color::NONE == GetColor() &&
            !Modes(CUSTOMARMY))
            JoinRNDArmy();
    }
}

void Castle::ActionNewMonth()
{
    // population halved
    if (world.GetWeekType().GetType() == Week::PLAGUE)
    {
        for (unsigned int& ii : dwelling)
            if (ii) ii /= 2;
    }
    else
        // Month Of
        if (world.GetWeekType().GetType() == Week::MONSTERS)
        {
            const uint32_t dwellings[] = {
                DWELLING_MONSTER1, DWELLING_UPGRADE2, DWELLING_UPGRADE3, DWELLING_UPGRADE4,
                DWELLING_UPGRADE5, DWELLING_MONSTER2, DWELLING_MONSTER3, DWELLING_MONSTER4,
                DWELLING_MONSTER5, 0
            };
            uint32_t* dw = nullptr;

            for (uint32_t ii = 0; dwellings[ii]; ++ii)
                if (nullptr != (dw = GetDwelling(dwellings[ii])))
                {
                    const Monster mons(race, dwellings[ii]);
                    if (mons.IsValid() && mons() == world.GetWeekType().GetMonster())
                    {
                        *dw += *dw * GetGrownMonthOf() / 100;
                        break;
                    }
                }
        }
}

// change castle color
void Castle::ChangeColor(int cl)
{
    SetColor(cl);
    army.SetColor(cl);
}

// return mage guild level
int Castle::GetLevelMageGuild() const
{
    if (building & BUILD_MAGEGUILD5) return 5;
    if (building & BUILD_MAGEGUILD4) return 4;
    if (building & BUILD_MAGEGUILD3) return 3;
    if (building & BUILD_MAGEGUILD2) return 2;
    if (building & BUILD_MAGEGUILD1) return 1;

    return 0;
}

const MageGuild& Castle::GetMageGuild() const
{
    return mageguild;
}

bool Castle::HaveLibraryCapability() const
{
    return race == Race::WZRD;
}

bool Castle::isLibraryBuild() const
{
    return race == Race::WZRD && isBuild(BUILD_SPEC);
}

void Castle::MageGuildEducateHero(HeroBase& hero) const
{
    mageguild.EducateHero(hero, GetLevelMageGuild(), isLibraryBuild());
}

string Castle::GetStringBuilding(uint32_t build, int race)
{
    string str_build[] = {
        _("Thieves' Guild"), _("Tavern"), _("Shipyard"), _("Well"), _("Statue"),
        _("Left Turret"),
        _("Right Turret"), _("Marketplace"), _("Moat"), _("Castle"), _("Tent"),
        _("Captain's Quarters"), _("Mage Guild, Level 1"),
        _("Mage Guild, Level 2"), _("Mage Guild, Level 3"), _("Mage Guild, Level 4"),
        _("Mage Guild, Level 5"), "Unknown"
    };

    string str_wel2[] = {
        _("Farm"), _("Garbage Heap"), _("Crystal Garden"), _("Waterfall"), _("Orchard"),
        _("Skull Pile")
    };

    string str_spec[] = {_("Fortifications"), _("Coliseum"), _("Rainbow"), _("Dungeon"), _("Library"), _("Storm")};

    string str_dwelling[] = {
        _("Thatched Hut"), _("Hut"), _("Treehouse"), _("Cave"), _("Habitat"), _("Excavation"),
        _("Archery Range"), _("Stick Hut"), _("Cottage"), _("Crypt"), _("Pen"), _("Graveyard"),
        _("Blacksmith"), _("Den"), _("Archery Range"), _("Nest"), _("Foundry"), _("Pyramid"),
        _("Armory"), _("Adobe"), _("Stonehenge"), _("Maze"), _("Cliff Nest"), _("Mansion"),
        _("Jousting Arena"), _("Bridge"), _("Fenced Meadow"), _("Swamp"), _("Ivory Tower"), _("Mausoleum"),
        _("Cathedral"), _("Pyramid"), _("Red Tower"), _("Green Tower"), _("Cloud Castle"), _("Laboratory")
    };

    string str_upgrade[] = {
        _("Upg. Archery Range"), _("Upg. Stick Hut"), _("Upg. Cottage"), _("Crypt"), _("Pen"), _("Upg. Graveyard"),
        _("Upg. Blacksmith"), _("Den"), _("Upg. Archery Range"), _("Nest"), _("Upg. Foundry"), _("Upg. Pyramid"),
        _("Upg. Armory"), _("Upg. Adobe"), _("Upg. Stonehenge"), _("Upg. Maze"), _("Cliff Nest"), _("Upg. Mansion"),
        _("Upg. Jousting Arena"), _("Upg. Bridge"), _("Fenced Meadow"), _("Swamp"), _("Upg. Ivory Tower"),
        _("Upg. Mausoleum"),
        _("Upg. Cathedral"), _("Pyramid"), _("Red Tower"), _("Red Tower"), _("Upg. Cloud Castle"), _("Laboratory"),
        "", "", "", _("Black Tower"), "", ""
    };

    string shrine = _("Shrine");

    uint32_t offset = 0;

    switch (race)
    {
    case Race::KNGT:
        offset = 0;
        break;
    case Race::BARB:
        offset = 1;
        break;
    case Race::SORC:
        offset = 2;
        break;
    case Race::WRLK:
        offset = 3;
        break;
    case Race::WZRD:
        offset = 4;
        break;
    case Race::NECR:
        offset = 5;
        break;
    default:
        break;
    }

    switch (build)
    {
    case BUILD_SHRINE:
        return shrine;
    case BUILD_THIEVESGUILD:
        return str_build[0];
    case BUILD_TAVERN:
        return str_build[1];
    case BUILD_SHIPYARD:
        return str_build[2];
    case BUILD_WELL:
        return str_build[3];
    case BUILD_STATUE:
        return str_build[4];
    case BUILD_LEFTTURRET:
        return str_build[5];
    case BUILD_RIGHTTURRET:
        return str_build[6];
    case BUILD_MARKETPLACE:
        return str_build[7];
    case BUILD_MOAT:
        return str_build[8];
    case BUILD_CASTLE:
        return str_build[9];
    case BUILD_TENT:
        return str_build[10];
    case BUILD_CAPTAIN:
        return str_build[11];
    case BUILD_MAGEGUILD1:
        return str_build[12];
    case BUILD_MAGEGUILD2:
        return str_build[13];
    case BUILD_MAGEGUILD3:
        return str_build[14];
    case BUILD_MAGEGUILD4:
        return str_build[15];
    case BUILD_MAGEGUILD5:
        return str_build[16];

    case BUILD_SPEC:
        return str_spec[offset];
    case BUILD_WEL2:
        return str_wel2[offset];

    case DWELLING_MONSTER1:
        return str_dwelling[offset];
    case DWELLING_MONSTER2:
        return str_dwelling[6 + offset];
    case DWELLING_MONSTER3:
        return str_dwelling[12 + offset];
    case DWELLING_MONSTER4:
        return str_dwelling[18 + offset];
    case DWELLING_MONSTER5:
        return str_dwelling[24 + offset];
    case DWELLING_MONSTER6:
        return str_dwelling[30 + offset];

    case DWELLING_UPGRADE2:
        return str_upgrade[offset];
    case DWELLING_UPGRADE3:
        return str_upgrade[6 + offset];
    case DWELLING_UPGRADE4:
        return str_upgrade[12 + offset];
    case DWELLING_UPGRADE5:
        return str_upgrade[18 + offset];
    case DWELLING_UPGRADE6:
        return str_upgrade[24 + offset];
    case DWELLING_UPGRADE7:
        return str_upgrade[30 + offset];

    default:
        break;
    }

    return str_build[17];
}

string Castle::GetDescriptionBuilding(uint32_t build, int race)
{
    string desc_build[] = {
        _(
            "The Thieves' Guild provides information on enemy players. Thieves' Guilds can also provide scouting information on enemy towns."
        ),
        _("The Tavern increases morale for troops defending the castle."),
        _("The Shipyard allows ships to be built."),
        _("The Well increases the growth rate of all dwellings by %{count} creatures per week."),
        _("The Statue increases your town's income by %{count} per day."),
        _("The Left Turret provides extra firepower during castle combat."),
        _("The Right Turret provides extra firepower during castle combat."),
        _(
            "The Marketplace can be used to convert one type of resource into another. The more marketplaces you control, the better the exchange rate."
        ),
        _(
            "The Moat slows attacking units. Any unit entering the moat must end its turn there and becomes more vulnerable to attack."
        ),
        _("The Castle improves town defense and increases income to %{count} gold per day."),
        _("The Tent provides workers to build a castle, provided the materials and the gold are available."),
        _("The Captain's Quarters provides a captain to assist in the castle's defense when no hero is present."),
        _("The Mage Guild allows heroes to learn spells and replenish their spell points."), "Unknown"
    };

    string desc_wel2[] = {
        _("The Farm increases production of Peasants by %{count} per week."),
        _("The Garbage Heap increases production of Goblins by %{count} per week."),
        _("The Crystal Garden increases production of Sprites by %{count} per week."),
        _("The Waterfall increases production of Centaurs by %{count} per week."),
        _("The Orchard increases production of Halflings by %{count} per week."),
        _("The Skull Pile increases production of Skeletons by %{count} per week.")
    };

    string desc_spec[] = {
        _(
            "The Fortifications increase the toughness of the walls, increasing the number of turns it takes to knock them down."
        ),
        _("The Coliseum provides inspiring spectacles to defending troops, raising their morale by two during combat."),
        _("The Rainbow increases the luck of the defending units by two."),
        _("The Dungeon increases the income of the town by %{count} / day."),
        _("The Library increases the number of spells in the Guild by one for each level of the guild."),
        _("The Storm adds +2 to the power of spells of a defending spell caster.")
    };

    string shrine_descr = _("The Shrine increases the necromancy skill of all your necromancers by 10 percent.");

    uint32_t offset = 0;

    switch (race)
    {
    case Race::KNGT:
        offset = 0;
        break;
    case Race::BARB:
        offset = 1;
        break;
    case Race::SORC:
        offset = 2;
        break;
    case Race::WRLK:
        offset = 3;
        break;
    case Race::WZRD:
        offset = 4;
        break;
    case Race::NECR:
        offset = 5;
        break;
    default:
        break;
    }

    switch (build)
    {
    case BUILD_SHRINE:
        return shrine_descr;
    case BUILD_THIEVESGUILD:
        return desc_build[0];
    case BUILD_TAVERN:
        return desc_build[1];
    case BUILD_SHIPYARD:
        return desc_build[2];
    case BUILD_WELL:
        return desc_build[3];
    case BUILD_STATUE:
        return desc_build[4];
    case BUILD_LEFTTURRET:
        return desc_build[5];
    case BUILD_RIGHTTURRET:
        return desc_build[6];
    case BUILD_MARKETPLACE:
        return desc_build[7];
    case BUILD_MOAT:
        return desc_build[8];
    case BUILD_CASTLE:
        return desc_build[9];
    case BUILD_TENT:
        return desc_build[10];
    case BUILD_CAPTAIN:
        return desc_build[11];
    case BUILD_MAGEGUILD1:
    case BUILD_MAGEGUILD2:
    case BUILD_MAGEGUILD3:
    case BUILD_MAGEGUILD4:
    case BUILD_MAGEGUILD5:
        return desc_build[12];

    case BUILD_SPEC:
        return desc_spec[offset];
    case BUILD_WEL2:
        return desc_wel2[offset];

    default:
        break;
    }

    return desc_build[13];
}

bool Castle::AllowBuyHero(const Heroes& hero, string* msg) const
{
    const Kingdom& myKingdom = GetKingdom();
    if (Modes(DISABLEHIRES) || myKingdom.Modes(Kingdom::DISABLEHIRES))
    {
        if (msg) *msg = _("Cannot recruit - you already recruit hero in current week.");
        return false;
    }

    CastleHeroes heroes = world.GetHeroes(*this);

    if (heroes.Guest())
    {
        // allow recruit with auto move guest to guard
        if (Settings::Get().ExtCastleAllowGuardians() && !heroes.Guard())
        {
            if (!heroes.Guest()->GetArmy().m_troops.CanJoinTroops(army.m_troops))
            {
                if (msg) *msg = _("Cannot recruit - guest to guard automove error.");
                return false;
            }
        }
        else
        {
            if (msg) *msg = _("Cannot recruit - you already have a Hero in this town.");
            return false;
        }
    }


    if (!myKingdom.AllowRecruitHero(false, hero.GetLevel()))
    {
        if (msg) *msg = _("Cannot recruit - you have too many Heroes.");
        return false;
    }

    if (!myKingdom.AllowRecruitHero(true, hero.GetLevel()))
    {
        if (msg) *msg = _("Cannot afford a Hero");
        return false;
    }

    return true;
}

Heroes* Castle::RecruitHero(Heroes* hero)
{
    if (!hero || !AllowBuyHero(*hero)) return nullptr;

    CastleHeroes heroes = world.GetHeroes(*this);
    if (heroes.Guest())
    {
        if (Settings::Get().ExtCastleAllowGuardians() && !heroes.Guard())
        {
            // move guest to guard
            SwapCastleHeroes(heroes);
        }
        else
            return nullptr;
    }

    // recruit
    if (!hero->Recruit(*this)) return nullptr;

    Kingdom& kingdom = GetKingdom();

    if (kingdom.GetLastLostHero() == hero)
        kingdom.ResetLastLostHero();

    kingdom.OddFundsResource(PaymentConditions::RecruitHero(hero->GetLevel()));

    // update spell book
    if (GetLevelMageGuild()) MageGuildEducateHero(*hero);

    if (Settings::Get().ExtWorldOneHeroHiredEveryWeek())
        kingdom.SetModes(Kingdom::DISABLEHIRES);

    if (Settings::Get().ExtCastleOneHeroHiredEveryWeek())
        SetModes(DISABLEHIRES);

    return hero;
}

/* recruit monster from building to castle army */
bool Castle::RecruitMonster(const Troop& troop)
{
    if (!troop.IsValid())
        return false;

    int dw_index = 0;

    switch (troop._monster.GetDwelling())
    {
    case DWELLING_MONSTER1:
        dw_index = 0;
        break;
    case DWELLING_UPGRADE2:
    case DWELLING_MONSTER2:
        dw_index = 1;
        break;
    case DWELLING_UPGRADE3:
    case DWELLING_MONSTER3:
        dw_index = 2;
        break;
    case DWELLING_UPGRADE4:
    case DWELLING_MONSTER4:
        dw_index = 3;
        break;
    case DWELLING_UPGRADE5:
    case DWELLING_MONSTER5:
        dw_index = 4;
        break;
    case DWELLING_UPGRADE7:
    case DWELLING_UPGRADE6:
    case DWELLING_MONSTER6:
        dw_index = 5;
        break;
    default:
        return false;
    }

    const Monster& ms = troop._monster;
    uint32_t count = troop.GetCount();

    // fix count
    if (dwelling[dw_index] < count) count = dwelling[dw_index];

    // buy
    const payment_t paymentCosts = troop.GetCost();
    Kingdom& kingdom = GetKingdom();

    if (!kingdom.AllowPayment(paymentCosts)) return false;

    // first: guard army join
    if (!GetArmy().m_troops.JoinTroop(ms, count))
    {
        CastleHeroes heroes = world.GetHeroes(*this);

        if (!heroes.Guest() || !heroes.Guest()->GetArmy().m_troops.JoinTroop(ms, count))
        {
            Message("", _("There is no room in the garrison for this army."), Font::BIG, Dialog::OK);
            return false;
        }
    }

    kingdom.OddFundsResource(paymentCosts);
    dwelling[dw_index] -= count;

    return true;
}

bool Castle::RecruitMonsterFromDwelling(uint32_t dw, uint32_t count)
{
    int dw_index = 0;

    switch (dw)
    {
    case DWELLING_MONSTER1:
        dw_index = 0;
        break;
    case DWELLING_UPGRADE2:
    case DWELLING_MONSTER2:
        dw_index = 1;
        break;
    case DWELLING_UPGRADE3:
    case DWELLING_MONSTER3:
        dw_index = 2;
        break;
    case DWELLING_UPGRADE4:
    case DWELLING_MONSTER4:
        dw_index = 3;
        break;
    case DWELLING_UPGRADE5:
    case DWELLING_MONSTER5:
        dw_index = 4;
        break;
    case DWELLING_UPGRADE7:
    case DWELLING_UPGRADE6:
    case DWELLING_MONSTER6:
        dw_index = 5;
        break;
    default:
        return false;
    }

    const Monster ms(race, GetActualDwelling(dw));

    // fix count
    if (dwelling[dw_index] < count) count = dwelling[dw_index];

    // buy
    const payment_t paymentCosts = ms.GetCost() * count;
    Kingdom& kingdom = GetKingdom();

    // may be guardian present
    Army& army2 = GetArmy();

    if (!kingdom.AllowPayment(paymentCosts) || !army2.m_troops.JoinTroop(ms, count)) return false;

    kingdom.OddFundsResource(paymentCosts);
    dwelling[dw_index] -= count;

    return true;
}

/* return current count monster in dwelling */
uint32_t Castle::GetDwellingLivedCount(uint32_t dw) const
{
    switch (dw)
    {
    case DWELLING_MONSTER1:
        return dwelling[0];
    case DWELLING_MONSTER2:
    case DWELLING_UPGRADE2:
        return dwelling[1];
    case DWELLING_MONSTER3:
    case DWELLING_UPGRADE3:
        return dwelling[2];
    case DWELLING_MONSTER4:
    case DWELLING_UPGRADE4:
        return dwelling[3];
    case DWELLING_MONSTER5:
    case DWELLING_UPGRADE5:
        return dwelling[4];
    case DWELLING_MONSTER6:
    case DWELLING_UPGRADE6:
    case DWELLING_UPGRADE7:
        return dwelling[5];

    default:
        break;
    }

    return 0;
}

/* return requires for building */
uint32_t Castle::GetBuildingRequires(uint32_t build) const
{
    uint32_t requires = 0;

    switch (build)
    {
    case BUILD_SPEC:
        switch (race)
        {
        case Race::WZRD:
            requires |= BUILD_MAGEGUILD1;
            break;

        default:
            break;
        }
        break;

    case DWELLING_MONSTER2:
        switch (race)
        {
        case Race::KNGT:
        case Race::BARB:
        case Race::WZRD:
        case Race::WRLK:
        case Race::NECR:
            requires |= DWELLING_MONSTER1;
            break;

        case Race::SORC:
            requires |= DWELLING_MONSTER1;
            requires |= BUILD_TAVERN;
            break;

        default:
            break;
        }
        break;

    case DWELLING_MONSTER3:
        switch (race)
        {
        case Race::KNGT:
            requires |= DWELLING_MONSTER1;
            requires |= BUILD_WELL;
            break;

        case Race::BARB:
        case Race::SORC:
        case Race::WZRD:
        case Race::WRLK:
        case Race::NECR:
            requires |= DWELLING_MONSTER1;
            break;

        default:
            break;
        }
        break;

    case DWELLING_MONSTER4:
        switch (race)
        {
        case Race::KNGT:
            requires |= DWELLING_MONSTER1;
            requires |= BUILD_TAVERN;
            break;

        case Race::BARB:
            requires |= DWELLING_MONSTER1;
            break;

        case Race::SORC:
            requires |= DWELLING_MONSTER2;
            requires |= BUILD_MAGEGUILD1;
            break;

        case Race::WZRD:
        case Race::WRLK:
            requires |= DWELLING_MONSTER2;
            break;

        case Race::NECR:
            requires |= DWELLING_MONSTER3;
            requires |= BUILD_THIEVESGUILD;
            break;

        default:
            break;
        }
        break;

    case DWELLING_MONSTER5:
        switch (race)
        {
        case Race::KNGT:
        case Race::BARB:
            requires |= DWELLING_MONSTER2;
            requires |= DWELLING_MONSTER3;
            requires |= DWELLING_MONSTER4;
            break;

        case Race::SORC:
            requires |= DWELLING_MONSTER4;
            break;

        case Race::WRLK:
            requires |= DWELLING_MONSTER3;
            break;

        case Race::WZRD:
            requires |= DWELLING_MONSTER3;
            requires |= BUILD_MAGEGUILD1;
            break;

        case Race::NECR:
            requires |= DWELLING_MONSTER2;
            requires |= BUILD_MAGEGUILD1;
            break;

        default:
            break;
        }
        break;

    case DWELLING_MONSTER6:
        switch (race)
        {
        case Race::KNGT:
            requires |= DWELLING_MONSTER2;
            requires |= DWELLING_MONSTER3;
            requires |= DWELLING_MONSTER4;
            break;

        case Race::BARB:
        case Race::SORC:
        case Race::NECR:
            requires |= DWELLING_MONSTER5;
            break;

        case Race::WRLK:
        case Race::WZRD:
            requires |= DWELLING_MONSTER4;
            requires |= DWELLING_MONSTER5;
            break;

        default:
            break;
        }
        break;

    case DWELLING_UPGRADE2:
        switch (race)
        {
        case Race::KNGT:
        case Race::BARB:
            requires |= DWELLING_MONSTER2;
            requires |= DWELLING_MONSTER3;
            requires |= DWELLING_MONSTER4;
            break;

        case Race::SORC:
            requires |= DWELLING_MONSTER2;
            requires |= BUILD_WELL;
            break;

        case Race::NECR:
            requires |= DWELLING_MONSTER2;
            break;

        default:
            break;
        }
        break;

    case DWELLING_UPGRADE3:
        switch (race)
        {
        case Race::KNGT:
            requires |= DWELLING_MONSTER2;
            requires |= DWELLING_MONSTER3;
            requires |= DWELLING_MONSTER4;
            break;

        case Race::SORC:
            requires |= DWELLING_MONSTER3;
            requires |= DWELLING_MONSTER4;
            break;

        case Race::WZRD:
            requires |= DWELLING_MONSTER3;
            requires |= BUILD_WELL;
            break;

        case Race::NECR:
            requires |= DWELLING_MONSTER3;
            break;

        default:
            break;
        }
        break;

    case DWELLING_UPGRADE4:
        switch (race)
        {
        case Race::KNGT:
        case Race::BARB:
            requires |= DWELLING_MONSTER2;
            requires |= DWELLING_MONSTER3;
            requires |= DWELLING_MONSTER4;
            break;

        case Race::SORC:
        case Race::WRLK:
        case Race::NECR:
            requires |= DWELLING_MONSTER4;
            break;

        default:
            break;
        }
        break;

    case DWELLING_UPGRADE5:
        switch (race)
        {
        case Race::KNGT:
            requires |= DWELLING_MONSTER2;
            requires |= DWELLING_MONSTER3;
            requires |= DWELLING_MONSTER4;
            requires |= DWELLING_MONSTER5;
            break;

        case Race::BARB:
            requires |= DWELLING_MONSTER5;
            break;

        case Race::WZRD:
            requires |= BUILD_SPEC;
            requires |= DWELLING_MONSTER5;
            break;

        case Race::NECR:
            requires |= BUILD_MAGEGUILD2;
            requires |= DWELLING_MONSTER5;
            break;

        default:
            break;
        }
        break;

    case DWELLING_UPGRADE6:
        switch (race)
        {
        case Race::KNGT:
            requires |= DWELLING_MONSTER2;
            requires |= DWELLING_MONSTER3;
            requires |= DWELLING_MONSTER4;
            requires |= DWELLING_MONSTER6;
            break;

        case Race::WRLK:
        case Race::WZRD:
            requires |= DWELLING_MONSTER6;
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }

    return requires;
}

/* check allow buy building */
int Castle::CheckBuyBuilding(uint32_t build) const
{
    if (build & building) return ALREADY_BUILT;

    switch (build)
    {
        // allow build castle
    case BUILD_CASTLE:
        if (!Modes(ALLOWCASTLE)) return BUILD_DISABLE;
        break;
        // buid shipyard only nearly sea
    case BUILD_SHIPYARD:
        if (!HaveNearlySea()) return BUILD_DISABLE;
        break;
    case BUILD_SHRINE:
        if (Race::NECR != GetRace() || !Settings::Get().PriceLoyaltyVersion()) return BUILD_DISABLE;
        break;
    case BUILD_TAVERN:
        if (Race::NECR == GetRace()) return BUILD_DISABLE;
        break;

    default:
        break;
    }

    if (!Modes(ALLOWBUILD)) return NOT_TODAY;

    if (isCastle())
    {
        if (build == BUILD_TENT) return BUILD_DISABLE;
    }
    else
    {
        if (build != BUILD_CASTLE) return NEED_CASTLE;
    }

    switch (build)
    {
        // check upgrade dwelling
    case DWELLING_UPGRADE2:
        if ((Race::WRLK | Race::WZRD) & race) return UNKNOWN_UPGRADE;
        break;
    case DWELLING_UPGRADE3:
        if ((Race::BARB | Race::WRLK) & race) return UNKNOWN_UPGRADE;
        break;
    case DWELLING_UPGRADE4:
        if (Race::WZRD & race) return UNKNOWN_UPGRADE;
        break;
    case DWELLING_UPGRADE5:
        if ((Race::SORC | Race::WRLK) & race) return UNKNOWN_UPGRADE;
        break;
    case DWELLING_UPGRADE6:
        if ((Race::BARB | Race::SORC | Race::NECR) & race) return UNKNOWN_UPGRADE;
        break;
    case DWELLING_UPGRADE7:
        if (Race::WRLK != race) return UNKNOWN_UPGRADE;
        break;

    default:
        break;
    }

    // check build requirements
    const uint32_t requires(GetBuildingRequires(build));

    for (uint32_t itr = 0x00000001; itr; itr <<= 1)
        if (requires & itr && !(building & itr)) return REQUIRES_BUILD;

    // check valid payment
    if (!GetKingdom().AllowPayment(PaymentConditions::BuyBuilding(race, build))) return LACK_RESOURCES;

    return ALLOW_BUILD;
}

int Castle::GetAllBuildingStatus(const Castle& castle)
{
    if (!castle.Modes(ALLOWBUILD)) return NOT_TODAY;
    if (!castle.isCastle()) return NEED_CASTLE;

    const uint32_t rest = ~castle.building;

    for (uint32_t itr = 0x00000001; itr; itr <<= 1)
        if (rest & itr && ALLOW_BUILD == castle.CheckBuyBuilding(itr)) return ALLOW_BUILD;

    for (uint32_t itr = 0x00000001; itr; itr <<= 1)
        if (rest & itr && LACK_RESOURCES == castle.CheckBuyBuilding(itr)) return LACK_RESOURCES;

    for (uint32_t itr = 0x00000001; itr; itr <<= 1)
        if (rest & itr && REQUIRES_BUILD == castle.CheckBuyBuilding(itr)) return REQUIRES_BUILD;

    return UNKNOWN_COND;
}

bool Castle::AllowBuyBuilding(uint32_t build) const
{
    return ALLOW_BUILD == CheckBuyBuilding(build);
}

/* buy building */
bool Castle::BuyBuilding(uint32_t build)
{
    if (!AllowBuyBuilding(build)) return false;

    GetKingdom().OddFundsResource(PaymentConditions::BuyBuilding(race, build));

    // add build
    building |= build;

    switch (build)
    {
    case BUILD_CASTLE:
        building &= ~BUILD_TENT;
        Maps::UpdateSpritesFromTownToCastle(GetCenter());
        Maps::ClearFog(GetIndex(), GetViewDistance(Game::VIEW_CASTLE), GetColor());
        break;

    case BUILD_MAGEGUILD1:
    case BUILD_MAGEGUILD2:
    case BUILD_MAGEGUILD3:
    case BUILD_MAGEGUILD4:
    case BUILD_MAGEGUILD5:
        EducateHeroes();
        break;

    case BUILD_CAPTAIN:
        captain.LoadDefaults(HeroBase::CAPTAIN, race);
        if (GetLevelMageGuild()) MageGuildEducateHero(captain);
        break;

    case BUILD_SPEC:
        // build library
        if (HaveLibraryCapability()) EducateHeroes();
        break;

    case DWELLING_MONSTER1:
        dwelling[0] = Monster(race, DWELLING_MONSTER1).GetGrown();
        break;
    case DWELLING_MONSTER2:
        dwelling[1] = Monster(race, DWELLING_MONSTER2).GetGrown();
        break;
    case DWELLING_MONSTER3:
        dwelling[2] = Monster(race, DWELLING_MONSTER3).GetGrown();
        break;
    case DWELLING_MONSTER4:
        dwelling[3] = Monster(race, DWELLING_MONSTER4).GetGrown();
        break;
    case DWELLING_MONSTER5:
        dwelling[4] = Monster(race, DWELLING_MONSTER5).GetGrown();
        break;
    case DWELLING_MONSTER6:
        dwelling[5] = Monster(race, DWELLING_MONSTER6).GetGrown();
        break;
    default:
        break;
    }

    // disable day build
    ResetModes(ALLOWBUILD);

    return true;
}

/* draw image castle to position */
void Castle::DrawImageCastle(const Point& pt, Surface& destSurface) const
{
    const Maps::Tiles& tile = world.GetTiles(GetIndex());
    uint32_t index;
    Point dst_pt;

    // draw ground
    switch (tile.GetGround())
    {
    case Maps::Ground::GRASS:
        index = 0;
        break;
    case Maps::Ground::SNOW:
        index = 10;
        break;
    case Maps::Ground::SWAMP:
        index = 20;
        break;
    case Maps::Ground::LAVA:
        index = 30;
        break;
    case Maps::Ground::DESERT:
        index = 40;
        break;
    case Maps::Ground::DIRT:
        index = 50;
        break;
    case Maps::Ground::WASTELAND:
        index = 60;
        break;
    case Maps::Ground::BEACH:
        index = 70;
        break;

    default:
        return;
    }
    for (uint32_t ii = 0; ii < 5; ++ii)
    {
        const Sprite& sprite = AGG::GetICN(ICN::OBJNTWBA, index + ii);
        dst_pt.x = pt.x + ii * 32 + sprite.x();
        dst_pt.y = pt.y + 3 * 32 + sprite.y();
        sprite.Blit(dst_pt, destSurface);
    }
    for (uint32_t ii = 0; ii < 5; ++ii)
    {
        const Sprite& sprite = AGG::GetICN(ICN::OBJNTWBA, index + 5 + ii);
        dst_pt.x = pt.x + ii * 32 + sprite.x();
        dst_pt.y = pt.y + 4 * 32 + sprite.y();
        sprite.Blit(dst_pt, destSurface);
    }

    // draw castle
    switch (race)
    {
    case Race::KNGT:
        index = 0;
        break;
    case Race::BARB:
        index = 32;
        break;
    case Race::SORC:
        index = 64;
        break;
    case Race::WRLK:
        index = 96;
        break;
    case Race::WZRD:
        index = 128;
        break;
    case Race::NECR:
        index = 160;
        break;
    default:
        break;
    }
    if (!(BUILD_CASTLE & building)) index += 16;
    const Sprite& sprite2 = AGG::GetICN(ICN::OBJNTOWN, index);
    dst_pt.x = pt.x + 2 * 32 + sprite2.x();
    dst_pt.y = pt.y + sprite2.y();
    sprite2.Blit(dst_pt, destSurface);
    for (uint32_t ii = 0; ii < 5; ++ii)
    {
        const Sprite& sprite = AGG::GetICN(ICN::OBJNTOWN, index + 1 + ii);
        dst_pt.x = pt.x + ii * 32 + sprite.x();
        dst_pt.y = pt.y + 32 + sprite.y();
        sprite.Blit(dst_pt, destSurface);
    }
    for (uint32_t ii = 0; ii < 5; ++ii)
    {
        const Sprite& sprite = AGG::GetICN(ICN::OBJNTOWN, index + 6 + ii);
        dst_pt.x = pt.x + ii * 32 + sprite.x();
        dst_pt.y = pt.y + 2 * 32 + sprite.y();
        sprite.Blit(dst_pt, destSurface);
    }
    for (uint32_t ii = 0; ii < 5; ++ii)
    {
        const Sprite& sprite = AGG::GetICN(ICN::OBJNTOWN, index + 11 + ii);
        dst_pt.x = pt.x + ii * 32 + sprite.x();
        dst_pt.y = pt.y + 3 * 32 + sprite.y();
        sprite.Blit(dst_pt, destSurface);
    }
}

int Castle::GetICNBoat(int race)
{
    switch (race)
    {
    case Race::BARB:
        return ICN::TWNBBOAT;
    case Race::KNGT:
        return ICN::TWNKBOAT;
    case Race::NECR:
        return ICN::TWNNBOAT;
    case Race::SORC:
        return ICN::TWNSBOAT;
    case Race::WRLK:
        return ICN::TWNWBOAT;
    case Race::WZRD:
        return ICN::TWNZBOAT;
    default:
        break;
    }

    return ICN::UNKNOWN;
}

/* get building name ICN */
int Castle::GetICNBuilding(uint32_t build, int race)
{
    if (Race::BARB == race)
    {
        switch (build)
        {
        case BUILD_CASTLE:
            return ICN::TWNBCSTL;
        case BUILD_TENT:
            return ICN::TWNBTENT;
        case BUILD_SPEC:
            return ICN::TWNBSPEC;
        case BUILD_CAPTAIN:
            return ICN::TWNBCAPT;
        case BUILD_WEL2:
            return ICN::TWNBWEL2;
        case BUILD_LEFTTURRET:
            return ICN::TWNBLTUR;
        case BUILD_RIGHTTURRET:
            return ICN::TWNBRTUR;
        case BUILD_MOAT:
            return ICN::TWNBMOAT;
        case BUILD_MARKETPLACE:
            return ICN::TWNBMARK;
        case BUILD_THIEVESGUILD:
            return ICN::TWNBTHIE;
        case BUILD_TAVERN:
            return ICN::TWNBTVRN;
        case BUILD_WELL:
            return ICN::TWNBWELL;
        case BUILD_STATUE:
            return ICN::TWNBSTAT;
        case BUILD_SHIPYARD:
            return ICN::TWNBDOCK;
        case BUILD_MAGEGUILD1:
        case BUILD_MAGEGUILD2:
        case BUILD_MAGEGUILD3:
        case BUILD_MAGEGUILD4:
        case BUILD_MAGEGUILD5:
            return ICN::TWNBMAGE;
        case DWELLING_MONSTER1:
            return ICN::TWNBDW_0;
        case DWELLING_MONSTER2:
            return ICN::TWNBDW_1;
        case DWELLING_UPGRADE2:
            return ICN::TWNBUP_1;
        case DWELLING_MONSTER3:
            return ICN::TWNBDW_2;
        case DWELLING_MONSTER4:
            return ICN::TWNBDW_3;
        case DWELLING_UPGRADE4:
            return ICN::TWNBUP_3;
        case DWELLING_MONSTER5:
            return ICN::TWNBDW_4;
        case DWELLING_UPGRADE5:
            return ICN::TWNBUP_4;
        case DWELLING_MONSTER6:
            return ICN::TWNBDW_5;
        default:
            break;
        }
    }
    else if (Race::KNGT == race)
    {
        switch (build)
        {
        case BUILD_CASTLE:
            return ICN::TWNKCSTL;
        case BUILD_TENT:
            return ICN::TWNKTENT;
        case BUILD_SPEC:
            return ICN::TWNKSPEC;
        case BUILD_CAPTAIN:
            return ICN::TWNKCAPT;
        case BUILD_WEL2:
            return ICN::TWNKWEL2;
        case BUILD_LEFTTURRET:
            return ICN::TWNKLTUR;
        case BUILD_RIGHTTURRET:
            return ICN::TWNKRTUR;
        case BUILD_MOAT:
            return ICN::TWNKMOAT;
        case BUILD_MARKETPLACE:
            return ICN::TWNKMARK;
        case BUILD_THIEVESGUILD:
            return ICN::TWNKTHIE;
        case BUILD_TAVERN:
            return ICN::TWNKTVRN;
        case BUILD_WELL:
            return ICN::TWNKWELL;
        case BUILD_STATUE:
            return ICN::TWNKSTAT;
        case BUILD_SHIPYARD:
            return ICN::TWNKDOCK;
        case BUILD_MAGEGUILD1:
        case BUILD_MAGEGUILD2:
        case BUILD_MAGEGUILD3:
        case BUILD_MAGEGUILD4:
        case BUILD_MAGEGUILD5:
            return ICN::TWNKMAGE;
        case DWELLING_MONSTER1:
            return ICN::TWNKDW_0;
        case DWELLING_MONSTER2:
            return ICN::TWNKDW_1;
        case DWELLING_UPGRADE2:
            return ICN::TWNKUP_1;
        case DWELLING_MONSTER3:
            return ICN::TWNKDW_2;
        case DWELLING_UPGRADE3:
            return ICN::TWNKUP_2;
        case DWELLING_MONSTER4:
            return ICN::TWNKDW_3;
        case DWELLING_UPGRADE4:
            return ICN::TWNKUP_3;
        case DWELLING_MONSTER5:
            return ICN::TWNKDW_4;
        case DWELLING_UPGRADE5:
            return ICN::TWNKUP_4;
        case DWELLING_MONSTER6:
            return ICN::TWNKDW_5;
        case DWELLING_UPGRADE6:
            return ICN::TWNKUP_5;
        default:
            break;
        }
    }
    else if (Race::NECR == race)
    {
        switch (build)
        {
        case BUILD_CASTLE:
            return ICN::TWNNCSTL;
        case BUILD_TENT:
            return ICN::TWNNTENT;
        case BUILD_SPEC:
            return ICN::TWNNSPEC;
        case BUILD_CAPTAIN:
            return ICN::TWNNCAPT;
        case BUILD_WEL2:
            return ICN::TWNNWEL2;
        case BUILD_LEFTTURRET:
            return ICN::TWNNLTUR;
        case BUILD_RIGHTTURRET:
            return ICN::TWNNRTUR;
        case BUILD_MOAT:
            return ICN::TWNNMOAT;
        case BUILD_MARKETPLACE:
            return ICN::TWNNMARK;
        case BUILD_THIEVESGUILD:
            return ICN::TWNNTHIE;
            // shrine
        case BUILD_SHRINE:
            return ICN::TWNNTVRN;
        case BUILD_WELL:
            return ICN::TWNNWELL;
        case BUILD_STATUE:
            return ICN::TWNNSTAT;
        case BUILD_SHIPYARD:
            return ICN::TWNNDOCK;
        case BUILD_MAGEGUILD1:
        case BUILD_MAGEGUILD2:
        case BUILD_MAGEGUILD3:
        case BUILD_MAGEGUILD4:
        case BUILD_MAGEGUILD5:
            return ICN::TWNNMAGE;
        case DWELLING_MONSTER1:
            return ICN::TWNNDW_0;
        case DWELLING_MONSTER2:
            return ICN::TWNNDW_1;
        case DWELLING_UPGRADE2:
            return ICN::TWNNUP_1;
        case DWELLING_MONSTER3:
            return ICN::TWNNDW_2;
        case DWELLING_UPGRADE3:
            return ICN::TWNNUP_2;
        case DWELLING_MONSTER4:
            return ICN::TWNNDW_3;
        case DWELLING_UPGRADE4:
            return ICN::TWNNUP_3;
        case DWELLING_MONSTER5:
            return ICN::TWNNDW_4;
        case DWELLING_UPGRADE5:
            return ICN::TWNNUP_4;
        case DWELLING_MONSTER6:
            return ICN::TWNNDW_5;
        default:
            break;
        }
    }
    else if (Race::SORC == race)
    {
        switch (build)
        {
        case BUILD_CASTLE:
            return ICN::TWNSCSTL;
        case BUILD_TENT:
            return ICN::TWNSTENT;
        case BUILD_SPEC:
            return ICN::TWNSSPEC;
        case BUILD_CAPTAIN:
            return ICN::TWNSCAPT;
        case BUILD_WEL2:
            return ICN::TWNSWEL2;
        case BUILD_LEFTTURRET:
            return ICN::TWNSLTUR;
        case BUILD_RIGHTTURRET:
            return ICN::TWNSRTUR;
        case BUILD_MOAT:
            return ICN::TWNSMOAT;
        case BUILD_MARKETPLACE:
            return ICN::TWNSMARK;
        case BUILD_THIEVESGUILD:
            return ICN::TWNSTHIE;
        case BUILD_TAVERN:
            return ICN::TWNSTVRN;
        case BUILD_WELL:
            return ICN::TWNSWELL;
        case BUILD_STATUE:
            return ICN::TWNSSTAT;
        case BUILD_SHIPYARD:
            return ICN::TWNSDOCK;
        case BUILD_MAGEGUILD1:
        case BUILD_MAGEGUILD2:
        case BUILD_MAGEGUILD3:
        case BUILD_MAGEGUILD4:
        case BUILD_MAGEGUILD5:
            return ICN::TWNSMAGE;
        case DWELLING_MONSTER1:
            return ICN::TWNSDW_0;
        case DWELLING_MONSTER2:
            return ICN::TWNSDW_1;
        case DWELLING_UPGRADE2:
            return ICN::TWNSUP_1;
        case DWELLING_MONSTER3:
            return ICN::TWNSDW_2;
        case DWELLING_UPGRADE3:
            return ICN::TWNSUP_2;
        case DWELLING_MONSTER4:
            return ICN::TWNSDW_3;
        case DWELLING_UPGRADE4:
            return ICN::TWNSUP_3;
        case DWELLING_MONSTER5:
            return ICN::TWNSDW_4;
        case DWELLING_MONSTER6:
            return ICN::TWNSDW_5;
        default:
            break;
        }
    }
    else if (Race::WRLK == race)
    {
        switch (build)
        {
        case BUILD_CASTLE:
            return ICN::TWNWCSTL;
        case BUILD_TENT:
            return ICN::TWNWTENT;
        case BUILD_SPEC:
            return ICN::TWNWSPEC;
        case BUILD_CAPTAIN:
            return ICN::TWNWCAPT;
        case BUILD_WEL2:
            return ICN::TWNWWEL2;
        case BUILD_LEFTTURRET:
            return ICN::TWNWLTUR;
        case BUILD_RIGHTTURRET:
            return ICN::TWNWRTUR;
        case BUILD_MOAT:
            return ICN::TWNWMOAT;
        case BUILD_MARKETPLACE:
            return ICN::TWNWMARK;
        case BUILD_THIEVESGUILD:
            return ICN::TWNWTHIE;
        case BUILD_TAVERN:
            return ICN::TWNWTVRN;
        case BUILD_WELL:
            return ICN::TWNWWELL;
        case BUILD_STATUE:
            return ICN::TWNWSTAT;
        case BUILD_SHIPYARD:
            return ICN::TWNWDOCK;
        case BUILD_MAGEGUILD1:
        case BUILD_MAGEGUILD2:
        case BUILD_MAGEGUILD3:
        case BUILD_MAGEGUILD4:
        case BUILD_MAGEGUILD5:
            return ICN::TWNWMAGE;
        case DWELLING_MONSTER1:
            return ICN::TWNWDW_0;
        case DWELLING_MONSTER2:
            return ICN::TWNWDW_1;
        case DWELLING_MONSTER3:
            return ICN::TWNWDW_2;
        case DWELLING_MONSTER4:
            return ICN::TWNWDW_3;
        case DWELLING_UPGRADE4:
            return ICN::TWNWUP_3;
        case DWELLING_MONSTER5:
            return ICN::TWNWDW_4;
        case DWELLING_MONSTER6:
            return ICN::TWNWDW_5;
        case DWELLING_UPGRADE6:
            return ICN::TWNWUP_5;
        case DWELLING_UPGRADE7:
            return ICN::TWNWUP5B;
        default:
            break;
        }
    }
    else if (Race::WZRD == race)
    {
        switch (build)
        {
        case BUILD_CASTLE:
            return ICN::TWNZCSTL;
        case BUILD_TENT:
            return ICN::TWNZTENT;
        case BUILD_SPEC:
            return ICN::TWNZSPEC;
        case BUILD_CAPTAIN:
            return ICN::TWNZCAPT;
        case BUILD_WEL2:
            return ICN::TWNZWEL2;
        case BUILD_LEFTTURRET:
            return ICN::TWNZLTUR;
        case BUILD_RIGHTTURRET:
            return ICN::TWNZRTUR;
        case BUILD_MOAT:
            return ICN::TWNZMOAT;
        case BUILD_MARKETPLACE:
            return ICN::TWNZMARK;
        case BUILD_THIEVESGUILD:
            return ICN::TWNZTHIE;
        case BUILD_TAVERN:
            return ICN::TWNZTVRN;
        case BUILD_WELL:
            return ICN::TWNZWELL;
        case BUILD_STATUE:
            return ICN::TWNZSTAT;
        case BUILD_SHIPYARD:
            return ICN::TWNZDOCK;
        case BUILD_MAGEGUILD1:
        case BUILD_MAGEGUILD2:
        case BUILD_MAGEGUILD3:
        case BUILD_MAGEGUILD4:
        case BUILD_MAGEGUILD5:
            return ICN::TWNZMAGE;
        case DWELLING_MONSTER1:
            return ICN::TWNZDW_0;
        case DWELLING_MONSTER2:
            return ICN::TWNZDW_1;
        case DWELLING_MONSTER3:
            return ICN::TWNZDW_2;
        case DWELLING_UPGRADE3:
            return ICN::TWNZUP_2;
        case DWELLING_MONSTER4:
            return ICN::TWNZDW_3;
        case DWELLING_MONSTER5:
            return ICN::TWNZDW_4;
        case DWELLING_UPGRADE5:
            return ICN::TWNZUP_4;
        case DWELLING_MONSTER6:
            return ICN::TWNZDW_5;
        case DWELLING_UPGRADE6:
            return ICN::TWNZUP_5;
        default:
            break;
        }
    }

    return ICN::UNKNOWN;
}

CastleHeroes Castle::GetHeroes() const
{
    return world.GetHeroes(*this);
}

bool Castle::HaveNearlySea() const
{
    // check nearest ocean
    if (Maps::isValidAbsPoint(center.x, center.y + 2))
    {
        const s32 index = Maps::GetIndexFromAbsPoint(center.x, center.y + 2);
        const Maps::Tiles& left = world.GetTiles(index - 1);
        const Maps::Tiles& right = world.GetTiles(index + 1);
        const Maps::Tiles& center = world.GetTiles(index);

        return left.isWater() || right.isWater() || center.isWater();
    }
    return false;
}

bool TilePresentBoat(const Maps::Tiles& tile)
{
    return tile.isWater() &&
        (tile.GetObject() == MP2::OBJ_BOAT || tile.GetObject() == MP2::OBJ_HEROES);
}

bool Castle::PresentBoat() const
{
    // 2 cell down
    if (Maps::isValidAbsPoint(center.x, center.y + 2))
    {
        const s32 index = Maps::GetIndexFromAbsPoint(center.x, center.y + 2);
        const s32 max = world.w() * world.h();

        if (index + 1 < max)
        {
            const Maps::Tiles& left = world.GetTiles(index - 1);
            const Maps::Tiles& right = world.GetTiles(index + 1);
            const Maps::Tiles& center = world.GetTiles(index);

            if (TilePresentBoat(left) || TilePresentBoat(right) || TilePresentBoat(center)) return true;
        }
    }
    return false;
}

uint32_t Castle::GetActualDwelling(uint32_t build) const
{
    switch (build)
    {
    case DWELLING_MONSTER1:
    case DWELLING_UPGRADE2:
    case DWELLING_UPGRADE3:
    case DWELLING_UPGRADE4:
    case DWELLING_UPGRADE5:
    case DWELLING_UPGRADE7:
        return build;
    case DWELLING_MONSTER2:
        return building & DWELLING_UPGRADE2 ? DWELLING_UPGRADE2 : build;
    case DWELLING_MONSTER3:
        return building & DWELLING_UPGRADE3 ? DWELLING_UPGRADE3 : build;
    case DWELLING_MONSTER4:
        return building & DWELLING_UPGRADE4 ? DWELLING_UPGRADE4 : build;
    case DWELLING_MONSTER5:
        return building & DWELLING_UPGRADE5 ? DWELLING_UPGRADE5 : build;
    case DWELLING_MONSTER6:
        return building & DWELLING_UPGRADE7
                   ? DWELLING_UPGRADE7
                   : building & DWELLING_UPGRADE6
                   ? DWELLING_UPGRADE6
                   : build;
    case DWELLING_UPGRADE6:
        return building & DWELLING_UPGRADE7 ? DWELLING_UPGRADE7 : build;
    default:
        break;
    }

    return BUILD_NOTHING;
}

uint32_t Castle::GetUpgradeBuilding(uint32_t build) const
{
    switch (build)
    {
    case BUILD_TENT:
        return BUILD_CASTLE;
    case BUILD_MAGEGUILD1:
        return BUILD_MAGEGUILD2;
    case BUILD_MAGEGUILD2:
        return BUILD_MAGEGUILD3;
    case BUILD_MAGEGUILD3:
        return BUILD_MAGEGUILD4;
    case BUILD_MAGEGUILD4:
        return BUILD_MAGEGUILD5;
    default:
        break;
    }

    if (Race::BARB == race)
    {
        switch (build)
        {
        case DWELLING_MONSTER2:
            return DWELLING_UPGRADE2;
        case DWELLING_MONSTER4:
            return DWELLING_UPGRADE4;
        case DWELLING_MONSTER5:
            return DWELLING_UPGRADE5;
        default:
            break;
        }
    }
    else if (Race::KNGT == race)
    {
        switch (build)
        {
        case DWELLING_MONSTER2:
            return DWELLING_UPGRADE2;
        case DWELLING_MONSTER3:
            return DWELLING_UPGRADE3;
        case DWELLING_MONSTER4:
            return DWELLING_UPGRADE4;
        case DWELLING_MONSTER5:
            return DWELLING_UPGRADE5;
        case DWELLING_MONSTER6:
            return DWELLING_UPGRADE6;
        default:
            break;
        }
    }
    else if (Race::NECR == race)
    {
        switch (build)
        {
        case DWELLING_MONSTER2:
            return DWELLING_UPGRADE2;
        case DWELLING_MONSTER3:
            return DWELLING_UPGRADE3;
        case DWELLING_MONSTER4:
            return DWELLING_UPGRADE4;
        case DWELLING_MONSTER5:
            return DWELLING_UPGRADE5;
        default:
            break;
        }
    }
    else if (Race::SORC == race)
    {
        switch (build)
        {
        case DWELLING_MONSTER2:
            return DWELLING_UPGRADE2;
        case DWELLING_MONSTER3:
            return DWELLING_UPGRADE3;
        case DWELLING_MONSTER4:
            return DWELLING_UPGRADE4;
        default:
            break;
        }
    }
    else if (Race::WRLK == race)
    {
        switch (build)
        {
        case DWELLING_MONSTER4:
            return DWELLING_UPGRADE4;
        case DWELLING_MONSTER6:
            return isBuild(DWELLING_UPGRADE6) ? DWELLING_UPGRADE7 : DWELLING_UPGRADE6;
        default:
            break;
        }
    }
    else if (Race::WZRD == race)
    {
        switch (build)
        {
        case DWELLING_MONSTER3:
            return DWELLING_UPGRADE3;
        case DWELLING_MONSTER5:
            return DWELLING_UPGRADE5;
        case DWELLING_MONSTER6:
            return DWELLING_UPGRADE6;
        default:
            break;
        }
    }

    return build;
}

bool Castle::PredicateIsCapital(const Castle* castle)
{
    return castle && castle->Modes(CAPITAL);
}

bool Castle::PredicateIsCastle(const Castle* castle)
{
    return castle && castle->isCastle();
}

bool Castle::PredicateIsTown(const Castle* castle)
{
    return castle && !castle->isCastle();
}

bool Castle::PredicateIsBuildMarketplace(const Castle* castle)
{
    return castle && castle->isBuild(BUILD_MARKETPLACE);
}

string Castle::String() const
{
    ostringstream os;
    const CastleHeroes heroes = GetHeroes();
    const Heroes* hero = nullptr;

    os << "name            : " << name << endl <<
        "race            : " << Race::String(race) << endl <<
        "color           : " << Color::String(GetColor()) << endl <<
        "build           : " << "0x" << hex << building << dec << endl <<
        "present boat    : " << (PresentBoat() ? "yes" : "no") << endl <<
        "nearly sea      : " << (HaveNearlySea() ? "yes" : "no") << endl <<
        "is castle       : " << (isCastle() ? "yes" : "no") << endl <<
        "army            : " << army.String() << endl;

    if (nullptr != (hero = heroes.Guard()))
    {
        os <<
            "army guard      : " << hero->GetArmy().String() << endl;
    }

    if (nullptr != (hero = heroes.Guest()))
    {
        os <<
            "army guest      : " << hero->GetArmy().String() << endl;
    }

    return os.str();
}

int Castle::GetAttackModificator(string* strs)
{
    return 0;
}

int Castle::GetDefenseModificator(string* strs)
{
    return 0;
}

int Castle::GetPowerModificator(string* strs) const
{
    int result = 0;

    if (Race::NECR == race && isBuild(BUILD_SPEC))
    {
        const int mod = 2;
        result += mod;
        if (strs)
        {
            strs->append(GetStringBuilding(BUILD_SPEC, race));
            StringAppendModifiers(*strs, mod);
        }
    }

    return result;
}

int Castle::GetKnowledgeModificator(string* strs)
{
    return 0;
}

int Castle::GetMoraleModificator(string* strs) const
{
    int result = Morale::NORMAL;

    // and tavern
    if (isBuild(BUILD_TAVERN))
    {
        const int mod = 1;
        result += mod;
        if (strs)
        {
            strs->append(GetStringBuilding(BUILD_TAVERN, race));
            StringAppendModifiers(*strs, mod);
            strs->append("\n");
        }
    }

    // and barbarian coliseum
    if (Race::BARB == race && isBuild(BUILD_SPEC))
    {
        const int mod = 2;
        result += mod;
        if (strs)
        {
            strs->append(GetStringBuilding(BUILD_SPEC, race));
            StringAppendModifiers(*strs, mod);
            strs->append("\n");
        }
    }

    return result;
}

int Castle::GetLuckModificator(string* strs) const
{
    int result = Luck::NORMAL;

    if (Race::SORC == race && isBuild(BUILD_SPEC))
    {
        const int mod = 2;
        result += mod;
        if (strs)
        {
            strs->append(GetStringBuilding(BUILD_SPEC, race));
            StringAppendModifiers(*strs, mod);
            strs->append("\n");
        }
    }

    return result;
}

void Castle::RecruitAllMonster()
{
    bool skip_recruit = false;

    // skip recruit: AI with customization of empty army
    if (Modes(CUSTOMARMY) &&
        isControlAI() &&
        !army.m_troops.IsValid() && !army.m_troops.HasMonster(Monster(Monster::UNKNOWN)))
        skip_recruit = true;

    if (!skip_recruit)
        for (uint32_t dw = DWELLING_MONSTER6; dw >= DWELLING_MONSTER1; dw >>= 1)
            if (isBuild(dw)) RecruitMonsterFromDwelling(dw, GetDwellingLivedCount(dw));
}

const Army& Castle::GetArmy() const
{
    const CastleHeroes heroes = world.GetHeroes(*this);
    return heroes.Guard() ? heroes.Guard()->GetArmy() : army;
}

Army& Castle::GetArmy()
{
    CastleHeroes heroes = world.GetHeroes(*this);
    return heroes.Guard() ? heroes.Guard()->GetArmy() : army;
}

const Army& Castle::GetActualArmy() const
{
    CastleHeroes heroes = world.GetHeroes(*this);
    const Heroes* hero = heroes.GuardFirst();
    return hero ? hero->GetArmy() : army;
}

Army& Castle::GetActualArmy()
{
    CastleHeroes heroes = world.GetHeroes(*this);
    Heroes* hero = heroes.GuardFirst();
    return hero ? hero->GetArmy() : army;
}

bool Castle::AllowBuyBoat() const
{
    // check payment and present other boat
    return HaveNearlySea() && GetKingdom().AllowPayment(PaymentConditions::BuyBoat()) && !PresentBoat();
}

bool Castle::BuyBoat() const
{
    if (!AllowBuyBoat()) return false;
    if (isControlHuman()) AGG::PlaySound(M82::BUILDTWN);

    if (!Maps::isValidAbsPoint(center.x, center.y + 2))
        return false;

    const s32 index = Maps::GetIndexFromAbsPoint(center.x, center.y + 2);
    Maps::Tiles& left = world.GetTiles(index - 1);
    Maps::Tiles& right = world.GetTiles(index + 1);
    Maps::Tiles& center = world.GetTiles(index);
    Kingdom& kingdom = GetKingdom();

    if (MP2::OBJ_ZERO == left.GetObject() && left.isWater())
    {
        kingdom.OddFundsResource(PaymentConditions::BuyBoat());

        left.SetObject(MP2::OBJ_BOAT);
    }
    else if (MP2::OBJ_ZERO == right.GetObject() && right.isWater())
    {
        kingdom.OddFundsResource(PaymentConditions::BuyBoat());

        right.SetObject(MP2::OBJ_BOAT);
    }
    else if (MP2::OBJ_ZERO == center.GetObject() && center.isWater())
    {
        kingdom.OddFundsResource(PaymentConditions::BuyBoat());

        center.SetObject(MP2::OBJ_BOAT);
    }

    return true;
}

int Castle::GetRace() const
{
    return race;
}

const string& Castle::GetName() const
{
    return name;
}

int Castle::GetControl() const
{
    /* gray towns: ai control */
    return GetColor() & Color::ALL ? GetKingdom().GetControl() : CONTROL_AI;
}

bool Castle::AllowBuild() const
{
    return Modes(ALLOWBUILD);
}

bool Castle::isBuild(uint32_t bd) const
{
    return building & bd;
}

bool Castle::isNecromancyShrineBuild() const
{
    return race == Race::NECR && BUILD_SHRINE & building;
}

uint32_t Castle::GetGrownWell()
{
    return GameStatic::GetCastleGrownWell();
}

uint32_t Castle::GetGrownWel2()
{
    return GameStatic::GetCastleGrownWel2();
}

uint32_t Castle::GetGrownWeekOf(const Monster& mons)
{
    return Settings::Get().ExtWorldNewVersionWeekOf() ? mons.GetGrown() : GameStatic::GetCastleGrownWeekOf();
}

uint32_t Castle::GetGrownMonthOf()
{
    return GameStatic::GetCastleGrownMonthOf();
}

void Castle::Scoute() const
{
    Maps::ClearFog(GetIndex(), GetViewDistance(isCastle() ? Game::VIEW_CASTLE : Game::VIEW_TOWN), GetColor());
}

void Castle::JoinRNDArmy()
{
    const Monster mon1(race, DWELLING_MONSTER1);
    const Monster mon2(race, DWELLING_MONSTER2);
    const Monster mon3(race, DWELLING_MONSTER3);

    switch (Rand::Get(1, 4))
    {
    case 1:
        army.m_troops.JoinTroop(mon1, mon1.GetRNDSize(false) * 3);
        army.m_troops.JoinTroop(mon2, mon2.GetRNDSize(false));
        break;

    case 2:
        army.m_troops.JoinTroop(mon1, mon1.GetRNDSize(false) * 2);
        army.m_troops.JoinTroop(mon2, mon2.GetRNDSize(false) * 2);
        break;

    case 3:

        army.m_troops.JoinTroop(mon1, mon1.GetRNDSize(false) * 2);
        army.m_troops.JoinTroop(mon2, mon2.GetRNDSize(false));
        army.m_troops.JoinTroop(mon3, mon3.GetRNDSize(false) * 2 / 3);
        break;

    default:
        army.m_troops.JoinTroop(mon1, mon1.GetRNDSize(false));
        army.m_troops.JoinTroop(mon3, mon3.GetRNDSize(false));
        break;
    }
}

void Castle::ActionPreBattle()
{
    if (isControlAI())
        AI::CastlePreBattle(*this);
    else if (Settings::Get().ExtBattleMergeArmies())
    {
        CastleHeroes heroes = world.GetHeroes(*this);
        Heroes* hero = heroes.GuardFirst();
        if (hero && army.m_troops.IsValid())
            hero->GetArmy().JoinStrongestFromArmy(army);
    }
}

void Castle::ActionAfterBattle(bool attacker_wins)
{
    if (attacker_wins)
    {
        army.m_troops.Clean();
        ResetModes(CUSTOMARMY);
    }

    if (isControlAI())
        AI::CastleAfterBattle(*this, attacker_wins);
}

Castle* VecCastles::Get(const Point& position) const
{
    const auto it = find_if(_items.begin(), _items.end(), [&](auto& castle)
    {
        return castle->isPosition(position);
    });
    return _items.end() != it ? *it : nullptr;
}

Castle* VecCastles::GetFirstCastle() const
{
    auto it = find_if(_items.begin(), _items.end(), [](auto& castle)
    {
        return castle->isCastle();
    });
    return _items.end() != it ? *it : nullptr;
}

void VecCastles::ChangeColors(int col1, int col2)
{
    for (auto& it : _items)
        if (it->GetColor() == col1) it->ChangeColor(col2);
}

AllCastles::AllCastles()
{
    // reserve memory
    _items.reserve(MAXCASTLES);
}

AllCastles::~AllCastles()
{
    clear();
}

void AllCastles::Init()
{
    if (!_items.empty())
        clear();
}

void AllCastles::clear()
{
    for (auto& it : _items)
        delete it;
    _items.clear();
}

void AllCastles::Scoute(int colors) const
{
    for (auto it : _items)
        if (colors & it->GetColor()) it->Scoute();
}


ByteVectorWriter& operator<<(ByteVectorWriter& msg, const Castle& castle)
{
    const ColorBase& color = castle;

    msg <<
        static_cast<const MapPosition &>(castle) <<
        castle.modes <<
        castle.race <<
        castle.building <<
        castle.captain <<
        color <<
        castle.name <<
        castle.mageguild <<
        static_cast<uint32_t>(CASTLEMAXMONSTER);

    for (unsigned int ii : castle.dwelling)
        msg << ii;

    return msg << castle.army;
}

ByteVectorReader& operator>>(ByteVectorReader& msg, Castle& castle)
{
    ColorBase& color = castle;
    uint32_t dwellingcount;

    msg >>
        static_cast<MapPosition &>(castle) >>
        castle.modes >>
        castle.race >>
        castle.building >>
        castle.captain >>
        color >>
        castle.name >>
        castle.mageguild;

    msg >> dwellingcount;
    for (uint32_t ii = 0; ii < dwellingcount; ++ii)
        msg >> castle.dwelling[ii];

    msg >> castle.army;
    castle.army.SetCommander(&castle.captain);

    return msg;
}

ByteVectorWriter& operator<<(ByteVectorWriter& msg, const VecCastles& castles)
{
    msg << static_cast<uint32_t>(castles._items.size());

    for (auto castle : castles._items)
        msg << (castle ? castle->GetIndex() : static_cast<s32>(-1));

    return msg;
}

ByteVectorReader& operator>>(ByteVectorReader& msg, VecCastles& castles)
{
    s32 index;
    uint32_t size;
    msg >> size;

    castles._items.resize(size, nullptr);

    for (auto& castle : castles._items)
    {
        msg >> index;
        castle = index < 0 ? nullptr : world.GetCastle(Maps::GetPoint(index));
    }

    return msg;
}

ByteVectorWriter& operator<<(ByteVectorWriter& msg, const AllCastles& castles)
{
    msg << static_cast<uint32_t>(castles._items.size());

    for (auto castle : castles._items)
        msg << *castle;

    return msg;
}

ByteVectorReader& operator>>(ByteVectorReader& msg, AllCastles& castles)
{
    uint32_t size;
    msg >> size;

    castles.clear();
    castles._items.resize(size, nullptr);

    for (auto& castle : castles._items)
    {
        castle = new Castle();
        msg >> *castle;
    }

    return msg;
}

void Castle::SwapCastleHeroes(CastleHeroes& heroes)
{
    if (heroes.Guest() && heroes.Guard())
    {
        heroes.Guest()->SetModes(Heroes::GUARDIAN);
        heroes.Guest()->ResetModes(Heroes::SLEEPER);
        heroes.Guard()->ResetModes(Heroes::GUARDIAN);
        heroes.Swap();

        world.GetTiles(center.x, center.y).SetHeroes(nullptr);

        Point position(heroes.Guard()->GetCenter());
        position.y -= 1;
        heroes.Guard()->SetCenter(position);
        heroes.Guard()->GetPath().Reset();

        position = heroes.Guest()->GetCenter();
        position.y += 1;
        heroes.Guest()->SetCenter(position);
        heroes.Guest()->GetPath().Reset();

        world.GetTiles(center.x, center.y).SetHeroes(heroes.Guest());
    }
    else if (heroes.Guest() && !heroes.Guard())
    {
        heroes.Guest()->SetModes(Heroes::GUARDIAN);
        heroes.Guest()->ResetModes(Heroes::SLEEPER);
        heroes.Swap();
        heroes.Guard()->GetArmy().m_troops.JoinTroops(army.m_troops);

        world.GetTiles(center.x, center.y).SetHeroes(nullptr);

        Point position(heroes.Guard()->GetCenter());
        position.y -= 1;
        heroes.Guard()->SetCenter(position);
        heroes.Guard()->GetPath().Reset();
    }
    else if (!heroes.Guest() && heroes.Guard())
    {
        heroes.Guard()->ResetModes(Heroes::GUARDIAN);
        heroes.Swap();

        Point position(heroes.Guest()->GetCenter());
        position.y += 1;
        heroes.Guest()->SetCenter(position);
        heroes.Guest()->GetPath().Reset();

        world.GetTiles(center.x, center.y).SetHeroes(heroes.Guest());
    }
}

string Castle::GetStringBuilding(uint32_t build) const
{
    return GetStringBuilding(build, GetRace());
}

string Castle::GetDescriptionBuilding(uint32_t build) const
{
    string res = GetDescriptionBuilding(build, GetRace());

    switch (build)
    {
    case BUILD_WELL:
        StringReplace(res, "%{count}", GetGrownWell());
        break;

    case BUILD_WEL2:
        StringReplace(res, "%{count}", GetGrownWel2());
        break;

    case BUILD_CASTLE:
        {
            StringReplace(res, "%{count}", ProfitConditions::FromBuilding(BUILD_CASTLE, race).gold);

            if (isBuild(BUILD_CASTLE))
            {
                res.append("\n \n");
                res.append(Battle::Tower::GetInfo(*this));
            }

            if (isBuild(BUILD_MOAT))
            {
                res.append("\n \n");
                res.append(Battle::Board::GetMoatInfo());
            }
        }
        break;

    case BUILD_SPEC:
    case BUILD_STATUE:
        {
            payment_t profit = ProfitConditions::FromBuilding(build, GetRace());
            StringReplace(res, "%{count}", profit.gold);
        }
        break;

    default:
        break;
    }

    return res;
}
