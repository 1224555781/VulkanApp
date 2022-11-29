
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

std::string normalizePath(const std::string& messyPath) {
    std::filesystem::path path(messyPath);
    std::filesystem::path canonicalPath = std::filesystem::weakly_canonical(path);
    std::string npath = canonicalPath.make_preferred().string();
    return npath;
}
std::vector<uint8> FPlatformFile::ReadFileToBinary(const std::string& path)
{
    
    std::string ProjectPath = GetProjectPath();
    std::string TargetPath = ProjectPath + path;
    while (ProjectPath.find("\\"))
    {
        ProjectPath = ProjectPath.replace(ProjectPath.find("\\"), 1, "/");
    }
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

std::string FPlatformFile::GetProjectPath() const
{
    char buff[FILENAME_MAX]; //create string buffer to hold path
    GetCurrentDir(buff, FILENAME_MAX);
    return std::string{ buff };
}


