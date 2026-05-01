#include "../inc/HttpRequest.hpp"

HttpRequest::HttpRequest()
	: Server(NULL)
	, socket_fd(-1)
	, header_complete(false)
	, request_complete(false)
	, expecting_body(false)
	, is_chunked(false)
	, body_complete(false)
	, content_length(0)
	, answer_type(STATIC)
	, status_code(200)
	, fd_ressource(-1)
{
}

HttpRequest::HttpRequest(const HttpRequest& other)
	: Server(other.Server)
	, socket_fd(other.socket_fd)
	, method(other.method)
	, uri(other.uri)
	, version(other.version)
	, path(other.path)
	, headers(other.headers)
	, header_complete(other.header_complete)
	, request_complete(other.request_complete)
	, is_directory(other.is_directory)
	, partial_request(other.partial_request)
	, raw_header(other.raw_header)
	, redirection_url(other.redirection_url)
	, expecting_body(other.expecting_body)
	, is_chunked(other.is_chunked)
	, body_complete(other.body_complete)
	, content_length(other.content_length)
	, raw_body(other.raw_body)
	, partial_body(other.partial_body)
	, answer_type(other.answer_type)
	, status_code(other.status_code)
	, fd_ressource(other.fd_ressource)
	, http_answer(other.http_answer)
	, content_type(other.content_type)
	, answer_body(other.answer_body)
{
}

HttpRequest& HttpRequest::operator=(const HttpRequest& other)
{
	if (this != &other)
	{
		Server = other.Server;
		socket_fd = other.socket_fd;
		method = other.method;
		uri = other.uri;
		version = other.version;
		path = other.path;
		headers = other.headers;
		header_complete = other.header_complete;
		partial_request = other.partial_request;
		raw_header = other.raw_header;
		request_complete = other.request_complete;
		expecting_body = other.expecting_body;
		is_chunked = other.is_chunked;
		body_complete = other.body_complete;
		content_length = other.content_length;
		raw_body = other.raw_body;
		partial_body = other.partial_body;
		answer_type = other.answer_type;
		status_code = other.status_code;
		http_answer = other.http_answer;
		content_type = other.content_type;
		answer_body = other.answer_body;
		fd_ressource = other.fd_ressource;
	}
	return *this;
}

HttpRequest::~HttpRequest()
{
}