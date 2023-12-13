#include "framework.h"
#include "SapperWindows.h"
#include "resource.h"
#include "SapperLogic.h"
#include <commdlg.h>
#include <string>
#include <fstream>

#define MAX_LOADSTRING 100

// Шлобальні зміні:
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
HWND mainWnd;

bool gameStarted = false;                                       // Чи розпочато гру
bool boardGenerated = false;                                    // Чи було сгенеровано поле
bool gameLosed = false;                                         // Чи гру було програно
int points = 0;                                                 // Кількість очок
int seconds = 0;                                                // Секунд з початку гри


COLORREF backgroundc[] = { RGB(255, 255, 255) , RGB(10, 10, 10) , RGB(20, 20, 255) };
HBRUSH background[] = { CreateSolidBrush(backgroundc[0]),       // Світла
    CreateSolidBrush(backgroundc[1]),                           // Темна
    CreateSolidBrush(backgroundc[2]) };                         // Синя
HBRUSH closed[] = { CreateSolidBrush(RGB(80, 80, 80)),          // Світла
    CreateSolidBrush(RGB(160, 160, 160)),                       // Темна
    CreateSolidBrush(RGB(80, 80, 255)) };                       // Синя
HBRUSH opened[] = { CreateSolidBrush(RGB(235, 235, 235)),       // Світла
    CreateSolidBrush(RGB(220, 220, 220)),                       // Темна
    CreateSolidBrush(RGB(220, 220, 255)) };                     // Синя
HBRUSH bombed[] = { CreateSolidBrush(RGB(200, 0, 0)),           // Світла
    CreateSolidBrush(RGB(120, 0, 0)),                           // Темна
    CreateSolidBrush(RGB(200, 0, 0)) };                         // Синя
HBRUSH marked[] = { CreateSolidBrush(RGB(255, 200, 0)),         // Світла
    CreateSolidBrush(RGB(150, 50, 0)),                          // Темна
    CreateSolidBrush(RGB(200, 0, 0))};                          // Синя
HBRUSH win[] = { CreateSolidBrush(RGB(20, 255, 20)),            // Світла
    CreateSolidBrush(RGB(5, 100, 5)),                           // Темна
    CreateSolidBrush(RGB(20, 255, 20)) };                       // Синя

Board gameBoard;                                                // Ігрове поле

// Параметри гри
int _newdifficult = 0;                                          // Складність обрана через меню "Параметри"
int difficult = 0;                                              // Складність поточної гри
int theme = 0;                                                  // Обрана тема

// Параметри створення поля
int sizeBoard[] = {6, 10, 15};                                  // Розмір поля для різних складностей
int bombsCount[] = {5, 25, 80};                                 // Кількість бомб для різних складностей

// Параметри малювання поля
int sizePr = 50;                                                
int cSize;                                                      // Розмір клітинки
int sBetween;                                                   // Відстань між двома клітинками
int sX;                                                         // Початок поля за Х
int sY;                                                         // Початок поля за Y

char timerText[6] = { '\0' };;                                  // Буффер часу

// Прототипи функцій
ATOM                MyRegisterClass(HINSTANCE hInstance);       // Регістрація класу вікна
BOOL                InitInstance(HINSTANCE, int);               // Збережнення екземпляру та створення головного вікна
DWORD WINAPI        Timer(LPVOID);                              // Функція потоку таймера
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);        // Функція головного вікна
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);          // функція вікна "Про програму"
INT_PTR CALLBACK    Leaders(HWND, UINT, WPARAM, LPARAM);        // Функція вікна "Лідери"   
INT_PTR CALLBACK    Settings(HWND, UINT, WPARAM, LPARAM);       // Функція вікна "Параметри"
INT_PTR CALLBACK    Win(HWND, UINT, WPARAM, LPARAM);            // Функція вікна "Перемога"
std::string         GetChampions();                             // Функція, що повертає таблицю лідерів
void                AddChampion(std::string);                   // Функція, що додає нового лідера
void                RecalcStartPos(HWND);                       // Функція, що розраховує параметри малювання поля
void                DrawRectangle(HDC, int, int);               // Функція, що малює ігрову клітинку
void                CheckRectangleClicked(HWND, int, int);      // Функція, що перевіряє чи було натиснено саме на клітинку  
void                MarkUpRectangle(HWND, int, int);            // Функція, що помічає клітинку, як ту, що може мати бомбу
void                InvokePaintWindow(HWND);                    // Функція, що викликає перемалювання вікна
void                CheckOnEmpty(int, int);                     // Функція, що відкриває усі клітинки з 0 бомб, навколо вказаної клітинки
void                DrawTimer(HDC);                             // Функція, що малює таймер
bool                IsWin();                                    // функція, що перевіряє чи виграна гра
void                ReadSettings();                             // Зчитуваня параметрів з файлу
void                WriteSettings();                            // Збереження параметрів до файлу
void                ReadGame();                                 // Зчитування гри з файлу
void                WriteGame();                                // Збереження гри до файлу

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    MyRegisterClass(hInstance);
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SAPPERWINDOWS));
    MSG msg;
    // Цикл повідомлень
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return (int) msg.wParam;
}

//  Регістрація класу вікна
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXA wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SAPPERWINDOWS));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEA(IDC_SAPPERWINDOWS);
    wcex.lpszClassName  = "SAPPERAPP";
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExA(&wcex);
}

//  Збережнення екземпляру та створення головного вікна
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance;
   _CrtSetDebugFillThreshold(0);
   HWND hWnd = CreateWindowA("SAPPERAPP", "Сапер", WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }
   mainWnd = hWnd;
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

// Функція головного вікна
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    mainWnd = hWnd;
    POINT pt;
    switch (message)
    {
    case WM_CREATE:
    {
        // Створення таймеру та зчитування параметрів при створенні вікна
        CreateThread(NULL, 0, Timer, NULL, 0, 0);
        ReadSettings();
    }
    break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Опрацювання всіх команд
            switch (wmId)
            {
            // Виклик вікна "Про програму"
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            // Виклик вікна "Параметри"
            case IDM_SETTINGS:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, Settings);
                break;
            // Закриття програми
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            // Початок гри
            case IDM_STARTGAME:
                // Зміна складності на нову
                if (_newdifficult != difficult) {
                    difficult = _newdifficult;
                }
                // Очищеня парамтрів минулої гри
                points = 0;
                gameLosed = false;
                gameStarted = true;
                boardGenerated = false;
                InvokePaintWindow(hWnd);
                break;
            case IDM_SaveGame:
                // Виклик збереження гри
                WriteGame();
                break;
            case IDM_LoadGame:
                // Виклик зчитування гри з файлу
                ReadGame();
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            // Зміна складності на нову, якщо гру ще не було рощпочато
            if (!boardGenerated && _newdifficult != difficult) {
                difficult = _newdifficult;
            }
            RECT winRect;
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // Положення відображення складності гри
            RECT diffPlacement;
            diffPlacement.left = 60;
            diffPlacement.right = 120;
            diffPlacement.top = 20;
            diffPlacement.bottom = 40;
            GetWindowRect(hWnd, &winRect);
            SetBkMode(hdc, TRANSPARENT);
            SelectObject(hdc, background[theme]);
            // Малювання фону
            Rectangle(hdc, 0, 0, 2 * (winRect.right - winRect.left), 2 * (winRect.bottom -  winRect.top));
            // Малювання складності
            if (difficult == 0) {
                SetTextColor(hdc, RGB(20, 255, 20));
                DrawText(hdc, "Легко", 5, &diffPlacement, DT_CENTER | DT_VCENTER);
            } else if (difficult == 1) {
                SetTextColor(hdc, RGB(255, 200, 0));
                DrawText(hdc, "Середнє", 7, &diffPlacement, DT_CENTER | DT_VCENTER);
            } else {
                SetTextColor(hdc, RGB(255, 0, 0));
                DrawText(hdc, "Важко", 5, &diffPlacement, DT_CENTER | DT_VCENTER);
            }
            // Перерахування параметрів малювання
            RecalcStartPos(hWnd);
            // Малювання клітинок
            for (int i = 0; i < sizeBoard[difficult]; i++) {
                for (int j = 0; j < sizeBoard[difficult]; j++) {
                    DrawRectangle(hdc, i, j);
                }
            }
            // Малювання таймеру
            DrawTimer(hdc);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        // Збереження параметрів, перед виходом з програми
        WriteSettings();
        PostQuitMessage(0);
        break;
    case WM_LBUTTONDOWN:
        // Перевірка, чи було натиснено на клітинку
        GetCursorPos(&pt);
        ScreenToClient(hWnd, &pt);
        CheckRectangleClicked(hWnd, pt.x, pt.y);
        break;
    case WM_RBUTTONDOWN:
        // Виклик маркування клітинки
        GetCursorPos(&pt);
        ScreenToClient(hWnd, &pt);
        MarkUpRectangle(hWnd, pt.x, pt.y);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Функція таймера
DWORD WINAPI Timer(LPVOID param) {
    HDC hdc = GetDC(mainWnd);
    while (true) {
        Sleep(1000);
        if (!gameLosed && boardGenerated) seconds++;
        DrawTimer(hdc);
    }
}

// Функція вікна "Про програму"
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// Функція вікна "Лідери"
INT_PTR CALLBACK Leaders(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_PAINT: {
        return (INT_PTR)TRUE;
    }
    case WM_INITDIALOG: {
        // Відображення списку лідерів при створенні вікна
        SetDlgItemText(hDlg, IDC_LEADERS, GetChampions().c_str());
        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// Функція вікна "Параметри"
INT_PTR CALLBACK Settings(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        // Відображення поточних параметрів
        if (_newdifficult == 0)
            CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO3, IDC_RADIO1);
        else if (_newdifficult == 1)
            CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO3, IDC_RADIO2);
        else if (_newdifficult == 2)
            CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO3, IDC_RADIO3);
        if (theme == 0)
            CheckRadioButton(hDlg, IDC_RADIO4, IDC_RADIO6, IDC_RADIO4);
        else if (theme == 1)
            CheckRadioButton(hDlg, IDC_RADIO4, IDC_RADIO6, IDC_RADIO5);
        else if (theme == 2)
            CheckRadioButton(hDlg, IDC_RADIO4, IDC_RADIO6, IDC_RADIO6);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            // Збереження нових параметрів
            if (LOWORD(wParam) == IDOK) {
                if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO1)) {
                    _newdifficult = 0;
                }
                else if (BST_CHECKED ==IsDlgButtonChecked(hDlg, IDC_RADIO2)) {
                    _newdifficult = 1;
                }
                else if (BST_CHECKED ==IsDlgButtonChecked(hDlg, IDC_RADIO3)) {
                    _newdifficult = 2;
                }
                if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO4)) {
                    theme = 0;
                }
                else if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO5)) {
                    theme = 1;
                }
                else if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO6)) {
                    theme = 2;
                }
            }
            InvokePaintWindow(mainWnd);
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// Функція вікна "Перемога"
INT_PTR CALLBACK Win(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            // Додання нового лідеру
            char* winN = new char[100];
            GetDlgItemText(hDlg, IDC_NAMEPLACE, winN, 100);
            std::string newChampion = winN;
            AddChampion(winN);
            EndDialog(hDlg, LOWORD(wParam));
            DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG2), NULL, Leaders);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// Функція, що розраховує параметри малюваня
void RecalcStartPos(HWND hWnd) {
    RECT winRect;
    GetWindowRect(hWnd, &winRect);
    int lowest = winRect.right - winRect.left > winRect.bottom - winRect.top ? winRect.bottom - winRect.top : winRect.right - winRect.left;
    int cells = winRect.right - winRect.left > winRect.bottom - winRect.top ? sizeBoard[difficult] : sizeBoard[difficult];
    cSize = (lowest * sizePr / 100) / cells;
    sBetween = cSize * sizePr / 100;
    sX =
        (int)((float)(winRect.right - winRect.left - (sizeBoard[difficult] * (cSize + sBetween))) / 2);
    sY =
        (int)(winRect.bottom - winRect.top - (sizeBoard[difficult] * (cSize + sBetween) + sBetween)) / 2;
}

// Фунція малювання клітинки
void DrawRectangle(HDC hdc, int h, int w) {
    RECT rect = RECT();
    rect.left = sX + w * (sBetween + cSize);
    rect.top = sY + h * (sBetween + cSize);
    rect.right = rect.left + cSize;
    rect.bottom = rect.top + cSize;
    if (boardGenerated) {
        if (!gameBoard[h][w].opened) {
            if (gameLosed && gameBoard[h][w].haveBomb) {
                SelectObject(hdc, bombed[theme]);
            }
            else if (IsWin() && gameBoard[h][w].haveBomb) {
                SelectObject(hdc, win[theme]);
            }
            else if (gameBoard[h][w].marked) {
                SelectObject(hdc, marked[theme]);
            } else {
                SelectObject(hdc, closed[theme]);
            }
        }
        else if (!gameBoard[h][w].haveBomb) {
            SelectObject(hdc, opened[theme]);
        }
        else if (gameBoard[h][w].haveBomb) {
            SelectObject(hdc, bombed[theme]);
        }
        Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
        rect.top += cSize / 6;
        if (!gameBoard[h][w].haveBomb && gameBoard[h][w].opened) {
            // Малювання кількості бомб (кожна цифра свого кольору)
            switch (gameBoard[h][w].bombsCountClose)
            {
            case 1:
                SetTextColor(hdc, RGB(0, 160, 0));
                DrawText(hdc, "1", 1, &rect, DT_CENTER | DT_VCENTER);
                break;
            case 2:
                SetTextColor(hdc, RGB(80, 160, 0));
                DrawText(hdc, "2", 1, &rect, DT_CENTER | DT_VCENTER);
                break;
            case 3:
                SetTextColor(hdc, RGB(40, 40, 80));
                DrawText(hdc, "3", 1, &rect, DT_CENTER | DT_VCENTER);
                break;
            case 4:
                SetTextColor(hdc, RGB(80, 0, 80));
                DrawText(hdc, "4", 1, &rect, DT_CENTER | DT_VCENTER);
                break;
            case 5:
                SetTextColor(hdc, RGB(40, 0, 160));
                DrawText(hdc, "5", 1, &rect, DT_CENTER | DT_VCENTER);
                break;
            case 6:
                SetTextColor(hdc, RGB(160, 40, 80));
                DrawText(hdc, "6", 1, &rect, DT_CENTER | DT_VCENTER);
                break;
            case 7:
                SetTextColor(hdc, RGB(160, 40, 0));
                DrawText(hdc, "7", 1, &rect, DT_CENTER | DT_VCENTER);
                break;
            case 8:
                SetTextColor(hdc, RGB(200, 40, 0));
                DrawText(hdc, "8", 1, &rect, DT_CENTER | DT_VCENTER);
                break;
            default:
                break;
            }
        }
    }
    else {
        SelectObject(hdc, closed[theme]);
        Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
    }
}

// Перевірка чи було натиснено на клітинку
void CheckRectangleClicked(HWND hWnd, int x, int y) {
    if ((x-sX) % (cSize + sBetween) <= cSize && (y-sY) % (cSize + sBetween) <= cSize
        && (x-sX) < (cSize + sBetween) * sizeBoard[difficult] && (y - sY) < (cSize + sBetween) * sizeBoard[difficult]
        && x - sX > 0 && y - sY > 0) {
        int xInd = (int)((x - sX) / (cSize + sBetween));
        int yInd = (int)((y - sY) / (cSize + sBetween));
        if (!boardGenerated) {
            gameBoard = GenerateBoard(sizeBoard[difficult], sizeBoard[difficult], bombsCount[difficult], xInd, yInd);
            seconds = 0;
            CheckOnEmpty(xInd, yInd);
            boardGenerated = true;
        }
        else if (!gameLosed) {
            CheckOnEmpty(xInd, yInd);
            if (gameBoard[yInd][xInd].haveBomb)
                gameLosed = true;
        }
        InvokePaintWindow(hWnd);
        if (IsWin() && !gameLosed) {
            DialogBox(hInst, MAKEINTRESOURCE(IDD_WINDIALOG), hWnd, Win);
        }
    }
}

// Маркування клітинки
void MarkUpRectangle(HWND hWnd, int x, int y) {
    if (boardGenerated) {
        if ((x - sX) % (cSize + sBetween) <= cSize && (y - sY) % (cSize + sBetween) <= cSize
            && (x - sX) < (cSize + sBetween) * sizeBoard[difficult] && (y - sY) < (cSize + sBetween) * sizeBoard[difficult]) {
            int xInd = (int)((x - sX) / (cSize + sBetween));
            int yInd = (int)((y - sY) / (cSize + sBetween));
            if (!gameBoard[yInd][xInd].opened)
                gameBoard[yInd][xInd].marked = !gameBoard[yInd][xInd].marked;
            InvokePaintWindow(hWnd);
        }
    }
}

// Виклик перемалювання головного вікна
void InvokePaintWindow(HWND hWnd) {
    UpdateWindow(hWnd);
    RECT rectWin;
    RECT rect = RECT();
    GetWindowRect(hWnd, &rectWin);
    rect.left = 0;
    rect.top;
    rect.right = rectWin.left - rect.right;
    rect.bottom = rectWin.bottom - rectWin.top;
    InvalidateRect(hWnd, &rectWin, true);
}

// Функція, що відкриває усі клітинки з 0 бомб, навколо вказаної клітинки
void CheckOnEmpty(int x, int y) {
    if (x >= 0 && x < sizeBoard[difficult] && y >= 0 && y < sizeBoard[difficult] && !IsWin()) {
        if (gameBoard[y][x].bombsCountClose == 0 && !gameBoard[y][x].opened) {
            gameBoard[y][x].opened = true;
            points++;
            CheckOnEmpty(x - 1, y - 1);
            CheckOnEmpty(x - 1, y);
            CheckOnEmpty(x - 1, y + 1);
            CheckOnEmpty(x, y - 1);
            CheckOnEmpty(x, y);
            CheckOnEmpty(x, y + 1);
            CheckOnEmpty(x + 1, y - 1);
            CheckOnEmpty(x + 1, y);
            CheckOnEmpty(x + 1, y + 1);
        }
        else {
            if (!gameBoard[y][x].opened) {
                gameBoard[y][x].opened = true;
                points++;
            }
        }
    }
}

// Малювання таймеру
void DrawTimer(HDC hdc) {
    int _minutes;
    int _seconds;
    _minutes = seconds / 60;
    _seconds = seconds % 60;
    SelectObject(hdc, background[theme]);
    SetBkColor(hdc, backgroundc[theme]);
    if (difficult == 0) {
        SetTextColor(hdc, RGB(20, 255, 20));
    }
    else if (difficult == 1) {
        SetTextColor(hdc, RGB(255, 200, 0));
    }
    else {
        SetTextColor(hdc, RGB(255, 0, 0));
    }
    if (_minutes < 10) {
        timerText[0] = '0';
        timerText[1] = (char)('0' + _minutes);
    }
    else {
        timerText[0] = (char)('0' + _minutes / 10);
        timerText[1] = (char)('0' + _minutes % 10);
    }
    timerText[2] = ':';
    if (_seconds < 10) {
        timerText[3] = '0';
        timerText[4] = (char)('0' + _seconds);
    }
    else {
        timerText[3] = (char)('0' + _seconds / 10);
        timerText[4] = (char)('0' + _seconds % 10);
    }
    TextOutA(hdc, 20, 20, timerText, strlen(timerText));
}

// Чи виграно гру
bool IsWin() {
    return points == (sizeBoard[difficult] * sizeBoard[difficult]) - bombsCount[difficult];
}

// Збереження параметрів гри
void WriteSettings() {
    CreateDirectoryA("C:\\Users\\toste\\AppData\\Roaming\\SapperApp", NULL);
    HANDLE file = CreateFileA(
        "C:\\Users\\toste\\AppData\\Roaming\\SapperApp\\Settings.txt",
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    DWORD writted;
    char buff[2];
    _itoa_s(_newdifficult, buff, 10);
    WriteFile(file, buff, 1, &writted, NULL);
    _itoa_s(theme, buff, 10);
    WriteFile(file, buff, 1, &writted, NULL);
    CloseHandle(file);
}

// Зчитування параметрів гри
void ReadSettings() {
    CreateDirectoryA("C:\\Users\\toste\\AppData\\Roaming\\SapperApp", NULL);
    HANDLE file = CreateFileA(
        "C:\\Users\\toste\\AppData\\Roaming\\SapperApp\\Settings.txt",
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (GetLastError() != ERROR_FILE_NOT_FOUND) {
        DWORD readed;
        char buff[2];
        ReadFile(file, buff, 2, &readed, NULL);
        _newdifficult = buff[0] - '0';
        theme = buff[1] - '0';
    }
    CloseHandle(file);
}

OPENFILENAME file;
char filename[260];

// Зчитування гри з файлу
void ReadGame() {
    ZeroMemory(&file, sizeof(file));
    file.hwndOwner = mainWnd;
    file.lStructSize = sizeof(OPENFILENAME);
    file.lpstrFile = filename;
    file.nMaxFile = sizeof(filename);
    file.lpstrFileTitle = NULL;
    file.nMaxFileTitle = 0;
    file.lpstrFilter = { "Sapper file\0*.sf\0\0" };
    file.lpstrInitialDir = NULL;
    file.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    DWORD dwCounter;
    if (GetOpenFileNameA(&file)) {
        HANDLE openedFile = CreateFile(filename, GENERIC_READ, 0, NULL,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        char buffint[10] = {'\0'};
        char buffbool[2] = { '\0' };;
        // Секунди
        ReadFile(openedFile, buffint, sizeof(buffint), &dwCounter, NULL);
        seconds = atoi(buffint);
        // Складність
        ReadFile(openedFile, buffint, sizeof(buffint), &dwCounter, NULL);
        difficult = atoi(buffint);
        // Відтворення стола
        gameBoard = new SapperCell * [sizeBoard[difficult]];
        for (int i = 0; i < sizeBoard[difficult]; i++) {
            gameBoard[i] = new SapperCell[sizeBoard[difficult]];
            for (int j = 0; j < sizeBoard[difficult]; j++) {
                gameBoard[i][j] = SapperCell();
                // Кількість бомб поруч
                ReadFile(openedFile, buffint, sizeof(buffint), &dwCounter, NULL);
                gameBoard[i][j].bombsCountClose = atoi(buffint);
                // Чи є бомба в клітці
                ReadFile(openedFile, buffbool, sizeof(buffbool), &dwCounter, NULL);
                gameBoard[i][j].haveBomb = atoi(buffbool);
                // Чи марковано клітку
                ReadFile(openedFile, buffbool, sizeof(buffbool), &dwCounter, NULL);
                gameBoard[i][j].marked = atoi(buffbool);
                // Чи відкрита клітка
                ReadFile(openedFile, buffbool, sizeof(buffbool), &dwCounter, NULL);
                gameBoard[i][j].opened = atoi(buffbool);
            }
        }
        gameStarted = true;
        boardGenerated = true;
        gameLosed = false;
        _newdifficult = difficult;
        CloseHandle(openedFile);
        InvokePaintWindow(mainWnd);
    }
}

// Збереження гри до файлу
void WriteGame() {
    if (!boardGenerated || gameLosed) return;
    ZeroMemory(&file, sizeof(file));
    file.hwndOwner = mainWnd;
    file.lStructSize = sizeof(OPENFILENAME);
    file.lpstrFile = filename;
    file.nMaxFile = sizeof(filename);
    file.lpstrDefExt = "sf";
    file.nMaxFileTitle = 0;
    file.lpstrFilter = { "Sapper file\0*.sf\0\0" };
    file.lpstrInitialDir = NULL;
    file.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    DWORD dwCounter;
    if (GetSaveFileNameA(&file))  {
        HANDLE openedFile = CreateFile(filename, GENERIC_WRITE, 0, NULL,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        char buffint[10];
        char buffbool[2];
        // Секунди
        _itoa_s(seconds, buffint, 10);
        WriteFile(openedFile, buffint, sizeof(buffint), &dwCounter, NULL);
        // Складність
        _itoa_s(difficult, buffint, 10);
        WriteFile(openedFile, buffint, sizeof(buffint), &dwCounter, NULL);
        for (int i = 0; i < sizeBoard[difficult]; i++) {
            for (int j = 0; j < sizeBoard[difficult]; j++) {
                // Кількість бомб поруч
                _itoa_s(gameBoard[i][j].bombsCountClose, buffint, 10);
                WriteFile(openedFile, buffint, sizeof(buffint), &dwCounter, NULL);
                // Чи є бомба в клітці
                _itoa_s((int)gameBoard[i][j].haveBomb, buffbool, 10);
                WriteFile(openedFile, buffbool, sizeof(buffbool), &dwCounter, NULL);
                // Чи марковано клітку
                _itoa_s((int)gameBoard[i][j].marked, buffbool, 10);
                WriteFile(openedFile, buffbool, sizeof(buffbool), &dwCounter, NULL);
                // Чи відкрита клітка
                _itoa_s((int)gameBoard[i][j].opened, buffbool, 10);
                WriteFile(openedFile, buffbool, sizeof(buffbool), &dwCounter, NULL);
            }
        }
        CloseHandle(openedFile);
    }
}

// Додавання нового лідеру
void AddChampion(std::string ChampionName) {
    std::ifstream file;
    if (difficult == 0) {
        file.open("champions0.txt");
    }
    else if (difficult == 1) {
        file.open("champions1.txt");
    }
    else {
        file.open("champions2.txt");
    }
    int countOfChamps = 1;
    std::string* champions = new std::string[1];
    int* champ_times = new int[1];
    if (file.peek() != std::ifstream::traits_type::eof()) {
        file >> countOfChamps;
        file.ignore(1);
        countOfChamps++;
        champions = new std::string[countOfChamps];
        champ_times = new int[countOfChamps];
        for (int i = 0; i < countOfChamps - 1; i++) {
            std::getline(file, champions[i]);
            file >> champ_times[i];
            file.ignore(1);
        }
        
    }
    champions[countOfChamps - 1] = ChampionName;
    champ_times[countOfChamps - 1] = seconds;
    file.close();

    int left = 0, right = countOfChamps - 1;
    while (left < right) {
        for (int i = left; i < right; i++) {
            if (champ_times[i] > champ_times[i+1]) {
                int tempi = champ_times[i];
                std::string temps = champions[i];
                champ_times[i] = champ_times[i + 1];
                champions[i] = champions[i + 1];
                champ_times[i + 1] = tempi;
                champions[i + 1] = temps;
            }
        }
        right--;
        for (int i = right; i > left; i--) {
            if (champ_times[i - 1] > champ_times[i]) {
                int tempi = champ_times[i];
                std::string temps = champions[i];
                champ_times[i] = champ_times[i - 1];
                champions[i] = champions[i - 1];
                champ_times[i - 1] = tempi;
                champions[i - 1] = temps;
            }
        }
    }

    std::ofstream fileo;
    if (difficult == 0) {
        fileo.open("champions0.txt", std::ios_base::trunc);
    }
    else if (difficult == 1) {
        fileo.open("champions1.txt", std::ios_base::trunc);
    }
    else {
        fileo.open("champions2.txt", std::ios_base::trunc);
    }
    fileo << countOfChamps << "\n";
    for (int i = 0; i < countOfChamps; i++) {
        fileo << champions[i] << "\n";
        fileo << champ_times[i] << "\n";
    }
    fileo.close();
}

// Отримання лідерів
std::string GetChampions() {
    std::ifstream file;
    if (difficult == 0) {
        file.open("champions0.txt");
    }
    else if (difficult == 1) {
        file.open("champions1.txt");
    }
    else {
        file.open("champions2.txt");
    }
    int countOfChamps;
    file >> countOfChamps;
    file.ignore(1);
    std::string champions = "";
    std::string buff;
    std::string timer_t = "00:00";
    int sec;
    for (int i = 0; i < countOfChamps; i++) {
        std::getline(file, buff);
        champions += buff;
        file >> sec;
        file.ignore(1);
        int _seconds = sec % 60;
        int _minutes = sec / 60;
        if (_minutes < 10) {
            timer_t[0] = '0';
            timer_t[1] = (char)('0' + _minutes);
        }
        else {
            timer_t[0] = (char)('0' + _minutes / 10);
            timer_t[1] = (char)('0' + _minutes % 10);
        }
        timer_t[2] = ':';
        if (_seconds < 10) {
            timer_t[3] = '0';
            timer_t[4] = (char)('0' + _seconds);
        }
        else {
            timer_t[3] = (char)('0' + _seconds / 10);
            timer_t[4] = (char)('0' + _seconds % 10);
        }
        champions += (std::string)" " + timer_t + "\r\n";
    }
    file.close();
    return champions;
}