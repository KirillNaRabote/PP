#include <windows.h>
#include <string>
#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include <fstream>
#include <sstream>

using namespace std;

struct ThreadProcParam
{
	size_t numOfThread;
	int numOfOper;
	std::chrono::time_point<std::chrono::system_clock> startTime;
	string outputFileName;
};

std::vector<int> generateRandomNumbers(int n) {
	std::random_device rd;
	std::mt19937 gen(rd());

	std::vector<int> randomNumbers(n);

	for (int i = 0; i < n; i++) {
		randomNumbers[i] = gen();
	}

	return randomNumbers;
}

template <typename T>
int partition(std::vector<T>& arr, int low, int high) {
	T pivot = arr[high];
	int i = low - 1;
	for (int j = low; j < high; j++) {
		if (arr[j] < pivot) {
			i++;
			std::swap(arr[i], arr[j]);
		}
	}
	std::swap(arr[i + 1], arr[high]);
	return i + 1;
}

template <typename T>
void quickSort(std::vector<T>& arr, int low, int high) {
	if (low < high) {
		int pi = partition(arr, low, high);
		quickSort(arr, low, pi - 1);
		quickSort(arr, pi + 1, high);
	}
}

DWORD WINAPI ThreadProc(CONST LPVOID lpParam)
{
	ThreadProcParam& param = (*(ThreadProcParam*)lpParam);
	size_t numOfThread = (*(ThreadProcParam*)lpParam).numOfThread;
	int numOfOper = (*(ThreadProcParam*)lpParam).numOfOper;
	const auto startTime = (*(ThreadProcParam*)lpParam).startTime;
	string outputFileName = (*(ThreadProcParam*)lpParam).outputFileName;

	ofstream outputFile(outputFileName, ofstream::app);

	while (numOfOper)
	{
		vector<int> vec = generateRandomNumbers(500);
		quickSort(vec, 0, vec.size() - 1);

		auto endTime = chrono::system_clock::now();
		chrono::duration<double> diff = endTime - startTime;
		stringstream msg;
		msg << param.numOfThread << "|" << diff.count() << endl;

		outputFile << msg.str();

		numOfOper--;
	}

	ExitThread(0);
}

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		cout << "Invalid parameters" << endl
			<< "Usage: lw3.exe <output file> <number of operations>" << endl;

		return 1;
	}

	system("pause");

	const auto startTime = chrono::system_clock::now();

	size_t numOfOper = atoi(argv[2]);
	size_t numOfThreads = 2;

	string outputFileName = argv[1];
	ofstream outputFile(outputFileName, ofstream::trunc);
	outputFile.close();

	HANDLE* handles = new HANDLE[numOfThreads];

	vector<ThreadProcParam> args;

	for (size_t i = 0; i < numOfThreads; i++)
	{
		ThreadProcParam param;
		param.numOfThread = i + 1;
		param.numOfOper = numOfOper;
		param.startTime = startTime;
		param.outputFileName = outputFileName;

		args.push_back(param);
	}

	for (size_t i = 0; i < numOfThreads; i++)
	{
		handles[i] = CreateThread(NULL, 0, &ThreadProc, (LPVOID)&(args[i]), NULL,
			NULL);
	}

	WaitForMultipleObjects(numOfThreads, handles, true, INFINITE);

	return 0;
}