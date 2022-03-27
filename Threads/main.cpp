#include<iostream>
#include<thread>

using namespace std;
using namespace std::chrono_literals;

bool finish = false;

void Hello()
{
	while (!finish)
	{
		cout << "+ ";
		std::this_thread::sleep_for(1s);
	}
}

void World()
{
	while (!finish)
	{
		cout << "- ";
		std::this_thread::sleep_for(1s);
	}
}
void main()
{
	setlocale(LC_ALL, "");
	std::thread hello_thread;
	std::thread world_thread;

	hello_thread = std::thread(Hello);
	world_thread = std::thread(World);

	cin.get();

	finish = true;

	hello_thread.join();
	world_thread.join();
}