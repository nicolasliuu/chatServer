#include <cassert>
#include <ctime>
#include "guard.h"
#include "message_queue.h"

MessageQueue::MessageQueue() {
  // initialze the mutex and the semaphore
  pthread_mutex_init(&m_lock, NULL);
  sem_init(&m_avail, 0, 0); // 0 for sharing between threads of a process
}

MessageQueue::~MessageQueue() {
  // Destroy the mutex and the semaphore
  pthread_mutex_destroy(&m_lock);
  sem_destroy(&m_avail);
  // Delete messages:
  Message *nextMessage = nullptr;
  while ((nextMessage = dequeue()) != nullptr) {
    nextMessage = NULL;
  }
}

void MessageQueue::enqueue(Message *msg) {
  // TODO: put the specified message on the queue
  Guard guard(m_lock);
  m_messages.push_back(msg);
  // be sure to notify any thread waiting for a message to be
  // available by calling sem_post
  sem_post(&m_avail); // Notify any thread waiting for a message to be available
}

Message *MessageQueue::dequeue() {
  struct timespec ts;

  // get the current time using clock_gettime:
  // we don't check the return value because the only reason
  // this call would fail is if we specify a clock that doesn't
  // exist
  clock_gettime(CLOCK_REALTIME, &ts);

  // compute a time one second in the future
  ts.tv_sec += 1;

  // Call sem_timedwait to wait up to 1 second for a message
  //       to be available, return nullptr if no message is available
  if (sem_timedwait(&m_avail, &ts) == 0) {
    Guard g(m_lock);

    // Remove next message from the queue and return it
    Message *msg = m_messages.front();
    m_messages.pop_front();
    return msg;
  } else {
    return nullptr;
  }
}
