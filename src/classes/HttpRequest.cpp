#include "../inc/HttpRequest.hpp"

// --- Constructor ---
HttpRequest::HttpRequest()
	// --- Server & Connection ---
	: Server(NULL)
	, socket_fd(-1)

	// --- Request State ---
	, HeaderComplete(false)
	, RequestComplete(false)

	// --- Body Handling ---
	, ExpectingBody(false)
	, IsChunked(false)
	, BodyComplete(false)
	, ContentLength(0)

	// --- Response ---
	, AnswerType(STATIC)
	, StatusCode(200)
	, fd_Ressource(-1)
{
}

// --- Copy Constructor ---
HttpRequest::HttpRequest(const HttpRequest& other)
	: Server(other.Server)
	, socket_fd(other.socket_fd)
	, method(other.method)
	, uri(other.uri)
	, version(other.version)
	, Path(other.Path)
	, headers(other.headers)
	, HeaderComplete(other.HeaderComplete)
	, RequestComplete(other.RequestComplete)
	, IsDirectory(other.IsDirectory)
	, PartialRequest(other.PartialRequest)
	, RawHeader(other.RawHeader)
	, RedirectionUrl(other.RedirectionUrl)
	, ExpectingBody(other.ExpectingBody)
	, IsChunked(other.IsChunked)
	, BodyComplete(other.BodyComplete)
	, ContentLength(other.ContentLength)
	, RawBody(other.RawBody)
	, PartialBody(other.PartialBody)
	, AnswerType(other.AnswerType)
	, StatusCode(other.StatusCode)
	, fd_Ressource(other.fd_Ressource)
	, HttpAnswer(other.HttpAnswer)
	, ContentType(other.ContentType)
	, AnswerBody(other.AnswerBody)
{
}


// --- Assignment Operator ---
HttpRequest& HttpRequest::operator=(const HttpRequest& other)
{
	if (this != &other)
	{
		Server = other.Server;
		socket_fd = other.socket_fd;
		method = other.method;
		uri = other.uri;
		version = other.version;
		Path = other.Path;
		headers = other.headers;
		HeaderComplete = other.HeaderComplete;
		PartialRequest = other.PartialRequest;
		RawHeader = other.RawHeader;
		RequestComplete = other.RequestComplete;
		ExpectingBody = other.ExpectingBody;
		IsChunked = other.IsChunked;
		BodyComplete = other.BodyComplete;
		ContentLength = other.ContentLength;
		RawBody = other.RawBody;
		PartialBody = other.PartialBody;
		AnswerType = other.AnswerType;
		StatusCode = other.StatusCode;
		HttpAnswer = other.HttpAnswer;
		ContentType = other.ContentType;
		AnswerBody = other.AnswerBody;
		fd_Ressource = other.fd_Ressource;
	}
	return *this;
}

// --- Destructor ---
HttpRequest::~HttpRequest()
{
}