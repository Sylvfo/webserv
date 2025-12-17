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

// needs to be replaced
bool HttpRequest::ReceiveBody()
{
	this->BodyComplete = false;
	if (!this->PartialBody.empty())
	{
		this->RawBody.append(this->PartialBody);
		this->PartialBody.clear();
	}

	// For edge-triggered epoll, we must read ALL available data in a loop
	// until we get EAGAIN/EWOULDBLOCK
	std::vector<char> buffer(RECEIVE_CHUNK_SIZE);

	while (true)
	{
		ssize_t bytes = recv(socket_fd, &buffer[0], buffer.size(), 0);

		if (bytes > 0)
		{
			this->RawBody.append(&buffer[0], bytes);

			if (this->RawBody.size() > this->Server->client_max_body_size)
			{
				std::cout << SOFT_RED "[ERROR] Body exceeds max size (413)" << RESET << std::endl;
				this->StatusCode = 413;
				this->AnswerType = ERROR;
				return false;
			}

			// Check if body is complete
			if (!this->IsChunked && this->RawBody.size() >= this->ContentLength)
			{
				if (this->RawBody.size() > this->ContentLength)
					this->RawBody = this->RawBody.substr(0, this->ContentLength);
				this->BodyComplete = true;
				std::cout << LIGHT_CYAN "[BODY] Complete (" << this->RawBody.size() << " bytes)" << RESET << std::endl;
				return true;
			}

			// Continue reading more data (edge-triggered mode)
			continue;
		}
		else if (bytes == 0)
		{
			std::cout << SOFT_RED "[ERROR] Connection closed during body" << RESET << std::endl;
			return false;
		}
		else // bytes < 0
		{
			// Check if it's just no more data available (EAGAIN/EWOULDBLOCK)
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				// Check if we already have the complete body
				if (!this->IsChunked && this->RawBody.size() >= this->ContentLength)
				{
					if (this->RawBody.size() > this->ContentLength)
						this->RawBody = this->RawBody.substr(0, this->ContentLength);
					this->BodyComplete = true;
					std::cout << LIGHT_CYAN "[BODY] Complete (" << this->RawBody.size() << " bytes)" << RESET << std::endl;
					return true;
				}

				// Body not complete yet, will continue on next epoll event
				return true;
			}
			else
			{
				std::cout << SOFT_RED "[ERROR] recv() error: " << strerror(errno) << RESET << std::endl;
				return false;
			}
		}
	}

	return true;
}