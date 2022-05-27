# packetsInC

![udptcp](https://user-images.githubusercontent.com/94362354/168884092-4e8151e9-5413-4cc0-aa1b-f526f54781b5.png)

### Assignment:
You make 4 applications (UDP Client, UDP Server, TCP Client and TCP Server) that can communicate with each other over the internet via Berkley sockets in C. The 2 Server applications and the 2 Client applications may run on your laptop.

#### UDP Smartphone sensor stream analyzer (15%) [App]

##### Which parts have been achieved from the UDP Client?
 
 - [X] Working Client application that sends number of packets to send via UDP (proof Packet Sender) [2/15]
 
##### Which parts have been achieved from the UDP Server?
 
 - [X] Working Server application receiving data via UDP (Evidence Packet Sender) [2/15]
 
 - [X] CSV of received stream is created [1/15]
 
 - [X] Tracks the number of packets received [1/15]
 
 - [X] Set a number of packets to receive [1/15]
 
 - [X] The time difference between first and last packet is shown [1/15]
 
 - [X] *Timeout can be set [2/15]
 
 - [X] *When timed out the number actually received versus expected number is shown with packet loss calculation [2/15]
 
 - [X] *The data is parsed on the server and a min, max and avg is kept [2/15]
 
 - [X] *Statistical data is kept in a file [1/15]
 
#### TCP Chatbox with history via HTTP (25%)

##### Which parts have been achieved from the TCP Client?
 
 - [X] Client can make TCP connection (proof via WireShark) [1/25]
 
 - [X] Client can successfully send and receive TCP packets (proof via WireShark) [1/25]
 
 - [X] Client forwards the messages entered by the user (Proof Packet Sender) [1/25]
 
 - [X] *Messages can be sent and received simultaneously on the Client without an agreed rotational role (e.g. multi-threaded) [4/25]
 
 ##### Which parts have been achieved from the TCP Server?
 
 - [X] Server can listen to a port for incoming TCP connections [1/25]
 
- [X] Server accepts connection and talks to connected client over TCP [1/25]
 
- [X] Server application forwards received message to all other participants [1/25]
 
- [X] Server does not send to sender its own message [1/25]
 
- [X] Server sends the sender information along with the message (i.e. IP address + sender's port) OR Server sends the information of new client connection to all other connected clients (i.e. IP address + sender's port) [2/ 25]
 
- [X] Server sends the last 16 chat messages to the client on new connection [2/25]
 
- [x] Server can perform HTTP communication with the web server (proof via WireShark) [2/25]
 
 - [X] *Server forwards the chat messages over HTTP to the web server (proof via WireShark) [2/25]
 
- [x] *Server requests the last 16 chat messages via HTTP to the web server (at startup) [2/25]
 
 - [X] *Multiple Clients can be simultaneously connected and chatting with each other (i.e. processed asynchronously or synchronously on the server) [4/25]