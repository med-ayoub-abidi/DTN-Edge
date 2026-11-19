
#include "data_acquisition.hpp"
#include <iostream>
#include <exception>

int main()
{
    try
    {
        DataAcquisition acquisition(
            "/tmp/dtn_edge.sock",

            [](const std::string& message)
            {
                std::cout << "Callback appele : "
                          << message << std::endl;
            }
        );

        std::cout << "Demarrage de Data Acquisition..."
                  << std::endl;

        acquisition.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Erreur : " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
