#pragma once

#include <pqxx/pqxx>
#include <string.h>
#include <set>
#include <map>
#include "database.h"
#include "HTTPclient.h"
#include "ParcerHTML.h"

HTTPclient client; // ������ ��� ���������� ��������
std::set<std::string> Links; // ����� ������, ��������� �� ��������
std::map<std::string, int> Frequencies; // ������� ����, ��������� �� ��������
int link_id; // ������������� ��������, ������� �����������
std::map<std::string, int> WordIdPair; // �������������� � �������������� ����� � �������

std::set<std::string> indexator(database& DB, std::string inLink);

