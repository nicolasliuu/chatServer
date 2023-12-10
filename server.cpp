#include <pthread.h>
#include <iostream>
#include <sstream>
#include <memory>
#include <set>
#include <vector>
#include <cctype>
#include <cassert>
#include "message.h"
#include "connection.h"
#include "user.h"
#include "room.h"
#include "guard.h"
#include "server.h"

////////////////////////////////////////////////////////////////////////
// Server implementation data types
////////////////////////////////////////////////////////////////////////

// TODO: add any additional data types that might be helpful
//       for implementing the Server member functions

////////////////////////////////////////////////////////////////////////
// Client thread functions
////////////////////////////////////////////////////////////////////////

namespace {

void *worker(void *arg) {
  pthread_detach(pthread_self());
  //create new user object
  //call Room::add_member
  //call chat with sender/receiver somewhere in this function

  // TODO: use a static cast to convert arg from a void* to
  //       whatever pointer type describes the object(s) needed
  //       to communicate with a client (sender or receiver)
  //convert arg to connectioninfo
  ConnInfo *connectionInfo = static_cast<ConnInfo*> (arg);
  Server *server = connectionInfo->server;

  // TODO: read login message (should be tagged either with
  //       TAG_SLOGIN or TAG_RLOGIN), send response
  //receive message
  Message msg;
  connectionInfo->conn->receive(msg);
  std::string room;
  std::string username;
  std::string message;
  if (msg.tag == TAG_RLOGIN) {
    connectionInfo->conn->send(Message(TAG_OK, "logged in as " + msg.data));
    server->chat_with_receiver(connectionInfo);
  } else if (msg.tag == TAG_SLOGIN) {
    connectionInfo->conn->send(Message(TAG_OK, "logged in as " + msg.data));
    connectionInfo->username = msg.data;
    server->chat_with_sender(connectionInfo);
  }

  // TODO: depending on whether the client logged in as a sender or
  //       receiver, communicate with the client (implementing
  //       separate helper functions for each of these possibilities
  //       is a good idea)

  return nullptr;
}

}

////////////////////////////////////////////////////////////////////////
// Server member function implementation
////////////////////////////////////////////////////////////////////////

Server::Server(int port)
  : m_port(port)
  , m_ssock(-1) {
  // TODO: initialize mutex
  pthread_mutex_init(&m_lock, NULL);//pass m_lock
}

Server::~Server() {
  // TODO: destroy mutex
  pthread_mutex_destroy(&m_lock);//put an object in the parameter
}

bool Server::listen() {
  // TODO: use open_listenfd to create the server socket, return true
  //       if successful, false if not
  const char* port_str = std::to_string(m_port).c_str();
  int fd = open_listenfd(port_str);
  m_ssock = fd;
  if (fd >= 0) {
    return true;
  }
  return false;
}

void Server::chat_with_receiver(struct ConnInfo *connectionInfo) {  
  //receive message
  Message msg;
  connectionInfo->conn->receive(msg);
  //get username from message, pass it in when creating user object
  //parse message to get username and room
  //should receive message in form of delivery:[room]:[sender]:[message]
  std::string room;
  std::string username;
  std::string message;
  if (msg.tag == TAG_DELIVERY) {
  //parse msg.data to get room, sender, and message
    std::string data = msg.data;//[room]:[sender]:[message]
    size_t colonPos = data.find(':');
    room = data.substr(0, colonPos);
    data = data.substr(colonPos + 1);//[sender]:[message]
    colonPos = data.find(':');
    username = data.substr(0, colonPos);
    data = data.substr(colonPos + 1);//[message]
    message = data;
    // //send output message
    // User *user = new User(username);
    // //create room
    // Room *r = find_or_create_room(room);
    // //call Room::add_member
    // r->add_member(user);
    // r->broadcast_message(username, message); //output message
    connectionInfo->conn->send(Message(TAG_SENDALL, message)); //probably not the right tag
  } else if (msg.tag == TAG_JOIN) {
    User *user = new User(connectionInfo->username);
    // Register sender to room
    room = msg.data;
    // Find or create the room
    Room *r = find_or_create_room(room);
    // Add the member using our username
    r->add_member(user);
    connectionInfo->conn->send(Message(TAG_OK, "joined " + room));
  }
}

void Server::chat_with_sender(struct ConnInfo *connectionInfo) {
  while (1) {
    Message msg; 
    std::string room;
    User *user = new User(connectionInfo->username);
    connectionInfo->conn->receive(msg);
    if (msg.tag == TAG_JOIN) {
      // Register sender to room
      room = msg.data;
      // Find or create the room
      Room *r = find_or_create_room(room);
      // Add the member using our username
      r->add_member(user);
      connectionInfo->conn->send(Message(TAG_OK, "joined " + room));
    } else if (msg.tag == TAG_LEAVE) {
      // De-register sender from room
      room = msg.data;
      // Find the room
      Room *r = find_or_create_room(room);
      // Remove the member using username
      r->remove_member(user);
      connectionInfo->conn->send(Message(TAG_OK, "left " + room));
    } else if (msg.tag == TAG_QUIT) {
        // Destroy connection data
        delete connectionInfo;
        // Destroy user data
        delete user;
        // Close connection
        connectionInfo->conn->close();
        // Exit thread
        pthread_exit(NULL);
    } else { //SENDALL
      // Synch and broadcast message to all members of the room


    }
  }
}

void Server::handle_client_requests() {
  while (1)  {
    int clientfd = Accept(m_ssock, NULL, NULL);
    if (clientfd <= 0) {
      std::cerr << "Error accepting connection" << std::endl;
      continue;
    }
    // Create connection from clientfd
    Connection *conn = new Connection(clientfd);

    //pthread create, pass in conninfo struct
    //create a pthread_t pointer
    //pthread_Attr should be null
    //void* (*)(void*) pass in worker()
    //void* pass in conninfo struct


    /* Create ConnInfo object */
    struct ConnInfo *connectionInfo = new struct ConnInfo;
    connectionInfo->conn = conn;
    connectionInfo->server = this;
    //need to also set username
    /* start new thread to handle client connection */
    pthread_t thread;
    if (pthread_create(&thread, NULL, worker, connectionInfo) != 0) {
      std::cerr << "Error creating thread" << std::endl;
      delete connectionInfo;
      continue;
    };

  }
}

Room *Server::find_or_create_room(const std::string &room_name) {
  // TODO: return a pointer to the unique Room object representing
  //       the named chat room, creating a new one if necessary
  //iterate thru all the rooms (m_rooms) to check if the room of the given name already exists
  //if name is unique, create a new room and return a pointer to it
  //if room already exists, return a pointer to the existing room
  std::map<std::string, Room*>::iterator it;
  for(it = m_rooms.begin(); it != m_rooms.end(); it++) {
    std::string currentRoomName = it->second->get_room_name();
    if (currentRoomName.compare(room_name) == 0) {
      return it->second;//the room already exists
    }    
  }
  //the room doesn't exist, create a new room
  m_rooms[room_name] = new Room(room_name);
  return m_rooms[room_name];
}