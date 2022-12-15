#include "iostream"
#include "any"
#include "array"
#include <vector>
#include <thread>
//#include "MultiThreadTest.h"
#include "glm/glm.hpp"
#include "GLFW/include/GLFW/glfw3.h"
#include <map>
#include "Core/Public/template_test.h"
#include "Core/Public/VulkanApplication.h"


//void* operator new(size_t size)
//{
//	Print("New Somthing");
//	return malloc(size);
//}
int main()
{
#if __cplusplus == 202002L
	auto TemplateLamb = []<typename T>(T t, auto && U)->decltype(auto) { return t + U; };

	auto Value = TemplateLamb(5, 32);

	Print(Value);
#endif // __cplusplus == 202002L
	Test::template_test->TestFunction();

	VulkanApplication* application = new VulkanApplication;
	application->InitApplication();
	application->MainLoop();
	application->Destroy();
	delete application;

	return 0;
}