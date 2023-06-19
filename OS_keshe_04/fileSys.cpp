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

// 保存分割后的文件名
vector<string> filePathSteps;

string res;

// 声明
void filePathSplit(string);

// 共享内存结构体
struct SharedData 
{
    bool ready;                 // 客户端是否准备好
    char message[256];          // 客户端发送的消息
    char processedMessage[512]; // 处理后的消息
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
        // 格式化
        curDir = rootDir;
        curDir->subDirectories.clear();
        curDir->subFile.clear();

        pageUsed = 0;
        cout << "格式化完成" << endl;
        res = "格式化完成\n";
        return;
    }

    void listDirFile()
    {
        // 列举目录和子文件
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
        // 上传本地文件
        string buf = "";
        ifstream ifs;
        ifs.open(oldFilepath, ios::in);
        if (!ifs.is_open())
        {
            cout << "文件不存在" << endl;
            res = "文件不存在\n";
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
        res = "上传完成:" + oldFilepath + "\n";
    }

    void downloadFile(string filename, string targetFilename)
    {
        // 下载文件到本地
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
        res = "下载完成:" + targetFilename + "\n";
    }

    void makeDir(string filename)
    {
        // 创建目录
        for (Directory* dir : curDir->subDirectories)
        {
            if (dir->name == filename)
            {
                cout << "创建失败：存在重名文件夹" << endl;
                res = "创建失败：存在重名文件夹\n";
                return;
            }
        }
        Directory* newDir = new Directory(filename);
        curDir->subDirectories.push_back(newDir);
    }

    bool makeFile(string filename)
    {
        // 创建文件
        for (File* file : curDir->subFile)
        {
            if (file->name == filename)
            {
                cout << "创建失败：存在重名文件" << endl;
                res = "创建失败：存在重名文件\n";
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

                // 如果现有的文件未分配在页表中
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
                        // 添加页面号到文件中
                        file->pageNumList.push_back(page->pageId);

                        // 添加页面到文件系统
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
        // 子文件读取
        
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
        cout << "读取失败：文件未找到" << endl;
        res = "读取失败：文件未找到\n";
        return "";
    }

    void removeSubFile(string filename)
    {
        // 子文件删除
        // 查找下标
        int i = 0;
        bool fileExists = false;
        for (File* file : curDir->subFile)
        {
            if (filename == file->name)
            {
                for (int pageNum : file->pageNumList)
                {
                    // 回收页面
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
            cout << "删除了：" << filename << endl;
            res = "删除了：" + filename + "\n";
        }
        else
        {
            cout << "删除失败：文件未找到" << endl;
            res = "删除失败：文件未找到\n";
        }
    }

    void removeSubDir(string dirname)
    {
        // 子目录删除
        // 查找下标
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
            cout << "删除了：" << dirname << endl;
            res = "删除了：" + dirname + "\n";
        }
        else 
        {
            cout << "删除失败：文件夹未找到" << endl;
            res = "删除失败：文件夹未找到\n";
        }
    }

    bool getIntoSubDir(string filename)
    {
        // 进入子文件夹
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
        cout << "路径不存在" << endl;
        res = "路径不存在\n";
        return false;
    }

    bool returnOuterDir()
    {
        // 返回上级目录
        if (depth == 0)
        {
            return false;
        }

        // 从根目录重新追溯路径
        curDir = rootDir;

        // 按照当前路径深度来确定追溯终点
        filePathSplit(filepath);
        filepath = "";
        // 由于进入子文件夹之后会导致depth加一，所以先保存depth
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
        // 获得当前文件路径
        return filepath;
    }

    int getPageUsed()
    {
        // 获得使用的页面数
        return pageUsed;
    }

private:
    Directory* rootDir; // 文件系统的根文件夹
    Directory* curDir;  // 当前选定的文件夹
    string filepath;    // 当前的文件路径
    vector<Page*> pages;// 文件页表
    Page cache[1];      // LRU页面缓存
    int pageUsed;       // 当前使用过的页数
    int depth;          // 当前路径的深度
};

void filePathSplit(string filePath)
{
    // 将传入的文件路径分割为多个文件名
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
    // 执行命令
    // 分割输入的命令
    vector<string> comSteps;
    istringstream iss(command);
    string path;
    while (getline(iss, path, ' '))
    {
        comSteps.push_back(path);
    }

    // 判断输入的命令
    // 支持 cd, ls, mkdir, mkfile, format, read, rm -d, rm -f, read, download, upload
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
                cout << "上级目录不存在" << endl;
                res = "上级目录不存在\n";
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
        cout << "不受支持的命令:" << comSteps[0] << endl;
        res = "不受支持的命令:" + comSteps[0] + "\n";
    }
    return 1;
}

string executeFileSys(VirtualFileSys& fileSys, string userCommand)
{
    // 虚拟文件系统API
    string diskRoot = "F://";

    executeCommand(fileSys, userCommand);
    string path = fileSys.getCurPath();
    cout << diskRoot << path << "> ";
    path = diskRoot + path + "> ";

    return path;
}

// 服务端的创建区域和命令处理区域
int main() 
{
    // 创建共享内存
    HANDLE hMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE, 
        NULL, 
        PAGE_READWRITE, 
        0, 
        sizeof(SharedData), 
        L"MySharedMemory"); 

    if (hMapFile == NULL) 
    {
        cerr << "无法创建共享内存对象" << endl;
        return 1;
    }

    // 将共享内存对象映射到进程的地址空间
    SharedData* sharedData = (SharedData*)MapViewOfFile(
        hMapFile,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        sizeof(SharedData));

    if (sharedData == NULL) 
    {
        cerr << "无法映射共享内存到进程地址空间" << endl;
        CloseHandle(hMapFile);
        return 1;
    }

    // 创建文件系统
    string diskname = "F";
    VirtualFileSys fileSys(diskname);


    cout << "服务端->>\n\n";
    while (true)
    {
        // 等待客户端准备好
        while (!sharedData->ready) 
        {
            Sleep(100);
        }

        // 分析客户端发送的命令
        cout << "接收到客户端命令：" << sharedData->message << endl;
        string userCommand = sharedData->message;
        string header;
        regex pattern("^job");

        if (userCommand == "server info")
        {
            // 调用系统监控API
            res = "";
            for (int i = 0; i < 2; i++)
            {
                res += to_string((2 - i) * 5) + "秒前：\n";
                res += "使用的1K页面数：" + to_string(fileSys.getPageUsed()) + "\n";
                res += getSysInfo();
            }
        }
        else if (regex_search(userCommand, pattern))
        {
            // 调用作业API
            res = "";
            res = initJobServer(userCommand);
            Sleep(200);
        }
        else
        {
            // 调用文件系统API
            res = "";
            header = executeFileSys(fileSys, userCommand);
        }

        // 将处理后的消息写入共享内存
        string resMessage = header + res;
        strcpy_s(sharedData->processedMessage, sizeof(sharedData->processedMessage), resMessage.c_str());

        // 通知客户端处理完成
        sharedData->ready = false;
    }

    // 关闭共享内存对象和映射
    UnmapViewOfFile(sharedData);
    CloseHandle(hMapFile);

    return 0;
}