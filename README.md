kompilować z opcją -std=c++11 (g++ i clang++)

lub -std=c++0x (zaleznie od wersji kompilatora)

g++ main.cpp -o fcm -std=c++0x -lm

./fcm

g++ main_ts.cpp -o ts -std=c++0x -lm -DBOOST_UBLAS_NDEBUG

./ts
