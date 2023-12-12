Since synchronization is an important part of this assignment, we’d like you to support a report on your synchronization 
in your README.txt. Please include where your critical sections are, how you determined them, and why you chose the 
synchronization primitives for each section. You should also explain how your critical sections ensure that the synchronization
 requirements are met without introducing synchronization hazards (e.g. race conditions and deadlocks).

Our critical sections are in:
- Room::add_member
- Room::remove_member
- Room::broadcast_message
- MessageQueue::enqueue
- MessageQueue::dequeue

