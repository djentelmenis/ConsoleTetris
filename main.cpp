#include <iostream>
#include <thread>
#include <Windows.h>
#include <vector>
#include <SDL_mixer.h>

using namespace std;

bool DoesPieceFit(int , int nRotation, int nPosX, int nPosY);
int Rotate(int px, int py, int r);

wstring tetromino[7];
int nFieldWidth = 12;
int nFieldHeight = 18;
unsigned char *pField = nullptr;

int nScreenWidth = 120;		// Console Screen Size X (columns)
int nScreenHeight = 30;		// Console Screen Size X (rows)

int Rotate(int px, int py, int r) {
	switch (r % 4)
	{
	case 0:
		return py * 4 + px;				// 0 degrees
	case 1:
		return 12 + py - (px * 4);		// 90 degrees
	case 2:
		return 15 - (py * 4) - px;		// 180 degrees
	case 3:
		return 3 - py + (px * 4);		// 270 degrees
	}
	return 0;
}

int main() {
	// Init audio
	Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);
	Mix_Music *songIntro = Mix_LoadMUS("intro.wav");
	Mix_Music *songGame = Mix_LoadMUS("tetr.wav");
	Mix_Music *songEnd = Mix_LoadMUS("end.wav");
	Mix_Chunk *soundDown = Mix_LoadWAV("swosh.wav");
	Mix_Chunk *soundLine = Mix_LoadWAV("line.wav");

	//Create assets
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");

	tetromino[1].append(L"..X.");
	tetromino[1].append(L".XX.");
	tetromino[1].append(L".X..");
	tetromino[1].append(L"....");

	tetromino[2].append(L".X.");
	tetromino[2].append(L".XX.");
	tetromino[2].append(L"..X.");
	tetromino[2].append(L"....");

	tetromino[3].append(L"....");
	tetromino[3].append(L".XX.");
	tetromino[3].append(L".XX.");
	tetromino[3].append(L"....");

	tetromino[4].append(L"..X.");
	tetromino[4].append(L".XX.");
	tetromino[4].append(L"..X.");
	tetromino[4].append(L"....");

	tetromino[5].append(L"....");
	tetromino[5].append(L".XX.");
	tetromino[5].append(L"..X.");
	tetromino[5].append(L"..X.");

	tetromino[6].append(L"....");
	tetromino[6].append(L".XX.");
	tetromino[6].append(L".X..");
	tetromino[6].append(L".X..");

	// Intro music
	Mix_PlayMusic(songIntro, -1);

	// Greeting text
	cout << "Hello there," << endl
		<< "This is basically a TETRIS rip-off, so You should know the rules" << endl << endl
		<< "Use the Arrow keys to navigate the pieces and Z to rotate" << endl
		<< "You get extra score for finishing more than one line" << endl << endl
		<< "P.S." << endl
		<< "The speed increaes with time" << endl << endl;
	system("pause");

	bool bNewGame = true;
	// Program run loop ========================================
	system("cls");
	while (bNewGame) {

		pField = new unsigned char[nFieldWidth * nFieldHeight]; // Creates play
		for (int x = 0; x < nFieldWidth; x++)	//Board Boundary
			for (int y = 0; y < nFieldHeight; y++)
				pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;

		// Create Screen Buffer
		wchar_t *screen = new wchar_t[nScreenWidth*nScreenHeight];
		for (int i = 0; i < nScreenWidth*nScreenHeight; i++) screen[i] = L' ';
		HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
		SetConsoleActiveScreenBuffer(hConsole);
		DWORD dwBytesWritten = 0;

		// Game Logic Stuff
		bool bGameOver = false;

		int nCurrentPiece = rand() % 7;
		int nCurrentRotation = 0;
		int nCurrentX = nFieldWidth / 2;
		int nCurrentY = 0;

		bool bKey[4];
		bool bRightHold = false;
		bool bLeftHold = false;
		bool bRotateHold = false;

		int nSpeed = 20;
		int nSpeedCounter = 0;
		bool bForceDown = false;
		int nPieceCount = 0;
		int nScore = 0;
		int nTime = 0;
		double nTimeCounter = 0;

		vector<int> vLines;

		// Play music
		Mix_PlayMusic(songGame, -1);

		while (!bGameOver) {
			// GAME TIMING ========================================
			this_thread::sleep_for(30ms);	//Game Tick
			nSpeedCounter++;
			bForceDown = (nSpeedCounter == nSpeed);

			nTimeCounter += 0.030;
			if (nTimeCounter > 1) {
				nTime++;
				nTimeCounter -= 1;
			}

			// INPUT ==============================================
			for (int k = 0; k < 4; k++)								// R   L   D  Z
				bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;

			// GAME LOGIC =========================================
			nCurrentY += (bKey[2] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;		// Down

			if (bKey[0]) {
				nCurrentX += (!bRightHold && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;	//Right
				bRightHold = true;
			}
			else
				bRightHold = false;

			if (bKey[1]) {
				nCurrentX -= (!bLeftHold && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;	//Left
				bLeftHold = true;
			}
			else
				bLeftHold = false;

			if (bKey[3]) {
				nCurrentRotation += (!bRotateHold && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;	//Rotation
				bRotateHold = true;
			}
			else
				bRotateHold = false;

			if (bForceDown) {
				if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
					nCurrentY++;	//If it can go down, it will go down
				else {
					//Lock current piece in the field
					for (int px = 0; px < 4; px++)
						for (int py = 0; py < 4; py++)
							if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
								pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;

					nPieceCount++;
					if (nPieceCount % 10 == 0)
						if (nSpeed >= 1)
							nSpeed--;

					//Check if there are horizontal lines
					for (int py = 0; py <4; py++)
						if (nCurrentY + py < nFieldHeight - 1) {
							bool bLine = true;
							for (int px = 1; px < nFieldWidth - 1; px++)
								bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;

							if (bLine) {
								//Play Sound
								Mix_PlayChannel(-1, soundLine, 0);

								//Remove Line, set it to "="
								for (int px = 1; px < nFieldWidth - 1; px++)
									pField[(nCurrentY + py) * nFieldWidth + px] = 8;

								vLines.push_back(nCurrentY + py);
							}
						}

					nScore += 25;
					if (!vLines.empty()) nScore += (1 << vLines.size()) * 100;

					//Choose next piece
					nCurrentX = nFieldWidth / 2;
					nCurrentY = 0;
					nCurrentRotation = 0;
					nCurrentPiece = rand() % 7;
					Mix_PlayChannel(-1, soundDown, 0);	//Plays sound

														//If new piece does not fit
					bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
				}

				nSpeedCounter = 0;
			}

			// RENDER OUTPUT ======================================

			// Draw Field
			for (int x = 0; x < nFieldWidth; x++)
				for (int y = 0; y < nFieldHeight; y++)
					screen[(y + 2) * nScreenWidth + (x + 2)] = L" ABCDEFG=#"[pField[y * nFieldWidth + x]];

			// Draw Current Piece
			for (int px = 0; px < 4; px++)
				for (int py = 0; py < 4; py++)
					if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
						screen[(nCurrentY + py + 2) * nScreenWidth + (nCurrentX + px + 2)] = nCurrentPiece + 65;

			// Draw Score & Speed & Time
			swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 16, L"SCORE: % 8d", nScore);
			swprintf_s(&screen[4 * nScreenWidth + nFieldWidth + 6], 16, L"SPEED: % 8d", 20 - nSpeed);
			swprintf_s(&screen[6 * nScreenWidth + nFieldWidth + 6], 16, L"TIME:  % 8d", nTime);

			if (!vLines.empty()) {
				//Display Frame
				WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
				this_thread::sleep_for(400ms); //Delay a bit

				for (auto &v : vLines)
					for (int px = 1; px < nFieldWidth - 1; px++) {
						for (int py = v; py > 0; py--)
							pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
						pField[px] = 0;
					}

				vLines.clear();
			}

			// Display Frame
			WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
		}

		// When Game over
		CloseHandle(hConsole);
		cout << "Game Over!!" << endl << "Score:" << nScore << endl << "Time played: " << nTime << " seconds" << endl << endl << endl;
		Mix_PlayMusic(songEnd, 1);
		this_thread::sleep_for(2000ms);
		char cPlayAgain = 'x';
		while (cPlayAgain != 'y' || 'n') {
			cout << endl << "Play again? y/n" << endl << endl;
			cin >> cPlayAgain;
			if (cPlayAgain == 'y') {
				bGameOver = false;
				cPlayAgain = 'x';
				system("CLS");
				break;
			}
			else if (cPlayAgain == 'n') {
				bNewGame = false;
				break;
			}
		}
	}



	// Close audio
	Mix_FreeMusic(songGame);
	Mix_FreeMusic(songEnd);
	Mix_FreeChunk(soundDown);
	Mix_FreeChunk(soundLine);
	Mix_CloseAudio();

	return 0;
}

bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY) {
	for (int px = 0; px < 4; px++)
		for (int py = 0; py < 4; py++) {
			// Get index into piece
			int pi = Rotate(px, py, nRotation);

			// Get index into field
			int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

			if (nPosX + px >= 0 && nPosX + px < nFieldWidth) {
				if (nPosY + px >= 0 && nPosY + px < nFieldHeight) {
					if (tetromino[nTetromino][pi] == L'X' && pField[fi] != 0)
						return false; // fail on first hit
				}
			}
		}

	return true;
}