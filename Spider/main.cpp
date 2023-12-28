#include <iostream>
#include <tuple>
#include <pqxx/pqxx>
#include <windows.h>
#include <exception>
#include <string.h>
#include <thread>
#include "ParcerINI.h"
#include "database.h"
#include "indexator.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <queue>


class safe_queue {

	std::queue<std::function<void()>> tasks;	//очередь для хранения задач
	std::mutex mutex;							//для реализации блокировки
	std::condition_variable cv;					//для уведомлений


public:
	int getTaskSize() {
		return tasks.size();
	}
	std::mutex& getMutex() {
		return mutex;
	}
	std::condition_variable& getCond() {
		return cv;
	}
	//Метод push записывает в начало очереди новую задачу.
	//При этом захватывает мьютекс, а после окончания операции нотифицируется условная переменная.
	void push(std::function<void()> task) {
		std::unique_lock<std::mutex> lock(mutex);
		tasks.push(task);
		cv.notify_one();
	}
	//Метод pop находится в ожидании, пока не придут уведомление на условную переменную.
	//При нотификации условной переменной данные считываются из очереди.
	std::function<void()> pop() {
		std::unique_lock<std::mutex> lock(mutex);
		cv.wait(lock, [this] { return !tasks.empty(); });
		auto task = tasks.front();
		tasks.pop();
		return task;
	}

};


class thread_pool {
	safe_queue task_queue;
	std::vector<std::thread> threads;
	bool stop;
public:
	thread_pool(size_t num_threads) : stop(false) {
		for (size_t i = 0; i < num_threads; ++i) {
			threads.emplace_back([this]() {
				while (task_queue.getTaskSize() > 0) {
					auto task = task_queue.pop();
					task();
				}
				});
		}
		/*for (size_t i = 0; i < num_threads; ++i) {
			threads.emplace_back(std::thread(&thread_pool::work));
		}*/
	}
	~thread_pool() {
		for (auto& thread : threads) {
			if (thread.joinable()) { thread.join(); }
		}
		stop = true;
	}
	//Метод submit помещает в очередь очередную задачу.
	//В качестве принимаемого аргумента метод может принимать или объект шаблона std::function, или объект шаблона package_task.
	template <typename Func, typename... Args>
	void submit(Func&& func, Args&&... args) {
		task_queue.push(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
	}

	//Метод work выбирает из очереди очередную задачи и исполняет ее.
	//Данный метод передается конструктору потоков для исполнения
	void work() {
		while (task_queue.getTaskSize() > 0) {
			auto task = task_queue.pop();
			task();
		}
	}
};



std::string DataBaseHostName;
std::string DataBaseName;
std::string DataBaseUserName;
std::string DataBasePassword;
int DataBasePort;

std::string SpiderStarPageURL;
int SpiderDepth;

std::string FinderAddress;
int FinderPort;


int main()
{
	//setlocale(LC_ALL, "Russian");
	SetConsoleCP(65001);
	SetConsoleOutputCP(65001); //UTF-8
	// Прочитаем конфигурацию в файле configuration.ini
	try {
		char buffer[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, buffer);
		std::string filePath = std::string(buffer) + "\\configuration.ini"; // определим путь к исполняемому файлу
		std::cout << filePath;
		ParcerINI parser = ParcerINI::ParcerINI(filePath);

		DataBaseHostName = parser.get_value<std::string>("DataBase.HostName");
		DataBaseName = parser.get_value<std::string>("DataBase.DatabaseName");
		DataBaseUserName = parser.get_value<std::string>("DataBase.UserName");
		DataBasePassword = parser.get_value<std::string>("DataBase.Password");
		DataBasePort = parser.get_value<int>("DataBase.Port");

		SpiderStarPageURL = parser.get_value<std::string>("Spider.StartPageURL");
		SpiderDepth = parser.get_value<int>("Spider.Depth");

		FinderAddress = parser.get_value<std::string>("Finder.Address");
		FinderPort = parser.get_value<int>("Finder.Port");


		std::cout << "DataBaseHostName: " << DataBaseHostName << std::endl;
		std::cout << "DataBaseName: " << DataBaseName << std::endl;
		std::cout << "DataBaseUserName: " << DataBaseUserName << std::endl;
		std::cout << "DataBasePassword: " << DataBasePassword << std::endl;
		std::cout << "DataBasePort: " << DataBasePort << std::endl;
		std::cout << "SpiderStarPageURL: " << SpiderStarPageURL << std::endl;
		std::cout << "SpiderDepth: " << SpiderDepth << std::endl;
		std::cout << "FinderAddress: " << FinderAddress << std::endl;
		std::cout << "FinderPort: " << FinderPort << std::endl;
	}
	catch (const std::exception& ex) {

		std::string except = ex.what();
		std::cout << "\n" << except;
	}

	// Создадим подключение к базе данных
	thread_pool task_queue(16);
	database DB;

	try {
		DB.SetConnection(DataBaseHostName, DataBaseName, DataBaseUserName, DataBasePassword, DataBasePort);
		DB.table_delete();
		DB.table_create();
	}
	catch (const std::exception& ex) {
		std::cout << "Try to create tables in databse\n";
		std::string except = ex.what();
		std::cout << "\n" << except;
	}

	
	//std::set<std::string> indexator_result = indexator(DB, SpiderStarPageURL);

	int threads_num = std::thread::hardware_concurrency();
	thread_pool pool(threads_num);
	//std::set<std::string> indexator_result{ "www.lib.ru", "4put.ru", "www.recself.ru", "artofwar.ru"};
	//Тестовая индексация
	std::set<std::string> indexator_result = {
		"4put.ru",
		"artofwar.ru",
		"artofwar.ru/b/bobrov",
		"artofwar.ru/f/frolow_i_a",
		"az.lib.ru",
		"det.lib.ru",
		"fan.lib.ru",
		"fan.lib.ru/e/eskov",
		"fan.lib.ru/g/gromow_a_n",
		"lib.ru",
		"lib.ru/~moshkow",
		"lit.lib.ru/t/ten_w_k",
		"music.lib.ru",
		"okopka.ru",
		"okopka.ru/s/sawenkowa_f",
		"pressa-voiny.ru",
		"samlib.ru",
		"samlib.ru/s/shainjan_k_s",
		"top100.rambler.ru/top100/Literature",
		"turizm.lib.ru",
		"world.lib.ru",
		"www.artlib.ru",
		"www.kulichki.com/moshkow",
		"www.lib.ru",
		"www.lib.ru/Forum",
		"www.lib.ru/HISTORY",
		"www.lib.ru/HITPARAD",
		"www.lib.ru/INPROZ",
		"www.lib.ru/POEZIQ",
		"www.lib.ru/PROZA",
		"www.lib.ru/RUFANT",
		"www.lib.ru/RUSS_DETEKTIW",
		"www.lib.ru/AWARDS",
		"www.lib.ru/Forum",
		"www.lib.ru/MEMUARY/DONBASS",
		"www.lib.ru/Mirrors",
		"www.lib.ru/What-s-new",
		"www.litportal.ru",
		"www.recself.ru"
	};

	// Основной поток выполняет задачи из indexator_result
	std::thread T1 = std::thread([&pool, indexator_result, &DB]() mutable {
		for (const auto& newLink : indexator_result) {
			pool.submit([&DB, newLink] {
				std::cout << "Task submitted for: " << newLink << std::endl;
				indexator(DB, newLink);
				//std::this_thread::sleep_for(std::chrono::seconds(1));
				});
		}
		});
	T1.join(); // Завершения основного потока
	std::thread T2 = std::thread([&pool] {pool.work(); });
	T2.join();
}