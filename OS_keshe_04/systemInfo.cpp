#include <iostream>
#include <sstream>
#include <stdio.h>
#include <Windows.h>
#include <Psapi.h>
#include <tchar.h>
#include <Pdh.h>
#include <TlHelp32.h>

using namespace std;
const int INTERVAL = 5000; // ���ʱ��Ϊ5��
string res_info; // ���ݸ��ͻ���

class SysInfo
{
public:
    // ��ȡϵͳ״̬�������������̨��ͬʱ׼�����ݵ��ͻ���
    void printSystemStatus()
    {
        float cpuUsage = getCpuUsage();
        int memoryUsage = getMemoryUsage();

        __int64 totalDiskSpace, availableDiskSpace;
        getDiskUsage(totalDiskSpace, availableDiskSpace);
        double diskUsage = (double)(totalDiskSpace - availableDiskSpace) / totalDiskSpace * 100;

        int processStatus = getProcessStatus();

        cout << "CPUʹ���ʣ�" << to_string(cpuUsage) << "%" << endl;
        cout << "�ڴ���ʹ�ã�" << to_string(memoryUsage) << " Bytes" << endl;
        cout << "����ʹ���ʣ�" << to_string(diskUsage) << "%" << endl;
        cout << "����������" << to_string(processStatus) << endl << endl;

        res_info += "CPUʹ���ʣ�" + to_string(cpuUsage) + "%\n"
            + "�ڴ�ʹ���ʣ�" + to_string(memoryUsage) + " Bytes\n"
            + "����ʹ���ʣ�" + to_string(diskUsage) + "%\n"
            + "����������" + to_string(processStatus) + "\n\n";
    }

private:
    // CPUռ����
    float getCpuUsage()
    {
        PDH_HQUERY query;
        PDH_HCOUNTER counter;
        PdhOpenQuery(NULL, NULL, &query);
        PdhAddCounter(query, L"\\Processor(_Total)\\% Processor Time", NULL, &counter);
        PdhCollectQueryData(query);
        Sleep(1000);  // �ȴ�һ��ʱ���Ի�ȡ׼ȷ������
        PDH_FMT_COUNTERVALUE counterValue;
        PdhCollectQueryData(query);
        PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, NULL, &counterValue);
        PdhCloseQuery(query);
        return (float)(counterValue.doubleValue);
    }

    // ��ȡ�ڴ�ʹ�����
    int getMemoryUsage()
    {
        PROCESS_MEMORY_COUNTERS_EX pmc;
        GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
        return pmc.WorkingSetSize;
    }

    // ��ȡ����ʹ�����
    void getDiskUsage(__int64& total, __int64& available)
    {
        ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;
        GetDiskFreeSpaceEx(NULL, &freeBytesAvailable, &totalBytes, &totalFreeBytes);
        total = (__int64)(totalBytes.QuadPart);
        available = (__int64)(freeBytesAvailable.QuadPart);
    }

    // ��ȡ����״̬
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
    // ϵͳ���API
    SysInfo info;
    res_info = "";

    info.printSystemStatus();
    Sleep(INTERVAL);

    return res_info;
}

