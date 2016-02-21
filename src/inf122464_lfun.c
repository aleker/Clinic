#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include"inf122464_all.h"
#include"inf122464_lfun.h"

void displayInstruction() {
    printf("\tIf you want to logout, enter 'q'.\n");
    printf("\tIf you want to view list of your appointments, enter 'l'.\n");
    printf("\tIf you want to take a leave, enter 'v': ");
    return;
}

void takeLeave(int msgid, struct msgbuf* doctor) {
    struct msgbuf leave;
    leave = *doctor;
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
    leave.time_of_visit = (difftime(chosenTime2, chosenTime) / 86400) + 1;     // in days;
    printf("\tDays of leave: %d\n", leave.time_of_visit);
    while (true) {
        printf("\tAre you sure you want to take a holiday? (Y/N): ");
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

void displayAllYourVisits(int msgid, struct msgbuf doctor) {
    int my_pid = getpid();
    int visit_counter = 0;
    struct msgbuf visit;
    visit.index = doctor.index;
    visit.typ = D_VISITS_REQUEST;
    visit.pid = my_pid;
    msgsnd(msgid, &visit, MSGBUF_SIZE, 0);
    printf("\n\tLIST OF YOUR VISITS:\n");
    printf("\t-----------------------------------------\n");
    while (true) {
        msgrcv(msgid, &visit, MSGBUF_SIZE , my_pid, 0);
        if (visit.index == 1000) {
            break;
        }
        else {
            visit_counter++;
            printf("\t%d\n", visit_counter);
            int i;
            for (i = 0; i < 11; i++) {
                doctor.pesel[i] = visit.pesel[i];
            }
            doctor.date_of_visit = visit.date_of_visit;
            doctor.time_of_visit = visit.time_of_visit;
            displayVisit(&visit, &doctor);
            //printf("\t%d %s", *visit_counter, ctime( &visit.date_of_visit));
        }
    }

    return;
}