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

  // TODO: read login message (should be tagged either with
  //       TAG_SLOGIN or TAG_RLOGIN), send response

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
}

Server::~Server() {
  // TODO: destroy mutex
}

bool Server::listen() {
  // TODO: use open_listenfd to create the server socket, return true
  //       if successful, false if not
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
}