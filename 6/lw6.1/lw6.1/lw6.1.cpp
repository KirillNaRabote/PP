#include <iostream>
#include <omp.h>
#include <string>
#include <chrono>

using namespace std;

int main()
{
	setlocale(LC_ALL, "ru");

	int operationNumber = 10000000;

	//с использованием for

	auto start = chrono::steady_clock::now();

	double pi = 0.0;

	for (int i = 0; i <= operationNumber; i++)
	{
		pi += pow(-1.0, i) / (2 * i + 1) * 4;
	}

	auto end = chrono::steady_clock::now();
	chrono::duration<double> diff = end - start;

	cout << "pi = " << pi << " (с использованием for). Время выполнения: " << diff.count() << endl;

	//с использованием for


	//с использованием parallel for

	start = chrono::steady_clock::now();

	pi = 0.0;

#pragma omp parallel for
	for (int i = 0; i <= operationNumber; i++)
	{
		pi += pow(-1.0, i) / (2 * i + 1) * 4;
	}

	end = chrono::steady_clock::now();
	diff = end - start;

	cout << "pi = " << pi << " (с использованием parallel for). Время выполнения: " << diff.count() << endl;

	//с использованием parallel for



	//с использованием parallel for и atomic

	start = chrono::steady_clock::now();

	pi = 0.0;

#pragma omp parallel for
	for (int i = 0; i <= operationNumber; i++)
	{
#pragma omp atomic
		pi += pow(-1.0, i) / (2 * i + 1) * 4;
	}

	end = chrono::steady_clock::now();
	diff = end - start;

	cout << "pi = " << pi << " (с использованием parallel for и atomic). Время выполнения: " << diff.count() << endl;

	//с использованием parallel for и atomic



	//с использованием reduction

	start = chrono::steady_clock::now();

	pi = 0.0;

#pragma omp parallel for reduction(+:pi)
	for (int i = 0; i <= operationNumber; i++)
	{
		pi += pow(-1.0, i) / (2 * i + 1) * 4;
	}

	end = chrono::steady_clock::now();
	diff = end - start;

	cout << "pi = " << pi << " (с использованием reduction). Время выполнения: " << diff.count() << endl;

	//с использованием reduction
}