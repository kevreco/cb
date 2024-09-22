#include <windows.h>
#include <winuser.h>

int main(void)
{
	MessageBoxW(NULL, L"Window Title!", L"Caption!", MB_OK);
	return 0;
}