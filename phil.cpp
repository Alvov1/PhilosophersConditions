#include <iostream>
#include <Windows.h>
#include <string>
#include <ctime>

const size_t phils = 5;
HANDLE* Phils = nullptr;
HANDLE* semaphores = nullptr;

HANDLE stdOutSema;

time_t start = 0;
time_t limit = 0;
time_t timeToEat = 0;

/* Потоки начинают есть парами. Возможны случаи:
 * 1 и 4,
 * 2 и 5,
 * 3 и 1,
 * 4 и 2,
 * 5 и 3.
 * Каждый философ захватывает семафор следующего философа,
 * и освобождает его после того, как сам поест.
 * Первыми начинаю есть философы 1 и 4. Их семафоры изначально не заняты:
 * 1 -> 2
 * 2 -> 3
 * 3 -> 4       Изначально свободный
 * 4 -> 5
 * 5 -> 1       Изначально свободный
 * */

void message(const unsigned threadNumber, bool eats) {
    static const char* eating = /*" - eats."*/ ":T->E";
    static const char* thinking = /*" - thinks.";*/ ":E->T";

    WaitForSingleObject(stdOutSema, INFINITE);
        std::cout << clock() << ":" << threadNumber + 1 << (eats ? eating : thinking) << std::endl;
    ReleaseSemaphore(stdOutSema, 1, nullptr);
}

void ended(const unsigned threadNumber) {
    WaitForSingleObject(stdOutSema, INFINITE);
        std::cout << "    - Thread " << threadNumber + 1 << " has finished running." << std::endl;
    ReleaseSemaphore(stdOutSema, 1, nullptr);
}

DWORD WINAPI threadEntry(void* param) {
    auto threadNumber = *((unsigned*) param);
    auto nextNumber = (threadNumber + 1) % phils;
    HANDLE* mySema = &semaphores[threadNumber];
    HANDLE* nextSema = &semaphores[nextNumber];

    WaitForSingleObject(*mySema, INFINITE);
    while (clock() < limit + start) {

        message(threadNumber, true);
        Sleep(timeToEat);
        message(threadNumber, false);

        /* Пускаем следующий поток. */
        ReleaseSemaphore(*nextSema, 1, nullptr);
        WaitForSingleObject(*mySema, INFINITE);
    }
    ReleaseSemaphore(*nextSema, 1, nullptr);

    ended(threadNumber);
    return 0;
}

void createSemas() {
    semaphores = new HANDLE[phils];
    for(auto i = 0; i < phils; ++i)
        semaphores[i] = CreateSemaphoreA(nullptr, (i == 0 || i == 3) ? 1 : 0, 1, nullptr);
    stdOutSema = CreateSemaphoreA(nullptr, 1, 1, nullptr);
}
void createThreads() {
    Phils = new HANDLE[phils];
    for(auto i = 0; i < phils; ++i) {
        auto* threadNumber = new unsigned;
        *threadNumber = i;
        Phils[i] = CreateThread(nullptr, 0, threadEntry, (void *) threadNumber, 0, nullptr);
    }
}

int main(const int argc, const char** argv) {
    if(argc != 3) {
        std::cout << "Error with arguments. " << std::endl;
        return -1;
    }

    limit = std::stoi(argv[1]);
    timeToEat = std::stoi(argv[2]);

    std::cout << "Time limit - " << limit << std::endl;
    std::cout << "Time for eating - " << timeToEat << std::endl;

    createSemas();
    start = clock();
    createThreads();

    WaitForMultipleObjects(phils, Phils, true, INFINITE);
    for(auto i = 0; i < phils; ++i)
        CloseHandle(Phils[i]);
    return 0;
}
