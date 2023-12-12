#include "guard.h"
#include "message.h"
#include "message_queue.h"
#include "user.h"
#include "room.h"

Room::Room(const std::string &room_name)
  : room_name(room_name) {
  // initialize the mutex
  pthread_mutex_init(&lock, NULL);
}

Room::~Room() {
  // destroy the mutex
  pthread_mutex_destroy(&lock);
}

void Room::add_member(User *user) {
  if (members.find(user) != members.end()) {
    return;
  }
  Guard guard(lock);
  members.insert(user);
}

void Room::remove_member(User *user) {
  if (members.find(user) == members.end()) {
    return;
  }
  Guard guard(lock);
  members.erase(user);
}

void Room::broadcast_message(const std::string &sender_username, const std::string &message_text) {
  // send a message to every (receiver) User in the room
  Guard guard(lock);

  for (User *user : members) {
    user->mqueue.enqueue(new Message(TAG_DELIVERY, room_name + ":" + sender_username + ":" + message_text));
  }
}
