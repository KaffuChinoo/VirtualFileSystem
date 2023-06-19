#include <iostream>
#include <cstring>
#include <sstream>
#include <Windows.h>

using namespace std;

// �����ڴ�ṹ��
struct SharedData
{
    bool ready;                 // �ͻ����Ƿ�׼����
    char message[256];          // �ͻ��˷��͵���Ϣ
    char processedMessage[512]; // ��������Ϣ
};

int main()
{
    // �򿪹����ڴ����
    HANDLE hMapFile = OpenFileMapping(
        FILE_MAP_ALL_ACCESS, 
        FALSE,
        L"MySharedMemory");

    if (hMapFile == NULL) 
    {
        cerr << "�޷��򿪹����ڴ����" << endl;
        return 1;
    }

    // �������ڴ����ӳ�䵽���̵ĵ�ַ�ռ�
    SharedData* sharedData = (SharedData*)MapViewOfFile(
        hMapFile,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        sizeof(SharedData)); // �����ڴ��С

    if (sharedData == NULL) 
    {
        cerr << "�޷�ӳ�乲���ڴ浽���̵�ַ�ռ�" << endl;
        CloseHandle(hMapFile);
        return 1;
    }

    cout << "�ͻ���->>\n\n����������->>\n\n";

    while (true)
    {
        // ��ȡ�û��������Ϣ
        cout << "Client:>> ";
        string message;
        getline(cin, message);

        // ����Ϣд�빲���ڴ�
        strcpy_s(sharedData->message, sizeof(sharedData->message), message.c_str());

        // ֪ͨ������׼����
        sharedData->ready = true;

        // �ȴ��������������
        while (sharedData->ready) 
        {
            Sleep(100); 
        }

        // ��ȡ��������������ص���Ϣ
        cout << "====Server====\n" << sharedData->processedMessage << "\n====Server====\n";
    }

    // �رչ����ڴ�����ӳ��
    UnmapViewOfFile(sharedData);
    CloseHandle(hMapFile);

    return 0;
}
