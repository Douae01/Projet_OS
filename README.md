# Projet_OS

Ce projet implémente plusieurs exemples d'application pour un système embarqué basé sur un RTOS (système d'exploitation en temps réel). Les exemples incluent la gestion des tâches, l'utilisation des sémaphores, la gestion des entrées/sorties et la communication via un port série.

## Table des matières

- **Main 1** : Gestion de plusieurs tâches
- **Main 2** : Tâches identiques
- **Main 3** : Utilisation des sémaphores
- **Main 4** : Mutex pour synchronisation
- **Main 5** : Gestion des LEDs
- **Main 6** : Synchronisation avec sémaphores et LEDs
- **Main 7** : Gestion de tâches multiples avec fin
- **Main 8** : Lecture depuis un périphérique (fichier)
- **Main 9** : Commandes pour contrôler des LEDs via port série
- **Main 10** : Commande par utilisateur avec bouton pour LEDs
- **Main 11** : Commandes série pour contrôler LEDs et tâches
- **Main 12** : Lecture de données d'accéléromètre et affichage

## Prérequis

- Un environnement de développement pour microcontrôleur supportant un RTOS.
- Périphériques GPIO pour les LEDs et les boutons.
- Accéléromètre et périphériques série (si nécessaire pour certains exemples).

## Détails des mains

### Main 1 : Gestion de plusieurs tâches
Ce premier main montre comment créer et gérer plusieurs tâches concurrentes, où chaque tâche affiche son ID. Les tâches sont exécutées en boucle infinie et sont programmées avec un simple délai.

### Main 2 : Tâches identiques
Trois tâches identiques sont créées, exécutant le même code avec une boucle infinie et un délai.

### Main 3 : Utilisation des sémaphores
Dans ce main, deux tâches utilisent un sémaphore pour garantir l'exclusion mutuelle lors de l'accès à une ressource partagée.

### Main 4 : Mutex pour synchronisation
Ce main montre l'utilisation d'un mutex pour synchroniser l'accès aux ressources partagées entre les tâches.

### Main 5 : Gestion des LEDs
Ce code montre comment contrôler les LEDs d'un système embarqué, en alternant leur état à intervalles réguliers.

### Main 6 : Synchronisation avec sémaphores et LEDs
Dans ce main, l'utilisation d'un mutex permet de synchroniser les tâches pendant qu'elles contrôlent les LEDs.

### Main 7 : Gestion de tâches multiples avec fin
Ce main illustre la création et la gestion de tâches multiples qui ont une condition d'arrêt.

### Main 8 : Lecture depuis un périphérique (fichier)
Ce main montre comment ouvrir un fichier et lire des données depuis un périphérique pour les afficher.

### Main 9 : Commandes pour contrôler des LEDs via port série
Ce code permet de contrôler l'état des LEDs via des commandes envoyées par le port série.

### Main 10 : Commande par utilisateur avec bouton pour LEDs
Ce main utilise un bouton pour déclencher l'allumage des LEDs via le port série.

### Main 11 : Commandes série pour contrôler LEDs et tâches
L'utilisateur peut envoyer des commandes par port série pour allumer/éteindre des LEDs et lancer/arrêter des tâches spécifiques.

### Main 12 : Lecture de données d'accéléromètre et affichage
Ce dernier main montre la lecture des données d'un accéléromètre et leur affichage sur un terminal série.

## Conclusion

Ce projet présente différents exemples d'utilisation de tâches, de sémaphores, et de gestion des entrées/sorties dans un environnement embarqué avec un RTOS. Chaque main illustre un aspect différent de la gestion des ressources et de la communication dans un système embarqué.

