#include "Window.h"

namespace ScoobzEngine{
	Window::Window(){}
	Window::~Window(){}

	int Window::Create(){

		glfwInit(); // starts up glfw.. built in function
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //glfw works in opengl, were turning this function off
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); // window is resizable, true for yes false for no...
		window = glfwCreateWindow(windowWidth, windowHeight, "ScoobzEngine", nullptr, nullptr); // create the window, give it info
		
		return 0;
	}
}