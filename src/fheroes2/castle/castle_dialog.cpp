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

#include <string>
#include "agg.h"
#include "button.h"
#include "world.h"
#include "cursor.h"
#include "settings.h"
#include "game.h"
#include "race.h"
#include "text.h"
#include "dialog.h"
#include "statusbar.h"
#include "army_bar.h"
#include "m82.h"
#include "mus.h"
#include "bitmodes.h"
#include "icn.h"

void CastleRedrawTownName(const Castle& castle, const Point& dst);

bool AllowFlashBuilding(uint32_t build)
{
    switch (build)
    {
    case BUILD_TAVERN:
    case BUILD_SHRINE:
    case BUILD_SHIPYARD:
    case BUILD_WELL:
    case BUILD_STATUE:
    case BUILD_LEFTTURRET:
    case BUILD_RIGHTTURRET:
    case BUILD_MARKETPLACE:
    case BUILD_WEL2:
    case BUILD_MOAT:
    case BUILD_SPEC:
    case BUILD_CASTLE:
    case BUILD_CAPTAIN:
    case BUILD_MAGEGUILD1:
    case BUILD_MAGEGUILD2:
    case BUILD_MAGEGUILD3:
    case BUILD_MAGEGUILD4:
    case BUILD_MAGEGUILD5:
    case BUILD_TENT:
    case DWELLING_UPGRADE2:
    case DWELLING_UPGRADE3:
    case DWELLING_UPGRADE4:
    case DWELLING_UPGRADE5:
    case DWELLING_UPGRADE6:
    case DWELLING_UPGRADE7:
    case DWELLING_MONSTER1:
    case DWELLING_MONSTER2:
    case DWELLING_MONSTER3:
    case DWELLING_MONSTER4:
    case DWELLING_MONSTER5:
    case DWELLING_MONSTER6:
        return true;

    default:
        break;
    }

    return false;
}

Sprite GetActualSpriteBuilding(const Castle& castle, uint32_t build)
{
    uint32_t index = 0;
    // correct index (mage guild)
    switch (build)
    {
    case BUILD_MAGEGUILD1:
        index = 0;
        break;
    case BUILD_MAGEGUILD2:
        index = Race::NECR == castle.GetRace() ? 6 : 1;
        break;
    case BUILD_MAGEGUILD3:
        index = Race::NECR == castle.GetRace() ? 12 : 2;
        break;
    case BUILD_MAGEGUILD4:
        index = Race::NECR == castle.GetRace() ? 18 : 3;
        break;
    case BUILD_MAGEGUILD5:
        index = Race::NECR == castle.GetRace() ? 24 : 4;
        break;
    default:
        break;
    }

    return AGG::GetICN(Castle::GetICNBuilding(build, castle.GetRace()), index);
}

building_t GetCurrentFlash(const Castle& castle, CastleDialog::CacheBuildings& cache)
{
    LocalEvent& le = LocalEvent::Get();
    auto it = cache._items.begin();
    building_t flash = BUILD_NOTHING;


    for (auto& _item : cache._items)
    {
        if (castle.isBuild(_item.id) && _item.coord & le.GetMouseCursor() &&
            AllowFlashBuilding(_item.id))
        {
            if (_item.id & BUILD_MAGEGUILD)
            {
                uint32_t lvl = castle.GetLevelMageGuild();

                if ((_item.id == BUILD_MAGEGUILD1 && lvl > 1) ||
                    (_item.id == BUILD_MAGEGUILD2 && lvl > 2) ||
                    (_item.id == BUILD_MAGEGUILD3 && lvl > 3) ||
                    (_item.id == BUILD_MAGEGUILD4 && lvl > 4))
                    continue;
            }
            break;
        }
    }

    if (it != cache._items.end())
    {
        flash = (*it).id;

        if (!(*it).contour.isValid())
        {
            const Sprite& sprite = GetActualSpriteBuilding(castle, flash);
            (*it).contour = Sprite(sprite.RenderContour(RGBA(0xe0, 0xe0, 0)), sprite.x() - 1, sprite.y() - 1);
        }
    }

    return flash;
}

void RedrawIcons(const Castle& castle, const CastleHeroes& heroes, const Point& pt)
{
    Display& display = Display::Get();

    const Heroes* hero1 = heroes.Guard();
    const Heroes* hero2 = heroes.Guest();

    AGG::GetICN(ICN::STRIP, 0).Blit(pt.x, pt.y + 256);
    Surface icon1, icon2;
    if (hero1)
        icon1 = hero1->GetPortrait(PORT_BIG);
    else if (castle.isBuild(BUILD_CAPTAIN))
        icon1 = castle.GetCaptain().GetPortrait(PORT_BIG);
    else
        icon1 = AGG::GetICN(ICN::CREST, Color::GetIndex(castle.GetColor()));
    if (hero2)
        icon2 = hero2->GetPortrait(PORT_BIG);
    else
        icon2 = AGG::GetICN(ICN::STRIP, 3);
    if (icon1.isValid())
        icon1.Blit(pt.x + 5, pt.y + 262, display);
    if (icon2.isValid())
        icon2.Blit(pt.x + 5, pt.y + 361, display);
    if (!hero2)
        AGG::GetICN(ICN::STRIP, 11).Blit(pt.x + 112, pt.y + 361);
}

Surface GetMeetingSprite()
{
    const Sprite& sprite = AGG::GetICN(ICN::ADVMCO, 8);

    Surface res(sprite.GetSize() + Size(4, 4), false);
    res.Fill(ColorBlack);
    res.DrawBorder(RGBA(0xe0, 0xb4, 0));
    sprite.Blit(2, 2, res);

    return res;
}

MeetingButton::MeetingButton(s32 px, s32 py)
{
    sf = GetMeetingSprite();

    SetPos(px, py);
    SetSize(sf.w(), sf.h());
    SetSprite(sf, sf);
}

SwapButton::SwapButton(s32 px, s32 py)
{
    sf = GetMeetingSprite().RenderRotate(1);

    SetPos(px, py);
    SetSize(sf.w(), sf.h());
    SetSprite(sf, sf);
}

int Castle::OpenDialog(bool readonly, bool fade)
{
    Settings& conf = Settings::Get();

    const bool interface = conf.ExtGameEvilInterface();
    if (conf.ExtGameDynamicInterface())
        conf.SetEvilInterface(GetRace() & (Race::BARB | Race::WRLK | Race::NECR));

    Display& display = Display::Get();

    CastleHeroes heroes = world.GetHeroes(*this);

    // cursor
    Cursor& cursor = Cursor::Get();

    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);

    // fade
    if (conf.ExtGameUseFade()) display.Fade();

    Dialog::FrameBorder background(Size(640, 480));

    const Point& cur_pt = background.GetArea();
    Point dst_pt(cur_pt);
    string msg_status;

    // date string
    string msg_date = _("Month: %{month}, Week: %{week}, Day: %{day}");
    StringReplace(msg_date, "%{month}", world.GetMonth());
    StringReplace(msg_date, "%{week}", world.GetWeek());
    StringReplace(msg_date, "%{day}", world.GetDay());

    // button prev castle
    dst_pt.y += 480 - 19;
    Button buttonPrevCastle(dst_pt.x, dst_pt.y, ICN::SMALLBAR, 1, 2);

    // bottom small bar
    const Sprite& bar = AGG::GetICN(ICN::SMALLBAR, 0);
    dst_pt.x += buttonPrevCastle.w;
    bar.Blit(dst_pt);

    StatusBar statusBar;
    statusBar.SetFont(Font::BIG);
    statusBar.SetCenter(dst_pt.x + bar.w() / 2, dst_pt.y + 11);

    // button next castle
    dst_pt.x += bar.w();
    Button buttonNextCastle(dst_pt.x, dst_pt.y, ICN::SMALLBAR, 3, 4);

    // color crest
    const Sprite& crest = AGG::GetICN(ICN::CREST, Color::GetIndex(GetColor()));
    dst_pt.x = cur_pt.x + 5;
    dst_pt.y = cur_pt.y + 262;
    const Rect rectSign1(dst_pt, crest.w(), crest.h());

    RedrawIcons(*this, heroes, cur_pt);

    // castle troops selector
    dst_pt.x = cur_pt.x + 112;
    dst_pt.y = cur_pt.y + 262;

    // castle army bar
    ArmyBar selectArmy1((heroes.Guard() ? &heroes.Guard()->GetArmy() : &army), false, readonly);
    selectArmy1.SetColRows(5, 1);
    selectArmy1.SetPos(dst_pt.x, dst_pt.y);
    selectArmy1.SetHSpace(6);
    selectArmy1.Redraw();

    // portrait heroes or captain or sign
    dst_pt.x = cur_pt.x + 5;
    dst_pt.y = cur_pt.y + 361;

    const Rect rectSign2(dst_pt.x, dst_pt.y, 100, 92);

    // castle_heroes troops background
    dst_pt.x = cur_pt.x + 112;
    dst_pt.y = cur_pt.y + 361;

    ArmyBar selectArmy2(nullptr, false, readonly);
    selectArmy2.SetColRows(5, 1);
    selectArmy2.SetPos(dst_pt.x, dst_pt.y);
    selectArmy2.SetHSpace(6);

    if (heroes.Guest())
    {
        heroes.Guest()->MovePointsScaleFixed();
        selectArmy2.SetArmy(&heroes.Guest()->GetArmy());
        selectArmy2.Redraw();
    }

    // resource
    const Rect rectResource = RedrawResourcePanel(cur_pt);

    // button swap
    SwapButton buttonSwap(cur_pt.x + 4, cur_pt.y + 345);
    MeetingButton buttonMeeting(cur_pt.x + 88, cur_pt.y + 345);

    if (heroes.Guest() && heroes.Guard() && !readonly)
    {
        buttonSwap.Draw();
        buttonMeeting.Draw();
    }

    // button exit
    dst_pt.x = cur_pt.x + 553;
    dst_pt.y = cur_pt.y + 428;
    Button buttonExit(dst_pt.x, dst_pt.y, ICN::SWAPBTN, 0, 1);

    // fill cache buildings
    CastleDialog::CacheBuildings cacheBuildings(*this, cur_pt);

    // draw building
    RedrawAllBuilding(*this, cur_pt, cacheBuildings);

    if (2 > world.GetKingdom(GetColor()).GetCastles()._items.size() || readonly)
    {
        buttonPrevCastle.Press();
        buttonPrevCastle.SetDisable(true);

        buttonNextCastle.Press();
        buttonNextCastle.SetDisable(true);
    }

    buttonPrevCastle.Draw();
    buttonNextCastle.Draw();
    buttonExit.Draw();

    AGG::PlayMusic(MUS::FromRace(race));

    LocalEvent& le = LocalEvent::Get();
    cursor.Show();
    display.Flip();

    int result = Dialog::ZERO;
    bool need_redraw = false;

    // dialog menu loop
    while (le.HandleEvents())
    {
        // exit
        if (le.MouseClickLeft(buttonExit) || HotKeyPressEvent(Game::EVENT_DEFAULT_EXIT))
        {
            result = Dialog::CANCEL;
            break;
        }

        if (buttonPrevCastle.isEnable())
            le.MousePressLeft(buttonPrevCastle) ? buttonPrevCastle.PressDraw() : buttonPrevCastle.ReleaseDraw();
        if (buttonNextCastle.isEnable())
            le.MousePressLeft(buttonNextCastle) ? buttonNextCastle.PressDraw() : buttonNextCastle.ReleaseDraw();

        le.MousePressLeft(buttonExit) ? buttonExit.PressDraw() : buttonExit.ReleaseDraw();

        if (le.MouseClickLeft(rectResource))
            ResourceInfo("", _("Income:"), world.GetKingdom(GetColor()).GetIncome(INCOME_ALL), Dialog::OK);
        else if (le.MousePressRight(rectResource))
            Dialog::ResourceInfo("", _("Income:"), world.GetKingdom(GetColor()).GetIncome(INCOME_ALL), 0);

        // selector troops event
        if ((selectArmy2.isValid() &&
            ((le.MouseCursor(selectArmy1.GetArea()) && selectArmy1.QueueEventProcessing(selectArmy2, &msg_status)) ||
                (le.MouseCursor(selectArmy2.GetArea()) && selectArmy2.QueueEventProcessing(selectArmy1, &msg_status)))) ||
            (!selectArmy2.isValid() && le.MouseCursor(selectArmy1.GetArea()) &&
            selectArmy1.QueueEventProcessing(&msg_status)))
        {
            cursor.Hide();
            need_redraw = true;
        }

        if (conf.ExtCastleAllowGuardians() && !readonly)
        {
            Army* army1 = nullptr;
            Army* army2 = nullptr;

            // swap guest <-> guardian
            if (heroes.Guest() && heroes.Guard())
            {
                if (le.MouseClickLeft(buttonSwap))
                {
                    SwapCastleHeroes(heroes);
                    army1 = &heroes.Guard()->GetArmy();
                    army2 = &heroes.Guest()->GetArmy();
                }
                else if (le.MouseClickLeft(buttonMeeting))
                {
                    heroes.Guest()->MeetingDialog(*heroes.Guard());
                    need_redraw = true;
                }
            }
            else
                // move hero to guardian
                if (heroes.Guest() && !heroes.Guard() && le.MouseClickLeft(rectSign1))
                {
                    if (!heroes.Guest()->GetArmy().m_troops.CanJoinTroops(army.m_troops))
                    {
                        // FIXME: correct message
                        Message(_("Join Error"), _("Army is full"), Font::BIG, Dialog::OK);
                    }
                    else
                    {
                        SwapCastleHeroes(heroes);
                        army1 = &heroes.Guard()->GetArmy();
                    }
                }
                else
                    // move guardian to hero
                    if (!heroes.Guest() && heroes.Guard() && le.MouseClickLeft(rectSign2))
                    {
                        SwapCastleHeroes(heroes);
                        army2 = &heroes.Guest()->GetArmy();
                    }

            if (army1 || army2)
            {
                cursor.Hide();
                if (selectArmy1.isSelected()) selectArmy1.ResetSelected();
                if (selectArmy2.isValid() && selectArmy2.isSelected()) selectArmy2.ResetSelected();

                if (army1 && army2)
                {
                    selectArmy1.SetArmy(army1);
                    selectArmy2.SetArmy(army2);
                }
                else if (army1)
                {
                    selectArmy1.SetArmy(army1);
                    selectArmy2.SetArmy(nullptr);
                }
                else if (army2)
                {
                    selectArmy1.SetArmy(&army);
                    selectArmy2.SetArmy(army2);
                }

                RedrawIcons(*this, heroes, cur_pt);
                need_redraw = true;
            }
        }

        // view guardian
        if (!readonly && heroes.Guard() && le.MouseClickLeft(rectSign1))
        {
            Game::DisableChangeMusic(true);
            Game::OpenHeroesDialog(*heroes.Guard());

            if (selectArmy1.isSelected()) selectArmy1.ResetSelected();
            if (selectArmy2.isValid() && selectArmy2.isSelected()) selectArmy2.ResetSelected();

            need_redraw = true;
        }
        else
            // view hero
            if (!readonly && heroes.Guest() && le.MouseClickLeft(rectSign2))
            {
                Game::DisableChangeMusic(true);
                Game::OpenHeroesDialog(*heroes.Guest());

                if (selectArmy1.isSelected()) selectArmy1.ResetSelected();
                if (selectArmy2.isValid() && selectArmy2.isSelected()) selectArmy2.ResetSelected();

                need_redraw = true;
            }

        // prev castle
        if (buttonPrevCastle.isEnable() && le.MouseClickLeft(buttonPrevCastle))
        {
            result = Dialog::PREV;
            break;
        }
        // next castle
        if (buttonNextCastle.isEnable() && le.MouseClickLeft(buttonNextCastle))
        {
            result = Dialog::NEXT;
            break;
        }

        // buildings event

        for (auto it = cacheBuildings._items.rbegin(); it != cacheBuildings._items.rend(); ++it)
        {
            if ((*it).id != GetActualDwelling((*it).id) || !isBuild((*it).id))
                continue;
            if (!readonly && le.MouseClickLeft((*it).coord) &&
                RecruitMonster(Dialog::RecruitMonster(
                    Monster(race, GetActualDwelling((*it).id)), GetDwellingLivedCount((*it).id), true)))
                need_redraw = true;
            else if (le.MousePressRight((*it).coord))
                Dialog::DwellingInfo(Monster(race, GetActualDwelling((*it).id)), GetDwellingLivedCount((*it).id));

            if (le.MouseCursor((*it).coord))
                msg_status = Monster(race, (*it).id).GetName();
        }

        for (auto it = cacheBuildings._items.begin(); it != cacheBuildings._items.end(); ++it)
        {
            if (BUILD_MAGEGUILD & (*it).id)
            {
                for (uint32_t id = BUILD_MAGEGUILD5; id >= BUILD_MAGEGUILD1; id >>= 1)
                    if (isBuild(id) && id == (*it).id)
                    {
                        if (le.MouseClickLeft((*it).coord))
                        {
                            if (heroes.Guard() && !heroes.Guard()->HaveSpellBook() &&
                                heroes.Guard()->BuySpellBook(this))
                                need_redraw = true;

                            if (heroes.Guest() && !heroes.Guest()->HaveSpellBook() &&
                                heroes.Guest()->BuySpellBook(this))
                                need_redraw = true;

                            OpenMageGuild();
                        }
                        else if (le.MousePressRight((*it).coord))
                            Dialog::Message(GetStringBuilding((*it).id), GetDescriptionBuilding((*it).id), Font::BIG);

                        if (le.MouseCursor((*it).coord))
                            msg_status = GetStringBuilding((*it).id);
                    }
            }
            else if (isBuild((*it).id))
            {
                if (le.MouseClickLeft((*it).coord))
                {
                    if (selectArmy1.isSelected()) selectArmy1.ResetSelected();
                    if (selectArmy2.isValid() && selectArmy2.isSelected()) selectArmy2.ResetSelected();

                    if (readonly &&
                        (*it).id & (BUILD_SHIPYARD | BUILD_MARKETPLACE | BUILD_WELL | BUILD_TENT | BUILD_CASTLE))
                        Message(GetStringBuilding((*it).id), GetDescriptionBuilding((*it).id), Font::BIG,
                                Dialog::OK);
                    else
                        switch ((*it).id)
                        {
                        case BUILD_THIEVESGUILD:
                            Dialog::ThievesGuild(false);
                            break;

                        case BUILD_TAVERN:
                            OpenTavern();
                            break;

                        case BUILD_CAPTAIN:
                        case BUILD_STATUE:
                        case BUILD_WEL2:
                        case BUILD_MOAT:
                        case BUILD_SPEC:
                        case BUILD_SHRINE:
                            Message(GetStringBuilding((*it).id), GetDescriptionBuilding((*it).id),
                                    Font::BIG, Dialog::OK);
                            break;

                        case BUILD_SHIPYARD:
                            if (Dialog::OK == Dialog::BuyBoat(AllowBuyBoat()))
                            {
                                BuyBoat();
                                need_redraw = true;
                            }
                            break;

                        case BUILD_MARKETPLACE:
                            Dialog::Marketplace();
                            need_redraw = true;
                            break;

                        case BUILD_WELL:
                            OpenWell();
                            need_redraw = true;
                            break;

                        case BUILD_TENT:
                            if (!Modes(ALLOWCASTLE))
                                Message(_("Town"), _("This town may not be upgraded to a castle."),
                                        Font::BIG, Dialog::OK);
                            else if (Dialog::OK == DialogBuyCastle(true))
                            {
                                AGG::PlaySound(M82::BUILDTWN);

                                RedrawAnimationBuilding(*this, cur_pt, cacheBuildings, BUILD_CASTLE);
                                BuyBuilding(BUILD_CASTLE);

                                need_redraw = true;
                            }
                            break;

                        case BUILD_CASTLE:
                            {
                                const Heroes* prev = heroes.Guest();
                                const uint32_t build = OpenTown();
                                heroes = world.GetHeroes(*this);
                                bool buyhero = heroes.Guest() && heroes.Guest() != prev;

                                if (BUILD_NOTHING != build)
                                {
                                    AGG::PlaySound(M82::BUILDTWN);

                                    RedrawAnimationBuilding(*this, cur_pt, cacheBuildings, build);
                                    BuyBuilding(build);

                                    if (BUILD_CAPTAIN == build)
                                        RedrawIcons(*this, heroes, cur_pt);

                                    need_redraw = true;
                                }

                                if (buyhero)
                                {
                                    if (prev)
                                    {
                                        selectArmy1.SetArmy(&heroes.Guard()->GetArmy());
                                        selectArmy2.SetArmy(nullptr);
                                        cursor.Hide();
                                        RedrawIcons(*this, CastleHeroes(nullptr, heroes.Guard()), cur_pt);
                                        selectArmy1.Redraw();
                                        if (selectArmy2.isValid()) selectArmy2.Redraw();
                                        cursor.Show();
                                        display.Flip();
                                    }
                                    selectArmy2.SetArmy(&heroes.Guest()->GetArmy());
                                    AGG::PlaySound(M82::BUILDTWN);

                                    // animate fade in for hero army bar
                                    const Rect rt(0, 98, 552, 107);
                                    Surface sf(rt, false);
                                    AGG::GetICN(ICN::STRIP, 0).Blit(rt, 0, 0, sf);
                                    Surface port = heroes.Guest()->GetPortrait(PORT_BIG);
                                    if (port.isValid()) port.Blit(6, 6, sf);
                                    const Point savept = selectArmy2.GetPos();
                                    selectArmy2.SetPos(112, 5);
                                    selectArmy2.Redraw(sf);
                                    selectArmy2.SetPos(savept.x, savept.y);

                                    RedrawResourcePanel(cur_pt);

                                    int alpha = 0;
                                    while (le.HandleEvents() && alpha < 240)
                                    {
                                        if (AnimateInfrequentDelay(Game::CASTLE_BUYHERO_DELAY))
                                        {
                                            cursor.Hide();
                                            sf.SetAlphaMod(alpha);
                                            sf.Blit(cur_pt.x, cur_pt.y + 356, display);
                                            cursor.Show();
                                            display.Flip();
                                            alpha += 10;
                                        }
                                    }

                                    RedrawIcons(*this, heroes, cur_pt);
                                    need_redraw = true;
                                }
                            }
                            break;

                        default:
                            break;
                        }
                }
                else if (le.MousePressRight((*it).coord))
                    Dialog::Message(GetStringBuilding((*it).id), GetDescriptionBuilding((*it).id), Font::BIG);

                if (le.MouseCursor((*it).coord))
                    msg_status = GetStringBuilding((*it).id);
            }
        }


        if (need_redraw)
        {
            cursor.Hide();
            selectArmy1.Redraw();
            if (selectArmy2.isValid()) selectArmy2.Redraw();
            CastleRedrawTownName(*this, cur_pt);
            RedrawResourcePanel(cur_pt);
            if (heroes.Guest() && heroes.Guard() && !readonly)
            {
                buttonSwap.Draw();
                buttonMeeting.Draw();
            }
            if (buttonExit.isPressed()) buttonExit.Draw();
            cursor.Show();
            display.Flip();
            need_redraw = false;
        }

        // status message exit
        if (le.MouseCursor(buttonExit))
            msg_status = isCastle() ? _("Exit castle") : _("Exit town");
        else if (le.MouseCursor(rectResource))
            msg_status = _("Show income");
        else
            // status message prev castle
            if (le.MouseCursor(buttonPrevCastle))
                msg_status = _("Show previous town");
            else
                // status message next castle
                if (le.MouseCursor(buttonNextCastle))
                    msg_status = _("Show next town");
                else if (heroes.Guest() && heroes.Guard() && le.MouseCursor(buttonSwap))
                    msg_status = _("Swap Heroes");
                else if (heroes.Guest() && heroes.Guard() && le.MouseCursor(buttonMeeting))
                    msg_status = _("Meeting Heroes");
                else
                    // status message over sign
                    if ((heroes.Guard() && le.MouseCursor(rectSign1)) ||
                        (heroes.Guest() && le.MouseCursor(rectSign2)))
                        msg_status = _("View Hero");

        if (msg_status.empty())
            statusBar.ShowMessage(msg_date);
        else
        {
            statusBar.ShowMessage(msg_status);
            msg_status.clear();
        }

        // animation sprite
        if (AnimateInfrequentDelay(Game::CASTLE_AROUND_DELAY))
        {
            cursor.Hide();
            RedrawAllBuilding(*this, cur_pt, cacheBuildings,
                              conf.ExtCastleAllowFlash()
                                  ? GetCurrentFlash(*this, cacheBuildings)
                                  : BUILD_NOTHING);
            cursor.Show();
            display.Flip();
        }
    }

    if (heroes.Guest() && conf.ExtHeroRecalculateMovement())
        heroes.Guest()->RecalculateMovePoints();

    if (conf.ExtGameDynamicInterface())
        conf.SetEvilInterface(interface);

    Game::DisableChangeMusic(false);

    return result;
}

void Castle::DrawCastleFundsAtPos(Display& display, const Funds& resource, Rect src_rt)
{
    display.FillRect(src_rt, ColorBlack);

    Text text;

    Point dst_pt;
    // sprite wood
    dst_pt.x = src_rt.x + 1;
    dst_pt.y = src_rt.y + 10;
    const Sprite& wood = AGG::GetICN(ICN::RESOURCE, 0);
    wood.Blit(dst_pt);

    // count wood
    text.Set(Int2Str(resource.wood), Font::SMALL);
    dst_pt.y += 22;
    text.Blit(dst_pt.x + (wood.w() - text.w()) / 2, dst_pt.y);

    // sprite sulfur
    dst_pt.x = src_rt.x + 42;
    dst_pt.y = src_rt.y + 6;
    const Sprite& sulfur = AGG::GetICN(ICN::RESOURCE, 3);
    sulfur.Blit(dst_pt);

    // count sulfur
    text.Set(Int2Str(resource.sulfur));
    dst_pt.y += 26;
    text.Blit(dst_pt.x + (sulfur.w() - text.w()) / 2, dst_pt.y);

    // sprite crystal
    dst_pt.x = src_rt.x + 1;
    dst_pt.y = src_rt.y + 45;
    const Sprite& crystal = AGG::GetICN(ICN::RESOURCE, 4);
    crystal.Blit(dst_pt);

    // count crystal
    text.Set(Int2Str(resource.crystal));
    dst_pt.y += 33;
    text.Blit(dst_pt.x + (crystal.w() - text.w()) / 2, dst_pt.y);

    // sprite mercury
    dst_pt.x = src_rt.x + 44;
    dst_pt.y = src_rt.y + 47;
    const Sprite& mercury = AGG::GetICN(ICN::RESOURCE, 1);
    mercury.Blit(dst_pt);

    // count mercury
    text.Set(Int2Str(resource.mercury));
    dst_pt.y += 34;
    text.Blit(dst_pt.x + (mercury.w() - text.w()) / 2, dst_pt.y);

    // sprite ore
    dst_pt.x = src_rt.x + 1;
    dst_pt.y = src_rt.y + 92;
    const Sprite& ore = AGG::GetICN(ICN::RESOURCE, 2);
    ore.Blit(dst_pt);

    // count ore
    text.Set(Int2Str(resource.ore));
    dst_pt.y += 26;
    text.Blit(dst_pt.x + (ore.w() - text.w()) / 2, dst_pt.y);

    // sprite gems
    dst_pt.x = src_rt.x + 45;
    dst_pt.y = src_rt.y + 92;
    const Sprite& gems = AGG::GetICN(ICN::RESOURCE, 5);
    gems.Blit(dst_pt);

    // count gems
    text.Set(Int2Str(resource.gems));
    dst_pt.y += 26;
    text.Blit(dst_pt.x + (gems.w() - text.w()) / 2, dst_pt.y);

    // sprite gold
    dst_pt.x = src_rt.x + 6;
    dst_pt.y = src_rt.y + 130;
    const Sprite& gold = AGG::GetICN(ICN::RESOURCE, 6);
    gold.Blit(dst_pt);

    // count gold
    text.Set(Int2Str(resource.gold));
    dst_pt.y += 24;
    text.Blit(dst_pt.x + (gold.w() - text.w()) / 2, dst_pt.y);

    // sprite button exit
    dst_pt.x = src_rt.x + 1;
    dst_pt.y = src_rt.y + 166;
    const Sprite& exit = AGG::GetICN(ICN::SWAPBTN, 0);
    exit.Blit(dst_pt);
}

/* redraw resource info panel */
Rect Castle::RedrawResourcePanel(const Point& pt) const
{
    Display& display = Display::Get();
    const Funds& resource = world.GetKingdom(GetColor()).GetFunds();

    Rect src_rt(pt.x + 552, pt.y + 262, 82, 192);

    DrawCastleFundsAtPos(display, resource, src_rt);

    return src_rt;
}
