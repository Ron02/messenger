#pragma comment (lib, "ws2_32.lib")
#include "WSAInitializer.h"
#include "Server.h"

int main () 
{
		try
		{
			WSAInitializer wsaInit;
			Server myServer;

			myServer.serve(8826);
		}
		catch (std::exception & e)
		{
			std::cout << "Error occured: " << e.what() << std::endl;
		}
		system("PAUSE");
		return 0;
}