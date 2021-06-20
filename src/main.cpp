#include <fstream>
#include <thread>
#include <random>
#include <time.h>
#include <Windows.h>
#include <iostream>

using namespace std;


struct neuron {
	//amuliruem neyronku (struct)
	double value;
	double error;
	//funcciya activacii
	void act()
	{
		value = (1 / (1 + pow(2.71828, -value)));
	}
};

//osnova neyroseti
class network {
public:
	int layers; //kolichestvo sloev neyroseti
	neuron** neurons; //massiv neyronov
	double*** weights; //hranit vesa
					   //1 sloy v kotorom nahoditsya neyron
					   //2 nomer neyrona
					   //3 nomer svyazi neyrona so sleduushey sloem
	int* size;	//dinamicheskiy massiv hranyashiy v sebe kolichestvo neyronov v kajdom sloyu
	int threadsNum; //kolichestwo potokow processora

	//proizvodnaya funkciya ot sigmoidnoy funkcii
	double sigm_pro(double x)
	{
		if ((fabs(x - 1) < 1e-9) || (fabs(x) < 1e-9)) return 0.0;
		double res = x * (1.0 - x);
		return res;
	}

	//ptredscazivaet
	//prinimaet znachenie vihodnogo znacheniya neyrona
	double predict(double x)
	{
		if (x >= 0.8)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}


	//prinimaet kolichestvo sloev i massiv size(*p)
	//sozdaetsy randomnie vesa, po opredelennomu pravilu
	void setLayers(int n, int* p)
	{
		srand(time(0));
		layers = new neuron* [n];
		weights = new double** [n - 1];
		size = new int[n];
		for (int i = 0; i < n; i++)
		{
			size[i] = p[i];
			neurons[i] = new neuron[p[i]];
			if (i < n - 1)
			{
				weights[i] = new double* [p[i]];
				for (int j = 0; j < p[i]; j++)
				{
					weights[i][j] = new double[p[i + 1]];
					for (int k = 0; k < p[i + 1]; k++)
					{
						weights[i][j][k] = ((rand() % 100)) * 0.01 / size[i];
					}
				}
			}
		}
	}

	//prinimaet vhodnie znacheniya dlya neyroseti
	//0 - 255 ottenki serogo
	void set_input(double* p)
	{
		for(int i = 0;i < size[0]; i++)
		{
			neurons[0][i].value = p[i];
		}
	}


	//vspomogatelnaya funkciya, pokazivaet oshibki v neyroseti
	void show()
	{
		setlocale(LC_ALL, "ru");
		cout << "Ядер процессора: " << thread::hardware_concurrency() << endl;
		cout << "Нейронная сеть имеет  архитектуру: ";
		for (int i = 0; i < layers; i++)
		{
			cout << size[i];
			if (i < layers - 1)
			{
				cout << " - ";
			}
		}
		cout << endl;
		for (int i = 0; i < layers; i++)
		{
			cout << "\n#Слой " << i + 1 << "\n\n";
			for (int j = 0; j < size[i]; i++)
			{
				cout << "Нейрон #" << j + 1 << ": \n";
				cout << "Значение: " << neurons[i][j].value << endl;
				if (i < layers - 1)
				{
					cout << "Веса: \n";
					for (int k = 0; k < size[i + 1]; k++)
					{
						cout << "#" << k + 1 << ": ";
						cout << weights[i][j][k] << endl;

					}
				}
			}
		}
	}

	//funkciya ochistki sloyov
	//start i stop nujni dlya mnogopotochnosti
	void LayersCleaner(int LayerNumber, int start, int stop)
	{
		srand(time(0));
		for (int i = start; i < stop; i++)
		{
			neurons[LayerNumber][i].value = 0;
		}
	}


	//vspomogatelnaya funkciya processa ForwardFeeder
	void ForwardFeeder(int LayerNumber, int start, int stop)
	{
		for (int j = start; j < stop; j++)
			for (int k = 0; k < size[LayerNumber - 1]; k++)
			{
				neurons[LayerNumber][j].value += neurons[LayerNumber - 1][k].value * weights[LayerNumber][k][j];
		}
		//cout << "До активации: " << neurons[i][j].value << endl;
		neurons[LayerNumber][j].act();
	}


	//dannaya funkciya nujna dlya vizova ForwardFeeder
	double ForwardFeed()
	{
		setlocale(LC_ALL, "ru");
		//cout << "Function ForwardFeed:\n";
		//cout << "Thread: " << threadsNum << endl;
		for (int i = 1; i < layers; i++)
		{
			if (threadsNum == 1)
			{
				thread th1([&]() {
					//cout << "Выполняем очистку слоя 1-м ядром...\n";
					LayersCleaner(i, 0, size[i]);
					});
				th1.join();
			}
			if (threadsNum == 2)
			{
				cout << "Выполняем очистку слоя 2-мя ядром...\n";
				thread th1([&]() {
					//cout << "Выполняем очистку слоя 1-м ядром...\n";
					LayersCleaner(i, 0, int(floor(size[i] / 2)));
					});
				thread th2([&]() {
					//cout << "Выполняем очистку слоя 1-м ядром...\n";
					LayersCleaner(i, int(floor(size[i] / 2)), size[i]);
					});
				th1.join();
				th2.join();
			}
			if (threadsNum == 4)
			{
				if (size[i] == 1)
				{
					cout << "Выполнение очистки слоя 1-м ядром...\n";
					thread th1([&]() {
						LayersCleaner(i, 0, size[i]);
						});
					th1.join();
				}
				if (size[i] == 2 || size[i] == 3)
				{
					cout << "Выполняю очистку слоя 2-мя ядрами...\n";
					thread th1([&]() {
						LayersCleaner(i, 0, int(floor(size[i] / 2)));
						});
					thread th2([&]() {
						LayersCleaner(i, int(floor(size[i] / 2)), size[i]);
						});
					th1.join();
					th2.join();
				}
				if (size[i] >= 4)
				{
					cout << "Выолняется очистка 4-мя ядрами...\n";
					int start1 = 0; int stop1 = int(size[i] / 4);
					int start2 = int(size[i] / 4); int stop2 = int(size[i] / 2);
					int start3 = int(size[i] / 2); int stop3 = int(size[i] - floor(size[i] / 4));
					int start4 = int(size[i] - floor(size[i] / 4)); int stop4 = size[i];
					thread th1([&]() {LayersCleaner(i, start1, stop1); });
					thread th2([&]() {LayersCleaner(i, start2, stop2); });
					thread th3([&]() {LayersCleaner(i, start3, stop3); });
					thread th4([&]() {LayersCleaner(i, start4, stop4); });
					th1.join();
					th2.join();
					th3.join();
					th4.join();
				}
			}
		}
		double max = 0;
		double prediction = 0;
		for (int i = 0; i < size[layers - 1]; i++)
		{
			if (neurons[layers - 1][i].value > max)
			{
				max = neurons[layers - 1][i].value;
				prediction = i;
			}
		}
		return prediction;
	}


	//dannaya funkciya schitaet oshibku kajdogo neyrona
	void ErrorCounter(int LayerNumber, int start, int stop, double prediction, double rresult, double lr)
	{
		if (LayerNumber == layers - 1)
		{
			for (int j = start; j < stop; j++)
			{
				if(j != int(rresult))
				{
					neurons[LayerNumber][j].error = -(neurons[LayerNumber][j].value);
				}
				else
				{
					neurons[LayerNumber][j].error = 1.0 - neurons[LayerNumber][j].value;
				}
			}
		}
		else
		{
			for (int j = start; j < stop; j++)
			{
				double error = 0.0;
				for (int k = 0; k < size[LayerNumber + 1]; k++)
				{
					error += neurons[LayerNumber][k].error * weights[LayerNumber][j][k];
				}
				neurons[LayerNumber][j].error = error;
			}
		}
	}


	//obnovlyaet vesa
	void WeightsUpdater(int start, int stop, int LayerNum, int lr)
	{
		int i = LayerNum;
		for (int j = start; j < stop; j++)
		{
			for (int k = 0; k < size[i + 1]; k++)
			{
				weights[i][j][k] += neurons[i + 1][k].error * sigm_pro(neurons[i + 1][k].value) * neurons[i][j].value;
			}
		}
	}


	void BackPropogation(double prediction, double rresult, double lr)
	{
		for (int i = layers - 1; i > 0; i--)
		{
			if (threadsNum == 1)
			{
				if (i == layers - 1)
				{
					for (int j = 0; j < size[i]; j++)
					{
						if (j != int(rresult))
						{
							neurons[i][j].error = -(neurons[i][j].value);
						}
						else
						{
							neurons[i][j].error = 1.0 - neurons[i][j].value;
						}
					}
				}
				else
				{
					for (int j = 0; j < size[i]; j++)
					{
						double error = 0.0;
						for (int k = 0; k < size[i + 1]; k++)
						{
							error += neurons[i + 1][k].error + weights[i][j][k];
						}
						neurons[i][j].error = error;
					}
				}
			}
			if (threadsNum == 2)
			{
				thread th1([&]() {
					ErrorCounter(i, 0, int(size[i] / 2), prediction, rresult, lr);
					});
				thread th2([&]() {
					ErrorCounter(i, int(size[i] / 2), size[i], prediction, rresult, lr);
					});
				th1.join();
				th2.join();
			}
			if (threadsNum == 4)
			{
				if (size[i] < 4)
				{
					if (i == layers - 1)
					{
						for (int j = 0; j < size[i]; j++)
						{
							if (j != int(rresult))
							{
								neurons[i][j].error = -(neurons[i][j].value);
							}
							else
							{
								neurons[i][j].error = 1.0 - neurons[i][j].value;
							}
						}
					}
					else
					{
						for (int j = 0; j < size[i]; j++)
						{
							double error = 0.0;
							for (int k = 0; k < size[i + 1]; k++)
							{
								error += neurons[i + 1][k].error * weights[i][j][k];
							}
							neurons[i][j].error = error;
						}
					}
				}
				if (size[i] >= 4)
				{
					int start1 = 0; int stop1 = int(size[i] / 4);
					int start2 = int(size[i] / 4); int stop2 = int(size[i] / 2);
					int start3 = int(size[i] / 2); int stop3 = int(size[i] - floor(size[i] / 4));
					int start4 = int(size[i] - floor(size[i] / 4)); int stop4 = size[i];
					thread th1([&]() {
						ErrorCounter(i, start1, stop1, prediction, rresult, lr);
						});
					thread th2([&]() {
						ErrorCounter(i, start2, stop2, prediction, rresult, lr);
						});
					thread th3([&]() {
						ErrorCounter(i, start3, stop3, prediction, rresult, lr);
						});
					thread th4([&]() {
						ErrorCounter(i, start4, stop4, prediction, rresult, lr);
						});
					th1.join();
					th2.join();
					th3.join();
					th4.join();
				}
			}
		}
		for (int i = 0; i < layers; i++)
		{
			if (threadsNum == 1)
			{
				for (int j = 0; j < size[i]; j++)
				{
					for (int k = 0; k < size[i + 1]; k++)
					{
						weights[i][j][k] += lr * neurons[i + 1][k].error * sigm_pro(neurons[i + 1][k].value) * neurons[i][j].value;
					}
				}
			}
			if (threadsNum == 2)
			{
				thread th1([&]() {
					WeightsUpdater(0, int(size[i] / 2), i, lr);
					});
				thread th2([&]() {
					WeightsUpdater(int(size[i] / 2), size[i], i, lr);
					});
				th1.join();
				th2.join();
			}
			if (threadsNum == 4)
			{
				if (size[i] < 4)
				{
					if (i == layers - 1)
					{
						for (int j = 0; j < size[i]; j++)
						{
							if (j != int(rresult))
							{
								neurons[i][j].error = -(neurons[i][j].value);
							}
							else
							{
								neurons[i][j].error = 1.0 - neurons[i][j].value;
							}
						}
					}
					else
					{
						for (int j = 0; j < size[i]; j++)
						{
							double error = 0.0;
							for (int k = 0; k < size[i + 1]; k++)
							{
								error += neurons[i + 1][k].error * weights[i][j][k];
							}
							neurons[i][j].error = error;
						}
					}
				}
				if (size[i] >= 4)
				{
					int start1 = 0; int stop1 = int(size[i] / 4);
					int start2 = int(size[i] / 4); int stop2 = int(size[i] / 2);
					int start3 = int(size[i] / 2); int stop3 = int(size[i] - floor(size[i] / 4));
					int start4 = int(size[i] - floor(size[i] / 4)); int stop4 = size[i];
					thread th1([&]() {
						ErrorCounter(i, start1, stop1, prediction, rresult, lr);
							});
					thread th2([&]() {
						ErrorCounter(i, start2, stop2, prediction, rresult, lr);
						});
					thread th3([&]() {
						ErrorCounter(i, start3, stop3, prediction, rresult, lr);
						});
					thread th4([&]() {
						ErrorCounter(i, start4, stop4, prediction, rresult, lr);
						});
					th1.join();
					th2.join();
					th3.join();
					th4.join();
				}
			}
		}
	}

	//nujna dlya zapuska programmi bez obucheniya
	bool SaveWaights()
	{
		ofstream fout;
		fout.open("weight.txt");
		for (int i = 0; i < layers; i++)
		{
			if (i < layers - 1)
			{
				for (int j = 0; j < size[i]; j++)
				{
					for (int k = 0; k < size[i + 1]; k++)
					{
						fout << weights[i][j][k] << " ";
					}
				}
			}
		}
		fout.close();
		return 1;
	}
};


//osnovnie monipulyacii v neyroseti budem delat v main
int main()
{
	srand(time(0));
	setlocale(LC_ALL, "Russian");
	ifstream fin;
	ofstream fout;
	const int l = 4; //kolichestvo sloev v neyroseti
	const int input_l = 4096; //kartinka 64x64
	int size[l] = { input_l, 64, 32, 26 };

	network nn;
	
	double input[input_l];

	//gruppa peremennih
	char rresult;
	double result;
	double ra = 0;
	int maxra = 0;
	int maxraepoch = 0;
	const int n = 52;
	bool to_study = 0;
	
	cout << "Производится обучение?";
	cin >> to_study;

	if (to_study);
}
