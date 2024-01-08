#include "HTTPClient.h"

#include <cstdlib>
#include <iostream>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/locale.hpp>
#include <regex>
#include <fstream>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

void HTTPclient::performGetRequest(const std::string& host, const std::string& port,
	const std::string& target, int version_in) {


	int version = 11;//version_in == 5 && !std::strcmp("1.0", argv[4]) ? 10 : 11;

	// The io_context is required for all I/O
	net::io_context ioc;

	// These objects perform our I/O
	tcp::resolver resolver(ioc);
	beast::tcp_stream stream(ioc);

	// Look up the domain name
	auto const results = resolver.resolve(host, port);

	// Make the connection on the IP address we get from a lookup
	stream.connect(results);
	
	// Set up an HTTP GET request message
	http::request<http::string_body> req{ http::verb::get, target, version };
	req.set(http::field::host, host);
	req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

	// Send the HTTP request to the remote host
	http::write(stream, req);

	// This buffer is used for reading and must be persisted
	beast::flat_buffer buffer;

	// Declare a container to hold the response
	http::response<http::dynamic_body> res;

	// Receive the HTTP response
	http::read(stream, buffer, res);

	// Получение значения заголовка Content-Type для определения типа кодировки
	auto contentTypeHeader = res.find("Content-Type");
	
	std::string TypeHeaderStr = contentTypeHeader->value();
	std::regex charsetPattern(R"(charset=([^\s;]+))", std::regex::icase);
	std::sregex_iterator it(TypeHeaderStr.begin(), TypeHeaderStr.end(), charsetPattern);
	std::sregex_iterator end;
	std::smatch match;

	// Определим charset из Content-Type
	std::string charset;
	if (std::regex_search(TypeHeaderStr, match, charsetPattern)) {
		if (match.size() > 1) {
			charset = match[1];
		}
	}

	std::stringstream response_stream;
	response_stream << res;

	// Составим одну большую строку для всей страницы
	std::string line;
	while (std::getline(response_stream, line)) {
		lines.append(" ");
		lines.append(line);
	}

	// Определим charset из meta tag в случае отсутствия его в Content-Type
	if (charset.length() == 0)
	{
		std::regex charsetPattern_(R"(charset\s*['"]?([^'">\s]+)['"]?\s*[>,;\s*])", std::regex::icase);
		std::smatch match_;
		if (std::regex_search(lines, match_, charsetPattern_)) {
			charset = match_[1];
		}
		else {
			lines.clear();
			beast::error_code ec;
			stream.socket().shutdown(tcp::socket::shutdown_both, ec);
			// Проверено с помощью логиривания, что такая страница плохая, из неё данные не нужно брать
			throw std::domain_error("\nUndefined charset for page: " + host + /*"\n" + lines +*/ "\n" + "-> most likely has been moved\n");
		}
	}

	// Выполним перекодировку
	const std::string UTF8{ "UTF-8" };
	std::string utf8_line = boost::locale::conv::between(lines, UTF8, charset);
	lines = std::move(utf8_line);

	// Gracefully close the socket
	beast::error_code ec;
	stream.socket().shutdown(tcp::socket::shutdown_both, ec);

	// not_connected happens sometimes
	// so don't bother reporting it.
	//
	if (ec && ec != beast::errc::not_connected)
	{
		throw beast::system_error{ ec };
	}

	// If we get here then the connection is closed gracefully
}

std::string HTTPclient::getData() {

	if (lines.length()) return lines;
}