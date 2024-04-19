#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

char secret[10] = "Denied";

int main() {
	char user[20];
	printf("Enter your username: ");
	fflush(stdout);
	fgets(user, 5000, stdin);
	if(strcmp(user, "admin") == 0 && strcmp(secret, "Granted") == 0) {
		printf("Welcome, admin!\n");
	} else {
		printf("Who are you?\n");
	}
	fflush(stdout);
}
