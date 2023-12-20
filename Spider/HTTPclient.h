#pragma once
#pragma once

#include <string>
#include <vector>

class HTTPclient {
	
public:

	// Function declaration for performing HTTP GET request
	int performGetRequest(const std::string& host, const std::string& port,
		const std::string& target, int version_in);
	std::vector<std::string> getData();
private:
	std::vector<std::string> lines; // получим HTTP построчно
};