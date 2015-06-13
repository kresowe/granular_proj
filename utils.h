#ifndef UTILS_HPP
#define UTILS_HPP
#include <algorithm>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/tokenizer.hpp>
#include <boost/token_functions.hpp>
#include <fstream>
#include <string>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <list>
#include <iterator>
namespace utils
{
    using boost::numeric::ublas::matrix;
    using boost::numeric::ublas::vector;
    template<typename Iterator>
        double mean(Iterator begin, Iterator end)
        {
            return std::accumulate(begin, end, 0.0)/(std::distance(begin,end));
        }
    template<typename Iterator>
        double variance(Iterator begin, Iterator end)
        {
            double tmp_mean = mean(begin, end);
            return std::accumulate(begin, end, 0.0, [&](double x, double x_x)
                    {
                    return x+(x_x-tmp_mean)*(x_x-tmp_mean);
                    })/(std::distance(begin,end));
        }
    void normalize_rows(matrix<double> & mat)
    {
        for(auto it1 = mat.begin1(); it1 != mat.end1(); ++it1)
        {
            double sum = std::accumulate(it1.begin(), it1.end(), 0.0);
            std::transform(it1.begin(), it1.end(), it1.begin(),
                    [sum](double x){return x/sum;});
        }
    }
    template<typename Iterator>
    void normalize(Iterator begin, Iterator end)
    {
        double sum = std::accumulate(begin, end, 0.0);
        std::transform(begin, end, begin,
                [sum](double x)
                {
                    return x / sum;
                });
    }

    //Michal
    void split_on_whitespace(std::string str, std::vector<std::string> &elements) {
        boost::char_separator<char> sep{" "};
        boost::tokenizer<boost::char_separator<char>> tok(str, sep);
        std::copy(tok.begin(),tok.end(), std::back_inserter(elements));

        /*int pos = 0;
        elements.clear();
        while ((pos = str.find(" ")) != std::string::npos)
        {
            elements.push_back(str.substr(0, pos));
            while (str[pos] == ' ')
                pos++;
            str.erase(0, pos);
        }*/
    }

    int number_of_lines_in_file(const char *filename) {
        std::ifstream fin;
        int number_of_lines = 0;
        std::string line;

        fin.open(filename);
        if (!fin.is_open()) //przy nadmiarze czasu zmienic na obsluge wyjatkow
            std::cerr << "Cannot open data file " << filename << ".\n";
        while (std::getline(fin, line))
            number_of_lines++;
        fin.close();
        std::cout << "File has " << number_of_lines << " lines.\n";
        return number_of_lines;
    }

    void choose_random_data(const char *filename, std::list<int> &lines_list) {
        srand(time(NULL));
        int number_of_lines = number_of_lines_in_file(filename);
        int data_number = 0.6 * number_of_lines;
        int random_line;

        for (int i = 0; i < data_number; i++)
        {
            random_line = rand() % number_of_lines;
            lines_list.push_back(random_line);
        }
        lines_list.sort();
        lines_list.unique();
    }

    void list_to_vector(std::list<int> a_list, std::vector<int> &a_vec) {
        int i = 0;
        for (std::list<int>::iterator it = a_list.begin(); it != a_list.end(); ++it)
        {
            a_vec[i] = *it;
            i++;
        }
    }

    /*
    * Function for X data (multi-dimension variable)
    * filename - data file name
    * container - object which is to store data from file when program is running
    * columns - list of columns in file which contain data.
    * file_random_lines_vec - lines (records) of data to load (we choose some of them randomally)
    * incl - use mentioned lines if true, otherwise use all the lines excluding file_random_lines_vec
    *       Therefore incl = true for training data set and incl = false for testing data set.
    */
    void load_data(const char *filename, std::vector<vector<double>> &container, 
        std::vector<int> columns, std::vector<int> file_random_lines_vec, 
        bool incl) {
        int n = columns.size();
        std::ifstream fin;
        std::string line;
        std::vector<std::string> data;
        std::stringstream strin;
        int lines;
        double val;
        int i, j;

        fin.open(filename);
        if (!fin.is_open()) //przy nadmiarze czasu zmienic na obsluge wyjatkow
            std::cerr << "Cannot open data file " << filename << ".\n";

        lines = 0;
        j = 0;

        for (j = 0; j < file_random_lines_vec.size(); j++)
            std::cout << file_random_lines_vec[j] << std::endl;

        j = 0;
        while (!fin.eof() && j < file_random_lines_vec.size())
        {
            data.clear();
            std::getline(fin, line);
            std::cout << "Linia: " << line << std::endl;
            std::cout << lines << " " << file_random_lines_vec[j] << std::endl;
            if ((incl && lines != file_random_lines_vec[j]) || 
                (!incl && lines == file_random_lines_vec[j]))
            {
                lines++;
                continue;
            }
            //uzyj tylko tych, ktore zostaly wylosowane
            split_on_whitespace(line, data); //splituj ja
            
            for (unsigned m = 0; m < data.size(); m++)
                std::cout << data[m] << std::endl;
            std::cout << std::endl;

            //zapisz w formie double do container.
            for (i = 0; i < n; i++) 
            {
                std::cout << "value0: " << data[columns[i]] << std::endl; //(1)
                std::string::size_type sz;
                container[j][i] = std::stod (data[columns[i]],&sz);
                /*
                strin.str(data[columns[i]]);
                strin >> val;
                
                std::cout << "value: " << val << std::endl; //(2) 
                container[j][i] = val;*/
                std::cout << container[j][i] << std::endl;
            }
            lines++;
            j++;

        }
        /*for (j = 0; j < container.size(); j++)
            for (i = 0; i < n; i++)
                std::cout << container[j][i] << std::endl;*/
        fin.close();
    }

    /*
    * Function for y data (scalar (1D) variable)
    * overloads
    */
    void load_data(const char *filename, std::vector<double> container, int column, 
        const std::vector<int> file_random_lines_vec, bool incl) {
        std::ifstream fin;
        std::string line;
        std::vector<std::string> data;
        std::istringstream strin;
        int lines;
        double val;

        fin.open(filename);
        if (!fin.is_open()) //przy nadmiarze czasu zmienic na obsluge wyjatkow
            std::cerr << "Cannot open data file " << filename << ".\n";

        lines = 0;
        int j = 0;
        while (!fin.eof() && j < file_random_lines_vec.size())
        {
            std::getline(fin, line);
            if ((incl && lines != file_random_lines_vec[j]) || 
                (!incl && lines == file_random_lines_vec[j]))
            {
                lines++;
                continue;
            }
            split_on_whitespace(line, data); //splituj ja

            //zapisz w formie double do container.
            strin.str(data[column]);
            strin >> val;
            container[j] = val;
            lines++;
            j++;

        }
        fin.close();
    }

    //koniec Michal
}; 

#endif// UTILS_HPP
