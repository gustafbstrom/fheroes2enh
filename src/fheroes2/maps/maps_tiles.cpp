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

#include <iomanip>
#include <algorithm>
#include "agg.h"
#include "world.h"
#include "race.h"
#include "settings.h"
#include "ground.h"
#include "game.h"
#include "game_interface.h"
#include "mounts.h"
#include "icn.h"
#include "trees.h"
#include "objxloc.h"
#include "objtown.h"
#include "objwatr.h"
#include "objlava.h"
#include "objmult.h"
#include "objdsrt.h"
#include "objdirt.h"
#include "objsnow.h"
#include "objswmp.h"
#include "objgras.h"
#include "objcrck.h"
#include "til.h"
#include <sstream>

u8 monster_animation_cicle[] = {0, 1, 2, 1, 0, 3, 4, 5, 4, 3};


Maps::TilesAddon::TilesAddon() : uniq(0), level(0), object(0), index(0), tmp(0)
{
}

Maps::TilesAddon::TilesAddon(int lv, uint32_t gid, int obj, uint32_t ii) : uniq(gid), level(lv), object(obj), index(ii),
                                                                           tmp(0)
{
}

string Maps::TilesAddon::String(int lvl) const
{
    ostringstream os;
    os << "----------------" << lvl << "--------" << endl <<
        "uniq            : " << uniq << endl <<
        "object          : " << "0x" << setw(2) << setfill('0') << hex << static_cast<int>(object) <<
        ", (" << ICN::GetString(MP2::GetICNObject(object)) << ")" << endl <<
        "index           : " << dec << static_cast<int>(index) << endl <<
        "level           : " << static_cast<int>(level) << ", (" << level % 4 << ")" << endl <<
        "tmp             : " << static_cast<int>(tmp) << endl;
    return os.str();
}

Maps::TilesAddon& Maps::TilesAddon::operator=(const TilesAddon& ta)
{
    level = ta.level;
    object = ta.object;
    index = ta.index;
    uniq = ta.uniq;

    return *this;
}

bool Maps::TilesAddon::isUniq(uint32_t id) const
{
    return uniq == id;
}

bool Maps::TilesAddon::isICN(int icn) const
{
    return icn == MP2::GetICNObject(object);
}

bool Maps::TilesAddon::PredicateSortRules1(const TilesAddon& ta1, const TilesAddon& ta2)
{
    return ta1.level % 4 > ta2.level % 4;
}

bool Maps::TilesAddon::PredicateSortRules2(const TilesAddon& ta1, const TilesAddon& ta2)
{
    return ta1.level % 4 < ta2.level % 4;
}

int Maps::TilesAddon::GetLoyaltyObject(const TilesAddon& addon)
{
    switch (MP2::GetICNObject(addon.object))
    {
    case ICN::X_LOC1:
        if (addon.index == 3) return MP2::OBJ_ALCHEMYTOWER;
        if (addon.index < 3) return MP2::OBJN_ALCHEMYTOWER;
        if (70 == addon.index) return MP2::OBJ_ARENA;
        if (3 < addon.index && addon.index < 72) return MP2::OBJN_ARENA;
        if (77 == addon.index) return MP2::OBJ_BARROWMOUNDS;
        if (71 < addon.index && addon.index < 78) return MP2::OBJN_BARROWMOUNDS;
        if (94 == addon.index) return MP2::OBJ_EARTHALTAR;
        if (77 < addon.index && addon.index < 112) return MP2::OBJN_EARTHALTAR;
        if (118 == addon.index) return MP2::OBJ_AIRALTAR;
        if (111 < addon.index && addon.index < 120) return MP2::OBJN_AIRALTAR;
        if (127 == addon.index) return MP2::OBJ_FIREALTAR;
        if (119 < addon.index && addon.index < 129) return MP2::OBJN_FIREALTAR;
        if (135 == addon.index) return MP2::OBJ_WATERALTAR;
        if (128 < addon.index && addon.index < 137) return MP2::OBJN_WATERALTAR;
        break;

    case ICN::X_LOC2:
        if (addon.index == 4) return MP2::OBJ_STABLES;
        if (addon.index < 4) return MP2::OBJN_STABLES;
        if (addon.index == 9) return MP2::OBJ_JAIL;
        if (4 < addon.index && addon.index < 10) return MP2::OBJN_JAIL;
        if (addon.index == 37) return MP2::OBJ_MERMAID;
        if (9 < addon.index && addon.index < 47) return MP2::OBJN_MERMAID;
        if (addon.index == 101) return MP2::OBJ_SIRENS;
        if (46 < addon.index && addon.index < 111) return MP2::OBJN_SIRENS;
        if (110 < addon.index && addon.index < 136) return MP2::OBJ_REEFS;
        break;

    case ICN::X_LOC3:
        if (addon.index == 30) return MP2::OBJ_HUTMAGI;
        if (addon.index < 32) return MP2::OBJN_HUTMAGI;
        if (addon.index == 50) return MP2::OBJ_EYEMAGI;
        if (31 < addon.index && addon.index < 59) return MP2::OBJN_EYEMAGI;
        // fix
        break;

    default:
        break;
    }

    return MP2::OBJ_ZERO;
}

int Maps::TilesAddon::GetPassable(const TilesAddon& ta)
{
    int icn = MP2::GetICNObject(ta.object);

    switch (icn)
    {
    case ICN::MTNSNOW:
    case ICN::MTNSWMP:
    case ICN::MTNLAVA:
    case ICN::MTNDSRT:
    case ICN::MTNMULT:
    case ICN::MTNGRAS:
        return ObjMnts1::GetPassable(icn, ta.index);

    case ICN::MTNCRCK:
    case ICN::MTNDIRT:
        return ObjMnts2::GetPassable(icn, ta.index);

    case ICN::TREJNGL:
    case ICN::TREEVIL:
    case ICN::TRESNOW:
    case ICN::TREFIR:
    case ICN::TREFALL:
    case ICN::TREDECI:
        return ObjTree::GetPassable(ta.index);
    case ICN::OBJNSNOW:
        return ObjSnow::GetPassable(ta.index);
    case ICN::OBJNSWMP:
        return ObjSwmp::GetPassable(ta.index);
    case ICN::OBJNGRAS:
        return ObjGras::GetPassable(ta.index);
    case ICN::OBJNGRA2:
        return ObjGra2::GetPassable(ta.index);
    case ICN::OBJNCRCK:
        return ObjCrck::GetPassable(ta.index);
    case ICN::OBJNDIRT:
        return ObjDirt::GetPassable(ta.index);
    case ICN::OBJNDSRT:
        return ObjDsrt::GetPassable(ta.index);
    case ICN::OBJNMUL2:
        return ObjMul2::GetPassable(ta.index);
    case ICN::OBJNMULT:
        return ObjMult::GetPassable(ta.index);
    case ICN::OBJNLAVA:
        return ObjLava::GetPassable(ta.index);
    case ICN::OBJNLAV3:
        return ObjLav3::GetPassable(ta.index);
    case ICN::OBJNLAV2:
        return ObjLav2::GetPassable(ta.index);
    case ICN::OBJNWAT2:
        return ObjWat2::GetPassable(ta.index);
    case ICN::OBJNWATR:
        return ObjWatr::GetPassable(ta.index);

    case ICN::OBJNTWBA:
        return ObjTwba::GetPassable(ta.index);
    case ICN::OBJNTOWN:
        return ObjTown::GetPassable(ta.index);

    case ICN::X_LOC1:
        return ObjXlc1::GetPassable(ta.index);
    case ICN::X_LOC2:
        return ObjXlc2::GetPassable(ta.index);
    case ICN::X_LOC3:
        return ObjXlc3::GetPassable(ta.index);

        // MANUAL.ICN
    case ICN::TELEPORT1:
    case ICN::TELEPORT2:
    case ICN::TELEPORT3:
    case ICN::FOUNTAIN:
        return 0;

    default:
        break;
    }

    return DIRECTION_ALL;
}

int Maps::TilesAddon::GetActionObject(const TilesAddon& ta)
{
    switch (MP2::GetICNObject(ta.object))
    {
    case ICN::MTNSNOW:
    case ICN::MTNSWMP:
    case ICN::MTNLAVA:
    case ICN::MTNDSRT:
    case ICN::MTNMULT:
    case ICN::MTNGRAS:
        return ObjMnts1::GetActionObject(ta.index);

    case ICN::MTNCRCK:
    case ICN::MTNDIRT:
        return ObjMnts2::GetActionObject(ta.index);

    case ICN::TREJNGL:
    case ICN::TREEVIL:
    case ICN::TRESNOW:
    case ICN::TREFIR:
    case ICN::TREFALL:
    case ICN::TREDECI:
        return ObjTree::GetActionObject(ta.index);

    case ICN::OBJNSNOW:
        return ObjSnow::GetActionObject(ta.index);
    case ICN::OBJNSWMP:
        return ObjSwmp::GetActionObject(ta.index);
    case ICN::OBJNGRAS:
        return ObjGras::GetActionObject(ta.index);
    case ICN::OBJNGRA2:
        return ObjGra2::GetActionObject(ta.index);
    case ICN::OBJNCRCK:
        return ObjCrck::GetActionObject(ta.index);
    case ICN::OBJNDIRT:
        return ObjDirt::GetActionObject(ta.index);
    case ICN::OBJNDSRT:
        return ObjDsrt::GetActionObject(ta.index);
    case ICN::OBJNMUL2:
        return ObjMul2::GetActionObject(ta.index);
    case ICN::OBJNMULT:
        return ObjMult::GetActionObject(ta.index);
    case ICN::OBJNLAVA:
        return ObjLava::GetActionObject(ta.index);
    case ICN::OBJNLAV3:
        return ObjLav3::GetActionObject(ta.index);
    case ICN::OBJNLAV2:
        return ObjLav2::GetActionObject(ta.index);
    case ICN::OBJNWAT2:
        return ObjWat2::GetActionObject(ta.index);
    case ICN::OBJNWATR:
        return ObjWatr::GetActionObject(ta.index);

    case ICN::OBJNTWBA:
        return ObjTwba::GetActionObject(ta.index);
    case ICN::OBJNTOWN:
        return ObjTown::GetActionObject(ta.index);

    case ICN::X_LOC1:
        return ObjXlc1::GetActionObject(ta.index);
    case ICN::X_LOC2:
        return ObjXlc2::GetActionObject(ta.index);
    case ICN::X_LOC3:
        return ObjXlc3::GetActionObject(ta.index);

        // MANUAL.ICN
    case ICN::TELEPORT1:
    case ICN::TELEPORT2:
    case ICN::TELEPORT3:
        return MP2::OBJ_STONELIGHTS;
    case ICN::FOUNTAIN:
        return MP2::OBJ_FOUNTAIN;

    default:
        break;
    }

    return MP2::OBJ_ZERO;
}

bool Maps::TilesAddon::isRoad(int direct) const
{
    switch (MP2::GetICNObject(object))
    {
        // from sprite road
    case ICN::ROAD:
        if (0 == index ||
            4 == index ||
            5 == index ||
            13 == index ||
            26 == index)
            return direct & (Direction::TOP | Direction::BOTTOM);
        if (2 == index ||
            21 == index ||
            28 == index)
            return direct & (Direction::LEFT | Direction::RIGHT);
        if (17 == index ||
            29 == index)
            return direct & (Direction::TOP_LEFT | Direction::BOTTOM_RIGHT);
        if (18 == index ||
            30 == index)
            return direct & (Direction::TOP_RIGHT | Direction::BOTTOM_LEFT);
        if (3 == index)
            return direct & (Direction::TOP | Direction::BOTTOM | Direction::LEFT | Direction::RIGHT);
        if (6 == index) return direct & (Direction::TOP | Direction::BOTTOM | Direction::RIGHT);
        if (7 == index) return direct & (Direction::TOP | Direction::RIGHT);
        if (9 == index) return direct & (Direction::BOTTOM | Direction::RIGHT);
        if (12 == index) return direct & (Direction::BOTTOM | Direction::LEFT);
        if (14 == index) return direct & (Direction::TOP | Direction::BOTTOM | Direction::LEFT);
        if (16 == index) return direct & (Direction::TOP | Direction::LEFT);
        if (19 == index) return direct & (Direction::TOP_LEFT | Direction::BOTTOM_RIGHT);
        if (20 == index) return direct & (Direction::TOP_RIGHT | Direction::BOTTOM_LEFT);
        break;

        // castle and tower (gate)
    case ICN::OBJNTOWN:
        if (13 == index ||
            29 == index ||
            45 == index ||
            61 == index ||
            77 == index ||
            93 == index ||
            109 == index ||
            125 == index ||
            141 == index ||
            157 == index ||
            173 == index ||
            189 == index)
            return direct & (Direction::TOP | Direction::BOTTOM);
        break;
        // castle lands (gate)
    case ICN::OBJNTWBA:
        if (7 == index ||
            17 == index ||
            27 == index ||
            37 == index ||
            47 == index ||
            57 == index ||
            67 == index ||
            77 == index)
            return direct & (Direction::TOP | Direction::BOTTOM);
        break;

    default:
        break;
    }

    return false;
}

bool Maps::TilesAddon::isStream(const TilesAddon& ta)
{
    return ICN::STREAM == MP2::GetICNObject(ta.object) ||
        (ICN::OBJNMUL2 == MP2::GetICNObject(ta.object) && ta.index < 14);
}

bool Maps::TilesAddon::isRoad(const TilesAddon& ta)
{
    return ICN::ROAD == MP2::GetICNObject(ta.object);
}

bool Maps::TilesAddon::isWaterResource(const TilesAddon& ta)
{
    return ICN::OBJNWATR == MP2::GetICNObject(ta.object) &&
        (0 == ta.index || // buttle
            19 == ta.index || // chest
            45 == ta.index || // flotsam
            111 == ta.index) // surviror
        ;
}

bool Maps::TilesAddon::isWhirlPool(const TilesAddon& ta)
{
    return ICN::OBJNWATR == MP2::GetICNObject(ta.object) &&
        (ta.index >= 202 && ta.index <= 225);
}

bool Maps::TilesAddon::isStandingStone(const TilesAddon& ta)
{
    return ICN::OBJNMULT == MP2::GetICNObject(ta.object) &&
        (ta.index == 84 || ta.index == 85);
}

bool Maps::TilesAddon::isResource(const TilesAddon& ta)
{
    // OBJNRSRC
    return (ICN::OBJNRSRC == MP2::GetICNObject(ta.object) && ta.index % 2) ||
        // TREASURE
        ICN::TREASURE == MP2::GetICNObject(ta.object);
}

bool Maps::TilesAddon::isRandomResource(const TilesAddon& ta)
{
    // OBJNRSRC
    return ICN::OBJNRSRC == MP2::GetICNObject(ta.object) && 17 == ta.index;
}

bool Maps::TilesAddon::isArtifact(const TilesAddon& ta)
{
    // OBJNARTI (skip ultimate)
    return ICN::OBJNARTI == MP2::GetICNObject(ta.object) && ta.index > 0x10 && ta.index % 2;
}

bool Maps::TilesAddon::isRandomArtifact(const TilesAddon& ta)
{
    // OBJNARTI
    return ICN::OBJNARTI == MP2::GetICNObject(ta.object) && 0xA3 == ta.index;
}

bool Maps::TilesAddon::isRandomArtifact1(const TilesAddon& ta)
{
    // OBJNARTI
    return ICN::OBJNARTI == MP2::GetICNObject(ta.object) && 0xA7 == ta.index;
}

bool Maps::TilesAddon::isRandomArtifact2(const TilesAddon& ta)
{
    // OBJNARTI
    return ICN::OBJNARTI == MP2::GetICNObject(ta.object) && 0xA9 == ta.index;
}

bool Maps::TilesAddon::isRandomArtifact3(const TilesAddon& ta)
{
    // OBJNARTI
    return ICN::OBJNARTI == MP2::GetICNObject(ta.object) && 0xAB == ta.index;
}

bool Maps::TilesAddon::isUltimateArtifact(const TilesAddon& ta)
{
    // OBJNARTI
    return ICN::OBJNARTI == MP2::GetICNObject(ta.object) && 0xA4 == ta.index;
}

bool Maps::TilesAddon::isCampFire(const TilesAddon& ta)
{
    // MTNDSRT
    return (ICN::OBJNDSRT == MP2::GetICNObject(ta.object) && 61 == ta.index) ||
        // OBJNMULT
        (ICN::OBJNMULT == MP2::GetICNObject(ta.object) && 131 == ta.index) ||
        // OBJNSNOW
        (ICN::OBJNSNOW == MP2::GetICNObject(ta.object) && 4 == ta.index);
}

bool Maps::TilesAddon::isMonster(const TilesAddon& ta)
{
    // MONS32
    return ICN::MONS32 == MP2::GetICNObject(ta.object);
}

bool Maps::TilesAddon::isArtesianSpring(const TilesAddon& ta)
{
    return ICN::OBJNCRCK == MP2::GetICNObject(ta.object) &&
        (ta.index == 3 || ta.index == 4);
}

bool Maps::TilesAddon::isSkeleton(const TilesAddon& ta)
{
    return ICN::OBJNDSRT == MP2::GetICNObject(ta.object) && ta.index == 84;
}

bool Maps::TilesAddon::isSkeletonFix(const TilesAddon& ta)
{
    return ICN::OBJNDSRT == MP2::GetICNObject(ta.object) && ta.index == 83;
}

bool Maps::TilesAddon::isOasis(const TilesAddon& ta)
{
    return ICN::OBJNDSRT == MP2::GetICNObject(ta.object) &&
        (ta.index == 108 || ta.index == 109);
}

bool Maps::TilesAddon::isWateringHole(const TilesAddon& ta)
{
    return ICN::OBJNCRCK == MP2::GetICNObject(ta.object) &&
        (ta.index >= 217 && ta.index <= 220);
}

bool Maps::TilesAddon::isJail(const TilesAddon& ta)
{
    return ICN::X_LOC2 == MP2::GetICNObject(ta.object) && 0x09 == ta.index;
}

bool Maps::TilesAddon::isEvent(const TilesAddon& ta)
{
    // OBJNMUL2
    return ICN::OBJNMUL2 == MP2::GetICNObject(ta.object) && 0xA3 == ta.index;
}

bool Maps::TilesAddon::isMine(const TilesAddon& ta)
{
    // EXTRAOVR
    return ICN::EXTRAOVR == MP2::GetICNObject(ta.object);
}

bool Maps::TilesAddon::isBoat(const TilesAddon& ta)
{
    // OBJNWAT2
    return ICN::OBJNWAT2 == MP2::GetICNObject(ta.object) && 0x17 == ta.index;
}

bool Maps::TilesAddon::isMiniHero(const TilesAddon& ta)
{
    // MINIHERO
    return ICN::MINIHERO == MP2::GetICNObject(ta.object);
}

bool Maps::TilesAddon::isCastle(const TilesAddon& ta)
{
    // OBJNTOWN
    return ICN::OBJNTOWN == MP2::GetICNObject(ta.object);
}

bool Maps::TilesAddon::isRandomCastle(const TilesAddon& ta)
{
    // OBJNTWRD
    return ICN::OBJNTWRD == MP2::GetICNObject(ta.object) && 32 > ta.index;
}

bool Maps::TilesAddon::isRandomMonster(const TilesAddon& ta)
{
    // MONS32
    return ICN::MONS32 == MP2::GetICNObject(ta.object) &&
        (0x41 < ta.index && 0x47 > ta.index);
}

bool Maps::TilesAddon::isBarrier(const TilesAddon& ta)
{
    return ICN::X_LOC3 == MP2::GetICNObject(ta.object) &&
        60 <= ta.index && 102 >= ta.index && 0 == ta.index % 6;
}

int Maps::TilesAddon::ColorFromBarrierSprite(const TilesAddon& ta)
{
    // 60, 66, 72, 78, 84, 90, 96, 102
    return ICN::X_LOC3 == MP2::GetICNObject(ta.object) &&
           60 <= ta.index && 102 >= ta.index
               ? (ta.index - 60) / 6 + 1
               : 0;
}

int Maps::TilesAddon::ColorFromTravellerTentSprite(const TilesAddon& ta)
{
    // 110, 114, 118, 122, 126, 130, 134, 138
    return ICN::X_LOC3 == MP2::GetICNObject(ta.object) &&
           110 <= ta.index && 138 >= ta.index
               ? (ta.index - 110) / 4 + 1
               : 0;
}

bool Maps::TilesAddon::isAbandoneMineSprite(const TilesAddon& ta)
{
    return (ICN::OBJNGRAS == MP2::GetICNObject(ta.object) && 6 == ta.index) ||
        (ICN::OBJNDIRT == MP2::GetICNObject(ta.object) && 8 == ta.index);
}

bool Maps::TilesAddon::isFlag32(const TilesAddon& ta)
{
    return ICN::FLAG32 == MP2::GetICNObject(ta.object);
}

bool Maps::TilesAddon::isX_LOC123(const TilesAddon& ta)
{
    return ICN::X_LOC1 == MP2::GetICNObject(ta.object) ||
        ICN::X_LOC2 == MP2::GetICNObject(ta.object) ||
        ICN::X_LOC3 == MP2::GetICNObject(ta.object);
}

bool Maps::TilesAddon::isShadow(const TilesAddon& ta)
{
    const int icn = MP2::GetICNObject(ta.object);

    switch (icn)
    {
    case ICN::MTNDSRT:
    case ICN::MTNGRAS:
    case ICN::MTNLAVA:
    case ICN::MTNMULT:
    case ICN::MTNSNOW:
    case ICN::MTNSWMP:
        return ObjMnts1::isShadow(ta.index);

    case ICN::MTNCRCK:
    case ICN::MTNDIRT:
        return ObjMnts2::isShadow(ta.index);

    case ICN::TREDECI:
    case ICN::TREEVIL:
    case ICN::TREFALL:
    case ICN::TREFIR:
    case ICN::TREJNGL:
    case ICN::TRESNOW:
        return ObjTree::isShadow(ta.index);

    case ICN::OBJNCRCK:
        return ObjCrck::isShadow(ta.index);
    case ICN::OBJNDIRT:
        return ObjDirt::isShadow(ta.index);
    case ICN::OBJNDSRT:
        return ObjDsrt::isShadow(ta.index);
    case ICN::OBJNGRA2:
        return ObjGra2::isShadow(ta.index);
    case ICN::OBJNGRAS:
        return ObjGras::isShadow(ta.index);
    case ICN::OBJNMUL2:
        return ObjMul2::isShadow(ta.index);
    case ICN::OBJNMULT:
        return ObjMult::isShadow(ta.index);
    case ICN::OBJNSNOW:
        return ObjSnow::isShadow(ta.index);
    case ICN::OBJNSWMP:
        return ObjSwmp::isShadow(ta.index);
    case ICN::OBJNWAT2:
        return ObjWat2::isShadow(ta.index);
    case ICN::OBJNWATR:
        return ObjWatr::isShadow(ta.index);

    case ICN::OBJNARTI:
    case ICN::OBJNRSRC:
        return 0 == ta.index % 2;

    case ICN::OBJNTWRD:
        return ta.index > 31;
    case ICN::OBJNTWSH:
        return true;

    default:
        break;
    }

    return false;
}

bool Maps::TilesAddon::isMounts(const TilesAddon& ta)
{
    switch (MP2::GetICNObject(ta.object))
    {
    case ICN::MTNSNOW:
    case ICN::MTNSWMP:
    case ICN::MTNLAVA:
    case ICN::MTNDSRT:
    case ICN::MTNMULT:
    case ICN::MTNGRAS:
        return !ObjMnts1::isShadow(ta.index);

    case ICN::MTNCRCK:
    case ICN::MTNDIRT:
        return !ObjMnts2::isShadow(ta.index);

    default:
        break;
    }

    return false;
}


bool Maps::TilesAddon::isRocs(const TilesAddon& ta)
{
    switch (MP2::GetICNObject(ta.object))
    {
        // roc objects
    case ICN::OBJNSNOW:
        if ((ta.index > 21 && ta.index < 25) || (ta.index > 25 && ta.index < 29) ||
            ta.index == 30 || ta.index == 32 || ta.index == 34 || ta.index == 35 ||
            (ta.index > 36 && ta.index < 40))
            return true;
        break;

    case ICN::OBJNSWMP:
        if (ta.index == 201 || ta.index == 205 ||
            (ta.index > 207 && ta.index < 211))
            return true;
        break;

    case ICN::OBJNGRAS:
        if ((ta.index > 32 && ta.index < 36) || ta.index == 37 || ta.index == 38 ||
            ta.index == 40 || ta.index == 41 || ta.index == 43 || ta.index == 45)
            return true;
        break;

    case ICN::OBJNDIRT:
        if (ta.index == 92 || ta.index == 93 || ta.index == 95 || ta.index == 98 ||
            ta.index == 99 || ta.index == 101 || ta.index == 102 || ta.index == 104 || ta.index == 105)
            return true;
        break;

    case ICN::OBJNCRCK:
        if (ta.index == 10 || ta.index == 11 ||
            ta.index == 18 || ta.index == 19 || ta.index == 21 || ta.index == 22 ||
            (ta.index > 23 && ta.index < 28) || (ta.index > 28 && ta.index < 33) ||
            ta.index == 34 || ta.index == 35 || ta.index == 37 || ta.index == 38 ||
            (ta.index > 39 && ta.index < 45) || ta.index == 46 || ta.index == 47 ||
            ta.index == 49 || ta.index == 50 || ta.index == 52 || ta.index == 53 || ta.index == 55)
            return true;
        break;

    case ICN::OBJNWAT2:
        if (ta.index == 0 || ta.index == 2)
            return true;
        break;

    case ICN::OBJNWATR:
        if (ta.index == 182 || ta.index == 183 ||
            (ta.index > 184 && ta.index < 188))
            return true;
        break;

    default:
        break;
    }

    return false;
}


bool Maps::TilesAddon::isForests(const TilesAddon& ta)
{
    switch (MP2::GetICNObject(ta.object))
    {
    case ICN::TREJNGL:
    case ICN::TREEVIL:
    case ICN::TRESNOW:
    case ICN::TREFIR:
    case ICN::TREFALL:
    case ICN::TREDECI:
        return !ObjTree::isShadow(ta.index);

    default:
        break;
    }

    return false;
}

bool Maps::TilesAddon::isStump(const TilesAddon& ta)
{
    switch (MP2::GetICNObject(ta.object))
    {
    case ICN::OBJNSNOW:
        if (ta.index == 41 || ta.index == 42)
            return true;
        break;

    default:
        break;
    }

    return false;
}

bool Maps::TilesAddon::isDeadTrees(const TilesAddon& ta)
{
    switch (MP2::GetICNObject(ta.object))
    {
    case ICN::OBJNMUL2:
        if (ta.index == 16 || ta.index == 18 || ta.index == 19)
            return true;
        break;

    case ICN::OBJNMULT:
        if (ta.index == 0 || ta.index == 2 || ta.index == 4)
            return true;
        break;

    case ICN::OBJNSNOW:
        if ((ta.index > 50 && ta.index < 53) || (ta.index > 54 && ta.index < 59) ||
            (ta.index > 59 && ta.index < 63) || (ta.index > 63 && ta.index < 67) ||
            ta.index == 68 || ta.index == 69 || ta.index == 71 || ta.index == 72 ||
            ta.index == 74 || ta.index == 75 || ta.index == 77)
            return true;
        break;

    case ICN::OBJNSWMP:
        if (ta.index == 161 || ta.index == 162 ||
            (ta.index > 163 && ta.index < 170) ||
            (ta.index > 170 && ta.index < 175) ||
            ta.index == 176 || ta.index == 177)
            return true;
        break;

    default:
        break;
    }

    return false;
}

bool Maps::TilesAddon::isCactus(const TilesAddon& ta)
{
    switch (MP2::GetICNObject(ta.object))
    {
    case ICN::OBJNDSRT:
        if (ta.index == 24 || ta.index == 26 || ta.index == 28 ||
            (ta.index > 29 && ta.index < 33) ||
            ta.index == 34 || ta.index == 36 || ta.index == 37 || ta.index == 39 ||
            ta.index == 40 || ta.index == 42 || ta.index == 43 ||
            ta.index == 45 || ta.index == 48 || ta.index == 49 ||
            ta.index == 51 || ta.index == 53)
            return true;
        break;

    case ICN::OBJNCRCK:
        if (ta.index == 14 || ta.index == 16)
            return true;
        break;

    default:
        break;
    }

    return false;
}

bool Maps::TilesAddon::isTrees(const TilesAddon& ta)
{
    switch (MP2::GetICNObject(ta.object))
    {
        // tree objects
    case ICN::OBJNDSRT:
        if (ta.index == 3 || ta.index == 4 || ta.index == 6 ||
            ta.index == 7 || ta.index == 9 || ta.index == 10 ||
            ta.index == 12 || ta.index == 74 || ta.index == 76)
            return true;
        break;

    case ICN::OBJNDIRT:
        if (ta.index == 115 || ta.index == 118 || ta.index == 120 ||
            ta.index == 123 || ta.index == 125 || ta.index == 127)
            return true;
        break;

    case ICN::OBJNGRAS:
        if (ta.index == 80 || (ta.index > 82 && ta.index < 86) ||
            ta.index == 87 || (ta.index > 88 && ta.index < 92))
            return true;
        break;

    default:
        break;
    }

    return false;
}

void Maps::TilesAddon::UpdateAbandoneMineLeftSprite(TilesAddon& ta, int resource)
{
    if (ICN::OBJNGRAS == MP2::GetICNObject(ta.object) && 6 == ta.index)
    {
        ta.object = 128; // MTNGRAS
        ta.index = 82;
    }
    else if (ICN::OBJNDIRT == MP2::GetICNObject(ta.object) && 8 == ta.index)
    {
        ta.object = 104; // MTNDIRT
        ta.index = 112;
    }
    else if (ICN::EXTRAOVR == MP2::GetICNObject(ta.object) && 5 == ta.index)
    {
        switch (resource)
        {
        case Resource::ORE:
            ta.index = 0;
            break;
        case Resource::SULFUR:
            ta.index = 1;
            break;
        case Resource::CRYSTAL:
            ta.index = 2;
            break;
        case Resource::GEMS:
            ta.index = 3;
            break;
        case Resource::GOLD:
            ta.index = 4;
            break;
        default:
            break;
        }
    }
}

void Maps::TilesAddon::UpdateAbandoneMineRightSprite(TilesAddon& ta)
{
    if (ICN::OBJNDIRT == MP2::GetICNObject(ta.object) && ta.index == 9)
    {
        ta.object = 104;
        ta.index = 113;
    }
    else if (ICN::OBJNGRAS == MP2::GetICNObject(ta.object) && ta.index == 7)
    {
        ta.object = 128;
        ta.index = 83;
    }
}

void Maps::TilesAddon::UpdateFountainSprite(TilesAddon& ta)
{
    if (ICN::OBJNMUL2 == MP2::GetICNObject(ta.object) && 15 == ta.index)
    {
        ta.object = 0x14;
        ta.index = 0;
    }
}

void Maps::TilesAddon::UpdateTreasureChestSprite(TilesAddon& ta)
{
    if (ICN::OBJNRSRC == MP2::GetICNObject(ta.object) && 19 == ta.index)
    {
        ta.object = 0x15;
        ta.index = 0;
    }
}

int Maps::TilesAddon::UpdateStoneLightsSprite(TilesAddon& ta)
{
    if (ICN::OBJNMUL2 == MP2::GetICNObject(ta.object))
        switch (ta.index)
        {
        case 116:
            ta.object = 0x11;
            ta.index = 0;
            return 1;
        case 119:
            ta.object = 0x12;
            ta.index = 0;
            return 2;
        case 122:
            ta.object = 0x13;
            ta.index = 0;
            return 3;
        default:
            break;
        }
    return 0;
}

pair<int, int> Maps::TilesAddon::ColorRaceFromHeroSprite(const TilesAddon& ta)
{
    pair<int, int> res;

    if (7 > ta.index)
        res.first = Color::BLUE;
    else if (14 > ta.index)
        res.first = Color::GREEN;
    else if (21 > ta.index)
        res.first = Color::RED;
    else if (28 > ta.index)
        res.first = Color::YELLOW;
    else if (35 > ta.index)
        res.first = Color::ORANGE;
    else
        res.first = Color::PURPLE;

    //res.second = Race::NONE;

    switch (ta.index % 7)
    {
    case 0:
        res.second = Race::KNGT;
        break;
    case 1:
        res.second = Race::BARB;
        break;
    case 2:
        res.second = Race::SORC;
        break;
    case 3:
        res.second = Race::WRLK;
        break;
    case 4:
        res.second = Race::WZRD;
        break;
    case 5:
        res.second = Race::NECR;
        break;
    case 6:
        res.second = Race::RAND;
        break;
    }

    return res;
}

bool Maps::TilesAddon::ForceLevel1(const TilesAddon& ta)
{
    // broken ship: left roc
    return ICN::OBJNWAT2 == MP2::GetICNObject(ta.object) && ta.index == 11;
}

bool Maps::TilesAddon::ForceLevel2(const TilesAddon& ta)
{
    return false;
}

/* Maps::Addons */
void Maps::Addons::Remove(uint32_t uniq)
{
    Addons clean;
    for (auto& addon : _items)
    {
        if (!addon.isUniq(uniq))
            clean._items.push_back(addon);
    }
    *this = clean;
}

uint32_t PackTileSpriteIndex(uint32_t index, uint32_t shape) /* index max: 0x3FFF, shape value: 0, 1, 2, 3 */
{
    return shape << 14 | (0x3FFF & index);
}

/* Maps::Tiles */
Maps::Tiles::Tiles() : maps_index(0), pack_sprite_index(0), tile_passable(DIRECTION_ALL),
                       mp2_object(0), fog_colors(Color::ALL), quantity1(0), quantity2(0), quantity3(0)
{
}

void Maps::Tiles::Init(s32 index, const MP2::mp2tile_t& mp2)
{
    tile_passable = DIRECTION_ALL;
    quantity1 = mp2.quantity1;
    quantity2 = mp2.quantity2;
    quantity3 = 0;
    fog_colors = Color::ALL;

    SetTile(mp2.tileIndex, mp2.shape);
    SetIndex(index);
    SetObject(mp2.generalObject);

    addons_level1._items.clear();
    addons_level2._items.clear();

    AddonsPushLevel1(mp2);
    AddonsPushLevel2(mp2);
}

void Maps::Tiles::SetIndex(int index)
{
    maps_index = index;
}

int Maps::Tiles::GetQuantity3() const
{
    return quantity3;
}

void Maps::Tiles::SetQuantity3(int mod)
{
    quantity3 = mod;
}

Heroes* Maps::Tiles::GetHeroes() const
{
    return MP2::OBJ_HEROES == mp2_object && GetQuantity3() ? world.GetHeroes(GetQuantity3() - 1) : nullptr;
}

void Maps::Tiles::SetHeroes(Heroes* hero)
{
    if (hero)
    {
        hero->SetMapsObject(mp2_object);
        SetQuantity3(hero->GetID() + 1);
        SetObject(MP2::OBJ_HEROES);
    }
    else
    {
        hero = GetHeroes();

        if (hero)
        {
            SetObject(hero->GetMapsObject());
            hero->SetMapsObject(MP2::OBJ_ZERO);
        }
        else
            SetObject(MP2::OBJ_ZERO);

        SetQuantity3(0);
    }
}

Point Maps::Tiles::GetCenter() const
{
    return GetPoint(GetIndex());
}

s32 Maps::Tiles::GetIndex() const
{
    return maps_index;
}

int Maps::Tiles::GetObject(bool skip_hero /* true */) const
{
    if (!skip_hero && MP2::OBJ_HEROES == mp2_object)
    {
        const Heroes* hero = GetHeroes();
        return hero ? hero->GetMapsObject() : MP2::OBJ_ZERO;
    }

    return mp2_object;
}

void Maps::Tiles::SetObject(int object)
{
    mp2_object = object;
}

void Maps::Tiles::SetTile(uint32_t sprite_index, uint32_t shape)
{
    pack_sprite_index = PackTileSpriteIndex(sprite_index, shape);
}

uint32_t Maps::Tiles::TileSpriteIndex() const
{
    return pack_sprite_index & 0x3FFF;
}

uint32_t Maps::Tiles::TileSpriteShape() const
{
    return pack_sprite_index >> 14;
}

Surface Maps::Tiles::GetTileSurface() const
{
    return AGG::GetTIL(TIL::GROUND32, TileSpriteIndex(), TileSpriteShape());
}

bool isMountsRocs(const Maps::TilesAddon& ta)
{
    return Maps::TilesAddon::isMounts(ta) || Maps::TilesAddon::isRocs(ta);
}

bool isForestsTrees(const Maps::TilesAddon& ta)
{
    return Maps::TilesAddon::isForests(ta) || Maps::TilesAddon::isTrees(ta) ||
        Maps::TilesAddon::isCactus(ta);
}

bool Exclude4LongObject(const Maps::TilesAddon& ta)
{
    return Maps::TilesAddon::isStream(ta) ||
        Maps::TilesAddon::isRoad(ta) || Maps::TilesAddon::isShadow(ta);
}

bool HaveLongObjectUniq(const Maps::Addons& level, uint32_t uid)
{
    for (const auto& it : level._items)
        if (!Exclude4LongObject(it) && it.isUniq(uid)) return true;
    return false;
}

bool TopObjectDisable(const Maps::TilesAddon& ta)
{
    return isMountsRocs(ta) || isForestsTrees(ta);
}

bool Maps::Tiles::isLongObject(int direction)
{
    const Size wSize(world.w(), world.h());
    if (!isValidDirection(GetIndex(), direction, wSize))
    {
        return false;
    }
    Tiles& tile = world.GetTiles(GetDirectionIndex(GetIndex(), direction));

    for (const auto& it : addons_level1._items)
        if (!Exclude4LongObject(it) &&
            (HaveLongObjectUniq(tile.addons_level1, it.uniq) ||
                (!TilesAddon::isTrees(it) && HaveLongObjectUniq(tile.addons_level2, it.uniq))))
            return true;
    return false;
}

void Maps::Tiles::UpdatePassable()
{
    tile_passable = DIRECTION_ALL;

    const int obj = GetObject(false);
    bool emptyobj = MP2::OBJ_ZERO == obj || MP2::OBJ_COAST == obj || MP2::OBJ_EVENT == obj;

    if (MP2::isActionObject(obj, isWater()))
    {
        tile_passable = MP2::GetObjectDirect(obj);
        return;
    }

    Size wSize(world.w(), world.h());
    // on ground
    if (MP2::OBJ_HEROES != mp2_object && !isWater())
    {
        bool mounts1 = addons_level1._items.end() != find_if(addons_level1._items.begin(), addons_level1._items.end(),
                                                             isMountsRocs);
        bool mounts2 = addons_level2._items.end() != find_if(addons_level2._items.begin(), addons_level2._items.end(),
                                                             isMountsRocs);
        bool trees1 = addons_level1._items.end() != find_if(addons_level1._items.begin(), addons_level1._items.end(),
                                                            isForestsTrees);
        bool trees2 = addons_level2._items.end() != find_if(addons_level2._items.begin(), addons_level2._items.end(),
                                                            isForestsTrees);

        // fix coast passable
        if (tile_passable &&
            //! MP2::isActionObject(obj, false) &&
            !emptyobj &&
            TileIsCoast(GetIndex(), Direction::TOP | Direction::BOTTOM | Direction::LEFT | Direction::RIGHT) &&
            addons_level1._items.size() != static_cast<size_t>(count_if(
                addons_level1._items.begin(), addons_level1._items.end(),
                [](const TilesAddon& it)
                {
                    return TilesAddon::isShadow(it);
                })))
        {
            tile_passable = 0;
        }

        // fix mountain layer
        if (tile_passable &&
            (MP2::OBJ_MOUNTS == obj || MP2::OBJ_TREES == obj) &&
            mounts1 && (mounts2 || trees2))
        {
            tile_passable = 0;
        }

        // fix trees layer
        if (tile_passable &&
            (MP2::OBJ_MOUNTS == obj || MP2::OBJ_TREES == obj) &&
            trees1 && (mounts2 || trees2))
        {
            tile_passable = 0;
        }

        // town twba
        if (tile_passable &&
            FindAddonICN1(ICN::OBJNTWBA) && (mounts2 || trees2))
        {
            tile_passable = 0;
        }

        if (isValidDirection(GetIndex(), Direction::TOP, wSize))
        {
            Tiles& top = world.GetTiles(GetDirectionIndex(GetIndex(), Direction::TOP));
            // fix: rocs on water
            if (top.isWater() &&
                top.tile_passable &&
                !(Direction::TOP & top.tile_passable))
            {
                top.tile_passable = 0;
            }
        }
    }

    // fix bottom border: disable passable for all no action objects
    if (tile_passable &&
        !isValidDirection(GetIndex(), Direction::BOTTOM, wSize) &&
        !emptyobj &&
        !MP2::isActionObject(obj, isWater()))
    {
        tile_passable = 0;
    }

    // check all sprite (level 1)
    for (const auto& it : addons_level1._items)
    {
        if (tile_passable)
        {
            tile_passable &= TilesAddon::GetPassable(it);
        }
    }

    // fix top passable
    if (isValidDirection(GetIndex(), Direction::TOP, wSize))
    {
        Tiles& top = world.GetTiles(GetDirectionIndex(GetIndex(), Direction::TOP));

        if (isWater() == top.isWater() &&
            top.addons_level1._items.end() !=
            find_if(top.addons_level1._items.begin(), top.addons_level1._items.end(), TopObjectDisable) &&
            !MP2::isActionObject(top.GetObject(false), isWater()) &&
            (tile_passable && !(tile_passable & DIRECTION_TOP_ROW)) &&
            !(top.tile_passable & DIRECTION_TOP_ROW))
        {
            top.tile_passable = 0;
        }
    }

    // fix corners
    if (isValidDirection(GetIndex(), Direction::LEFT, wSize))
    {
        Tiles& left = world.GetTiles(GetDirectionIndex(GetIndex(), Direction::LEFT));

        // left corner
        if (left.tile_passable &&
            isLongObject(Direction::TOP) &&
            !((Direction::TOP | Direction::TOP_LEFT) & tile_passable) &&
            Direction::TOP_RIGHT & left.tile_passable)
        {
            left.tile_passable &= ~Direction::TOP_RIGHT;
        }
        else
            // right corner
            if (tile_passable &&
                left.isLongObject(Direction::TOP) &&
                !((Direction::TOP | Direction::TOP_RIGHT) & left.tile_passable) &&
                Direction::TOP_LEFT & tile_passable)
            {
                tile_passable &= ~Direction::TOP_LEFT;
            }
    }
}

uint32_t Maps::Tiles::GetObjectUID(int obj) const
{
    const TilesAddon* addon = FindObjectConst(obj);
    return addon ? addon->uniq : 0;
}

int Maps::Tiles::GetPassable() const
{
    return tile_passable;
}

void Maps::Tiles::AddonsPushLevel1(const MP2::mp2tile_t& mt)
{
    if (mt.objectName1 && mt.indexName1 < 0xFF)
        AddonsPushLevel1(TilesAddon(0, mt.uniqNumber1, mt.objectName1, mt.indexName1));
}

void Maps::Tiles::AddonsPushLevel1(const MP2::mp2addon_t& ma)
{
    if (ma.objectNameN1 && ma.indexNameN1 < 0xFF)
        AddonsPushLevel1(TilesAddon(ma.quantityN, ma.uniqNumberN1, ma.objectNameN1, ma.indexNameN1));
}

void Maps::Tiles::AddonsPushLevel1(const TilesAddon& ta)
{
    if (TilesAddon::ForceLevel2(ta))
        addons_level2._items.push_back(ta);
    else
        addons_level1._items.push_back(ta);
}

void Maps::Tiles::AddonsPushLevel2(const MP2::mp2tile_t& mt)
{
    if (mt.objectName2 && mt.indexName2 < 0xFF)
        AddonsPushLevel2(TilesAddon(0, mt.uniqNumber2, mt.objectName2, mt.indexName2));
}

void Maps::Tiles::AddonsPushLevel2(const MP2::mp2addon_t& ma)
{
    if (ma.objectNameN2 && ma.indexNameN2 < 0xFF)
        AddonsPushLevel2(TilesAddon(ma.quantityN, ma.uniqNumberN2, ma.objectNameN2, ma.indexNameN2));
}

void Maps::Tiles::AddonsPushLevel2(const TilesAddon& ta)
{
    if (TilesAddon::ForceLevel1(ta))
        addons_level1._items.push_back(ta);
    else
        addons_level2._items.push_back(ta);
}

void Maps::Tiles::AddonsSort()
{
    if (!addons_level1._items.empty())
        std::sort(addons_level1._items.begin(), addons_level1._items.end(), TilesAddon::PredicateSortRules1);
    if (!addons_level2._items.empty())
        std::sort(addons_level2._items.begin(), addons_level2._items.end(), TilesAddon::PredicateSortRules2);
}

int Maps::Tiles::GetGround() const
{
    const uint32_t index = TileSpriteIndex();

    // list grounds from GROUND32.TIL
    if (30 > index)
        return Ground::WATER;
    if (92 > index)
        return Ground::GRASS;
    if (146 > index)
        return Ground::SNOW;
    if (208 > index)
        return Ground::SWAMP;
    if (262 > index)
        return Ground::LAVA;
    if (321 > index)
        return Ground::DESERT;
    if (361 > index)
        return Ground::DIRT;
    if (415 > index)
        return Ground::WASTELAND;

    //else if(432 > pack_sprite_index)

    return Ground::BEACH;
}

bool Maps::Tiles::isWater() const
{
    return 30 > TileSpriteIndex();
}

void Maps::Tiles::Remove(uint32_t uniq)
{
    if (!addons_level1._items.empty()) addons_level1.Remove(uniq);
    if (!addons_level2._items.empty()) addons_level2.Remove(uniq);
}

void Maps::Tiles::RedrawTile(Surface& dst) const
{
    const Interface::GameArea& area = Interface::Basic::Get().GetGameArea();
    const Point mp = GetPoint(GetIndex());

    if (area.GetRectMaps() & mp)
    {
        const Surface& tileSurface = GetTileSurface();
        area.BlitOnTile(dst, tileSurface, 0, 0, mp);
    }
}

void Maps::Tiles::RedrawBottom(Surface& dst, bool skip_objs) const
{
    const Interface::GameArea& area = Interface::Basic::Get().GetGameArea();
    const Point mp = GetPoint(GetIndex());

    if (!(area.GetRectMaps() & mp) || addons_level1._items.empty())
        return;
    for (const auto& it : addons_level1._items)
    {
        // skip
        if (skip_objs &&
            MP2::isRemoveObject(GetObject()) &&
            FindObjectConst(GetObject()) == &it)
            continue;

        const u8& object = it.object;
        const u8& index = it.index;
        const int icn = MP2::GetICNObject(object);

        if (ICN::UNKNOWN == icn || ICN::MINIHERO == icn || ICN::MONS32 == icn)
            continue;
        const Sprite& sprite = AGG::GetICN(icn, index);
        area.BlitOnTile(dst, sprite, mp);

        // possible anime
        uint32_t anime_index = ICN::AnimationFrame(icn, index, Game::MapsAnimationFrame(), quantity2);
        if (anime_index == 0)
            continue;
        const Sprite& anime_sprite = AGG::GetICN(icn, anime_index);
        area.BlitOnTile(dst, anime_sprite, mp);
    }
}

void Maps::Tiles::RedrawPassable(Surface& dst)
{
}

void Maps::Tiles::RedrawObjects(Surface& dst) const
{
    switch (GetObject())
    {
        // boat
    case MP2::OBJ_BOAT:
        RedrawBoat(dst);
        break;
        // monster
    case MP2::OBJ_MONSTER:
        RedrawMonster(dst);
        break;
        //
    default:
        break;
    }
}

void Maps::Tiles::RedrawMonster(Surface& dst) const
{
    const Settings& conf = Settings::Get();
    const Point mp = GetPoint(GetIndex());
    const Interface::GameArea& area = Interface::Basic::Get().GetGameArea();
    s32 dst_index = -1;

    if (!(area.GetRectMaps() & mp)) return;

    // scan hero around
    MapsIndexes v;
    ScanAroundObject(GetIndex(), MP2::OBJ_HEROES, v);
    for (int it : v)
    {
        const Tiles& tile = world.GetTiles(it);
        dst_index = it;

        if (MP2::OBJ_HEROES != mp2_object ||
            // skip bottom, bottom_right, bottom_left with ground objects
            (DIRECTION_BOTTOM_ROW & Direction::Get(GetIndex(), it) && MP2::isGroundObject(tile.GetObject(false))) ||
            // skip ground check
            tile.isWater() != isWater())
            dst_index = -1;
        else
            break;
    }

    const uint32_t sprite_index = QuantityMonster().GetSpriteIndex();

    // draw attack sprite
    if (-1 != dst_index && !conf.ExtWorldOnlyFirstMonsterAttack())
    {
        bool revert = false;

        switch (Direction::Get(GetIndex(), dst_index))
        {
        case Direction::TOP_LEFT:
        case Direction::LEFT:
        case Direction::BOTTOM_LEFT:
            revert = true;
            break;
        default:
            break;
        }

        const Sprite& sprite_first = AGG::GetICN(ICN::MINIMON, sprite_index * 9 + (revert ? 8 : 7));
        area.BlitOnTile(dst, sprite_first, sprite_first.x() + 16, TILEWIDTH + sprite_first.y(), mp);
    }
    else
    {
        // draw first sprite
        const Sprite& sprite_first = AGG::GetICN(ICN::MINIMON, sprite_index * 9);
        area.BlitOnTile(dst, sprite_first, sprite_first.x() + 16, TILEWIDTH + sprite_first.y(), mp);

        // draw second sprite
        const Sprite& sprite_next = AGG::GetICN(ICN::MINIMON, sprite_index * 9 + 1 +
                                                monster_animation_cicle[
                                                    (Game::MapsAnimationFrame() + mp.x * mp.y) %
                                                    ARRAY_COUNT(monster_animation_cicle)]);
        area.BlitOnTile(dst, sprite_next, sprite_next.x() + 16, TILEWIDTH + sprite_next.y(), mp);
    }
}

void Maps::Tiles::RedrawBoat(Surface& dst) const
{
    const Point mp = GetPoint(GetIndex());
    const Interface::GameArea& area = Interface::Basic::Get().GetGameArea();

    if (area.GetRectMaps() & mp)
    {
        // FIXME: restore direction from Maps::Tiles
        const Sprite& sprite = AGG::GetICN(ICN::BOAT32, 18);
        area.BlitOnTile(dst, sprite, sprite.x(), TILEWIDTH + sprite.y(), mp);
    }
}

bool SkipRedrawTileBottom4Hero(const Maps::TilesAddon& ta, int passable)
{
    if (Maps::TilesAddon::isStream(ta) || Maps::TilesAddon::isRoad(ta))
        return true;
    switch (MP2::GetICNObject(ta.object))
    {
    case ICN::UNKNOWN:
    case ICN::MINIHERO:
    case ICN::MONS32:
        return true;

    case ICN::OBJNWATR:
        return ta.index >= 202 && ta.index <= 225; /* whirlpool */

    case ICN::OBJNTWBA:
    case ICN::ROAD:
    case ICN::STREAM:
        return true;

    case ICN::OBJNCRCK:
        return ta.index == 58 || ta.index == 59 || ta.index == 64 || ta.index == 65 ||
            ta.index == 188 || ta.index == 189 || passable & DIRECTION_TOP_ROW;

    case ICN::OBJNDIRT:
    case ICN::OBJNDSRT:
    case ICN::OBJNGRA2:
    case ICN::OBJNGRAS:
    case ICN::OBJNLAVA:
    case ICN::OBJNSNOW:
    case ICN::OBJNSWMP:
        return passable & DIRECTION_TOP_ROW;

    default:
        break;
    }

    return false;
}

void Maps::Tiles::RedrawBottom4Hero(Surface& dst) const
{
    const Interface::GameArea& area = Interface::Basic::Get().GetGameArea();
    const Point mp = GetPoint(GetIndex());

    if (!(area.GetRectMaps() & mp) || addons_level1._items.empty())
        return;
    for (const auto& it : addons_level1._items)
    {
        if (SkipRedrawTileBottom4Hero(it, tile_passable))
            continue;
        const u8& object = it.object;
        const u8& index = it.index;
        const int icn = MP2::GetICNObject(object);

        const Sprite& sprite = AGG::GetICN(icn, index);
        area.BlitOnTile(dst, sprite, mp);
        uint32_t anime_index = ICN::AnimationFrame(icn, index, Game::MapsAnimationFrame(), quantity2);
        // possible anime
        if (anime_index != 0)
        {
            const Sprite& anime_sprite = AGG::GetICN(icn, anime_index);
            area.BlitOnTile(dst, anime_sprite, mp);
        }
    }
}

void Maps::Tiles::RedrawTop(Surface& dst, const TilesAddon* skip) const
{
    const Interface::GameArea& area = Interface::Basic::Get().GetGameArea();
    const Point mp = GetPoint(GetIndex());

    if (!(area.GetRectMaps() & mp)) return;

    // animate objects
    if (MP2::OBJ_ABANDONEDMINE == GetObject())
    {
        const Sprite& anime_sprite = AGG::GetICN(ICN::OBJNHAUN, Game::MapsAnimationFrame() % 15);
        area.BlitOnTile(dst, anime_sprite, mp);
    }
    else if (MP2::OBJ_MINES == GetObject())
    {
        const TilesAddon* addon = FindObjectConst(MP2::OBJ_MINES);
        if (addon && addon->tmp == Spell::HAUNT)
        {
            const Sprite& anime_sprite = AGG::GetICN(ICN::OBJNHAUN, Game::MapsAnimationFrame() % 15);
            area.BlitOnTile(dst, anime_sprite, mp);
        }
        else if (addon && addon->tmp >= Spell::SETEGUARDIAN && addon->tmp <= Spell::SETWGUARDIAN)
        {
            area.BlitOnTile(dst,
                            AGG::GetICN(ICN::MONS32, Monster(Spell(addon->tmp)).GetSpriteIndex()), TILEWIDTH, 0, mp);
        }
    }

    if (addons_level2._items.empty())
        return;
    for (const auto& it : addons_level2._items)
    {
        if (skip && skip == &it) continue;

        const u8& object = it.object;
        const u8& index = it.index;
        const int icn = MP2::GetICNObject(object);

        if (ICN::UNKNOWN != icn && ICN::MINIHERO != icn && ICN::MONS32 != icn)
        {
            const Sprite& sprite = AGG::GetICN(icn, index);
            area.BlitOnTile(dst, sprite, mp);

            // possible anime
            if (uint32_t anime_index = ICN::AnimationFrame(icn, index, Game::MapsAnimationFrame()))
            {
                const Sprite& anime_sprite = AGG::GetICN(icn, anime_index);
                area.BlitOnTile(dst, anime_sprite, mp);
            }
        }
    }
}

void Maps::Tiles::RedrawTop4Hero(Surface& dst, bool skip_ground) const
{
    const Interface::GameArea& area = Interface::Basic::Get().GetGameArea();
    const Point mp = GetPoint(GetIndex());

    if (!(area.GetRectMaps() & mp) || addons_level2._items.empty())
        return;
    for (const auto& it : addons_level2._items)
    {
        if (skip_ground && MP2::isGroundObject(it.object)) continue;

        const u8& object = it.object;
        const u8& index = it.index;
        const int icn = MP2::GetICNObject(object);

        if (!ICN::HighlyObjectSprite(icn, index))
            continue;
        const Sprite& sprite = AGG::GetICN(icn, index);
        area.BlitOnTile(dst, sprite, mp);

        // possible anime
        if (uint32_t anime_index = ICN::AnimationFrame(icn, index, Game::MapsAnimationFrame()))
        {
            const Sprite& anime_sprite = AGG::GetICN(icn, anime_index);
            area.BlitOnTile(dst, anime_sprite, mp);
        }
    }
}

Maps::TilesAddon* Maps::Tiles::FindAddonICN1(int icn1)
{
    const auto it = find_if(addons_level1._items.begin(), addons_level1._items.end(),
                            [&](const TilesAddon& it) { return it.isICN(icn1); });

    return it != addons_level1._items.end() ? &*it : nullptr;
}

Maps::TilesAddon* Maps::Tiles::FindAddonICN2(int icn2)
{
    auto it = find_if(addons_level2._items.begin(), addons_level2._items.end(),
                      [&](const TilesAddon& it) { return it.isICN(icn2); });

    return it != addons_level2._items.end() ? &*it : nullptr;
}

Maps::TilesAddon* Maps::Tiles::FindAddonLevel1(uint32_t uniq1)
{
    auto it = find_if(addons_level1._items.begin(), addons_level1._items.end(),
                      [&](const TilesAddon& it) { return it.isUniq(uniq1); });

    return it != addons_level1._items.end() ? &*it : nullptr;
}

Maps::TilesAddon* Maps::Tiles::FindAddonLevel2(uint32_t uniq2)
{
    auto it = find_if(addons_level2._items.begin(), addons_level2._items.end(),
                      [&](const TilesAddon& it) { return it.isUniq(uniq2); });

    return it != addons_level2._items.end() ? &*it : nullptr;
}


string Maps::Tiles::String() const
{
    ostringstream os;

    os <<
        "----------------:--------" << endl <<
        "maps index      : " << GetIndex() << ", " << GetString(GetCenter()) << endl <<
        "tile index      : " << TileSpriteIndex() << endl <<
        "ground          : " << Ground::String(GetGround());
    if (isRoad())
    {
        auto it = find_if(addons_level1._items.begin(), addons_level1._items.end(),
                          [](const TilesAddon& it)
                          {
                              return it.isRoad(DIRECTION_ALL);
                          });
        os << ", (" << "road";
        if (ICN::ROAD == MP2::GetICNObject((*it).object))
            os << ", " << "index: " << static_cast<int>((*it).index);
        os << ")";
    }
    os << endl <<
        "passable        : " << (tile_passable ? Direction::String(tile_passable) : "false");
    os <<
        endl <<
        "mp2 object      : " << "0x" << setw(2) << setfill('0') << GetObject() <<
        ", (" << MP2::StringObject(GetObject()) << ")" << endl <<
        "quantity 1      : " << static_cast<int>(quantity1) << endl <<
        "quantity 2      : " << static_cast<int>(quantity2) << endl <<
        "quantity 3      : " << GetQuantity3() << endl;

    for (const auto& it : addons_level1._items)
        os << it.String(1);

    for (const auto& it : addons_level2._items)
        os << it.String(2);

    os <<
        "----------------I--------" << endl;

    // extra obj info
    switch (GetObject())
    {
        // dwelling
    case MP2::OBJ_RUINS:
    case MP2::OBJ_TREECITY:
    case MP2::OBJ_WAGONCAMP:
    case MP2::OBJ_DESERTTENT:
    case MP2::OBJ_TROLLBRIDGE:
    case MP2::OBJ_DRAGONCITY:
    case MP2::OBJ_CITYDEAD:
        //
    case MP2::OBJ_WATCHTOWER:
    case MP2::OBJ_EXCAVATION:
    case MP2::OBJ_CAVE:
    case MP2::OBJ_TREEHOUSE:
    case MP2::OBJ_ARCHERHOUSE:
    case MP2::OBJ_GOBLINHUT:
    case MP2::OBJ_DWARFCOTT:
    case MP2::OBJ_HALFLINGHOLE:
    case MP2::OBJ_PEASANTHUT:
    case MP2::OBJ_THATCHEDHUT:
        //
    case MP2::OBJ_MONSTER:
        os << "count           : " << MonsterCount() << endl;
        break;

    case MP2::OBJ_HEROES:
        {
            const Heroes* hero = GetHeroes();
            if (hero) os << hero->String();
        }
        break;

    case MP2::OBJN_CASTLE:
    case MP2::OBJ_CASTLE:
        {
            const Castle* castle = world.GetCastle(GetCenter());
            if (castle) os << castle->String();
        }
        break;

    default:
        {
            const MapsIndexes& v = GetTilesUnderProtection(GetIndex());
            if (!v.empty())
            {
                os << "protection      : ";
                for (int it : v)
                    os << it << ", ";
                os << endl;
            }
            break;
        }
    }

    if (MP2::isCaptureObject(GetObject(false)))
    {
        const CapturedObject& co = world.GetCapturedObject(GetIndex());

        os <<
            "capture color   : " << Color::String(co.objcol.second) << endl;
        if (co.guardians.IsValid())
        {
            os <<
                "capture guard   : " << co.guardians.GetName() << endl <<
                "capture caunt   : " << co.guardians.GetCount() << endl;
        }
    }

    os << "----------------:--------" << endl;
    return os.str();
}

void Maps::Tiles::FixObject()
{
    if (MP2::OBJ_ZERO != mp2_object)
        return;
    if (addons_level1._items.end() != find_if(addons_level1._items.begin(), addons_level1._items.end(),
                                              TilesAddon::isArtifact))
        SetObject(MP2::OBJ_ARTIFACT);
    else if (addons_level1._items.end() !=
        find_if(addons_level1._items.begin(), addons_level1._items.end(), TilesAddon::isResource))
        SetObject(MP2::OBJ_RESOURCE);
}

bool Maps::Tiles::GoodForUltimateArtifact() const
{
    return !isWater() && (addons_level1._items.empty() ||
            addons_level1._items.size() ==
            static_cast<size_t>(count_if(addons_level1._items.begin(), addons_level1._items.end(),
                                         [&](const TilesAddon& it) { return TilesAddon::isShadow(it); }))) &&
        isPassable(nullptr, Direction::CENTER, true);
}

bool TileIsGround(s32 index, int ground)
{
    return ground == world.GetTiles(index).GetGround();
}

/* accept move */
bool Maps::Tiles::isPassable(const Heroes& hero) const
{
    if (hero.isShipMaster())
    {
        return isWater() && MP2::OBJ_BOAT != GetObject();
    }
    if (!isWater())
        return true;
    switch (GetObject())
    {
        // fix shipwreck: place on water
    case MP2::OBJ_SHIPWRECK:
        // check later
        break;

        // for: meetings/attack hero
    case MP2::OBJ_HEROES:
        {
            // scan ground
            MapsIndexes v;
            GetAroundIndexes(GetIndex(), v);
            if (v.end() == find_if(v.begin(), v.end(),
                                   [](int it)
                                   {
                                       return TileIsGround(it, static_cast<int>(Ground::WATER));
                                   }))
            {
                return false;
            }
        }
        break;

    default:
        // ! hero->isShipMaster() && isWater()
        return false;
    }
    return true;
}

bool Maps::Tiles::isPassable(const Heroes* hero, int direct, bool skipfog) const
{
    if (!skipfog && isFog(Settings::Get().CurrentColor()))
        return false;

    return !(hero && !isPassable(*hero)) && direct & tile_passable;
}

void Maps::Tiles::SetObjectPassable(bool pass)
{
    switch (GetObject(false))
    {
    case MP2::OBJ_TROLLBRIDGE:
        if (pass)
            tile_passable |= Direction::TOP_LEFT;
        else
            tile_passable &= ~Direction::TOP_LEFT;
        break;

    default:
        break;
    }
}

/* check road */
bool Maps::Tiles::isRoad(int direct) const
{
    for (const auto& addon : addons_level1._items)
    {
        if (addon.isRoad(direct))
            return true;
    }
    return false;
}

bool Maps::Tiles::isStream() const
{
    for (const auto& addon : addons_level1._items)
    {
        if (TilesAddon::isStream(addon))
            return true;
    }
    return false;
}

Maps::TilesAddon* Maps::Tiles::FindObject(int objs) const
{
    return const_cast<TilesAddon *>(FindObjectConst(objs));
}

const Maps::TilesAddon* Maps::Tiles::FindObjectConst(int objs) const
{
    auto it = !addons_level1._items.empty() ? addons_level1._items.begin() : addons_level1._items.end();

    switch (objs)
    {
    case MP2::OBJ_CAMPFIRE:
        it = find_if(addons_level1._items.begin(), addons_level1._items.end(), TilesAddon::isCampFire);
        break;

    case MP2::OBJ_TREASURECHEST:
    case MP2::OBJ_ANCIENTLAMP:
    case MP2::OBJ_RESOURCE:
        it = find_if(addons_level1._items.begin(), addons_level1._items.end(), TilesAddon::isResource);
        break;

    case MP2::OBJ_RNDRESOURCE:
        it = find_if(addons_level1._items.begin(), addons_level1._items.end(), TilesAddon::isRandomResource);
        break;

    case MP2::OBJ_FLOTSAM:
    case MP2::OBJ_SHIPWRECKSURVIROR:
    case MP2::OBJ_WATERCHEST:
    case MP2::OBJ_BOTTLE:
        it = find_if(addons_level1._items.begin(), addons_level1._items.end(), TilesAddon::isWaterResource);
        break;

    case MP2::OBJ_ARTIFACT:
        it = find_if(addons_level1._items.begin(), addons_level1._items.end(), TilesAddon::isArtifact);
        break;

    case MP2::OBJ_RNDARTIFACT:
        it = find_if(addons_level1._items.begin(), addons_level1._items.end(), TilesAddon::isRandomArtifact);
        break;

    case MP2::OBJ_RNDARTIFACT1:
        it = find_if(addons_level1._items.begin(), addons_level1._items.end(), TilesAddon::isRandomArtifact1);
        break;

    case MP2::OBJ_RNDARTIFACT2:
        it = find_if(addons_level1._items.begin(), addons_level1._items.end(), TilesAddon::isRandomArtifact2);
        break;

    case MP2::OBJ_RNDARTIFACT3:
        it = find_if(addons_level1._items.begin(), addons_level1._items.end(), TilesAddon::isRandomArtifact3);
        break;

    case MP2::OBJ_RNDULTIMATEARTIFACT:
        it = find_if(addons_level1._items.begin(), addons_level1._items.end(), TilesAddon::isUltimateArtifact);
        break;

    case MP2::OBJ_MONSTER:
        it = find_if(addons_level1._items.begin(), addons_level1._items.end(), TilesAddon::isMonster);
        break;

    case MP2::OBJ_WHIRLPOOL:
        it = find_if(addons_level1._items.begin(), addons_level1._items.end(), TilesAddon::isWhirlPool);
        break;

    case MP2::OBJ_STANDINGSTONES:
        it = find_if(addons_level1._items.begin(), addons_level1._items.end(), TilesAddon::isStandingStone);
        break;

    case MP2::OBJ_ARTESIANSPRING:
        it = find_if(addons_level1._items.begin(), addons_level1._items.end(), TilesAddon::isArtesianSpring);
        break;

    case MP2::OBJ_OASIS:
        it = find_if(addons_level1._items.begin(), addons_level1._items.end(), TilesAddon::isOasis);
        break;

    case MP2::OBJ_WATERINGHOLE:
        it = find_if(addons_level1._items.begin(), addons_level1._items.end(), TilesAddon::isWateringHole);
        break;

    case MP2::OBJ_MINES:
        it = find_if(addons_level1._items.begin(), addons_level1._items.end(), TilesAddon::isMine);
        break;

    case MP2::OBJ_JAIL:
        it = find_if(addons_level1._items.begin(), addons_level1._items.end(), TilesAddon::isJail);
        break;

    case MP2::OBJ_EVENT:
        it = find_if(addons_level1._items.begin(), addons_level1._items.end(), TilesAddon::isEvent);
        break;

    case MP2::OBJ_BOAT:
        it = find_if(addons_level1._items.begin(), addons_level1._items.end(), TilesAddon::isBoat);
        break;

    case MP2::OBJ_BARRIER:
        it = find_if(addons_level1._items.begin(), addons_level1._items.end(), TilesAddon::isBarrier);
        break;

    case MP2::OBJ_HEROES:
        it = find_if(addons_level1._items.begin(), addons_level1._items.end(), TilesAddon::isMiniHero);
        break;

    case MP2::OBJ_CASTLE:
        it = find_if(addons_level1._items.begin(), addons_level1._items.end(), TilesAddon::isCastle);
        if (it == addons_level1._items.end())
        {
            it = find_if(addons_level2._items.begin(), addons_level2._items.end(), TilesAddon::isCastle);
            return addons_level2._items.end() != it ? &*it : nullptr;
        }
        break;

    case MP2::OBJ_RNDCASTLE:
        it = find_if(addons_level1._items.begin(), addons_level1._items.end(), TilesAddon::isRandomCastle);
        if (it == addons_level1._items.end())
        {
            it = find_if(addons_level2._items.begin(), addons_level2._items.end(), TilesAddon::isRandomCastle);
            return addons_level2._items.end() != it ? &*it : nullptr;
        }
        break;

    case MP2::OBJ_RNDMONSTER:
    case MP2::OBJ_RNDMONSTER1:
    case MP2::OBJ_RNDMONSTER2:
    case MP2::OBJ_RNDMONSTER3:
    case MP2::OBJ_RNDMONSTER4:
        it = find_if(addons_level1._items.begin(), addons_level1._items.end(), TilesAddon::isRandomMonster);
        break;

    case MP2::OBJ_SKELETON:
        it = find_if(addons_level1._items.begin(), addons_level1._items.end(), TilesAddon::isSkeleton);
        break;

    default:
        //FIXME for: " << MP2::StringObject(objs));
        break;
    }

    return addons_level1._items.end() != it ? &*it : nullptr;
}

Maps::TilesAddon* Maps::Tiles::FindFlags()
{
    auto it = find_if(addons_level1._items.begin(), addons_level1._items.end(), TilesAddon::isFlag32);

    if (it == addons_level1._items.end())
    {
        it = find_if(addons_level2._items.begin(), addons_level2._items.end(), TilesAddon::isFlag32);
        return addons_level2._items.end() != it ? &*it : nullptr;
    }

    return addons_level1._items.end() != it ? &*it : nullptr;
}

/* ICN::FLAGS32 version */
void Maps::Tiles::CaptureFlags32(int obj, int col)
{
    uint32_t index = 0;

    switch (col)
    {
    case Color::BLUE:
        index = 0;
        break;
    case Color::GREEN:
        index = 1;
        break;
    case Color::RED:
        index = 2;
        break;
    case Color::YELLOW:
        index = 3;
        break;
    case Color::ORANGE:
        index = 4;
        break;
    case Color::PURPLE:
        index = 5;
        break;
    default:
        index = 6;
        break;
    }

    switch (obj)
    {
    case MP2::OBJ_WINDMILL:
        index += 42;
        CorrectFlags32(index, false);
        break;
    case MP2::OBJ_WATERWHEEL:
        index += 14;
        CorrectFlags32(index, false);
        break;
    case MP2::OBJ_MAGICGARDEN:
        index += 42;
        CorrectFlags32(index, false);
        break;

    case MP2::OBJ_MINES:
        index += 14;
        CorrectFlags32(index, true);
        break;
        //case MP2::OBJ_DRAGONCITY:	index += 35; CorrectFlags32(index); break; unused
    case MP2::OBJ_LIGHTHOUSE:
        index += 42;
        CorrectFlags32(index, false);
        break;

    case MP2::OBJ_ALCHEMYLAB:
        {
            Size wSize(world.w(), world.h());
            index += 21;
            if (isValidDirection(GetIndex(), Direction::TOP, wSize))
            {
                Tiles& tile = world.GetTiles(GetDirectionIndex(GetIndex(), Direction::TOP));
                tile.CorrectFlags32(index, true);
            }
        }
        break;

    case MP2::OBJ_SAWMILL:
        {
            index += 28;
            Size wSize(world.w(), world.h());
            if (isValidDirection(GetIndex(), Direction::TOP_RIGHT, wSize))
            {
                Tiles& tile = world.GetTiles(GetDirectionIndex(GetIndex(), Direction::TOP_RIGHT));
                tile.CorrectFlags32(index, true);
            }
        }
        break;

    case MP2::OBJ_CASTLE:
        {
            index *= 2;
            Size wSize(world.w(), world.h());
            if (isValidDirection(GetIndex(), Direction::LEFT, wSize))
            {
                Tiles& tile = world.GetTiles(GetDirectionIndex(GetIndex(), Direction::LEFT));
                tile.CorrectFlags32(index, true);
            }

            index += 1;
            if (isValidDirection(GetIndex(), Direction::RIGHT, wSize))
            {
                Tiles& tile = world.GetTiles(GetDirectionIndex(GetIndex(), Direction::RIGHT));
                tile.CorrectFlags32(index, true);
            }
        }
        break;

    default:
        break;
    }
}

/* correct flags, ICN::FLAGS32 vesion */
void Maps::Tiles::CorrectFlags32(uint32_t index, bool up)
{
    TilesAddon* taddon = FindFlags();

    // replace flag
    if (taddon)
        taddon->index = index;
    else if (up)
        // or new flag
        addons_level2._items.emplace_back(TilesAddon::UPPER, world.GetUniq(), 0x38, index);
    else
        // or new flag
        addons_level1._items.emplace_back(TilesAddon::UPPER, world.GetUniq(), 0x38, index);
}

void Maps::Tiles::FixedPreload(Tiles& tile)
{
    // fix skeleton: left position
    auto it = find_if(tile.addons_level1._items.begin(), tile.addons_level1._items.end(),
                      TilesAddon::isSkeletonFix);

    if (it != tile.addons_level1._items.end())
    {
        tile.SetObject(MP2::OBJN_SKELETON);
    }

    // fix price loyalty objects.
    if (!Settings::Get().PriceLoyaltyVersion())
        return;
    switch (tile.GetObject())
    {
    case MP2::OBJ_UNKNW_79:
    case MP2::OBJ_UNKNW_7A:
    case MP2::OBJ_UNKNW_F9:
    case MP2::OBJ_UNKNW_FA:
        {
            int newobj = MP2::OBJ_ZERO;
            it = find_if(tile.addons_level1._items.begin(), tile.addons_level1._items.end(),
                         TilesAddon::isX_LOC123);
            if (it != tile.addons_level1._items.end())
            {
                newobj = TilesAddon::GetLoyaltyObject(*it);
            }
            else
            {
                it = find_if(tile.addons_level2._items.begin(), tile.addons_level2._items.end(),
                             TilesAddon::isX_LOC123);
                if (it != tile.addons_level2._items.end())
                    newobj = TilesAddon::GetLoyaltyObject(*it);
            }

            if (MP2::OBJ_ZERO != newobj)
                tile.SetObject(newobj);
        }
        break;

    default:
        break;
    }
}

/* true: if protection or has guardians */
bool Maps::Tiles::CaptureObjectIsProtection() const
{
    const int object = GetObject(false);

    if (!MP2::isCaptureObject(object))
    {
        return false;
    }
    if (MP2::OBJ_CASTLE != object)
        return QuantityTroop().IsValid();
    Castle* castle = world.GetCastle(GetCenter());
    if (castle)
        return castle->GetArmy().m_troops.IsValid();

    return false;
}

void Maps::Tiles::RemoveObjectSprite()
{
    TilesAddon* addon = nullptr;

    switch (GetObject())
    {
    case MP2::OBJ_WATERCHEST:
    case MP2::OBJ_BOTTLE:
    case MP2::OBJ_FLOTSAM:
    case MP2::OBJ_SHIPWRECKSURVIROR:
    case MP2::OBJ_TREASURECHEST:
    case MP2::OBJ_ANCIENTLAMP:
    case MP2::OBJ_RESOURCE:
    case MP2::OBJ_ARTIFACT:
    case MP2::OBJ_CAMPFIRE:
        addon = FindObject(GetObject());
        break;

    case MP2::OBJ_JAIL:
        RemoveJailSprite();
        tile_passable = DIRECTION_ALL;
        break;
    case MP2::OBJ_BARRIER:
        RemoveBarrierSprite();
        tile_passable = DIRECTION_ALL;
        break;

    default:
        break;
    }

    if (addon)
    {
        Size wSize(world.w(), world.h());
        // remove shadow sprite from left cell
        if (isValidDirection(GetIndex(), Direction::LEFT, wSize))
            world.GetTiles(GetDirectionIndex(GetIndex(), Direction::LEFT)).Remove(addon->uniq);

        Remove(addon->uniq);
    }
}

void Maps::Tiles::RemoveBarrierSprite()
{
    TilesAddon* addon = FindObject(MP2::OBJ_BARRIER);

    if (!addon)
        return;
    Size wSize(world.w(), world.h());
    // remove left sprite
    if (isValidDirection(GetIndex(), Direction::LEFT, wSize))
    {
        const s32 left = GetDirectionIndex(GetIndex(), Direction::LEFT);
        world.GetTiles(left).Remove(addon->uniq);
    }

    Remove(addon->uniq);
}

void Maps::Tiles::RemoveJailSprite()
{
    TilesAddon* addon = FindObject(MP2::OBJ_JAIL);

    if (!addon)
        return;
    Size wSize(world.w(), world.h());
    // remove left sprite
    if (isValidDirection(GetIndex(), Direction::LEFT, wSize))
    {
        const s32 left = GetDirectionIndex(GetIndex(), Direction::LEFT);
        world.GetTiles(left).Remove(addon->uniq);

        // remove left left sprite
        if (isValidDirection(left, Direction::LEFT, wSize))
            world.GetTiles(GetDirectionIndex(left, Direction::LEFT)).Remove(addon->uniq);
    }

    // remove top sprite
    if (isValidDirection(GetIndex(), Direction::TOP, wSize))
    {
        const s32 top = GetDirectionIndex(GetIndex(), Direction::TOP);
        world.GetTiles(top).Remove(addon->uniq);
        world.GetTiles(top).SetObject(MP2::OBJ_ZERO);
        world.GetTiles(top).FixObject();

        // remove top left sprite
        if (isValidDirection(top, Direction::LEFT, wSize))
        {
            world.GetTiles(GetDirectionIndex(top, Direction::LEFT)).Remove(addon->uniq);
            world.GetTiles(GetDirectionIndex(top, Direction::LEFT)).SetObject(MP2::OBJ_ZERO);
            world.GetTiles(GetDirectionIndex(top, Direction::LEFT)).FixObject();
        }
    }

    Remove(addon->uniq);
}

void Maps::Tiles::UpdateAbandoneMineSprite(Tiles& tile)
{
    const auto it = find_if(tile.addons_level1._items.begin(), tile.addons_level1._items.end(),
                            TilesAddon::isAbandoneMineSprite);
    const uint32_t uniq = it != tile.addons_level1._items.end() ? (*it).uniq : 0;

    Size wSize(world.w(), world.h());
    if (uniq)
    {
        const int type = tile.QuantityResourceCount().first;

        for (auto& addonIt : tile.addons_level1._items)
            TilesAddon::UpdateAbandoneMineLeftSprite(addonIt, type);

        if (isValidDirection(tile.GetIndex(), Direction::RIGHT, wSize))
        {
            Tiles& tile2 = world.GetTiles(GetDirectionIndex(tile.GetIndex(), Direction::RIGHT));
            TilesAddon* mines = tile2.FindAddonLevel1(uniq);

            if (mines) TilesAddon::UpdateAbandoneMineRightSprite(*mines);
            if (tile2.GetObject() == MP2::OBJN_ABANDONEDMINE) tile2.SetObject(MP2::OBJN_MINES);
        }
    }

    if (isValidDirection(tile.GetIndex(), Direction::LEFT, wSize))
    {
        Tiles& tile2 = world.GetTiles(GetDirectionIndex(tile.GetIndex(), Direction::LEFT));
        if (tile2.GetObject() == MP2::OBJN_ABANDONEDMINE) tile2.SetObject(MP2::OBJN_MINES);
    }

    if (!isValidDirection(tile.GetIndex(), Direction::TOP, wSize))
        return;
    Tiles& tile2 = world.GetTiles(GetDirectionIndex(tile.GetIndex(), Direction::TOP));
    if (tile2.GetObject() == MP2::OBJN_ABANDONEDMINE) tile2.SetObject(MP2::OBJN_MINES);

    if (isValidDirection(tile2.GetIndex(), Direction::LEFT, wSize))
    {
        Tiles& tile3 = world.GetTiles(GetDirectionIndex(tile2.GetIndex(), Direction::LEFT));
        if (tile3.GetObject() == MP2::OBJN_ABANDONEDMINE) tile3.SetObject(MP2::OBJN_MINES);
    }

    if (isValidDirection(tile2.GetIndex(), Direction::RIGHT, wSize))
    {
        Tiles& tile3 = world.GetTiles(GetDirectionIndex(tile2.GetIndex(), Direction::RIGHT));
        if (tile3.GetObject() == MP2::OBJN_ABANDONEDMINE) tile3.SetObject(MP2::OBJN_MINES);
    }
}

void Maps::Tiles::UpdateRNDArtifactSprite(Tiles& tile)
{
    TilesAddon* addon = nullptr;
    uint32_t index = 0;
    Artifact art;

    switch (tile.GetObject())
    {
    case MP2::OBJ_RNDARTIFACT:
        addon = tile.FindObject(MP2::OBJ_RNDARTIFACT);
        art = Artifact::Rand(Artifact::ART_LEVEL123);
        index = art.IndexSprite();
        break;
    case MP2::OBJ_RNDARTIFACT1:
        addon = tile.FindObject(MP2::OBJ_RNDARTIFACT1);
        art = Artifact::Rand(Artifact::ART_LEVEL1);
        index = art.IndexSprite();
        break;
    case MP2::OBJ_RNDARTIFACT2:
        addon = tile.FindObject(MP2::OBJ_RNDARTIFACT2);
        art = Artifact::Rand(Artifact::ART_LEVEL2);
        index = art.IndexSprite();
        break;
    case MP2::OBJ_RNDARTIFACT3:
        addon = tile.FindObject(MP2::OBJ_RNDARTIFACT3);
        art = Artifact::Rand(Artifact::ART_LEVEL3);
        index = art.IndexSprite();
        break;
    default:
        return;
    }

    if (!art.IsValid())
    {
        return;
    }
    if (!addon)
        return;
    addon->index = index;
    tile.SetObject(MP2::OBJ_ARTIFACT);

    Size wSize(world.w(), world.h());
    // replace shadow artifact
    if (!isValidDirection(tile.GetIndex(), Direction::LEFT, wSize))
        return;
    Tiles& left_tile = world.GetTiles(GetDirectionIndex(tile.GetIndex(), Direction::LEFT));
    TilesAddon* shadow = left_tile.FindAddonLevel1(addon->uniq);

    if (shadow) shadow->index = index - 1;
}

void Maps::Tiles::UpdateRNDResourceSprite(Tiles& tile)
{
    TilesAddon* addon = tile.FindObject(MP2::OBJ_RNDRESOURCE);

    if (!addon) return;
    addon->index = Resource::GetIndexSprite(Resource::Rand());
    tile.SetObject(MP2::OBJ_RESOURCE);

    Size wSize(world.w(), world.h());
    // replace shadow artifact
    if (!isValidDirection(tile.GetIndex(), Direction::LEFT, wSize))
        return;
    Tiles& left_tile = world.GetTiles(GetDirectionIndex(tile.GetIndex(), Direction::LEFT));
    TilesAddon* shadow = left_tile.FindAddonLevel1(addon->uniq);

    if (shadow) shadow->index = addon->index - 1;
}

void Maps::Tiles::UpdateStoneLightsSprite(Tiles& tile)
{
    for (auto it = tile.addons_level1._items.begin(); it != tile.addons_level1._items.end(); ++it)
        tile.QuantitySetTeleportType(TilesAddon::UpdateStoneLightsSprite(*it));
}

void Maps::Tiles::UpdateFountainSprite(Tiles& tile)
{
    for (auto& it : tile.addons_level1._items)
        TilesAddon::UpdateFountainSprite(it);
}

void Maps::Tiles::UpdateTreasureChestSprite(Tiles& tile)
{
    for (auto& it : tile.addons_level1._items)
        TilesAddon::UpdateTreasureChestSprite(it);
}

bool Maps::Tiles::isFog(int colors) const
{
    // colors may be the union friends
    return (fog_colors & colors) == colors;
}

void Maps::Tiles::ClearFog(int colors)
{
    fog_colors &= ~colors;
}

void Maps::Tiles::RedrawFogs(Surface& dst, int color) const
{
    const Interface::GameArea& area = Interface::Basic::Get().GetGameArea();
    const Point mp = GetPoint(GetIndex());

    // get direction around foga
    int around = 0;
    const Directions& directions = Direction::All();

    const Size wSize(world.w(), world.h());
    for (int direction : directions)
        if (!isValidDirection(GetIndex(), direction, wSize) ||
            world.GetTiles(GetDirectionIndex(GetIndex(), direction)).isFog(color))
            around |= direction;

    if (isFog(color)) around |= Direction::CENTER;

    // TIL::CLOF32
    if (DIRECTION_ALL == around)
    {
        const Surface& sf = AGG::GetTIL(TIL::CLOF32, GetIndex() % 4, 0);
        area.BlitOnTile(dst, sf, 0, 0, mp);
        return;
    }
    uint32_t index = 0;
    bool revert = false;

    // see ICN::CLOP32: sprite 10
    if (around & Direction::CENTER &&
        !(around & (Direction::TOP | Direction::BOTTOM | Direction::LEFT | Direction::RIGHT)))
    {
        index = 10;
        revert = false;
    }
    else
        // see ICN::CLOP32: sprite 6, 7, 8
        if (around & (Direction::CENTER | Direction::TOP) &&
            !(around & (Direction::BOTTOM | Direction::LEFT | Direction::RIGHT)))
        {
            index = 6;
            revert = false;
        }
        else if (around & (Direction::CENTER | Direction::RIGHT) &&
            !(around & (Direction::TOP | Direction::BOTTOM | Direction::LEFT)))
        {
            index = 7;
            revert = false;
        }
        else if (around & (Direction::CENTER | Direction::LEFT) &&
            !(around & (Direction::TOP | Direction::BOTTOM | Direction::RIGHT)))
        {
            index = 7;
            revert = true;
        }
        else if (around & (Direction::CENTER | Direction::BOTTOM) &&
            !(around & (Direction::TOP | Direction::LEFT | Direction::RIGHT)))
        {
            index = 8;
            revert = false;
        }
        else
            // see ICN::CLOP32: sprite 9, 29
            if (around & (DIRECTION_CENTER_COL) && !(around & (Direction::LEFT | Direction::RIGHT)))
            {
                index = 9;
                revert = false;
            }
            else if (around & (DIRECTION_CENTER_ROW) && !(around & (Direction::TOP | Direction::BOTTOM)))
            {
                index = 29;
                revert = false;
            }
            else
                // see ICN::CLOP32: sprite 15, 22
                if (around == (DIRECTION_ALL & ~Direction::TOP_RIGHT))
                {
                    index = 15;
                    revert = false;
                }
                else if (around == (DIRECTION_ALL & ~Direction::TOP_LEFT))
                {
                    index = 15;
                    revert = true;
                }
                else if (around == (DIRECTION_ALL & ~Direction::BOTTOM_RIGHT))
                {
                    index = 22;
                    revert = false;
                }
                else if (around == (DIRECTION_ALL & ~Direction::BOTTOM_LEFT))
                {
                    index = 22;
                    revert = true;
                }
                else
                    // see ICN::CLOP32: sprite 16, 17, 18, 23
                    if (around == (DIRECTION_ALL & ~(Direction::TOP_RIGHT | Direction::BOTTOM_RIGHT)))
                    {
                        index = 16;
                        revert = false;
                    }
                    else if (around == (DIRECTION_ALL & ~(Direction::TOP_LEFT | Direction::BOTTOM_LEFT)))
                    {
                        index = 16;
                        revert = true;
                    }
                    else if (around == (DIRECTION_ALL & ~(Direction::TOP_RIGHT | Direction::BOTTOM_LEFT)))
                    {
                        index = 17;
                        revert = false;
                    }
                    else if (around == (DIRECTION_ALL & ~(Direction::TOP_LEFT | Direction::BOTTOM_RIGHT)))
                    {
                        index = 17;
                        revert = true;
                    }
                    else if (around == (DIRECTION_ALL & ~(Direction::TOP_LEFT | Direction::TOP_RIGHT)))
                    {
                        index = 18;
                        revert = false;
                    }
                    else if (around == (DIRECTION_ALL & ~(Direction::BOTTOM_LEFT | Direction::BOTTOM_RIGHT)))
                    {
                        index = 23;
                        revert = false;
                    }
                    else
                        // see ICN::CLOP32: sprite 13, 14
                        if (around == (DIRECTION_ALL & ~DIRECTION_TOP_RIGHT_CORNER))
                        {
                            index = 13;
                            revert = false;
                        }
                        else if (around == (DIRECTION_ALL & ~DIRECTION_TOP_LEFT_CORNER))
                        {
                            index = 13;
                            revert = true;
                        }
                        else if (around == (DIRECTION_ALL & ~DIRECTION_BOTTOM_RIGHT_CORNER))
                        {
                            index = 14;
                            revert = false;
                        }
                        else if (around == (DIRECTION_ALL & ~DIRECTION_BOTTOM_LEFT_CORNER))
                        {
                            index = 14;
                            revert = true;
                        }
                        else
                            // see ICN::CLOP32: sprite 11, 12
                            if (around & (Direction::CENTER | Direction::LEFT | Direction::BOTTOM_LEFT | Direction::
                                    BOTTOM) &&
                                !(around & (Direction::TOP | Direction::TOP_RIGHT | Direction::RIGHT)))
                            {
                                index = 11;
                                revert = false;
                            }
                            else if (around & (Direction::CENTER | Direction::RIGHT | Direction::BOTTOM_RIGHT |
                                    Direction::BOTTOM) &&
                                !(around & (Direction::TOP | Direction::TOP_LEFT | Direction::LEFT)))
                            {
                                index = 11;
                                revert = true;
                            }
                            else if (around & (Direction::CENTER | Direction::LEFT | Direction::TOP_LEFT | Direction::
                                    TOP) &&
                                !(around & (Direction::BOTTOM | Direction::BOTTOM_RIGHT | Direction::RIGHT)))
                            {
                                index = 12;
                                revert = false;
                            }
                            else if (around & (Direction::CENTER | Direction::RIGHT | Direction::TOP_RIGHT | Direction::
                                    TOP) &&
                                !(around & (Direction::BOTTOM | Direction::BOTTOM_LEFT | Direction::LEFT)))
                            {
                                index = 12;
                                revert = true;
                            }
                            else
                                // see ICN::CLOP32: sprite 19, 20, 22
                                if (around & (DIRECTION_CENTER_ROW | Direction::BOTTOM | Direction::TOP | Direction::
                                        TOP_LEFT) &&
                                    !(around & (Direction::BOTTOM_LEFT | Direction::BOTTOM_RIGHT | Direction::TOP_RIGHT)
                                    ))
                                {
                                    index = 19;
                                    revert = false;
                                }
                                else if (around & (DIRECTION_CENTER_ROW | Direction::BOTTOM | Direction::TOP | Direction
                                        ::TOP_RIGHT) &&
                                    !(around & (Direction::BOTTOM_LEFT | Direction::BOTTOM_RIGHT | Direction::TOP_LEFT))
                                )
                                {
                                    index = 19;
                                    revert = true;
                                }
                                else if (around & (DIRECTION_CENTER_ROW | Direction::BOTTOM | Direction::TOP | Direction
                                        ::BOTTOM_LEFT) &&
                                    !(around & (Direction::TOP_RIGHT | Direction::BOTTOM_RIGHT | Direction::TOP_LEFT)))
                                {
                                    index = 20;
                                    revert = false;
                                }
                                else if (around & (DIRECTION_CENTER_ROW | Direction::BOTTOM | Direction::TOP | Direction
                                        ::BOTTOM_RIGHT) &&
                                    !(around & (Direction::TOP_RIGHT | Direction::BOTTOM_LEFT | Direction::TOP_LEFT)))
                                {
                                    index = 20;
                                    revert = true;
                                }
                                else if (around & (DIRECTION_CENTER_ROW | Direction::BOTTOM | Direction::TOP) &&
                                    !(around & (Direction::TOP_RIGHT | Direction::BOTTOM_RIGHT | Direction::BOTTOM_LEFT
                                        |
                                        Direction::TOP_LEFT)))
                                {
                                    index = 22;
                                    revert = false;
                                }
                                else
                                    // see ICN::CLOP32: sprite 24, 25, 26, 30
                                    if (around & (DIRECTION_CENTER_ROW | Direction::BOTTOM | Direction::BOTTOM_LEFT) &&
                                        !(around & (Direction::TOP | Direction::BOTTOM_RIGHT)))
                                    {
                                        index = 24;
                                        revert = false;
                                    }
                                    else if (around & (DIRECTION_CENTER_ROW | Direction::BOTTOM | Direction::
                                            BOTTOM_RIGHT) &&
                                        !(around & (Direction::TOP | Direction::BOTTOM_LEFT)))
                                    {
                                        index = 24;
                                        revert = true;
                                    }
                                    else if (around & (DIRECTION_CENTER_COL | Direction::LEFT | Direction::TOP_LEFT) &&
                                        !(around & (Direction::RIGHT | Direction::BOTTOM_LEFT)))
                                    {
                                        index = 25;
                                        revert = false;
                                    }
                                    else if (around & (DIRECTION_CENTER_COL | Direction::RIGHT | Direction::TOP_RIGHT)
                                        &&
                                        !(around & (Direction::LEFT | Direction::BOTTOM_RIGHT)))
                                    {
                                        index = 25;
                                        revert = true;
                                    }
                                    else if (around & (DIRECTION_CENTER_COL | Direction::BOTTOM_LEFT | Direction::LEFT)
                                        &&
                                        !(around & (Direction::RIGHT | Direction::TOP_LEFT)))
                                    {
                                        index = 26;
                                        revert = false;
                                    }
                                    else if (around & (DIRECTION_CENTER_COL | Direction::BOTTOM_RIGHT | Direction::RIGHT
                                        ) &&
                                        !(around & (Direction::LEFT | Direction::TOP_RIGHT)))
                                    {
                                        index = 26;
                                        revert = true;
                                    }
                                    else if (around & (DIRECTION_CENTER_ROW | Direction::TOP_LEFT | Direction::TOP) &&
                                        !(around & (Direction::BOTTOM | Direction::TOP_RIGHT)))
                                    {
                                        index = 30;
                                        revert = false;
                                    }
                                    else if (around & (DIRECTION_CENTER_ROW | Direction::TOP_RIGHT | Direction::TOP) &&
                                        !(around & (Direction::BOTTOM | Direction::TOP_LEFT)))
                                    {
                                        index = 30;
                                        revert = true;
                                    }
                                    else
                                        // see ICN::CLOP32: sprite 27, 28
                                        if (around & (Direction::CENTER | Direction::BOTTOM | Direction::LEFT) &&
                                            !(around & (Direction::TOP | Direction::TOP_RIGHT | Direction::RIGHT |
                                                Direction::BOTTOM_LEFT)))
                                        {
                                            index = 27;
                                            revert = false;
                                        }
                                        else if (around & (Direction::CENTER | Direction::BOTTOM | Direction::RIGHT) &&
                                            !(around & (Direction::TOP | Direction::TOP_LEFT | Direction::LEFT |
                                                Direction::BOTTOM_RIGHT)))
                                        {
                                            index = 27;
                                            revert = true;
                                        }
                                        else if (around & (Direction::CENTER | Direction::LEFT | Direction::TOP) &&
                                            !(around & (Direction::TOP_LEFT | Direction::RIGHT | Direction::BOTTOM |
                                                Direction::BOTTOM_RIGHT)))
                                        {
                                            index = 28;
                                            revert = false;
                                        }
                                        else if (around & (Direction::CENTER | Direction::RIGHT | Direction::TOP) &&
                                            !(around & (Direction::TOP_RIGHT | Direction::LEFT | Direction::BOTTOM |
                                                Direction::BOTTOM_LEFT)))
                                        {
                                            index = 28;
                                            revert = true;
                                        }
                                        else
                                            // see ICN::CLOP32: sprite 31, 32, 33
                                            if (around & (DIRECTION_CENTER_ROW | Direction::TOP) &&
                                                !(around & (Direction::BOTTOM | Direction::TOP_LEFT | Direction::
                                                    TOP_RIGHT)))
                                            {
                                                index = 31;
                                                revert = false;
                                            }
                                            else if (around & (DIRECTION_CENTER_COL | Direction::RIGHT) &&
                                                !(around & (Direction::LEFT | Direction::TOP_RIGHT | Direction::
                                                    BOTTOM_RIGHT)))
                                            {
                                                index = 32;
                                                revert = false;
                                            }
                                            else if (around & (DIRECTION_CENTER_COL | Direction::LEFT) &&
                                                !(around & (Direction::RIGHT | Direction::TOP_LEFT | Direction::
                                                    BOTTOM_LEFT)))
                                            {
                                                index = 32;
                                                revert = true;
                                            }
                                            else if (around & (DIRECTION_CENTER_ROW | Direction::BOTTOM) &&
                                                !(around & (Direction::TOP | Direction::BOTTOM_LEFT | Direction::
                                                    BOTTOM_RIGHT)))
                                            {
                                                index = 33;
                                                revert = false;
                                            }
                                            else
                                                // see ICN::CLOP32: sprite 0, 1, 2, 3, 4, 5
                                                if (around & (DIRECTION_CENTER_ROW | DIRECTION_BOTTOM_ROW) &&
                                                    !(around & Direction::TOP))
                                                {
                                                    index = GetIndex() % 2 ? 0 : 1;
                                                    revert = false;
                                                }
                                                else if (around & (DIRECTION_CENTER_ROW | DIRECTION_TOP_ROW) &&
                                                    !(around & Direction::BOTTOM))
                                                {
                                                    index = GetIndex() % 2 ? 4 : 5;
                                                    revert = false;
                                                }
                                                else if (around & (DIRECTION_CENTER_COL | DIRECTION_LEFT_COL) &&
                                                    !(around & Direction::RIGHT))
                                                {
                                                    index = GetIndex() % 2 ? 2 : 3;
                                                    revert = false;
                                                }
                                                else if (around & (DIRECTION_CENTER_COL | DIRECTION_RIGHT_COL) &&
                                                    !(around & Direction::LEFT))
                                                {
                                                    index = GetIndex() % 2 ? 2 : 3;
                                                    revert = true;
                                                }
                                                    // unknown
                                                else
                                                {
                                                    const Surface& sf = AGG::GetTIL(TIL::CLOF32, GetIndex() % 4, 0);
                                                    area.BlitOnTile(dst, sf, 0, 0, mp);
                                                    return;
                                                }

    const Sprite& sprite = AGG::GetICN(ICN::CLOP32, index, revert);
    area.BlitOnTile(dst, sprite, revert ? sprite.x() + TILEWIDTH - sprite.w() : sprite.x(), sprite.y(), mp);
}

ByteVectorWriter& Maps::operator<<(ByteVectorWriter& msg, const TilesAddon& ta)
{
    return msg << ta.level << ta.uniq << ta.object << ta.index << ta.tmp;
}

ByteVectorReader& Maps::operator>>(ByteVectorReader& msg, TilesAddon& ta)
{
    msg >> ta.level;
    ta.uniq = msg.getBE32();
    return msg >> ta.object >> ta.index >> ta.tmp;
}

ByteVectorWriter& Maps::operator<<(ByteVectorWriter& msg, const Tiles& tile)
{
    return msg <<
        tile.maps_index <<
        tile.pack_sprite_index <<
        tile.tile_passable <<
        tile.mp2_object <<
        tile.fog_colors <<
        tile.quantity1 <<
        tile.quantity2 <<
        tile.quantity3 <<
        tile.addons_level1._items <<
        tile.addons_level2._items;
}

ByteVectorReader& Maps::operator>>(ByteVectorReader& msg, Tiles& tile)
{
    return msg >>
        tile.maps_index >>
        tile.pack_sprite_index >>
        tile.tile_passable >>
        tile.mp2_object >>
        tile.fog_colors >>
        tile.quantity1 >>
        tile.quantity2 >>
        tile.quantity3 >>
        tile.addons_level1._items >>
        tile.addons_level2._items;
}
