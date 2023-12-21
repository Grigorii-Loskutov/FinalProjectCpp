#pragma once
#include <vector>
#include <string>
#include <map>
#include <set>

class ParcerHTML {
private:
	std::vector<std::string> Words; // ������ ��� �������� ����� �������� ��������, ��������� �� �������������� (������ �����)
	std::set<std::string> Links; // ������������� ��� �������� ������, ��������� �� ��������
	std::map<std::string, int> Frequencies; // ����� ��� �������� ������ ��������� ���� (����: �����, ���������� � �������)
public:
	ParcerHTML(std::vector<std::string> HTML_strings);
	std::set<std::string> getLinks();
	std::vector<std::string> getWords();
};