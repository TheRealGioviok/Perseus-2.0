#include "uci.h"
#include "stdio.h"
#include "iostream"
#include <string>
#include <sstream>
#include "chessBoard.h"



moveInt parseNormalMove(char* moveString) {
	int source = (moveString[0] - 'a') + (8 - (moveString[1] - '0')) * 8;
	int target = (moveString[2] - 'a') + (8 - (moveString[3] - '0')) * 8;
	int promoted = 0;
	return encodeMove(source, target, promoted, 0, 0, 0, 0, 0);
}


void parseCommand(std::string command, Game* game) {
	command += " "; //cazo di sicureza
	//std::cout << "Command received: " << command << "\n";
	// parse UCI "position" command

	if (command.find("ucinewgame") != std::string::npos) {
		game->parseFen(startPosition);
		game->print();
		return;
	}
	if (command.find("position") != std::string::npos) {
		command = command.substr(9, command.size() - 9);
		//std::cout << "Command is now: " << command << "\n";
		//position startpos
		if (command.find("startpos") != std::string::npos) {
			game->parseFen(startPosition);
			command = command.substr(9, command.size() - 9);
			//std::cout << "Command is now: " << command << "\n";
		}
		//position fen
		else if (command.find("fen") != std::string::npos) {
			command = command.substr(4, command.size() - 4);
			//std::cout << "Command is now: " << command << "\n";
			std::stringstream ss(command);
			std::string read;
			std::string fen="";
			int cnt = 0;
			while (ss >> read && cnt<6) {
				fen += read + " ";
				cnt++;
			}
			//std::cout << "FEN IS " << fen;
			//std::cout << "\n";
			game->parseFen(fen.c_str());
			command = command.substr(fen.size(), command.size() - fen.size());
			//std::cout << "Command is now: " << command << "\n";
		}
		//position startpos/fen moves ...
		if (command.find("moves") != std::string::npos) {
			command = command.substr(6, command.size() - 6);
			//std::cout << "Command is now: " << command << "\n";
			//loop checking for moves
			std::stringstream ss(command);
			std::string move;
			while (ss >> move) {
				if (!game->makeMove(game->getLegal(move.c_str()), allMoves)) {
					return;
				}
			}
		}
		game->print();
	}
	//go
	if (command.find("go") != std::string::npos) {
		command = command.substr(3, command.size() - 3);
		//std::cout << "Command is now: " << command << "\n";
		//init depth
		int depth = 6;
		//go depth n (fixed search)
		if (command.find("depth") != std::string::npos) {
			command = command.substr(6, command.size() - 6);
			//std::cout << "Command is now: " << command << "\n";

			std::stringstream ss(command);
			ss >> depth;
			std::cout << "Starting search at depth " << depth << "\n";
			
		}
		else if (command.find("perft") != std::string::npos) {
			command = command.substr(6, command.size() - 6);
			//std::cout << "Command is now: " << command << "\n";

			std::stringstream ss(command);
			ss >> depth;
			std::cout << "Starting perft at depth " << depth << "\n";
			perftDriver(depth, game);
		}
		game->searchPosition(depth);
		//game->searchPosDepth(depth);
	}
	
	
}

void uciLoop() {
	std::cout.flush();
	std::cin.clear();
	std::cin.ignore(100000, '\n');
	std::cin.sync();

	std::string input = "";
	
	std::cout << "id name Perseus\nid name Giovanni Maria Manduca\ntransposition table size 0MB\n";

	//start protocol
	std::cout << "uciok\n";
	Game g(startPosition);
	//mainloop
	while (1) {

		input = ""; //reset read
		//std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		std::cout.flush();

		//get user/GUI input
		std::getline(std::cin, input);
		//std::cout << "Got " << input<<"\n";

		if (input.at(0) == ' ')continue; //make sure it is not a newline

		if (input.find("uci") != std::string::npos && input.size()<=5) {
			std::cout << "id name Perseus\nid name Giovanni Maria Manduca\ntransposition table size 0MB\n";
			//start protocol
			std::cout << "uciok\n";
		}
		//quit
		if (input.find("quit") != std::string::npos) {
			std::cout << "Quitting...\n";
			break;
		}

		//parse uci is ready
		if (input.find("isready") != std::string::npos) {
			std::cout << "readyok\n";
			continue;
		}
		parseCommand(input, &g);
	}
}