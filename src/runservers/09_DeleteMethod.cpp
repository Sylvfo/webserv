#include "Webserv.hpp"

void HttpRequest::DeleteRequest()
{
	std::cout << "DELETE method to do " << std::endl;
	//to do...

	//get uri from server
	// loop through locations to find longest match

	std::string uri = HTTPHeader.getUri();

	// finde longest match
	size_t longestMatch = 0;
	int matchedIndex = -1;

	for (size_t i = 0; i < Server->locations.size(); i++)
	{
		uri.find()
	}


✅ Get URI from HTTPHeader.getUri()
✅ Loop through Server->locations to find longest match
✨ NEW: Check if "DELETE" is in location.methods → 405 if not
✅ Build file path: Server->root + uri (or use location.root if set)
✅ Use stat() to check if file exists → 404 if not
✅ Use stat() to check if it's NOT a directory → 403 if it is
✅ Use access() to check write permission on parent directory → 403 if denied
✅ Call std::remove(filePath.c_str()) to delete
✅ Check return value: 0 = success (204), non-zero = failure (500)

}