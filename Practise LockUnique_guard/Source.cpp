#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>

using namespace std;

class FileSync
{
    mutex fileMutex;
    condition_variable fileReadyCondition;

    bool ready;
    string filename;

public:
    FileSync(const string& filename)
    {
        this->filename = filename;
        ready = false;
    }

    void waitingThread()
    {
        unique_lock<mutex> lock(fileMutex);

        cout << "WAIT" << endl;

        while (!ready)
        {
            fileReadyCondition.wait(lock);
        }

        cout << "READ" << endl;

        ifstream in(filename);
        if (!in.is_open())
        {
            cout << "NOFILE" << endl;
            return;
        }

        string line;
        while (getline(in, line))
        {
            cout << line << endl;
        }

        in.close();
    }

    void signalingThread()
    {
        this_thread::sleep_for(chrono::seconds(1));

        {
            lock_guard<mutex> guard(fileMutex);

            ofstream out(filename);
            if (!out.is_open())
            {
                ready = true;
                fileReadyCondition.notify_one();
                return;
            }

            cout << "WRITE" << endl;

            for (int i = 1; i <= 5; i++)
            {
                out << "Line " << i << "\n";
            }

            out.close();
            ready = true;
        }

        fileReadyCondition.notify_one();
    }
};

int main()
{
    setlocale(0, "ru");

    FileSync fileSync("input.txt");

    thread writer([&fileSync]()
        {
            fileSync.signalingThread();
        });

    thread reader([&fileSync]()
        {
            fileSync.waitingThread();
        });

    writer.join();
    reader.join();

    cout << "DONE" << endl;
    return 0;
}
