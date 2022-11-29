#pragma once
#include <winsock2.h>

struct FServerInfo
{
    operator bool() const
    {
        return true;
    }
};
class MServer
{
public:
    MServer();

    //inline int CreateSocket()const
    //{
    //     //WSAStartup(MAKEWORD(2,2))
    //    SOCKET socketID = socket(AF_INET, SOCK_DGRAM, 0);
    //    sockaddr_in ServerAddr;
    //    ServerAddr.sin_family = AF_INET;
    //    ServerAddr.sin_port = htons(7777);
    //    ServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    //    bind(socketID, (sockaddr*)(&ServerAddr), sizeof(ServerAddr));
    //    listen(socketID, SOMAXCONN);
    //    return  socketID;
    //}


};

