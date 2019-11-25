/*
 * Author: Daniel Rogers
 * Date: 11/12/2019
 *
 * This C++ program is a playable Othello game, which consists of two players
 * ('w' and 'b') competing for space on the board. The user has the option of
 * playing against a friend or the AI by tuning the PLAY_AI variable.
 *
 * The AI uses Mini-max to aid in its decision making. Due to the complexity
 * and size of Othello gametrees, alpha-beta pruning has been implemented
 * to shave off considerable time in the AI's decision making. A tunable
 * "MINIMAX_DEPTH" paramater is available to easily adjust how deep the AI goes.
 * A simple heursitic which takes into account discs belonging to each player,
 * corner occupation, and number of available moves is used by the AI to give
 * value to the board configurations it considers.
*/

#include <iostream>
#include <stdexcept>
#include <array>
#include <vector>
#include <regex>


const bool PLAY_AI = true; // set to true if you want to play the AI
const int MINIMAX_DEPTH = 5; // depth of the game tree search
const bool DEBUG_MODE = false;

// flips appropriate pieces after a disc is placed down (called after verifying the move isFlippable)
void flip(char (&board)[8][8], int row, int col, char player){
    // declare a list of positions of discs that will be flipped
    // e.g. {{0,1}, {0,2}} means disc at location board[0][1] & board[0][2] will be flipped
    std::vector<std::vector<int>> discs_to_flip;

    char otherPlayer = (player == 'b') ? 'w' : 'b';

    // use deltas to find all 8 surrounding positions
    int surroundingPosDeltas[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, // 3 positions above
                                      {0, -1}, {0, 1}, // 2 positions on same row
                                      {1, -1}, {1, 0}, {1, 1}}; // 3 positions below

    // for every delta representing a neighboring position...
    for(auto deltas : surroundingPosDeltas){
        //std::cout << "deltas: [" << deltas[0] << ", " << deltas[1] << "]" << '\n';

        // save what row/col currently on
        int curr_row = row + deltas[0];
        int curr_col = col + deltas[1];

        // ignore if this goes off of the board
        if(curr_row > 7 || curr_row < 0 || curr_col > 7 || curr_col < 0)
            continue;


        // save character in this position
        char char_in_pos = board[curr_row][curr_col];

        // use this variable to save whether or not a line of pieces should be flipped
        bool flip_this_direction = false;

        // if the character in this delta position is the opponent's piece...
        if(char_in_pos == otherPlayer){
            //std::cout << "Found other player at location: [" << curr_row << ", " << curr_col << "], " << char_in_pos << '\n';

            // continue in this delta position until the next character is no longer the opponent's or you go off the board
            while(char_in_pos == otherPlayer){
                curr_row += deltas[0];
                curr_col += deltas[1];

                // check to see if new position is off board
                if(curr_row > 7 || curr_row < 0 || curr_col > 7 || curr_col < 0)
                    break;

                // save the character
                char_in_pos = board[curr_row][curr_col];
            }

            // if the player's piece is found after traversing over the opponent's piece(s), we know we will be flipping
            if(char_in_pos == player)
                flip_this_direction = true;

            // if we found out we should be flipping...
            if(flip_this_direction){
                // save current position
                curr_row = row + deltas[0];
                curr_col = col + deltas[1];
                char_in_pos = board[curr_row][curr_col];

                // traverse over the opponent's pieces, while saving the positions to the big list to be flipped later
                while(char_in_pos == otherPlayer){
                    //std::cout << "flipping [" << curr_row << ", " << curr_col << "]\n";
                    std::vector<int> disc = {curr_row, curr_col};
                    discs_to_flip.push_back(disc);
                    curr_row += deltas[0];
                    curr_col += deltas[1];

                    // save next character
                    char_in_pos = board[curr_row][curr_col];
                }

            }
        }
    }

    // after we've collecting the row/col of all discs to flipped, flip them to the current player's color/character
    for(auto pos : discs_to_flip)
        board[pos[0]][pos[1]] = player;
}

// a move "isFlippable" if it causes pieces to flip
bool isFlippable(char board[8][8], int row, int col, char player) {
    char otherPlayer = (player == 'b') ? 'w' : 'b';

    // Check all 8 surround positions
    int surroundingPosDeltas[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, // 3 positions above
                                      {0, -1}, {0, 1}, // 2 positions on same row
                                      {1, -1}, {1, 0}, {1, 1}}; // 3 positions below

    // for every delta of the surrounding positions
    for(auto deltas : surroundingPosDeltas){

        // skip if the position is off of game board
        if(row+deltas[0] > 7 || row+deltas[0] < 0 || col+deltas[1] > 7 || col+deltas[1] < 0){
            continue;
        }

        //std::cout << "deltas: [" << deltas[0] << ", " << deltas[1] << "]" << '\n';
        char char_in_pos = board[row+deltas[0]][col+deltas[1]]; // grab the character in that spot

        // if the character in this delta spot is the opponent's piece...
        if(char_in_pos == otherPlayer){
            // save spot's row and col #
            int curr_row = row + deltas[0];
            int curr_col = col + deltas[1];

            //std::cout << "Found other player at location: [" << curr_row << ", " << curr_col << "], " << char_in_pos << '\n';

            //continue along this delta trajectory until you stop seeing the opponent's pieces
            while(char_in_pos == otherPlayer){
                curr_row += deltas[0];
                curr_col += deltas[1];

                // check to see if new position is off board
                if(curr_row > 7 || curr_row < 0 || curr_col > 7 || curr_row < 0)
                    break;

                // save the next character
                char_in_pos = board[curr_row][curr_col];
            }

            // if the player's piece is seen after one (+more) of the opponent's pieces, the original move is a flippable one
            if(char_in_pos == player)
                return true;
        }
    }

    // if no flippable spot is found after checking all surrounding positions, the original move is not a flippable one
    return false;
}

// set board[row][col] to player's piece, and flip appropriate pieces
void makeMove(char (&board)[8][8], int row, int col, char player){
    //std::cout << "Updating row: " << row << " col: " << col << '\n';
    // set provided row/col position to the player's character piece
    board[row][col] = player;

    // flip discs from resulting move
    flip(board, row, col, player);
}

// used to algorithmically calculate legal moves belonging to passed-in player
std::vector<std::vector<int>> calculateLegalMoves(char board[8][8], char player) {

    // declare main move list
    std::vector<std::vector<int>> move_list;

    for(int i = 0; i < 8; ++i){
        for(int j = 0; j < 8; ++j){
            // first make sure the spot is empty
            if(board[i][j] == '-'){

                // check to see if placing a piece there will flip one (+more) of the opponent's pieces
                if(isFlippable(board, i, j, player)){

                    // if so, create a 2-element vector representative of the move and push it to the big move list
                    std::vector<int> move = {i, j};
                    move_list.push_back(move);
                }

            }
        }
    }

    return move_list;

}

// for a given board configuration, determine if a move is legal (searches through a previously generated movelist)
bool isLegalMove(char board[8][8], std::vector<std::vector<int>> move_list, int row, int col, char player) {
    std::vector<int> proposedMove = {row, col};
//    for (int i : proposedMove) {
//        std::cout << i << ' ';
//    }

    //This error should NOT occur, as the regex pattern validates the user's input
    if(row > 7 || row < 0 || col > 7 || col < 0)
        throw std::range_error{"isLegalMove()"};

    // Make sure position is empty
    if(board[row][col] != '-'){
        return false;
    }

    if (std::find(move_list.begin(), move_list.end(), proposedMove) != move_list.end()){
        return true;
    }

    return false;
}

// return a list of all the moves available to black
std::vector<std::vector<int>> getBlackLegalMoves(char board[8][8]) {
    return calculateLegalMoves(board, 'b');
}

// return a list of all the moves available to white
std::vector<std::vector<int>> getWhiteLegalMoves(char board[8][8]) {
    return calculateLegalMoves(board, 'w');
}

// for the passed-in player, print all legal moves (displayed on board update)
void printLegalMoves(char board[8][8], char player){
    if(player == 'b'){
        std::cout << "Black legal moves:\n";
        auto v = getBlackLegalMoves(board);
        for ( const auto &vec : v ) {
            std::cout << "(" << vec[0]  << "," << vec[1] << ")  ";
        }
        std::cout << std::endl;
    } else {
        std::cout << "White legal moves:\n";
        auto x = getWhiteLegalMoves(board);
        for ( const auto &vec : x ) {
            std::cout << "(" << vec[0]  << "," << vec[1] << ")  ";
        }
        std::cout << std::endl;
    }
}

// pass in a generated move list to "pretty print" them
void printLegalMoves(std::vector<std::vector<int>> move_list){
    for ( const auto &vec : move_list ){
        std::cout << "(" << vec[0]  << "," << vec[1] << ")  ";
    }
    std::cout << std::endl;
}

// overload the << operator to "pretty print" the board
std::ostream& operator<<(std::ostream& os, const char board[8][8]){
    std::cout << "   0  1  2  3  4  5  6  7\n";
    for(int i = 0; i < 8; ++i){
        std::cout << (i) << "  ";
        for (int j = 0; j < 8; ++j) {
            std::cout << board[i][j] << "  ";
        }
        std::cout << '\n';
    }
    return os;
}

// used to determine if the game is ended, makes sure at least 1 player has a move to make
bool isGameOver(char board[8][8]){
    return getBlackLegalMoves(board).empty() && getWhiteLegalMoves(board).empty();
}

// go through whole board, and count pieces of passed-in player
int getScore(char board[8][8], char player){
    int total = 0;
    for(int i = 0; i < 8; ++i)
        for(int j = 0; j < 8; ++j)
            if(board[i][j] == player)
                total += 1;

    return total;
}

// "pretty print" the winner of the game at the end of the game loop
void printWinner(char (&board)[8][8]){
    int white_total = getScore(board, 'w');
    int black_total = getScore(board, 'b');

    std::cout << "Black total: " << black_total << '\n';
    std::cout << "White total: " << white_total << '\n';
    if(black_total == white_total){
        std::cout << "TIE GAME\n";
        return;
    }

    std::cout << ((black_total > white_total) ? "Black" : "White") << " wins!\n";
}

// heursitic used to give value to varying states of the game
int heuristic(char board[8][8]){

    // intialize black and white total
    int b_total = 0;
    int w_total = 0;

    // factor in the amount of moves each player has
    b_total += getBlackLegalMoves(board).size();
    w_total += getWhiteLegalMoves(board).size();

    // factor in the amount of pieces each player has on the board
    b_total += getScore(board, 'b');
    w_total += getScore(board, 'w');

    // factor in the importance of all 4 corners
    if(board[0][0] == 'w'){
        w_total += 10;
    }
    if(board[0][0] == 'b'){
        b_total += 10;
    }
    if(board[7][0] == 'w'){
        w_total += 10;
    }
    if(board[7][0] == 'b'){
        b_total += 10;
    }
    if(board[0][7] == 'w'){
        w_total += 10;
    }
    if(board[0][7] == 'b'){
        b_total += 10;
    }
    if(board[7][7] == 'w'){
        w_total += 10;
    }
    if(board[7][7] == 'b'){
        b_total += 10;
    }

    // subtract white's total from black, let black be the maximizer
    return (b_total-w_total);
}

// a node which will be part of the game tree, main pieces of info include: state (board configuration) & associated value
struct Node
{
    Node ** children;
    int child_count;
    std::vector<std::vector<int>> move_list;
    char state[8][8];
    int val;
};

// method used to initialize a game tree (called everytime the AI has a turn)
Node * CreateTree(char board[8][8], int depth, char player)
{
    Node * node = new Node();

    // get the appropriate list moves
    node->move_list = (player == 'w') ? getWhiteLegalMoves(board) : getBlackLegalMoves(board);

    // keep a count of children for indexes later on
    node->child_count = node->move_list.size();

    // copy the passed in board state to the state of the current node
    std::memcpy(node->state, board, 8 * 8 * sizeof(char));

    // determine other player's character
    char other_player = (player == 'w') ? 'b' : 'w';

    // only create children if we're not too deep and this node should have children
    if (depth > 0 && node->child_count > 0) {
        // create an array of nodes as the children of the current node
        node->children = new Node * [node->child_count];

        // cycle through the children and create nodes for them
        for (int i = 0; i < node->child_count; ++i){
            char tmp_board[8][8];
            std::memcpy(tmp_board, board, 8 * 8 * sizeof(char));

            // must make the associating move first so a subtree of 'that' board configuration can be created
            makeMove(tmp_board, node->move_list[i][0], node->move_list[i][1], player);

            // turn the child into a tree itself
            node->children[i] = CreateTree(tmp_board, depth - 1, other_player);
        }
    } else {
        node->children = NULL;
    }

    return node;
}

// crucial minimax method for making smart AI choices (other methods may be added in the future)
int minimax(Node *position, int depth, int alpha, int beta, bool maximizing_player){

    // if we're at the final layer or this state is a dead sate, return static heurstic
    if(depth == 0 || isGameOver(position->state)){
        //std::cout<< "returning heursitic: " << heuristic(position->state) << '\n';
        return heuristic(position->state);
    }

    // if maximizing layer...
    if(maximizing_player){
        int max_eval = -9999999; // set max to worst case

        // for all of the children nodes, recursively call minimax
        // decrease the depth parameter with each call, so we can guarantee we will get to the base case above
        for(int i = 0; i < position->child_count; ++i){
            int eval = minimax(position->children[i], depth - 1, alpha, beta, false);
            max_eval = std::max(max_eval, eval); // update max if evaluation is >

            //update alpha appropriately, and check for eligibility of alpha prune
            alpha = std::max(alpha, eval);
            if(beta <= alpha) {
                if (DEBUG_MODE) {
                    std::cout << "DEBUG: PRUNED " << (position->child_count - (i+1)) << " children.\n";
                }
                break;
            }
        }
        position->val = max_eval; // store the max_eval in this node
        return max_eval;
    } else { // minimizing layer...
        int min_eval = 9999999; // set min to worst case
        for(int i = 0; i < position->child_count; ++i){
            int eval = minimax(position->children[i], depth -1, alpha, beta, true);
            min_eval = std::min(min_eval, eval); // update min if evaluation is <

            // update beta appropriately, and check for eligibility of beta prune
            beta = std::min(beta, eval);
            if(beta <= alpha)
                break;
        }
        position->val = min_eval; // store min_eval in this node
        return min_eval;
    }
}

// simplified minimax without alpha-beta pruning, similar to above
int minimax(Node *position, int depth, bool maximizing_player){
    //std::cout << "DEPTH = " << depth << '\n';
    if(depth == 0 || isGameOver(position->state)){
        //std::cout<< "returning heursitic: " << heuristic(position->state) << '\n';
        return heuristic(position->state);
    }

    if(maximizing_player){
        int max_eval = -9999999;
        for(int i = 0; i < position->child_count; ++i){
            int eval = minimax(position->children[i], depth - 1, false);
            max_eval = std::max(max_eval, eval);
        }
        position->val = max_eval;
        return max_eval;
    } else {
        int min_eval = 9999999;
        for(int i = 0; i < position->child_count; ++i){
            int eval = minimax(position->children[i], depth -1, true);
            min_eval = std::min(min_eval, eval);
        }
        position->val = min_eval;
        return min_eval;
    }
}

int main() {

    std::cout << "This CLI program is a playable Othello game, which consists of two players\n"
                 "('w' and 'b') competing for space on a 8x8 square grid. Flanking your opponent \n"
                 "with your pieces will cause their pieces to flip and become yours. If there exists\n"
                 "a move where you can flip 1 (or more) of your opponent's pieces, you must play it.\n"
                 "If no such move exists, you pass your turn to your opponent. Black always plays the\n"
                 "first move.\n\n"
                 "Make your moves to the grid by entering '<row #> <column #>' with numbers [0-7].\n"
                 "Good luck!\n\n";

    //**** Initialize Game Board *********
    char board[8][8];
    for(auto & i : board){
        for (char & j : i) {
            j = '-';
        }
    }

    board[3][3] = 'w'; board[3][4] = 'b';
    board[4][3] = 'b'; board[4][4] = 'w';
    //************************************

    int total_moves = 0;
    char player = 'b'; // black always goes first
    std::regex move_input_pattern("[0-7] [0-7]"); // regex for row/col input

    if(PLAY_AI){ // If playing the AI...

        std::regex player_selection_pattern("w|b"); // regex for w/b player selection
        std::cout << "Enter 'b' to play as black or 'w' to play as white: ";
        std::string selected_player;
        // loop until user makes a valid choice of player
        while(true)
        {
            getline(std::cin, selected_player);
            if(!std::regex_match(selected_player, player_selection_pattern)){
                std::cout << "\nInvalid input: \"Enter 'b' to be black or 'w' to be white. \n";
                continue;
            }
            break;
        }

        char player_char = selected_player[0];
        std::cout << "You have chosen to play as " << ((player_char == 'w') ? "white" : "black") << "!\n\n";

        // set AI as the opposite of what the player chose
        char ai_char = ((player_char == 'w') ? 'b' : 'w');

        // main game loop
        while(!isGameOver(board)){
            // calculate the move list of the current player
            std::vector<std::vector<int>> move_list = calculateLegalMoves(board, player);

            //************ TURN PASS CONDITIONS **********************
            if (player == 'b' && getBlackLegalMoves(board).empty()){
                //std::cout << "Black is out of moves, PASS to White.\n";
                player = 'w';
                continue;
            }

            if (player == 'w' && getWhiteLegalMoves(board).empty()){
                //std::cout << "White is out of moves, PASS to Black.\n";
                player = 'b';
                continue;
            }
            //*********************************************************

            int white_total = getScore(board, 'w');
            int black_total = getScore(board, 'b');

            std::cout << "Black total: " << black_total << '\n';
            std::cout << "White total: " << white_total << '\n';

            std::cout << board; // show board
            std::cout << '\n';
            if(player == player_char){
                printLegalMoves(board, player_char); // show possible moves

                std::string user_input;
                // loop until user provides a legal move in the correct row/col format
                while(true){
                    // Print input prompt
                    std::cout << ((player == 'w') ? "Your move (w): " : "Your move (b): ");
                    std::getline(std::cin, user_input);

                    if(!std::regex_match(user_input, move_input_pattern)){
                        std::cout << "\nInvalid input: Moves are inputted as '<row #> <column #>' with numbers [0-7].\n";
                        std::cout << "e.g. If you want to place your piece at row #1, column #2 input '1 2'.\n\n";
                        continue;
                    } else {
                        // user_input = [<some num>, " ", <some num>], nums will be at indices 0 and 2
                        // subtract '0's ascii value (48) from the user nums to get the real integer
                        int row = user_input[0] - '0';
                        int col = user_input[2] - '0';

                        try{
                            // if the inserted move is legal, make the move
                            if(isLegalMove(board, move_list, row, col, player)){
                                makeMove(board, row, col, player);
                            } else {
                                std::cout << "Illegal move! Try again.\n";
                                continue;
                            }
                        } catch(std::range_error& e){
                            std::cout << e.what() << " - attempted access to element outside of game board, modification after initial input";
                            return 1;
                        }
                        break;
                    }

                }
                // user has finished turn

            } else { // AI turn
                    auto gametree = CreateTree(board, MINIMAX_DEPTH, player); // game tree representing MINIMAX_DEPTH decisions
                    bool maximizer = (player == 'b') ? true : false;

                    // find optimal value
                    int optimial_val = minimax(gametree, MINIMAX_DEPTH, -99999999, 99999999, maximizer);
                    //int optimial_val = minimax(gametree, MINIMAX_DEPTH, maximizer);

                    if(DEBUG_MODE){
                        std::cout << "DEBUG: AI considered " << gametree->child_count << " initial moves for this board configuration.\n";
                        printLegalMoves(gametree->move_list);
                        for(int i = 0; i < gametree->child_count; ++i){
                            std::cout << "\t" << i << "th node's heuristic value = " << gametree->children[i]->val << '\n';
                        }
                        std::cout << '\n';
                    }

                    //std::cout << "Optimal val: " << optimial_val << '\n';
                    // loop through children of root node to find the node with the optimal value
                    for(int i = 0; i < gametree->child_count; ++i){
                        //std::cout << gametree->children[i]->val << '\n';
                        if(gametree->children[i]->val == optimial_val){
                            bool same_config = true;
                            for(int j = 0; j < 7; ++j){
                                for(int k = 0; k < 7; ++k){
                                    if(gametree->children[i]->state[j][k] != board[j][k])
                                        same_config = false;
                                }
                            }
                            //std::cout << "the " << i << "th child of gametree has the optimizal value.\n";

                            // copy this optimial choice of node's state into the main game board
                            if(!same_config)
                                std::memcpy(board, gametree->children[i]->state, 8 * 8 * sizeof(char));
                            else{ // if no good move for ai, just pick the first move from the legal move list
                                makeMove(board, move_list[0][0], move_list[0][1], player);
                            }
                            break;
                        }
                    }
            }

            total_moves += 1;
            //std::cout << '\n' << gb; // Show board

            // Switch players
            player = (player == 'w') ? 'b' : 'w';

        }

    } else { // Playing 2 player game
        while(!isGameOver(board)){
            std::vector<std::vector<int>> move_list = calculateLegalMoves(board, player);

            std::cout << ((player == 'w') ? "White's Movelist: " : "Black's Movelist: \n");
            printLegalMoves(move_list);
            std::cout << board; // Show board

            if (player == 'b' && getBlackLegalMoves(board).size() == 0){
                //std::cout << "Black is out of moves, PASS to White.\n";
                player = 'w';
                continue;
            }

            if (player == 'w' && getWhiteLegalMoves(board).size() == 0){
                //std::cout << "White is out of moves, PASS to Black.\n";
                player = 'b';
                continue;
            }

            // Print input prompt
            std::cout << ((player == 'w') ? "White's Move: " : "Black's Move: ");

            std::string user_input;
            std::getline(std::cin, user_input);
            //std::cout << "You entered: " << user_input << '\n';

            if(!std::regex_match(user_input, move_input_pattern)){
                std::cout << "\nInvalid input: Moves are inputted as '<row #> <column #>' with numbers [1-8].\n";
                std::cout << "e.g. If you want to place your piece at row #1, column #2 input '1 2'.\n\n";
                continue;
            }

            // user_input = [<some num>, " ", <some num>], nums will be at indices 0 and 2
            // subtract '0's ascii value (48) from the user nums to get the real integer
            int row = user_input[0] - '0';
            int col = user_input[2] - '0';

            try{
                if(isLegalMove(board, move_list, row, col, player)){
                    makeMove(board, row, col, player);
                } else {
                    std::cout << "Illegal move! Try again.\n";
                    continue;
                }
            } catch(std::range_error& e){
                std::cout << e.what() << " - attempted access to element outside of game board, modification after initial input";
                return 1;
            }

            total_moves += 1;
            int white_total = getScore(board, 'w');
            int black_total = getScore(board, 'b');

            std::cout << "Black total: " << black_total << '\n';
            std::cout << "White total: " << white_total << "\n\n";

            // Switch players
            player = (player == 'w') ? 'b' : 'w';
        }

    }

    std::cout << board; // Show final board
    printWinner(board);
    return 0;
}
