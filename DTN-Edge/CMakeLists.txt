# Version minimale de CMake
cmake_minimum_required(VERSION 3.10)

# Nom du projet
project(DTN_Edge)

# Utilisation du langage C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Création de l'exécutable avec les fichiers nécessaires
add_executable(dtn_edge
    src/main.cpp
    src/data_acquisition.cpp
)

# Indiquer le dossier des fichiers header
target_include_directories(dtn_edge PRIVATE include)