/* Name: Nicholas Gibbs
 * GNumber: 01482635
 * Section: 262-001
 * */

#include "listnode.h"
#include "exec.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

//struct to hold history string and id
typedef struct history_item {
	int id;
	char * str;
} Item;

void get_command(char * input, char *** args, Item ** history, ListNode ** list);
void clear_history(Item ** history);

//function to reset all chars of the input to null characters
void clear_input(char * input) {
	if (input == NULL) {
		return;
	}
	//iterate and set each to null until all characters are null
	for (int i = 0; input[i] != '\0'; i++) {
		input[i] = '\0';
	}
}

//function to dealloc any strings passed as args
void clear_args(char *** args) {
	if (args == NULL) {
		return;
	}
	//set all previous arg strings to null and dealloc for future uses
	for (int i = 0; args[0][i] != NULL; i++) {
                free(args[0][i]);
		args[0][i] = NULL;
        }
	//free pointer containing pointers to argument strings
        free(args[0]);
	args[0] = NULL;
}

//gets the user input from stdin and assigns it to input variable
int get_input(char * input) {
	char buffer[10000];
	printf("262$");
	fflush(stdout);
	//check if end of file
	if (fgets(buffer, 10000, stdin) == NULL) {
		//return 1 if end of file is reached
		return 1;
	}
	//copy buffer to input
	strcpy(input, buffer);
	return 0;
}

//function to determine if each command has the proper number of arguments
int check_args(char *** args) {
	//gets the command name
	char * name = args[0][0];
	//all the following check if the name matches and if not, prints an error message to stderr
	//returns 0 if correct number of arguments
	//returns -1 if not enough arguments provided
	//returns 1 if too many arguments provided
	if (strcmp(name, "quit") == 0) {
		return 0;
	} else if (strcmp(name, "cd") == 0) {
		if (args[0][2] != NULL) {
			fprintf(stderr, "error: too many arguments provided\n");
			return 1;
		}
		return 0;
	} else if (strcmp(name, "history") == 0) {
		if (args[0][2] != NULL) {
			fprintf(stderr, "error: too many arguments provided\n");
			return 1;
		}
		return 0;
	} else if (strcmp(name, "new") == 0) {
		if (args[0][1] == NULL) {
			fprintf(stderr, "error: too few arguments provided\n");
			return -1;
		}
		return 0;
	} else if (strcmp(name, "list") == 0) {
		if (args[0][1] != NULL) {
			fprintf(stderr, "error: too many arguments provided\n");
			return 1;
		}
		return 0;
	} else if (strcmp(name, "open") == 0) {
		if (args[0][1] == NULL || args[0][2] == NULL) {
			fprintf(stderr, "error: incorrect number of arguments provided\n");
			return -1;
		} else if (args[0][3] != NULL) {
			fprintf(stderr, "error: incorrect number of arguments provided\n");
			return 1;
		}
		return 0;
	} else if (strcmp(name, "execute") == 0) {
		if (args[0][1] == NULL) {
			fprintf(stderr, "error: incorrect number of arguments provided\n");
			return -1;
		} else if (args[0][2] != NULL) {
			fprintf(stderr, "error: incorrect number of arguments provided\n");
			return 1;
		}
		return 0;
	} else if (strcmp(name, "remove") == 0) {
		if (args[0][1] == NULL) {
			fprintf(stderr, "error: too few arguments provided\n");
			return -1;
		} else if (args[0][2] != NULL) {
			fprintf(stderr, "error: too many arguments provided\n");
			return 1;
		}
		return 0;
	}
	return 0;
}

//parses input from string to seperate arguments and assigns each argument into args variable
void parse_input(char * input, char *** args) {
	//temp is the size of the temporary 'arg' pointer which holds the current argument as a string
	int temp = 0;
	//counter to count the number of current arguments
	int counter = 0;
	//arg to hold the current argument being parsed
	char * arg = NULL;
	//iterate until end of input
	for (int i = 0; input[i] != '\0'; i++) {
		//allows multiple whitespace characters betwee arguments
		if (input[i] == ' ' || input[i] == '\n' || input[i] == '\t') {
			if (input[i + 1] == ' ' || input[i + 1] == '\n' || input[i + 1] == '\t') {
				continue;
			}
		}
		temp++;
		//adjust size to add a new character
		arg = realloc(arg, temp);
		if (input[i] == ' ' || input[i] == '\0' || input[i] == '\n') {
			//current argument number goes up
			counter++;
			//realloc args 2d array to contain an additional argument string
			args[0] = realloc(args[0], counter * sizeof(char *));
			args[0][counter - 1] = NULL;
			//realloc argument string to the size of 'arg', which contains the current argument
			args[0][counter - 1] = realloc(args[0][counter - 1], temp);
			arg[temp - 1] = '\0';
			//copy the temporary argument holder into 'args' variable
			strcpy(args[0][counter - 1], arg);
			temp = 0;
			//check if end of input
			if (input[i + 1] == '\0') {
				//realloc additional space for null
				args[0] = realloc(args[0], (counter + 1) * sizeof(char*));
				args[0][counter] = NULL;
				break;
			}
			continue;
		}
		//set character in arg to character in input
		arg[temp - 1] = input[i];
	}
	//free arg
	free(arg);
	arg = NULL;
}

//frees all elements of a ListNode node
void free_all_list(ListNode * node) {
	//free command
	free(node->command);
	//free all the argument pointers
	for (int i = 0; node->arguments[i] != NULL; i++) {
		free(node->arguments[i]);
	}
	//free argument pointers container
	free(node->arguments);
	//set next and prev to null
	node->next = NULL;
	node->prev = NULL;
	//if node has file contents, free that as well
	if (node->file_contents != NULL) {
		free(node->file_contents);
	}
	//finally free the node itself
	free(node);
	return;
}

//function to free all and end the shell
void quit_shell(char *** args, Item ** history, ListNode * list) {
	//free arguments first
	if (args[0] != NULL) {
		//iterates through argument pointers and frees each
		for (int i = 0; args[0][i] != NULL; i++) {
			free(args[0][i]);
		}
		//frees the argument pointers container
		free(args[0]);
	}
	args = NULL;
	//function to clear all of the history
	clear_history(history);
	//frees each item of list
	if (list != NULL) {
		//initialize walker to go to each list item
		ListNode * walker = list;
		while (walker != NULL) {
			//sets temporary variable to walker, walker goes to next node
			ListNode * temp = walker;
			walker = walker->next;
			//frees everything contained in node
			free_all_list(temp);
		}
	}
	//exits shell
	exit(0);
}

//function to change the directory
void change_directory(char * location) {
	//checks for error with the location and changes directory otherwise
	if (chdir(location) != 0) {
		fprintf(stderr, "error: %s\n", strerror(errno));
	}
}

//function to update the history given an input
void update_history(Item ** history, char * input) {
	//keeps temporary count of the current items in history
	int temp = -1;
	//checks if history is full
	int full = 0;
	//iterates through each item of histroy
	for (int i = 0; history[i] != NULL; i++) {
		temp = i;
		//checks if history is full
		if (i == 99) {
			full = 1;
			break;
		}
	}
	//removes and updates history to accomodate new item if full
	if (full == 1) {
		//frees the item being removed and its string
		free(history[0]->str);
		free(history[0]);
		//changes each item to be one lower and updates id accordingly
		for (int i = 0; i < 99; i++) {
			history[i] = history[i + 1];
			history[i]->id -= 1;
		}
		temp -= 1;
	}
	temp++;
	//creates new item to go into history
	Item * new_item = malloc(sizeof(Item));
	//sets id based on how many items already exist
	new_item->id = temp;
	//mallocs to hold input string plus null terminator
	new_item->str = malloc(strlen(input) + 1);
	//copies the input to the new item
	strcpy(new_item->str, input);
	//sets the item to the proper location in history
	history[temp] = new_item;
}

//function to print, in history, each item's id and string
void print_history(Item ** history) {
	//iterate through each item
	for (int i = 0; history[i] != NULL; i++) {
		//print the id and string
		printf("%d: %s", history[i]->id, history[i]->str);
		//in case some strings alreayd have a newline character to prevent weird gaps in between items
		if (history[i]->str[strlen(history[i]->str) - 1] != '\n') {
			printf("\n");
		}
	}
}

//function that returns the item in history with id 'index'
int get_history(Item ** history, int index) {
	//iterates through history
	for (int i = 0; history[i] != NULL; i++) {
		if (history[i]->id == index) {
			//returns index of item in history with id 'index'
			return i;
		}
	}
	//returns -1 otherwise
	return -1;
}

//function to remove all items from history
void clear_history(Item ** history) {
	//iterates through history
	for (int i = 0; history[i] != NULL; i++) {
		//frees item and its string
		free(history[i]->str);
		free(history[i]);
		history[i] = NULL;
	}
}

//function to print all ListNode nodes
void print_nodes(ListNode * head) {
	//check if any nodes exist
	if (head == NULL) {
		printf("Error: No nodes in list.\n");
		return;
	}
	//walker to iterate through the list
	ListNode * walker = head;
	while (walker != NULL) {
		//prints the id and command of the node
		printf("List Node %d\n", walker->id);
		printf("\tCommand: %s\n", walker->command);
		printf("\tFile Contents:\n");
		//if no file contents, prints nothing else
		if (walker->file_contents != NULL) {
			//if has file contents
			//copies the file contents from walker into a temporary variable
			char * copied = malloc(strlen(walker->file_contents) + 1);
			strcpy(copied, walker->file_contents);
			char * token;
			//seperates the copied file contents by newline characters
			token = strtok(copied, "\n");
			//prints each token with tab characters
			printf("\t\t%s\n", token);
			while ((token = strtok(NULL, "\n")) != NULL) {
				printf("\t\t%s\n", token);
			}
			//frees the temporary variable
			free(copied);
			copied = NULL;
		}
		//walker goes to next item in list
		walker = walker->next;
	}
}

//function to find a ListNode node by its id
ListNode * get_node_by_id(ListNode * head, int id) {
	//check if any nodes exist
	if (head == NULL) {
		fprintf(stderr, "error: list does not exist\n");
		return NULL;
	}
	//walker to iterate through list
	ListNode * walker = head;
	while (walker != NULL) {
		//if node with id exists, return it
		if (id == walker->id) {
			return walker;
		}
		//walker goes to next item if not found yet
		walker = walker->next;
	}
	//returns null and error if no node has matching id
	fprintf(stderr, "error: %s\n", "Id does not exist");
	return NULL;
}

//function to remove a node from the list
void remove_list(ListNode * node) {
	//adjusts the nodes previous and next of current node to point to each other respectively
	node->prev->next = node->next;
	node->next->prev = node->prev;
	//calls function to free everything in node
	free_all_list(node);
}

//function to run a command based on the arguments
void get_command(char * input, char *** args, Item ** history, ListNode ** list) {
	//checks if given command has proper number of arguments
	int result = check_args(args);
	//endptr for strtol calls
	char * endptr = NULL;
	if (strcmp(args[0][0], "quit") == 0) {
		//calls the quit_shell function if the parsed command from input is 'quit'
		quit_shell(args, history, *list);
	} else if (strcmp(args[0][0], "cd") == 0) {
		//updates history with input
		update_history(history, input);
		//if wrong number of arguments, return
		if (result == 1) {
			return;
		}
		//calls the change_directory function based on argument 1 from input
		change_directory(args[0][1]);
	} else if (strcmp(args[0][0], "history") == 0) {
		//if wrong number of arguments, return
		if (result == 1) {
			return;
		}
		//decide which command to run based on current arguments
		if (args[0][1] == NULL) {
			//print history if input is 'history'
			print_history(history);
		} else if (strcmp(args[0][1], "-c") == 0) {
			//clear history if input is 'history -c'
			clear_history(history);
		//strtol to make sure it is a proper number
		} else if (strtol(args[0][1], &endptr, 10), endptr != args[0][1] && *endptr == '\0') {
			//assign result to the number
			result = strtol(args[0][1], NULL, 10);
			//assign result to the index in history of previous result, if it exists
			result = get_history(history, result);
			//if no item in history at provided number, error and return
			if (result == -1) {
				fprintf(stderr, "error: %s\n", "Index in history list does not exist");
				return;
			}
			//empty args variable in preperation of reparsing the new input, from the string in the index in history
			clear_args(args);
			//temporary variable to prevent bugs from history being resassigned in recursion
			char * hold = malloc(strlen(history[result]->str) + 1);;
			//copies history string into temporary variable
			strcpy(hold, history[result]->str);
			//parses new input
			parse_input(hold, args);
			//recursively calls function but with new input/args
        		get_command(hold, args, history, list);
			//frees temporary variable
			free(hold);
			hold = NULL;
		}
	} else if (strcmp(args[0][0], "new") == 0) {
		//updates history
		update_history(history, input);
		//returns if wrong number of arguments
		if (result == -1) {
			return;
		}
		//creates new list node
		ListNode * node = malloc(sizeof(ListNode));
		//initialize everything
                node->next = NULL;
                node->prev = NULL;
		//malloc enough space for the command and null terminator
		node->command = malloc(strlen(args[0][1]) + 1);
		//malloc a 2d array for containing pointers to each argument
		node->arguments = malloc(sizeof(char*));
		//malloc first item of the 2d array with enough space for the respective argument plus null ternimator
		node->arguments[0] = malloc(strlen(args[0][1]) + 1);
		//copy the command from input into the command and first element of arguments
                strcpy(node->command, args[0][1]);
                strcpy(node->arguments[0], args[0][1]);
		//set arguments_length to 1
                node->arguments_length = 1;
		//iterate through provided arguments
		for (int i = 1; args[0][i + 1] != NULL; i++) {
			//realloc to accomodate an additional pointer to another argument
			node->arguments = realloc(node->arguments, (i + 1) * sizeof(char*));
			//malloc size of argument into the i'th item in node->arguments plus null terminator
			node->arguments[i] = malloc(strlen(args[0][i + 1]) + 1);
			//copy the orignial argument into the node's arguments
			//args[0][i + 1] to accomodate for 'new' being in args but not included in the node's arguments
                        strcpy(node->arguments[i], args[0][i + 1]);
			//increase arguments length by 1
                        node->arguments_length = i + 1;
                }
		//realloc to include null at the end of the arguments
		node->arguments = realloc(node->arguments, (node->arguments_length + 1) * sizeof(char*));
                node->arguments[node->arguments_length] = '\0';
		//initialize file contents to null
		node->file_contents = '\0';
		//test if doesnt exist
                if (*list == NULL) {
			//if not, node is the new header and id is set to 0
                        *list = node;
                        (*list)->id = 0;
                } else if ((*list)->next == NULL) {
			//if only 1 item in list,
                        if ((*list)->id == 0) {
				//either set node to be next if head node is at id = 0
                                node->id = 1;
                                (*list)->next = node;
                                node->prev = *list;
                        } else {
				//or set node to be first (prev) if head node is not ad id = 0
                                node->id = 0;
                                node->next = *list;
                                (*list)->prev = node;
                        }
                } else {
			//initialize head node
                        ListNode * head = *list;
                        while (head->next != NULL) {
				//iterate through list to find proper space given id's
                                if (head->id != head->next->id - 1) {
					//executed if gap between current id and next id is not 1 (e.x. 0 and 2, leaves space for a node at id = 1)
					//set node with correct id and next/prev, set surrounding nodes to accomodate for the new node
                                        node->id = head->id + 1;
                                        node->prev = head;
                                        node->next = head->next;
                                        head->next->prev = node;
                                        head->next = node;
                                        break;
                                } else {
					//executed if everything is in order
					head = head->next;
					//if everything in order and no more nodes,
					if (head->next == NULL) {
						//add new node at end of list and set the last node to this one instead of null
						node->id = head->id + 1;
						node->prev = head;
						head->next = node;
						break;
					}
				}
                        }
        	}
	} else if (strcmp(args[0][0], "list") == 0) {
		//update history
		update_history(history, input);
		//checks if correct number of arguments provided
		if (result == 1) {
			return;
		}
		//calls function to print all nodes 
		print_nodes(*list);
	} else if (strcmp(args[0][0], "open") == 0) {
		//update history
		update_history(history, input);
		//checks if correct number of arguments provided
		if (result != 0) {
			return;
		}
		//creates temporary node to test if node with id from input exists
		ListNode * temp = NULL;
		//strtol to check if proper number
                if (strtol(args[0][1], &endptr, 10), endptr != args[0][1] && *endptr == '\0') {
			//attempts to get node with id equal to number from input
                        temp = get_node_by_id(*list, strtol(args[0][1], NULL, 10));
                } else {
                        fprintf(stderr, "error: %s\n", "Id does not exist");
                        return;
                }
		//if node with provided number doesnt exist, return
                if (temp == NULL) {
                        return;
                }
		//initializes file to open
                FILE * file;
		//opens in read mode
                file = fopen(args[0][2], "r");
		//error if file not opened correctly
                if (file == NULL) {
                        fprintf(stderr, "error: file cannot be opened\n");
                        return;
                }
		//size of file
                int file_size = 0;
		//jumps to end of the file
                fseek(file, 0, SEEK_END);
		//assigns file_size to file position indicator
                file_size = ftell(file);
		//allocs enough space for file contents plus null terminator based on file size
                temp->file_contents = realloc(temp->file_contents, file_size + 1);
		//sets file position indicator back to start
                rewind(file);
		//reads file into file contents
                fread(temp->file_contents, 1, file_size, file);
		//null ternimates file contents
                temp->file_contents[file_size] = '\0';
		//closes file
                fclose(file);
	} else if (strcmp(args[0][0], "execute") == 0) {
		//update history
		update_history(history, input);
		//returns if wrong number of arguments provided
		if (result != 0) {
			return;
		}
		//strtol to test if proper number given
		if (strtol(args[0][1], &endptr, 10), endptr != args[0][1] && *endptr == '\0') {
                        //attempts to get node with given id
			ListNode * node = get_node_by_id(*list, strtol(args[0][1], NULL, 10));
			//if too many arguments, error and return
			if (node->arguments_length > 128) {
				fprintf(stderr, "error: %s\n", "too many arguments");
				return;
			}
			//run command stored in node
			result = run_command(node);
                } else {
			//return if id is not a proper number
                        fprintf(stderr, "error: %s\n", "Id does not exist");
                        return;
                }
		//check exit status from run_command
		if (result & 0xFF00) {
			fprintf(stderr, "error: %s\n", strerror(result));
			return;
		}
	} else if (strcmp(args[0][0], "remove") == 0) {
		//update history
		update_history(history, input);
		//checks if wrong number of arguments provided
		if (result != 0) {
			return;
		}
		//strtol to check if proper number
		if (strtol(args[0][1], &endptr, 10), endptr != args[0][1] && *endptr == '\0') {
			//attempts to get node with id
			ListNode * node = get_node_by_id(*list, strtol(args[0][1], NULL, 10));
			//if node doesnt exist, print error and return
			if (node == NULL) {
				fprintf(stderr, "error: %s\n", "Id does not exist");
                                return;
			}
			//call function to remove node from list
			remove_list(node);
		} else {
			//if not a proper number, error and return
			fprintf(stderr, "error: %s\n", "Id does not exist");
                        return;
		}
	} else {
		//prevent whitespace from appearing in history
		if (args[0][0][0] == '\n' || args[0][0][0] == ' ' || args[0][0][0] == '\0') {
			return;
		}
		//update history with unknown command
		update_history(history, input);
		return;
	}
	//reset endptr
	endptr = NULL;
}

//main function
int main() {
	//initialize input to fit 10000 characters
	char input[10000];
	//initialize args as 2d array
	char ** args = malloc(sizeof(char*));
	//initialize history to hold 100 pointers to items
	Item * history[100] = {NULL};
	//initialize list to null
	ListNode * list = NULL;
	//iterate until quit_shell called
	do {
		//calls function to clear input
		clear_input(input);
		//calls function to clear args
		clear_args(&args);
		//gets input from stdin
		int x = get_input(input);
		if (x) {
			//executes if end of file is reached, then quit_shell to free all and quit
			quit_shell(&args, history, list);
		}
		//get args from input
		parse_input(input, &args);
		//run command based on input/args
		get_command(input, &args, history, &list);
	} while (1);
	exit(0);
}

