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
            if (status != "������")
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
        status = "�����";
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
                cout << "��ҵ�Ѵ���" << endl;
                return "��ҵ�Ѵ���\n";
            }
        }
        jobList.push_back(newJob);
        cout << "��������ҵ��idΪ" << newJob->jobId << endl;
        return "��������ҵ��idΪ" + to_string(newJob->jobId) + "\n";
    }

    void updateJobList(int jobId = -1)
    {
        // ����ʱ���
        curTimestamp++;

        if (jobList.empty())
        {
            return;
        }

        // ��ʼ������ҵ
        if (jobId != -1)
        {
            // �����û�����������ҵ
            for (Job* job : jobList)
            {
                if (job->jobId == jobId)
                {
                    job->status = "������";
                    job->timestamp = curTimestamp;
                    
                    job->startJob();
                    // �ж���ҵ�Ƿ��������
                    if (job->rate == 100)
                    {
                        job->status = "�����";
                        job->timestamp = -1;
                    }
                    break;
                }
            }
        }
        else
        {
            // Ĭ�ϴ����һ����ҵ
            Job* job = jobList[0];
            job->status = "������";
            job->timestamp = curTimestamp;

            job->startJob();
            // �ж���ҵ�Ƿ��������
            if (job->rate == 100)
            {
                job->status = "�����";
                job->timestamp = -1;
            }
        }

        // �����ʹ�ù��ģ�ʱ������ģ���ҵ����ǰ��
        sort(jobList.begin(), jobList.end(), [](Job* job1, Job* job2) {
            return job1->timestamp > job2->timestamp;
            });

        // ����������ҵ״̬����Ϊ�ȴ���
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
                if (job->status == "������" || job->status == "")
                {
                    job->status = "�ȴ���";
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
                    job->status = "��ȡ��";
                    job->timestamp = -1;
                    cout << "ȡ������ҵ��idΪ" << jobId << endl;
                    res_jobInfo = "ȡ������ҵ��idΪ" + to_string(jobId) + "\n";
                }
            }
        }
        else
        {
            res_jobInfo = "��ҵ������\n";
        }

        // �����ʹ�ù��ģ�ʱ������ģ���ҵ����ǰ��
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
                    job->status = "�ȴ���";
                    job->timestamp = 0;
                    cout << "��ͣ����ҵ��idΪ" << jobId << endl;
                    res_jobInfo = "��ͣ����ҵ��idΪ" + to_string(jobId) + "\n";
                }
            }
        }
        else
        {
            res_jobInfo = "��ҵ������\n";
        }

        // �����ʹ�ù��ģ�ʱ������ģ���ҵ����ǰ��
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
        res_jobInfo = "�����" + to_string(num) +  "����ҵ\n";
        return;
    }

    string getJobinfo()
    {
        cout << "��ҵ��\t���̺�\t״̬\t����%\n";
        res_jobInfo = "��ҵ��\t���̺�\t״̬\t����%\n";
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
    vector<Job*> jobList; //��ҵ˳���б�
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
    // ��ҵ������API
    vector<string> comSteps;
    // �ָ����������
    istringstream iss(command);
    string path;
    while (getline(iss, path, ' '))
    {
        comSteps.push_back(path);
    }

    // �ж����������
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