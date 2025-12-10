#include "Webserv.hpp"

// new all functions which are needed for http parsing
// should replace 04_recive_request_.cpp and RequestHeader.cpp

void HttpRequest::ReceiveRequest()
{
	int MaxHeaderSize = 8192; // 8KB nginx standard
	int ChunkSize = 1024;

	//Read until header/body seperator
	std::vector<char> temp_buffer(ChunkSize);

	while (this->RawRequest.find("\r\n\r\n") == std::string::npos)
	{
		ssize_t bytes_received = recv(socket_fd, &temp_buffer[0], temp_buffer.size(), 0);

		// needs to handle according to non blocking
		if (bytes_received == -1)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN) // need to check with sylvie how to handle?
			{
				// Non-blocking condition: No data ready.
				// THIS IS NOT AN ERROR, it is a signal to stop and wait.
				return; // Exit the function gracefully and wait for the next poll event.
			}
			else // a genuin system error occured, very unlikely but still fall back
			{
				this->AnswerType = ERROR;
				this->StatusCode = 500;  // Internal Server Error
				return;
			}
		}

		if (bytes_received == 0)
		{
			this->AnswerType = ERROR;
			this->StatusCode = 400;  // Bad Request
			return;
		}

		this->RawRequest.append(&temp_buffer[0], bytes_received);

		// Check if headers are getting too large
		if (this->RawRequest.size() > MaxHeaderSize)
		{
			this->AnswerType = ERROR;
			this->StatusCode = 431;  // Request Header Fields Too Large
			return;
		}
	}

	size_t seperator_pos = this->RawRequest.find("\r\n\r\n");

	this->RequestHeader = this->RawRequest.substr(0, seperator_pos);
	this->RequestBody = this->RawRequest.substr(seperator_pos + 4);
}
