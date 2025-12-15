#include "Webserv.hpp"

void HttpRequest::PostRequest()
{
	std::cout << "POST method to do " << std::endl;
	//to do...

	std::cout << PASTEL_AQUA "Enter POST Request Handler" << RESET << std::endl;

	// std::map<std::string, std::string> headers = HTTPHeader.getHeaders();
	// do we have access to the header content throurg a class?

	std::string ContentType = this->headers["content-type"];

	// debugging prints
	std::cout << "Content-Type: " << ContentType << std::endl;
	std::cout << "Request Body: " << this->RawBody << std::endl;

	// different content types need different parsing
	// thats why we get the content type

	if (ContentType.find("application/x-www-form-urlencoded") != std::string::npos)
	{
		// simple form data
		HandleFormData();
	}
	else if (ContentType.find("multipart/form-data") != std::string::npos)
	{
		// File upload - too complex for now
		StatusCode = 501;
		//StatusCode = "501 Not Implemented";
		AnswerBody = "File upload not yet implemented";
		ContentType = "text/plain";
		ContentLength = AnswerBody.size();
	}
	else if (ContentType.find("application/json") != std::string::npos)
	{
		// JSON - not implemented yet
		StatusCode = 501;
		//StatusCode = "501 Not Implemented";
		AnswerBody = "JSON support coming soon!";
		ContentType = "text/plain";
		ContentLength = AnswerBody.size();
	}
	else
	{
		// Unknown type
		StatusCode = 415;
	//	StatusCode = "415 Unsupported Media Type";
		AnswerBody = "Content-Type not supported: " + ContentType;
		ContentType = "text/plain";
		ContentLength = AnswerBody.size();
	}
}

void HttpRequest::HandleFormData()
{
    std::cout << "Handling form data..." << std::endl;

    // STEP 1: Get upload path (from Step 1.1 above)
    std::string currentUri = this->uri;
    std::string uploadPath;

    // Find the longest matching location (most specific)
    size_t longestMatch = 0;
    int matchedIndex = -1;

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
            }
        }
    }

    if (matchedIndex >= 0)
    {
        uploadPath = Server->locations[matchedIndex].upload_path;
        std::cout << "Matched location: " << Server->locations[matchedIndex].path << std::endl;
        std::cout << "Upload path from config: '" << uploadPath << "'" << std::endl;
    }

    if (uploadPath.empty())
    {
        uploadPath = Server->root + "/uploads";
        std::cout << "No upload_path configured, using default: " << uploadPath << std::endl;
    }

    // STEP 2: Parse form data
    std::map<std::string, std::string> formData = parseFormData(this->RawBody);

    // Decode values
    for (std::map<std::string, std::string>::iterator it = formData.begin();
         it != formData.end(); ++it)
    {
        it->second = urlDecode(it->second);  // Decode in place
    }

    // STEP 3: Generate filename
    std::string timestamp = getCurrentTimestamp();
    std::string filename = uploadPath + "/post_" + timestamp + ".txt";

    std::cout << "Attempting to save to: " << filename << std::endl;

    // STEP 4: Save to file
    std::ofstream outFile(filename.c_str());
    if (!outFile.is_open())
    {
		StatusCode = 500;
        //StatusCode = "500 Internal Server Error";
        AnswerBody = "Failed to save data";
        ContentType = "text/plain";
        ContentLength = AnswerBody.size();
        return;
    }

    outFile << "=== POST Data ===" << std::endl;
    for (std::map<std::string, std::string>::iterator it = formData.begin();
         it != formData.end(); ++it)
    {
        outFile << it->first << ": " << it->second << std::endl;
    }
    outFile.close();

    // STEP 5: Build success response
	StatusCode = 201;
    //StatusCode = "201 Created";
    AnswerBody = "Data saved successfully!\n";
    ContentType = "text/plain";
    ContentLength = AnswerBody.size();
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
