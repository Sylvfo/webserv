What the subject says about allowed functions:
"Make sure to leverage as many C++ features as possible (e.g., choose <cstring>
over <string.h>). You are allowed to use C functions, but always prefer their C++
versions if possible."


Magic command to check for forbidden functions in Webserv:
`nm -uC ./webserv | grep -vE "std::|operator new|operator delete|typeinfo|__cxa|__gxx|__dso|__libc|__stack|__errno|__isoc|__gmon|_Unwind|_ITM" | grep -vE "\b(execve|pipe|strerror|gai_strerror|errno|dup|dup2|fork|socketpair|htons|htonl|ntohs|ntohl|select|poll|epoll_create|epoll_ctl|epoll_wait|kqueue|kevent|socket|accept|listen|send|recv|chdir|bind|connect|getaddrinfo|freeaddrinfo|setsockopt|getsockname|getprotobyname|fcntl|close|read|write|waitpid|kill|signal|access|stat|open|opendir|readdir|closedir)\b"`

magic command with ignoring
Removes "weak" symbols (marked with w). These are usually system-provided
weak references that you don't need to worry about.
`nm -uC ./webserv | grep -vE "^\s+w\s+" | grep -vE "std::|operator new|operator delete|typeinfo|__cxa|__gxx|__dso|__libc|__stack|__errno|__isoc|__gmon|_Unwind|_ITM" | grep -vE "\b(execve|pipe|strerror|gai_strerror|errno|dup|dup2|fork|socketpair|htons|htonl|ntohs|ntohl|select|poll|epoll_create|epoll_ctl|epoll_wait|kqueue|kevent|socket|accept|listen|send|recv|chdir|bind|connect|getaddrinfo|freeaddrinfo|setsockopt|getsockname|getprotobyname|fcntl|close|read|write|waitpid|kill|signal|access|stat|open|opendir|readdir|closedir)\b"`



magic command with extra functions:
`nm -uC ./webserv | grep -vE "^\s+w\s+" | grep -vE "std::|operator new|operator delete|typeinfo|__cxa|__gxx|__dso|__libc|__stack|__errno|__isoc|__gmon|_Unwind|_ITM" | grep -vE "\b(execve|pipe|strerror|gai_strerror|errno|dup|dup2|fork|socketpair|htons|htonl|ntohs|ntohl|select|poll|epoll_create|epoll_ctl|epoll_wait|kqueue|kevent|socket|accept|listen|send|recv|chdir|bind|connect|getaddrinfo|freeaddrinfo|setsockopt|getsockname|getprotobyname|fcntl|close|read|write|waitpid|kill|signal|access|stat|open|opendir|readdir|closedir)\b" | grep -vE "memcmp|memset|remove|strlen|time|tolower"`

notes for added functions:

they cannot be found in the codebase or there is an explanation, so they get silenced

not in code base, so gets inserted by compiler:
memcmp
memset
strlen
time*
tolower


other reasons:

remove:
The Subject's "External Functions" list does not include `unlink` (the standard system call to delete files).
It's also forbidden to use `fork` for anything other than CGI.
This means we cannot `fork` and `execve` the `rm` command to delete a file.
Since implementing the `DELETE` method is mandatory, the only legal way to delete a file is to use the C++ Standard Library function `std::remove` (which is part of C++98 standards)

*time:
we use <ctime> which is the correct implementation of the c wrapper in cpp, using <sys/time.h> would not be correct.
native c++ time functions are only added after c++11, so we had to use this one.
