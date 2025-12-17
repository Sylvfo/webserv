#include "HttpRequest.hpp"

bool HttpRequest::ReceiveHeader()
{
	std::vector<char> temp_buffer(MAX_HEADER_SIZE);
	ssize_t bytes_received = recv(socket_fd, &temp_buffer[0], temp_buffer.size(), 0);

	if (bytes_received > 0)
	{
		this->PartialRequest.append(&temp_buffer[0], bytes_received);

		if (this->PartialRequest.size() > MAX_HEADER_SIZE)
		{
			std::cout << SOFT_RED "[RECEIVE_HEADER] 431" << RESET << std::endl;
			this->AnswerType = ERROR;
			this->StatusCode = 431;
			return false;
		}

		size_t seperator_pos = this->PartialRequest.find("\r\n\r\n");
		if (seperator_pos != std::string::npos)
		{
			std::cout << SOFT_RED "[RECEIVE_HEADER] seperator found" << RESET << std::endl;
			this->RawHeader = this->PartialRequest.substr(0, seperator_pos);
			this->PartialBody = this->PartialRequest.substr(seperator_pos + 4);
			this->HeaderComplete = true;
			this->PartialRequest.clear();
		}
		else if (seperator_pos == std::string::npos)
		{
			std::cout << SOFT_RED "[RECEIVE_HEADER] uri too long" << RESET << std::endl;
			this->AnswerType = ERROR;
			this->StatusCode = 414;
			return false;
		}
		return true;
	}
	if (bytes_received == 0)
		return false;
	return true;
}

bool HttpRequest::ReceiveBody()
{
	// First, transfer any partial body from header reception
	if (!this->PartialBody.empty())
	{
		this->RawBody.append(this->PartialBody);
		this->PartialBody.clear();
	}

	// Try to read more data (non-blocking, level-triggered epoll will call us again if needed)
	std::vector<char> buffer(RECEIVE_CHUNK_SIZE);
	ssize_t bytes_received = recv(socket_fd, &buffer[0], buffer.size(), 0);

	if (bytes_received > 0)
	{
		this->RawBody.append(&buffer[0], bytes_received);
	}
	else if (bytes_received == 0)
	{
		std::cout << SOFT_RED "[ERROR] Connection closed during body" << RESET << std::endl;
		return false;
	}
	// bytes_received < 0: no data available right now, epoll will notify us later

	// Check if body exceeds max size
	if (this->RawBody.size() > this->Server->client_max_body_size)
	{
		std::cout << SOFT_RED "[ERROR] Body exceeds max size (413)" << RESET << std::endl;
		this->StatusCode = 413;
		this->AnswerType = ERROR;
		return false;
	}

	// Check if body is complete (for non-chunked transfers)
	if (!this->IsChunked && this->RawBody.size() >= this->ContentLength)
	{
		if (this->RawBody.size() > this->ContentLength)
			this->RawBody = this->RawBody.substr(0, this->ContentLength);
		this->BodyComplete = true;
		std::cout << LIGHT_CYAN "[BODY] Complete (" << this->RawBody.size() << " bytes)" << RESET << std::endl;
	}

	return true;
}