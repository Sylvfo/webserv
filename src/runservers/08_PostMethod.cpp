#include "Webserv.hpp"

void HttpRequest::postRequest()
{
	std::string content_type = this->headers["content-type"];

	if (content_type.find("application/x-www-form-urlencoded") != std::string::npos)
	{
		_handleFormData();
	}
	else if (content_type.find("multipart/form-data") != std::string::npos)
	{
		_handleMultipart();
	}
	else
	{
		status_code = 415;
		answer_body = "Content-Type not supported: " + content_type;
		content_type = "text/plain";
		content_length = answer_body.size();
	}
}

void HttpRequest::_handleMultipart()
{
	std::string contentType = this->headers["content-type"];
	size_t boundaryPos = contentType.find("boundary=");
	if (boundaryPos == std::string::npos)
	{
		this->status_code = 400;
		return;
	}
	std::string boundary = "--" + contentType.substr(boundaryPos + 9);

	size_t pos = 0;
	while (true)
	{
		size_t startPos = this->raw_body.find(boundary, pos);
		if (startPos == std::string::npos) break;

		if (startPos + boundary.length() + 2 <= this->raw_body.length())
		{
			if (this->raw_body.substr(startPos + boundary.length(), 2) == "--")
				break;
		}

		size_t partStart = startPos + boundary.length() + 2;

		size_t nextBoundary = this->raw_body.find(boundary, partStart);
		if (nextBoundary == std::string::npos) break;

		size_t partEnd = nextBoundary - 2;

		size_t headerEnd = this->raw_body.find("\r\n\r\n", partStart);
		if (headerEnd != std::string::npos && headerEnd < partEnd)
		{
			std::string partHeaders = this->raw_body.substr(partStart, headerEnd - partStart);

			size_t bodyStart = headerEnd + 4;
			std::string partBody = this->raw_body.substr(bodyStart, partEnd - bodyStart);

			std::string filename = "default.bin";
			size_t namePos = partHeaders.find("filename=\"");
			if (namePos != std::string::npos)
			{
				size_t endName = partHeaders.find("\"", namePos + 10);
				if (endName != std::string::npos)
					filename = partHeaders.substr(namePos + 10, endName - (namePos + 10));
			}

			std::string uploadPath = Server->root + "/uploads";

			std::string fullPath = uploadPath + "/" + filename;
			std::ofstream outFile(fullPath.c_str(), std::ios::binary);
			if (outFile.is_open())
			{
				outFile.write(partBody.c_str(), partBody.size());
				outFile.close();
			}
		}
		pos = nextBoundary;
	}

	this->status_code = 201;
	this->answer_body = "File(s) uploaded successfully.";
}

void HttpRequest::_handleFormData()
{
	std::string currentUri = this->uri;
	std::string uploadPath;

	size_t longestMatch = 0;
	int matchedIndex = -1;

	for (size_t i = 0; i < Server->locations.size(); i++)
	{
		std::string locationPath = Server->locations[i].path;
		if (currentUri.find(locationPath) == 0)
		{
			if (locationPath.length() > longestMatch)
			{
				longestMatch = locationPath.length();
				matchedIndex = i;
			}
		}
	}

	if (matchedIndex >= 0)
	{
		uploadPath = Server->locations[matchedIndex].upload_path;
	}

	if (uploadPath.empty())
	{
		uploadPath = Server->root + "/uploads";
	}

	std::map<std::string, std::string> formData = _parseFormData(this->raw_body);

	for (std::map<std::string, std::string>::iterator it = formData.begin();
			it != formData.end(); ++it)
	{
		it->second = _urlDecode(it->second);  // Decode in place
	}

	std::string timestamp = _getCurrentTimestamp();
	std::string filename = uploadPath + "/post_" + timestamp + ".txt";

	std::ofstream outFile(filename.c_str());
	if (!outFile.is_open())
	{
		status_code = 500;
		answer_body = "Failed to save data";
		content_type = "text/plain";
		content_length = answer_body.size();
		return;
	}

	outFile << "=== POST Data ===" << std::endl;
	for (std::map<std::string, std::string>::iterator it = formData.begin();
			it != formData.end(); ++it)
	{
		outFile << it->first << ": " << it->second << std::endl;
	}
	outFile.close();

	status_code = 201;
	answer_body = "Data saved successfully!\n";
	content_type = "text/plain";
	content_length = answer_body.size();
}

std::map<std::string, std::string> HttpRequest::_parseFormData(const std::string& body)
{
	std::map<std::string, std::string> result;

	size_t start = 0;
	size_t end = body.find('&');

	while (end != std::string::npos)
	{
		std::string pair = body.substr(start, end - start);
		size_t equalPos = pair.find('=');
		if (equalPos != std::string::npos)
		{
			std::string key = pair.substr(0, equalPos);
			std::string value = pair.substr(equalPos + 1);
			result[key] = value;
		}
		start = end + 1;
		end = body.find('&', start);
	}

	std::string pair = body.substr(start);
	size_t equalPos = pair.find('=');
	if (equalPos != std::string::npos)
	{
		std::string key = pair.substr(0, equalPos);
		std::string value = pair.substr(equalPos + 1);
		result[key] = value;
	}
	return result;
}

std::string HttpRequest::_getCurrentTimestamp()
{
	time_t now = std::time(0);
	std::ostringstream oss;
	oss << now;
	return oss.str();
}
