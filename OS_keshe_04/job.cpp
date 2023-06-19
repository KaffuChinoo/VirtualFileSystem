#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <Windows.h>
#include <thread>
#include <chrono>

using namespace std;

string res_jobInfo;

class Job
{
public:
    int jobId;
    int pid;
    string status;
    int rate;
    int runtime;
    int timestamp;

    Job(int id)
    {
        this->jobId = id;
        this->rate = 0;
        this->pid = 525 + id;
        this->runtime = 0;
        this->timestamp = 0;
    }

    void runJob()
    {
        for (int i = 0; i <= 10; )
        {
            if (status != "运行中")
            {
                this_thread::sleep_for(chrono::seconds(1));
                continue;
            }
            if (i > runtime)
            {
                runtime = i;
                rate = runtime * 10;
            }
            this_thread::sleep_for(chrono::seconds(1));
            i++;
        }
        runtime = 10;
        rate = 100;
        status = "已完成";
    }

    void startJob()
    {
        thread t(&Job::runJob, this);
        t.detach();
    }
};

class JobHandler
{
public:
    JobHandler()
    {
        curTimestamp = 0;
    }

    string addNewJob(Job* newJob)
    {
        for (Job* job : jobList)
        {
            if (job->jobId == newJob->jobId)
            {
                cout << "作业已存在" << endl;
                return "作业已存在\n";
            }
        }
        jobList.push_back(newJob);
        cout << "创建了作业，id为" << newJob->jobId << endl;
        return "创建了作业，id为" + to_string(newJob->jobId) + "\n";
    }

    void updateJobList(int jobId = -1)
    {
        // 更新时间戳
        curTimestamp++;

        if (jobList.empty())
        {
            return;
        }

        // 开始运行作业
        if (jobId != -1)
        {
            // 按照用户输入运行作业
            for (Job* job : jobList)
            {
                if (job->jobId == jobId)
                {
                    job->status = "运行中";
                    job->timestamp = curTimestamp;
                    
                    job->startJob();
                    // 判断作业是否运行完成
                    if (job->rate == 100)
                    {
                        job->status = "已完成";
                        job->timestamp = -1;
                    }
                    break;
                }
            }
        }
        else
        {
            // 默认处理第一个作业
            Job* job = jobList[0];
            job->status = "运行中";
            job->timestamp = curTimestamp;

            job->startJob();
            // 判断作业是否运行完成
            if (job->rate == 100)
            {
                job->status = "已完成";
                job->timestamp = -1;
            }
        }

        // 把最近使用过的（时间戳最大的）作业排在前面
        sort(jobList.begin(), jobList.end(), [](Job* job1, Job* job2) {
            return job1->timestamp > job2->timestamp;
            });

        // 将其他的作业状态设置为等待中
        if (!jobList.empty())
        {
            bool first = true;
            for (Job* job : jobList)
            {
                if (first)
                {
                    first = false;
                    continue;
                }
                if (job->status == "运行中" || job->status == "")
                {
                    job->status = "等待中";
                }
            }
        }
        return;
    }

    void cancelJob(int jobId)
    {
        if (!jobList.empty())
        {
            for (Job* job : jobList)
            {
                if (job->jobId == jobId)
                {
                    job->status = "已取消";
                    job->timestamp = -1;
                    cout << "取消了作业，id为" << jobId << endl;
                    res_jobInfo = "取消了作业，id为" + to_string(jobId) + "\n";
                }
            }
        }
        else
        {
            res_jobInfo = "作业不存在\n";
        }

        // 把最近使用过的（时间戳最大的）作业排在前面
        sort(jobList.begin(), jobList.end(), [](Job* job1, Job* job2) {
            return job1->timestamp > job2->timestamp;
            });

        return;
    }

    void pauseJob(int jobId)
    {
        if (!jobList.empty())
        {
            for (Job* job : jobList)
            {
                if (job->jobId == jobId)
                {
                    job->status = "等待中";
                    job->timestamp = 0;
                    cout << "暂停了作业，id为" << jobId << endl;
                    res_jobInfo = "暂停了作业，id为" + to_string(jobId) + "\n";
                }
            }
        }
        else
        {
            res_jobInfo = "作业不存在\n";
        }

        // 把最近使用过的（时间戳最大的）作业排在前面
        sort(jobList.begin(), jobList.end(), [](Job* job1, Job* job2) {
            return job1->timestamp > job2->timestamp;
            });

        return;
    }

    void cleanJob()
    {
        int num = 0;
        for (auto it = jobList.begin(); it != jobList.end();)
        {
            if ((*it)->timestamp == -1)
            {
                num++;
                it = jobList.erase(it);
            }
            else
            {
                ++it;
            }
        }
        res_jobInfo = "清除了" + to_string(num) +  "个作业\n";
        return;
    }

    string getJobinfo()
    {
        cout << "作业号\t进程号\t状态\t进度%\n";
        res_jobInfo = "作业号\t进程号\t状态\t进度%\n";
        for (Job* job : jobList)
        {
            cout << job->jobId << "\t" << job->pid << "\t" << job->status << "\t" << job->rate << "\n";
            res_jobInfo += to_string(job->jobId) + "\t" + to_string(job->pid) + "\t"
                + job->status + "\t" + to_string(job->rate) + "\n";
        }
        return res_jobInfo;
    }

private:
    int curTimestamp;
    vector<Job*> jobList; //作业顺序列表
};

int addJob(JobHandler& jobs, int id)
{
    Job* newJob = new Job(id);
    res_jobInfo = jobs.addNewJob(newJob);
    return id;
}

JobHandler jobs;
string initJobServer(string command)
{
    // 作业服务器API
    vector<string> comSteps;
    // 分割输入的命令
    istringstream iss(command);
    string path;
    while (getline(iss, path, ' '))
    {
        comSteps.push_back(path);
    }

    // 判断输入的命令
    res_jobInfo = "";
    int recentlyUsed = -1;
    if (comSteps[1] == "create")
    {
        int id = stoi(comSteps[2]);
        addJob(jobs, id);
        recentlyUsed = id;
    }
    else if (comSteps[1] == "info")
    {
        res_jobInfo = jobs.getJobinfo();
    }
    else if (comSteps[1] == "cancel")
    {
        int id = stoi(comSteps[2]);
        jobs.cancelJob(id);
    }
    else if (comSteps[1] == "pause")
    {
        int id = stoi(comSteps[2]);
        jobs.pauseJob(id);
    }
    else if (comSteps[1] == "start")
    {
        int id = stoi(comSteps[2]);
        recentlyUsed = id;
    }
    else if (comSteps[1] == "clean")
    {
        jobs.cleanJob();
    }

    jobs.updateJobList(recentlyUsed);
    Sleep(300);
    return res_jobInfo;
}