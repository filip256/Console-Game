#include<stdio.h>
#include<stdbool.h>
#include<windows.h>
#include<time.h>

#define MAP_SIZE_X 45
#define MAP_SIZE_Y 45
#define MAX_PROJ 256
#define MAX_ENTITY 64
#define MAX_SPEED 20

HWND cHwnd;
CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
HANDLE hOutput;
HANDLE hInput;

struct mapTile
{
	char symbol[2];
	int fg, bg, solidity;
}mapArray[MAP_SIZE_Y][MAP_SIZE_X];
struct VECTOR2I
{
	int x, y;
};

bool loadMap(const char* filename)
{

	FILE* file = fopen(filename, "r");
	if (!file)
		return false;

	for (int i = 0; i < MAP_SIZE_Y; ++i)
	{
		for (int j = 0; j < MAP_SIZE_X; ++j)
		{
			mapArray[i][j].symbol[0] = fgetc(file);
			mapArray[i][j].symbol[1] = 0;
		}
		fgetc(file);// escape endline
	}
	fgetc(file);// escape endline
	for (int i = 0; i < MAP_SIZE_Y; ++i)
	{
		for (int j = 0; j < MAP_SIZE_X; ++j)
			mapArray[i][j].fg = fgetc(file) - 'a';
		fgetc(file);// escape endline
	}
	fgetc(file);// escape endline
	for (int i = 0; i < MAP_SIZE_Y; ++i)
	{
		for (int j = 0; j < MAP_SIZE_X; ++j)
			mapArray[i][j].bg = fgetc(file) - 'a';
		fgetc(file);// escape endline
	}
	fgetc(file);// escape endline
	for (int i = 0; i < MAP_SIZE_Y; ++i)
	{
		for (int j = 0; j < MAP_SIZE_X; ++j)
			mapArray[i][j].solidity = fgetc(file) - '0';
		fgetc(file);// escape endline
	}
	fgetc(file);// escape endline
	fclose(file);
	return true;
}
int loadHighscore(const char* filename)
{
	FILE* file = fopen(filename, "r");
	if (!file)
		return -1;

	int val = 0;
	fscanf(file, "%d", &val);
	fclose(file);
	return val;
}
void saveHighscore(const char* filename, const int score)
{
	FILE* file = fopen(filename, "w");

	char aux[64];
	_itoa(score, aux, 10);
	fputs(aux, file);

	fclose(file);
}

bool chance(const int percent)
{
	if (rand() % 100 < percent)
		return true;
	return false;
}

void clearScreen()
{
	COORD topLeft = { 0, 0 };
	DWORD written;
	FillConsoleOutputCharacterA(hOutput, ' ', (DWORD)(bufferInfo.dwSize.X * bufferInfo.dwSize.Y), topLeft, &written);
	FillConsoleOutputAttribute(hOutput, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE, (DWORD)(bufferInfo.dwSize.X * bufferInfo.dwSize.Y), topLeft, &written);
	SetConsoleCursorPosition(hOutput, topLeft);
}
void drawText(const char* text, const short int x, const short int y, const int fg, const int bg)
{
	COORD pos; pos.X = x; pos.Y = y;
	SetConsoleCursorPosition(hOutput, pos);
	const int attr = fg + bg * 16;
	static int lastAttrib = -1;
	if(attr == lastAttrib)
		printf("%s", text);
	else
	{
		SetConsoleTextAttribute(hOutput, attr);
		printf("%s", text);
		lastAttrib = attr;
	}
}
void startup()
{
	//Setting a font 
	CONSOLE_FONT_INFOEX cfi;
	cfi.cbSize = sizeof(cfi);
	cfi.nFont = 0;
	cfi.dwFontSize.X = 12;                   // Width of each character in the font
	cfi.dwFontSize.Y = 12;                  // Height
	cfi.FontFamily = FF_DONTCARE;
	cfi.FontWeight = FW_NORMAL;
	wcscpy_s(cfi.FaceName, 32, L"Lucida Console"); // Choose your font
	SetCurrentConsoleFontEx(hOutput, FALSE, &cfi);

	//Setting the window size
	RECT r;
	GetWindowRect(cHwnd, &r);
	MoveWindow(cHwnd, r.left, r.top, 990, 580, TRUE);

	//Corelating buffer to the size
	COORD size = { 79 , 45 };
	SetConsoleScreenBufferSize(hOutput, size);

	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(hOutput, &cursorInfo);
	cursorInfo.bVisible = false;
	cursorInfo.dwSize = 1;
	SetConsoleCursorInfo(hOutput, &cursorInfo);

	DWORD prev_mode;
	GetConsoleMode(hInput, &prev_mode);
	SetConsoleMode(hInput, prev_mode & ~ENABLE_QUICK_EDIT_MODE);

	SetWindowLong(cHwnd, GWL_STYLE, GetWindowLong(cHwnd, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);

	SetConsoleTitle(TEXT("Wizard Arena"));
}

struct VECTOR2I getTracePoint(const int x1, const int y1, const int x2, const int y2)
{
	/*
	struct VECTOR2I newP;
	if(x1 == x2)
	{
		newP.x = x1;
		if (y1 < y2)
			newP.y = y1 + 1;
		else if (y1 > y2)
			newP.y = y1 - 1;
		else
			newP.y = y1;
	}
	else if (x1 < x2)
	{
		double Xq = (y2 - y1) / (x2 - x1), Cq = (y1*x2 - x1 * y2) / (x2 - x1);
		newP.x = x1 + 1; newP.y = (int)newP.x * Xq + Cq;
	}
	else
	{
		double Xq = (y2 - y1) / (x2 - x1), Cq = (y1*x2 - x1 * y2) / (x2 - x1);
		newP.x = x1 - 1; newP.y = (int)newP.x * Xq + Cq;
	}
	return newP;
	*/
	struct VECTOR2I newP;
	if (x1 == x2)
	{
		newP.x = x1;
		if (y1 < y2)
			newP.y = y1 + 1;
		else if (y1 > y2)
			newP.y = y1 - 1;
		else
			newP.y = y1;
	}
	else if (y1 == y2)
	{
		newP.y = y1;
		if (x1 < x2)
			newP.x = x1 + 1;
		else if (x1 > x2)
			newP.x = x1 - 1;
		else
			newP.x = x1;
	}
	else
	{
		if (rand() & 1)
		{
			newP.y = y1;
			if (x1 < x2)
				newP.x = x1 + 1;
			else
				newP.x = x1 - 1;
		}
		else
		{
			newP.x = x1;
			if (y1 < y2)
				newP.y = y1 + 1;
			else
				newP.y = y1 - 1;
		}
	}
	return newP;
}

struct Projectile
{
	bool isActive, friendly;
	char symbol[2];
	int x, y, fg, bg;
	int stepX, stepY, range, speed, stepCount;
	int special, damage;
};
int getFreeProjectile(struct Projectile* proj)
{
	for (int i = 0; i < MAX_PROJ; ++i)
		if (!proj[i].isActive)
			return i;
	return -1;
}
int newProjectile(struct Projectile* proj, const int startX, const int startY, const int stepX, const int stepY, const int range, const int speed,const char symb, const int fg, const int bg, const int special, const int damage, const bool friendly)
{
	int free = getFreeProjectile(proj);
	if (free != -1)
	{
		proj[free].isActive = true;
		proj[free].symbol[0] = symb; proj[free].symbol[1] = 0;
		proj[free].fg = fg; proj[free].bg = bg;
		proj[free].stepX = stepX;
		proj[free].stepY = stepY;
		proj[free].damage = damage;

		proj[free].x = startX;
		proj[free].y = startY;
		proj[free].range = range;
		proj[free].speed = speed;
		proj[free].stepCount = 0;
		proj[free].special = special; //0 - none, 1 - explode, 2 - speed up, 3 - constant respawn
		proj[free].friendly = friendly;
	}
}
void displayProjectile(const struct Projectile* proj)
{
	if (proj->bg == -1)
		drawText(proj->symbol, proj->x, proj->y, proj->fg, mapArray[proj->y][proj->x].bg);
	else
		drawText(proj->symbol, proj->x, proj->y, proj->fg, proj->bg);
}
bool stepProjectile(struct Projectile* proj)
{
	if(proj->stepCount) drawText(mapArray[proj->y][proj->x].symbol, proj->x, proj->y, mapArray[proj->y][proj->x].fg, mapArray[proj->y][proj->x].bg);

	if ((proj->range == -1 || proj->stepCount < proj->range) && proj->x + proj->stepX >= 0 && proj->y + proj->stepY >= 0 && proj->x + proj->stepX < MAP_SIZE_X && proj->y + proj->stepY < MAP_SIZE_Y && !mapArray[proj->y + proj->stepY][proj->x + proj->stepX].solidity)
	{
		if(proj->special == 2 && proj->stepCount == 4)
			proj->speed = 18;

		if (proj->special == 7 && proj->bg > -1) { int aux = proj->fg;  proj->fg = proj->bg; proj->bg = aux; }
		proj->x += proj->stepX;
		proj->y += proj->stepY;
		displayProjectile(proj);
		proj->stepCount++;

		return true;
	}
	proj->isActive = false;
	return false;
}
void clearProjectiles(struct Projectile* proj)
{
	for (int i = 0; i < MAX_PROJ; ++i)
		proj[i].isActive = false;
}

struct Entity
{
	char symbol[2];
	int x, y, fg, bg, maxHP, maxMana, hp, mana, xp, level;
	int cooldown;
	bool isActive, isInvincible;
};
void displayEntity(const struct Entity* player)
{
	if (player->bg == -1)
		drawText(player->symbol, player->x, player->y, player->fg, mapArray[player->y][player->x].bg);
	else
		drawText(player->symbol, player->x, player->y, player->fg, player->bg);
}
int getFreeEntity(struct Entity* ent)
{
	for (int i = 0; i < MAX_ENTITY; ++i)
		if (!ent[i].isActive)
			return i;
	return -1;
}
int newEntity(struct Entity* ent, const int x, const int y, const char symb, const int fg, const int bg, const int hp, const int level)
{
	int free = getFreeEntity(ent);
	if (free != -1)
	{
		ent[free].isActive = true;
		ent[free].x = x; ent[free].y = y;
		ent[free].cooldown = 0;
		ent[free].symbol[0] = symb; ent[free].symbol[1] = 0;
		ent[free].fg = fg; ent[free].bg = bg;
		ent[free].hp = hp;
		ent[free].level = level;
	}
	return free;
}
bool isFreePosition(struct Entity* ent, const int x, const int y)
{
	if (x < 0 || y < 0 || x >= MAP_SIZE_X || y >= MAP_SIZE_Y || mapArray[y][x].solidity == 1)
		return false;
	for (int i = 0; i < MAX_ENTITY; ++i)
		if (ent[i].isActive && x == ent[i].x && y == ent[i].y)
			return false;
	return true;
}
void setPosition(struct Entity* player, const int newX, const int newY)
{
	if (newX >= 0 && newY >= 0 && newX < MAP_SIZE_X && newY < MAP_SIZE_Y && !mapArray[newY][newX].solidity)
	{
		drawText(mapArray[player->y][player->x].symbol, player->x, player->y, mapArray[player->y][player->x].fg, mapArray[player->y][player->x].bg);
		player->x = newX;
		player->y = newY;
		displayEntity(player);
	}
}

struct VECTOR2I getSpawnPoint(struct Entity* enemy, const int px, const int py)
{
	struct VECTOR2I rnd;
	do
	{
		rnd.x = rand() % MAP_SIZE_X; rnd.y = rand() % MAP_SIZE_Y;
	} while (abs(rnd.x - px) + abs(rnd.y - py) <= 10 || !isFreePosition(enemy, rnd.x, rnd.y) || !isFreePosition(enemy, rnd.x - 1, rnd.y) || !isFreePosition(enemy, rnd.x, rnd.y - 1) || !isFreePosition(enemy, rnd.x + 1, rnd.y) || !isFreePosition(enemy, rnd.x, rnd.y + 1));
	return rnd;
}

void resetPlayer(struct Entity* player)
{
	player->isActive = true;
	player->symbol[0] = 'ê'; player->symbol[1] = 0; player->fg = 15; player->bg = -1;
	player->x = 22; player->y = 22;
	player->cooldown = 0;
	player->hp = player->maxHP = 1000; player->mana = player->maxMana = 100;
	player->level = 1;
	player->isInvincible = false;
	setPosition(player, 22, 22);
}
void clearEntities(struct Entity* ent)
{
	for (int i = 0; i < MAX_ENTITY; ++i)
		ent[i].isActive = false;
}

void fullDisplay(const struct Entity* player)
{
	for (int i = 0; i < MAP_SIZE_Y; ++i)
	{
		for (int j = 0; j < MAP_SIZE_X; ++j)
		{
			drawText(mapArray[i][j].symbol, j, i, mapArray[i][j].fg, mapArray[i][j].bg);
		}
	}
	displayEntity(player);
}

struct VisualBar
{
	int x, y, value, maxValue, lenght, fg, bg;
	char string[64];
};
void displayBar(struct VisualBar* bar)
{
	drawText(bar->string, bar->x, bar->y, bar->fg, bar->bg);
}
void setValue(struct VisualBar* bar, const int value)
{
	bar->value = value;
	int aux = ((float)bar->value / bar->maxValue) * bar->lenght;
	for (int i = 0; i < aux; ++i)
		bar->string[i] = 'Û';
	for (int i = aux; i < bar->lenght; ++i)
		bar->string[i] = '°';
	if(value > 0)
		bar->string[0] = 'Û';

	displayBar(bar);
}

int main()
{
	loadMap("map2.txt");
	cHwnd = GetConsoleWindow();
	hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	hInput = GetStdHandle(STD_INPUT_HANDLE);
	GetConsoleScreenBufferInfo(hOutput, &bufferInfo);
	clearScreen();
	srand(time(NULL));
	startup();

	struct Entity player;
	resetPlayer(&player);

	struct Entity enemy[MAX_ENTITY];
	for (int i = 0; i < MAX_ENTITY; ++i)
	{
		enemy[i].x = -1; enemy[i].y = -1;
		enemy[i].isActive = false;
	}

	struct Projectile bolt[MAX_PROJ];
	for (int i = 0; i < MAX_PROJ; ++i)
	{
		bolt[i].x = -1; bolt[i].y = -1;
		bolt[i].isActive = false;
	}

	drawText("Health:", 50, 4, 8, 0);
	struct VisualBar healthBar;
	{
		healthBar.lenght = 25; healthBar.string[healthBar.lenght] = 0;
		healthBar.x = 50; healthBar.y = 5;
		healthBar.maxValue = player.maxHP;
		healthBar.fg = 4;
		healthBar.bg = 0;
		setValue(&healthBar, player.hp);
	}
	drawText("Mana:", 50, 7, 8, 0);
	struct VisualBar manaBar;
	{
		manaBar.lenght = 25; manaBar.string[manaBar.lenght] = 0;
		manaBar.x = 50; manaBar.y = 8;
		manaBar.maxValue = player.maxMana;
		manaBar.fg = 1;
		manaBar.bg = 0;
		setValue(&manaBar, player.mana);
	}
	
	drawText("Score:", 50, 10, 8, 0); drawText("0             ", 63, 10, 15, 0);
	drawText("Highscore:", 50, 12, 8, 0);

	drawText("WASD", 50, 25, 7, 0); drawText("-", 55, 25, 6, 0); drawText("Move", 57, 25, 14, 0);
	drawText("IJKL", 50, 26, 7, 0); drawText("-", 55, 26, 6, 0); drawText("Etherwave", 57, 26, 14, 0);
	drawText("U", 50, 27, 7, 0); drawText("-", 55, 27, 6, 0); drawText("Ignition Blast", 57, 27, 14, 0);
	drawText("H", 50, 28, 7, 0); drawText("-", 55, 28, 6, 0); drawText("Transcendence", 57, 28, 14, 0);

	bool doDisplay = true, startState = true;
	int frameCount = 0, killCount = 0, highscore, enemyCount = 0;

	while (true)
	{
		if (startState)
		{
			resetPlayer(&player);
			clearEntities(enemy);
			clearProjectiles(bolt);
			fullDisplay(&player);
			drawText("Press       to play", 13, 17, 6, 0);
			drawText("SPACE", 19, 17, 11, 0);
			drawText("³", 22, 19, 6, 0); drawText("v", 22, 20, 6, 0);

			highscore = loadHighscore("score.txt");
			if (highscore != -1)
			{
				char aux[64]; _itoa(highscore, aux, 10);
				drawText(aux, 63, 12, 14, 0);
			}

			while (!(GetKeyState(' ') & 0x8000));

			setValue(&manaBar, player.mana);
			setValue(&healthBar, player.hp);
			frameCount = 0;
			enemyCount = 0;

			killCount = 0;
			drawText("0             ", 63, 10, 15, 0);

			startState = false;
			doDisplay = true;

			//newEntity(enemy, 30, 25, '%', 13, -1, 100, 3);
			//newEntity(enemy, 2, 25, '&', 4, -1, 20, 1);
			//newEntity(enemy, 30, 35, '#', 6, -1, 200, 4);
			//newEntity(enemy, 30, 35, '$', 6, -1, 200, 5);
		}
		else
		{
			if (frameCount % 2 == 0)
			{
				if (GetKeyState('A') & 0x8000)
				{
					if (GetKeyState('W') & 0x8000 && (isFreePosition(enemy, player.x - 1, player.y - 1) || player.isInvincible))
						setPosition(&player, player.x - 1, player.y - 1);
					else if (GetKeyState('S') & 0x8000 && (isFreePosition(enemy, player.x - 1, player.y + 1) || player.isInvincible))
						setPosition(&player, player.x - 1, player.y + 1);
					else if(isFreePosition(enemy, player.x - 1, player.y) || player.isInvincible)
						setPosition(&player, player.x - 1, player.y);
				}
				else if (GetKeyState('D') & 0x8000)
				{
					if (GetKeyState('W') & 0x8000 && (isFreePosition(enemy, player.x + 1, player.y - 1) || player.isInvincible))
						setPosition(&player, player.x + 1, player.y - 1);
					else if (GetKeyState('S') & 0x8000 && (isFreePosition(enemy, player.x + 1, player.y + 1) || player.isInvincible))
						setPosition(&player, player.x + 1, player.y + 1);
					else if(isFreePosition(enemy, player.x + 1, player.y) || player.isInvincible)
						setPosition(&player, player.x + 1, player.y);
				}
				else if (GetKeyState('W') & 0x8000)
				{
					if(isFreePosition(enemy, player.x, player.y - 1) || player.isInvincible)
						setPosition(&player, player.x, player.y - 1);
				}
				else if (GetKeyState('S') & 0x8000)
				{
					if(isFreePosition(enemy, player.x, player.y + 1) || player.isInvincible)
						setPosition(&player, player.x, player.y + 1);
				}

				if (GetKeyState('J') & 0x8000)
				{
					if (frameCount - player.cooldown > 20 && player.mana >= 10)
					{
						player.cooldown = frameCount;
						player.mana -= 10;
						setValue(&manaBar, player.mana);

						newProjectile(bolt, player.x, player.y, -1, 0, -1, 19, 'Ä', 6, -1, 0, 10, true);
						newProjectile(bolt, player.x - 2, player.y, -1, 0, -1, 19, '<', 10, -1, 1, 20, true);
						newProjectile(bolt, player.x, player.y, -1, 1, 10, 18, '/', 6, -1, 4, 20, true);
						newProjectile(bolt, player.x, player.y, -1, -1, 10, 18, '\\', 6, -1, 4, 20, true);
					}
				}
				else if (GetKeyState('L') & 0x8000)
				{
					if (frameCount - player.cooldown > 20 && player.mana >= 10)
					{
						player.cooldown = frameCount;

						player.mana -= 10;
						setValue(&manaBar, player.mana);

						newProjectile(bolt, player.x, player.y, 1, 0, -1, 19, 'Ä', 6, -1, 0, 10, true);
						newProjectile(bolt, player.x + 2, player.y, 1, 0, -1, 19, '>', 10, -1, 1, 20, true);
						newProjectile(bolt, player.x, player.y, 1, -1, 10, 18, '/', 6, -1, 3, 20, true);
						newProjectile(bolt, player.x, player.y, 1, 1, 10, 18, '\\', 6, -1, 3, 20, true);
					}
				}
				else if (GetKeyState('I') & 0x8000)
				{
					if (frameCount - player.cooldown > 20 && player.mana >= 10)
					{
						player.cooldown = frameCount;

						player.mana -= 10;
						setValue(&manaBar, player.mana);

						newProjectile(bolt, player.x, player.y, 0, -1, -1, 19, '³', 6, -1, 0, 10, true);
						newProjectile(bolt, player.x, player.y - 2, 0, -1, -1, 19, '^', 10, -1, 1, 20, true);
						newProjectile(bolt, player.x, player.y, 1, -1, 10, 18, '/', 6, -1, 5, 20, true);
						newProjectile(bolt, player.x, player.y, -1, -1, 10, 18, '\\', 6, -1, 5, 20, true);
					}
				}
				else if (GetKeyState('K') & 0x8000)
				{
					if (frameCount - player.cooldown > 20 && player.mana >= 10)
					{
						player.cooldown = frameCount;

						player.mana -= 10;
						setValue(&manaBar, player.mana);

						newProjectile(bolt, player.x, player.y, 0, 1, -1, 19, '³', 6, -1, 0, 10, true);
						newProjectile(bolt, player.x, player.y + 2, 0, 1, -1, 19, 'v', 10, -1, 1, 20, true);
						newProjectile(bolt, player.x, player.y, -1, 1, 10, 18, '/', 6, -1, 6, 20, true);
						newProjectile(bolt, player.x, player.y, 1, 1, 10, 18, '\\', 6, -1, 6, 20, true);
					}
				}
				else if (GetKeyState('U') & 0x8000)
				{
					if (frameCount - player.cooldown > 30 && player.mana >= 30)
					{
						player.cooldown = frameCount;
						player.mana -= 30;
						newProjectile(bolt, player.x, player.y, -1, 1, 3, 17, '#', 6, 4, 7, 10, true);
						newProjectile(bolt, player.x, player.y, -1, 0, 5, 17, '<', 6, 4, 7, 10, true);
						newProjectile(bolt, player.x, player.y, -1, -1, 3, 17, '#', 6, 4, 7, 10, true);
						newProjectile(bolt, player.x, player.y, 0, -1, 5, 17, '^', 6, 4, 7, 10, true);
						newProjectile(bolt, player.x, player.y, 1, -1, 3, 17, '#', 6, 4, 7, 10, true);
						newProjectile(bolt, player.x, player.y, 1, 0, 5, 17, '>', 6, 4, 7, 10, true);
						newProjectile(bolt, player.x, player.y, 1, 1, 3, 17, '#', 6, 4, 7, 10, true);
						newProjectile(bolt, player.x, player.y, 0, 1, 5, 17, 'v', 6, 4, 7, 10, true);
					}
				}
				else if (GetKeyState('H') & 0x8000)
				{
					if (frameCount - player.cooldown > 20)
					{
						if (!player.isInvincible)
						{
							player.isInvincible = true;
							player.bg = 6;
						}
						else
						{
							player.isInvincible = false;
							player.bg = -1;
						}
						player.cooldown = frameCount;
					}
				}
			}

			if (frameCount % 20 == 0)
			{
				if (chance(50) && enemyCount < MAX_ENTITY)
				{
					struct VECTOR2I rnd = getSpawnPoint(enemy, player.x, player.y);
					newProjectile(bolt, rnd.x, rnd.y, 0, 0, 1, 10, '±', 10, 11, 0, 0, false);
					newEntity(enemy, rnd.x, rnd.y, '&', 4, -1, 50, 1);
				}
				if (chance((killCount - 30) / 2) && enemyCount < MAX_ENTITY)
				{
					struct VECTOR2I rnd = getSpawnPoint(enemy, player.x, player.y);
					newProjectile(bolt, rnd.x, rnd.y, 0, 0, 1, 10, '±', 10, 11, 0, 0, false);
					newEntity(enemy, rnd.x, rnd.y, '@', 12, -1, 30, 2);
				}
				if (chance((killCount - 100) / 8) && enemyCount < MAX_ENTITY - 20)
				{
					struct VECTOR2I rnd = getSpawnPoint(enemy, player.x, player.y);
					newProjectile(bolt, rnd.x, rnd.y, 0, 0, 1, 10, '±', 10, 11, 0, 0, false);
					newEntity(enemy, rnd.x, rnd.y, '$', 9, -1, 40, 5);
				}
				if (chance((killCount - 400) / 18) && enemyCount < MAX_ENTITY - 40)
				{
					struct VECTOR2I rnd = getSpawnPoint(enemy, player.x, player.y);
					newProjectile(bolt, rnd.x, rnd.y, 0, 0, 1, 10, '±', 10, 11, 0, 0, false);
					newEntity(enemy, rnd.x, rnd.y, '%', 13, -1, 100, 3);
				}
				if (chance((killCount - 300) / 32) && enemyCount < MAX_ENTITY - 40)
				{
					struct VECTOR2I rnd = getSpawnPoint(enemy, player.x, player.y);
					newProjectile(bolt, rnd.x, rnd.y, 0, 0, 1, 10, '±', 10, 11, 0, 0, false);
					newEntity(enemy, rnd.x, rnd.y, '#', 6, -1, 120, 4);
				}
			}

			for (int i = 0; i < MAX_PROJ; ++i)
			{
				if (bolt[i].isActive)
				{
					bool stepErr = true;
					if (frameCount % (MAX_SPEED - bolt[i].speed) == 0)
					{
						stepErr = stepProjectile(&bolt[i]); //step
						if (bolt[i].special == 3) //L arrow copy
						{
							newProjectile(bolt, bolt[i].x, bolt[i].y, bolt[i].stepX, 0, -1, 19, '>', 2, 0, 1, 20, true);
							newProjectile(bolt, bolt[i].x, bolt[i].y, -bolt[i].stepX, 0, 2, 17, 'ú', 14, 0, 0, 5, true);
							newProjectile(bolt, bolt[i].x, bolt[i].y, 0, -bolt[i].stepY, 2, 17, 'ú', 14, 0, 0, 5, true);
						}
						if (bolt[i].special == 4)//J arrow copy
						{
							newProjectile(bolt, bolt[i].x, bolt[i].y, bolt[i].stepX, 0, -1, 19, '<', 2, 0, 1, 20, true);
							newProjectile(bolt, bolt[i].x, bolt[i].y, -bolt[i].stepX, 0, 2, 17, 'ú', 14, 0, 0, 5, true);
							newProjectile(bolt, bolt[i].x, bolt[i].y, 0, -bolt[i].stepY, 2, 17, 'ú', 14, 0, 0, 5, true);
						}
						if (bolt[i].special == 5)//I arrow copy
						{
							newProjectile(bolt, bolt[i].x, bolt[i].y, 0, bolt[i].stepY, -1, 19, '^', 2, 0, 1, 20, true);
							newProjectile(bolt, bolt[i].x, bolt[i].y, 0, -bolt[i].stepY, 2, 17, 'ú', 14, 0, 0, 5, true);
							newProjectile(bolt, bolt[i].x, bolt[i].y, -bolt[i].stepX, 0, 2, 17, 'ú', 14, 0, 0, 5, true);
						}
						if (bolt[i].special == 6)//K arrow copy
						{
							newProjectile(bolt, bolt[i].x, bolt[i].y, 0, bolt[i].stepY, -1, 19, 'v', 2, 0, 1, 20, true);
							newProjectile(bolt, bolt[i].x, bolt[i].y, 0, -bolt[i].stepY, 2, 17, 'ú', 14, 0, 0, 5, true);
							newProjectile(bolt, bolt[i].x, bolt[i].y, -bolt[i].stepX, 0, 2, 17, 'ú', 14, 0, 0, 5, true);
						}
					}

					if (stepErr) //only if step worked
					{
						if (bolt[i].friendly)
						{
							for (int j = 0; j < MAX_ENTITY; ++j) //check collision
							{
								if (enemy[j].isActive && bolt[i].x == enemy[j].x && bolt[i].y == enemy[j].y)
								{
									drawText(mapArray[bolt[i].y][bolt[i].x].symbol, bolt[i].x, bolt[i].y, mapArray[bolt[i].y][bolt[i].x].fg, mapArray[bolt[i].y][bolt[i].x].bg);
									bolt[i].isActive = false;
									if (enemy[j].hp > bolt[i].damage)
										enemy[j].hp -= bolt[i].damage;
									else
									{
										enemy[j].hp = 0;
										enemy[j].isActive = false;
										++killCount;
										char aux[64]; _itoa(killCount, aux, 10);
										drawText(aux, 63, 10, 15, 0);
										if(killCount > highscore)
											drawText(aux, 63, 12, 6, 0);
									}
									if (bolt[i].special == 1)
									{
										newProjectile(bolt, bolt[i].x, bolt[i].y, -1, -1, 2, 18, '*', 6, 2, 0, 10, true);
										newProjectile(bolt, bolt[i].x, bolt[i].y, 1, -1, 2, 18, '*', 6, 2, 0, 10, true);
										newProjectile(bolt, bolt[i].x, bolt[i].y, -1, 1, 2, 18, '*', 6, 2, 0, 10, true);
										newProjectile(bolt, bolt[i].x, bolt[i].y, 1, 1, 2, 18, '*', 6, 2, 0, 10, true);
									}
									else if (bolt[i].special == 7) //blast spread
									{
										newProjectile(bolt, bolt[i].x, bolt[i].y, -1, 1, 3, 17, '#', 6, 4, 7, 10, true);
										newProjectile(bolt, bolt[i].x, bolt[i].y, -1, 0, 5, 17, '<', 6, 4, 7, 10, true);
										newProjectile(bolt, bolt[i].x, bolt[i].y, -1, -1, 3, 17, '#', 6, 4, 7, 10, true);
										newProjectile(bolt, bolt[i].x, bolt[i].y, 0, -1, 5, 17, '^', 6, 4, 7, 10, true);
										newProjectile(bolt, bolt[i].x, bolt[i].y, 1, -1, 3, 17, '#', 6, 4, 7, 10, true);
										newProjectile(bolt, bolt[i].x, bolt[i].y, 1, 0, 5, 17, '>', 6, 4, 7, 10, true);
										newProjectile(bolt, bolt[i].x, bolt[i].y, 1, 1, 3, 17, '#', 6, 4, 7, 10, true);
										newProjectile(bolt, bolt[i].x, bolt[i].y, 0, 1, 5, 17, 'v', 6, 4, 7, 10, true);
									}
										 
								}
							}
						}
						else
						{
							if (!player.isInvincible && bolt[i].x == player.x && bolt[i].y == player.y) //player collision
							{
								drawText(mapArray[bolt[i].y][bolt[i].x].symbol, bolt[i].x, bolt[i].y, mapArray[bolt[i].y][bolt[i].x].fg, mapArray[bolt[i].y][bolt[i].x].bg);
								bolt[i].isActive = false;
								if (player.hp > bolt[i].damage)
									player.hp -= bolt[i].damage;
								else
									player.hp = 0;
								setValue(&healthBar, player.hp);
							}
						}
					}
					else
					{
						if (bolt[i].special == 1)
						{
							newProjectile(bolt, bolt[i].x, bolt[i].y, -1, -1, 2, 18, '*', 6, 2, 0, 10, true);
							newProjectile(bolt, bolt[i].x, bolt[i].y, 1, -1, 2, 18, '*', 6, 2, 0, 10, true);
							newProjectile(bolt, bolt[i].x, bolt[i].y, -1, 1, 2, 18, '*', 6, 2, 0, 10, true);
							newProjectile(bolt, bolt[i].x, bolt[i].y, 1, 1, 2, 18, '*', 6, 2, 0, 10, true);
						}
					}
				}
			}

			if (frameCount % 5 == 0)
			{
				for (int i = 0; i < MAX_ENTITY; ++i)
				{
					enemyCount = 0;
					if (enemy[i].isActive)
					{
						++enemyCount;
						//close dmg
						if (!player.isInvincible && enemy[i].level != 4 && abs(enemy[i].x - player.x) + abs(enemy[i].y - player.y) <= 1)
						{
							if (player.hp > 5)
								player.hp -= 5;
							else
								player.hp = 0;
							setValue(&healthBar, player.hp);
						}
						else
						{
							if (enemy[i].level == 1) //basic follow
							{
								struct VECTOR2I eNewPos = getTracePoint(enemy[i].x, enemy[i].y, player.x, player.y);
								if (isFreePosition(enemy, eNewPos.x, eNewPos.y))
									setPosition(&enemy[i], eNewPos.x, eNewPos.y);
							}
							else if (enemy[i].level == 2) //allign and shoot
							{

								if (enemy[i].x == player.x)
								{
									if (frameCount - enemy[i].cooldown > 20)
									{
										if (enemy[i].y < player.y)
											newProjectile(bolt, enemy[i].x, enemy[i].y, 0, 1, -1, 18, 'o', 6, 4, 7, 20, false);
										else
											newProjectile(bolt, enemy[i].x, enemy[i].y, 0, -1, -1, 18, 'o', 6, 4, 7, 20, false);
										enemy[i].cooldown = frameCount;
									}
									else
									{
										if (enemy[i].y < player.y)
										{
											if (isFreePosition(enemy, enemy[i].x, enemy[i].y + 1))
												setPosition(&enemy[i], enemy[i].x, enemy[i].y + 1);
										}
										else
										{
											if (isFreePosition(enemy, enemy[i].x, enemy[i].y - 1))
												setPosition(&enemy[i], enemy[i].x, enemy[i].y - 1);
										}
									}
								}
								else if (enemy[i].y == player.y)
								{
									if (frameCount - enemy[i].cooldown > 20)
									{
										if (enemy[i].x < player.x)
											newProjectile(bolt, enemy[i].x, enemy[i].y, 1, 0, -1, 18, 'o', 6, 4, 7, 20, false);
										else
											newProjectile(bolt, enemy[i].x, enemy[i].y, -1, 0, -1, 18, 'o', 6, 4, 7, 20, false);
										enemy[i].cooldown = frameCount;
									}
									else
									{
										if (enemy[i].x < player.x)
										{
											if (isFreePosition(enemy, enemy[i].x + 1, enemy[i].y))
												setPosition(&enemy[i], enemy[i].x + 1, enemy[i].y);
										}
										else
										{
											if (isFreePosition(enemy, enemy[i].x - 1, enemy[i].y))
												setPosition(&enemy[i], enemy[i].x - 1, enemy[i].y);
										}
									}
								}
								else
								{
									bool free = false;
									if (abs(player.x - enemy[i].x) <= abs(player.y - enemy[i].y))
									{
										if (player.x > enemy[i].x && isFreePosition(enemy, enemy[i].x + 1, enemy[i].y))
											setPosition(&enemy[i], enemy[i].x + 1, enemy[i].y), free = true;
										else if (isFreePosition(enemy, enemy[i].x - 1, enemy[i].y))
											setPosition(&enemy[i], enemy[i].x - 1, enemy[i].y), free = true;
									}
									if (!free)
									{
										if (player.y > enemy[i].y && isFreePosition(enemy, enemy[i].x, enemy[i].y + 1))
											setPosition(&enemy[i], enemy[i].x, enemy[i].y + 1);
										else if (isFreePosition(enemy, enemy[i].x, enemy[i].y - 1))
											setPosition(&enemy[i], enemy[i].x, enemy[i].y - 1);
									}
								}

							}
							else if (enemy[i].level == 3) //summoner
							{
								if (abs(enemy[i].x - player.x) + abs(enemy[i].y - player.y) > 10)
								{
									if (frameCount % 30 == 0)
									{
										if (isFreePosition(enemy, enemy[i].x - 1, enemy[i].y))
											newEntity(enemy, enemy[i].x - 1, enemy[i].y, 'ù', 1 + rand() % 6, -1, 10, 1);
										if (isFreePosition(enemy, enemy[i].x, enemy[i].y - 1))
											newEntity(enemy, enemy[i].x, enemy[i].y - 1, 'ù', 1 + rand() % 6, -1, 10, 1);
										if (isFreePosition(enemy, enemy[i].x + 1, enemy[i].y))
											newEntity(enemy, enemy[i].x + 1, enemy[i].y, 'ù', 1 + rand() % 6, -1, 10, 1);
										if (isFreePosition(enemy, enemy[i].x, enemy[i].y + 1))
											newEntity(enemy, enemy[i].x, enemy[i].y + 1, 'ù', 1 + rand() % 6, -1, 10, 1);
									}
								}
								else //teleport
								{
									int rndX, rndY;
									do
									{
										rndX = rand() % MAP_SIZE_X; rndY = rand() % MAP_SIZE_Y;
									} while (!isFreePosition(enemy, rndX, rndY) || !isFreePosition(enemy, rndX - 1, rndY) || !isFreePosition(enemy, rndX, rndY - 1) || !isFreePosition(enemy, rndX + 1, rndY) || !isFreePosition(enemy, rndX, rndY + 1) || abs(rndX - player.x) + abs(rndY - player.y) <= 10);
									newProjectile(bolt, enemy[i].x, enemy[i].y, 0, 0, 1, 10, '±', 10, 11, 0, 0, false);
									setPosition(&enemy[i], rndX, rndY);
									newProjectile(bolt, enemy[i].x, enemy[i].y, 0, 0, 1, 10, '±', 10, 11, 0, 0, false);
								}
							}
							else if (enemy[i].level == 4) //turret
							{
								if (frameCount - enemy[i].cooldown > 40)
								{
									newProjectile(bolt, enemy[i].x, enemy[i].y, 1, 0, -1, 15, 'ø', 11, -1, 2, 20, false);
									newProjectile(bolt, enemy[i].x, enemy[i].y, -1, 0, -1, 15, 'ø', 11, -1, 2, 20, false);
									newProjectile(bolt, enemy[i].x, enemy[i].y, 0, 1, -1, 15, 'ø', 11, -1, 2, 20, false);
									newProjectile(bolt, enemy[i].x, enemy[i].y, 0, -1, -1, 15, 'ø', 11, -1, 2, 20, false);
									enemy[i].cooldown = frameCount;
								}
							}
							else if (enemy[i].level == 5) //allign and shoot block
							{

								if (enemy[i].x == player.x)
								{
									if (frameCount - enemy[i].cooldown > 30)
									{
										if (enemy[i].y < player.y)
										{
											newProjectile(bolt, enemy[i].x, enemy[i].y + 1, 0, 1, -1, 19, '°', 4, 3, 7, 30, false);
											newProjectile(bolt, enemy[i].x - 1, enemy[i].y, 0, 1, -1, 19, '°', 4, 3, 7, 30, false);
											newProjectile(bolt, enemy[i].x + 1, enemy[i].y, 0, 1, -1, 19, '°', 4, 3, 7, 30, false);
										}
										else
										{
											newProjectile(bolt, enemy[i].x, enemy[i].y - 1, 0, -1, -1, 19, '°', 4, 3, 7, 30, false);
											newProjectile(bolt, enemy[i].x - 1, enemy[i].y, 0, -1, -1, 19, '°', 4, 3, 7, 30, false);
											newProjectile(bolt, enemy[i].x + 1, enemy[i].y, 0, -1, -1, 19, '°', 4, 3, 7, 30, false);
										}
										enemy[i].cooldown = frameCount;
									}
									else
									{
										if (enemy[i].y < player.y)
										{
											if (isFreePosition(enemy, enemy[i].x, enemy[i].y + 1))
												setPosition(&enemy[i], enemy[i].x, enemy[i].y + 1);
										}
										else
										{
											if (isFreePosition(enemy, enemy[i].x, enemy[i].y - 1))
												setPosition(&enemy[i], enemy[i].x, enemy[i].y - 1);
										}
									}
								}
								else if (enemy[i].y == player.y)
								{
									if (frameCount - enemy[i].cooldown > 30)
									{
										if (enemy[i].x < player.x)
										{
											newProjectile(bolt, enemy[i].x + 1, enemy[i].y, 1, 0, -1, 19, '°', 4, 3, 7, 10, false);
											newProjectile(bolt, enemy[i].x, enemy[i].y - 1, 1, 0, -1, 19, '°', 4, 3, 7, 10, false);
											newProjectile(bolt, enemy[i].x, enemy[i].y + 1, 1, 0, -1, 19, '°', 4, 3, 7, 10, false);
										}
										else
										{
											newProjectile(bolt, enemy[i].x - 1, enemy[i].y, -1, 0, -1, 19, '°', 4, 3, 7, 10, false);
											newProjectile(bolt, enemy[i].x, enemy[i].y - 1, -1, 0, -1, 19, '°', 4, 3, 7, 10, false);
											newProjectile(bolt, enemy[i].x, enemy[i].y + 1, -1, 0, -1, 19, '°', 4, 3, 7, 10, false);
										}
										enemy[i].cooldown = frameCount;
									}
									else
									{
										if (enemy[i].x < player.x)
										{
											if (isFreePosition(enemy, enemy[i].x + 1, enemy[i].y))
												setPosition(&enemy[i], enemy[i].x + 1, enemy[i].y);
										}
										else
										{
											if (isFreePosition(enemy, enemy[i].x - 1, enemy[i].y))
												setPosition(&enemy[i], enemy[i].x - 1, enemy[i].y);
										}
									}
								}
								else
								{
									bool free = false;
									if (abs(player.x - enemy[i].x) <= abs(player.y - enemy[i].y))
									{
										if (player.x > enemy[i].x && isFreePosition(enemy, enemy[i].x + 1, enemy[i].y))
											setPosition(&enemy[i], enemy[i].x + 1, enemy[i].y), free = true;
										else if (isFreePosition(enemy, enemy[i].x - 1, enemy[i].y))
											setPosition(&enemy[i], enemy[i].x - 1, enemy[i].y), free = true;
									}
									if (!free)
									{
										if (player.y > enemy[i].y && isFreePosition(enemy, enemy[i].x, enemy[i].y + 1))
											setPosition(&enemy[i], enemy[i].x, enemy[i].y + 1);
										else if (isFreePosition(enemy, enemy[i].x, enemy[i].y - 1))
											setPosition(&enemy[i], enemy[i].x, enemy[i].y - 1);
									}
								}

							}
						}
						setPosition(&enemy[i], enemy[i].x, enemy[i].y);
					}
				}
				if (player.mana < player.maxMana)
				{
					player.mana += 2;
					setValue(&manaBar, player.mana);
				}
				if (player.isInvincible && player.mana >= 2)
				{
					if (player.hp < player.maxHP)
					{
						player.hp += 5;
						setValue(&healthBar, player.hp);
					}
					player.mana -= 3;
				}
			}

			displayEntity(&player);

			if (doDisplay)
			{
				fullDisplay(&player);
				for (int i = 0; i < MAX_ENTITY; ++i)
					if (enemy[i].isActive) setPosition(&enemy[i], enemy[i].x, enemy[i].y);
				doDisplay = false;
			}

			++frameCount;

			if (player.mana <= 0)
			{
				player.isInvincible = false;
				player.bg = -1;
			}
			if (player.hp <= 0)
			{
				if(killCount > highscore)
					saveHighscore("score.txt", killCount);
				player.symbol[0] = '+';
				player.fg = 8;
				displayEntity(&player);
				Sleep(200);
				int blood[MAP_SIZE_X];
				for (int i = 0; i < MAP_SIZE_X; ++i)
					blood[i] = 3 + rand() % (int)(MAP_SIZE_Y / 1.5);
				for (int i = 0; i < 3 + MAP_SIZE_Y / 1.5; ++i)
				{
					for (int j = 0; j < MAP_SIZE_X; ++j)
						if (blood[j] >= i)
							drawText("Û", j, i, 4, 0);
					Sleep(1);
				}
				Sleep(1200);
				startState = true;
			}
		}
		Sleep(10);
	}
	return 0;
}