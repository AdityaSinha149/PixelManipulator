#include "greenScreen.h"
#include "image.h"

#include <algorithm>
#include <dirent.h>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <vector>
#include <chrono>


namespace {
std::vector<std::string> listFiles(const std::string& dir)
{
    DIR* handle = opendir(dir.c_str());
    if (!handle)
    {
        throw std::runtime_error("Unable to open directory: " + dir);
    }

    std::vector<std::string> files;
    while (dirent* entry = readdir(handle))
    {
        // Skip current/parent and hidden entries.
        if (entry->d_name[0] == '.')
        {
            continue;
        }
        files.emplace_back(dir + "/" + entry->d_name);
    }
    closedir(handle);

    std::sort(files.begin(), files.end());
    return files;
}
}

int main()
{
    const auto screens = listFiles("pics/screens");
    const auto inputs = listFiles("pics/inputs");
    const size_t pairCount = std::min(screens.size(), inputs.size());

    if (pairCount == 0)
    {
        std::cerr << "No input pairs found in pics/screens and pics/inputs\n";
        return 1;
    }

    size_t numToProcess = 0;
    std::cout << "How many image pairs do you want to process? (max: " << pairCount << ") ";
    std::cin >> numToProcess;
    if (numToProcess > pairCount) numToProcess = pairCount;

    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < numToProcess; ++i)
    {
        Image screenImage{screens[i]};
        Image inputImage{inputs[i]};

        std::string outputFile = "pics/outputs/output_" + std::to_string(i) + ".jpg";
        greenScreenImage::applyGreenScreen(screenImage, inputImage, outputFile);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Total time elapsed: " << elapsed.count() << " ms\n";
    return 0;
}