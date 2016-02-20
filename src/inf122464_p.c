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
#include "inf122464_pfun.h"

int main() {
    int msgid = createMessageQueue("PATIENT");
    clearCalendar(&day);
    struct msgbuf patient;
    patient.pid = getpid();
    quit = false;
    bool log_on = false;
    while (!quit) {
        if (!log_on) {
            printf("---------\n");
            printf("MAIN MENU\n");
            printf("---------\n");
            printf("\tIf you want to login, enter 'l'.\n");
            printf("\tIf you want to register, enter 'r'.\n");
            printf("\tIf you want to quit, enter 'q': ");
            char choice;
            scanf("%s", &choice);
            if (choice == 'q') {
                quit = true;
            }
            else if (choice == 'r') {
                patient = registerNewPatient(msgid);
                // TODO nie zawsze log_on przy rejestrowaniu nowego pacjenta
                log_on = true;
            }
            else if (choice == 'l') {
                patient = login(msgid, P_LOGIN_REQUEST);
                if (patient.index < 1000) {
                    log_on = true;
                }
            }
        }
        else if (log_on) {
            printf("------------\n");
            printf("HELLO %s!\n", patient.name);
            printf("------------\n");
            printf("// Test: %c\n", patient.test);
            displayInstruction();
            while (true) {
                char decision;
                scanf("%s", &decision);
                if (decision == 'q') {
                    log_on = false;
                    printf("\tYou've logout successfully\n");
                    break;
                }
                else if (decision == 'd') {
                    requestForDoctorsList(msgid, &patient);
                }
                else if (decision == 's') {
                    displayStatusOfVisit(msgid, patient);
                    displayInstruction();
                }
                else if (decision == 'c') {
                    cancelVisit(msgid, &patient);
                    displayInstruction();
                }
                else if (decision == 'v') {
                    changeDateOfVisit(msgid, &patient);
                    displayInstruction();
                }
            }
        }
    }
    return 0;
} 
