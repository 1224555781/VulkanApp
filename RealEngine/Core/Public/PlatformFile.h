#pragma once
#include <string>
#include <vector>

#include "VulkanTypeDefine.h"

class FPlatformFile
{
public:
    static FPlatformFile* Get()
    {
        if (PlatformFileManager == nullptr)
        {
            PlatformFileManager = new FPlatformFile;
        }
        return PlatformFileManager;
    }

    virtual  ~FPlatformFile();

    std::vector<uint8> ReadFileToBinary(const std::string& path);

    std::string GetProjectPath()const;
private:
    inline static FPlatformFile* PlatformFileManager = nullptr;
};

