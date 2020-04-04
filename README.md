# chat2me
An instant messaging application made in C by :

- BIASIBETTI Laura

- DUVAL Axel

- ISSARTEL Solène  

Project proposed by **Polytech Montpellier** in 2020. You can fin documentation in the code and instructions for running programs. We provide you with compiled programs in case you can't.
  

## Sprint 1
A server relays text messages between two clients.

- **Run**
```
 chat2me> ./server <chosen_port>
 chat2me> ./client <your_ip_address> <chosen_port>
```
- **Compile**
```
chat2me> gcc -o server server.c
chat2me> gcc -o client client.c
```

## Sprint 2

A multi-threaded server and two multi-threaded clients


- **Run**
```
 chat2me> ./server <chosen_port>
 chat2me> ./client <your_ip_address> <chosen_port>
```
- **Compile**
```
chat2me> gcc -o server server.c -lpthread
chat2me> gcc -o client client.c -lpthread
```

  

## Sprint 2 v2

A multi-threaded server and severals clients with usernames which can join or quit the conversation


- **Run**
```
 chat2me> ./server <chosen_port>
 chat2me> ./client <your_ip_address> <chosen_port>
```
- **Compile**
```
chat2me> gcc -o server server.c -lpthread
chat2me> gcc -o client client.c -lpthread
```
  

## Sprint 3

Files transfert

  

## Sprint 4

Management of discussion channels

  

## Sprint 5

Web interface for entering and displaying messages, with "instant" sending
