#include <windows.h>
#include <string>
#include <iostream>
#include <sstream>

using namespace std;

DWORD WINAPI ThreadProc(CONST LPVOID lpParam)
{
	stringstream message;
	message << "Поток №" << (size_t)lpParam << " выполняет свою работу" << endl;
	cout << message.str();

	ExitThread(0);
}
int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "ru");

	if (argc != 2)
	{
		cout << "Должно быть 2 параметра" << endl;
		return 1;
	}

	size_t numOfThreads = atoi(argv[1]);

	HANDLE* handles = new HANDLE[numOfThreads];
	for (size_t i = 0; i < numOfThreads; i++)
	{
		handles[i] = CreateThread(NULL, 0, &ThreadProc, (LPVOID)(i + 1), CREATE_SUSPENDED,
			NULL);
	}

	for (size_t i = 0; i < numOfThreads; i++)
	{
		ResumeThread(handles[i]);
	}

	WaitForMultipleObjects(numOfThreads, handles, true, INFINITE);
	return 0;
}