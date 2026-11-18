
#include "data_acquisition.hpp"

// Bibliothèques nécessaires pour les sockets Unix
#include <sys/socket.h>
#include <sys/un.h>

// Bibliothèques pour read(), write(), close() et unlink()
#include <unistd.h>

// Bibliothèques standard
#include <iostream>
#include <cstring>
#include <cerrno>
#include <stdexcept>
#include <utility>

// Taille maximale d'un message : 1 MiB
#define MAX_MESSAGE_SIZE (1024 * 1024)

// Constructeur de la classe DataAcquisition
DataAcquisition::DataAcquisition(
    std::string socket_path,
    std::function<void(const std::string&)> callback
)
    // Initialisation des attributs de la classe
    : socket_path_(std::move(socket_path)),
      callback_(std::move(callback))
{
}

// Fonction principale : démarre le serveur local
void DataAcquisition::run()
{
    // 1. Créer un socket Unix de type SOCK_STREAM
    int server = socket(AF_UNIX, SOCK_STREAM, 0);

    // Vérifier si la création du socket a réussi
    if (server == -1) {
        throw std::runtime_error("Erreur lors de la creation du socket");
    }

    // 2. Préparer l'adresse du socket
    sockaddr_un addr{};

    addr.sun_family = AF_UNIX;

    // Vérifier que le chemin du socket n'est pas trop long
    if (socket_path_.size() >= sizeof(addr.sun_path)) {
        close(server);
        throw std::runtime_error("Chemin du socket trop long");
    }

    // Copier le chemin du socket dans la structure
    std::strncpy(
        addr.sun_path,
        socket_path_.c_str(),
        sizeof(addr.sun_path) - 1
    );

    // Supprimer un ancien fichier socket s'il existe
    unlink(socket_path_.c_str());

    // 3. Associer le socket à son adresse avec bind()
    if (bind(server, (sockaddr*)&addr, sizeof(addr)) == -1) {
        close(server);
        throw std::runtime_error("Erreur bind()");
    }

    // 4. Mettre le serveur en attente des connexions
    if (listen(server, 8) == -1) {
        close(server);
        unlink(socket_path_.c_str());
        throw std::runtime_error("Erreur listen()");
    }

    std::cout << "Data Acquisition : serveur demarre"
              << std::endl;

    // Boucle infinie : attendre les clients
    while (true) {

        // 5. Accepter une connexion cliente
        int client = accept(server, nullptr, nullptr);

        // Vérifier si la connexion a réussi
        if (client == -1) {
            if (errno == EINTR) {
                continue;
            }

            std::cerr << "Erreur accept()" << std::endl;
            continue;
        }

        // Buffer temporaire pour recevoir les données
        char buf[4096];

        // Stocker le message complet reçu
        std::string payload;

        bool valid = true;
        bool finished = false;

        // 6. Lire les données jusqu'au caractère '\n'
        while (!finished) {

            ssize_t n = read(client, buf, sizeof(buf));

            // Vérifier la lecture
            if (n < 0) {
                if (errno == EINTR) {
                    continue;
                }

                valid = false;
                break;
            }

            // Fin de la connexion avant réception du message complet
            if (n == 0) {
                valid = false;
                break;
            }

            // Parcourir les caractères reçus
            for (ssize_t i = 0; i < n; i++) {

                // Le caractère '\n' indique la fin du message
                if (buf[i] == '\n') {
                    finished = true;
                    break;
                }

                // Vérifier la taille maximale du message
                if (payload.size() >= MAX_MESSAGE_SIZE) {
                    valid = false;
                    finished = true;
                    break;
                }

                // Ajouter le caractère au message
                payload += buf[i];
            }
        }

        // 7. Vérifier que le message n'est pas vide
        if (payload.empty()) {
            valid = false;
        }

        // 8. Transmettre le message au Message Manager
        if (valid) {

            std::cout << "Message recu : "
                      << payload << std::endl;

            // Appeler la fonction callback
            callback_(payload);

            // 9. Répondre au client avec OK
            write(client, "OK\n", 3);

        } else {

            // Répondre avec une erreur si le message est invalide
            write(client, "ERROR\n", 6);
        }

        // 10. Fermer la connexion cliente
        close(client);
    }

    // Fermer le serveur (fin normale non atteinte dans cette boucle)
    close(server);

    // Supprimer le fichier socket
    unlink(socket_path_.c_str());
}
