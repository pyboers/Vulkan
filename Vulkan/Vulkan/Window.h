#pragma once
#include <GLFW/glfw3.h>

class Window {
private:
	int m_width, m_height;

public:
	GLFWwindow* m_window;

	Window(int width, int height, const char* name);

	~Window();
};