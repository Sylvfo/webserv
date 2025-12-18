

#include "../inc/Webserv.hpp"
//http://127.0.0.1:8080/
// c++ My

std::string intToString(int numb);

int MyFirstWebServ()
{
	std::cout << "Attempt to create a server" << std::endl;

	int wsocket;
	int new_wsocket;
	struct sockaddr_in server;
	socklen_t server_len;//avant unsigned int
	int BUFFER_SIZE = 30720;///

	//create a socket
	// for webservers or HTTP web servers we need to creat a TCP socket Transcription Control Protocol
	// celui de la video wsocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
	wsocket = socket(AF_INET, SOCK_STREAM, 0); // = IPV4, stream, 0 = TCP
	if (wsocket == -1)
	{
		std::cout << "Could not create a socket" << std::endl;
	}

	//bind socket to address
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;//inet_addr("127.0.0.1");
	server.sin_port = htons(8080);
	server_len = sizeof(server);

	if (bind(wsocket, (struct sockaddr *)&server, server_len)!= 0)
	{
		std::cout << "Could not bind socket" << std::endl;
	}
	//listening to address
	if (listen(wsocket, 20) != 0)
	{
		std::cout << "Could not start listening" << std::endl;
	}
	std::cout << "Listening on 127.0.0.1:8080" << std::endl;

//*************************************************************** */
	// loop to keep receiving information from our client requests
	while (true)
	{
		// accept client request
		new_wsocket = accept(wsocket, (struct sockaddr *)&server, &server_len);
		if (new_wsocket == -1)
			std::cout << "Could not accept" << std::endl;
		// here security you can make
		// read request
		char buff[30720] = {0};
		int bytes = recv(new_wsocket, buff, BUFFER_SIZE, 0);
		if (bytes < 0){
			std::cout << "Could not read client request";
		}
		std::string serverMessage = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ";
		std::string response = "<html><h1>Hello Babe</h1></html>";
		//lit un fichier
		serverMessage.append(intToString(response.size()));
		serverMessage.append("\n\n");
		serverMessage.append(response);
		int bytesSent = 0;
		int totalBytesSent = 0;
		while(totalBytesSent < (int)serverMessage.size())
		{
			//send
			bytesSent = send(new_wsocket, serverMessage.c_str(), serverMessage.size(), 0);
			if (bytesSent < 0)
			{
				std::cout << "Could not send response" ;
			}
			totalBytesSent += bytesSent;
		}
		std::cout << "sent response to client" << std::endl;
		close(new_wsocket);
	}
	close(wsocket);
	return (0);
}

std::string intToString(int numb)
{
	std::string value;
	std::stringstream out;

	out << numb;
	value = out.str();
	return (value);

}