#include "Server.h"


Server::Server()
{

	_serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	help = Helper();

	if (_serverSocket == INVALID_SOCKET)
		throw std::exception(__FUNCTION__ " - socket");
}

Server::~Server()
{
	try
	{
		closesocket(_serverSocket);
	}
	catch (...) {}
}

//config and init server to listen on the chosen port
void Server::serve(int port)
{

	struct sockaddr_in sa = { 0 };

	sa.sin_port = htons(port); // port that server will listen for
	sa.sin_family = AF_INET;   // must be AF_INET
	sa.sin_addr.s_addr = INADDR_ANY;    // when there are few ip's for the machine. We will use always "INADDR_ANY"

	// again stepping out to the global namespace
	// Connects between the socket and the configuration (port and etc..)
	if (bind(_serverSocket, (struct sockaddr*) & sa, sizeof(sa)) == SOCKET_ERROR)
		throw std::exception(__FUNCTION__ " - bind");

	// Start listening for incoming requests of clients
	if (listen(_serverSocket, SOMAXCONN) == SOCKET_ERROR)
		throw std::exception(__FUNCTION__ " - listen");
	std::cout << "Listening on port " << port << std::endl;

		// the main thread is only accepting clients 
		// and add then to the list of handlers
	std::cout << "Waiting for client connection request" << std::endl;
	accept();

}


//listen to new clients , when one arrive create new thread for him
void Server::accept()
{
	// notice that we step out to the global namespace
	// for the resolution of the function accept

	// this accepts the client and create a specific socket from server to this client
	while (true) 
	{
		SOCKET client_socket = ::accept(_serverSocket, NULL, NULL);

		if (client_socket == INVALID_SOCKET)
			throw std::exception(__FUNCTION__);

		std::cout << "Client accepted. Server and client can speak" << std::endl;

		// the function that handle the conversation with the client

		std::thread thr(&Server::clientHandler , this , client_socket);
		thr.detach();
	}
}

//client handler , listen to new requests and send answers by proto rules
void Server::clientHandler(SOCKET clientSocket)
{
	bool login = true;
	char msg[500];
	std::string sMsg;
	std::string sMsg1;
	std::string userName =  "";
	try
	{
		while (true)
		{
			if (login)
			{
				recv(clientSocket, msg, 400, 0);
				userName = help.getName(msg);
				_users.insert(userName);
				sMsg = "1010000000" + help.getPaddedNumber(help.getAllUsers(_users).length(), 5) + help.getAllUsers(_users);
				send(clientSocket, sMsg.c_str(), sMsg.size(), 0);
				login = false;
			}
			else 
			{
				recv(clientSocket, msg, 400, 0);
				if (std::string(msg).substr(5 + stoi (std::string(msg).substr(3, 2)) , 5 ) == "00000")
				{
						help.send_update_message_to_client(clientSocket, getChat(userName, help.getName(std::string(msg))), help.getName(std::string(msg)), help.getAllUsers(_users));
				}
				else 
				{
					int msgSize = atoi((std::string(msg).substr(5 + stoi(std::string(msg).substr(3, 2)), 5)).c_str());
					std::string msgCont = std::string(msg).substr(10 + stoi(std::string(msg).substr(3, 2)), msgSize);
					std::string secondName = help.getName(msg);
					fileHandler(userName, secondName, msgCont);
					help.send_update_message_to_client(clientSocket, getChat(userName, help.getName(std::string(msg))), help.getName(std::string(msg)), help.getAllUsers(_users));
				}
			}
			
		}

	}
	catch (const std::exception & e)
	{
		_users.erase(userName);
		closesocket(clientSocket);
	}


}

//handling the file system , when new chat created creats file for him and update existing chats
void Server :: fileHandler (std::string firstUser , std::string secondUser , std::string cont) 
{
	std::fstream file;
	std::string formatCont;
	std::string fileName = firstUser > secondUser ? firstUser + "_" + secondUser +".txt" : secondUser + "_" + firstUser + ".txt";
	if (!files.count(fileName)) 
	{
		std::mutex* mtx = new std::mutex();
		files.insert({ fileName , mtx });
		std::ofstream f (fileName);
		f.close();
	}
	formatCont = "&MAGSH_MESSAGE&&Author&" + firstUser + "&DATA&" + cont;
	files[fileName]->lock();
	file.open(fileName, std::ios::app);
	file << formatCont;
	file.close();
	files[fileName]->unlock();
}

//get the chat file of 2 users names
std::string Server :: getChat (std::string firstUser, std::string secondUser) 
{
	std::string fileName = firstUser > secondUser ? firstUser + "_" + secondUser + ".txt" : secondUser + "_" + firstUser + ".txt";
	std::string line;
	std::string cont = "";
	std::ifstream file;
	int size;
	if (!files.count(fileName))
		return "";
	files[fileName]->lock();
	file.open(fileName);
	while (std::getline(file, line))
		cont += line;
	file.close();
	files[fileName]->unlock();
	return cont;
}

