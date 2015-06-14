#include <iostream>
#include "utils.h"
#include <chrono>
#include <fstream>
#include "FuzzyCMeans.h"
#include <vector>
#include <list>
#include <iterator>
#include <iomanip>

#define DBOOST_UBLAS_NDEBUG 1 
#define BOOST_UBLAS_NDEBUG 1 

using boost::numeric::ublas::matrix;
using boost::numeric::ublas::vector;

/*
* Takagi sugeno
* filename - name of file with data
* columns - number of columns in file which contain data
* c - number of clusters
* outfile_name - name of file where output is written
*/
void ts(const char *filename, std::vector<int> columns, int y_column, int c, 
	const char *outfile_name, double coefficient);

void newline()
{
    std::cout << std::endl;
}

int main()
{
	int i;

	//Housing
	//wybor kolumn 
    std::vector<int> columns;
    columns.push_back(1);
    columns.push_back(2);
    for (i = 4; i <= 13; i++)
    	columns.push_back(i);
    int y_column = 0;

    std::ofstream fout;
    fout.open("housing_result.dat");
    fout.close();

    //for (int clusters = 2; clusters < 21; clusters += 2) //takes a few minutes //uncomment to analyze quality(clusters)
    int clusters = 5; //short version
    	ts("housing.data", columns, y_column, clusters, "housing_result.dat", 0.6);

    /*quality(coefficient)
    
    ts("housing.data", columns, y_column, clusters, "housing_result_coeff.dat", 0.3);
    ts("housing.data", columns, y_column, clusters, "housing_result_coeff.dat", 0.4);
    ts("housing.data", columns, y_column, clusters, "housing_result_coeff.dat", 0.5);
    ts("housing.data", columns, y_column, clusters, "housing_result_coeff.dat", 0.6);
    ts("housing.data", columns, y_column, clusters, "housing_result_coeff.dat", 0.7);
    ts("housing.data", columns, y_column, clusters, "housing_result_coeff.dat", 0.8);
    ts("housing.data", columns, y_column, clusters, "housing_result_coeff.dat", 0.9);
	*/

	return 0;
}



void ts(const char *filename, std::vector<int> columns, int y_column, int c, 
	const char *outfile_name, double coefficient) {
	//int N = 300; //liczba danych
	//int n = 3; //wymiar danych
	//int c = 5; //liczba grup
	std::ofstream fout;
    fout.open(outfile_name, std::ofstream::app);
    double train_quality, test_quality;

	//wczytywanie danych
	//wylosowanie zbioru treningowego
	std::list<int> file_random_lines;
	utils::choose_random_data(filename, file_random_lines, coefficient);
    std::vector<int> file_random_lines_vec(file_random_lines.size());

/*
    std::ostream_iterator<int, char> out(std::cout, " ");
	copy(file_random_lines.begin(), file_random_lines.end(), out);
	std::cout << std::endl;
	*/
    
    utils::list_to_vector(file_random_lines, file_random_lines_vec);

    //dane X
    std::vector<vector<double>> X(file_random_lines_vec.size(), vector<double>(columns.size()));	
    //dane y; kazdemu wektorowi x bedacemu elementem X odpowiada jedna liczba y nalezaca do Y
	std::vector<double> Y(file_random_lines_vec.size());	

    //wlasciwe wczytanie danych
    utils::load_data(filename, X, columns, file_random_lines_vec, true);
    utils::load_data(filename, Y, y_column, file_random_lines_vec, true);
	std::cout << "test1\n";

	//tutaj zapisujemy nasze dane x do obiektu FuzzyCMeans i nastepnie znajdujemy U_ij czyli nasze A_i(x_k)
	FuzzyCMeans ts_training(X, c, 2);
	std::cout << "test2\n";
	ts_training.init_y(Y);
	std::cout << "test3\n";
	ts_training.loop();
	std::cout << "test4\n";
	ts_training.calculate_G(); // budujemy macierz G.
	//ts_training.print_G();
	std::cout << "test5\n";
	ts_training.calculate_a_opt();
	std::cout << "test6\n";
	ts_training.calculate_y_estimated();
	train_quality = ts_training.quality_ts();
	std::cout << "Training: quality Q = " << std::setw(10) << std::setprecision(8) << std::fixed 
		<< train_quality << std::endl;
	std::cout.unsetf(std::ios_base::fixed);
	std::vector<double> a_opt_train(ts_training.get_a_opt());
	std::cout << "test7\n";

	X.resize(utils::number_of_lines_in_file(filename) - file_random_lines_vec.size(), vector<double>(columns.size()));
	Y.resize(utils::number_of_lines_in_file(filename) - file_random_lines_vec.size());
	std::cout << "test8\n";
	utils::load_data(filename, X, columns, file_random_lines_vec, false);
	std::cout << "test9\n";
    utils::load_data(filename, Y, y_column, file_random_lines_vec, false);
    std::cout << "test10\n";

    //for (int i = 0; i < )

    FuzzyCMeans ts_testing(X, c, 2);
    std::cout << "test11\n";
	ts_testing.init_y(Y);
	std::cout << "test12\n";
	ts_testing.loop();
	std::cout << "test13\n";
	ts_testing.calculate_G(); // budujemy macierz G.
	ts_testing.set_a_opt(a_opt_train);
	ts_testing.calculate_y_estimated();
	test_quality = ts_testing.quality_ts();
	std::cout << "testing: quality Q = " << std::setw(10) << std::setprecision(8) << std::fixed 
		<< test_quality << std::endl;
	fout << c << " " << train_quality << " " << test_quality << " " << coefficient << std::endl;
	fout.close();

}