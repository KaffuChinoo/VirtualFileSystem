#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <Windows.h>
#include <regex>
#include <fstream>

#include "systemInfo.h"
#include "job.h"

using namespace std;

// ����ָ����ļ���
vector<string> filePathSteps;

string res;

// ����
void filePathSplit(string);

// �����ڴ�ṹ��
struct SharedData 
{
    bool ready;                 // �ͻ����Ƿ�׼����
    char message[256];          // �ͻ��˷��͵���Ϣ
    char processedMessage[512]; // ��������Ϣ
};

struct File
{
    string name;
    vector<int> pageNumList;

    File(string name)
    {
        this->name = name;
    }
    File() {};
};

struct Directory
{
    string name;

    vector<File*> subFile;
    vector<Directory*> subDirectories;

    Directory(string name)
    {
        this->name = name;
    }
    Directory() {};
};

struct Page
{
    int pageId;
    char content[1024] = { '\0' };

    Page(int id)
    {
        this->pageId = id;
    }
    Page()
    {
        this->pageId = -1;
    }
};

class VirtualFileSys
{
public:
    VirtualFileSys(string diskname)
    {
        rootDir = new Directory(diskname);
        curDir = rootDir;
        depth = 0;
        pageUsed = 0;
    }

    void format()
    {
        // ��ʽ��
        curDir = rootDir;
        curDir->subDirectories.clear();
        curDir->subFile.clear();

        pageUsed = 0;
        cout << "��ʽ�����" << endl;
        res = "��ʽ�����\n";
        return;
    }

    void listDirFile()
    {
        // �о�Ŀ¼�����ļ�
        res = "\n";
        if (!curDir->subDirectories.empty())
        {
            for (Directory* dir : curDir->subDirectories)
            {
                cout << "\t" << "<dir>" << "\t" << dir->name << endl;
                res += "\t<dir>\t" + dir->name + "\n";
            }
        }
        if (!curDir->subFile.empty())
        {
            for (File* file : curDir->subFile)
            {
                cout << "\t\t" << file->name << endl;
                res += "\t\t" + file->name + "\n";
            }
        }
    }

    void uploadFile(string oldFilepath ,string newFilename)
    {
        // �ϴ������ļ�
        string buf = "";
        ifstream ifs;
        ifs.open(oldFilepath, ios::in);
        if (!ifs.is_open())
        {
            cout << "�ļ�������" << endl;
            res = "�ļ�������\n";
            return;
        }

        if (!makeFile(newFilename))
        {
            return;
        }

        string temp;
        while (getline(ifs, temp))
        {
            buf += temp + "\n";
        }

        writeFile(newFilename, buf);
        ifs.close();
        res = "�ϴ����:" + oldFilepath + "\n";
    }

    void downloadFile(string filename, string targetFilename)
    {
        // �����ļ�������
        string buf;
        ofstream ofs;
        ofs.open(targetFilename, ios::out);
        
        for (File* file : curDir->subFile)
        {
            if (file->name == filename)
            {
                for (int pageNum : file->pageNumList)
                {
                    buf += pages[pageNum]->content;
                }
            }
        }
        
        ofs << buf;
        ofs.close();
        res = "�������:" + targetFilename + "\n";
    }

    void makeDir(string filename)
    {
        // ����Ŀ¼
        for (Directory* dir : curDir->subDirectories)
        {
            if (dir->name == filename)
            {
                cout << "����ʧ�ܣ����������ļ���" << endl;
                res = "����ʧ�ܣ����������ļ���\n";
                return;
            }
        }
        Directory* newDir = new Directory(filename);
        curDir->subDirectories.push_back(newDir);
    }

    bool makeFile(string filename)
    {
        // �����ļ�
        for (File* file : curDir->subFile)
        {
            if (file->name == filename)
            {
                cout << "����ʧ�ܣ����������ļ�" << endl;
                res = "����ʧ�ܣ����������ļ�\n";
                return false;
            }
        }
        File* file = new File(filename);
        curDir->subFile.push_back(file);
        return true;
    }

    void writeFile(string filename, string content)
    {
        for (File* file : curDir->subFile)
        {
            if (file->name == filename)
            {
                //file->content = content;

                // ������е��ļ�δ������ҳ����
                if (file->pageNumList.empty())
                {
                    int pageCount = (int)content.length() / 1024 + 1;
                    for (int i = 0; i < pageCount; i++)
                    {
                        Page* page = new Page(pageUsed);
                        pageUsed++;
                        for (int j = 0; j < content.length() - 1; j++)
                        {
                            if (j == 1023)
                            {
                                break;
                            }
                            page->content[j] = content.at(j);
                        }
                        // ���ҳ��ŵ��ļ���
                        file->pageNumList.push_back(page->pageId);

                        // ���ҳ�浽�ļ�ϵͳ
                        pages.push_back(page);
                    }
                }
                else
                {
                    for (int pageNum : file->pageNumList)
                    {
                        for (int j = 0; j < content.length() - 1; j++)
                        {
                            if (j == 1023)
                            {
                                break;
                            }
                            pages[pageNum]->content[j] = content.at(j);
                        }
                    }
                }
            }
        }
    }

    string readSubFile(string filename)
    {
        // ���ļ���ȡ
        
        string buf = "";
        for (File* file : curDir->subFile)
        {
            if (file->name == filename)
            {
                //buf = file->content;
                for (int pageNum : file->pageNumList)
                {
                    if (cache[0].content[0] == '\0')
                    {
                        for (int i = 0; i < 1024; i++)
                        {
                            if (pages[pageNum]->content[i] == '\0')
                            {
                                break;
                            }
                            cache[0].content[i] = pages[pageNum]->content[i];
                        }
                        buf += cache[0].content;
                        continue;
                    }
                    if (cache[0].pageId == pageNum)
                    {
                        buf += cache[0].content;
                    }
                    else
                    {
                        for (int i = 0; i < 1024; i++)
                        {
                            cache[0].content[i] = '\0';
                        }
                        for (int i = 0; i < 1024; i++)
                        {
                            if (pages[pageNum]->content[i] == '\0')
                            {
                                break;
                            }
                            cache[0].content[i] = pages[pageNum]->content[i];
                        }
                        buf += cache[0].content;
                    }
                }

                cout << endl << filename << ":" << endl << buf << endl;
                res += "\n" + filename + ":" + "\n" + buf + "\n";
                return buf;
            }
        }
        cout << "��ȡʧ�ܣ��ļ�δ�ҵ�" << endl;
        res = "��ȡʧ�ܣ��ļ�δ�ҵ�\n";
        return "";
    }

    void removeSubFile(string filename)
    {
        // ���ļ�ɾ��
        // �����±�
        int i = 0;
        bool fileExists = false;
        for (File* file : curDir->subFile)
        {
            if (filename == file->name)
            {
                for (int pageNum : file->pageNumList)
                {
                    // ����ҳ��
                    delete[] pages[pageNum];
                    pageUsed--;
                }
                fileExists = true;
                break;
            }
            i++;
        }

        if (fileExists)
        {
            curDir->subFile.erase(curDir->subFile.begin() + i);
            cout << "ɾ���ˣ�" << filename << endl;
            res = "ɾ���ˣ�" + filename + "\n";
        }
        else
        {
            cout << "ɾ��ʧ�ܣ��ļ�δ�ҵ�" << endl;
            res = "ɾ��ʧ�ܣ��ļ�δ�ҵ�\n";
        }
    }

    void removeSubDir(string dirname)
    {
        // ��Ŀ¼ɾ��
        // �����±�
        int i = 0;
        bool dirExists = false;
        for (Directory* dir : curDir->subDirectories)
        {
            if (dirname == dir->name)
            {
                dirExists = true;
                break;
            }
            i++;
        }

        if (dirExists)
        {
            curDir->subDirectories.erase(curDir->subDirectories.begin() + i);
            cout << "ɾ���ˣ�" << dirname << endl;
            res = "ɾ���ˣ�" + dirname + "\n";
        }
        else 
        {
            cout << "ɾ��ʧ�ܣ��ļ���δ�ҵ�" << endl;
            res = "ɾ��ʧ�ܣ��ļ���δ�ҵ�\n";
        }
    }

    bool getIntoSubDir(string filename)
    {
        // �������ļ���
        for (Directory* dir : curDir->subDirectories)
        {
            if (dir->name == filename)
            {
                curDir = dir;
                filepath += (dir->name + "/");
                depth++;
                return true;
            }
        }
        cout << "·��������" << endl;
        res = "·��������\n";
        return false;
    }

    bool returnOuterDir()
    {
        // �����ϼ�Ŀ¼
        if (depth == 0)
        {
            return false;
        }

        // �Ӹ�Ŀ¼����׷��·��
        curDir = rootDir;

        // ���յ�ǰ·�������ȷ��׷���յ�
        filePathSplit(filepath);
        filepath = "";
        // ���ڽ������ļ���֮��ᵼ��depth��һ�������ȱ���depth
        int oldDepth = depth;
        for (int i = 0; i < oldDepth - 1; i++)
        {
            getIntoSubDir(filePathSteps[i]);
        }
        depth = oldDepth - 1;
        return true;
    }

    string getCurPath()
    {
        // ��õ�ǰ�ļ�·��
        return filepath;
    }

    int getPageUsed()
    {
        // ���ʹ�õ�ҳ����
        return pageUsed;
    }

private:
    Directory* rootDir; // �ļ�ϵͳ�ĸ��ļ���
    Directory* curDir;  // ��ǰѡ�����ļ���
    string filepath;    // ��ǰ���ļ�·��
    vector<Page*> pages;// �ļ�ҳ��
    Page cache[1];      // LRUҳ�滺��
    int pageUsed;       // ��ǰʹ�ù���ҳ��
    int depth;          // ��ǰ·�������
};

void filePathSplit(string filePath)
{
    // ��������ļ�·���ָ�Ϊ����ļ���
    filePathSteps.clear();
    istringstream iss(filePath);
    string path;
    while (getline(iss, path, '/'))
    {
        filePathSteps.push_back(path);
    }
}

int executeCommand(VirtualFileSys& fileSys, string command)
{
    // ִ������
    // �ָ����������
    vector<string> comSteps;
    istringstream iss(command);
    string path;
    while (getline(iss, path, ' '))
    {
        comSteps.push_back(path);
    }

    // �ж����������
    // ֧�� cd, ls, mkdir, mkfile, format, read, rm -d, rm -f, read, download, upload
    if (comSteps.empty())
    {
        return 1;
    }
    if (comSteps[0] == "cd")
    {
        // cd ..
        if (comSteps[1] == "..")
        {
            if (!fileSys.returnOuterDir())
            {
                cout << "�ϼ�Ŀ¼������" << endl;
                res = "�ϼ�Ŀ¼������\n";
            }
            return 1;
        }

        // cd [dirname]
        filePathSplit(comSteps[1]);
        for (string path : filePathSteps)
        {
            if (!fileSys.getIntoSubDir(path))
            {
                break;
            }
        }
    }
    else if (comSteps[0] == "ls" || comSteps[0] == "dir")
    {
        fileSys.listDirFile();
    }
    else if (comSteps[0] == "rm")
    {
        if (comSteps[1] == "-f")
        {
            fileSys.removeSubFile(comSteps[2]);
        }
        else if (comSteps[1] == "-d")
        {
            fileSys.removeSubDir(comSteps[2]);
        }
    }
    else if (comSteps[0] == "mkdir")
    {
        fileSys.makeDir(comSteps[1]);
    }
    else if (comSteps[0] == "mkfile")
    {
        fileSys.makeFile(comSteps[1]);
    }
    else if (comSteps[0] == "read" || comSteps[0] == "type")
    {
        fileSys.readSubFile(comSteps[1]);
    }
    else if (comSteps[0] == "format")
    {
        fileSys.format();
    }
    else if (comSteps[0] == "upload")
    {
        fileSys.uploadFile(comSteps[1], comSteps[2]);
    }
    else if (comSteps[0] == "download")
    {
        fileSys.downloadFile(comSteps[1], comSteps[2]);
    }
    else if (comSteps[0] == "write")
    {
        string buf = "";
        if (comSteps.size() >= 3)
        {
            for (int i = 0; i < comSteps.size() - 2; i++)
            {
                buf += comSteps[i + 2] + " ";
            }
        }
        fileSys.writeFile(comSteps[1], buf);
    }
    else if (comSteps[0] == "exit")
    {
        return 0;
    }
    else
    {
        cout << "����֧�ֵ�����:" << comSteps[0] << endl;
        res = "����֧�ֵ�����:" + comSteps[0] + "\n";
    }
    return 1;
}

string executeFileSys(VirtualFileSys& fileSys, string userCommand)
{
    // �����ļ�ϵͳAPI
    string diskRoot = "F://";

    executeCommand(fileSys, userCommand);
    string path = fileSys.getCurPath();
    cout << diskRoot << path << "> ";
    path = diskRoot + path + "> ";

    return path;
}

// ����˵Ĵ�����������������
int main() 
{
    // ���������ڴ�
    HANDLE hMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE, 
        NULL, 
        PAGE_READWRITE, 
        0, 
        sizeof(SharedData), 
        L"MySharedMemory"); 

    if (hMapFile == NULL) 
    {
        cerr << "�޷����������ڴ����" << endl;
        return 1;
    }

    // �������ڴ����ӳ�䵽���̵ĵ�ַ�ռ�
    SharedData* sharedData = (SharedData*)MapViewOfFile(
        hMapFile,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        sizeof(SharedData));

    if (sharedData == NULL) 
    {
        cerr << "�޷�ӳ�乲���ڴ浽���̵�ַ�ռ�" << endl;
        CloseHandle(hMapFile);
        return 1;
    }

    // �����ļ�ϵͳ
    string diskname = "F";
    VirtualFileSys fileSys(diskname);


    cout << "�����->>\n\n";
    while (true)
    {
        // �ȴ��ͻ���׼����
        while (!sharedData->ready) 
        {
            Sleep(100);
        }

        // �����ͻ��˷��͵�����
        cout << "���յ��ͻ������" << sharedData->message << endl;
        string userCommand = sharedData->message;
        string header;
        regex pattern("^job");

        if (userCommand == "server info")
        {
            // ����ϵͳ���API
            res = "";
            for (int i = 0; i < 2; i++)
            {
                res += to_string((2 - i) * 5) + "��ǰ��\n";
                res += "ʹ�õ�1Kҳ������" + to_string(fileSys.getPageUsed()) + "\n";
                res += getSysInfo();
            }
        }
        else if (regex_search(userCommand, pattern))
        {
            // ������ҵAPI
            res = "";
            res = initJobServer(userCommand);
            Sleep(200);
        }
        else
        {
            // �����ļ�ϵͳAPI
            res = "";
            header = executeFileSys(fileSys, userCommand);
        }

        // ����������Ϣд�빲���ڴ�
        string resMessage = header + res;
        strcpy_s(sharedData->processedMessage, sizeof(sharedData->processedMessage), resMessage.c_str());

        // ֪ͨ�ͻ��˴������
        sharedData->ready = false;
    }

    // �رչ����ڴ�����ӳ��
    UnmapViewOfFile(sharedData);
    CloseHandle(hMapFile);

    return 0;
}