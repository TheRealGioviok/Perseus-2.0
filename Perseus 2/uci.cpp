#include "uci.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include "chessBoard.h"
#include <io.h>
#include <Windows.h>

flag quit = 0;
flag movesToGo = 30;
U64 moveTime = -1;
U64 ucitime = -1;
flag inc = 0;
U64 startTime = 0;
U64 stopTime = 0;
flag timeSet = 0;
flag stopped = 0;
flag uciModeSet = uciSearch;
flag moveCount = 0;

int inputWaiting(){
	static int init = 0, pipe;
	static HANDLE inh;
	DWORD dw;

	if (!init){
		init = 1;
		inh = GetStdHandle(STD_INPUT_HANDLE);
		pipe = !GetConsoleMode(inh, &dw);
		if (!pipe){
			SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
			FlushConsoleInputBuffer(inh);
		}
	}

	if (pipe){
		if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) return 1;
		return dw;
	}

	else{
		GetNumberOfConsoleInputEvents(inh, &dw);
		return dw <= 1 ? 0 : dw;
	}
}

// read GUI/user input
void readInput(){
	// bytes to read holder
	int bytes;

	// GUI/user input
	char input[256] = "";
	char *endc;

	// "listen" to STDIN
	if (inputWaiting())	{
		// tell engine to stop calculating
		stopped = 1;

		// loop to read bytes from STDIN
		do{
			// read bytes from STDIN
			bytes = _read(_fileno(stdin), input, 256);
		}
		// until bytes available
		while (bytes < 0);

		// searches for the first occurrence of '\n'
		endc = strchr(input, '\n');

		// if found new line set value at pointer to 0
		if (endc) *endc = 0;

		// if input is available
		if (strlen(input) > 0){
			// match UCI "quit" command
			if (!strcmp(input, "quit"))
			{
				//std::cout << "morte allo spaghetto volante"<<std::endl;
				// tell engine to terminate exacution    
				quit = 1;
			}

			// // match UCI "stop" command
			else if (!strncmp(input, "stop", 4)) {
				// tell engine to terminate exacution
				quit = 1;
			}
		}
	}
}

// a bridge function to interact between search and GUI input
void communicate() {
	// if time is up break here
	if (uciModeSet == uciTime && getTimeMs() > moveTime) {
		// tell engine to stop calculating
		stopped = 1;
	}
	// read GUI input
	readInput();
}

U64 calcMoveTime() {
	//std::cout << "movecount is -> " << moveCount << "\n";
	moveCount++;
	//std::cout << "Going to use " << ((timeToUse[moveCount] * ucitime) / 16000) + inc/2 << "ms\n";
	return (uciTime / timeToUse[moveCount])+ (U64)inc/2; //ucitime is total remaining time for computer
																		   // inc is increment
}


moveInt parseNormalMove(char* moveString) {
	int source = (moveString[0] - 'a') + (8 - (moveString[1] - '0')) * 8;
	int target = (moveString[2] - 'a') + (8 - (moveString[3] - '0')) * 8;
	int promoted = 0;
	return encodeMove(source, target, promoted, 0, 0, 0, 0, 0);
}


void parseCommand(std::string command, Game* game) {
	if(command.at(command.size()-1) != ' ')command += " "; //cazo di sicureza
	//std::cout << "Command received: " << command << "\n";
	// parse UCI "position" command

	if (command.find("ucinewgame") != std::string::npos) {
		game->parseFen(startPosition);
		game->print();
		moveCount = 0;
		wipeTT();
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
				//std::cout << "Newmove is now: " << move << "\n";
				moveInt parsedMove = game->getLegal(move.c_str());
				if (!game->makeMove(parsedMove)) return;
				if (isIrreversible(parsedMove))repetitionIndex = 0;
				repetitionTable[repetitionIndex++] = game->pos.hashKey;
			}

			//std::cout << "Remaining garbage is " << ss.str() << "\n";
		}
		game->print();
	}

	if (command.find("movetime") != std::string::npos) {
		U64 timer = getTimeMs();
		uciModeSet = uciTime;
		int depth = 1000;
		command = command.substr(8, command.size() - 8);
		//std::cout << "Command is now: " << command << "\n";

		std::stringstream ss(command);
		ss >> ucitime;
		moveTime = ucitime;
	}

	//go
	if (command.find("go") != std::string::npos) {
		U64 timer = getTimeMs();
		command = command.substr(3, command.size() - 3);
		//std::cout << "Command is now: " << command << "\n";
		//init depth
		int depth = 6;
		//go depth n (fixed search)

		if (command.find("time") != std::string::npos) {
			uciModeSet = uciTime;
			depth = 1000;
			std::cout << "Time!\n";
			std::stringstream ss(command);
			
			std::string coms[10];
			std::cout << command << "\n";
			for (int i = 0; i < 10; i++) {
				if (ss.good()) {
					ss >> coms[i];
				}
				else {
					break;
				}
			}

			for (int i = 0; i < 10; i+=2) {
				std::cout<<"evaluating "<<coms[i]<<" : "<<coms[i+1]<<"\n";
				if (coms[i] == "wtime" && game->pos.side == white) {
					ucitime = std::stoi(coms[i+1]);
				}
				else if (coms[i] == "btime" && game->pos.side == black) {
					ucitime = std::stoi(coms[i + 1]);
				}
				else if (coms[i] == "winc" && game->pos.side == white) {
					inc = std::stoi(coms[i + 1]);
				}
				else if (coms[i] == "binc" && game->pos.side == black) {
					inc = std::stoi(coms[i + 1]);
				}
				else { 
					break;
				}
			}
			
			std::cout << "start time is " << getTimeMs();
			
			std::cout << "end time will be " << moveTime << "\n";

		}

		else if (command.find("infinite") != std::string::npos) {
			depth = 1000;
			std::cout << "Analysis mode enabled\n";
			uciModeSet = uciInfinite;
		}

		else if (command.find("depth") != std::string::npos) {
			uciModeSet = uciSearch;
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
		U64 time2 = getTimeMs();
		//std::cout << "Time elapsed : " << (time2 - timer) << "\n";
		//std::cout << "Nodes: " << game->nodes << "\n";
		//std::cout << "Speed: " << (game->nodes / (time2 - timer)) * 1000 << "N/S\n";
		//game->searchPosDepth(depth);
	}

	if (command.find("make") != std::string::npos) {
		
		command = command.substr(4, command.size() - 4);
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

			U64 timer = getTimeMs();
			game->searchPosition(depth);
			U64 time2 = getTimeMs();
			std::cout << "Time elapsed : " << (time2 - timer) << "\n";
			std::cout << "Nodes: " << game->nodes << "\n";
			std::cout << "Speed: " << (game->nodes / (time2 - timer)) << "kN/S\n";
			game->makeMove(pvTable[0][0]);
			game->print();
		}
		else {
			command = command.substr(1, command.size() - 2);
			moveInt move = game->getLegal(command.c_str());
			if (move) {
				std::cout << "Playing move " << command << "\n";
				game->makeMove(move);
				game->print();
			}
			else {
				std::cout << "MOve is not legal... Try again\n";
			}
		}
		
	}
	
	std::cout << std::flush;
}

void uciLoop() {
	std::cout.flush();
	std::cin.clear();
	std::cin.ignore(500, '\n');
	std::cin.sync();

	std::string input = "";
	
	std::cout << "id name Perseus\nid author Giovanni Maria Manduca\ntransposition table size 0MB\n";

	//start protocol
	std::cout << "uciok\n";
	Game g(startPosition);
	//mainloop
	while (1) {

		input = ""; //reset read
		//std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		//std::cout.flush();

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
		std::cout << std::endl;
		std::cin.clear();
		//std::cin.ignore(500, '\n');
		std::cin.sync();
	}
}