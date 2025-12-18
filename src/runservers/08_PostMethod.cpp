#include "Webserv.hpp"

void HttpRequest::postRequest()
{
	std::string content_type = this->headers["content-type"];

	// different content types need different parsing
	// thats why we get the content type

	if (content_type.find("application/x-www-form-urlencoded") != std::string::npos)
	{
		// simple form data
		_handleFormData();
	}
	else if (content_type.find("multipart/form-data") != std::string::npos)
	{
		_handleMultipart();
	}
	else if (content_type.find("application/json") != std::string::npos)
	{
		// JSON - not implemented yet
		status_code = 501;
		answer_body = "JSON support coming soon!";
		content_type = "text/plain";
		content_length = answer_body.size();
	}
	else
	{
		// Unknown type
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

    // 1. Define Boundaries
    // Header: boundary=XYZ
    // Body: --XYZ (start) and --XYZ-- (end)
    std::string boundary = "--" + contentType.substr(boundaryPos + 9);
    
    // 2. Loop through raw_body
    size_t pos = 0;
    while (true)
    {
        // Find start of a part
        size_t startPos = this->raw_body.find(boundary, pos);
        if (startPos == std::string::npos) break; 

        // Check if it's the end boundary (--boundary--)
        if (startPos + boundary.length() + 2 <= this->raw_body.length())
        {
            if (this->raw_body.substr(startPos + boundary.length(), 2) == "--")
                break; // End of upload
        }

        // The part data usually starts after boundary + \r\n
        size_t partStart = startPos + boundary.length() + 2;
        
        // Find end of this part (the next boundary)
        size_t nextBoundary = this->raw_body.find(boundary, partStart);
        if (nextBoundary == std::string::npos) break;

        // The content ends before the \r\n that precedes the next boundary
        size_t partEnd = nextBoundary - 2;

        // Isolate the Header vs Body of this part
        size_t headerEnd = this->raw_body.find("\r\n\r\n", partStart);
        if (headerEnd != std::string::npos && headerEnd < partEnd)
        {
            // Extract Part Headers
            std::string partHeaders = this->raw_body.substr(partStart, headerEnd - partStart);
            
            // Extract Part Content (Starts after \r\n\r\n)
            size_t bodyStart = headerEnd + 4;
            std::string partBody = this->raw_body.substr(bodyStart, partEnd - bodyStart);

            // Get Filename
            std::string filename = "default.bin";
            size_t namePos = partHeaders.find("filename=\"");
            if (namePos != std::string::npos)
            {
                size_t endName = partHeaders.find("\"", namePos + 10);
                if (endName != std::string::npos)
                    filename = partHeaders.substr(namePos + 10, endName - (namePos + 10));
            }

            // Determine Upload path
            // (You can copy the logic from _handleFormData to match location blocks here)
            std::string uploadPath = Server->root + "/uploads"; 
            
            // Write File
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
    }

    if (uploadPath.empty())
    {
        uploadPath = Server->root + "/uploads";
    }

    // STEP 2: Parse form data
    std::map<std::string, std::string> formData = _parseFormData(this->raw_body);

    // Decode values
    for (std::map<std::string, std::string>::iterator it = formData.begin();
         it != formData.end(); ++it)
    {
        it->second = _urlDecode(it->second);  // Decode in place
    }

    // STEP 3: Generate filename
    std::string timestamp = _getCurrentTimestamp();
    std::string filename = uploadPath + "/post_" + timestamp + ".txt";

    // STEP 4: Save to file
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

    // STEP 5: Build success response
	status_code = 201;
    answer_body = "Data saved successfully!\n";
    content_type = "text/plain";
    content_length = answer_body.size();
}

std::map<std::string, std::string> HttpRequest::_parseFormData(const std::string& body)
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

std::string HttpRequest::_getCurrentTimestamp()
{
	time_t now = time(NULL);
	std::ostringstream oss;
	oss << now;
	return oss.str();
}
