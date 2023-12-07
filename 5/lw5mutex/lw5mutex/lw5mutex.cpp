#include <windows.h>
#include <string>
#include <iostream>
#include "tchar.h"
#include <fstream>
#include <mutex>

HANDLE mtx;

int ReadFromFile() {
	std::fstream myfile("balance.txt", std::ios_base::in);
	int result;
	myfile >> result;
	myfile.close();

	return result;
}

void WriteToFile(int data) {
	std::fstream myfile("balance.txt", std::ios_base::out);
	myfile << data << std::endl;
	myfile.close();
}

int GetBalance() {
	int balance = ReadFromFile();
	return balance;
}

void Deposit(int money) {
	int balance = GetBalance();
	balance += money;

	WriteToFile(balance);
	printf("Balance after deposit: %d\n", balance);
}

void Withdraw(int money) {
	if (GetBalance() < money) {
		printf("Cannot withdraw money, balance lower than %d\n", money);
		return;
	}

	Sleep(20);
	int balance = GetBalance();
	balance -= money;
	WriteToFile(balance);
	printf("Balance after withdraw: %d\n", balance);
}

DWORD WINAPI DoDeposit(CONST LPVOID lpParameter)
{
	DWORD dwWaitResult;

	// Request ownership of mutex.
	dwWaitResult = WaitForSingleObject(
		mtx,    // handle to mutex
		INFINITE);  // no time-out interval

	switch (dwWaitResult)
	{
		// The thread got ownership of the mutex
	case WAIT_OBJECT_0:
		__try {
			Deposit((int)lpParameter);
		}

		__finally {
			// Release ownership of the mutex object
			if (!ReleaseMutex(mtx))
			{
				// Handle error.
				std::cout << "Got a error!" << std::endl;
			}
		}
		break;

		// The thread got ownership of an abandoned mutex
		// The database is in an indeterminate state
	case WAIT_ABANDONED:
		return FALSE;
	}
	ExitThread(0);
	return TRUE;
}

DWORD WINAPI DoWithdraw(CONST LPVOID lpParameter)
{
	DWORD dwWaitResult;

	// Request ownership of mutex.

	dwWaitResult = WaitForSingleObject(
		mtx,    // handle to mutex
		INFINITE);  // no time-out interval

	switch (dwWaitResult)
	{
		// The thread got ownership of the mutex
	case WAIT_OBJECT_0:
		__try {
			Withdraw((int)lpParameter);
		}

		__finally {
			// Release ownership of the mutex object
			if (!ReleaseMutex(mtx))
			{
				// Handle error.
				std::cout << "Got a error!" << std::endl;
			}
		}
		break;

		// The thread got ownership of an abandoned mutex
		// The database is in an indeterminate state
	case WAIT_ABANDONED:
		return FALSE;
	}
	ExitThread(0);
	return TRUE;
}

int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE* handles = new HANDLE[50];

	mtx = CreateMutex(NULL, FALSE, _T("mutex"));
	if (mtx == NULL)
	{
		std::cout << "Mutex can't be create" << std::endl;
		return 1;
	}

	WriteToFile(0);

	SetProcessAffinityMask(GetCurrentProcess(), 1);
	for (int i = 0; i < 50; i++) {
		handles[i] = (i % 2 == 0)
			? CreateThread(NULL, 0, &DoDeposit, (LPVOID)230, CREATE_SUSPENDED, NULL)
			: CreateThread(NULL, 0, &DoWithdraw, (LPVOID)1000, CREATE_SUSPENDED, NULL);
		ResumeThread(handles[i]);
	}

	WaitForMultipleObjects(50, handles, true, INFINITE);
	printf("Final Balance: %d\n", GetBalance());

	getchar();

	CloseHandle(mtx);

	return 0;
}