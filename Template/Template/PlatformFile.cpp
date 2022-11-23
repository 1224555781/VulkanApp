#include "PlatformFile.h"

#include <fstream>

FPlatformFile::~FPlatformFile()
{
    if (PlatformFileManager)
    {
        delete PlatformFileManager;
    }
}

std::vector<uint8> FPlatformFile::ReadFileToBinary(const std::string& path)
{
    std::basic_ifstream<uint8, std::char_traits<uint8>> file(path, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    int32 fileSize = (size_t)file.tellg();
    std::vector<uint8> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}
