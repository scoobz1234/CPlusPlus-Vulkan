#include "stdafx.h"
#include "TombGame.h"
#include <stdexcept>
#include <iostream>

int main(int argc, char** argv) {
	
	TombGame tombGame; // create the game object...

	try {
		tombGame.Run(); // try to call the Run function
	}
	catch (const std::runtime_error& e) { // if it fails lets log a runtime error e
		std::cerr << e.what() << std::endl; // tell me about that error...
		return EXIT_FAILURE; // then exit with failure
	}

	return EXIT_SUCCESS; // or exit success...
}