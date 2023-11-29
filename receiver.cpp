#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

int main(int argc, char **argv) {
  if (argc != 5) {
    std::cerr << "Usage: ./receiver [server_address] [port] [username] [room]\n";
    return 1;
  }

  std::string server_hostname = argv[1];
  int server_port = std::stoi(argv[2]);
  std::string username = argv[3];
  std::string room_name = argv[4];

  Connection conn;
  Message response;

  // TODO: connect to server
  conn.connect(server_hostname, server_port);

  // TODO: send rlogin and join messages (expect a response from
  //       the server for each one)
  conn.send(Message(TAG_RLOGIN, username));
  conn.receive(response);
  if (response.tag == TAG_OK) {
    conn.send(Message(TAG_JOIN, room_name));//only send join message after getting ok
  }

  // TODO: loop waiting for messages from server
  //       (which should be tagged with TAG_DELIVERY)
  while (true) { //infinite loop, exit when ctrl+c
    conn.receive(response);
    //response contains delivery:[room]:[sender]:[message]
    //check that response contains delivery tag
    if (response.tag == TAG_DELIVERY) {
      std::string room;
      std::string sender;
      std::string message;
      std::string data = response.data;//[room]:[sender]:[message]
      size_t colonPos = data.find(':');
      room = data.substr(0, colonPos);
      data = data.substr(colonPos + 1);//[sender]:[message]
      colonPos = data.find(':');
      sender = data.substr(0, colonPos);
      data = data.substr(colonPos + 1);//[message]
      message = data;
      std::cout << sender << ": " << message << "\n";
    }
    //parse response.data to get room, sender, and message
    //check that room and sender in response match room and sender in args
    //cout [username of sender]: [message text]

  }

  return 0;
}
