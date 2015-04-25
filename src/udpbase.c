#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE           2048
#define PORT             50000

// Can be "*" or "192.168.56.10"
#define LISTEN "192.168.56.10"

int
main(int argc, char **argv) {

  int sockfd;
  struct sockaddr_in srvaddr;
  struct sockaddr_in cliaddr;
  socklen_t          clilen;
  
  char buf[BUFSIZE];
  ssize_t nbytes;

  /* 
   * Création du socket
   */
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  /* 
   * Affectation du nom au socket
   * on définit l'adresse et le port d'écoute
   */
  srvaddr.sin_family = AF_INET;
  srvaddr.sin_port = htons(PORT);
  if ( strncmp(LISTEN, "*", sizeof("*")) == 0) {
    srvaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  }
  else {
    srvaddr.sin_addr.s_addr = inet_addr(LISTEN);
  }
  if (bind(sockfd, (struct sockaddr *)&srvaddr, sizeof srvaddr) < 0) {
    perror("bind");
    exit(EXIT_FAILURE);
  }

  /*
   * Attente de la réception d'un message
   */
  bzero(buf, BUFSIZE);
  bzero((char *) &cliaddr, sizeof cliaddr);
  clilen = sizeof cliaddr;
  
  nbytes = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr *) &cliaddr, &clilen);
  
  printf("%ld bytes received\n", (long)nbytes);
  printf("From:\t %s:%hu\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
  printf("Data:\t %s\n", buf);

  /*
   * Envoi de la réponse au client
   */
  nbytes = sendto(sockfd, buf, nbytes, 0, (struct sockaddr*) &cliaddr, clilen);
  if (nbytes < 0) {
    perror("sendto");
    exit(EXIT_FAILURE);
  }

  printf("%d bytes sent\n", nbytes);

  /*
   * Fermeture du socket
   */
  if (close(sockfd) < 0) {
    perror("close");
  }
  
  return 0;
}
