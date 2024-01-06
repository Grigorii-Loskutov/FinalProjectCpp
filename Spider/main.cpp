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

// Создадим функцию для рекурсивного вызова
void recursiveMultiTreadIndexator(database& DB, int Depth, std::set<std::string> inLinkSet) {
	if (0 == Depth) return;

	std::cout << "\n\t Current recursion level: " << Depth << std::endl;
	std::cout << "\n\t Number of new links of current level: " << inLinkSet.size() << std::endl;


	// Добавим все новые линки в базу данных (обработка исключений внутри класса database)
	//for (const auto& setIter : inLinkSet) {
	//	DB.link_add(setIter);
	//}

	// Определим количество логических процессоров
	int threads_num = std::thread::hardware_concurrency();
	threads_num = inLinkSet.size() > 100 ? 100 : inLinkSet.size();
	std::cout << "\nNumber of treads: " << threads_num << std::endl;

	// Мьютекс для блокировки записи результатов в выходной вектор разными потоками
	std::mutex vectorMutex;
	
	// Создадим пул потоков по количеству логических процессоров
	thread_pool pool(threads_num);

	// Вектор для сбора результатов отдельных потоков
	std::vector<std::tuple <std::string, std::set<std::string>, std::map<std::string, int>>> resultsVector;

	// Счётчик оставшихся для индексации ссылок
	int decrementLinks = inLinkSet.size();

	std::thread T1 = std::thread([&pool, &DB, inLinkSet, Depth, &resultsVector, &vectorMutex, &decrementLinks]() mutable {
		for (const auto& newLink : inLinkSet) {
			pool.submit([&DB, newLink, Depth, &resultsVector, &pool, &vectorMutex, &decrementLinks, inLinkSet] {
				std::cout << "\nRecursion = " << Depth << " -> ";				std::cout << "\nRecursion = " << Depth << " -> ";
				std::cout << "Left links:  " << decrementLinks << " -> ";
				std::cout << "Task submitted for: " << newLink << std::endl;
				// Получим ссылки, найденные на конкретной странице
				std::tuple <std::string, std::set<std::string>, std::map<std::string, int>> indexatorResult = indexator(newLink);

				// Добавим результы в вектор, в который все потоки складывают свои данные (с разделением ресурсов)
				// Защитим доступ к outLinksVector с помощью мьютекса
				std::lock_guard<std::mutex> lock(vectorMutex);

				if (inLinkSet.count(std::get<0>(indexatorResult)) != 0) {
					resultsVector.push_back(indexatorResult);
				}
				else {
					std::cout << "\n Unworked link\n ";
				}
				--decrementLinks;
				});
		}
		});
	T1.join(); // Завершения основного потока
	std::thread T2 = std::thread([&pool] {pool.work(); });
	// Подождём. когда выполнятся все потоки для очередного уровня Depth
	T2.join();
	Sleep(1000);

	// --------   Начнем разбор результатов -------------
	int number;
	// Все уникальные найденные ссылки на всех страницах
	std::set<std::string> outLinksSet;

	// Все уникальные найденные слова на всех страницах
	std::set<std::string> findedWords;

	for (const auto& tupleIter : resultsVector) {

		// Текущая ссылка для добавления
		std::string currentLink = std::get<0>(tupleIter);
		int currentLinkID = -1;

		// Если кортеж не содержит ссылки (страница не была распарсена), то перейдём на следующую итерацию
		if (currentLink.length() == 0)
		{
			//std::cout << "\n Empty input link: " << currentLink << std::endl;
			continue;
		}

		// Сделаем  один большой set из всех найденных ссылок на всех страницах 
		for (const auto& setIter : std::get<1>(tupleIter)) {
			if (setIter.length() == 0)
			{
				std::cout << "\n Empty link\n";
			}
			else
			{
				outLinksSet.insert(setIter);  // Добавляем элементы в выходной outLinksSet
			}
		}

		// Сделаем  один большой set из всех найденных слов на всех страницах данной итерации
		for (const auto& mapIter : std::get<2>(tupleIter)) {
			if (mapIter.first.length() == 0)
			{
				std::cout << "\n Empty word\n";
			}
			{
				findedWords.insert(mapIter.first);  // Добавляем ключи (слова) в выходной findedWords
			}
		}
	}
	std::cout << "\n Level: " << Depth << ", finded new links: " << outLinksSet.size();
	std::cout << "\n Level: " << Depth << ", finded words: " << findedWords.size();

	// Получим таблицу слов с ID целиком
	std::map<std::string, int> WordIdPair = DB.getWordId();
	std::cout << "\n Level: " << Depth << ", now in DataBase words: " << WordIdPair.size();

	bool isNewWordAdd = false;

	// Отдельный set для всех новых слов
	//std::set<std::string> newWords;

	// Найдем новые слова, если такие есть
	std::cout << "\nFind new words...\n";
	int newWordCounter = 0;
	for (const auto& wordIter : findedWords) {
		if (WordIdPair[wordIter] == 0) {
			DB.word_add(wordIter);
			isNewWordAdd = true;
			newWordCounter++;
		}
	}
	std::cout << "\n Level: " << Depth << ", finded new words: " << newWordCounter;

	// Добавим в базу новые слова, если такие нашлись
	std::cout << "\nAdd new words in DataBase...\n";
	if (isNewWordAdd) {

		//for (const auto& setIter : newWords) {
		//	DB.word_add(setIter);
		//}

		// Обновим таблицу слов (с учетом добавленных)
		std::cout << "\n Level: " << Depth << ", now in DataBase words: " << WordIdPair.size();

		// Получим таблицу слов с ID целиком ещё раз
		WordIdPair = DB.getWordId();
	}


	int frecuencyIncrement = 0; // Счётчик добавлений частот
	int linkIncrement = 0; //Счётчик добавлений ссылок
	int failWordsIncrement = 0; // счётчик недобавленных слов?
	// Начнём добавлять в базу новые линки и частоты
	std::cout << "\nAdd new frequencies in DataBase...\n";
	for (const auto& tupleIter : resultsVector) {

		// Текущая ссылка для добавления
		std::string currentLink = std::get<0>(tupleIter);
		int currentLinkID = -1;

		// Если кортеж не содержит ссылки (страница не была распарсена), то перейдём на следующую итерацию
		if (currentLink.length() == 0)
		{
			std::cout << "\n Empty input link: " << currentLink << std::endl;
			continue;
		}
		// Если ссылка ненулевая, добавим ее в базу данных
		else {
			DB.link_add(currentLink);
			// Получим ID текущего линка
			currentLinkID = DB.getLinkId(currentLink);
		}
		
		if (currentLinkID == -1) {
			std::cout << "\n There is no ID for link:" << currentLink << std::endl;
			continue;
		}

		// Добавляем частоты слов, найденных на currentLinkID
		try
		{
			for (const auto& mapIter : std::get<2>(tupleIter)) {
				std::string currentWord = mapIter.first; // Текущее слово
				int currentWordID = WordIdPair[currentWord];
				int currentFrecuency = mapIter.second;  //Текущая частота

				if (currentWordID == 0) {
					//std::cout << "\nThere is no word:" << currentWord << " in DataBase" << ": ID = " << currentWordID;
					++failWordsIncrement;
					continue;
				}
				if (currentFrecuency == 0) {
					std::cout << "\nThere is no frecuency value for word:" << currentWord;
					continue;
				}
				if (currentLinkID <= 0) {
					std::cout << "\nThere is no ID for link:" << currentWord;
					continue;
				}
				// Делаем запись в базу данных
				DB.frequency_add(currentLinkID, currentWordID, currentFrecuency);
				++frecuencyIncrement;
			}
			++linkIncrement;
		}
		catch (const std::exception& ex) {
			std::cout << __FILE__ << ", line: " << __LINE__ << std::endl;
			std::cout << "\n Fail to add frecuency for word: ";
			std::string except = ex.what();
			std::cout << "\n" << except;
		}
	}
	std::cout << "\n Level: " << Depth << ", added  frecuencies: " << frecuencyIncrement << " for links: " << linkIncrement;
	std::cout << "\n Fail words:" << failWordsIncrement << std::endl;
	Sleep(5000);

	// Очистим все контейнеры перед рекурсивным вызововом
	resultsVector.clear();
	//outLinksSet.clear();
	findedWords.clear();
	pool.~thread_pool();
	vectorMutex.~mutex();
	// Запустим рекурсивно индексацию для всех найденных ссылок
	recursiveMultiTreadIndexator(DB, Depth - 1, outLinksSet);
}

std::string DataBaseHostName;
std::string DataBaseName;
std::string DataBaseUserName;
std::string DataBasePassword;
int DataBasePort;

std::string SpiderStarPageURL;
int SpiderDepth;
bool newDB;

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
		newDB = parser.get_value<int>("Spider.NewDB");

		FinderAddress = parser.get_value<std::string>("Finder.Address");
		FinderPort = parser.get_value<int>("Finder.Port");


		std::cout << "DataBaseHostName: " << DataBaseHostName << std::endl;
		std::cout << "DataBaseName: " << DataBaseName << std::endl;
		std::cout << "DataBaseUserName: " << DataBaseUserName << std::endl;
		std::cout << "DataBasePassword: " << DataBasePassword << std::endl;
		std::cout << "DataBasePort: " << DataBasePort << std::endl;
		std::cout << "SpiderStarPageURL: " << SpiderStarPageURL << std::endl;
		std::cout << "SpiderDepth: " << SpiderDepth << std::endl;
		std::cout << "Delete old data base: " << (newDB ? "true" : "false") << std::endl;
		std::cout << "FinderAddress: " << FinderAddress << std::endl;
		std::cout << "FinderPort: " << FinderPort << std::endl;
	}
	catch (const std::exception& ex) {

		std::string except = ex.what();
		std::cout << "\n" << except;
	}

	// Создадим подключение к базе данных
	database DB;

	try {
		DB.SetConnection(DataBaseHostName, DataBaseName, DataBaseUserName, DataBasePassword, DataBasePort);
		if (newDB) { DB.table_delete(); }
		DB.table_create();
	}
	catch (const std::exception& ex) {
		std::cout << "Try to create tables in databse\n";
		std::string except = ex.what();
		std::cout << "\n" << except;
	}
	// Запустим индексатор для первой страницы поиска
	std::set<std::string> inLinkSet; //= indexator(DB, SpiderStarPageURL);
	inLinkSet.insert(SpiderStarPageURL);
	Sleep(1000);
	recursiveMultiTreadIndexator(DB, SpiderDepth, inLinkSet);

	DB.CloseConnection();
}
