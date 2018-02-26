#pragma once
#include <GLFW/glfw3.h>

namespace ScoobzEngine{

	class Window{
	public:
		Window();~Window();

		//FUNCTION DECLARATIONS
		int Create();

		//HANDLES
		GLFWwindow* window; // container for the window, must be a pointer.
		
		//MEMBER VARIABLES
		int windowWidth = 800;
		int windowHeight = 600;
	};
}