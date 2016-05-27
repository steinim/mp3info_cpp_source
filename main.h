#include <windows.h>
#include "CMP3Info.h"

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK MainDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
int openFile(HWND hDlg);
void saveTag(HWND hDlg);
void playFile();

HINSTANCE hInstance;
HWND hMainWnd;
const char szAppName[]="MP3Info Viewer Example";

CMP3Info* mp3Info;