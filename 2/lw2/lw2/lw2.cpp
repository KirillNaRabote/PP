#include <opencv2/opencv.hpp>
#include <fstream>
#include <vector>
#include <windows.h>
#include <chrono>

using namespace std;

void blurImage(cv::Mat& image, cv::Mat& result, int start_row, int end_row)
{
    for (int i = start_row; i < end_row; i++)
    {
        cv::GaussianBlur(image.row(i), result.row(i), cv::Size(5, 5), 0);
    }
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
    ofstream outputFile;
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

    vector<DWORD_PTR> affinity = {1, 3, 7, 15};
    DWORD_PTR mask = affinity[coresNumber - 1];
    SetProcessAffinityMask(GetCurrentProcess(), (DWORD_PTR)mask);
    
    auto start = chrono::high_resolution_clock::now();

    int rowsPerThread = image.rows / threadsNumber;
        
    vector<thread> threads;

    for (int i = 0; i < threadsNumber; i++)
    {
        int startRow = i * rowsPerThread;
        int endRow = (i == threadsNumber - 1) ? image.rows : (i + 1) * rowsPerThread;
      
        threads.emplace_back([&image, &blurredImage, startRow, endRow]()
            {
                blurImage(image, blurredImage, startRow, endRow);
            });
    }

    for (auto& t : threads)
    {
        t.join();
    }

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