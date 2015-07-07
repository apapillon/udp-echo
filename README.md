# udp-echo

Ce projet est une demo de mise en place d'un server Echo en UDP.

La difficulté réside dans le fait que le protocole UDP est un protocole non connecté et que l'envoi d'une trame de réponse nécessite de spécifier l'interface de retour.

## src/udpbase.c
Ce fichier met en place un server ECHO UDP simple, la réponse est envoyée via la fonction de base __sendto__.
Le fonctionnement de celui-ci ne pose pas de problème sur un ordinateur avec une seule et unique IP par réseau. L'envoi sera régenté par la table de routage de celui-ci.

## src/udpfinal.c
Ce fichier met en place un server ECHO UDP spécifiant l'interface d'envoi de la réponse via la fonction __sendmsg__ et l'option de socket __IP_PKTINFO__. Le fonctionnement de celui fonctionne sur des ordinateurs ayant de multiples IP sur le meme réseau. La réponse est envoyé par l'interface ayant reçu le message de base. La table de routage n'a pas d'effet sur son fonctionnement.

-----
Ce projet regroupe les fichiers d'exemple de l'article : http://www.perhonen.fr/blog/2015/04/envoi-de-paquet-udp-depuis-une-interface-precise-1607
