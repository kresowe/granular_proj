#ifndef FUZZYCMEANS_H
#define FUZZYCMEANS_H
#include <exception>
#include <iostream>
#include "utils.h"
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <iterator>
#include <random>
#include <chrono>
#include <cmath>
#include <fstream>
#include <string>
#include <numeric>
#include <functional>
#include <vector>
#include "invert-matrix.h"

using boost::numeric::ublas::matrix;
using boost::numeric::ublas::vector;
namespace ublas = boost::numeric::ublas; //Michal

extern void newline();

class FuzzyCMeans
{
    //typedef 
    public:
    FuzzyCMeans(std::vector<vector<double>>& X, unsigned clusters, unsigned fuzziness):
        fuzziness_(fuzziness), num_clusters_(clusters), 
        num_dimensions_(X[0].size()), N_(X.size()),
        clusters_(num_clusters_, vector<double>(num_dimensions_)), 
        X_(X), U_(N_, vector<double>(num_clusters_)),
        G_(N_, num_clusters_ * (num_dimensions_ + 1)), //Michal
        a_opt_(num_clusters_ * (num_dimensions_ + 1)), //Michal
        y_(N_), y_estimated(N_) //Michal
    {
        init_U();
        update_centers();
        print_info();
    }

    /*
     * zapisuje do pliku {fname | "cluster"}{numer klastra}.dat
     * i dla danych 2-wymiarowych tworzy skrypt do gnuplota
     */
    void print_clustered_data(std::string fname, bool enable_center = true)
    {
        std::vector<std::vector<vector<double>>> clust(num_clusters_);
        std::stringstream gnuplot_s;
        for(auto x = U_.begin(); x != U_.end(); ++x)
        {
            auto max = std::max_element(x->begin(), x->end());
            clust[std::distance(x->begin(), max)].push_back(X_[std::distance(U_.begin(), x)]);
        }
        if(num_dimensions_ <= 3)
        {
            gnuplot_s << "set terminal jpeg" << std::endl;
            gnuplot_s << "set out \'" << fname << ".jpeg\'" << std::endl;
            gnuplot_s << ((num_dimensions_==3)?"splot ":"plot ");
        }
        for(auto x = clust.begin(); x != clust.end(); ++x)
        {
            std::stringstream ss;
            ss << (fname == "" ? "cluster" : fname) << std::distance(clust.begin(), x) << ".dat";
            gnuplot_s << "\"" << ss.str() << "\" " <<(enable_center? " w lp" : "")<< " notitle" << ((std::distance(clust.begin(),x)<(num_clusters_-1) )? ',': ' ') ;
            std::ofstream f(ss.str());
            //komentarz ze srodkiem pakietu
            f << "# cluster center: ";
            for (auto &y: clusters_[std::distance(clust.begin(), x)])
            {
                f << y << ' ';
            }
            f << std::endl;
            for(auto &y: *x)
            {
                if(num_dimensions_ <= 3 && enable_center)
                {
                    for (auto &y: clusters_[std::distance(clust.begin(), x)]) {
                        f << y << ' ';
                    }
                    f << std::endl;
                }
                for(auto d: y)
                    f << d << ' ';
                f << std::endl;
            }
        }
        if(num_dimensions_ <=3)
        {
            std::stringstream splot;
            splot << "plot_" << fname << ".gpl";
            std::ofstream ff(splot.str());
            ff << gnuplot_s.str();
        }
    }
    //wyswietla w zasadzie wszystko
    void print_info()
    {
        std::cout << "************************************" << std::endl;
        std::cout << "Fuzziness: " << fuzziness_ << std::endl;
        std::cout << "Number of clusters: " << num_clusters_ << std::endl;
        std::cout << "Number of dimensions: " << num_dimensions_ << std::endl;
        std::cout << "Number of points: " << N_ << std::endl;
        newline();
        print_U();
        newline();
        print_X();
        newline();
        print_clusters();
        newline();
        std::cout << "************************************" << std::endl;
    }
    //generowanie macierzy U
    void randomize_U()
    {
        std::default_random_engine generator;//(
               //std::random_device{}());
        std::uniform_real_distribution<double> 
            distribution(0.0,1.0);
        auto gen = std::bind ( distribution, generator);
        for(auto& prototype: U_)
            std::generate(prototype.begin(), prototype.end(),
                    [&gen](){return gen();});
    }
    // normalizacja macierzy U
    void normalize_U()
    {
        for(auto& prototype: U_)
            utils::normalize(prototype.begin(), prototype.end());
    }

    //funkcja do generycznego wyswietalania
    void print(std::vector<vector<double>> & tmp)
    {
        for(auto i = tmp.begin(); i != tmp.end(); ++i)
        {
            for(auto j = (*i).begin(); j != (*i).end(); ++j)
            {
                std::cout << *j << ' '; 
            }
            std::cout << std::endl;
        }

    }
    //wyswietla wektory centrów klastrów
    void print_clusters()
    {
        std::cout << "Clusters: "<< std::endl;
        print(clusters_);
    }
    //wysiwetla macierz U
    void print_U()
    {
        std::cout << "Membership: "<< std::endl;
        print(U_);
    }
    //wysiwetla macierz  danych
    void print_X()
    {
        std::cout << "Data: "<< std::endl;
        print(X_);
    }

    //nowa wartosc generowana dla indeksów i, j
    // dla x_i, 
    double new_U_ij(unsigned i, unsigned j)
    {
        int k;
        double t, p, sum;
        sum = 0.0;
        p = 2 / (fuzziness_ - 1);
        for (k = 0; k < num_clusters_; k++)
        {
            t = vec_norm(i, j) / vec_norm(i, k);
            t = pow(t, p);
            sum += t;
        }return 1.0 / sum;

    }
    //generacja + normalizacja U
    void init_U()
    {
       randomize_U();
       normalize_U();
    }
    //aktualizacjia U
    double update_U()
    {
        double max = 0.0;
        for(unsigned j = 0; j < num_clusters_; ++j)
            for(unsigned i = 0; i < N_; ++i)
            {
                double n = new_U_ij(i, j);
                double diff = n - U_[i][j];
                diff = fabs(diff);
                U_[i][j] = n;
                if(max < diff)
                    max = diff;
            }
        return max;
    }

    //aktualizacjia  centrów klastrów
    void update_centers()
    {
        std::vector<vector<double>> tmp(N_, vector<double>(num_clusters_));
        double numerator, denominator;
        for (unsigned i = 0; i < N_; i++) {
            for (unsigned j = 0; j < num_clusters_; j++) {
                tmp[i][j] = pow(U_[i][j], fuzziness_);
            }
        }
        for (unsigned j = 0; j < num_clusters_; j++) {
            for (unsigned k = 0; k < num_dimensions_; k++) {
                numerator = 0.0;
                denominator = 0.0;
                for (unsigned i = 0; i < N_; i++) {
                    numerator += tmp[i][j] * X_[i][k];
                    denominator += tmp[i][j];
                }
                clusters_[j][k] = numerator / denominator;
            }
        }
    }

    //powtarzanie aktualizacji az do osiagniecia zbieznosci
    void loop()
    {
        double m = 1.0;
        do {
            update_centers();
            m = update_U();
        }while(m > 1e-6);
       /*
        for(unsigned i = 0; i < 27; ++i)
        {
            update_centers();
            print_info();
            double aaa = update_U();
        }
        */
    }

    //Michal
   /* double get_x(unsigned k, unsigned l) {
        return X_[k][l];
    }

    double get_u(unsigned i, unsigned k) {
        return U_[k][i];
    }*/

    void init_y(std::vector<double> v) {
        std::cout << "test\n";
        for (unsigned i = 0; i < N_; i++)
            y_[i] = v[i];

    }

    void calculate_G() {
        for (unsigned i = 0; i < N_; i++)
        {
            for (unsigned j = 0; j < num_clusters_; j++) 
            {
                G_(i, j * (num_dimensions_ + 1)) = U_[i][j] * 1.; 
                for (unsigned k = 0; k < num_dimensions_; k++)
                {
                    G_(i, j * (num_dimensions_ + 1) -1 + (k + 2)) = U_[i][j] * X_[i][k];
                }
            }
        }
    }

    void calculate_a_opt() {
        //a_opt = (G^T * G)^-1 * G^T * y
        //prepare calculations
        matrix<double> G_transp = ublas::trans(G_); //transpose G matrix
        matrix<double> A = ublas::prod(G_transp, G_); //G^T * G
        matrix<double> A_inv(N_, num_clusters_ * (num_dimensions_ + 1));
        InvertMatrix<double>(A, A_inv);

        //stl to ublas
        vector<double> a_opt_v(num_clusters_ * (num_dimensions_ + 1));
        vector<double> y_v(N_);
        for (unsigned i = 0; i < y_v.size(); i++)
            y_v(i) = y_[i];

        //actually calculate
        a_opt_v = ublas::prod(ublas::prod(A, G_transp), y_v);

        //ublas to stl
        for (unsigned i = 0; i < y_v.size(); i++)
            y_[i] = y_v(i);
    }

    void calculate_y_estimated() {
        //stl to ublas
        vector<double> a_opt_v(num_clusters_ * (num_dimensions_ + 1));
        vector<double> y_estimated_v(N_);
        for (unsigned i = 0; i < a_opt_v.size(); i++)
            a_opt_v(i) = a_opt_[i];

        //calculate
        y_estimated_v = prod(G_, a_opt_v);

        //ublas to stl
        for (unsigned i = 0; i < y_estimated_v.size(); i++)
            y_estimated[i] = y_estimated_v(i);
    }

    //znormalizowany wsplczynnik jakosci dla TS
    double quality_ts(){
        double result = 0.;
        for (unsigned i = 0; i < N_; i++)
            result += (y_[i] - y_estimated[i]) * (y_[i] - y_estimated[i]);
        return std::sqrt(result / N_);
    }

    std::vector<double> get_a_opt() {
        return a_opt_;
    }

    void set_a_opt(std::vector<double> vec) {
        a_opt_.clear();
        for (unsigned i = 0; i < vec.size(); i++)
        {
            a_opt_.push_back(vec[i]);
        }
    }



    //koniec Michal

    private:
    //pomocnicza funkcja do wyznaczania dlugosci roznicy odpowiednich wektorów(rzedów z macierzy) z X(danych) i centrów klastrów
    double vec_norm(unsigned i, unsigned j)
    {
        double sum = 0.0;
        for(unsigned k = 0; k < num_dimensions_; ++k)
        {
            sum += pow(X_[i][k] - clusters_[j][k], 2);
        }
        return sqrt(sum);
    }

    const unsigned fuzziness_;
    const unsigned num_clusters_;
    const unsigned num_dimensions_;
    const unsigned N_;
    //matrix<double> clusters_;
    std::vector<vector<double>> clusters_;
    std::vector<vector<double>>& X_; //pierwszy indeks - numer danych (moje k), drugi - wymiar 
    std::vector<vector<double>> U_; //pierwszy indeks - moje k, drugi - moje i
    //matrix<double>& X_;
    matrix<double> G_;
    std::vector<double> a_opt_;
    std::vector<double> y_;
    std::vector<double> y_estimated;
};

#endif// FUZZYCMEANS_H
