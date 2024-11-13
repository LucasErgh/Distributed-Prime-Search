# Distributed Prime Search (_WIP_)

Program with basic server-client relationship that handles multiple connections. Distrubutes ranges of numbers to
search across clients who then send back lists of prime numbers found within those ranges.

## Project Structure

I used CMake and C++ with the winsock2 library. Connections are handled with TCP sockets. Further, there are three
directories, Server, Client, and Common where Common contains a library of code shared by the two applications.

### Server

The server has two main parts, `ServerLogic` class and `SocketManager` class. The `ServerLogic` is the main driver
of the program, it handles storing prime numbers found and distributing work to the `SocketManager`. 

### Client

The `SocketManager` contains a `Listener` and `ClientHandler` class. `Listener` receives new connections and
`ClientHandler` manages a single existing connection. 

### Common

The Common library contains code that is used by both the Client and the Server. As of now, it contains the code
that serializes and deserializes messages and a config.h (it's included in the .gitignore) file that stores the
default port and IP address used. However, I plan to add either a GUI or a way to format console output well
for the task.

## Data Transmission

Since I'm using TCP sockets I serialize each message into a byte array. Each byte array has two parts:
* Message header
    * One byte that indicates message type
    * Two bytes that indicate size of the payload
* Payload, this either contains a list of prime numbers found or a range to be searched. A list of primes can
  be of any size, a range is going to just be two `unsigned long long`s.

## Future Improvements

This is a work in progress, so there are multiple things I would like to add. This includes:
* GUI (w/ win32 or QT) or output formatting (w/ windows.h) to it less of a mess to look at.
* A way for the client applications to access the primes found so far, enabling the use of a Seive of Eratosthenes. 
  Either,
    * Database, probably with noSQL or MongoDB.
    * Logic for Clients to request primes found so far through their socket w/ the Server.
    * Note: I must consider how much time it would take to communicate over network and how much memory would be
      used to store a big number of prime numbers. Then think of a balanced way to handle those two constraints.
* Improve logic for finding prime numbers, i.e. they are always 1+-6n for some integer n.
* Change algorithms to stop using mutexs because they're so wasteful.
* Think about creating and maintaing client threads before and after the client connects to reduce startup time. 
