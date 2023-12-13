#pragma once
#define Board SapperCell**
#include <cstdlib>

// Струкутра,, що реалізує одну клітинку поля
struct SapperCell {
	bool initialized = false;
	int bombsCountClose = 0;
	bool haveBomb = false;
	bool marked = false;
	bool opened = false;
	// Встановлює значення клітинки
	bool SetCell(bool isBombed) {
		if (initialized) {
			return false;
		}
		else {
			initialized = true;
			haveBomb = isBombed;
		}
	}
};

// Функція генерує поле з вказаною кількістю бомб
Board GenerateBoard(int width, int height, int bombsCount, int sWidth, int sHeight);
// Функція, що рахує дістанцію між двома точками
int Distance(int x1, int x2, int y1, int y2);
// Визначає, чи потрібно ставити бомбу
bool IsBombSpawned(int squareLeft, int bombsLeft);

// Функція генерує поле з вказаною кількістю бомб
Board GenerateBoard(int width, int height, int bombsCount, int sWidth, int sHeight) {
	int squareLeft = width * height - 1;
	int bombsLeft = bombsCount;
	Board result = new SapperCell*[height];
	// Створення порожнього поля
	for (int i = 0; i < height; i++) {
		result[i] = new SapperCell[width];
		for (int j = 0; j < width; j++) {
			result[i][j] = SapperCell();
		}
	}
	// Ініциалізація початкової клітинки
	result[sHeight][sWidth].SetCell(false);
	// Ініциалізація усього іншого поля
	int maxDistance = width > height ? width : height;
	int cDistance = 1;
	while (maxDistance > cDistance)
	{
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				if (!result[i][j].initialized && bombsLeft > 0) {
					if (Distance(i, sWidth, j, sHeight) <= cDistance) {
						bool bombSpawned = IsBombSpawned(squareLeft, bombsLeft);
						result[i][j].SetCell(bombSpawned);
						if (bombSpawned) bombsLeft--;
						squareLeft--;
					}
				}
			}
		}
		cDistance++;
	}
	// Додання кількості бомб до клітинок
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int bombsClose = 0;
			// Left
			if (i != 0) if (result[i - 1][j].haveBomb) bombsClose++;
			// Left-Up
			if (i != 0 && j != 0) if (result[i - 1][j - 1].haveBomb) bombsClose++;
			// Up
			if (j != 0) if (result[i][j - 1].haveBomb) bombsClose++;
			// Right-Up
			if (j != 0 && i != height - 1) if (result[i + 1][j - 1].haveBomb) bombsClose++;
			// Right
			if (i != height - 1) if (result[i + 1][j].haveBomb) bombsClose++;
			// Right-Down
			if (i != height - 1 && j != width - 1) if (result[i + 1][j + 1].haveBomb) bombsClose++;
			// Down
			if (j != width - 1) if (result[i][j + 1].haveBomb) bombsClose++;
			// Left-Down
			if (i != 0 && j != width - 1) if (result[i - 1][j + 1].haveBomb) bombsClose++;
			result[i][j].bombsCountClose = bombsClose;
		}
	}
	return result;
}

// Функція, що рахує дістанцію між двома точками
int Distance(int x1,int x2,int y1,int y2) {
	return (int)(sqrt(pow(x1 - x2,2) + pow(y1 - y2,2)));
}

// Визначає, чи потрібно ставити бомбу
bool IsBombSpawned(int squareLeft, int bombsLeft) {
	return 0.01 * (std::rand() % 101) > 0.8 - 1.0 / (float)(squareLeft - bombsLeft + 1);
}