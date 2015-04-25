/* 
 * udpsend.c - Send msg with from interface control 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/uio.h>

#define BUFSIZE 1024
#define PORT 50000

union control_data {
    struct cmsghdr  cmsg;
    u_char          data[CMSG_SPACE(sizeof(struct in_pktinfo))];
};

int
main(int argc, char **argv) {

  int sockfd;
  int sockopt;
  struct sockaddr_in srvaddr;
  struct sockaddr_in cliaddr;
  struct sockaddr_in rcvaddr;
  
  char buf[BUFSIZE];
  struct iovec iovrcv[1];
  struct msghdr msgrcv;
  union control_data cmsg;
  struct cmsghdr *cmsgptr;
  struct in_pktinfo *pktinfo;

  unsigned int src_ifindex;
  
  struct msghdr msg;
  struct iovec iov[1];
  char cmsgbuf[CMSG_SPACE(sizeof(struct in_pktinfo))];

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
   * Demande la récupération des informations PKTINFO
   */
  sockopt = 1;
  if (setsockopt(sockfd, IPPROTO_IP, IP_PKTINFO, &sockopt, sizeof sockopt) < 0) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  /* 
   * Affectation du nom au socket
   * on définit l'adresse et le port d'écoute
   */
  bzero((char *) &srvaddr, sizeof srvaddr);
  srvaddr.sin_family = AF_INET;
  srvaddr.sin_port = htons(PORT);
  srvaddr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(sockfd, (struct sockaddr *)&srvaddr, sizeof srvaddr) < 0) {
    perror("bind");
    exit(EXIT_FAILURE);
  }
  
  /*
   * Initialisation de la structure msghdr demandé par recvmsg
   *   iov     contient l'emplacement du buffer et sa taille
   *   cliaddr contient les informations sur l'émetteur
   *   cmsg    contient les informations de control en fonction des options
   *           demandées
   */
  bzero(buf, BUFSIZE);
  bzero((char *) &cliaddr, sizeof cliaddr);
  bzero((char *) &msgrcv, sizeof msgrcv);
  
  iovrcv[0].iov_base = buf;
  iovrcv[0].iov_len = sizeof buf;
  
  msgrcv.msg_name = &cliaddr;
  msgrcv.msg_namelen = sizeof cliaddr;
  msgrcv.msg_iov = iovrcv;
  msgrcv.msg_iovlen = 1;
  msgrcv.msg_control = &cmsg;
  msgrcv.msg_controllen = sizeof cmsg;
  
  /*
   * Reception message
   */
  nbytes = recvmsg(sockfd, &msgrcv, 0);
  if (nbytes < 0) {
    perror("recvmsg");
    exit(EXIT_FAILURE);
  }

  /*
   * Recuperation adresse de reception
   */
  bzero((char *) &rcvaddr, sizeof rcvaddr);
  rcvaddr.sin_family = AF_INET;
  rcvaddr.sin_port = htons(PORT);
  
  for (cmsgptr = CMSG_FIRSTHDR(&msgrcv);
    cmsgptr != NULL;
    cmsgptr = CMSG_NXTHDR(&msgrcv, cmsgptr)) {
    if (cmsgptr->cmsg_level == IPPROTO_IP && cmsgptr->cmsg_type == IP_PKTINFO) {
      // CMSG_DATA returne unsigned char* (voir cmsg(3))
      // doit être caster en fonction de cmsgptr->cmsg_type
      // IP_PKTINFO contient un pktinfo (voir ip(7))
      pktinfo = (struct in_pktinfo *)CMSG_DATA(cmsgptr);
      rcvaddr.sin_addr.s_addr = pktinfo->ipi_addr.s_addr;

      // Récupère l'index de l'interface pour réutiliser lors de l'envoi
      src_ifindex = pktinfo->ipi_ifindex;
    }
  }

  /*
   * Affichage simple du message reçu
   */
  printf("From:\t %s:%hu\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
  printf("To:\t %s:%hu\n", inet_ntoa(rcvaddr.sin_addr), ntohs(rcvaddr.sin_port));
  printf("Data:\t %s\n", buf);
  printf("%ld bytes received\n", (long)nbytes);
  
  /****************************************************************************
   * Envoi
   ****************************************************************************/

  /*
   * Définition de la structure msghdr demandé par sendmsg
   *   iov      contient l'emplacement du message à envoyer et sa taille
   *   cliaddr  définit la destination du message
   *   cmsgbuf  contient les informations d'envoi (interface d'envoi + adresse
   *            IP d'envoi)
   *            ATTENTION à bien réserver la mémoire avant utilisation.
   */
  bzero((char *) &msg, sizeof msg);

  iov[0].iov_base = buf;
  iov[0].iov_len = strlen(buf);

  msg.msg_name = &cliaddr;
  msg.msg_namelen = sizeof cliaddr;
  msg.msg_iov = iov;
  msg.msg_iovlen = 1;
  msg.msg_control = cmsgbuf;
  msg.msg_controllen = sizeof cmsgbuf;
  
  cmsgptr = CMSG_FIRSTHDR(&msg);
  cmsgptr->cmsg_level = IPPROTO_IP;
  cmsgptr->cmsg_type = IP_PKTINFO;
  cmsgptr->cmsg_len = CMSG_LEN(sizeof(struct in_pktinfo));
  pktinfo = (struct in_pktinfo *)CMSG_DATA(cmsgptr);
  pktinfo->ipi_ifindex = src_ifindex;
  pktinfo->ipi_spec_dst = rcvaddr.sin_addr;

  /*
   * Envoi du message
   */
  nbytes = sendmsg(sockfd, &msg, 0);
  if (nbytes < 0) {
    perror("sendmsg");
    exit(EXIT_FAILURE);
  }
  
  /*
   * Affichage du nombre d'octets envoyé
   */
  printf("%d bytes sent\n", nbytes);

  /*
   * Fermeture du socket
   */
  if (close(sockfd) < 0) {
    perror("close");
  }
  
  return 0;
}
