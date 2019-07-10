/***************************************************************************
 *   Copyright (C) 2010 by Andrey Afletdinov <fheroes2@gmail.com>          *
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

#include "icn.h"
#include <algorithm>
#include "engine.h"
#include "agg.h"
#include "world.h"
#include "luck.h"
#include "morale.h"
#include "speed.h"
#include "settings.h"
#include "battle_arena.h"
#include "battle_interface.h"
#include "battle_troop.h"
#include "game_static.h"
#include "m82.h"
#include "rand.h"
#include "battle_army.h"
#include <sstream>
#include <iomanip>

namespace Battle
{
    monstersprite_t monsters_info[] = {

        {
            ICN::UNKNOWN, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
            {0, 0}, {0, 0}, {0, 0}, {0, 0}, M82::UNKNOWN, M82::UNKNOWN, M82::UNKNOWN, M82::UNKNOWN
        },

        // icn_file      idle     move     fly1     fly2     fly3     shot0    shot1    shot2    shot3    attk0    attk1    attk2    attk3    wcne     kill     m82_attk       m82_kill       m82_move       m82_wnce
        {
            ICN::PEASANT, {1, 4}, {5, 8}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {16, 6},
            {22, 6}, {28, 6}, {13, 3}, {34, 4}, M82::PSNTATTK, M82::PSNTKILL, M82::PSNTMOVE, M82::PSNTWNCE
        },
        {
            ICN::ARCHER, {1, 4}, {5, 8}, {0, 0}, {0, 0}, {0, 0}, {16, 4}, {20, 4}, {24, 4}, {28, 4}, {32, 4}, {36, 3},
            {39, 3}, {42, 3}, {13, 3}, {45, 6}, M82::ARCHATTK, M82::ARCHKILL, M82::ARCHMOVE, M82::ARCHWNCE
        },
        {
            ICN::ARCHER2, {1, 4}, {5, 8}, {0, 0}, {0, 0}, {0, 0}, {16, 4}, {20, 4}, {24, 4}, {28, 4}, {32, 4}, {36, 3},
            {39, 3}, {42, 3}, {13, 3}, {45, 6}, M82::ARCHATTK, M82::ARCHKILL, M82::ARCHMOVE, M82::ARCHWNCE
        },
        {
            ICN::PIKEMAN, {1, 4}, {5, 8}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {17, 3},
            {20, 4}, {24, 6}, {13, 4}, {30, 6}, M82::PIKEATTK, M82::PIKEKILL, M82::PIKEMOVE, M82::PIKEWNCE
        },
        {
            ICN::PIKEMAN2, {1, 4}, {5, 8}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {17, 3},
            {20, 4}, {24, 6}, {13, 4}, {30, 6}, M82::PIKEATTK, M82::PIKEKILL, M82::PIKEMOVE, M82::PIKEWNCE
        },
        {
            ICN::SWORDSMN, {39, 6}, {2, 8}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {12, 2}, {19, 5},
            {14, 5}, {24, 5}, {36, 3}, {29, 7}, M82::SWDMATTK, M82::SWDMKILL, M82::SWDMMOVE, M82::SWDMWNCE
        },
        {
            ICN::SWORDSM2, {39, 6}, {2, 8}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {12, 2}, {19, 5},
            {14, 5}, {24, 5}, {36, 3}, {29, 7}, M82::SWDMATTK, M82::SWDMKILL, M82::SWDMMOVE, M82::SWDMWNCE
        },
        {
            ICN::CAVALRYR, {19, 4}, {1, 7}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {8, 1}, {12, 3},
            {9, 3}, {15, 2}, {28, 2}, {23, 5}, M82::CAVLATTK, M82::CAVLKILL, M82::CAVLMOVE, M82::CAVLWNCE
        },
        {
            ICN::CAVALRYB, {19, 4}, {1, 7}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {8, 1}, {12, 3},
            {9, 3}, {15, 2}, {28, 2}, {23, 5}, M82::CAVLATTK, M82::CAVLKILL, M82::CAVLMOVE, M82::CAVLWNCE
        },
        {
            ICN::PALADIN, {1, 11}, {12, 8}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {20, 2}, {22, 3},
            {25, 3}, {28, 3}, {31, 2}, {34, 5}, M82::PLDNATTK, M82::PLDNKILL, M82::PLDNMOVE, M82::PLDNWNCE
        },
        {
            ICN::PALADIN2, {1, 11}, {12, 8}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {20, 2}, {22, 3},
            {25, 3}, {28, 3}, {31, 2}, {34, 5}, M82::PLDNATTK, M82::PLDNKILL, M82::PLDNMOVE, M82::PLDNWNCE
        },

        // icn_file      idle     move     fly1     fly2     fly3     shot0    shot1    shot2    shot3    attk0    attk1    attk2    attk3    wcne     kill     m82_attk       m82_kill       m82_move       m82_wnce
        {
            ICN::GOBLIN, {33, 7}, {1, 9}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {10, 3}, {17, 4},
            {13, 4}, {21, 4}, {25, 4}, {29, 4}, M82::GBLNATTK, M82::GBLNKILL, M82::GBLNMOVE, M82::GBLNWNCE
        },
        {
            ICN::ORC, {1, 4}, {5, 8}, {0, 0}, {0, 0}, {0, 0}, {16, 13}, {0, 0}, {0, 0}, {0, 0}, {28, 2}, {30, 3},
            {33, 4}, {37, 3}, {13, 3}, {40, 4}, M82::ORC_ATTK, M82::ORC_KILL, M82::ORC_MOVE, M82::ORC_WNCE
        },
        {
            ICN::ORC2, {1, 4}, {5, 8}, {0, 0}, {0, 0}, {0, 0}, {16, 13}, {0, 0}, {0, 0}, {0, 0}, {28, 2}, {30, 3},
            {33, 4}, {37, 3}, {13, 3}, {40, 4}, M82::ORC_ATTK, M82::ORC_KILL, M82::ORC_MOVE, M82::ORC_WNCE
        },
        {
            ICN::WOLF, {20, 6}, {7, 6}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {13, 3}, {2, 5},
            {16, 4}, {26, 3}, {28, 5}, M82::WOLFATTK, M82::WOLFKILL, M82::WOLFMOVE, M82::WOLFWNCE
        },
        {
            ICN::OGRE, {1, 4}, {5, 8}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {16, 6}, {22, 7},
            {29, 8}, {13, 3}, {37, 4}, M82::OGREATTK, M82::OGREKILL, M82::OGREMOVE, M82::OGREWNCE
        },
        {
            ICN::OGRE2, {1, 4}, {5, 8}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {16, 6},
            {22, 7}, {29, 8}, {13, 3}, {37, 4}, M82::OGREATTK, M82::OGREKILL, M82::OGREMOVE, M82::OGREWNCE
        },
        {
            ICN::TROLL, {16, 7}, {1, 15}, {0, 0}, {0, 0}, {0, 0}, {23, 5}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {48, 6},
            {23, 6}, {29, 5}, {54, 3}, {57, 9}, M82::TRLLATTK, M82::TRLLKILL, M82::TRLLMOVE, M82::TRLLWNCE
        },
        {
            ICN::TROLL2, {16, 7}, {1, 15}, {0, 0}, {0, 0}, {0, 0}, {23, 5}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {48, 6},
            {23, 6}, {29, 5}, {54, 3}, {57, 9}, M82::TRLLATTK, M82::TRLLKILL, M82::TRLLMOVE, M82::TRLLWNCE
        },
        {
            ICN::CYCLOPS, {30, 9}, {1, 7}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {16, 2}, {11, 2}, {21, 2}, {0, 0}, {14, 3},
            {8, 3}, {18, 3}, {23, 2}, {25, 5}, M82::CYCLATTK, M82::CYCLKILL, M82::CYCLMOVE, M82::CYCLWNCE
        },

        // icn_file      idle     move     fly1     fly2     fly3     shot0    shot1    shot2    shot3    attk0    attk1    attk2    attk3    wcne     kill     m82_attk       m82_kill       m82_move       m82_wnce
        {
            ICN::SPRITE, {16, 9}, {0, 0}, {1, 3}, {4, 4}, {7, 2}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {30, 6},
            {25, 5}, {36, 5}, {9, 2}, {11, 5}, M82::SPRTATTK, M82::SPRTKILL, M82::SPRTMOVE, M82::SPRTWNCE
        },
        {
            ICN::DWARF, {44, 5}, {1, 9}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {20, 7},
            {10, 10}, {27, 9}, {36, 8}, {49, 7}, M82::DWRFATTK, M82::DWRFKILL, M82::DWRFMOVE, M82::DWRFWNCE
        },
        {
            ICN::DWARF2, {44, 5}, {1, 9}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {20, 7},
            {10, 10}, {27, 9}, {36, 8}, {49, 7}, M82::DWRFATTK, M82::DWRFKILL, M82::DWRFMOVE, M82::DWRFWNCE
        },
        {
            ICN::ELF, {42, 6}, {1, 12}, {0, 0}, {0, 0}, {0, 0}, {13, 10}, {0, 0}, {0, 0}, {0, 0}, {23, 3}, {26, 3},
            {29, 3}, {32, 4}, {36, 2}, {38, 4}, M82::ELF_ATTK, M82::ELF_KILL, M82::ELF_MOVE, M82::ELF_WNCE
        },
        {
            ICN::ELF2, {42, 6}, {1, 12}, {0, 0}, {0, 0}, {0, 0}, {13, 10}, {0, 0}, {0, 0}, {0, 0}, {23, 3}, {26, 3},
            {29, 3}, {32, 4}, {36, 2}, {38, 4}, M82::ELF_ATTK, M82::ELF_KILL, M82::ELF_MOVE, M82::ELF_WNCE
        },
        {
            ICN::DRUID, {46, 4}, {3, 11}, {0, 0}, {0, 0}, {0, 0}, {14, 8}, {35, 4}, {22, 4}, {39, 5}, {14, 8}, {35, 4},
            {22, 4}, {39, 4}, {44, 2}, {26, 9}, M82::DRUIATTK, M82::DRUIKILL, M82::DRUIMOVE, M82::DRUIWNCE
        },
        {
            ICN::DRUID2, {46, 4}, {3, 11}, {0, 0}, {0, 0}, {0, 0}, {14, 8}, {35, 4}, {22, 4}, {39, 5}, {14, 8}, {35, 4},
            {22, 4}, {39, 4}, {44, 2}, {26, 9}, M82::DRUIATTK, M82::DRUIKILL, M82::DRUIMOVE, M82::DRUIWNCE
        },
        {
            ICN::UNICORN, {1, 9}, {10, 8}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {18, 9},
            {27, 7}, {34, 7}, {41, 4}, {45, 7}, M82::UNICATTK, M82::UNICKILL, M82::UNICMOVE, M82::UNICWNCE
        },
        {
            ICN::PHOENIX, {30, 5}, {0, 0}, {1, 3}, {4, 6}, {0, 0}, {0, 0}, {25, 3}, {23, 2}, {28, 2}, {0, 0}, {13, 4},
            {10, 4}, {17, 4}, {35, 2}, {37, 11}, M82::PHOEATTK, M82::PHOEKILL, M82::PHOEMOVE, M82::PHOEWNCE
        },

        // icn_file      idle     move     fly1     fly2     fly3     shot0    shot1    shot2    shot3    attk0    attk1    attk2    attk3    wcne     kill     m82_attk       m82_kill       m82_move       m82_wnce
        {
            ICN::CENTAUR, {65, 6}, {2, 6}, {0, 0}, {0, 0}, {0, 0}, {18, 4}, {27, 3}, {22, 5}, {30, 3}, {33, 1}, {38, 4},
            {34, 4}, {42, 4}, {46, 2}, {48, 7}, M82::CNTRATTK, M82::CNTRKILL, M82::CNTRMOVE, M82::CNTRWNCE
        },
        {
            ICN::GARGOYLE, {1, 4}, {0, 0}, {5, 2}, {7, 4}, {11, 1}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {14, 1}, {15, 4},
            {19, 4}, {23, 4}, {12, 2}, {27, 5}, M82::GARGATTK, M82::GARGKILL, M82::GARGMOVE, M82::GARGWNCE
        },
        {
            ICN::GRIFFIN, {16, 9}, {0, 0}, {1, 3}, {3, 3}, {5, 2}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {7, 1}, {8, 2},
            {14, 2}, {11, 3}, {9, 2}, {25, 9}, M82::GRIFATTK, M82::GRIFKILL, M82::GRIFMOVE, M82::GRIFWNCE
        },
        {
            ICN::MINOTAUR, {1, 5}, {6, 7}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {13, 3}, {20, 3},
            {16, 4}, {23, 4}, {27, 2}, {29, 6}, M82::MINOATTK, M82::MINOKILL, M82::MINOMOVE, M82::MINOWNCE
        },
        {
            ICN::MINOTAU2, {1, 5}, {6, 7}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {13, 3}, {20, 3},
            {16, 4}, {23, 4}, {27, 2}, {29, 6}, M82::MINOATTK, M82::MINOKILL, M82::MINOMOVE, M82::MINOWNCE
        },
        {
            ICN::HYDRA, {28, 15}, {1, 8}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {9, 8},
            {9, 8}, {9, 8}, {17, 5}, {21, 7}, M82::HYDRATTK, M82::HYDRKILL, M82::HYDRMOVE, M82::HYDRWNCE
        },
        {
            ICN::DRAGGREE, {41, 13}, {0, 0}, {1, 4}, {5, 3}, {8, 5}, {13, 1}, {17, 3}, {23, 3}, {30, 2}, {13, 1},
            {14, 3}, {20, 3}, {26, 4}, {32, 3}, {35, 6}, M82::DRGNATTK, M82::DRGNKILL, M82::DRGNMOVE, M82::DRGNWNCE
        },
        {
            ICN::DRAGRED, {41, 13}, {0, 0}, {1, 4}, {5, 3}, {8, 5}, {13, 1}, {17, 3}, {23, 3}, {30, 2}, {13, 1},
            {14, 3}, {20, 3}, {26, 4}, {32, 3}, {35, 6}, M82::DRGNATTK, M82::DRGNKILL, M82::DRGNMOVE, M82::DRGNWNCE
        },
        {
            ICN::DRAGBLAK, {41, 13}, {0, 0}, {1, 4}, {5, 3}, {8, 5}, {13, 1}, {17, 3}, {23, 3}, {30, 2}, {13, 1},
            {14, 3}, {20, 3}, {26, 4}, {32, 3}, {35, 6}, M82::DRGNATTK, M82::DRGNKILL, M82::DRGNMOVE, M82::DRGNWNCE
        },

        // icn_file      idle     move     fly1     fly2     fly3     shot0    shot1    shot2    shot3    attk0    attk1    attk2    attk3    wcne     kill     m82_attk       m82_kill       m82_move       m82_wnce
        {
            ICN::HALFLING, {1, 4}, {5, 8}, {0, 0}, {0, 0}, {0, 0}, {17, 5}, {22, 2}, {26, 1}, {27, 2}, {29, 1}, {30, 2},
            {32, 3}, {35, 2}, {13, 4}, {37, 4}, M82::HALFATTK, M82::HALFKILL, M82::HALFMOVE, M82::HALFWNCE
        },
        {
            ICN::BOAR, {1, 4}, {5, 8}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {15, 1}, {16, 3},
            {19, 3}, {22, 2}, {13, 2}, {24, 5}, M82::BOARATTK, M82::BOARKILL, M82::BOARMOVE, M82::BOARWNCE
        },
        {
            ICN::GOLEM, {34, 6}, {1, 12}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {22, 4},
            {13, 5}, {18, 4}, {32, 4}, {26, 6}, M82::GOLMATTK, M82::GOLMKILL, M82::GOLMMOVE, M82::GOLMWNCE
        },
        {
            ICN::GOLEM2, {34, 6}, {1, 12}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {22, 4},
            {13, 5}, {18, 4}, {32, 4}, {26, 6}, M82::GOLMATTK, M82::GOLMKILL, M82::GOLMMOVE, M82::GOLMWNCE
        },
        {
            ICN::ROC, {18, 7}, {0, 0}, {1, 3}, {4, 3}, {7, 1}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {14, 3}, {8, 3},
            {11, 3}, {25, 3}, {25, 9}, M82::ROC_ATTK, M82::ROC_KILL, M82::ROC_MOVE, M82::ROC_WNCE
        },
        {
            ICN::MAGE1, {1, 7}, {43, 8}, {0, 0}, {0, 0}, {0, 0}, {18, 1}, {21, 2}, {19, 2}, {23, 4}, {27, 1}, {34, 3},
            {28, 6}, {37, 6}, {8, 3}, {11, 7}, M82::MAGEATTK, M82::MAGEKILL, M82::MAGEMOVE, M82::MAGEWNCE
        },
        {
            ICN::MAGE2, {1, 7}, {43, 8}, {0, 0}, {0, 0}, {0, 0}, {18, 1}, {21, 2}, {19, 2}, {23, 4}, {27, 1}, {34, 3},
            {28, 6}, {37, 6}, {8, 3}, {11, 7}, M82::MAGEATTK, M82::MAGEKILL, M82::MAGEMOVE, M82::MAGEWNCE
        },
        {
            ICN::TITANBLU, {1, 6}, {7, 7}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {19, 5},
            {14, 5}, {24, 5}, {29, 2}, {31, 7}, M82::TITNATTK, M82::TITNKILL, M82::TITNMOVE, M82::TITNWNCE
        },
        {
            ICN::TITANBLA, {1, 6}, {7, 7}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {45, 5}, {38, 5}, {50, 5}, {0, 0}, {19, 5},
            {14, 5}, {24, 5}, {29, 2}, {31, 7}, M82::TITNATTK, M82::TITNKILL, M82::TITNMOVE, M82::TITNWNCE
        },

        // icn_file      idle     move     fly1     fly2     fly3     shot0    shot1    shot2    shot3    attk0    attk1    attk2    attk3    wcne     kill     m82_attk       m82_kill       m82_move       m82_wnce
        {
            ICN::SKELETON, {35, 4}, {3, 8}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {20, 4},
            {12, 4}, {16, 4}, {24, 3}, {27, 8}, M82::SKELATTK, M82::SKELKILL, M82::SKELMOVE, M82::SKELWNCE
        },
        {
            ICN::ZOMBIE, {14, 12}, {1, 13}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {26, 5},
            {31, 5}, {36, 5}, {40, 3}, {47, 7}, M82::ZOMBATTK, M82::ZOMBKILL, M82::ZOMBMOVE, M82::ZOMBWNCE
        },
        {
            ICN::ZOMBIE2, {14, 12}, {1, 13}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {26, 5},
            {31, 5}, {36, 5}, {40, 3}, {47, 7}, M82::ZOMBATTK, M82::ZOMBKILL, M82::ZOMBMOVE, M82::ZOMBWNCE
        },
        {
            ICN::MUMMYW, {1, 4}, {5, 12}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {21, 3},
            {24, 3}, {27, 3}, {17, 4}, {30, 10}, M82::MUMYATTK, M82::MUMYKILL, M82::MUMYMOVE, M82::MUMYWNCE
        },
        {
            ICN::MUMMY2, {1, 4}, {5, 12}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {21, 3},
            {24, 3}, {27, 3}, {17, 4}, {30, 10}, M82::MUMYATTK, M82::MUMYKILL, M82::MUMYMOVE, M82::MUMYWNCE
        },
        {
            ICN::VAMPIRE, {1, 4}, {0, 0}, {5, 4}, {9, 4}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {15, 4},
            {19, 4}, {23, 5}, {13, 3}, {28, 8}, M82::VAMPATTK, M82::VAMPKILL, M82::VAMPMOVE, M82::VAMPWNCE
        },
        {
            ICN::VAMPIRE2, {1, 4}, {0, 0}, {5, 4}, {9, 4}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {15, 4},
            {19, 4}, {23, 5}, {13, 3}, {28, 8}, M82::VAMPATTK, M82::VAMPKILL, M82::VAMPMOVE, M82::VAMPWNCE
        },
        {
            ICN::LICH, {1, 4}, {5, 8}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {13, 5}, {18, 5}, {23, 5}, {0, 0}, {13, 5},
            {18, 5}, {23, 5}, {28, 3}, {31, 7}, M82::LICHATTK, M82::LICHKILL, M82::LICHMOVE, M82::LICHWNCE
        },
        {
            ICN::LICH2, {1, 4}, {5, 8}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {13, 5}, {18, 8}, {23, 5}, {0, 0}, {13, 5},
            {18, 5}, {23, 5}, {28, 3}, {31, 7}, M82::LICHATTK, M82::LICHKILL, M82::LICHMOVE, M82::LICHWNCE
        },
        {
            ICN::DRAGBONE, {22, 7}, {0, 0}, {1, 4}, {5, 3}, {8, 4}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {47, 5},
            {12, 4}, {16, 4}, {20, 2}, {41, 6}, M82::BONEATTK, M82::BONEKILL, M82::BONEMOVE, M82::BONEWNCE
        },

        // icn_file      idle     move     fly1     fly2     fly3     shot0    shot1    shot2    shot3    attk0    attk1    attk2    attk3    wcne     kill     m82_attk       m82_kill       m82_move       m82_wnce
        {
            ICN::ROGUE, {1, 4}, {5, 8}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {16, 8},
            {24, 10}, {34, 9}, {13, 3}, {43, 7}, M82::ROGUATTK, M82::ROGUKILL, M82::ROGUMOVE, M82::ROGUWNCE
        },
        {
            ICN::NOMAD, {1, 9}, {10, 8}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {18, 2}, {26, 5},
            {20, 6}, {31, 4}, {35, 2}, {37, 5}, M82::NMADATTK, M82::NMADKILL, M82::NMADMOVE, M82::NMADWNCE
        },
        {
            ICN::GHOST, {1, 3}, {0, 0}, {0, 0}, {4, 5}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {10, 3},
            {13, 3}, {16, 4}, {20, 2}, {20, 8}, M82::GHSTATTK, M82::GHSTKILL, M82::GHSTMOVE, M82::GHSTWNCE
        },
        {
            ICN::GENIE, {1, 9}, {0, 0}, {10, 1}, {11, 4}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {26, 5},
            {16, 5}, {21, 5}, {31, 2}, {33, 7}, M82::GENIATTK, M82::GENIKILL, M82::GENIMOVE, M82::GENIWNCE
        },
        {
            ICN::MEDUSA, {17, 7}, {1, 16}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {65, 10},
            {41, 13}, {54, 11}, {36, 2}, {24, 12}, M82::MEDSATTK, M82::MEDSKILL, M82::MEDSMOVE, M82::MEDSWNCE
        },
        {
            ICN::EELEM, {1, 4}, {5, 8}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {16, 4},
            {20, 5}, {25, 6}, {13, 3}, {31, 11}, M82::EELMATTK, M82::EELMKILL, M82::EELMMOVE, M82::EELMWNCE
        },
        {
            ICN::AELEM, {1, 4}, {5, 8}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {16, 4},
            {20, 5}, {25, 6}, {13, 3}, {31, 11}, M82::AELMATTK, M82::AELMKILL, M82::AELMMOVE, M82::AELMWNCE
        },
        {
            ICN::FELEM, {1, 4}, {5, 8}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {16, 4},
            {20, 5}, {25, 6}, {13, 3}, {31, 11}, M82::FELMATTK, M82::FELMKILL, M82::FELMMOVE, M82::FELMWNCE
        },
        {
            ICN::WELEM, {1, 4}, {5, 8}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {16, 4},
            {20, 5}, {25, 6}, {13, 3}, {31, 11}, M82::WELMATTK, M82::WELMKILL, M82::WELMMOVE, M82::WELMWNCE
        },

        {
            ICN::UNKNOWN, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
            {0, 0}, {0, 0}, {0, 0}, {0, 0}, M82::UNKNOWN, M82::UNKNOWN, M82::UNKNOWN, M82::UNKNOWN
        },
        {
            ICN::UNKNOWN, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
            {0, 0}, {0, 0}, {0, 0}, {0, 0}, M82::UNKNOWN, M82::UNKNOWN, M82::UNKNOWN, M82::UNKNOWN
        },
        {
            ICN::UNKNOWN, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
            {0, 0}, {0, 0}, {0, 0}, {0, 0}, M82::UNKNOWN, M82::UNKNOWN, M82::UNKNOWN, M82::UNKNOWN
        },
        {
            ICN::UNKNOWN, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
            {0, 0}, {0, 0}, {0, 0}, {0, 0}, M82::UNKNOWN, M82::UNKNOWN, M82::UNKNOWN, M82::UNKNOWN
        },
        {
            ICN::UNKNOWN, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
            {0, 0}, {0, 0}, {0, 0}, {0, 0}, M82::UNKNOWN, M82::UNKNOWN, M82::UNKNOWN, M82::UNKNOWN
        },
    };

    uint32_t genie_enemy_half_percent = 10;
}

void Battle::UpdateMonsterAttributes(const string& spec)
{
}

void Battle::UpdateMonsterSpriteAnimation(const string& spec)
{
}

Battle::ModeDuration::ModeDuration() : pair<uint32_t, uint32_t>(0, 0)
{
}

Battle::ModeDuration::ModeDuration(uint32_t mode, uint32_t duration) : pair<uint32_t, uint32_t>(mode, duration)
{
}

bool Battle::ModeDuration::isMode(uint32_t mode) const
{
    return first & mode;
}

bool Battle::ModeDuration::isZeroDuration() const
{
    return 0 == second;
}

void Battle::ModeDuration::DecreaseDuration()
{
    if (second) --second;
}

Battle::ModesAffected::ModesAffected()
{
    _items.reserve(3);
}

uint32_t Battle::ModesAffected::GetMode(uint32_t mode) const
{
    const auto it = find_if(_items.begin(), _items.end(),
                            [&](const ModeDuration& it) { return it.isMode(mode); });
    return it == _items.end() ? 0 : (*it).second;
}

void Battle::ModesAffected::AddMode(uint32_t mode, uint32_t duration)
{
    auto it = find_if(_items.begin(), _items.end(),
                      [&](const ModeDuration& it) { return it.isMode(mode); });
    if (it == _items.end())
        _items.emplace_back(mode, duration);
    else
        (*it).second = duration;
}

void Battle::ModesAffected::RemoveMode(uint32_t mode)
{
    auto it = find_if(_items.begin(), _items.end(),
                      [&](const ModeDuration& it) { return it.isMode(mode); });
    if (it != _items.end())
    {
        // erase(it)
        if (it + 1 != _items.end()) std::swap(*it, _items.back());
        _items.pop_back();
    }
}

void Battle::ModesAffected::DecreaseDuration()
{
    for_each(_items.begin(), _items.end(), [](ModeDuration& it) { it.DecreaseDuration(); });
}

uint32_t Battle::ModesAffected::FindZeroDuration() const
{
    auto it = find_if(_items.begin(), _items.end(), [](const ModeDuration& it) { return it.isZeroDuration(); });
    return it == _items.end() ? 0 : (*it).first;
}

Battle::Unit::Unit(const Troop& t, s32 pos, bool ref) : ArmyTroop(nullptr, t),
                                                        uid(World::GetUniq()), hp(_monster.GetHitPoints(t._monster)),
                                                        count0(t.GetCount()), dead(0), shots(t._monster.GetShots()),
                                                        disruptingray(0), reflect(ref), animstate(0), animframe(0),
                                                        animstep(1), mirror(nullptr), blindanswer(false)
{
    // set position
    if (Board::isValidIndex(pos))
    {
        if (t._monster.isWide()) pos += reflect ? -1 : 1;
        SetPosition(pos);
    }

    ResetAnimFrame(AS_IDLE);
}

Battle::Unit::~Unit()
{
    // reset summon elemental and mirror image
    if (Modes(CAP_SUMMONELEM) || Modes(CAP_MIRRORIMAGE))
    {
        SetCount(0);
    }
}

void Battle::Unit::SetPosition(s32 pos)
{
    if (position.GetHead()) position.GetHead()->SetUnit(nullptr);
    if (position.GetTail()) position.GetTail()->SetUnit(nullptr);

    position.Set(pos, _monster.isWide(), reflect);

    if (position.GetHead()) position.GetHead()->SetUnit(this);
    if (position.GetTail()) position.GetTail()->SetUnit(this);
}

void Battle::Unit::SetPosition(const Position& pos)
{
    if (position.GetHead()) position.GetHead()->SetUnit(nullptr);
    if (position.GetTail()) position.GetTail()->SetUnit(nullptr);

    position = pos;

    if (position.GetHead()) position.GetHead()->SetUnit(this);
    if (position.GetTail()) position.GetTail()->SetUnit(this);

    if (_monster.isWide())
    {
        reflect = GetHeadIndex() < GetTailIndex();
    }
}

void Battle::Unit::SetReflection(bool r)
{
    if (reflect != r)
        position.Swap();

    reflect = r;
}

void Battle::Unit::UpdateDirection()
{
    // set auto reflect
    SetReflection(GetArena()->GetArmyColor1() != GetArmyColor());
}

bool Battle::Unit::UpdateDirection(const Rect& pos)
{
    bool need = position.GetRect().x > pos.x;

    if (need != reflect)
    {
        SetReflection(need);
        return true;
    }
    return false;
}

bool Battle::Unit::isBattle() const
{
    return true;
}

bool Battle::Unit::isModes(uint32_t v) const
{
    return Modes(v);
}

string Battle::Unit::GetShotString() const
{
    if (_monster.GetShots() == GetShots())
        return Int2Str(_monster.GetShots());

    ostringstream os;
    os << _monster.GetShots() << " (" << GetShots() << ")";
    return os.str();
}

string Battle::Unit::GetSpeedString() const
{
    ostringstream os;
    os << Speed::String(GetSpeed()) << " (" << GetSpeed() << ")";
    return os.str();
}

Surface Battle::Unit::GetContour(int val) const
{
    switch (val)
    {
    case CONTOUR_MAIN:
        return contours[0];
    case CONTOUR_REFLECT:
        return contours[1];
    case CONTOUR_BLACK:
        return contours[2];
    case CONTOUR_BLACK | CONTOUR_REFLECT:
        return contours[3];
    default:
        break;
    }

    return Surface();
}

uint32_t Battle::Unit::GetDead() const
{
    return dead;
}

uint32_t Battle::Unit::GetHitPointsLeft() const
{
    const auto monsterLife = _monster.GetHitPoints();
    return hp-monsterLife*(count-1);
}

uint32_t Battle::Unit::GetAffectedDuration(uint32_t mod) const
{
    return affected.GetMode(mod);
}

uint32_t Battle::Unit::GetSpeed() const
{
    return GetSpeed(false);
}

bool Battle::Unit::isUID(uint32_t v) const
{
    return uid == v;
}

uint32_t Battle::Unit::GetUID() const
{
    return uid;
}

const Battle::monstersprite_t& Battle::Unit::GetMonsterSprite() const
{
    return monsters_info[_monster.GetID()];
}

void Battle::Unit::InitContours()
{
    const monstersprite_t& msi = GetMonsterSprite();
    const Sprite& sprite1 = AGG::GetICN(msi.icn_file, msi.frm_idle.start, false);
    const Sprite& sprite2 = AGG::GetICN(msi.icn_file, msi.frm_idle.start, true);

    // main sprite
    contours[0] = sprite1.RenderContour(RGBA(0xe0, 0xe0, 0));

    // revert sprite
    contours[1] = sprite2.RenderContour(RGBA(0xe0, 0xe0, 0));

    // create white black sprite
    contours[2] = sprite1.RenderGrayScale();
    contours[3] = sprite2.RenderGrayScale();
}

void Battle::Unit::SetMirror(Unit* ptr)
{
    mirror = ptr;
}

uint32_t Battle::Unit::GetShots() const
{
    return shots;
}

const Battle::Position& Battle::Unit::GetPosition() const
{
    return position;
}

s32 Battle::Unit::GetHeadIndex() const
{
    return position.GetHead() ? position.GetHead()->GetIndex() : -1;
}

s32 Battle::Unit::GetTailIndex() const
{
    return position.GetTail() ? position.GetTail()->GetIndex() : -1;
}

void Battle::Unit::SetRandomMorale()
{
    switch (GetMorale())
    {
    case Morale::TREASON:
        if (9 > Rand::Get(1, 16)) SetModes(MORALE_BAD);
        break; // 50%
    case Morale::AWFUL:
        if (6 > Rand::Get(1, 15)) SetModes(MORALE_BAD);
        break; // 30%
    case Morale::POOR:
        if (2 > Rand::Get(1, 15)) SetModes(MORALE_BAD);
        break; // 15%
    case Morale::GOOD:
        if (2 > Rand::Get(1, 15)) SetModes(MORALE_GOOD);
        break; // 15%
    case Morale::GREAT:
        if (6 > Rand::Get(1, 15)) SetModes(MORALE_GOOD);
        break; // 30%
    case Morale::BLOOD:
        if (9 > Rand::Get(1, 16)) SetModes(MORALE_GOOD);
        break; // 50%
    default:
        break;
    }
}

void Battle::Unit::SetRandomLuck()
{
    s32 f = GetLuck();

    //check enemy: have bone dragon
    if (GetArena()->GetForce(GetArmyColor(), true).HasMonster(Monster::BONE_DRAGON)) --f;

    switch (f)
    {
    case Luck::CURSED:
        if (9 > Rand::Get(1, 16)) SetModes(LUCK_BAD);
        break; // 50%
    case Luck::AWFUL:
        if (6 > Rand::Get(1, 15)) SetModes(LUCK_BAD);
        break; // 30%
    case Luck::BAD:
        if (2 > Rand::Get(1, 15)) SetModes(LUCK_BAD);
        break; // 15%
    case Luck::GOOD:
        if (2 > Rand::Get(1, 15)) SetModes(LUCK_GOOD);
        break; // 15%
    case Luck::GREAT:
        if (6 > Rand::Get(1, 15)) SetModes(LUCK_GOOD);
        break; // 30%
    case Luck::IRISH:
        if (9 > Rand::Get(1, 16)) SetModes(LUCK_GOOD);
        break; // 50%
    default:
        break;
    }

    if (Modes(SP_BLESS) && Modes(LUCK_GOOD))
        ResetModes(LUCK_GOOD);
    else if (Modes(SP_CURSE) && Modes(LUCK_BAD))
        ResetModes(LUCK_BAD);
}

bool Battle::Unit::isFly() const
{
    return _monster.isFly() && !Modes(SP_SLOW);
}

bool Battle::Unit::isValid() const
{
    return GetCount();
}

bool Battle::Unit::isReflect() const
{
    return reflect;
}

bool Battle::Unit::OutOfWalls() const
{
    return Board::isOutOfWallsIndex(GetHeadIndex()) ||
        (_monster.isWide() && Board::isOutOfWallsIndex(GetTailIndex()));
}

bool Battle::Unit::isHandFighting() const
{
    if (GetCount() && !Modes(CAP_TOWER))
    {
        const Indexes around = Board::GetAroundIndexes(*this);

        for (int it : around)
        {
            const Unit* enemy = Board::GetCell(it)->GetUnit();
            if (enemy && enemy->GetColor() != GetColor()) return true;
        }
    }

    return false;
}

bool Battle::Unit::isHandFighting(const Unit& a, const Unit& b)
{
    return a.isValid() && !a.Modes(CAP_TOWER) && b.isValid() && b.GetColor() != a.GetColor() &&
    (Board::isNearIndexes(a.GetHeadIndex(), b.GetHeadIndex()) ||
        (b._monster.isWide() && Board::isNearIndexes(a.GetHeadIndex(), b.GetTailIndex())) ||
        (a._monster.isWide() && (Board::isNearIndexes(a.GetTailIndex(), b.GetHeadIndex()) ||
            (b._monster.isWide() && Board::isNearIndexes(a.GetTailIndex(), b.GetTailIndex())))));
}

void Battle::Unit::NewTurn()
{
    if (_monster.isResurectLife()) hp = ArmyTroop::GetHitPointsTroop();

    ResetModes(TR_RESPONSED);
    ResetModes(TR_MOVED);
    ResetModes(TR_SKIPMOVE);
    ResetModes(TR_HARDSKIP);
    ResetModes(TR_DEFENSED);
    ResetModes(MORALE_BAD);
    ResetModes(MORALE_GOOD);
    ResetModes(LUCK_BAD);
    ResetModes(LUCK_GOOD);

    // decrease spell duration
    affected.DecreaseDuration();

    // remove spell duration
    uint32_t mode = 0;
    while (0 != (mode = affected.FindZeroDuration()))
    {
        affected.RemoveMode(mode);
        ResetModes(mode);

        // cancel mirror image
        if (mode == CAP_MIRROROWNER && mirror)
        {
            if (Arena::GetInterface()) Arena::GetInterface()->RedrawActionRemoveMirrorImage(*mirror);

            mirror->SetCount(0);
            mirror = nullptr;
        }
    }

    if (!Modes(SP_BLIND | IS_PARALYZE_MAGIC))
    {
        // define morale
        if (_monster.isAffectedByMorale())
            SetRandomMorale();

        // define luck
        SetRandomLuck();
    }
}

uint32_t Battle::Unit::GetSpeed(bool skip_standing_check) const
{
    if (!skip_standing_check && (!GetCount() || Modes(TR_MOVED | SP_BLIND | IS_PARALYZE_MAGIC))) return Speed::STANDING;

    int speed = _monster.GetSpeed();
    Spell spell;

    if (Modes(SP_HASTE))
    {
        spell = Spell::HASTE;
        return spell.ExtraValue() ? speed + spell.ExtraValue() : Speed::GetOriginalFast(speed);
    }
    if (Modes(SP_SLOW))
    {
        spell = Spell::SLOW;
        return spell.ExtraValue() ? speed - spell.ExtraValue() : Speed::GetOriginalSlow(speed);
    }

    return speed;
}

uint32_t Battle::Unit::GetDamageMin(const Unit& enemy) const
{
    return CalculateDamageUnit(enemy, ArmyTroop::GetDamageMin());
}

uint32_t Battle::Unit::GetDamageMax(const Unit& enemy) const
{
    return CalculateDamageUnit(enemy, ArmyTroop::GetDamageMax());
}

uint32_t Battle::Unit::CalculateDamageUnit(const Unit& enemy, double dmg) const
{
    if (isArchers())
    {
        if (isHandFighting())
        {
            switch (GetID())
            {
                // skip
            case Monster::MAGE:
            case Monster::ARCHMAGE:
            case Monster::TITAN:
                break;

            default:
                dmg /= 2;
                break;
            }
        }
        else
        {
            // check skill archery +%10, +%25, +%50
            if (GetCommander())
            {
                dmg += dmg * GetCommander()->GetSecondaryValues(Skill::SkillT::ARCHERY) / 100;
            }

            // check castle defense
            if (GetArena()->GetObstaclesPenalty(*this, enemy)) dmg /= 2;

            // check spell shield
            if (enemy.Modes(SP_SHIELD)) dmg /= Spell(Spell::SHIELD).ExtraValue();
        }
    }

    // after blind
    if (Modes(SP_BLIND)) dmg /= 2;

    // stone cap.
    if (enemy.Modes(SP_STONE)) dmg /= 2;

    // check monster capability
    switch (GetID())
    {
    case Monster::CRUSADER:
        // double damage for undead
        if (enemy._monster.isUndead()) dmg *= 2;
        break;

    default:
        break;
    }

    // approximate.. from faq
    int r = GetAttack() - enemy.GetDefense();
    if (enemy._monster.isDragons() && Modes(SP_DRAGONSLAYER)) r += Spell(Spell::DRAGONSLAYER).ExtraValue();
    dmg *= 1 + (0 < r ? 0.1 * min(r, 20) : 0.05 * max(r, -15));

    return static_cast<uint32_t>(dmg) < 1 ? 1 : static_cast<uint32_t>(dmg);
}

uint32_t Battle::Unit::GetDamage(const Unit& enemy) const
{
    uint32_t res = 0;

    if (Modes(SP_BLESS))
        res = GetDamageMax(enemy);
    else if (Modes(SP_CURSE))
        res = GetDamageMin(enemy);
    else
        res = Rand::Get(GetDamageMin(enemy), GetDamageMax(enemy));

    if (Modes(LUCK_GOOD)) res <<= 1; // mul 2
    else if (Modes(LUCK_BAD)) res >>= 1; // div 2

    return res;
}

uint32_t Battle::Unit::HowManyCanKill(const Unit& b) const
{
    uint32_t dmg = (GetDamageMin(b) + GetDamageMax(b)) / 2;
    return b.HowManyWillKilled(dmg);
}

uint32_t Battle::Unit::HowManyWillKilled(uint32_t& dmg) const
{
    int unitLife = _monster.GetHitPoints(this->_monster);

    int killTopUnit = dmg > hp ? 1 : 0;
    if (killTopUnit)
    {
        dmg -= hp;
    }
    int unitsToKill = dmg / unitLife;
    unitsToKill += killTopUnit;
    dmg %= unitLife;
    return unitsToKill;
}

uint32_t Battle::Unit::ApplyDamage(uint32_t dmg)
{
    if (!dmg || !GetCount())
        return 0;
    uint32_t killed = HowManyWillKilled(dmg);

    // kill mirror image (slave)
    if (Modes(CAP_MIRRORIMAGE))
    {
        if (Arena::GetInterface()) Arena::GetInterface()->RedrawActionRemoveMirrorImage(*this);
        mirror->ResetModes(CAP_MIRROROWNER);
        dmg = hp;
        killed = GetCount();
        mirror = nullptr;
    }


    if (killed >= GetCount())
    {
        dead += GetCount();
        SetCount(0);
    }
    else
    {
        dead += killed;
        SetCount(GetCount() - killed);
    }
    hp -= dmg >= hp ? hp : dmg;

    if (!isValid()) PostKilledAction();

    return killed;
}

void Battle::Unit::PostKilledAction()
{
    // kill mirror image (master)
    if (Modes(CAP_MIRROROWNER))
    {
        if (Arena::GetInterface()) Arena::GetInterface()->RedrawActionRemoveMirrorImage(*mirror);
        modes = 0;
        mirror->hp = 0;
        mirror->SetCount(0);
        mirror->mirror = nullptr;
        mirror->animstate = 0;
        mirror->animframe = 0;
        mirror = nullptr;
        ResetModes(CAP_MIRROROWNER);
    }

    ResetModes(IS_MAGIC);
    ResetModes(TR_RESPONSED);
    ResetModes(TR_SKIPMOVE);
    ResetModes(LUCK_GOOD);
    ResetModes(LUCK_BAD);
    ResetModes(MORALE_GOOD);
    ResetModes(MORALE_BAD);

    SetModes(TR_MOVED);

    // save troop to graveyard
    // skip mirror and summon
    if (!Modes(CAP_MIRRORIMAGE) && !Modes(CAP_SUMMONELEM))
        Arena::GetGraveyard()->AddTroop(*this);

    Cell* head = Board::GetCell(GetHeadIndex());
    Cell* tail = Board::GetCell(GetTailIndex());
    if (head) head->SetUnit(nullptr);
    if (tail) tail->SetUnit(nullptr);

    // possible also..
}

uint32_t Battle::Unit::Resurrect(uint32_t points, bool allow_overflow, bool skip_dead)
{
    int resurrect = Monster::GetCountFromHitPoints(this->_monster, hp + points);

    SetCount(GetCount() + resurrect);
    hp += points;

    if (allow_overflow)
    {
        if (count0 < GetCount()) count0 = GetCount();
    }
    else if (GetCount() > count0)
    {
        resurrect -= GetCount() - count0;
        SetCount(count0);
        hp = _monster.GetHitPoints();
    }

    if (!skip_dead)
        dead -= resurrect < dead ? resurrect : dead;

    return resurrect;
}

uint32_t Battle::Unit::ApplyDamage(Unit& enemy, uint32_t dmg)
{
    uint32_t killed = ApplyDamage(dmg);
    uint32_t resurrect;

    switch (enemy.GetID())
    {
    case Monster::GENIE:
        // 10% half
        if (1 < GetCount() && killed < GetCount() &&
            genie_enemy_half_percent >= Rand::Get(1, 100))
        {
            killed = ApplyDamage(hp / 2);

            if (Arena::GetInterface())
            {
                string str(_("%{name} half the enemy troops!"));
                StringReplace(str, "%{name}", enemy.GetName());
                Arena::GetInterface()->SetStatus(str, true);
            }
        }
        break;

    default:
        break;
    }

    if (killed)
        switch (enemy.GetID())
        {
        case Monster::GHOST:
            resurrect = killed * static_cast<Monster>(enemy._monster).GetHitPoints();
            // grow troop
            enemy.Resurrect(resurrect, true, false);
            break;

        case Monster::VAMPIRE_LORD:
            resurrect = killed * _monster.GetHitPoints();
            // restore hit points
            enemy.Resurrect(resurrect, false, false);
            break;

        default:
            break;
        }

    // clean paralyze or stone magic
    if (Modes(IS_PARALYZE_MAGIC))
    {
        SetModes(TR_RESPONSED);
        SetModes(TR_MOVED);
        ResetModes(IS_PARALYZE_MAGIC);
        affected.RemoveMode(IS_PARALYZE_MAGIC);
    }

    // blind
    if (Modes(SP_BLIND))
    {
        blindanswer = true;
    }

    return killed;
}

bool Battle::Unit::AllowApplySpell(const Spell& spell, const HeroBase* hero, string* msg) const
{
    if (Modes(SP_ANTIMAGIC)) return false;

    if ((Modes(CAP_MIRRORIMAGE) || Modes(CAP_MIRROROWNER)) &&
        (spell == Spell::ANTIMAGIC || spell == Spell::MIRRORIMAGE))
        return false;

    // check global
    // if(GetArena()->DisableCastSpell(spell, msg)) return false; // disable - recursion!

    if (hero && spell.isApplyToFriends() && GetColor() != hero->GetColor()) return false;
    if (hero && spell.isApplyToEnemies() && GetColor() == hero->GetColor()) return false;
    if (isMagicResist(spell, hero ? hero->GetPower() : 0)) return false;

    const HeroBase* myhero = GetCommander();
    if (!myhero) return true;

    // check artifact
    Artifact guard_art(Artifact::UNKNOWN);
    switch (spell())
    {
    case Spell::CURSE:
    case Spell::MASSCURSE:
        guard_art = Artifact::HOLY_PENDANT;
        break;
    case Spell::HYPNOTIZE:
        guard_art = Artifact::PENDANT_FREE_WILL;
        break;
    case Spell::DEATHRIPPLE:
    case Spell::DEATHWAVE:
        guard_art = Artifact::PENDANT_LIFE;
        break;
    case Spell::BERSERKER:
        guard_art = Artifact::SERENITY_PENDANT;
        break;
    case Spell::BLIND:
        guard_art = Artifact::SEEING_EYE_PENDANT;
        break;
    case Spell::PARALYZE:
        guard_art = Artifact::KINETIC_PENDANT;
        break;
    case Spell::HOLYWORD:
    case Spell::HOLYSHOUT:
        guard_art = Artifact::PENDANT_DEATH;
        break;
    case Spell::DISPEL:
        guard_art = Artifact::WAND_NEGATION;
        break;

    default:
        break;
    }

    if (guard_art.IsValid() && myhero->HasArtifact(guard_art))
    {
        if (msg)
        {
            *msg = _("The %{artifact} artifact is in effect for this battle, disabling %{spell} spell.");
            StringReplace(*msg, "%{artifact}", guard_art.GetName());
            StringReplace(*msg, "%{spell}", spell.GetName());
        }
        return false;
    }

    return true;
}

bool Battle::Unit::ApplySpell(const Spell& spell, const HeroBase* hero, TargetInfo& target)
{
    if (!AllowApplySpell(spell, hero)) return false;
    uint32_t spoint = hero ? hero->GetPower() : 3;

    if (spell.isDamage())
        SpellApplyDamage(spell, spoint, hero, target);
    else if (spell.isRestore())
        SpellRestoreAction(spell, spoint, hero);
    else
    {
        SpellModesAction(spell, spoint, hero);
    }

    return true;
}

string Battle::Unit::String(bool more) const
{
    stringstream ss;

    ss << "Unit: " << "[ " <<
        // info
        GetCount() << " " << GetName() << ", " << Color::String(GetColor()) <<
        ", pos: " << GetHeadIndex() << ", " << GetTailIndex() << (reflect ? ", reflect" : "");

    if (more)
        ss <<
            ", mode(" << "0x" << hex << modes << dec << ")" <<
            ", uid(" << "0x" << setw(8) << setfill('0') << hex << uid << dec << ")" <<
            ", speed(" << Speed::String(GetSpeed()) << ", " << static_cast<int>(GetSpeed()) << ")" <<
            ", hp(" << hp << ")" << ", die(" << dead << ")" << ")";

    ss << " ]";

    return ss.str();
}

ByteVectorWriter& Battle::operator<<(ByteVectorWriter& msg, const ModesAffected& v)
{
    msg << static_cast<uint32_t>(v._items.size());

    for (const auto& _item : v._items)
        msg << _item.first << _item.second;

    return msg;
}


ByteVectorReader& Battle::operator>>(ByteVectorReader& msg, ModesAffected& v)
{
    uint32_t size = 0;
    msg >> size;
    v._items.clear();

    for (size_t ii = 0; ii < size; ++ii)
    {
        ModeDuration md;
        msg >> md.first >> md.second;
        v._items.push_back(md);
    }

    return msg;
}


ByteVectorWriter& Battle::operator<<(ByteVectorWriter& msg, const Unit& b)
{
    return msg <<
        b.modes <<
        b._monster.id <<
        b.count <<
        b.uid <<
        b.hp <<
        b.count0 <<
        b.dead <<
        b.shots <<
        b.disruptingray <<
        b.reflect <<
        b.GetHeadIndex() <<
        (b.mirror ? b.mirror->GetUID() : static_cast<uint32_t>(0)) <<
        b.affected <<
        b.blindanswer;
}

ByteVectorReader& Battle::operator>>(ByteVectorReader& msg, Unit& b)
{
    s32 head = -1;
    uint32_t uid = 0;

    msg >>
        b.modes >>
        b._monster.id >>
        b.count >>
        b.uid >>
        b.hp >>
        b.count0 >>
        b.dead >>
        b.shots >>
        b.disruptingray >>
        b.reflect >>
        head >>
        uid >>
        b.affected >>
        b.blindanswer;

    b.position.Set(head, b._monster.isWide(), b.isReflect());
    b.mirror = GetArena()->GetTroopUID(uid);

    return msg;
}

bool Battle::Unit::AllowResponse() const
{
    if (_monster.isAlwayResponse())
        return true;

    if (!Modes(TR_RESPONSED))
    {
        if (Modes(SP_BLIND))
            return blindanswer;
        return !Modes(IS_PARALYZE_MAGIC);
    }

    return false;
}

void Battle::Unit::SetResponse()
{
    SetModes(TR_RESPONSED);
}

void Battle::Unit::PostAttackAction(Unit& enemy)
{
    switch (GetID())
    {
    case Monster::ARCHMAGE:
        // 20% clean magic state
        if (enemy.isValid() && enemy.Modes(IS_GOOD_MAGIC) && 3 > Rand::Get(1, 10)) enemy.ResetModes(IS_GOOD_MAGIC);
        break;

    default:
        break;
    }

    // decrease shots
    if (isArchers())
    {
        // check ammo cart artifact
        const HeroBase* hero = GetCommander();
        if (!hero || !hero->HasArtifact(Artifact::AMMO_CART))
            --shots;
    }

    // clean berserker spell
    if (Modes(SP_BERSERKER))
    {
        ResetModes(SP_BERSERKER);
        affected.RemoveMode(SP_BERSERKER);
    }

    // clean hypnotize spell
    if (Modes(SP_HYPNOTIZE))
    {
        ResetModes(SP_HYPNOTIZE);
        affected.RemoveMode(SP_HYPNOTIZE);
    }
    if (enemy.Modes(SP_HYPNOTIZE))
    {
        enemy.ResetModes(SP_HYPNOTIZE);
        enemy.affected.RemoveMode(SP_HYPNOTIZE);
    }

    // clean luck capability
    ResetModes(LUCK_GOOD);
    ResetModes(LUCK_BAD);
}

void Battle::Unit::ResetBlind()
{
    // remove blind action
    if (!Modes(SP_BLIND))
        return;
    SetModes(TR_MOVED);
    ResetModes(SP_BLIND);
    affected.RemoveMode(SP_BLIND);
}

uint32_t Battle::Unit::GetAttack() const
{
    uint32_t res = ArmyTroop::GetAttack();

    if (Modes(SP_BLOODLUST)) res += Spell(Spell::BLOODLUST).ExtraValue();

    return res;
}

uint32_t Battle::Unit::GetDefense() const
{
    uint32_t res = ArmyTroop::GetDefense();

    if (GetArena()->GetArmyColor2() == GetColor() &&
        GetArena()->GetForce2().Modes(ARMY_GUARDIANS_OBJECT))
        res += 2;

    if (Modes(SP_STONESKIN)) res += Spell(Spell::STONESKIN).ExtraValue();
    else if (Modes(SP_STEELSKIN)) res += Spell(Spell::STEELSKIN).ExtraValue();

    // extra
    if (Modes(TR_DEFENSED)) res += 2;

    // disrupting ray accumulate effect
    if (disruptingray)
    {
        const uint32_t step = disruptingray * Spell(Spell::DISRUPTINGRAY).ExtraValue();
        if (step > res)
            res = 1;
        else
            res -= step;
    }

    // check moat
    if (Board::isMoatIndex(GetHeadIndex()))
        res -= GameStatic::GetBattleMoatReduceDefense();

    return res;
}

s32 Battle::Unit::GetScoreQuality(const Unit& defender) const
{
    const Unit& attacker = *this;

    // initial value: (hitpoints)
    const uint32_t& damage = (attacker.GetDamageMin(defender) + attacker.GetDamageMax(defender)) / 2;
    uint32_t dmg = attacker.isTwiceAttack() ? damage * 2 : damage;
    const uint32_t& kills = defender.HowManyWillKilled(dmg);
    double res = kills * _monster.GetHitPoints(defender._monster);
    bool noscale = false;

    // attacker
    switch (attacker.GetID())
    {
    case Monster::GHOST:
        // priority: from killed only
        noscale = true;
        break;

    case Monster::VAMPIRE_LORD:
        if (attacker.isHaveDamage())
        {
            // alive priority
            if (defender._monster.isElemental() || defender._monster.isUndead()) res /= 2;
        }
        break;

    default:
        break;
    }

    // scale on ability
    if (!noscale)
    {
        if (defender.isArchers()) res += res * 0.7;
        if (defender.isFly()) res += res * 0.6;
        if (defender._monster.isHideAttack()) res += res * 0.5;
        if (defender.isTwiceAttack()) res += res * 0.4;
        if (defender._monster.isResurectLife()) res += res * 0.3;
        if (defender._monster.isDoubleCellAttack()) res += res * 0.3;
        if (defender._monster.isAlwayResponse()) res -= res * 0.5;
    }

    // extra
    if (defender.Modes(CAP_MIRRORIMAGE)) res += res * 0.7;
    if (!attacker.isArchers())
    {
        if (defender.Modes(TR_RESPONSED))
            res += res * 0.3;
        else
        {
            if (defender.Modes(LUCK_BAD)) res += res * 0.3;
            else if (defender.Modes(LUCK_GOOD)) res -= res * 0.3;
        }
    }

    return static_cast<s32>(res) > 1 ? static_cast<uint32_t>(res) : 1;
}

uint32_t Battle::Unit::GetHitPointsTroop() const
{
    return hp;
}

int Battle::Unit::GetControl() const
{
    return Modes(SP_BERSERKER) || !GetArmy() ? CONTROL_AI : GetArmy()->GetControl();
}

bool Battle::Unit::isArchers() const
{
    return _monster.isArchers() && shots;
}

void Battle::Unit::SpellModesAction(const Spell& spell, uint32_t duration, const HeroBase* hero)
{
    if (hero)
    {
        uint32_t acount = hero->HasArtifact(Artifact::WIZARD_HAT);
        if (acount) duration += acount * Artifact(Artifact::WIZARD_HAT).ExtraValue();
        acount = hero->HasArtifact(Artifact::ENCHANTED_HOURGLASS);
        if (acount) duration += acount * Artifact(Artifact::ENCHANTED_HOURGLASS).ExtraValue();
    }

    switch (spell())
    {
    case Spell::BLESS:
    case Spell::MASSBLESS:
        if (Modes(SP_CURSE))
        {
            ResetModes(SP_CURSE);
            affected.RemoveMode(SP_CURSE);
        }
        SetModes(SP_BLESS);
        affected.AddMode(SP_BLESS, duration);
        ResetModes(LUCK_GOOD);
        break;

    case Spell::BLOODLUST:
        SetModes(SP_BLOODLUST);
        affected.AddMode(SP_BLOODLUST, 3);
        break;

    case Spell::CURSE:
    case Spell::MASSCURSE:
        if (Modes(SP_BLESS))
        {
            ResetModes(SP_BLESS);
            affected.RemoveMode(SP_BLESS);
        }
        SetModes(SP_CURSE);
        affected.AddMode(SP_CURSE, duration);
        ResetModes(LUCK_BAD);
        break;

    case Spell::HASTE:
    case Spell::MASSHASTE:
        if (Modes(SP_SLOW))
        {
            ResetModes(SP_SLOW);
            affected.RemoveMode(SP_SLOW);
        }
        SetModes(SP_HASTE);
        affected.AddMode(SP_HASTE, duration);
        break;

    case Spell::DISPEL:
    case Spell::MASSDISPEL:
        if (Modes(IS_MAGIC))
        {
            ResetModes(IS_MAGIC);
            affected.RemoveMode(IS_MAGIC);
        }
        break;

    case Spell::SHIELD:
    case Spell::MASSSHIELD:
        SetModes(SP_SHIELD);
        affected.AddMode(SP_SHIELD, duration);
        break;

    case Spell::SLOW:
    case Spell::MASSSLOW:
        if (Modes(SP_HASTE))
        {
            ResetModes(SP_HASTE);
            affected.RemoveMode(SP_HASTE);
        }
        SetModes(SP_SLOW);
        affected.AddMode(SP_SLOW, duration);
        break;

    case Spell::STONESKIN:
        if (Modes(SP_STEELSKIN))
        {
            ResetModes(SP_STEELSKIN);
            affected.RemoveMode(SP_STEELSKIN);
        }
        SetModes(SP_STONESKIN);
        affected.AddMode(SP_STONESKIN, duration);
        break;

    case Spell::BLIND:
        SetModes(SP_BLIND);
        blindanswer = false;
        affected.AddMode(SP_BLIND, duration);
        break;

    case Spell::DRAGONSLAYER:
        SetModes(SP_DRAGONSLAYER);
        affected.AddMode(SP_DRAGONSLAYER, duration);
        break;

    case Spell::STEELSKIN:
        if (Modes(SP_STONESKIN))
        {
            ResetModes(SP_STONESKIN);
            affected.RemoveMode(SP_STONESKIN);
        }
        SetModes(SP_STEELSKIN);
        affected.AddMode(SP_STEELSKIN, duration);
        break;

    case Spell::ANTIMAGIC:
        ResetModes(IS_MAGIC);
        SetModes(SP_ANTIMAGIC);
        affected.AddMode(SP_ANTIMAGIC, duration);
        break;

    case Spell::PARALYZE:
        SetModes(SP_PARALYZE);
        affected.AddMode(SP_PARALYZE, duration);
        break;

    case Spell::BERSERKER:
        SetModes(SP_BERSERKER);
        affected.AddMode(SP_BERSERKER, duration);
        break;

    case Spell::HYPNOTIZE:
        {
            SetModes(SP_HYPNOTIZE);
            uint32_t acount = hero ? hero->HasArtifact(Artifact::GOLD_WATCH) : 0;
            affected.AddMode(SP_HYPNOTIZE, acount ? duration * acount * 2 : duration);
        }
        break;

    case Spell::STONE:
        SetModes(SP_STONE);
        affected.AddMode(SP_STONE, duration);
        break;

    case Spell::MIRRORIMAGE:
        affected.AddMode(CAP_MIRRORIMAGE, duration);
        break;

    case Spell::DISRUPTINGRAY:
        ++disruptingray;
        break;

    default:
        break;
    }
}

void Battle::Unit::SpellApplyDamage(const Spell& spell, uint32_t spoint, const HeroBase* hero, TargetInfo& target)
{
    uint32_t dmg = spell.Damage() * spoint;

    switch (_monster.GetID())
    {
    case Monster::IRON_GOLEM:
    case Monster::STEEL_GOLEM:
        switch (spell())
        {
            // 50% damage
        case Spell::COLDRAY:
        case Spell::COLDRING:
        case Spell::FIREBALL:
        case Spell::FIREBLAST:
        case Spell::LIGHTNINGBOLT:
        case Spell::CHAINLIGHTNING:
        case Spell::ELEMENTALSTORM:
        case Spell::ARMAGEDDON:
            dmg /= 2;
            break;
        default:
            break;
        }
        break;

    case Monster::WATER_ELEMENT:
        switch (spell())
        {
            // 200% damage
        case Spell::FIREBALL:
        case Spell::FIREBLAST:
            dmg *= 2;
            break;
        default:
            break;
        }
        break;

    case Monster::AIR_ELEMENT:
        switch (spell())
        {
            // 200% damage
        case Spell::ELEMENTALSTORM:
        case Spell::LIGHTNINGBOLT:
        case Spell::CHAINLIGHTNING:
            dmg *= 2;
            break;
        default:
            break;
        }
        break;

    case Monster::FIRE_ELEMENT:
        switch (spell())
        {
            // 200% damage
        case Spell::COLDRAY:
        case Spell::COLDRING:
            dmg *= 2;
            break;
        default:
            break;
        }
        break;

    default:
        break;
    }

    // check artifact
    if (hero)
    {
        const HeroBase* myhero = GetCommander();
        uint32_t acount = 0;

        switch (spell())
        {
        case Spell::COLDRAY:
        case Spell::COLDRING:
            // +50%
            acount = hero->HasArtifact(Artifact::EVERCOLD_ICICLE);
            if (acount) dmg += dmg * acount * Artifact(Artifact::EVERCOLD_ICICLE).ExtraValue() / 100;
            acount = hero->HasArtifact(Artifact::EVERCOLD_ICICLE);
            if (acount) dmg += dmg * acount * Artifact(Artifact::EVERCOLD_ICICLE).ExtraValue() / 100;
            // -50%
            acount = myhero ? myhero->HasArtifact(Artifact::ICE_CLOAK) : 0;
            if (acount) dmg /= acount * 2;
            acount = myhero ? myhero->HasArtifact(Artifact::HEART_ICE) : 0;
            if (acount) dmg -= dmg * acount * Artifact(Artifact::HEART_ICE).ExtraValue() / 100;
            // 100%
            acount = myhero ? myhero->HasArtifact(Artifact::HEART_FIRE) : 0;
            if (acount) dmg *= acount * 2;
            break;

        case Spell::FIREBALL:
        case Spell::FIREBLAST:
            // +50%
            acount = hero->HasArtifact(Artifact::EVERHOT_LAVA_ROCK);
            if (acount) dmg += dmg * acount * Artifact(Artifact::EVERHOT_LAVA_ROCK).ExtraValue() / 100;
            // -50%
            acount = myhero ? myhero->HasArtifact(Artifact::FIRE_CLOAK) : 0;
            if (acount) dmg /= acount * 2;
            acount = myhero ? myhero->HasArtifact(Artifact::HEART_FIRE) : 0;
            if (acount) dmg -= dmg * acount * Artifact(Artifact::HEART_FIRE).ExtraValue() / 100;
            // 100%
            acount = myhero ? myhero->HasArtifact(Artifact::HEART_ICE) : 0;
            if (acount) dmg *= acount * 2;
            break;

        case Spell::LIGHTNINGBOLT:
            // +50%
            acount = hero->HasArtifact(Artifact::LIGHTNING_ROD);
            if (acount) dmg += dmg * acount * Artifact(Artifact::LIGHTNING_ROD).ExtraValue() / 100;
            // -50%
            acount = myhero ? myhero->HasArtifact(Artifact::LIGHTNING_HELM) : 0;
            if (acount) dmg /= acount * 2;
            break;

        case Spell::CHAINLIGHTNING:
            // +50%
            acount = hero->HasArtifact(Artifact::LIGHTNING_ROD);
            if (acount) dmg += acount * dmg / 2;
            // -50%
            acount = myhero ? myhero->HasArtifact(Artifact::LIGHTNING_HELM) : 0;
            if (acount) dmg /= acount * 2;
            // update orders damage
            switch (target.damage)
            {
            case 0:
                break;
            case 1:
                dmg /= 2;
                break;
            case 2:
                dmg /= 4;
                break;
            case 3:
                dmg /= 8;
                break;
            default:
                break;
            }
            break;

        case Spell::ELEMENTALSTORM:
        case Spell::ARMAGEDDON:
            // -50%
            acount = myhero ? myhero->HasArtifact(Artifact::BROACH_SHIELDING) : 0;
            if (acount) dmg /= acount * 2;
            break;

        default:
            break;
        }
    }

    // apply damage
    if (dmg)
    {
        target.damage = dmg;
        target.killed = ApplyDamage(dmg);
        if (target.defender && target.defender->Modes(SP_BLIND)) target.defender->ResetBlind();
    }
}

void Battle::Unit::SpellRestoreAction(const Spell& spell, uint32_t spoint, const HeroBase* hero)
{
    switch (spell())
    {
    case Spell::CURE:
    case Spell::MASSCURE:
        // clear bad magic
        if (Modes(IS_BAD_MAGIC))
        {
            ResetModes(IS_BAD_MAGIC);
            affected.RemoveMode(IS_BAD_MAGIC);
        }
        // restore
        hp += spell.Restore() * spoint;
        if (hp > ArmyTroop::GetHitPointsTroop()) hp = ArmyTroop::GetHitPointsTroop();
        break;

    case Spell::RESURRECT:
    case Spell::ANIMATEDEAD:
    case Spell::RESURRECTTRUE:
        {
            uint32_t restore = spell.Resurrect() * spoint;
            // remove from graveyard
            if (!isValid())
            {
                Arena::GetGraveyard()->RemoveTroop(*this);
                ResetAnimFrame(AS_IDLE);
            }
            // restore hp
            uint32_t acount = hero ? hero->HasArtifact(Artifact::ANKH) : 0;
            if (acount) restore *= acount * 2;

            const uint32_t resurrect = Resurrect(restore, false, spell == Spell::RESURRECT);

            if (Arena::GetInterface())
            {
                string str(_("%{count} %{name} rise(s) from the dead!"));
                StringReplace(str, "%{count}", resurrect);
                StringReplace(str, "%{name}", GetName());
                Arena::GetInterface()->SetStatus(str, true);
            }
        }
        break;

    default:
        break;
    }
}

bool Battle::Unit::isTwiceAttack() const
{
    switch (GetID())
    {
    case Monster::ELF:
    case Monster::GRAND_ELF:
    case Monster::RANGER:
        return !isHandFighting();

    default:
        break;
    }

    return _monster.isTwiceAttack();
}

bool Battle::Unit::isMagicResist(const Spell& spell, uint32_t spower) const
{
    return 100 <= GetMagicResist(spell, spower);
}

uint32_t Battle::Unit::GetMagicResist(const Spell& spell, uint32_t spower) const
{
    if (spell.isMindInfluence() &&
        (_monster.isUndead() || _monster.isElemental() ||
            GetID() == Monster::GIANT || GetID() == Monster::TITAN))
        return 100;

    if (spell.isALiveOnly() &&
        _monster.isUndead())
        return 100;

    if (spell.isUndeadOnly() &&
        !_monster.isUndead())
        return 100;

    if (Settings::Get().ExtBattleMagicTroopCanResist() && spell == GetSpellMagic(true))
        return 20;

    switch (GetID())
    {
    case Monster::ARCHMAGE:
        if (Settings::Get().ExtBattleArchmageCanResistBadMagic() &&
            (spell.isDamage() || spell.isApplyToEnemies()))
            return 20;
        break;

        // 25% unfortunatly
    case Monster::DWARF:
    case Monster::BATTLE_DWARF:
        if (spell.isDamage() || spell.isApplyToEnemies()) return 25;
        break;

    case Monster::GREEN_DRAGON:
    case Monster::RED_DRAGON:
    case Monster::BLACK_DRAGON:
        return 100;

    case Monster::PHOENIX:
        switch (spell())
        {
        case Spell::COLDRAY:
        case Spell::COLDRING:
        case Spell::FIREBALL:
        case Spell::FIREBLAST:
        case Spell::LIGHTNINGBOLT:
        case Spell::CHAINLIGHTNING:
        case Spell::ELEMENTALSTORM:
            return 100;
        default:
            break;
        }
        break;

    case Monster::CRUSADER:
        switch (spell())
        {
        case Spell::CURSE:
        case Spell::MASSCURSE:
            return 100;
        default:
            break;
        }
        break;

    case Monster::EARTH_ELEMENT:
        switch (spell())
        {
        case Spell::METEORSHOWER:
        case Spell::LIGHTNINGBOLT:
        case Spell::CHAINLIGHTNING:
            return 100;
        default:
            break;
        }
        break;

    case Monster::AIR_ELEMENT:
        switch (spell())
        {
        case Spell::METEORSHOWER:
            return 100;
        default:
            break;
        }
        break;

    case Monster::FIRE_ELEMENT:
        switch (spell())
        {
        case Spell::FIREBALL:
        case Spell::FIREBLAST:
            return 100;
        default:
            break;
        }
        break;

    case Monster::WATER_ELEMENT:
        switch (spell())
        {
        case Spell::COLDRAY:
        case Spell::COLDRING:
            return 100;
        default:
            break;
        }
        break;

    default:
        break;
    }

    switch (spell())
    {
    case Spell::CURE:
    case Spell::MASSCURE:
        if (!isHaveDamage() && !(modes & IS_MAGIC)) return 100;
        break;

    case Spell::RESURRECT:
    case Spell::RESURRECTTRUE:
    case Spell::ANIMATEDEAD:
        if (_monster.isElemental() || GetCount() == count0) return 100;
        break;

    case Spell::DISPEL:
        if (!(modes & IS_MAGIC)) return 100;
        break;

    case Spell::HYPNOTIZE:
        if (spell.ExtraValue() * spower < hp) return 100;
        break;

    default:
        break;
    }

    return 0;
}

bool Battle::Unit::isMagicAttack() const
{
    return GetSpellMagic(true) != Spell::NONE;
}


int Battle::Unit::GetSpellMagic(bool force) const
{
    switch (GetID())
    {
    case Monster::UNICORN:
        // 20% blind
        if (force || 3 > Rand::Get(1, 10)) return Spell::BLIND;
        break;

    case Monster::CYCLOPS:
        // 20% paralyze
        if (force || 3 > Rand::Get(1, 10)) return Spell::PARALYZE;
        break;

    case Monster::MUMMY:
        // 20% curse
        if (force || 3 > Rand::Get(1, 10)) return Spell::CURSE;
        break;

    case Monster::ROYAL_MUMMY:
        // 30% curse
        if (force || 4 > Rand::Get(1, 10)) return Spell::CURSE;
        break;

        /* skip: see Unit::PostAttackAction
case Monster::ARCHMAGE:
        // 20% dispel
        if(!force && 3 > Rand::Get(1, 10)) return Spell::DISPEL;
        break;
*/

    case Monster::MEDUSA:
        // 20% stone
        if (force || 3 > Rand::Get(1, 10)) return Spell::STONE;
        break;

    default:
        break;
    }

    return Spell::NONE;
}

bool Battle::Unit::isHaveDamage() const
{
    return hp < count0 * _monster.GetHitPoints();
}

int Battle::Unit::GetFrameStart() const
{
    return animstep < 0 ? GetFrameState().start + GetFrameState().count - 1 : GetFrameState().start;
}

int Battle::Unit::GetFrame() const
{
    return animframe;
}

void Battle::Unit::SetFrame(int val)
{
    animframe = val;
}

void Battle::Unit::SetFrameStep(int val)
{
    animstep = val;
}

int Battle::Unit::GetFrameOffset() const
{
    return animframe - GetFrameStart();
}

int Battle::Unit::GetFrameCount() const
{
    return GetFrameState().count;
}

void Battle::Unit::IncreaseAnimFrame(bool loop)
{
    if (!isFinishAnimFrame())
        animframe += animstep;
    else if (loop)
        animframe = GetFrameStart();
}

bool Battle::Unit::isStartAnimFrame() const
{
    return GetFrameStart() == animframe;
}

bool Battle::Unit::isFinishAnimFrame() const
{
    if (0 == GetFrameState().count)
        return true;
    if (animstep < 0)
        return animframe <= GetFrameState().start;
    if (animstep > 0)
        return animframe >= GetFrameState().start + GetFrameState().count - 1;

    return true;
}

const Battle::animframe_t& Battle::Unit::GetFrameState(int state) const
{
    const monstersprite_t& msi = GetMonsterSprite();

    switch (state)
    {
    case AS_IDLE:
        return msi.frm_idle;
    case AS_MOVE:
        return msi.frm_move;
    case AS_FLY1:
        return msi.frm_fly1;
    case AS_FLY2:
        return msi.frm_fly2;
    case AS_FLY3:
        return msi.frm_fly3;
    case AS_SHOT0:
        return msi.frm_shot0;
    case AS_SHOT1:
        return msi.frm_shot1;
    case AS_SHOT2:
        return msi.frm_shot2;
    case AS_SHOT3:
        return msi.frm_shot3;
    case AS_ATTK0:
        return msi.frm_attk0;
    case AS_ATTK1:
        return msi.frm_attk1;
    case AS_ATTK2:
        return msi.frm_attk2;
    case AS_ATTK3:
        return msi.frm_attk3;
    case AS_WNCE:
        return msi.frm_wnce;
    case AS_KILL:
        return msi.frm_kill;
    default:
        break;
    }

    return msi.frm_idle;
}

const Battle::animframe_t& Battle::Unit::GetFrameState() const
{
    return GetFrameState(animstate);
}

void Battle::Unit::ResetAnimFrame(int rule)
{
    animstep = 1;
    animstate = rule;
    animframe = GetFrameStart();

    if (AS_FLY3 == rule && 0 == GetFrameState().count)
    {
        animstep = -1;
        animstate = AS_FLY1;
        animframe = GetFrameStart();
    }
}

int Battle::Unit::M82Attk() const
{
    if (isArchers() && !isHandFighting())
    {
        switch (GetID())
        {
        case Monster::ARCHER:
        case Monster::RANGER:
            return M82::ARCHSHOT;
        case Monster::ORC:
        case Monster::ORC_CHIEF:
            return M82::ORC_SHOT;
        case Monster::TROLL:
        case Monster::WAR_TROLL:
            return M82::TRLLSHOT;
        case Monster::ELF:
        case Monster::GRAND_ELF:
            return M82::ELF_SHOT;
        case Monster::DRUID:
        case Monster::GREATER_DRUID:
            return M82::DRUISHOT;
        case Monster::CENTAUR:
            return M82::CNTRSHOT;
        case Monster::HALFLING:
            return M82::HALFSHOT;
        case Monster::MAGE:
        case Monster::ARCHMAGE:
            return M82::MAGESHOT;
        case Monster::TITAN:
            return M82::TITNSHOT;
        case Monster::LICH:
        case Monster::POWER_LICH:
            return M82::LICHSHOT;
        default:
            break;
        }
    }

    return GetMonsterSprite().m82_attk;
}

int Battle::Unit::M82Kill() const
{
    return GetMonsterSprite().m82_kill;
}

int Battle::Unit::M82Move() const
{
    return GetMonsterSprite().m82_move;
}

int Battle::Unit::M82Wnce() const
{
    return GetMonsterSprite().m82_wnce;
}

int Battle::Unit::M82Expl() const
{
    switch (GetID())
    {
    case Monster::VAMPIRE:
        return M82::VAMPEXT1;
    case Monster::VAMPIRE_LORD:
        return M82::VAMPEXT1;
    case Monster::LICH:
        return M82::LICHEXPL;
    case Monster::POWER_LICH:
        return M82::LICHEXPL;

    default:
        break;
    }

    return M82::UNKNOWN;
}

int Battle::Unit::ICNFile() const
{
    return GetMonsterSprite().icn_file;
}

int Battle::Unit::ICNMiss() const
{
    switch (GetID())
    {
    case Monster::ARCHER:
        return ICN::ARCH_MSL;
    case Monster::RANGER:
        return ICN::ARCH_MSL;
    case Monster::ORC:
        return ICN::ORC__MSL;
    case Monster::ORC_CHIEF:
        return ICN::ORC__MSL;
    case Monster::TROLL:
        return ICN::TROLLMSL;
    case Monster::WAR_TROLL:
        return ICN::TROLLMSL;
    case Monster::ELF:
        return ICN::ELF__MSL;
    case Monster::GRAND_ELF:
        return ICN::ELF__MSL;
    case Monster::DRUID:
        return ICN::DRUIDMSL;
    case Monster::GREATER_DRUID:
        return ICN::DRUIDMSL;
    case Monster::CENTAUR:
        return ICN::ARCH_MSL;
    case Monster::HALFLING:
        return ICN::HALFLMSL;
    case Monster::MAGE:
        return ICN::DRUIDMSL;
    case Monster::ARCHMAGE:
        return ICN::DRUIDMSL;
    case Monster::TITAN:
        return ICN::TITANMSL;
    case Monster::LICH:
        return ICN::LICH_MSL;
    case Monster::POWER_LICH:
        return ICN::LICH_MSL;

    default:
        break;
    }

    return ICN::UNKNOWN;
}

Rect Battle::Unit::GetRectPosition() const
{
    return position.GetRect();
}

Point Battle::Unit::GetBackPoint() const
{
    const Rect& rt = position.GetRect();
    return reflect ? Point(rt.x + rt.w, rt.y + rt.h / 2) : Point(rt.x, rt.y + rt.h / 2);
}

int Battle::Unit::GetStartMissileOffset(int state) const
{
    switch (GetID())
    {
    case Monster::ARCHER:
    case Monster::RANGER:
        switch (state)
        {
        case AS_SHOT1:
            return -15;
        case AS_SHOT2:
            return -3;
        case AS_SHOT3:
            return 10;
        default:
            break;
        }
        break;

    case Monster::ORC:
    case Monster::ORC_CHIEF:
        return 5;

    case Monster::TROLL:
    case Monster::WAR_TROLL:
        return -20;

    case Monster::LICH:
    case Monster::POWER_LICH:
        switch (state)
        {
        case AS_SHOT1:
            return -30;
        case AS_SHOT2:
            return -20;
        case AS_SHOT3:
            return 0;
        default:
            break;
        }
        break;

    case Monster::ELF:
    case Monster::GRAND_ELF:
        switch (state)
        {
        case AS_SHOT1:
            return -5;
        case AS_SHOT2:
            return 0;
        case AS_SHOT3:
            return 5;
        default:
            break;
        }
        break;

    case Monster::CENTAUR:
        switch (state)
        {
        case AS_SHOT1:
            return -20;
        case AS_SHOT2:
            return -10;
        case AS_SHOT3:
            return 5;
        default:
            break;
        }
        break;

    case Monster::DRUID:
    case Monster::GREATER_DRUID:
        switch (state)
        {
        case AS_SHOT1:
            return -20;
        case AS_SHOT2:
            return -5;
        case AS_SHOT3:
            return 15;
        default:
            break;
        }
        break;

    case Monster::HALFLING:
        switch (state)
        {
        case AS_SHOT1:
            return -20;
        case AS_SHOT2:
            return 10;
        case AS_SHOT3:
            return 20;
        default:
            break;
        }
        break;

    case Monster::MAGE:
    case Monster::ARCHMAGE:
        switch (state)
        {
        case AS_SHOT1:
            return -40;
        case AS_SHOT2:
            return -10;
        case AS_SHOT3:
            return 25;
        default:
            break;
        }
        break;

    case Monster::TITAN:
        switch (state)
        {
        case AS_SHOT1:
            return -80;
        case AS_SHOT2:
            return -20;
        case AS_SHOT3:
            return 15;
        default:
            break;
        }
        break;

    default:
        break;
    }

    return 0;
}

int Battle::Unit::GetArmyColor() const
{
    return ArmyTroop::GetColor();
}

int Battle::Unit::GetColor() const
{
    if (Modes(SP_BERSERKER))
        return 0;
    if (Modes(SP_HYPNOTIZE))
        return GetArena()->GetOppositeColor(GetArmyColor());

    // default
    return GetArmyColor();
}

const HeroBase* Battle::Unit::GetCommander() const
{
    return GetArmy() ? GetArmy()->GetCommander() : nullptr;
}
