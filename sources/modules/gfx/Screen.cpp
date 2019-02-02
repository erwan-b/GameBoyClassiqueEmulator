/*
** EPITECH PROJECT, 2019
** Epitech scolarship project (4 years remaining)
** File description:
**      Made on 2019/01 by ebernard
*/

#include "Screen.hpp"

gfx::Screen::Screen(emulator::Memory &mem) :
	_memory(mem), _line (0), _clock(0), _mode(0), _beginDisplay(0, 0)
{
	sf::Vector2f pos = {0, 1};

	for (auto &elem  : _pixels) {
		elem.position = pos;
		elem.color = sf::Color::Black;
		pos.x++;
		if (pos.x == 160) {
			pos.x = 0;
			pos.y++;
		}
	}
}

/**
 * Init sfml window
 */
void gfx::Screen::initWindow()
{
	_window = std::make_unique<sf::RenderWindow>(sf::VideoMode(160, 144), "EmulIt", sf::Style::Close);
	resetScreen();
	_window->display();
}

/**
 * Reset Screen color to black
 */
void gfx::Screen::resetScreen()
{
	for (auto &elem : _pixels){
		elem.color = sf::Color::Black;
	}
	_window->draw(_pixels.data(), _pixels.size(), sf::Points);
}

/**
 * Emulate the 4 step of a gameboy GPU
 * First: Horizontal blank after each line
 * Second: If he is at the bottom vertical blank and put the screen
 * Third: OAM read (blank) until the GPU passe the right time
 * Fourth: Render the image
 */
void gfx::Screen::put(size_t timer)
{
	_clock +=  timer;

	if (_mode == 0)
		Hblank();
	else if (_mode == 1)
		Vblank();
	else if (_mode == 2)
		ObjectRead();
	else if (_mode == 3)
		renderLine();
}

/**
 * emulate gpu timer
 * horizontal blank line after each line can
 */
void gfx::Screen::Hblank()
{
	if (_clock <= 204)
		return ;
	_line++;
	_clock = 0;

	if (_line == 143) {
		_mode = 1;
		_window->clear(sf::Color::Red);
		_window->draw(_pixels.data(), _pixels.size(), sf::Points);
		_window->display();
	} else {
		_mode = 2;
	}
}


/**
 * emulate gpu timer for vertical blank line.
 */
void gfx::Screen::Vblank()
{
	if (_clock <= 456)
		return ;
	_mode = 0;
	_line++;
	if (_line > 153) {
		_mode = 2;
		_line = 0;
	}
}

void gfx::Screen::ObjectRead()
{
	if (_clock <= 80)
		return ;
	_clock = 0;
	_mode = 3;
}

int gfx::Screen::getAdressFromTile(int tileNb)
{
	int adress = _bgMap ?  0x8000 : 8800;

	if (!_bgMap)
		tileNb += 128;

	return adress + tileNb * 2;
}

void gfx::Screen::renderLine()
{
	if (_clock <= 172)
		return ;

	int addr = _bgMap ? 0x9C00 : 0x9800;
	auto lineoffs = (_beginDisplay.x / 8);
	unsigned char bit;
	_clock = 0;
	_mode = 0;

	// Which line of tiles to use in the map
	addr += (_line + _beginDisplay.y) / 8;

	// Which tile to start with in the map line
	size_t tileNumber;

	// get addr from tile for line and index
	for (size_t i = 0; i < 160;) {
		bit = 1;
		tileNumber = _memory[addr + lineoffs + i / 8];
		do {
			(*this)[_line][i].color = getColorFromAdress(getAdressFromTile(tileNumber), bit);
			bit <<= (unsigned int)1;
			i++;
		} while ((i % 8) != 0 && i < 160);
	}
}

sf::Color gfx::Screen::getColorFromAdress(int address, unsigned char bit)
{
	int value = (_memory[address] & bit) + (_memory[address + 1] & bit);

	if (value == 0)
		return sf::Color(255, 255, 255);
	if (value == 1)
		return sf::Color(192, 192, 192);
	if (value == 2)
		return sf::Color(96, 96, 96);
	return sf::Color(0, 0, 0);
}
