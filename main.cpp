#include <iostream>
#include "utils.h"
#include <chrono>
#include <fstream>
#include "FuzzyCMeans.h"


using boost::numeric::ublas::matrix;
using boost::numeric::ublas::vector;

void newline()
{
    std::cout << std::endl;
}
int main()
{
    //generowanie danych
    std::vector<vector<double>> X(300, vector<double>(3));
    std::default_random_engine generator;//(
    //                    std::random_device{}());
    std::uniform_real_distribution<double> 
        distribution(-40.0,40.0);
    auto gen = std::bind ( distribution, generator);
    for(auto& x: X)
        std::generate(x.begin(), x.end(),
                [&gen](){return gen();});
    //koniec generowania/ poczatek FCM
    FuzzyCMeans fcm(X, 17, 2);
    fcm.loop();
    newline();
    //fcm.print_info();
    newline();
    //zapis do pliku + generacja plików i skryptu
    fcm.print_clustered_data("n2");
    fcm.print_clustered_data("a", false);
    //fcm.
    return 0;
}
