#include "badhttp.hpp"
#include <sstream>
#include <stdexcept>

namespace badhttp {

void Request::parse(const std::string& content) {
	uint32_t i;

	while (i<content.size() && content[i] != ' ') {
		method += content[i];
		i++;
	}
	i++;

	while (i < content.size() && content[i] != ' ' && content[i] != '?') {
		uri += content[i];
		i++;
	}

	if (i < content.size() && content[i] == '?') {
		i++;
		std::string key, value;
		bool readingKey = true;
		while (i < content.size() && content[i] != ' ') {
			if (content[i] == '=')
				readingKey = false;
			else if (content[i] == '&') {
				parameters[key] = value;
				key.clear();
				value.clear();
				readingKey = true;
			} else {
				if (readingKey)
					key += content[i];
				else
					value += content[i];
			}
			i++;
		}
		if (!key.empty())
			parameters[key] = value;
	}

	while (i < content.size() && content[i] == ' ') {
		i++;
	}

	while (i < content.size()) {
		std::string key, value;
		while (i < content.size() && content[i] != ':' && content[i] != '\r') {
			key += content[i];
			i++;
		}
		if (i < content.size() && content[i] == ':') {
			i++;
		}
		while (i < content.size() && (content[i] == ' ' || content[i] == '\t')) {
			i++;
		}
		while (i < content.size() && content[i] != '\r') {
			value += content[i];
			i++;
		}
		if (!key.empty() && !value.empty()) {
			headers[key] = value;
		}
		if (i + 1 < content.size() && content[i] == '\r' && content[i+1] == '\n') {
			i += 2;
		} else {
			break;
		}			
	}
}

Request::Request(const std::string& content) {
	parse(content);
}

Response::Response(const SOCKET clientSocket) : clientSocket(clientSocket) {}

void Response::respond() {
	std::stringstream responseSS;
	responseSS << "HTTP/1.1 " << status << " OK\r\n";
	responseSS << "Content-Length: " << content.size() << "\r\n";
	for (const auto& [headerKey, headerValue] : headers) {
		responseSS << headerKey << ": " << headerValue << ";\r\n";
	}
	responseSS << "\r\n";
	responseSS << content;
	const std::string responseCString = responseSS.str().c_str();
	send(clientSocket, responseCString.c_str(), responseCString.length(), 0);
	closesocket(clientSocket);	
}

void Server::handleClient(const SOCKET clientSocket) {
	char buffer[1024];
	uint32_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
	if (bytesReceived > 0) {
		buffer[bytesReceived] = '\0';
	}

	Request request(buffer);
	Response response(clientSocket);

	const std::string uri = request.getUri();
	if (routes.find(uri) != routes.end()) {
		routes[uri](clientSocket, request, response);
	} else if (routes.find("404") != routes.end()) {
		routes["404"](clientSocket, request, response);
	}
}

void Server::init(const uint16_t port) {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		throw std::runtime_error("WSAStartup failed");
	}

	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET)
		throw std::runtime_error("Socket creation failed");

	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = htons(port);

	if (bind(serverSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
		throw std::runtime_error("Bind failed");
	if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
		throw std::runtime_error("Listen failed");
}

void Server::route(const std::string& path, const std::function<void(Request, Response)>& function) {
	routes[path] = [function](SOCKET clientSocket, Request request, Response response) {
		function(request, response);
	};
}


void Server::run() {
	while (true) {
		sockaddr_in clientAddress;
		int clientAddressSize = sizeof(clientAddress);
		const SOCKET clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddress, &clientAddressSize);
		if (clientSocket == INVALID_SOCKET)
			continue;

		handleClient(clientSocket);
	}
}

}