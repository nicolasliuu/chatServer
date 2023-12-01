#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

int main(int argc, char **argv) {
  if (argc != 4) {
    std::cerr << "Usage: ./sender [server_address] [port] [username]\n";
    return 1;
  }

  std::string server_hostname;
  int server_port;
  std::string username;

  server_hostname = argv[1];
  server_port = std::stoi(argv[2]);
  username = argv[3];

  Connection conn;

  // Connect to server
  conn.connect(server_hostname, server_port);
  // Handle connection error
  if (!conn.is_open()) {
    std::cerr << "Error: could not connect to server\n";
    return 1;
  }
  
  // Send slogin message
  conn.send(Message(TAG_SLOGIN, username));
  // Handle response 
  Message response;
  
  // IF response tag is not ok, throw error
  if (!conn.receive(response)) {
    std::cerr << "Error: could not receive response from server\n";
    return 1;
  }
  if (response.tag == TAG_ERR) {
    std::cerr << response.data << "\n";
    return 1;
  } 


  //loop reading commands from user, sending messages to
  //       server as appropriate

  std::string input;

  while (true) {
    std::cout << "> ";
    std::getline(std::cin, input);
    std::istringstream iss(input);

    if (input.empty()) {
      continue;
    }

    if (input[0] == '/') {
      // Handle command
      if (input == "/quit") {
        conn.send(Message(TAG_QUIT, ""));
        if (conn.receive(response) && response.tag == TAG_OK) {
          return 0;
        } 
       } else if (input.substr(0, 5) == "/join") {
        std::string room;
        room = input.substr(6);
        conn.send(Message(TAG_JOIN, room));
      } else if (input.substr(0, 6) == "/leave") {
        conn.send(Message(TAG_LEAVE, ""));
      } else {
        std::cerr << "Error: Invalid command\n";
      }
    } else {
      // Handle message
      conn.send(Message(TAG_SENDALL, input));
    }

    if (!conn.receive(response)) {
      std::cerr << "No connection to server.\n";
      return 1;
    } else if (response.tag == TAG_ERR) {
      std::cerr << response.data << "\n";
    } else if (response.tag == TAG_OK || response.tag == TAG_DELIVERY) {
      // Do nothing here
    } else {
      std::cerr << "Error: Invalid response tag\n";
      return 1;
    }
  }

  conn.close();
  return 0;
}
