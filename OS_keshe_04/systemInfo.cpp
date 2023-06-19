#include <iostream>
#include <sstream>
#include <stdio.h>
#include <Windows.h>
#include <Psapi.h>
#include <tchar.h>
#include <Pdh.h>
#include <TlHelp32.h>

using namespace std;
const int INTERVAL = 5000; // 间隔时间为5秒
string res_info; // 传递给客户端

class SysInfo
{
public:
    // 获取系统状态，并输出到控制台，同时准备传递到客户端
    void printSystemStatus()
    {
        float cpuUsage = getCpuUsage();
        int memoryUsage = getMemoryUsage();

        __int64 totalDiskSpace, availableDiskSpace;
        getDiskUsage(totalDiskSpace, availableDiskSpace);
        double diskUsage = (double)(totalDiskSpace - availableDiskSpace) / totalDiskSpace * 100;

        int processStatus = getProcessStatus();

        cout << "CPU使用率：" << to_string(cpuUsage) << "%" << endl;
        cout << "内存已使用：" << to_string(memoryUsage) << " Bytes" << endl;
        cout << "磁盘使用率：" << to_string(diskUsage) << "%" << endl;
        cout << "进程总数：" << to_string(processStatus) << endl << endl;

        res_info += "CPU使用率：" + to_string(cpuUsage) + "%\n"
            + "内存使用率：" + to_string(memoryUsage) + " Bytes\n"
            + "磁盘使用率：" + to_string(diskUsage) + "%\n"
            + "进程总数：" + to_string(processStatus) + "\n\n";
    }

private:
    // CPU占用率
    float getCpuUsage()
    {
        PDH_HQUERY query;
        PDH_HCOUNTER counter;
        PdhOpenQuery(NULL, NULL, &query);
        PdhAddCounter(query, L"\\Processor(_Total)\\% Processor Time", NULL, &counter);
        PdhCollectQueryData(query);
        Sleep(1000);  // 等待一段时间以获取准确的数据
        PDH_FMT_COUNTERVALUE counterValue;
        PdhCollectQueryData(query);
        PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, NULL, &counterValue);
        PdhCloseQuery(query);
        return (float)(counterValue.doubleValue);
    }

    // 获取内存使用情况
    int getMemoryUsage()
    {
        PROCESS_MEMORY_COUNTERS_EX pmc;
        GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
        return pmc.WorkingSetSize;
    }

    // 获取磁盘使用情况
    void getDiskUsage(__int64& total, __int64& available)
    {
        ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;
        GetDiskFreeSpaceEx(NULL, &freeBytesAvailable, &totalBytes, &totalFreeBytes);
        total = (__int64)(totalBytes.QuadPart);
        available = (__int64)(freeBytesAvailable.QuadPart);
    }

    // 获取进程状态
    int getProcessStatus()
    {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        PROCESSENTRY32 processEntry;
        processEntry.dwSize = sizeof(PROCESSENTRY32);
        int count = 0;

        if (Process32First(snapshot, &processEntry))
        {
            do
            {
                count++;
            } while (Process32Next(snapshot, &processEntry));
        }

        CloseHandle(snapshot);
        return count;
    }

};

string getSysInfo()
{
    // 系统监控API
    SysInfo info;
    res_info = "";

    info.printSystemStatus();
    Sleep(INTERVAL);

    return res_info;
}

