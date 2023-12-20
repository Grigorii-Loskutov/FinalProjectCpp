template <class Body, class Allocator>
http::message_generator
handle_request(
    beast::string_view doc_root,
    http::request<Body, http::basic_fields<Allocator>>&& req)
{
    // ...

    if (req.method() == http::verb::post && req.target() == "/search")
    {
        // Добавьте обработку данных, пришедших с запросом POST
        // Выполните поиск на основе полученных данных
        // Сформируйте новую страницу с результатами поиска
        // Поместите результаты поиска в HTML и сформируйте ответ сервера с этой страницей
        // Например:
        std::string queryData; // Полученные данные из POST-запроса
        // Выполните поиск на основе queryData и сохраните результаты в переменную searchResults

        std::ostringstream body;
        body << "<html>\n"
             << "<head><title>Результаты поиска</title></head>\n"
             << "<body>\n"
             << "<div class=\"container\">\n"
             << "<h1>Результаты поиска</h1>\n"
             << "<p>Результаты вашего поиска: " << queryData << "</p>\n";
        // Добавьте здесь ваши результаты поиска в HTML
        for (const auto& result : searchResults)
        {
            body << "<p>" << result << "</p>\n";
        }
        body << "</div>\n"
             << "</body>\n"
             << "</html>\n";

        http::response<http::string_body> res{
            std::piecewise_construct,
            std::make_tuple(std::move(body.str())),
            std::make_tuple(http::status::ok, req.version()) };

        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.content_length(body.str().size());
        res.keep_alive(req.keep_alive());

        return res;
    }

    // ...
}