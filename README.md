# webserv
This project is about writing your own HTTP server. You will be able to test it with an actual browser. HTTP is one of the most widely used protocols on the internet. Understanding its intricacies will be useful, even if you won’t be working on a website

./webserv [configuration file]


ok so let's make a readme with the following structure:

- what is the webserv project?
- how we did our webserv
- how to run it
- how we tested it


Magic command to check for forbidden functions in Webserv:
`nm -uC ./webserv | grep -vE "std::|operator new|operator delete|typeinfo|__cxa|__gxx|__dso|__libc|__stack|__errno|__isoc|__gmon|_Unwind|_ITM" | grep -vE "\b(execve|pipe|strerror|gai_strerror|errno|dup|dup2|fork|socketpair|htons|htonl|ntohs|ntohl|select|poll|epoll_create|epoll_ctl|epoll_wait|kqueue|kevent|socket|accept|listen|send|recv|chdir|bind|connect|getaddrinfo|freeaddrinfo|setsockopt|getsockname|getprotobyname|fcntl|close|read|write|waitpid|kill|signal|access|stat|open|opendir|readdir|closedir)\b"`