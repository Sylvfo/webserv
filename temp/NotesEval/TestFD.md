# Test all fd are closed

## sleep

```cpp
//add sleep 2x to the main
int main(int ac, char **av)
{
	signal(SIGINT, handleSignInt);
	signal(SIGPIPE, SIG_IGN);

	WebServ *WeServ = NULL;
	WeServ = new WebServ;
	//At the beginning
	sleep(25);
	try
	{
		if (ac == 2)
			WeServ->parseConfig(av[1]);
		else if (ac == 1)
			WeServ->parseConfig("config/default.conf");
		else
		{
			delete WeServ;
			return 1;
		}
		WeServ->startServers();
		while(WeServ->epollWaiting() == true)
			;
	}
	catch (const std::exception &e)
	{
		std::cerr << SOFT_RED "[ERROR] Exception - " << e.what() << RESET << std::endl;
		delete WeServ;
		return 1;
	}
	delete WeServ;
	//At the end
	sleep(25);
	return 0;
}
```

## make it run

## look at the fd while running

open a new terminal

```bash
pidof webserv
//au demarage
ls -l /proc/PID/fd
//a la fin
ls -l /proc/PID/fd
```
//verifier zombies.
ps aux | grep defunct

//memory used
top -p $(pidof mon_programme)

## with siege

open a third terminal



```bash
pidof webserv
//au demarage
ls -l /proc/PID/fd
//a la fin
ls -l /proc/PID/fd
```

launch ./webserv

find pidof

launch siege in the third terminal

## test more thant 1024 fd
```bash
vim /home/sforster/SiegeBase/etc/siegerc
//I
//change limits
//:wq
cd -

siege -t2M -c1500 http://localhost:2224
```


ls /proc/3193021/fd