# badhttp
small http server library for c++

small example: 
```cpp
#include "badhttp/badhttp.hpp"

int main() {
	badhttp::Server server;
	server.init(80);

	server.route("/test", [&](badhttp::Request request, badhttp::Response response) {
		std::cout << "method: " << request.getMethod() << std::endl;
		std::cout << "uri: " << request.getUri() << std::endl;
		for (const auto& [key, value] : request.getParameters())
			std::cout << key << ": " << value << std::endl;
		for (const auto& [key, value] : request.getHeaders())
			std::cout << key << ": " << value << std::endl;

		response.setStatus(200);
		response.setHeader("Content-Type", "text/plain");
		response.setContent("Page content ...");
		response.respond();
	});

  // all missing routes will execute the "404" route
	server.route("404", [&](badhttp::Request request, badhttp::Response response) {
		response.setStatus(404);
		response.setHeader("Content-Type", "text/plain");
		response.setContent("Page not found!");
		response.respond();
	});	

	server.run();
	return 0;
}
