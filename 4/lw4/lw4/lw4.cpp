#include <opencv2/opencv.hpp>
#include <fstream>
#include <vector>
#include <windows.h>
#include <chrono>

using namespace std;

struct BlurImageParam
{
    cv::Mat* image;
    cv::Mat* result;

    size_t startRow;
    size_t endRow;
};

DWORD WINAPI BlurImage(const LPVOID lpParam)
{
    size_t startRow = (*(BlurImageParam*)lpParam).startRow;
    size_t endRow = (*(BlurImageParam*)lpParam).endRow;
    cv::Mat* image = (*(BlurImageParam*)lpParam).image;
    cv::Mat* result = (*(BlurImageParam*)lpParam).result;

    for (int i = startRow; i < endRow; i++)
    {
        cv::GaussianBlur((*image).row(i), (*result).row(i), cv::Size(5, 5), 0);
    }

    ExitThread(0);
}

int main(int argc, char* argv[])
{
    if (argc != 5)
    {
        cout << "Invalid parameters" << endl
            << "Usage: lw2.exe <InputFileName> <OutputFileName> <ThreadsNumber> <CoresNumber>" << endl;
        return 1;
    }

    string inFileName = argv[1];
    string outFileName = argv[2];
    int threadsNumber = stoi(argv[3]);
    int coresNumber = stoi(argv[4]);

    cv::Mat image = cv::imread(inFileName);
    if (image.empty())
    {
        cout << "Error loading image!" << endl;
        return 1;
    }

    if (threadsNumber < 1 || threadsNumber > 16)
    {
        cout << "threadsNumber must be [1; 16]" << endl;
        return 1;
    }

    if (coresNumber < 1 || coresNumber > 4)
    {
        cout << "coresNumber must be [1; 4]" << endl;
        return 1;
    }

    cv::Mat blurredImage(image.size(), image.type());

    vector<int> affinity = { 1, 3, 7, 15 };
    int mask = affinity[coresNumber - 1];
    SetProcessAffinityMask(GetCurrentProcess(), (DWORD_PTR)mask);

    auto start = chrono::high_resolution_clock::now();

    int rowsPerThread = image.rows / threadsNumber;

    HANDLE* threads = new HANDLE[threadsNumber];

    vector<BlurImageParam> args;
    for (size_t i = 0; i < threadsNumber; i++)
    {
        size_t startRow = i * rowsPerThread;
        size_t endRow = (i == threadsNumber - 1) ? image.rows : (i + 1) * rowsPerThread;
        BlurImageParam param;
        param.image = &image;
        param.result = &blurredImage;
        param.startRow = startRow;
        param.endRow = endRow;

        args.push_back(param);
    }

    for (size_t i = 0; i < threadsNumber; i++)
    {
        threads[i] = CreateThread(NULL, 0, &BlurImage, (LPVOID) & (args[i]), CREATE_SUSPENDED, NULL);
        ResumeThread(threads[i]);
    }

    WaitForMultipleObjects(threadsNumber, threads, true, INFINITE);

    cv::imwrite(outFileName, blurredImage);

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> diff = end - start;
    cout << "Time taken: " << diff.count() << " s" << std::endl;

    cv::imshow("Input Image", image);
    cv::imshow("Blurred Image", blurredImage);
    cv::waitKey(0);
    cv::destroyAllWindows();

    return 0;
}