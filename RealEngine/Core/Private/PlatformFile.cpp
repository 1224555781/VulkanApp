
#include "Core/Public/PlatformFile.h"

#include <filesystem>
#include <fstream>
#include "cstring"

FPlatformFile::~FPlatformFile()
{
    if (PlatformFileManager)
    {
        delete PlatformFileManager;
    }
}
#include <direct.h>
#define GetCurrentDir _getcwd


std::vector<uint8> FPlatformFile::ReadFileToBinary(const std::string& path)
{
    
    std::string ProjectPath = GetProjectPath();
    std::string TargetPath = ProjectPath + path;
    size_t index = 1;
    while (index  < 100000)
    {
        TargetPath = TargetPath.replace(TargetPath.find("\\"), 1, "/");
        index = TargetPath.find("\\");
    }
    std::basic_ifstream<uint8, std::char_traits<uint8>> file(TargetPath, std::ios::ate | std::ios::binary);
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

std::string FPlatformFile::GetProjectPath() const
{
    char buff[FILENAME_MAX]; //create string buffer to hold path
    GetCurrentDir(buff, FILENAME_MAX);
    std::string projectpath = std::string{ buff };
    int32 index = projectpath.find("out");
    projectpath = projectpath.replace(index, projectpath.size(), "");
    return projectpath;
}


