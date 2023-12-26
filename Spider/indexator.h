#pragma once

#include <pqxx/pqxx>
#include <string.h>
#include <set>
#include <map>
#include "database.h"
#include "HTTPclient.h"
#include "ParcerHTML.h"

HTTPclient client; // Клиент для скачивания страницы
std::set<std::string> Links; // Набор ссылок, найденных на странице
std::map<std::string, int> Frequencies; // Частоты слов, найденных на странице
int link_id; // Идентификатор страницы, которую индексируем
std::map<std::string, int> WordIdPair; // Идентификаторы и соответсвующие слова в таблице

std::set<std::string> indexator(database& DB, std::string inLink);

