#include "Webserv.hpp"

void HttpRequest::PostRequest()
{
	std::cout << SOFT_PINK "[POST] Processing POST request" << RESET << std::endl;

	std::string ContentType = this->headers["content-type"];

	// debugging prints
	std::cout << SOFT_PINK "[POST] Content-Type: " << ContentType << RESET << std::endl;
	std::cout << SOFT_PINK "[POST] Request Body size: " << this->RawBody.size() << " bytes" << RESET << std::endl;

	// different content types need different parsing
	// thats why we get the content type

	if (ContentType.find("application/x-www-form-urlencoded") != std::string::npos)
	{
		std::cout << SOFT_PINK "[POST] Handling form-urlencoded data" << RESET << std::endl;
		// simple form data
		HandleFormData();
	}
	else if (ContentType.find("multipart/form-data") != std::string::npos)
	{
		std::cout << SOFT_RED "[POST] File upload not yet implemented (501)" << RESET << std::endl;
		// File upload - too complex for now
		StatusCode = 501;
		//StatusCode = "501 Not Implemented";
		AnswerBody = "File upload not yet implemented";
		ContentType = "text/plain";
		ContentLength = AnswerBody.size();
	}
	else if (ContentType.find("application/json") != std::string::npos)
	{
		std::cout << SOFT_RED "[POST] JSON not yet supported (501)" << RESET << std::endl;
		// JSON - not implemented yet
		StatusCode = 501;
		//StatusCode = "501 Not Implemented";
		AnswerBody = "JSON support coming soon!";
		ContentType = "text/plain";
		ContentLength = AnswerBody.size();
	}
	else
	{
		std::cout << SOFT_RED "[POST] Unsupported Content-Type (415)" << RESET << std::endl;
		// Unknown type
		StatusCode = 415;
	//	StatusCode = "415 Unsupported Media Type";
		AnswerBody = "Content-Type not supported: " + ContentType;
		ContentType = "text/plain";
		ContentLength = AnswerBody.size();
	}
	std::cout << SOFT_PINK "[POST] POST request processing complete" << RESET << std::endl;
}

void HttpRequest::HandleFormData()
{
    std::cout << SOFT_PINK "[HANDLE_FORM] Handling form data..." << RESET << std::endl;

    // STEP 1: Get upload path (from Step 1.1 above)
    std::string currentUri = this->uri;
    std::string uploadPath;

    // Find the longest matching location (most specific)
    size_t longestMatch = 0;
    int matchedIndex = -1;

    std::cout << SOFT_PINK "[HANDLE_FORM] Searching for matching location for URI: " << currentUri << RESET << std::endl;
    for (size_t i = 0; i < Server->locations.size(); i++)
    {
        std::string locationPath = Server->locations[i].path;
        // Check if URI starts with this location path
        if (currentUri.find(locationPath) == 0)
        {
            // Check if this is a longer (more specific) match
            if (locationPath.length() > longestMatch)
            {
                longestMatch = locationPath.length();
                matchedIndex = i;
                std::cout << SOFT_PINK "[HANDLE_FORM] Found better match: " << locationPath << " (length: " << locationPath.length() << ")" << RESET << std::endl;
            }
        }
    }

    if (matchedIndex >= 0)
    {
        uploadPath = Server->locations[matchedIndex].upload_path;
        std::cout << SOFT_PINK "[HANDLE_FORM] Matched location: " << Server->locations[matchedIndex].path << RESET << std::endl;
        std::cout << SOFT_PINK "[HANDLE_FORM] Upload path from config: '" << uploadPath << "'" << RESET << std::endl;
    }

    if (uploadPath.empty())
    {
        uploadPath = Server->root + "/uploads";
        std::cout << SOFT_PINK "[HANDLE_FORM] No upload_path configured, using default: " << uploadPath << RESET << std::endl;
    }

    // STEP 2: Parse form data
    std::cout << SOFT_PINK "[HANDLE_FORM] Parsing form data..." << RESET << std::endl;
    std::map<std::string, std::string> formData = parseFormData(this->RawBody);
    std::cout << SOFT_PINK "[HANDLE_FORM] Parsed " << formData.size() << " form field(s)" << RESET << std::endl;

    // Decode values
    for (std::map<std::string, std::string>::iterator it = formData.begin();
         it != formData.end(); ++it)
    {
        it->second = urlDecode(it->second);  // Decode in place
        std::cout << SOFT_PINK "[HANDLE_FORM] Field: " << it->first << " = " << it->second << RESET << std::endl;
    }

    // STEP 3: Generate filename
    std::string timestamp = getCurrentTimestamp();
    std::string filename = uploadPath + "/post_" + timestamp + ".txt";

    std::cout << SOFT_PINK "[HANDLE_FORM] Attempting to save to: " << filename << RESET << std::endl;

    // STEP 4: Save to file
    std::ofstream outFile(filename.c_str());
    if (!outFile.is_open())
    {
		std::cout << SOFT_RED "[HANDLE_FORM] Failed to save data (500)" << RESET << std::endl;
		StatusCode = 500;
        //StatusCode = "500 Internal Server Error";
        AnswerBody = "Failed to save data";
        ContentType = "text/plain";
        ContentLength = AnswerBody.size();
        return;
    }

    std::cout << SOFT_PINK "[HANDLE_FORM] Writing data to file..." << RESET << std::endl;
    outFile << "=== POST Data ===" << std::endl;
    for (std::map<std::string, std::string>::iterator it = formData.begin();
         it != formData.end(); ++it)
    {
        outFile << it->first << ": " << it->second << std::endl;
    }
    outFile.close();
    std::cout << SOFT_PINK "[HANDLE_FORM] File saved successfully" << RESET << std::endl;

    // STEP 5: Build success response
	StatusCode = 201;
    //StatusCode = "201 Created";
    AnswerBody = "Data saved successfully!\n";
    ContentType = "text/plain";
    ContentLength = AnswerBody.size();
    std::cout << SOFT_PINK "[HANDLE_FORM] Form data handling complete (201 Created)" << RESET << std::endl;
}

std::map<std::string, std::string> HttpRequest::parseFormData(const std::string& body)
{
	std::map<std::string, std::string> result;

	// split at &
	size_t start = 0;
	size_t end = body.find('&');

	while (end != std::string::npos)
	{
		std::string pair = body.substr(start, end - start);
		// Split by =
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


	// Handle last pair (no trailing &)
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

std::string HttpRequest::getCurrentTimestamp()
{
	time_t now = time(NULL);
	std::ostringstream oss;
	oss << now;
	return oss.str();
}
