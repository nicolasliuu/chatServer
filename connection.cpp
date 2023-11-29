#include <sstream>
#include <cctype>
#include <cassert>
#include "csapp.h"
#include "message.h"
#include "connection.h"

Connection::Connection()
  : m_fd(-1)
  , m_last_result(SUCCESS) {
}

Connection::Connection(int fd)
  : m_fd(fd)
  , m_last_result(SUCCESS) {
  // TODO: call rio_readinitb to initialize the rio_t object
  rio_readinitb(&m_fdbuf, m_fd);
}

void Connection::connect(const std::string &hostname, int port) {
  // TODO: call open_clientfd to connect to the server
  // TODO: call rio_readinitb to initialize the rio_t object
  m_fd = open_clientfd(hostname.c_str(), std::to_string(port).c_str());//hostname and port need to be c strings
  rio_readinitb(&m_fdbuf, m_fd);
}

Connection::~Connection() {
  // TODO: close the socket if it is open
  if(is_open()) {
    close();
  }
}

bool Connection::is_open() const {
  // TODO: return true if the connection is open
  return m_fd >= 0;
}

void Connection::close() {
  // TODO: close the connection if it is open
  Close(m_fd); //same as lowercase close()
}

bool Connection::send(const Message &msg) {
  // TODO: send a message
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately
  if (rio_writen(m_fd, msg.getMessage().c_str(), msg.getMessage().length()) == -1) {
    m_last_result = EOF_OR_ERROR;
    return false;
  } else {
    m_last_result = SUCCESS;
    return true;
  }
}

bool Connection::receive(Message &msg) {

  char buffer[Message::MAX_LEN + 1] = {0};
  if (rio_readlineb(&m_fdbuf, buffer, Message::MAX_LEN) == -1) {
    m_last_result = EOF_OR_ERROR;
    return false;
  }

  // Create a tag with the buffer 
  std::string receivedMsg(buffer);
  size_t colonPos = receivedMsg.find(':');
  if (colonPos == std::string::npos || colonPos == receivedMsg.length() - 1) {
    m_last_result = INVALID_MSG;
    return false;
  }

  std::string tag = receivedMsg.substr(0, colonPos);
  std::string data = receivedMsg.substr(colonPos + 1);

  // Remove the trailing newline character
  if (!data.empty() && data.back() == '\n') {
    data.pop_back();
  }

  if (!data.empty() && data.back() == '\r') {
    data.pop_back();
  }

  msg.tag = tag;
  msg.data = data;
  m_last_result = SUCCESS;
  return true;
}