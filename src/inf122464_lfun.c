#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
// #include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include"inf122464_all.h"
#include"inf122464_lfun.h"

void displayVisit(struct msgbuf* visit) {
    printf("\t-----------------------------------------\n");
    printf("\tPATIENT:\n");
    printf("\tPESEL:\t\t");
    int i;
    for (i = 0; i < 11; i++) {
        printf("%d", visit->pesel[i]);
    }
    printf("\n\tDOCTOR:\n");
    printf("\tNAME:\t\t%s\n", visit->name);
    printf("\tSURNAME:\t%s\n", visit->surname);
    printf("\tAPPOINTMENT:\n");
    printf("\tDATE:\t\t%s", ctime( &visit->date_of_visit));
    printf("\tDURATION TIME:\t%d\n", visit->time_of_visit);
    printf("\t-----------------------------------------\n");
}

void acceptVisit(int msgid, struct msgbuf* doctor) {
    struct msgbuf new_date;
    int msgrcv_size = msgrcv(msgid, &new_date, MSGBUF_SIZE , doctor->index + 20, IPC_NOWAIT);
    if (msgrcv_size > 0) {
        printf("// dostałem nową datę\n");
        displayVisit(&new_date);
        while (true) {
            printf("\tDo you want to accept this visit? (Y/N): \n");
            char choice;
            scanf("%s", &choice);
            if (choice == 'N') {
                new_date.typ = new_date.pid;
                new_date.index = 1000;
                //msgsnd(msgid, &new_date, MSGBUF_SIZE, 0);
                return;
            }
            else if (choice == 'Y') {
                new_date.typ = NEW_VISIT;
                msgsnd(msgid, &new_date, MSGBUF_SIZE, 0);
                return;
            }
        }
    }
    else printf("\tThere is no visit to accept.\n");
    return;
}

void displayInstruction() {
    printf("\tIf you want to logout, enter 'q'.\n");
    printf("\tIf you want to take a leave, enter 'l'.\n");
    printf("\tIf you want to check if there is any visit to accept, enter 'a': ");
    return;
}

void takeLeave(int msgid, struct msgbuf* doctor) {
    struct msgbuf leave;
    leave = *doctor;
    //leave.index = doctor->index;
    //leave.name = doctor->name;
    //leave.surname = doctor->surname;
    leave.typ = D_LEAVE;

    time_t chosenTime;
    time(&chosenTime);
    struct tm * chosenDate;
    chosenDate = localtime(&chosenTime);
    int year, month ,day;
    bool appropriate = false;
     do {
         // DISPLAYING DATE RANGE:
         setLastDate();
         printf("\tChoose date between:\n\t%s", ctime(&today));
         printf("\t%s\n", ctime(&last_date));
         // CHOOSING STARTING DATE
         printf("\tSTART\n");
         printf("\tEnter year: ");
         fflush(stdout);
         scanf("%d", &year);
         printf("\tEnter month: ");
         fflush(stdout);
         scanf("%d", &month);
         printf("\tEnter day: ");
         fflush(stdout);
         scanf("%d", &day);

         chosenDate->tm_year = year - 1900;
         chosenDate->tm_mon = month - 1;
         chosenDate->tm_mday = day;
         chosenDate->tm_hour = 8; // 0-23
         chosenDate->tm_min = 0;
         chosenDate->tm_sec = 0;
         //chosenTime = improveDate(chosenTime);
         chosenTime = mktime(chosenDate);

         if (chosenTime >= today && chosenTime <= last_date) appropriate = true;
         else printf("\tYour starting date is not in range!\n\n");
     } while (!appropriate);
    //printf("\t%s", ctime(&chosenTime));
    printf("\t%d.%d.%d\n\n", day, month, year);

    // CHOOSING ENDING DATE
    time_t chosenTime2;
    time(&chosenTime2);
    struct tm * chosenDate2;
    chosenDate2 = localtime(&chosenTime2);
    int year2, month2 ,day2;
    bool appropriate2 = false;
    do {
        printf("\tEND\n");
        printf("\tEnter year: ");
        fflush(stdout);
        scanf("%d", &year2);
        printf("\tEnter month: ");
        fflush(stdout);
        scanf("%d", &month2);
        printf("\tEnter day: ");
        fflush(stdout);
        scanf("%d", &day2);

        chosenDate2->tm_year = year2 - 1900;
        chosenDate2->tm_mon = month2 - 1;
        chosenDate2->tm_mday = day2;
        chosenDate2->tm_hour = 8; // 0-23
        chosenDate2->tm_min = 0;
        chosenDate2->tm_sec = 0;
        //chosenTime2 = improveDate(chosenTime2);
        chosenTime2 = mktime(chosenDate2);

        if (chosenTime2 >= today && chosenTime2 <= last_date && chosenTime2 > chosenTime) appropriate2 = true;
        else printf("\tYour endinging date is not in range!\n\n");
    } while (!appropriate2);
    //printf("\t%s", ctime(&chosenTime2));
    printf("\t%d.%d.%d\n\n", day2, month2, year2);

    leave.date_of_visit = chosenTime;
    leave.time_of_visit = (difftime(chosenTime2, chosenTime) / 86400);     // in days;
    printf("\tDays of leave: %d\n", leave.time_of_visit);
    while (true) {
        printf("\tAre you sure you want to take a holday? (Y/N): ");
        char decision;
        scanf("%s", &decision);
        if (decision == 'N') {
            printf("\tQuitting.\n\n");
            return;
        }
        else if (decision == 'Y') {
            // SENDING INFO TO REGISTRATION:
            msgsnd(msgid, &leave, MSGBUF_SIZE, 0);
            printf("\tHave a nice holiday!\n\n");
            return;
        }
    }
}