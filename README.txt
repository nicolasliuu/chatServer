MS1:
    Jayden
        - implemented and debugged receiver.cpp and connection.cpp
    Nicolas
        - implemented and debugged sender.cpp and connection.cpp

MS2:
    Jayden
        - implemented and debugged server.cpp
        - wrote README
    Nicolas
        - implemented and debgged server.cpp, room.cpp, message_queue.cpp

Our critical sections:
- Room::add_member
    add_member is a critical section because we want to prevent the race condition where two threads try to add the same member
    to the member set (room) at the same time. We chose to use a Guard so that inserting a new member could be locked and
    unlocked accordingly.
- Room::remove_member
    remove_member is a critical section because we want to prevent the race condition where two threads try to remove the same member
    to the member set (room) at the same time. We chose to use a Guard so that removing a member could be locked and
    unlocked accordingly.
- Room::broadcast_message
    broadcast_message sends a message from one user to all the users in the room, and there is a chance that two users will try
    to send their respective messages at the same time. We chose to use a Guard so that each user's message queue is locked
    when a message is being sent, and unlocked when all message queues have received the message.
- MessageQueue::enqueue
    enqueue adds a message to the queue when a user wants to send a message. Like broadcast_message, there is a chance that 
    two users will try to send their respective messages at the same time, so this process needs to be synchronized to ensure
    that messages are added to the queue in a consistent order. We chose to use a Guard so that the queue could be locked
    when a message is to be enqueued and unlocked when the enqueueing process is finished. We also used the semaphore
    sem_post to notify any waiting threads that a message is available for dequeueing when the queueing process is done.
- MessageQueue::dequeue
    dequeue takes a message from the queue when a room needs to receive a message. Like broadcast_message, there is a chance that 
    multiple threads will try to dequeue a message at the same time, so this process needs to be synchronized to ensure
    that the threads will dequeue a message in a consistent order. We chose to use a Guard so that the queue could be locked
    when a message is to be dequeued and unlocked when the dequeueing process is finished. We also used the semaphore 
    sem_timedwait to make the threads wait a specified time before checking the queue for a new message to be available for
    dequeueing.
- Server::find_or_create_room
    find_or_create_room returns the respective room that a user wants to go to. Multiple users may want to join or create the 
    same room at the same time, so synchronizing this process makes sure that duplicate rooms are not created. We chose to use
    a Guard so that the map is locked when a user wants to join or create a room, and unlocked when the specified room is joined
    or created. 