#pragma once

#include <functional>
#include <iostream>
#include <inttypes.h>
#include <string>
#include <unordered_map>
#include <winsock2.h>

namespace badhttp {

class Request {
private:
	std::string method;
	std::string uri;
	std::unordered_map<std::string, std::string> parameters;
	std::unordered_map<std::string, std::string> headers;
public:
	void parse(const std::string& requestContent);
	Request(const std::string& requestContent);
	inline std::string getMethod() const { return method; };
	inline std::string getUri() const { return uri; };
	inline std::unordered_map<std::string, std::string> getParameters() const { return parameters; };
	inline std::unordered_map<std::string, std::string> getHeaders() const { return headers; };
};

class Response {
private:
	uint16_t status;
	std::unordered_map<std::string, std::string> headers;
	SOCKET clientSocket;
public:
	Response(SOCKET& clientSocket);
	inline void setStatus(const uint16_t newStatus) { status = newStatus; };
	inline void setHeader(const std::string& key, const std::string& value) { headers[key] = value; };
	void returnContent(const std::string& content);

};

class Server {
private:
	std::unordered_map<std::string, std::function<void(SOCKET clientSocket, Request, Response)>> routes;
	SOCKET serverSocket;
	void handleClient(SOCKET& clientSocket);
public:
	void init(const uint16_t port = 80);
	void route(const std::string& path, const std::function<void(Request, Response)>& function);
	void run();
};

}