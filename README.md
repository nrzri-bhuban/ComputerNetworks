## COMPUTER NETWORKS : LAB MANUAL ASSIGNMENTS
### Q1. CN Lab – Assignment 1 
Objective:  
To gain first hands on experience of basic Socket Programming. 
Exercise:  
Write a program to run TCP client and server socket programs where client first 
says “Hi” and in response server says “Hello”.  
Steps/ Hints: (if any):  
1. Create two mininet hosts,  
2. Open the hosts individually in xterm windows, 
3. In one host, run tcpserver program and then in another run the tcpclient.  
Learning Outcomes:  
1. Basics of TCP client and server programming.

#### INDIVIDUAL MD
[readme file](Assignment1/Readme.md)
#### OUTPUT
![Output Image](Assignment1/As1.jpg)

### Q2. CN Lab – Assignment 2 
Objective:  
To gain experience of TCP Socket Programming for simple applications. 
Exercise:  
Write a program using TCP socket to implement the following:  
i. Server maintains records of fruits in the format: fruit-name, quantity Last-sold, (server timestamp),  
ii. Multiple client purchase the fruits one at a time,  
iii. The fruit quantity is updated each time any fruit is sold,  
iv. Send regret message to a client if therequested quantityof the fruit is not available.  
v. Display the customer ids <IP, port> who has done transactions already. This list should be updated in the server every time a transaction occurs.  
vi. The total number of unique customers who did some transaction will be displayed to the customer every time.  
Steps/ Hints: (if any)   
1. Use at least two mininet hosts as clients,  
2. Server must be kept running using a loop condition, 
3. Take another socket (from accept() call) for keeping client information, 
4. Server must send the current stock information to the transacting host as queried in the question.     
Learning Outcomes:  
1. Multiple client’s communication via server socket can be learned.

#### INDIVIDUAL MD
[readme file](Assignment2/Readme.md)
#### OUTPUT
![Output Image](Assignment2/As2.jpg)


