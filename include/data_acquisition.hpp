
#pragma once

// Bibliothèque pour utiliser std::function (les callbacks)
#include <functional>

// Bibliothèque pour manipuler les chaînes de caractères
#include <string>

// Classe responsable de la réception des données des capteurs
class DataAcquisition {

public:

    // Constructeur de la classe
    // socket_path : chemin du socket Unix
    // callback : fonction appelée lorsqu'un message est reçu
    DataAcquisition(
        std::string socket_path,
        std::function<void(const std::string&)> callback
    );

    // Démarre le serveur local et attend les messages des clients
    void run();

private:

    // Stocke le chemin du socket Unix (exemple : /tmp/dtn_edge.sock)
    std::string socket_path_;

    // Stocke la fonction callback qui transmet les données au Message Manager
    std::function<void(const std::string&)> callback_;
};