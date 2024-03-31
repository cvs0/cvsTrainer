/*
	THIS FILE IS A PART OF RDR 2 SCRIPT HOOK SDK
				http://dev-c.com
			(C) Alexander Blade 2019
*/

#include "scriptmenu.h"
#include <stdexcept>

void DrawText(float x, float y, char *str)
{
	UI::DRAW_TEXT(GAMEPLAY::CREATE_STRING(10, "LITERAL_STRING", str), x, y);
}

void DrawRect(float lineLeft, float lineTop, float lineWidth, float lineHeight, int r, int g, int b, int a)
{
	GRAPHICS::DRAW_RECT((lineLeft + (lineWidth * 0.5f)), (lineTop + (lineHeight * 0.5f)), lineWidth, lineHeight, r, g, b, a, 0, 0);
}

void MenuItemBase::WaitAndDraw(int ms)
{
	if (ms <= 0)
	{
		// Invalid input, throw an exception or handle the error accordingly
		throw std::invalid_argument("ms must be a positive value");
	}

	DWORD time = GetTickCount64() + ms;
	bool waited = false;
	while (GetTickCount64() < time || !waited)
	{
		WAIT(0);
		waited = true;
		if (auto menu = GetMenu())
			menu->OnDraw();
	}
}

void MenuItemBase::SetStatusText(const std::string& text, int ms /*= 2500*/)
{
	if (text.empty())
	{
		// Invalid input, throw an exception or handle the error accordingly
		throw std::invalid_argument("text cannot be empty");
	}

	if (ms <= 0)
	{
		// Invalid input, throw an exception or handle the error accordingly
		throw std::invalid_argument("ms must be a positive value");
	}

	MenuController* controller;
	if (m_menu && (controller = m_menu->GetController()))
	{
		controller->SetStatusText(text, ms);
	}
}


void MenuItemBase::OnDraw(float lineTop, float lineLeft, bool active)
{
	if (m_lineHeight <= 0 || m_lineWidth <= 0)
	{
		// Invalid input, throw an exception or handle the error accordingly
		throw std::invalid_argument("lineHeight and lineWidth must be positive values");
	}

	// Validate color parameters
	if (m_colorText.r < 0 || m_colorText.r > 255 ||
		m_colorText.g < 0 || m_colorText.g > 255 ||
		m_colorText.b < 0 || m_colorText.b > 255 ||
		m_colorText.a < 0 || m_colorText.a > 255 ||
		m_colorTextActive.r < 0 || m_colorTextActive.r > 255 ||
		m_colorTextActive.g < 0 || m_colorTextActive.g > 255 ||
		m_colorTextActive.b < 0 || m_colorTextActive.b > 255 ||
		m_colorTextActive.a < 0 || m_colorTextActive.a > 255 ||
		m_colorRect.r < 0 || m_colorRect.r > 255 ||
		m_colorRect.g < 0 || m_colorRect.g > 255 ||
		m_colorRect.b < 0 || m_colorRect.b > 255 ||
		m_colorRect.a < 0 || m_colorRect.a > 255 ||
		m_colorRectActive.r < 0 || m_colorRectActive.r > 255 ||
		m_colorRectActive.g < 0 || m_colorRectActive.g > 255 ||
		m_colorRectActive.b < 0 || m_colorRectActive.b > 255 ||
		m_colorRectActive.a < 0 || m_colorRectActive.a > 255)
	{
		// Invalid input, throw an exception or handle the error accordingly
		throw std::invalid_argument("color values must be between 0 and 255");
	}

	// text
	ColorRgba color = active ? m_colorTextActive : m_colorText;
	UI::SET_TEXT_SCALE(0.0, m_lineHeight * 8.0f);
	UI::SET_TEXT_COLOR_RGBA(color.r, color.g, color.b, color.a);
	UI::SET_TEXT_CENTRE(0);
	UI::SET_TEXT_DROPSHADOW(0, 0, 0, 0, 0);
	DrawText(lineLeft + m_textLeft, lineTop + m_lineHeight / 4.5f, const_cast<char*>(GetCaption().c_str()));
	// rect
	color = active ? m_colorRectActive : m_colorRect;
	DrawRect(lineLeft, lineTop, m_lineWidth, m_lineHeight, color.r, color.g, color.b, color.a);
}

void MenuItemSwitchable::OnDraw(float lineTop, float lineLeft, bool active)
{
	// Validate lineTop and lineLeft parameters
	if (lineTop < 0 || lineLeft < 0)
	{
		// Invalid input, throw an exception or handle the error accordingly
		throw std::invalid_argument("lineTop and lineLeft must not be negative values");
	}

	// Validate lineWidth and lineHeight obtained from GetLineWidth() and GetLineHeight()
	float lineWidth = GetLineWidth();
	float lineHeight = GetLineHeight();
	if (lineWidth <= 0 || lineHeight <= 0)
	{
		// Invalid input, throw an exception or handle the error accordingly
		throw std::invalid_argument("lineWidth and lineHeight must be positive values");
	}

	// Proceed with the drawing logic
	MenuItemDefault::OnDraw(lineTop, lineLeft, active);
	ColorRgba color = active ? GetColorTextActive() : GetColorText();
	UI::SET_TEXT_SCALE(0.0, lineHeight * 8.0f);
	UI::SET_TEXT_COLOR_RGBA(color.r, color.g, color.b, static_cast<int>(color.a / 1.1f));
	UI::SET_TEXT_CENTRE(0);
	UI::SET_TEXT_DROPSHADOW(0, 0, 0, 0, 0);
	DrawText(lineLeft + lineWidth - lineWidth / 6.35f, lineTop + lineHeight / 4.8f, GetState() ? "[Y]" : "[N]");
}

void MenuItemMenu::OnDraw(float lineTop, float lineLeft, bool active)
{
	// Validate lineTop and lineLeft parameters
	if (lineTop < 0 || lineLeft < 0)
	{
		// Invalid input, throw an exception or handle the error accordingly
		throw std::invalid_argument("lineTop and lineLeft must not be negative values");
	}

	// Validate lineWidth and lineHeight obtained from GetLineWidth() and GetLineHeight()
	float lineWidth = GetLineWidth();
	float lineHeight = GetLineHeight();
	if (lineWidth <= 0 || lineHeight <= 0)
	{
		// Invalid input, throw an exception or handle the error accordingly
		throw std::invalid_argument("lineWidth and lineHeight must be positive values");
	}

	// Proceed with the drawing logic
	MenuItemDefault::OnDraw(lineTop, lineLeft, active);
	ColorRgba color = active ? GetColorTextActive() : GetColorText();
	UI::SET_TEXT_SCALE(0.0, lineHeight * 8.0f);
	UI::SET_TEXT_COLOR_RGBA(color.r, color.g, color.b, color.a / 2);
	UI::SET_TEXT_CENTRE(0);
	UI::SET_TEXT_DROPSHADOW(0, 0, 0, 0, 0);
	DrawText(lineLeft + lineWidth - lineWidth / 8, lineTop + lineHeight / 3.5f, "*");
}

void MenuItemMenu::OnSelect()
{
	if (!m_menu)
	{
		// Invalid state, throw an exception or handle the error accordingly
		throw std::logic_error("m_menu must not be null");
	}

	if (auto parentMenu = GetMenu())
	{
		if (auto controller = parentMenu->GetController())
		{
			controller->PushMenu(m_menu);
		}
	}
}

void MenuBase::OnDraw()
{
	float lineTop = MenuBase_menuTop;
	float lineLeft = MenuBase_menuLeft;
	if (m_itemTitle->GetClass() == eMenuItemClass::ListTitle)
		reinterpret_cast<MenuItemListTitle *>(m_itemTitle)->
			SetCurrentItemInfo(GetActiveItemIndex() + 1, static_cast<int>(m_items.size()));
	m_itemTitle->OnDraw(lineTop, lineLeft, false);
	lineTop += m_itemTitle->GetLineHeight();
	for (int i = 0; i < MenuBase_linesPerScreen; i++)
	{
		int itemIndex = m_activeScreenIndex * MenuBase_linesPerScreen + i;
		if (itemIndex == m_items.size())
			break;
		MenuItemBase *item = m_items[itemIndex];
		item->OnDraw(lineTop, lineLeft, m_activeLineIndex == i);
		lineTop += item->GetLineHeight() - item->GetLineHeight() * MenuBase_lineOverlap;
	}
}

int MenuBase::OnInput()
{
	const int itemCount = static_cast<int>(m_items.size());
	const int itemsLeft = itemCount % MenuBase_linesPerScreen;
	const int screenCount = itemCount / MenuBase_linesPerScreen + (itemsLeft ? 1 : 0);
	const int lineCountLastScreen = itemsLeft ? itemsLeft : MenuBase_linesPerScreen;

	auto buttons = MenuInput::GetButtonState();	

	int waitTime = 0;

	if (buttons.a || buttons.b || buttons.up || buttons.down)
	{
		MenuInput::MenuInputBeep();
		waitTime = buttons.b ? 200 : 150;
	}

	if (buttons.a)
	{
		int activeItemIndex = GetActiveItemIndex();
		m_items[activeItemIndex]->OnSelect();
	} else
	if (buttons.b)
	{ 
		if (auto controller = GetController())
			controller->PopMenu();
	} else		
	if (buttons.up)
	{
		if (m_activeLineIndex-- == 0)
		{
			if (m_activeScreenIndex == 0)
			{
				m_activeScreenIndex = screenCount - 1;
				m_activeLineIndex = lineCountLastScreen - 1;
			} else
			{
				m_activeScreenIndex--;
				m_activeLineIndex = MenuBase_linesPerScreen - 1;
			}
		}
	} else
	if (buttons.down)
	{
		m_activeLineIndex++;
		if (m_activeLineIndex == ((m_activeScreenIndex == (screenCount - 1)) ? lineCountLastScreen : MenuBase_linesPerScreen))
		{
			if (m_activeScreenIndex == screenCount - 1)
				m_activeScreenIndex = 0;
			else
				m_activeScreenIndex++;
			m_activeLineIndex = 0;
		}
	}

	return waitTime;
}

void MenuController::DrawStatusText()
{
	if (GetTickCount64() < m_statusTextMaxTicks)
	{
		UI::SET_TEXT_SCALE(0.55, 0.55);
		UI::SET_TEXT_COLOR_RGBA(255, 255, 255, 255);
		UI::SET_TEXT_CENTRE(1);
		UI::SET_TEXT_DROPSHADOW(0, 0, 0, 0, 0);
		DrawText(0.5, 0.5, const_cast<char *>(m_statusText.c_str()));
	}
}