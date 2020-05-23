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
 chat2me/sprint1> ./server <chosen_port>
 chat2me/sprint1> ./client <your_ip_address> <chosen_port>
```
- **Compile**
```
chat2me/sprint1> gcc -o server server.c
chat2me/sprint1> gcc -o client client.c
```

## Sprint 2

A multi-threaded server and two multi-threaded clients


- **Run**
```
 chat2me/sprint2> ./server <chosen_port>
 chat2me/sprint2> ./client <your_ip_address> <chosen_port>
```
- **Compile**
```
chat2me/sprint2> gcc -o server server.c -lpthread
chat2me/sprint2> gcc -o client client.c -lpthread
```

  

## Sprint 2 v2

A multi-threaded server and severals clients with usernames which can join or quit the conversation


- **Run**
```
 chat2me/sprint2_v2> ./server <chosen_port>
 chat2me/sprint2_v2> ./client <your_ip_address> <chosen_port>
```
- **Compile**
```
chat2me/sprint2_v2> gcc -o server server.c -lpthread
chat2me/sprint2_v2> gcc -o client client.c -lpthread
```
  

## Sprint 3

Files transfert.
To create a real scenario, with min 2 clients, you can run simultaneously client in the OtherClient and in the sprint3 folders.


- **Run**
```
 chat2me/sprint3> ./server <chosen_port>
 chat2me/sprint3> ./client <your_ip_address> <chosen_port>
```
- **Compile**
```
chat2me/sprint3> gcc -o server server.c -lpthread
chat2me/sprint3> gcc -o client client.c -lpthread
```
  

## Sprint 4 v1

Connection to discussion channels


- **Run**
```
 chat2me/sprint4/sprint4_v1> ./server <chosen_port>
 chat2me/sprint4/sprint4_v1> ./client <your_ip_address> <chosen_port>
```
- **Compile**
```
chat2me/sprint4/sprint4_v1> gcc -o server server.c -lpthread
chat2me/sprint4/sprint4_v1> gcc -o client client.c -lpthread
```

## Sprint 4 v1bis

Connection to discussion channels with adding a new thread


- **Run**
```
 chat2me/sprint4/sprint4_v1bis> ./server <chosen_port>
 chat2me/sprint4/sprint4_v1bis> ./client <your_ip_address> <chosen_port>
```
- **Compile**
```
chat2me/sprint4/sprint4_v1bis> gcc -o server server.c -lpthread
chat2me/sprint4/sprint4_v1bis> gcc -o client client.c -lpthread
```

## Sprint 4 v2

Management of discussion channels


- **Run**
```
 chat2me/sprint4/sprint4_v2> ./server <chosen_port>
 chat2me/sprint4/sprint4_v2> ./client <your_ip_address> <chosen_port>
```
- **Compile**
```
chat2me/sprint4/sprint4_v2> gcc -o server server.c -lpthread
chat2me/sprint4/sprint4_v2> gcc -o client client.c -lpthread
```


## Sprint 5

Web interface for entering and displaying messages, with "instant" sending
