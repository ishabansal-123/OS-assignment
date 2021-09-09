# OS-assignment
In the kernel space component, checking the state of free memory in the system. When the amount of free memory falls below the specified threshold, sending a signal SIGBALLOON to the application.
* A Mechanism that will deliver the SIGBALLOON signal to the registered application. The signal must be delivered when the amount of free memory falls below 1GB.
* Implemented the systems call that will swap the application specified pages.
