# Projet_OS﻿Documentation du Projet

Ce projet implémente plusieurs exemples d'application pour un système embarqué basé sur un RTOS (système d'exploitation en temps réel). Les exemples incluent la gestion des tâches, l'utilisation des sémaphores, la gestion des entrées/sorties et la communication via un port série.

Table des matières

Exemple 1 : Gestion de plusieurs tâches

Exemple 2 : Tâches identiques

Exemple 3 : Utilisation des sémaphores

Exemple 4 : Mutex pour synchronisation

Exemple 5 : Gestion des LEDs

Exemple 6 : Synchronisation avec sémaphores et LEDs

Exemple 7 : Gestion de tâches multiples avec fin

Exemple 8 : Lecture depuis un périphérique (fichier)

Exemple 9 : Commandes pour contrôler des LEDs via port série

Exemple 10 : Commande par utilisateur avec bouton pour LEDs

Exemple 11 : Commandes série pour contrôler LEDs et tâches

Exemple 12 : Lecture de données d'accéléromètre et affichage

Prérequis

Un environnement de développement pour microcontrôleur supportant un RTOS.

Périphériques GPIO pour les LEDs et les boutons.

Accéléromètre et périphériques série (si nécessaire pour certains exemples).

Exemple 1 : Gestion de plusieurs tâches

Ce premier exemple montre comment créer et gérer plusieurs tâches concurrentes, où chaque tâche affiche son ID. Les tâches sont exécutées en boucle infinie et sont programmées avec un simple délai.

void code1() { ... }

void code2() { ... }

void code3() { ... }

int main() {

task\_new(code1, 512); // tache 1

task\_new(code2, 512); // tache 2

task\_new(code3, 512); // tache 3

os\_start();

}

Exemple 2 : Tâches identiques

Trois tâches identiques sont créées, exécutant le même code avec une boucle infinie et un délai.

void code() { ... }

int main() {

task\_new(code, 512); // tache 1

task\_new(code, 512); // tache 2

task\_new(code, 512); // tache 3

os\_start();

}

Exemple 3 : Utilisation des sémaphores

Dans cet exemple, deux tâches utilisent un sémaphore pour garantir l'exclusion mutuelle lors de l'accès à une ressource partagée.

Semaphore\* sem = NULL;

void tache1() { ... }

void tache2() { ... }

int main() {

sem = sem\_new(0);

task\_new(tache1, 512);

task\_new(tache2, 256);

os\_start();

}

Exemple 4 : Mutex pour synchronisation

Cet exemple montre l'utilisation d'un mutex pour synchroniser l'accès aux ressources partagées entre les tâches.

Semaphore \* mutex = NULL;

void code() { ... }

int main() {

mutex = sem\_new(1);

task\_new(code, 512); // tache 1

task\_new(code, 512); // tache 2

task\_new(code, 512); // tache 3

os\_start();

}

Exemple 5 : Gestion des LEDs

Ce code montre comment contrôler les LEDs d'un système embarqué, en alternant leur état à intervalles réguliers.

static void leds\_init() { ... }

static void leds(uint32\_t val) { ... }

void tache1() { ... }

int main() {

leds\_init();

task\_new(tache1, 256);

task\_new(tache2, 128);

os\_start();

}

Exemple 6 : Synchronisation avec sémaphores et LEDs

Dans cet exemple, l'utilisation d'un mutex permet de synchroniser les tâches pendant qu'elles contrôlent les LEDs.

void tache1() { ... }

void tache2() { ... }

int main() {

leds\_init();

mutex = sem\_new(1);

task\_new(tache1, 512);

task\_new(tache2, 512);

task\_new(tache3, 128);

os\_start();

}

Exemple 7 : Gestion de tâches multiples avec fin

Ici, plusieurs tâches sont exécutées, mais l'une d'elles termine après un nombre défini d'itérations, affichant la fin de la tâche.

void code1() { ... }

void code2() { ... }

void code3() { ... }

void code4() { ... }

int main() {

task\_new(code1, 512); // task 1

task\_new(code2, 512); // task 2

task\_new(code3, 512); // task 3

task\_new(code4, 512); // task 4

os\_start();

}

Exemple 8 : Lecture depuis un périphérique (fichier)

Cet exemple montre comment ouvrir un fichier et lire des données depuis un périphérique pour les afficher.

char buffer[32];

int fd;

void tache1() { ... }

int main() {

task\_new(tache1, 512);

task\_new(idle, 0);

os\_start();

}

Exemple 9 : Commandes pour contrôler des LEDs via port série

Ce code permet de contrôler l'état des LEDs via des commandes envoyées par le port série.

void tache1() { ... }

int main() {

task\_new(tache1, 256);

task\_new(idle, 0);

os\_start();

}

Exemple 10 : Commande par utilisateur avec bouton pour LEDs

Cet exemple utilise un bouton pour déclencher l'allumage des LEDs via le port série.

void tache1() { ... }

int main() {

task\_new(tache1, 256);

task\_new(idle, 0);

os\_start();

}

Exemple 11 : Commandes série pour contrôler LEDs et tâches

L'utilisateur peut envoyer des commandes par port série pour allumer/éteindre des LEDs et lancer/arrêter des tâches spécifiques.

volatile int taskA\_running = 1;

void tache1() { ... }

void serialTask() { ... }

int main() {

task\_new(serialTask, 1024);

task\_new(idle, 0);

os\_start();

}

Exemple 12 : Lecture de données d'accéléromètre et affichage

Ce dernier exemple montre la lecture des données d'un accéléromètre et leur affichage sur un terminal série.

void accelTask() { ... }

int main() {

task\_new(accelTask, 1024);

task\_new(idle, 0);

os\_start();

}

Conclusion

Ce projet présente différents exemples d'utilisation de tâches, de sémaphores, et de gestion des entrées/sorties dans un environnement embarqué avec un RTOS. Chaque exemple illustre un aspect différent de la gestion des ressources et de la communication dans un système embarqué.
