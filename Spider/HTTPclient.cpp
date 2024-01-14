#include "HTTPClient.h"
#include "root_certificates.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/locale.hpp>
#include <regex>
#include <cstdlib>
#include <iostream>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;

void HTTPclient::performGetRequest(const std::string& host, const std::string& port,
	const std::string& target, int version_in) {


	int version = 11;//version_in == 5 && !std::strcmp("1.0", argv[4]) ? 10 : 11;

	// The io_context is required for all I/O
	net::io_context ioc;
	if (port == "80")
	{
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
				//lines.clear();
				//beast::error_code ec;
				//stream.socket().shutdown(tcp::socket::shutdown_both, ec);
				// Проверено с помощью логиривания, что такая страница плохая, из неё данные не нужно брать
				//throw std::domain_error("\nUndefined charset for page: " + host + target + /*"\n" + lines +*/ "\n" + "-> most likely has been moved\n");
			}
		}

		// Выполним перекодировку
		const std::string UTF8{ "UTF-8" };
		if ((charset.length() != 0) && charset != UTF8)
		{
			std::string utf8_line = boost::locale::conv::between(lines, UTF8, charset);
			lines = std::move(utf8_line);
		}

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
	}
	// If we get here then the connection is closed gracefully
	else if (port == "443") {
		// The io_context is required for all I/O
		net::io_context ioc;

		// The SSL context is required, and holds certificates
		ssl::context ctx(ssl::context::tlsv12_client);

		// This holds the root certificate used for verification
		load_root_certificates(ctx);

		// Verify the remote server's certificate
		ctx.set_verify_mode(ssl::verify_peer);

		// These objects perform our I/O
		tcp::resolver resolver(ioc);

		beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);

		// Set SNI Hostname (many hosts need this to handshake successfully)
		if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str()))
		{
			beast::error_code ec{ static_cast<int>(::ERR_get_error()), net::error::get_ssl_category() };
			throw beast::system_error{ ec };
		}

		// Look up the domain name
		auto const results = resolver.resolve(host.c_str(), port.c_str());

		// Make the connection on the IP address we get from a lookup
		//stream.connect(results);
		beast::get_lowest_layer(stream).connect(results);//<------

		// Perform the SSL handshake
		stream.handshake(ssl::stream_base::client);

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

		std::stringstream response_stream;
		response_stream << res;

		// Определим charset из Content-Type
		std::string charset;
		if (std::regex_search(TypeHeaderStr, match, charsetPattern)) {
			if (match.size() > 1) {
				charset = match[1];
			}
		}

		// Составим одну большую строку для всей страницы
		std::string line;
		while (std::getline(response_stream, line)) {
			lines.append(" ");
			lines.append(line);
		}
		//std::cout << lines;
		// Определим charset из meta tag в случае отсутствия его в Content-Type
		if (charset.length() == 0)
		{
			std::regex charsetPattern_(R"(charset\s*['"]?([^'">\s]+)['"]?\s*[>,;\s*])", std::regex::icase);
			std::smatch match_;
			if (std::regex_search(lines, match_, charsetPattern_)) {
				charset = match_[1];
			}
			else {
				//lines.clear();
				// Проверено с помощью логиривания, что такая страница плохая, из неё данные не нужно брать
				//throw std::domain_error("\nUndefined charset for page: " + host + target + /*"\n" + lines +*/ "\n" + "-> most likely has been moved\n");
			}
		}

		// Выполним перекодировку
		if (charset.length() != 0)
		{
			const std::string UTF8{ "UTF-8" };
			std::string utf8_line = boost::locale::conv::between(lines, UTF8, charset);
			lines = std::move(utf8_line);
		}
		// Gracefully close the socket
		beast::error_code ec;
		stream.shutdown(ec);

		if (ec == net::error::eof)
		{
			// Rationale:
			// http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
			ec = {};
		}
		if (ec)
			throw beast::system_error{ ec };
	}
	else {
		throw std::domain_error("\nThis is not HTTP or HTTPS port: " + port + "\n");
	}
}

std::string HTTPclient::getData() {

	if (lines.length()) return lines;
}