#include<iostream>
#include<thread>
#include<conio.h>
#include<Windows.h>

using namespace std;

#define Escape 27
#define Enter 13

#define MIN_TANK_VOLUME 20
#define MAX_TANK_VOLUME 80

#define MIN_ENGIN_CONSUMPTION 3
#define MAX_ENGINE_CONSUPTION 25
#define DEFAULT_ENGINE_CONSUMPTION 10

#define MAX_SPEED_LOW_LIMIT 150
#define MAX_SPEED_HIGHT_LIMIT 300
#define DEFAULT_MAX_SPEED 250

class Tank
{
	const unsigned int volume; // Характеристика бака - его объем
	double fuel_level;
public:
	unsigned int get_volume()const
	{
		return this->volume;
	}
	double get_fuel_level()const
	{
		return fuel_level;
	}
	void fill(double fuel_level)
	{
		if (fuel_level <= 0)return;
		if (this->fuel_level + fuel_level <= volume)this->fuel_level += fuel_level;
		else this->fuel_level = volume;
	}
	double give_fuel(double amount)
	{
		if (fuel_level - amount > 0) fuel_level -= amount;
		else fuel_level = 0;
		return fuel_level;
	}
	Tank(const unsigned int volume):
		volume(volume >= MIN_TANK_VOLUME && volume <= MAX_TANK_VOLUME?volume:60)
	{
		this->fuel_level = 0;
		cout << "Tank is ready" << endl;;
	}
	~Tank()
	{
		cout << "Tank is over" << endl;
	}
	void info()const
	{
		cout << "Tank volume: " << volume << " liters\n";
		cout << "Fuel level: " << fuel_level << " liters\n";
	}
};

class Engine
{
	double consumption; //расход на 100 км
	double consumption_per_second; //расход за одну секунду
	double default_consumption_per_second; //расход за одну секунду
	bool is_started; // двигатель заведен или заглушен
public:
	double get_consumption()const
	{
		return consumption;
	}
	double get_consumption_per_second()const
	{
		return consumption_per_second;
	}
	void set_connsuption(double consumption)
	{
		if (consumption >= MIN_ENGIN_CONSUMPTION && consumption <= MAX_ENGINE_CONSUPTION)
		{
			this->consumption = consumption;
		}
		else
		{
			this->consumption = 10;
		}
		this->consumption_per_second = this->consumption * 3e-5;
		this->default_consumption_per_second = this->consumption * 3e-5;
	}
	void set_consumption_per_second(double consumption)
	{
		if (consumption >= 0.0003 && consumption <= 0.003)
		{
			this->consumption_per_second = consumption;
		}
	}
	void speed_consumption(int speed)
	{
		if (speed >= 1 &&speed <= 60)consumption_per_second = 0.002;
		else if (speed >= 61 && speed <= 100)consumption_per_second = 0.0014;
		else if (speed >= 101 && speed <= 140)consumption_per_second = 0.002;
		else if (speed >= 141 && speed <= 200)consumption_per_second = 0.0025;
		else if (speed >= 201)consumption_per_second = 0.003;
		else if (speed == 0)consumption_per_second = get_consumption() * 3e-5;
	}
	void start()
	{
		is_started = true;
	}
	void stop()
	{
		is_started = false;
	}
	bool started()const
	{
		return is_started;
	}
	void info()const
	{
		cout << "Consumption per 100 km: " << consumption << " liters\n";
		cout << "Consumption per second: " << consumption_per_second << " liters\n";
	}

	Engine(double consumption)
	{
		set_connsuption(consumption);
		is_started = false;
		cout << "Engine is ready" << endl;
	}
	~Engine()
	{
		cout << "Engine is over" << endl;
	}

};

class Car
{
	Engine engine;
	Tank tank;
	bool driver_inside;
	int speed;
	const int MAX_SPEED;
	int accelleration;
	struct Control
	{
		std::thread panel_thread;
		std::thread free_wheeling_thread;
		std::thread engine_idle_thread;
	}control;
public:
	Car(double engine_consumptioon, unsigned int tank_volume, int accelleration = 10,int max_speed = DEFAULT_MAX_SPEED)
		:engine(engine_consumptioon), tank(tank_volume),
		MAX_SPEED(max_speed >= MAX_SPEED_LOW_LIMIT && max_speed <= MAX_SPEED_HIGHT_LIMIT ? max_speed : DEFAULT_MAX_SPEED)
	{
		driver_inside = false;
		speed = 0;
		this->accelleration = accelleration >= 5 && accelleration <= 25 ? accelleration : 12;
		cout << "Your car is ready" << endl;
	}
	~Car()
	{
		cout << "Car is over" << endl;
	}

	void get_in()
	{
		driver_inside = true;
		//panel
		control.panel_thread = std::thread(&Car::panel, this);
	}
	void get_out()
	{
		if (speed > 0)
		{
			cout << "Выход из машины во время движения может навредить вашему здоровью" << endl;
			return;
		}
		driver_inside = false;
		if(control.panel_thread.joinable())control.panel_thread.join();
		system("CLS");
		cout << "You are out of your car" << endl;
	}

	void start_engine()
	{
		if (driver_inside && tank.get_fuel_level())
		{
			engine.start();
			control.engine_idle_thread = std::thread(&Car::engine_idle, this);
		}
	}
	void stop_engine()
	{
		engine.stop();
		if (control.engine_idle_thread.joinable())control.engine_idle_thread.join();
	}

	void accellerate()
	{
		if (driver_inside && engine.started() && speed < MAX_SPEED)
		{
			speed += accelleration;
			engine.speed_consumption(speed);
		if (!control.free_wheeling_thread.joinable())
			control.free_wheeling_thread = std::thread(&Car::free_wheeling, this);
			std::this_thread::sleep_for(1s);
		}
	}
	void slow_down()
	{
		if (driver_inside && speed > 0)
		{
			speed -= accelleration;
			engine.speed_consumption(speed);
			if (speed < 0) speed = 0;
			std::this_thread::sleep_for(1s);
		}
	}

	void control_car()
	{
		cout << "Press Enter to get in" << endl;
		char key;
		do
		{
			key = _getch();
			switch (key)
			{
			case Enter:
			{
				if (driver_inside) get_out();
				else get_in();

				break;
			}//Выход/Вход из машины
			case 'I':
			case 'i':
				if (engine.started())stop_engine();
				else start_engine();
				break;

			case 'F':
			case 'f':
				if (driver_inside)
				{
					cout << "Для того ятобы заправится нужно выйти из машины" << endl;
				}
				else
				{
					double amount;
					cout << "Введите литраж: ";
					cin >> amount;
					tank.fill(amount);
					cout << "Заправка успешно завершена";
					break;
				}
			case 'W': case 'w':
			{
				if (driver_inside && engine.started())
				{
					if (speed < MAX_SPEED)speed += accelleration;
					if (speed > MAX_SPEED)speed = MAX_SPEED;
					if (!control.free_wheeling_thread.joinable())
						control.free_wheeling_thread = std::thread(&Car::free_wheeling, this);
					std::this_thread::sleep_for(1s);
				}
				break;
			}
			case 'S': case 's':
				if (driver_inside && speed > 0)
				{
					speed -= accelleration;
					if (speed < 0)speed = 0;
					std::this_thread::sleep_for(1s);
				}
				
				break;

			case Escape: speed = 0; get_out(); stop_engine();	break;
			}
			
		} while (key != Escape);
		if (speed == 0 && control.free_wheeling_thread.joinable())
			control.free_wheeling_thread.join();
	}

	void panel()const
	{
		while (driver_inside)
		{
			system("CLS");
			for (int i = 0; i < speed / 3; ++i)cout << "|";
			cout << endl;
			cout << "Speed:\t" << speed << "km/h\n";
			cout << "Fuel level: " << tank.get_fuel_level() << " liters\t";
			if (tank.get_fuel_level() < 5)
			{
				HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
				SetConsoleTextAttribute(hConsole, 0xC7);
				cout << "LOW FUEL";
				SetConsoleTextAttribute(hConsole, 0x07);
			}
			cout << endl;
			cout << "Engine is: " << (engine.started() ? "started" : "stopped") << endl;
			if(engine.started())cout << "Consumption per second: " << engine.get_consumption_per_second() << endl;
			std::this_thread::sleep_for(1s);
		}
	}
	void engine_idle()
	{
		while (engine.started() && tank.give_fuel(engine.get_consumption_per_second()))
		{
			std::this_thread::sleep_for(1s);
		}
	}

	void free_wheeling()
	{
		while (speed > 0)
		{
			speed--;
			if (speed < 0) speed = 0;
			engine.speed_consumption(speed);
			std::this_thread::sleep_for(1s);
		}
		engine.speed_consumption(speed);
	}
	void info()const
	{
		engine.info();
		tank.info();
	}
};

//#define TANK_CHECK
//#define ENGINE_CHECK

void main()
{
	setlocale(LC_ALL, "");
#ifdef TANK_CHECK

	Tank tank(40);
	tank.info();
	int fuel;
	while (true)
	{
		cout << "Введите объем топлива: ";
		cin >> fuel;
		tank.fill(fuel);
		tank.info();
	}
#endif // TANK_CHECK

#ifdef ENGINE_CHECK
	Engine engine(20);
	engine.info();
#endif // ENGINE_CHECK

	Car bmw(12, 60);
	/*bmw.info();*/
	bmw.control_car();
}