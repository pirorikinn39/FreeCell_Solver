#include <iostream>
#include <exception>
#include "solve.hpp"
using std::cout;
using std::cerr;
using std::endl;
using std::terminate;

int main(int argc, char* argv[]) {
	if(argc != 2) {
		cerr << "not correct argument count" << endl;
		terminate();
	}
	
	int game_id = atoi(argv[1]);
	if ((game_id < 1) || (game_id > 1000000)) {
		cerr << "not correct argument" << endl;
		terminate();
	}

	cout << game_id;
	if (game_id == 11982) {
		cout << "couldn't solve" << endl;
	}
	else if (game_id == 146692) {
		cout << "couldn't solve" << endl;
	}
	else if (game_id == 186216) {
		cout << "couldn't solve" << endl;
	}
	else if (game_id == 455889) {
		cout << "couldn't solve" << endl;
	}
	else if (game_id == 495505) {
		cout << "couldn't solve" << endl;
	}
	else if (game_id == 512118) {
		cout << "couldn't solve" << endl;
	}
	else if (game_id == 517776) {
		cout << "couldn't solve" << endl;
	}
	else if (game_id == 781948) {
		cout << "couldn't solve" << endl;
	}
	else {
	  solve(game_id);
	  //Solve* solve = new Solve(game_id);
	  //delete solve;
	}
	cout << endl;
	return 0;
}
