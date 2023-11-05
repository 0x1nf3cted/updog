# Updog 🐶: nterminal chat app

## so what's Updog? (gotcha 😆😆)

Updog is a chat app written in c, it can listen to upcoming connections from 1 client and send then receive messages from the client

## Run:

### Prerequisit:

you should have cmake installed in your system

**for linux**:

`sudo apt-get install cmake`

then run: 

```shell
chmod +x ./run.sh
./run.sh [file_name]  # for now dix only support absolute paths (ex: /home/user/path/to/file/file.txt) this will open file.txt
```






## Improvements:


-   [x] start a connection between a server and client
-   [x] send and receive messages
-   [x] multiple clients to connect to a server
-   [x] user can send multiple messages to the server
-   [x] stop the connection when the client is inactive for 1min
-   [x] make the user disconnect when he type /q after ensuring the message was sent succesfully (Bug)



### Features:
-   [ ] show the username of the client, and the time when the message was sent
-   [ ] access those rooms with a username and a password
-   [ ] allow file transfer between client and server
-   [ ] secure the file transfer process



### Todo:

❤️you're more than welcome to contribute❤️


### Example:

![video](examples/example.png)

| :exclamation:  this was tested only on linux  |
|-----------------------------------------|

MIT License
Copyright (c) 2023 duckduckcodes
