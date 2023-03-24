# Updog 🐶: netcat implementation in c

## so what's Updog? (gotcha 😆😆)

Updog is an amateur netcat implementation in c, it can listen to upcoming connections from 1 client and send then receive messages from the client

## Run:

-   use the script:

server side : `bash ./run.sh -l [port]`
client side: `bash ./run.sh -c [ipAdress] [port]`

-   build it manually:
`mkdir build`
`cmake ..`
`make`
`./updog`

## Improvements:

-   [x] start a connection between a server and client
-   [x] send and receive messages
-   [ ] allow user to send multiple messages to the server
-   [ ] allow multiple clients to connect to a server (create rooms)
-   [ ] access those rooms with a username and a password
-   [ ] allow file transfer between client and server
-   [ ] secure the file transfer process



❤️you're more than welcome to contribute❤️


| :exclamation:  this was tested only on linux  |
|-----------------------------------------|

MIT License
Copyright (c) 2023 duckduckcodes
