#include <iostream>
#include <cstring>
#include <sstream>
#include <Windows.h>

using namespace std;

// 共享内存结构体
struct SharedData
{
    bool ready;                 // 客户端是否准备好
    char message[256];          // 客户端发送的消息
    char processedMessage[512]; // 处理后的消息
};

int main()
{
    // 打开共享内存对象
    HANDLE hMapFile = OpenFileMapping(
        FILE_MAP_ALL_ACCESS, 
        FALSE,
        L"MySharedMemory");

    if (hMapFile == NULL) 
    {
        cerr << "无法打开共享内存对象" << endl;
        return 1;
    }

    // 将共享内存对象映射到进程的地址空间
    SharedData* sharedData = (SharedData*)MapViewOfFile(
        hMapFile,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        sizeof(SharedData)); // 共享内存大小

    if (sharedData == NULL) 
    {
        cerr << "无法映射共享内存到进程地址空间" << endl;
        CloseHandle(hMapFile);
        return 1;
    }

    cout << "客户端->>\n\n请输入命令->>\n\n";

    while (true)
    {
        // 读取用户输入的消息
        cout << "Client:>> ";
        string message;
        getline(cin, message);

        // 将消息写入共享内存
        strcpy_s(sharedData->message, sizeof(sharedData->message), message.c_str());

        // 通知服务器准备好
        sharedData->ready = true;

        // 等待服务器处理完成
        while (sharedData->ready) 
        {
            Sleep(100); 
        }

        // 读取并输出服务器返回的消息
        cout << "====Server====\n" << sharedData->processedMessage << "\n====Server====\n";
    }

    // 关闭共享内存对象和映射
    UnmapViewOfFile(sharedData);
    CloseHandle(hMapFile);

    return 0;
}
