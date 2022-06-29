/* Program to print and play checker games. */


#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <assert.h>

/* Definitions ------------------------------------------------------*/

// definitions relating to the board
#define BOARD_SIZE          8       // board size
#define ROWS_WITH_PIECES    3       // number of initial rows with pieces
#define CELL_EMPTY          '.'     // empty cell character
#define CELL_BPIECE         'b'     // black piece character
#define CELL_WPIECE         'w'     // white piece character
#define CELL_BTOWER         'B'     // black tower character
#define CELL_WTOWER         'W'     // white tower character
#define COST_PIECE          1       // one piece cost
#define COST_TOWER          3       // one tower cost
#define TREE_DEPTH          3       // minimax tree depth
#define COMP_ACTIONS        10      // number of computed actions

// definitions relating to actions
#define B_ACTION            1       //black's action
#define W_ACTION            0       //white's action
#define NE                  1       //North-East direction
#define SE                  2       //South-East direction
#define SW                  3       //South-West direction
#define NW                  4       //North-West direction
#define MAX_DISTANCE        2       //max distance a piece can move in one turn
#define MOVE_DISTANCE       1       //moving distance of a piece (not capture)

// errors, error messages, and legal moves
#define ERROR_MSG1          "ERROR: Source cell is outside of the board.\n"
#define ERROR_MSG2          "ERROR: Target cell is outside of the board.\n"
#define ERROR_MSG3          "ERROR: Source cell is empty.\n"
#define ERROR_MSG4          "ERROR: Target cell is not empty.\n"
#define ERROR_MSG5          "ERROR: Source cell holds opponent's piece/tower.\n"
#define ERROR_MSG6          "ERROR: Illegal action.\n"
#define ERROR_1             1
#define ERROR_2             2
#define ERROR_3             3
#define ERROR_4             4
#define ERROR_5             5
#define ERROR_6             6
#define LEGAL               7       //Move is legal

// command letters
#define COMMAND_P           'P'
#define COMMAND_A           'A'

// separators for printing and formatting
#define SEPARATOR_MAIN      "=====================================\n"
#define HEADER              "     A   B   C   D   E   F   G   H\n"
#define BOARD_SEPARATOR     "   +---+---+---+---+---+---+---+---+\n"

// useful row numbers and column numbers
#define ROW_ONE             1
#define ROW_TWO             2
#define ROW_THREE           3
#define ROW_SIX             6
#define ROW_SEVEN           7
#define ROW_EIGHT           8
#define COL_ONE             1
#define COL_EIGHT           8

// other definitions for computation
#define FOUND               1
#define NOT_FOUND           0
#define TRUE                1
#define FALSE               0
#define WIN                 0
#define NOT_WIN             1
#define DEPTH_0             0
#define DEPTH_3             3
#define CONVERSION          64      //Conversion from letters (A-Z)to numbers


/* type definitions ------------------------------ -------------------------*/

typedef unsigned char board_t[BOARD_SIZE][BOARD_SIZE];  
// In my program, board[row-1][col-1] will describe the squares on the board

// Data stored in each node of the minimax tree
typedef struct {
    int        action;              //white or black action
    int        leaf_cost;        
    int        depth;
    int        s_row;               //source row
    int        s_col;               //source column
    int        t_row;               //target row
    int        t_col;               //target column
    board_t    poss_board;          //possible board state
} data_t;

// Node of the minimax tree
typedef struct node node_t;
struct node {
    data_t    data;
    node_t    *head_ND;             //head of the child node of next depth
    node_t    *foot_ND;             //foot of the child node of next depth
    node_t    *next_CD;             //next node in the current depth
};


/* function prototypes ------------------------------------------------------*/
char stage_0(board_t board, int *action);
void initialise_board(board_t board);
void print_board(board_t board);
int  board_cost(board_t board);
int  is_promotion(board_t board);
int is_legal_action(board_t board, int s_row, int s_col, 
                    int t_row, int t_col, int action);
node_t *make_empty_tree(void);
node_t *insert_at_foot(node_t *node, data_t *info);
void copy_board(board_t start_board, board_t copied_board);
data_t *get_action(data_t *data, int s_row, int s_col, int direction);
node_t *fill_tree(node_t *tree);
void calculate_leaf_costs(node_t *tree);
int  stage_1(board_t board, int action);
void recursive_free_tree(node_t *tree);

/* main program controls all the action -------------------------------------*/
int
main(int argc, char *argv[]) {
    board_t board; char command; 
    int *action, i;                //action keeps track of the action number
    
    action = (int*)malloc(sizeof(*action));
    *action = 0;
    
    //initialise checkers board, and print
    initialise_board(board);
    printf("BOARD SIZE: 8x8\n");
    printf("#BLACK PIECES: 12\n");
    printf("#WHITE PIECES: 12\n");
    print_board(board);
    
    //perform stage_0, and pick up the command after stage_0 is done
    command = stage_0(board, action);
    
    //if command is 'A', perform stage_1. 
    if (command==COMMAND_A) {
        stage_1(board, *action);
    }
    
    //if command is 'P', perform stage_2. Also free the memory for 'action'
    if (command==COMMAND_P) {
        for (i=0; i<COMP_ACTIONS; i++) {
            if (stage_1(board, *action) == NOT_WIN) {
                *action += 1;
            } else {
                free(action);
                return EXIT_SUCCESS;
            }
        }
    }
    
    //stage 2 not executed. So free memory for 'action'
    free(action);
    return EXIT_SUCCESS;           
}

/* --------------------------------------------------------------------------*/

/* The function stage_0:
   -- Reads the input data
   -- Analyses the inputs to check for errors. 
   -- Prints the action, board cost, and the board if there are no errors.
   
   Also, this function will pickup on the command letter and return it
*/
char
stage_0(board_t board, int *action) {
    char s_col, t_col;          //source column and target column characters
    unsigned char *source_cell; //pointer to source cell on the board
    unsigned char *target_cell; //pointer to target cell on the board
    int s_row, t_row;           //source row and target row
    int s_colint, t_colint;     //source and target column, converted to numbers
    int error_num;              //describes the error number
    
    // Reading the input
    while(scanf("%c%d-%c%d ", &s_col, &s_row, &t_col, &t_row) == 4) {
    
        //convert column characters to integers
        s_colint = s_col - CONVERSION;  //e.g. column 'A' is converted to 1
        t_colint = t_col - CONVERSION;
        
        //check whether move is legal. 
        //If not legal, print error messages and terminate program
        *action += 1;
        error_num = is_legal_action(board, s_row, s_colint, 
                                    t_row, t_colint, *action);
        if (error_num == ERROR_1) {
            printf("%s", ERROR_MSG1);
        } else if (error_num == ERROR_2) {
            printf("%s", ERROR_MSG2);
        } else if (error_num == ERROR_3) {
            printf("%s", ERROR_MSG3);
        } else if (error_num == ERROR_4) {
            printf("%s", ERROR_MSG4);
        } else if (error_num == ERROR_5) {
            printf("%s", ERROR_MSG5);
        } else if (error_num == ERROR_6) {
            printf("%s", ERROR_MSG6);
        }
        if (error_num != LEGAL) {
            exit(0);
        }
        
        //from now, we know that the move is legal
        source_cell = &(board[s_row-1][s_colint-1]);
        target_cell = &(board[t_row-1][t_colint-1]);
        printf("%s", SEPARATOR_MAIN);
        
        //check who's action it is and print required output
        if (*source_cell == CELL_BPIECE || 
            *source_cell == CELL_BTOWER) {
            //must be black's action
            printf("BLACK ACTION #%d: %c%d-%c%d\n", *action, s_col, s_row, 
                   t_col, t_row);
        } else {
            //must be white's action
            printf("WHITE ACTION #%d: %c%d-%c%d\n", *action, s_col, s_row, 
                   t_col, t_row);    
        }
        
        //make the move on the board
        if (abs(s_colint-t_colint)==2 && abs(s_row-t_row)==2) {
            //this must be a capture move
            *target_cell = *source_cell;
            *source_cell = CELL_EMPTY;
            board[(s_row+t_row)/2 - 1][(s_colint+t_colint)/2 - 1] = CELL_EMPTY; 
        } else {
            //this is just a regular move
            *target_cell = *source_cell;
            *source_cell = CELL_EMPTY;
        }
        
        //check whether a piece should be promoted to a tower. 
        is_promotion(board);
        
        printf("BOARD COST: %d\n", board_cost(board));
        print_board(board);
        
        //reset value of s_col, to prevent possible confusion with the command
        s_col = '0';
    }
    
    //if there is a command at the end, s_col will pick up on it
    return s_col;
}

/* --------------------------------------------------------------------------*/

/* Stage_1 function:
   -- Uses the minimax decision rule to compute the next action for the player
   -- Performs the next action and prints it
   
   Also, this program returns WIN if the player has already won when the next 
   action is being computed. It returns NOT_WIN if the player has not won.
*/
int
stage_1(board_t board, int action) {
    node_t *tree;             // points to the root of the data structure
    node_t *curr;             // points to current node
    node_t *chosen_child;     // points to the node with the final chosen board 
    int min, max;             // minimum and maximum board costs
    int s_row, s_col, t_row, t_col;
    
    //Create the data structure 
    tree = make_empty_tree();
    
    //Check which player should make the next action
    if ((action+1)%2 == B_ACTION) {
        //black to move
        tree->data.action = B_ACTION;
    } else {
        //white to move
        tree->data.action = W_ACTION;
    }
    
    //initialise some data in the tree
    tree->data.depth = DEPTH_0;
    copy_board(board, tree->data.poss_board);
    
    //Compute all possible board states in the next 3 turns.
    //Then calculate the leaf costs based on the minimax decision rule
    fill_tree(tree);  
    calculate_leaf_costs(tree);
    
    //Check if the next depth (next action) exists. If not, a player has won.
    if (tree->head_ND == NULL) {
        if (tree->data.action == W_ACTION) {
            printf("BLACK WIN!\n"); 
        } else {
            printf("WHITE WIN!\n"); 
        }
        //free the tree and set it to NULL
        free(tree);
        tree = NULL;
        return WIN;
    }
    
    //Next depth must exist. Find out what the best action is by comparing the
    //board costs of the possible boards in depth 1
    curr = tree->head_ND;
    if (tree->data.action == W_ACTION) {
        //white to move. We want the action with the minimum board cost
        min = curr->data.leaf_cost;
        chosen_child = curr;
        while (curr) {
            if (curr->data.leaf_cost < min) {
                min = curr->data.leaf_cost;
                chosen_child = curr;
            }
            curr = curr->next_CD;
        }
    } else {
        //black to move. We want the action with the maximum board cost
        max = curr->data.leaf_cost;
        chosen_child = curr;
        while (curr) {
            if (curr->data.leaf_cost > max) {
                max = curr->data.leaf_cost;
                chosen_child = curr;
            }
            curr = curr->next_CD;
        }
    }
    
    //found the best action (chosen_child). Now set the source and target
    s_row = chosen_child->data.s_row;
    t_row = chosen_child->data.t_row;
    s_col = chosen_child->data.s_col;
    t_col = chosen_child->data.t_col;
    
    //print the action and the board
    printf("%s", SEPARATOR_MAIN);
    if (tree->data.action == B_ACTION) {
        //black's actions
        printf("*** BLACK ACTION #%d: %c%d-%c%d\n", action+1, s_col+CONVERSION, 
               s_row, t_col+CONVERSION, t_row);
    } else {
        //white's action
        printf("*** WHITE ACTION #%d: %c%d-%c%d\n", action+1, s_col+CONVERSION, 
               s_row, t_col+CONVERSION, t_row);
    }
    printf("BOARD COST: %d\n", board_cost(chosen_child->data.poss_board));
    print_board(chosen_child->data.poss_board);
    
    //before freeing the memory, copy the new board into the old board
    copy_board(chosen_child->data.poss_board, board);
    recursive_free_tree(tree);
    tree = NULL;
    
    return NOT_WIN;
}

/* --------------------------------------------------------------------------*/

/* checks whether or not there is a piece that is supposed to be promoted in 
   the current board state. If there is, then promote the piece on the board.
   Returns TRUE if something is promoted, FALSE if not.
   This function assumes that pieces are on legal squares. 
*/
int
is_promotion(board_t board) {
    int j;            //j+1 would be the column numbers from 1 to 8
    
    //first, check if a black piece made it to row 1 
    for (j=0; j<BOARD_SIZE; j++) {
        if (board[ROW_ONE-1][j]==CELL_BPIECE) {
            board[ROW_ONE-1][j] = CELL_BTOWER;
            return TRUE;
        }
    }
    //then, check if a white piece made it to row 8
    for (j=0; j<BOARD_SIZE; j++) {
        if (board[ROW_EIGHT-1][j]==CELL_WPIECE) {
            board[ROW_EIGHT-1][j] = CELL_WTOWER;
            return TRUE;
        }
    }
    
    //no promotions found
    return FALSE;
}

/* --------------------------------------------------------------------------*/

/* This function initialises the checkers board at the start of the game */
void
initialise_board(board_t board) {
    int i, j;     //i+1 would give the row number. j+1 would give column number
    
    //fill board with empty cells
    for (i=0; i<BOARD_SIZE; i++) {
        for (j=0; j<BOARD_SIZE; j++) {
            board[i][j] = CELL_EMPTY;
        }
    }
    
    //initialise pieces at the start of the game
    for (i=0; i<BOARD_SIZE; i++) {
        if (i+1==ROW_ONE || i+1==ROW_THREE) { 
            //white piece alternates every two columns, starting at column 2
            for (j=1; j<BOARD_SIZE; j+=2) {
                board[i][j] = CELL_WPIECE;
            }
        }
        if (i+1==ROW_TWO) {
            //white piece alternates, starting at column 1
            for (j=0; j<BOARD_SIZE; j+=2) {
                board[i][j] = CELL_WPIECE;
            }
        }
        if (i+1==ROW_SIX || i+1==ROW_EIGHT) {
            //black piece alternates, starting at column 1
            for (j=0; j<BOARD_SIZE; j+=2) {
                board[i][j] = CELL_BPIECE;
            }
        }
        if (i+1==ROW_SEVEN) {
            //black piece alternates, starting at column 2
            for (j=1; j<BOARD_SIZE; j+=2) {
                board[i][j] = CELL_BPIECE;
            }
        }
    }
    return;
}

/* --------------------------------------------------------------------------*/

/* Prints the current board*/
void
print_board(board_t board) {
    int i, j;    //again, i+1 is the row number, j+1 is the column number (1-8)
    
    printf("%s", HEADER);
    printf("%s", BOARD_SEPARATOR);
    //print board with some formatting
    for (i=0; i<BOARD_SIZE; i++) {
        printf(" %d |", i+1);
        for (j=0; j<BOARD_SIZE; j++) {
            printf(" %c |", board[i][j]);
        }
        printf("\n%s", BOARD_SEPARATOR);
    }
    return;
}

/* --------------------------------------------------------------------------*/

/* Calculates the current board cost using the formula: 3B + b - 3W - w */
int
board_cost(board_t board) {
    int i, j, cost=0;    //i+1 is the row number, j+1 is the column number
    
    for (i=0; i<BOARD_SIZE; i++) {
        for (j=0; j<BOARD_SIZE; j++) {
            if (board[i][j]==CELL_BPIECE) {
                cost += COST_PIECE;
            } else if (board[i][j]==CELL_WPIECE) {
                cost -= COST_PIECE;
            } else if (board[i][j]==CELL_BTOWER) {
                cost += COST_TOWER;
            } else if (board[i][j]==CELL_WTOWER) {
                cost -= COST_TOWER;
            }
        }
    }
    return cost;
}

/* --------------------------------------------------------------------------*/

/* This function takes the current board state, the coordinates of the source
   cell and the target cell, as well as the current action number. Using these
   values, it determines whether or not a move/capture is legal. 
   If the action is not legal, it will return the corresponding error number.
*/
int
is_legal_action(board_t board, int s_row, int s_col, int t_row, int t_col, int action) {
    char cell_captured, source_cell, target_cell;
    
    //1. Source cell is outside of board
    if (s_row<ROW_ONE || s_row>ROW_EIGHT || s_col<COL_ONE || s_col>COL_EIGHT) {
        return ERROR_1;
    }
    
    //2. Target cell is outside of board
    if (t_row<ROW_ONE || t_row>ROW_EIGHT || t_col<COL_ONE || t_col>COL_EIGHT) {
        return ERROR_2;
    }
    
    source_cell = board[s_row-1][s_col-1];
    target_cell = board[t_row-1][t_col-1];
    //3. Source cell is empty
    if (source_cell==CELL_EMPTY) {
        return ERROR_3;
    }
    
    //4. Target cell is not empty
    if (target_cell!=CELL_EMPTY) {
        return ERROR_4;
    }
    
    //5. Source cell holds opponent's piece/tower
    if ((action%2==W_ACTION && source_cell==CELL_BPIECE) || 
        (action%2==W_ACTION && source_cell==CELL_BTOWER) || 
        (action%2==B_ACTION && source_cell==CELL_WPIECE) || 
        (action%2==B_ACTION && source_cell==CELL_WTOWER)) { 
        return ERROR_5;
    }
    
    //6. Other illegal actions
    // a) Piece does not move diagonally
    if (abs(s_row-t_row) != abs(s_col-t_col)) {
        return ERROR_6;
    }
    // b) Piece jumps too far (greater than a distance of 2)
    if (abs(s_row-t_row)>MAX_DISTANCE || abs(s_col-t_col)>MAX_DISTANCE) {
        return ERROR_6;
    }
    // c) Piece captures player's own piece, or captures nothing
    if (abs(s_row-t_row)==MAX_DISTANCE && abs(s_col-t_col)==MAX_DISTANCE) {
        //this is a capture move
        cell_captured = board[(s_row+t_row)/2 - 1][(s_col+t_col)/2 - 1];
        if (cell_captured == CELL_EMPTY ||
            (action%2 == W_ACTION && cell_captured == CELL_WPIECE) ||
            (action%2 == W_ACTION && cell_captured == CELL_WTOWER) ||
            (action%2 == B_ACTION && cell_captured == CELL_BPIECE) ||
            (action%2 == B_ACTION && cell_captured == CELL_BTOWER)) {
                return ERROR_6;
            }
    }
    // d) Pieces moving backwards/capturing backwards
    if (source_cell == CELL_WPIECE) {
        if (s_row > t_row) {
            return ERROR_6;
        }
    } 
    if (source_cell == CELL_BPIECE) {
        if (t_row > s_row) {
            return ERROR_6;
        }
    }
    
    // No errors found, must be a legal move
    return LEGAL;
}

/* --------------------------------------------------------------------------*/

/* Creates an empty data tree, and returns a pointer to the root node */
node_t
*make_empty_tree(void) {
    node_t *root_node;
    root_node = (node_t*)malloc(sizeof(*root_node));
    root_node->head_ND = root_node->foot_ND = root_node->next_CD = NULL;
    return root_node;
}

/* --------------------------------------------------------------------------*/

/* Creates a new node, and inserts the info into that new node. Then, insert
   this new node into the foot of the next depth of 'node'. 
*/
node_t 
*insert_at_foot(node_t *node, data_t *info) {
    node_t *new;
    
    //make space for the new node and initialise some values/pointers
    new = (node_t*)malloc(sizeof(*new));
    new->data = *info;
    new->head_ND = new->foot_ND = new->next_CD = NULL;
    
    if (node->foot_ND == NULL) {
        //this is the first insertion into the next depth
        node->head_ND = node->foot_ND = new;
    } else {
        //not the first insertion into the next depth
        node->foot_ND->next_CD = new;
        node->foot_ND = new;  
    }
    return node;
}

/* --------------------------------------------------------------------------*/

/* Copies all the cells of 'start_board' into the 'copied board'. */
void
copy_board(board_t start_board, board_t copied_board) {
    int i, j;        //i+1 is the row number, j+1 is the column number
    
    for (i=0; i<BOARD_SIZE; i++) {
        for (j=0; j<BOARD_SIZE; j++) {
            copied_board[i][j] = start_board[i][j];
        }
    }
    return;
}

/* --------------------------------------------------------------------------*/

/* - Takes the data about a current turn, and the coordinates of the source 
     cell, and a particular direction.
   - Then, checks whether a move or a capture is possible for the piece in the 
     source cell, in the particular direction given. 
   - If action is possible, then create a new set of data for this action, and
     return the pointer to that new data. If no action is possible, return NULL.
   - This function also calculates the board cost, if the children board is in
     depth 3.
*/
data_t
*get_action(data_t *data, int s_row, int s_col, int direction) {
    int t_row, t_col;        //coordinates of the target cell
    int action_found=NOT_FOUND, is_capture=FALSE, is_move=FALSE;
    data_t *child_data;      //pointer to the new data set
    unsigned char *source_cell, *target_cell, *captured; 
    
    //check the given direction for moves (not captures) fist
    if (direction == NE) {
        t_row = s_row - MOVE_DISTANCE;
        t_col = s_col + MOVE_DISTANCE;
    } else if (direction == SE) {
        t_row = s_row + MOVE_DISTANCE;
        t_col = s_col + MOVE_DISTANCE;
    } else if (direction == SW) {
        t_row = s_row + MOVE_DISTANCE;
        t_col = s_col - MOVE_DISTANCE;
    } else {
        t_row = s_row - MOVE_DISTANCE;
        t_col = s_col - MOVE_DISTANCE;
    }
    if (is_legal_action(data->poss_board, s_row, s_col, 
        t_row, t_col, data->action) == LEGAL) {
        //legal action found. And it is a move, not a capture
        action_found = FOUND;
        is_move = TRUE;
    }
    
    //next, if no moves were found, check the given direction for captures
    if (direction == NE && !action_found) {
        t_row = s_row - MAX_DISTANCE;
        t_col = s_col + MAX_DISTANCE;
    } else if (direction == SE && !action_found) {
        t_row = s_row + MAX_DISTANCE;
        t_col = s_col + MAX_DISTANCE;
    } else if (direction == SW && !action_found) {
        t_row = s_row + MAX_DISTANCE;
        t_col = s_col - MAX_DISTANCE;
    } else if (direction == NW && !action_found) {
        t_row = s_row - MAX_DISTANCE;
        t_col = s_col - MAX_DISTANCE;
    }
    if (is_legal_action(data->poss_board, s_row, s_col,
        t_row, t_col, data->action) == LEGAL) {
        //legal action found. It is a capture move
        action_found = FOUND;
        is_capture = TRUE;
    }
    
    if (action_found) {
        //allocate space for child_data. And copy the board across
        child_data = (data_t*)malloc(sizeof(data_t));
        copy_board(data->poss_board, child_data->poss_board); 
        source_cell = &(child_data->poss_board[s_row-1][s_col-1]);
        target_cell = &(child_data->poss_board[t_row-1][t_col-1]);
        
        //make move/capture. Promote the piece to tower if needed
        if (is_move) {
            *target_cell = *source_cell;
            *source_cell = CELL_EMPTY;
        } else if (is_capture) {
            captured = &(child_data->poss_board[(s_row+t_row)/2-1][(s_col+t_col)/2-1]);
            *target_cell = *source_cell;
            *captured = CELL_EMPTY;
            *source_cell = CELL_EMPTY;
        }
        is_promotion(child_data->poss_board);
        
        //make other changes for the child_data, as it describes the next turn
        if (data->action == W_ACTION) {
            //was white's move. Next turn will be black's move
            child_data->action = B_ACTION;
        } else {
            //was black's move. Next turn will be white's move
            child_data->action = W_ACTION;
        }
        child_data->depth = data->depth + 1;
        
        //store the coordinates of the source cell and the target cell
        child_data->s_row = s_row;
        child_data->s_col = s_col;
        child_data->t_row = t_row;
        child_data->t_col = t_col;
        
        //if child_data is in depth 3, calculate the board cost as well
        if (child_data->depth == DEPTH_3) {
            child_data->leaf_cost = board_cost(child_data->poss_board);
        }
        
        return child_data;
    }
    
    // No action found
    return NULL;
}

/* --------------------------------------------------------------------------*/

/* Takes a node of the tree, and using the data stored in that node, compute 
   all the possible actions for the next 3 moves. Store these possible actions 
   into the tree.
*/
node_t
*fill_tree(node_t *tree) {
    int i, j;                //again, i+1 is the row, j+1 is the column
    int direction, row, col;
    data_t *child_data;      //ptr to data that stores the next possible action
    
    if (tree->data.depth == DEPTH_3) {
        //do nothing
        return NULL;
    }
    
    //not depth 3, can look for possible actions, following row major order
    for (i=0; i<BOARD_SIZE; i++) {
        for (j=0; j<BOARD_SIZE; j++) {
        
            //now looking at each cell on the board
            row = i + 1;
            col = j + 1;
            if (tree->data.poss_board[row-1][col-1] == CELL_EMPTY) {
                //cell is empty, can skip this cell
                continue;
            }
            
            //Cell is not empty, check each direction for actions
            for (direction=NE; direction<=NW; direction++) {
                child_data = get_action(&tree->data, row, col, direction);
                if (child_data != NULL) {
                    //found possible action
                    insert_at_foot(tree, child_data);
                    free(child_data);
                    child_data = NULL;
                    //recursively call the function again for the next depth
                    fill_tree(tree->foot_ND);
                }
            }
        }
    }
    return tree;
}

/* --------------------------------------------------------------------------*/

/* Uses the minimax decision rule to calculate leaf costs for boards from 
   depth 2, upwards to depth 0. 
*/
void
calculate_leaf_costs(node_t *tree) {
    node_t *curr;
    int max, min;
   
    //if at depth 3, don't need to do anything as the cost was already found. 
    if (tree->data.depth == 3) {
        return;
    }
    
    //Not depth 3. Check if the next action exists
    if (tree->head_ND == NULL) {
        //next action does not exist. A player wins here
        if (tree->data.action == W_ACTION) {
            //white has no action. So cost is INT_MAX
            tree->data.leaf_cost = INT_MAX;
        } else {
            //black has no action. So cost is INT_MIN
            tree->data.leaf_cost = INT_MIN;
        }
        return;
    }
    
    //Now we know that next depth exists
    curr = tree->head_ND;
    while (curr) {
        //recursive call to function, to go to the deepest nodes first
        calculate_leaf_costs(curr);
        curr = curr->next_CD;
    }
    
    //walk along the next depth to propagate leaf cost upwards
    if (tree->data.action == B_ACTION) {
        //black's action, want to find max cost
        curr = tree->head_ND;
        max = curr->data.leaf_cost;
        while (curr) {
            if (curr->data.leaf_cost > max) {
                max = curr->data.leaf_cost;
            }
            curr = curr->next_CD;
        }
        tree->data.leaf_cost = max;
    }
    if (tree->data.action == W_ACTION) {
        //white's action, want to find min cost
        curr = tree->head_ND;
        min = curr->data.leaf_cost;
        while (curr) {
            if (curr->data.leaf_cost < min) {
                min = curr->data.leaf_cost;
            }
            curr = curr->next_CD;
        }
        tree->data.leaf_cost = min;
    }
    
    return;
}

/* --------------------------------------------------------------------------*/

/* Frees the memory space allocated for the tree. */
void
recursive_free_tree(node_t *tree) {
    node_t *curr, *prev;
    
    curr = tree->head_ND;
    while (curr) {
        prev = curr;
        curr = curr->next_CD;
        recursive_free_tree(prev);
    }
    free(tree);
    return;
}

/* THE END -------------------------------------------------------------------*/

