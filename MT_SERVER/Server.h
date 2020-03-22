#pragma once
#include "generalInclude.h"
#include "proto.h"

class Server
{
public:
	Server();
	~Server();
	void serve(int port);

private:

	void accept();
	void clientHandler(SOCKET clientSocket);
	void fileHandler(std::string firstUser, std::string secondUser, std::string cont);
	std::string getChat(std::string firstUser, std::string secondUser);

	SOCKET _serverSocket;
	std::set <std::string> _users;
	Helper help;
	std::map <std::string, std::mutex*> files;

};


