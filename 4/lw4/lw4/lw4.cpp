#include <opencv2/opencv.hpp>
#include <fstream>
#include <vector>
#include <windows.h>
#include <chrono>
#include <math.h>
#include <numbers>

using namespace std;

struct BlurImageParam
{
    cv::Mat* image;
    cv::Mat* result;

    size_t startRow;
    size_t endRow;

    chrono::steady_clock::time_point startTime;

    int numOfThread;

    int threadPriority;
};

DWORD WINAPI BlurImage2(LPVOID lpParam) 
{
    size_t startRow = (*(BlurImageParam*)lpParam).startRow;
    size_t endRow = (*(BlurImageParam*)lpParam).endRow;

    cv::Mat* image = (*(BlurImageParam*)lpParam).image;
    cv::Mat* result = (*(BlurImageParam*)lpParam).result;

    chrono::steady_clock::time_point startTime = (*(BlurImageParam*)lpParam).startTime;

    int threadPriority = (*(BlurImageParam*)lpParam).threadPriority;

    int numOfThread = (*(BlurImageParam*)lpParam).numOfThread;

    string outputFileName = "ExecutionTimeForThread#" + to_string(numOfThread) + ".txt";
    ofstream outputFile(outputFileName);

    int kernelSize = 5;
    float sigma = 1.0;
    int halfSize = kernelSize / 2;
    float twoSigmaSquare = 2 * sigma * sigma;
    float weightSum = 0.0;

    // Создание ядра Гаусса
    std::vector<std::vector<float>> kernel(kernelSize, std::vector<float>(kernelSize, 0.0));
    for (int i = -halfSize; i <= halfSize; i++) {
        for (int j = -halfSize; j <= halfSize; j++) {
            float weight = exp(-(i * i + j * j) / twoSigmaSquare) / (3.1415 * twoSigmaSquare);
            kernel[i + halfSize][j + halfSize] = weight;
            weightSum += weight;
        }
    }

    // Нормализация ядра
    for (int i = 0; i < kernelSize; i++) {
        for (int j = 0; j < kernelSize; j++) {
            kernel[i][j] /= weightSum;
        }
    }

    // Применение ядра к изображению
    for (int i = startRow; i < endRow; i++) {
        for (int j = 0; j < (*image).cols; j++) {
            cv::Vec3f sum(0, 0, 0);
            for (int k = -halfSize; k <= halfSize; k++) {
                for (int l = -halfSize; l <= halfSize; l++) {
                    int x = j + l;
                    int y = i + k;
                    if (x >= 0 && x < (*image).cols && y >= 0 && y < (*image).rows) {
                        cv::Vec3b pixel = (*image).at<cv::Vec3b>(y, x);
                        float weight = kernel[k + halfSize][l + halfSize];
                        sum += cv::Vec3f(pixel[0], pixel[1], pixel[2]) * weight;
                    }
                }
            }
            cv::Vec3b blurredPixel(sum[0], sum[1], sum[2]);
            (*result).at<cv::Vec3b>(i, j) = blurredPixel;

            auto endTime = chrono::high_resolution_clock::now();
            chrono::duration<double> diff = endTime - startTime;

            /*string msg = to_string(numOfThread) + " | " + to_string(threadPriority) + " | " + to_string(diff.count()) + "\n";*/
            string msg = to_string(diff.count()) + "\n";

            outputFile << msg;
        }
    }

    ExitThread(0);
}

void PrintProgramParameters()
{
    cout << "Invalid parameters" << endl
        << "Usage: lw4.exe <InputImage> <OutputImage> <ThreadsNumber> <CoresNumber> <ThreadPriority #1> ... <ThreadPriority #ThreadsNumber>" << endl
        << "Priority must be [below_normal, normal, above_normal]" << endl;
}

int main(int argc, char* argv[])
{
    int threadsNumber;

    if (argc < 5)
    {
        PrintProgramParameters();
        return 1;
    }
    else
    {
        threadsNumber = stoi(argv[3]);
        if (5 + threadsNumber != argc)
        {
            PrintProgramParameters();
            return 1;
        }
    }

    string inImage = argv[1];
    string outImage = argv[2];
    int coresNumber = stoi(argv[4]);
    vector<int> threadsPriority;
    for (int i = 5; i < 5 + threadsNumber; i++)
    {
        string prior = argv[i];
        if (prior == "below_normal")
        {
            threadsPriority.push_back(THREAD_PRIORITY_BELOW_NORMAL);
        }
        else if (prior == "normal")
        {
            threadsPriority.push_back(THREAD_PRIORITY_NORMAL);
        }
        else if (prior == "above_normal")
        {
            threadsPriority.push_back(THREAD_PRIORITY_ABOVE_NORMAL);
        }
        else
        {
            PrintProgramParameters();
            return 1;
        }
    }


    cv::Mat image = cv::imread(inImage);
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
        param.numOfThread = i + 1;
        param.startTime = start;
        param.threadPriority = threadsPriority[i];

        args.push_back(param);
    }

    for (size_t i = 0; i < threadsNumber; i++)
    {
        threads[i] = CreateThread(NULL, 0, &BlurImage2, (LPVOID) & (args[i]), CREATE_SUSPENDED, NULL);
        SetThreadPriority(threads[i], threadsPriority[i]);
        ResumeThread(threads[i]);
    }

    WaitForMultipleObjects(threadsNumber, threads, true, INFINITE);

    cv::imwrite(outImage, blurredImage);

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> diff = end - start;
    cout << "Time taken: " << diff.count() << " s" << std::endl;

    cv::imshow("Input Image", image);
    cv::imshow("Blurred Image", blurredImage);
    cv::waitKey(0);
    cv::destroyAllWindows();

    return 0;
}