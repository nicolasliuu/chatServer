#include "guard.h"
#include "message.h"
#include "message_queue.h"
#include "user.h"
#include "room.h"

Room::Room(const std::string &room_name)
  : room_name(room_name) {
  // TODO: initialize the mutex
  pthread_mutex_init(&lock, NULL);
}

Room::~Room() {
  // TODO: destroy the mutex
  pthread_mutex_destroy(&lock);
}

void Room::add_member(User *user) {
  Guard guard(lock);
  members.insert(user);
}

void Room::remove_member(User *user) {
  Guard guard(lock);
  members.erase(user);
}

void Room::broadcast_message(const std::string &sender_username, const std::string &message_text) {
  // TODO: send a message to every (receiver) User in the room
  Guard guard(lock);
  Message *msg = new Message(TAG_DELIVERY, room_name + ":" + sender_username + ":" + message_text);
  for (User *user : members) {
    user->mqueue.enqueue(msg);
  }
}
