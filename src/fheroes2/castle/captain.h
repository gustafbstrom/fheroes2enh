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

#include "army.h"
#include "heroes_base.h"

class Captain : public HeroBase
{
public:
    explicit Captain(Castle&);

    bool isValid() const override;

    int GetAttack() const override;

    int GetDefense() const override;

    int GetPower() const override;

    int GetKnowledge() const override;

    int GetMorale() const override;

    int GetLuck() const override;

    int GetRace() const override;

    int GetColor() const override;

    int GetType() const override;

    int GetControl() const override;

    s32 GetIndex() const;

    const string& GetName() const override;

    const Castle* inCastle() const override;

    int GetLevelSkill(Skill::SkillT) const override;

    uint32_t GetSecondaryValues(Skill::SkillT) const override;

    const Army& GetArmy() const override;

    Army& GetArmy() override;

    uint32_t GetMaxSpellPoints() const override;

    void ActionPreBattle() override;

    void ActionAfterBattle() override;

    void PortraitRedraw(s32, s32, int type, Surface&) const override;

    Surface GetPortrait(int type) const;

private:
    Castle& home;
};
