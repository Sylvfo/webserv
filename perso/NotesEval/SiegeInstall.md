# Install siege on Linux at 42

BASH
```bash
git clone https://github.com/JoeDog/siege.git siege
cd siege
utils/bootstrap
//create dir SiegeBase in home
./configure --prefix=$HOME/SiegeBase
make
make install

//add the path to bash
nano ~/.bashrc
export PATH=$HOME/SiegeBase/bin:$PATH
. ~/.bashrc
siege --version
nano $HOME/.siege/siege.conf
verbose = false

```

```zhs
//add the path to zhs
nano ~/.zshrc
export PATH=$HOME/SiegeBase/bin:$PATH
. ~/.zshrc
```

# Set Parameters

### webserv config file:
localhost:Port_number

### siege commande line

siege time users -b http://machineIP:Port_number

#### time (but no request)
-t2M *for 2 minutes*
-t20S *for 20 secondes*

#### request (but no time)
-r2000 *2000 requests per users*

#### users:
-c100 *for 100 users*
at school max 255
To increase this limit, search your .siegerc file for 'limit' and change its value.
max fd per process is 1024, so +-/ 800/900 users

#### -b flag
-b *for no delay between requests (mandatory in the eval)*

#### -d flag
-d3 *This option instructs siege how long to delay between each page request. The value NUM represents the number of seconds between each one. This number can be a decimal value. In fact the default is half a second (--delay=0.5)*

#### -v flag
-v *verbose = true*
'> siege.log *in a file*

#### URL

find the machine number for siege:
```bash
ifconfig
```
use the same Port_number

#### multiple URL in a file

create a file 
> touch /home/sforster/SiegeBase/siege-sites.txt

add url
> vim /home/sforster/SiegeBase/siege-sites.txt
http://machineIP:Port_number
http://machineIP:Port_number
http://machineIP:Port_number
I for insert
Esq
:wq

-f *file flag*

# run it

### run webserv
in a terminal, run webserv
```bash
./webserv config/forsiege.conf
```

### run siege
in an other terminal, run siege
```bash
//Exemples
siege -t1M -c2 http://10.12.3.1:2222 

//run siege multiple URL in a file
siege -f /home/sforster/SiegeBase/siege-sites.txt -b -t1M -c2

siege -t2M -d3 -c8 http://10.12.3.1:2222 

//run siege with verbose logs in a file:
siege -t1M -c2 -v http://10.12.3.1:2222 > siege.log
```

see markdown in vsCode: ctrl k + v


# Siege in Docker

Work at school because but too slow for the -b flag

```bash
docker run --rm --network=host lopezs/siege -t1M -c2 -b http://ip:port
```

