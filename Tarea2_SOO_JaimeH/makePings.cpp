#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <array>

using namespace std;

class pingInfo{
public:
	int enviado, entregado, perdido;
	string estado, ip;
	pingInfo* nexta;
};

pingInfo* actPInfo = NULL;
pingInfo* listPI = NULL;
int maxIps = 0;
//pthread_mutex_t mutex;

void * routine(void *ip){

	array<char, 128> buffer;

	char *rIp = (char *)ip;
	char *result = new char[50];
	strcpy(result, "ping -q -c ");
	strcat(result, rIp);

	FILE *fp = popen(result, "r");
	if(!fp){
		cout << "Error en la ip: ";
		cout << rIp << endl;
		maxIps--;
		terminate();
	}

	int i = 0;
	while(fgets(buffer.data(), 128, fp) != NULL){
		if(i == 3){
			result = buffer.data();
			break;
		}
		i++;
	}
	auto returnC = pclose(fp);

	char *toSave = strtok(result, ",");
	toSave = strtok(NULL, " received");
	int recived = stoi(toSave);

	toSave = strtok(rIp, " ");
	int sended = stoi(toSave);
	
	//pthread_mutex_lock(&mutex);

	actPInfo->nexta = new pingInfo();

	actPInfo->enviado = sended;
	actPInfo->entregado = recived;
	actPInfo->perdido = sended - recived;
	actPInfo->ip = strtok(NULL, "\n");
	actPInfo->estado = (recived == 0 ? "DOWN" : "UP" );

	cout << actPInfo->ip << " \t " << actPInfo->enviado << " \t " << actPInfo->entregado << " \t " << actPInfo->perdido << " \t " << actPInfo->estado << endl;

	actPInfo = actPInfo->nexta;
	actPInfo->nexta = NULL;


	if(returnC == -1){
		cout <<  "Error en pclose \n";
	}

	maxIps--;
	//pthread_mutex_unlock(&mutex);
}



int main(int argc, char * argv[]){

	//pthread_mutex_init(&mutex, NULL);

	listPI = new pingInfo();
	listPI->nexta = NULL;
	actPInfo = listPI;

	ifstream input(argv[1]);
	string ip, comandInfo;

	cout << "IP \t\t Trans.\t Rec. \t Perd. \t Estado" << endl;
	cout << "--------------------------------------------------" << endl;


	while(getline(input, ip)){
		pthread_t p1;

		comandInfo = argv[2];
		comandInfo = comandInfo + " " + ip;
		if(pthread_create(&p1, NULL, &routine, (void *)(comandInfo.c_str())) != 0){
			return 1;
		}

		if(pthread_join(p1, NULL) != 0){
			return 2;
		}
		maxIps++;
	}

	//pthread_mutex_destroy(&mutex);

	while(maxIps > 0){
		sleep(1);
	}

	return 0;
}




