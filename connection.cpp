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
  rio_readinitb(&m_fdbuf, m_fd);
}

void Connection::connect(const std::string &hostname, int port) {
  m_fd = open_clientfd(hostname.c_str(), std::to_string(port).c_str());//hostname and port need to be c strings
  rio_readinitb(&m_fdbuf, m_fd);
}

Connection::~Connection() {
  if(is_open()) {//close the connection if it is still open
    close();
  }
}

bool Connection::is_open() const {
  return m_fd >= 0;
}

void Connection::close() {
  Close(m_fd);
}

bool Connection::send(const Message &msg) {
  // return true if successful, false if not
  // Sets m_last_result to the appropriate value
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

  // Create a tag with the buffer by finding the first colon and parsing it accordingly.
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