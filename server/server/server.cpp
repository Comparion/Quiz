#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include <algorithm>
#include <iterator>
#include <locale.h>
#include <chrono>
#include <thread>
#include <list>
#include <vector>
#include <string>

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

#undef max

using namespace std;


class Quiz
{
public:
	string question;
	string a, b, c, d;
	string correctAnswer;
	string stringToClient;

	int userAnswer;
	string answer;

	bool isCorrect;
	int questionsAsked = 0;
	int correctAnswers = 0;

	Quiz(string question, string a, string b, string c, string d, string correctAnswer)
	{
		this->question = question;
		this->a = a;
		this->b = b;
		this->c = c;
		this->d = d;
		this->correctAnswer = correctAnswer;
	}

	void questionReroll(string question, string a, string b, string c, string d, string correctAnswer)
	{
		this->question = question;
		this->a = a;
		this->b = b;
		this->c = c;
		this->d = d;
		this->correctAnswer = correctAnswer;
	}

	const char* printQuestion()
	{
		stringToClient = question + "\n" + a + "\n" + b + "\n" + c + "\n" + d + "\n" + "Poprawne odpowiedzi:" + to_string(correctAnswers) + "/" + to_string(questionsAsked) + "\n";
		return stringToClient.c_str();
	}

	void checkAnswer(const char* userAnswer)
	{
		if (strcmp(userAnswer, "1") == 0)
		{
			this->answer = a;
		}
		else if (strcmp(userAnswer, "2") == 0)
		{
			this->answer = b;
		}
		else if (strcmp(userAnswer, "3") == 0)
		{
			this->answer = c;
		}
		else if (strcmp(userAnswer, "4") == 0)
		{
			this->answer = d;
		}
		else
		{
			answer = "udzielono odpowiedzi z poza listy";
		}
		questionsAsked++;
		compareAnswers(answer);
	}

	void compareAnswers(string answer)
	{
		if (answer == this->correctAnswer)
		{
			isCorrect = true;
			correctAnswers++;
		}
		else
		{
			isCorrect = false;
		}
	}
};

std::fstream& GotoLine(std::fstream& file, int num)
{
	file.seekg(std::ios::beg);
	for (int i = 0; i < num - 1; ++i)
	{
		file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
	return file;
}

void assignQuestion(Quiz quest, string question, string a, string b, string c, string d, string answer)
{
	quest.questionReroll(question, a, b, c, d, answer);
}

int __cdecl main(void)
{

	setlocale(LC_CTYPE, "Polish");

	int categorySelection;
	fstream quizBase;
	bool categorySelected = false;
	int questionsCount;
	int linesCount;
	int questionNumber;
	int questionValue;

	string question;
	string a, b, c, d;
	string answer;

	srand(time(NULL));


	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo* result = NULL;
	struct addrinfo hints;

	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN] = "";
	int recvbuflen = DEFAULT_BUFLEN;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	closesocket(ListenSocket);
	const char* sendbufChooseCategory = "Wybierz kategorię:\n1. Historia\n2. Geografia\n3. Polityka\n4. Dowolna\n";

	iSendResult = send(ClientSocket, sendbufChooseCategory, (int)strlen(sendbufChooseCategory), 0);
	if (iSendResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}


	iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
	if (iResult > 0) {

		printf("Ilosc otzymanych bajtow: %d\nOtrzymana wiadomosc jest o tresci'%s'\n\n\n", iResult, recvbuf);

		while (categorySelected != true)
		{
			if (strcmp(recvbuf, "1") == 0)
			{
				quizBase.open("Historia.txt");
				categorySelected = true;
			}
			else if (strcmp(recvbuf, "2") == 0)
			{
				quizBase.open("Geografia.txt");
				categorySelected = true;
			}
			else if (strcmp(recvbuf, "3") == 0)
			{
				quizBase.open("Polityka.txt");
				categorySelected = true;
			}
			else if (strcmp(recvbuf, "4") == 0)
			{
				quizBase.open("Dowolna.txt");
				categorySelected = true;
			}
			else
			{
				iSendResult = send(ClientSocket, sendbufChooseCategory, (int)strlen(sendbufChooseCategory), 0);
				if (iSendResult == SOCKET_ERROR) {
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(ClientSocket);
					WSACleanup();
					return 1;
				}
				iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
				printf("Ilosc otzymanych bajtow: %d\nOtrzymana wiadomosc jest o tresci'%s'\n\n\n", iResult, recvbuf);
			}
		}

	}
	else if (iResult == 0)
		printf("Connection closing...\n");
	else {
		printf("recv failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}


	linesCount = count(istreambuf_iterator<char>(quizBase), istreambuf_iterator<char>(), '\n') + 1;

	questionsCount = linesCount / 7;

	vector<int>availableQuestions;

	for (int i = 0; i < questionsCount; i++)
	{
		availableQuestions.push_back(i);
	}

	questionValue = rand() % availableQuestions.size();
	questionNumber = availableQuestions[questionValue];
	GotoLine(quizBase, (questionNumber * 7) + 1);

	availableQuestions.erase(std::remove(availableQuestions.begin(), availableQuestions.end(), questionNumber), availableQuestions.end());

	getline(quizBase, question);
	getline(quizBase, a);
	getline(quizBase, b);
	getline(quizBase, c);
	getline(quizBase, d);
	getline(quizBase, answer);

	Quiz question1(question, a, b, c, d, answer);

	while (question1.questionsAsked < 10)
	{
		const char* stringToClient = question1.printQuestion();
		iSendResult = send(ClientSocket, stringToClient, (int)strlen(stringToClient), 0);
		if (iSendResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}
		printf("Wyslanych bajtow: %ld\nWyslana widomosc jest o tresci: '%s'\n\n\n", iSendResult, stringToClient);

		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			question1.checkAnswer(recvbuf);
			printf("Ilosc otzymanych bajtow: %d\nOtrzymana wiadomosc jest o tresci'%s'\n\n\n", iResult, recvbuf);
		}

		else if (iResult == 0)
			printf("Connection closed\n");
		else
			printf("recv failed with error: %d\n", WSAGetLastError());

		questionValue = rand() % availableQuestions.size();
		questionNumber = availableQuestions[questionValue];
		GotoLine(quizBase, (questionNumber * 7) + 1);

		availableQuestions.erase(std::remove(availableQuestions.begin(), availableQuestions.end(), questionNumber), availableQuestions.end());

		getline(quizBase, question);
		getline(quizBase, a);
		getline(quizBase, b);
		getline(quizBase, c);
		getline(quizBase, d);
		getline(quizBase, answer);

		question1.questionReroll(question, a, b, c, d, answer);
	}

	//system("CLS");
	string toClinet = "Poprawne odpowiedzi:" + to_string(question1.correctAnswers) + "/" + to_string(question1.questionsAsked) + "\n";
	const char* stringToClient = toClinet.c_str();
	iSendResult = send(ClientSocket, stringToClient, (int)strlen(stringToClient), 0);
	if (iSendResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}
	printf("Wyslanych bajtow: %ld\nWyslana widomosc jest o tresci: '%s'\n\n\n", iSendResult, stringToClient);
	//cout << "Poprawne odpowiedzi:" << question1.correctAnswers << "/" << question1.questionsAsked << endl;

	//} while (iResult > 0);

	// shutdown the connection since we're done
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	closesocket(ClientSocket);
	WSACleanup();

	return 0;
}
