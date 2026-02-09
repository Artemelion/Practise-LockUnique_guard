#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>

using namespace std;

mutex mtx;
condition_variable cv;

bool ready = false;
string fileText;

void waitingthread()
{
    string localCopy;

    {
        unique_lock<mutex> lock(mtx);

        cout << "ожидающий поток: жду чтение файла..." << endl;

        while (!ready)
        {
            cv.wait(lock);
        }
        localCopy = fileText;
    }

    cout << "ожидающий поток: файл прочитан, печатаю:" << endl;
    cout << localCopy << endl;
}

void signalingthread()
{
    this_thread::sleep_for(chrono::seconds(1));

    string local;
    ifstream in("input.txt");

    if (!in.is_open())
    {
        {
            lock_guard<mutex> Lockguard(mtx);
            fileText = "no file\n";
            ready = true;
        }
        cv.notify_one();
        return;
    }

    string line;
    while (getline(in, line))
    {
        local += line;
        local += '\n';
    }
    in.close();

    {
        lock_guard<mutex> Lockguard(mtx);
        fileText = local;
        ready = true;
        cout << "сигналящий поток: ready = true (ФАЙЛ ПРОЧИТАН)" << endl;
    }

    cv.notify_one();
}

int main()
{
    setlocale(0, "ru");

    thread t1(waitingthread);
    thread t2(signalingthread);

    t1.join();
    t2.join();

    cout << "готово." << endl;
    return 0;
}
