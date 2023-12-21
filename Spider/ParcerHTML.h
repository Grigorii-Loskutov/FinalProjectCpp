#pragma once
#include <vector>
#include <string>
#include <map>
#include <set>

class ParcerHTML {
private:
	std::string Words; // ������ ��� �������� ����� �������� ��������, ��������� �� �������������� (������ �����)
	std::set<std::string> Links; // ������������� ��� �������� ������, ��������� �� ��������
	std::map<std::string, int> Frequencies; // ����� ��� �������� ������ ��������� ���� (����: �����, ���������� � �������)
public:
	//ParcerHTML(std::vector<std::string> HTML_strings);
	ParcerHTML(std::string HTML_strings);
	std::set<std::string> getLinks();
	std::string getWords();
};