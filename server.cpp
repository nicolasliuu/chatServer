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
    // Initialize user with username;
    User *user = new User(msg.data);
    server->chat_with_receiver(user, connectionInfo);
  } else if (msg.tag == TAG_SLOGIN) {
    connectionInfo->conn->send(Message(TAG_OK, "logged in as " + msg.data));
    // Initialize user with username;
    User *user = new User(msg.data);
    server->chat_with_sender(user, connectionInfo);
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

void Server::chat_with_receiver(User *user, struct ConnInfo *connectionInfo) {  
  //get username from message, pass it in when creating user object
  //parse message to get username and room
  std::string roomName;
  std::string username;
  Message msg;

  if (!connectionInfo->conn->receive(msg)){
    connectionInfo->conn->send(Message(TAG_ERR, "invalid command"));
    delete user;
    delete connectionInfo;
    return;
  }
  if (msg.tag != TAG_JOIN) {
    connectionInfo->conn->send(Message(TAG_ERR, "invalid command"));
  }
  if (msg.tag == TAG_JOIN) {
      // Register sender to room
      roomName = msg.data;
      // Find or create the room
      Room *r = find_or_create_room(roomName);
      // Add the member using our username
      r->add_member(user);
      connectionInfo->conn->send(Message(TAG_OK, "joined " + roomName));
  }

  // While loop to send messages to receiver, formatted delivery:[room]:[sender]:[message]
  while (1) {
    // Dequeue messages from User's message queue.
    Message *msg; 
    msg = user->mqueue.dequeue();
    if (msg != nullptr) {
      // Send message to receiver
      if (!connectionInfo->conn->send(*msg)) {
        // Destroy connection data
          delete connectionInfo->conn;
          delete connectionInfo; // Closes the conneciton
          delete user;
          delete msg;
          return;
      }
    }
  }
}

void Server::chat_with_sender(User *user, struct ConnInfo *connectionInfo) {
  std::string roomName;
  Room *room = nullptr;
  while (1) {
    Message msg; 
    if (!connectionInfo->conn->receive(msg)){ // Receive message from sender
      delete user;
      delete connectionInfo;
      break;
    }
    if (msg.tag == TAG_JOIN) {
      // Register sender to room
      roomName = msg.data;
      // Find or create the room
      room = find_or_create_room(roomName);
      // Add the member using our username
      room->add_member(user);
      if (!connectionInfo->conn->send(Message(TAG_OK, "joined " + roomName))) {
        delete user;
        delete connectionInfo;
        break;
      }
    } else if (msg.tag == TAG_LEAVE) {
      // De-register sender from room
      if (room == nullptr || m_rooms.find(roomName) == m_rooms.end()) {
        connectionInfo->conn->send(Message(TAG_ERR, "not in a room"));
        continue;
      }
      // Remove the member using username
      room->remove_member(user);
      room = nullptr;
      if (!connectionInfo->conn->send(Message(TAG_OK, "left " + roomName))) {
        delete user;
        delete connectionInfo;
        break;
      }
    } else if (msg.tag == TAG_QUIT) {
      // De-register sender from room and send 
      if (room != nullptr && m_rooms.find(roomName) != m_rooms.end()) {
        room->remove_member(user);
        room = nullptr;
      }
      // Send quit message
      connectionInfo->conn->send(Message(TAG_OK, "quit"));
        // Destroy connection data
        connectionInfo->conn->close();
        delete connectionInfo;
        // Destroy user data
        delete user;
        break;
    } else if (msg.tag == TAG_SENDALL) { //SENDALL
      // Synch and broadcast message to all members of the room
      std::string message = msg.data;
      // Join command is issued before any chat commands, so we have to broadcast the message to the room.
      if (room != nullptr) {
        room->broadcast_message(user->username, message);
        if (!connectionInfo->conn->send(Message(TAG_OK, "sent"))) {
          delete user;
          delete connectionInfo;
          break;
        }
      } else {
        connectionInfo->conn->send(Message(TAG_ERR, "Not in a room"));
      }
    } else {
      connectionInfo->conn->send(Message(TAG_ERR, "invalid command"));
      break;
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
// return a pointer to the unique Room object representing
  //       the named chat room, creating a new one if necessary
  //iterate thru all the rooms (m_rooms) to check if the room of the given name already exists
  Guard guard(m_lock); // Lock the mutex before checking or modifying m_rooms

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