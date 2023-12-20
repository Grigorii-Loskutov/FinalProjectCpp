#include "HTTPClient.h"


#include <cstdlib>
#include <iostream>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

int HTTPclient::performGetRequest(const std::string& host, const std::string& port,
	const std::string& target, int version_in) {

	try
	{
		// Check command line arguments.
		if (version_in != 4 && version_in != 5)
		{
			std::cerr <<
				"Usage: http-client-sync <host> <port> <target> [<HTTP version: 1.0 or 1.1(default)>]\n" <<
				"Example:\n" <<
				"    http-client-sync www.example.com 80 /\n" <<
				"    http-client-sync www.example.com 80 / 1.0\n";
			return EXIT_FAILURE;
		}

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

		// Write the message to standard out
		//std::cout << res << std::endl;

		std::stringstream response_stream;
		response_stream << res;

		std::string line;
		while (std::getline(response_stream, line)) {
			lines.push_back(line);
		}

		// Gracefully close the socket
		beast::error_code ec;
		stream.socket().shutdown(tcp::socket::shutdown_both, ec);

		// not_connected happens sometimes
		// so don't bother reporting it.
		//
		if (ec && ec != beast::errc::not_connected)
			throw beast::system_error{ ec };

		// If we get here then the connection is closed gracefully
	}
	catch (std::exception const& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

std::vector<std::string> HTTPclient::getData() {
	if (sizeof(lines)) return lines;
}