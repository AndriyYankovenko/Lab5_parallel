#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <sstream>
#include <thread>
#include <vector>
#include <fstream>
#include <thread>
#include <mutex>
#include <cctype>

#define prot IPPROTO_TCP;

std::mutex console;


int NUM_OF_WORDS = 10;

std::vector<std::string> dictionary(NUM_OF_WORDS);

int get_words_difference(std::string word1, std::string word2) {
    if (word1.size() != word2.size()) {
        return -1;
    }
    else {
        int difference = 0;
        for (int i = 0; i < word1.size(); i++) {
            if (word1[i] != word2[i]) difference++;
        }
        return difference;
    }
}

std::vector<std::string> sentence_to_array(std::string sentence) {
    std::vector<std::string> words;
    std::istringstream ss(sentence);
    std::string word;
    while (ss >> word)
    {
        words.push_back(word);
    }
    return words;
}


std::string array_to_sentence(std::vector<std::string> words) {
    std::string sentence = "";
    for (auto word : words) {
        sentence += word + " ";
    }
    return sentence;
}

void read_dictionary_into_memory() {
    std::fstream myfile("wiki-100k.txt");
    std::string line;
    int i = 0;
    while (getline(myfile, line) && i < NUM_OF_WORDS)
    {
        if (line[0] != '#') {
            line[0] = tolower(line[0]);
            dictionary[i] = line;
            i++;
        }

    }
    myfile.close();
}

std::string fix_word(std::string word) {
    std::string word_fixed = word;
    for (auto dict_word : dictionary) {
        if (get_words_difference(dict_word, word) == 1) {
            word_fixed = dict_word;
            break;
        }
    }
    return word_fixed;
}

std::string fix_sentence(std::string sentence) {
    auto words = sentence_to_array(sentence);
    for (int i = 0; i < words.size();i++) {
        words[i] = fix_word(words[i]);
    }
    auto sentence_new = array_to_sentence(words);
    return sentence_new;
}


struct client {
    SOCKET socket = INVALID_SOCKET;
    int id;
    client( SOCKET sock,int id_) : socket(sock),id(id_) {};
};



class CorrectorServer {
    SOCKET ListenSocket = INVALID_SOCKET;
    int result;
    ADDRINFO hints;
    ADDRINFO* addrResult = NULL;


    int speak_client(client cl) {
        char recvBuffer[512];
        bool close_connection = false;
        while (!close_connection) {
            ZeroMemory(recvBuffer, 512);
            result = recv(cl.socket, recvBuffer, 512, 0);
            std::string request(recvBuffer);
            if (result > 0) {
                console.lock();
                std::cout << "Receieved data from client " << cl.id << ":\n" << recvBuffer << std::endl;
                console.unlock();
                if (request.find("EXIT") != std::string::npos) {
                    close_connection = true;
                }
                else {
                    std::string sentence = fix_sentence(request);
                    result = send(cl.socket, sentence.c_str(), (int)strlen(sentence.c_str()), 0);
                }

                if (close_connection) {
                    break;
                }
                
            }
        }
        shutdown(cl.socket, SD_SEND);
        return 0;
    }

    int listen_for_clients() {
        while (true) {
            int i = 0;
            console.lock();
            std::cout << "Server is waiting for connection" << std::endl;
            console.unlock();
            SOCKET ClientSocket = SOCKET_ERROR;
            while (ClientSocket == SOCKET_ERROR)
            {
                ClientSocket = accept(ListenSocket, NULL, NULL);

            }
            client cur_client(ClientSocket,i);
            std::thread thread(&CorrectorServer::speak_client, this, cur_client);
            thread.detach();
            console.lock();
            std::cout << "Client connected" << std::endl;
            console.unlock();
            i++;
        }
    }
public:
    void start() {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = prot;
        hints.ai_flags = AI_PASSIVE;

        getaddrinfo(NULL, "27015", &hints, &addrResult);

        ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);

        bind(ListenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);

        listen(ListenSocket, SOMAXCONN);
        
        listen_for_clients();

    }

};


int main()
{
    read_dictionary_into_memory();
    CorrectorServer server;
    server.start();
    return 0;
}