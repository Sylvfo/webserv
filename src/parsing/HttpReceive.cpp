#include "HttpRequest.hpp"

bool HttpRequest::receiveHeader()
{
	std::vector<char> temp_buffer(MAX_HEADER_SIZE);
	ssize_t bytes_received = recv(socket_fd, &temp_buffer[0], temp_buffer.size(), 0);

	if (bytes_received > 0)
	{
		this->partial_request.append(&temp_buffer[0], bytes_received);

		if (this->partial_request.size() > MAX_HEADER_SIZE)
		{
			std::cout << SOFT_RED "[RECEIVE_HEADER] 431" << RESET << std::endl;
			this->answer_type = ERROR;
			this->status_code = 431;
			return false;
		}

		size_t seperator_pos = this->partial_request.find("\r\n\r\n");
		if (seperator_pos != std::string::npos)
		{
			std::cout << SOFT_RED "[RECEIVE_HEADER] seperator found" << RESET << std::endl;
			this->raw_header = this->partial_request.substr(0, seperator_pos);
			this->partial_body = this->partial_request.substr(seperator_pos + 4);
			this->header_complete = true;
			this->partial_request.clear();
		}
		else if (seperator_pos == std::string::npos)
		{
			std::cout << SOFT_RED "[RECEIVE_HEADER] uri too long" << RESET << std::endl;
			this->answer_type = ERROR;
			this->status_code = 414;
			return false;
		}
		return true;
	}
	if (bytes_received == 0)
		return false;
	return true;
}

bool HttpRequest::receiveBody()
{
	if (!this->partial_body.empty())
	{
		this->raw_body.append(this->partial_body);
		this->partial_body.clear();
	}

	std::vector<char> buffer(RECEIVE_CHUNK_SIZE);

	ssize_t bytes_received = recv(socket_fd, &buffer[0], buffer.size(), 0);
	if (bytes_received > 0)
		this->raw_body.append(&buffer[0], bytes_received);
	else if (bytes_received == 0)
		return false;

	if (this->raw_body.size() > this->Server->client_max_body_size)
	{
		this->status_code = 413;
		this->answer_type = ERROR;
		return false;
	}
	if (!this->is_chunked && this->raw_body.size() >= this->content_length)
	{
		if (this->raw_body.size() > this->content_length)
			this->raw_body = this->raw_body.substr(0, this->content_length);
		this->body_complete = true;
	}
	return true;
}