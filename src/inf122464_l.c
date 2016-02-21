// #include<fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include<unistd.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "inf122464_all.h"
#include "inf122464_lfun.h"

int main() {
    int msgid = createMessageQueue("DOCTOR");
    clearCalendar(&day);
    struct msgbuf doctor;
    doctor.pid = getpid();
    quit = false;
    bool log_on = false;
    while (!quit) {
        if (!log_on) {
            printf("---------\n");
            printf("MAIN MENU\n");
            printf("---------\n");
            printf("\tIf you want to login, enter 'l'.\n");
            printf("\tIf you want to quit, enter 'q': ");
            char choice;
            scanf("%s", &choice);
            if (choice == 'q') {
                quit = true;
            }
            else if (choice == 'l') {
                doctor = login(msgid, D_LOGIN_REQUEST);
                if (doctor.index < 1000) {
                    log_on = true;
                }
            }
        }
        else if (log_on) {
            printf("------------\n");
            printf("HELLO %s!\n", doctor.name);
            printf("------------\n");
            printf("// Test: %c\n", doctor.test);
            displayInstruction();
            while (true) {
                char decision;
                scanf("%s", &decision);
                if (decision == 'q') {
                    log_on = false;
                    printf("\tYou've logout successfully\n");
                    break;
                }
                else if (decision == 'v') {
                    takeLeave(msgid, &doctor);
                    displayInstruction();
                }
                else if (decision == 'l') {
                    displayAllYourVisits(msgid, doctor);
                    displayInstruction();
                }
            }
        }
    }
    return 0;
}
