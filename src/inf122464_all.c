#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <stdbool.h>
#include <sys/msg.h>
#include <math.h>

#include "inf122464_all.h"
#include "inf122464_rfun.h"

// TODO zabezpieczenie jak za duzo liter wpiszę (bo loguje), zabezpieczenie aby po jednym słowie
struct msgbuf login(int msgid, int type) {
    struct msgbuf request;
    int pid_user = getpid(); // pid of user (patient or doctor)
    request.pid = pid_user;
    bool appropriate = false;
    int log_counter = 0;
    while (!appropriate && log_counter <= MAX_LOG_COUNTER) {
        log_counter++;
        appropriate = true;
        printf("-----\n");
        printf("LOGIN\n");
        printf("-----\n");
        int pesel_size;
        switch (type) {
            case P_LOGIN_REQUEST:
                do {
                    printf("\tPROVIDE YOUR PESEL (11 SIGNS): ");
                    char pesel[11];
                    scanf("%s", pesel);
                    pesel_size = strlen(pesel);
                    int i;
                    for (i = 0; i < pesel_size; i++) {
                        if (pesel[i] >= '0' && pesel[i] <= '9') {
                            request.pesel[i] = pesel[i] - 48;
                        }
                        else {
                            printf("\tYou can use only digits!\n");
                            pesel_size = 0;
                            break;
                        }
                    }
                } while (pesel_size != 11);
                break;
            case D_LOGIN_REQUEST:
                printf("\tPROVIDE YOUR NAME: ");
                scanf("%s", request.name);
                printf("\tPROVIDE YOUR SURNAME: ");
                scanf("%s", request.surname);
                break;
            default:
                break;
        }
        printf("\tPROVIDE YOUR PASSWORD: ");
        scanf("%s", request.password);
        request.typ = type;         // appropriate type
        request.pid = pid_user;
        msgsnd(msgid, &request, MSGBUF_SIZE, 0);
        msgrcv(msgid, &request, MSGBUF_SIZE, pid_user, 0);
        printf("//dostałem wiadomosc\n");
        if (request.index == 1000) {
            appropriate = false;
            printf("\tThe log in wasn't successful. Do you want to try to log in again? Write 'y' if yes or 'n' if no: ");
            char decision;
            scanf("%s", &decision);
            if (decision == 'n') {
                request.index = 1000;
                return request;
            }
        }
        else {//if (request.typ == LOGIN_ANSWER) {
            appropriate = true;
            printf("\tYou've login successfully.\n");
            return request;
        }
    }
    if (log_counter > MAX_LOG_COUNTER) request.index = 1000;
    return request;
}

int createMessageQueue(char string[30]) {
    printf("-------------------------------\n");
    printf("WELCOME IN REGISTRATION SYSTEM\n");
    printf("-------------------------------\n");
    printf("%s\n", string);
    printf("-------------------------------\n");
    int msgid = msgget(1234, 0644 | IPC_CREAT);
    return msgid;
}

void clearCalendar(char (*day2)[13][5]) {
    int i;
    for (i = 0; i < 13; i++) {
        int j;
        for (j = 0; j < 5; j++) {
            (*day2)[i][j] = 'f';
        }
    }
}

time_t improveDate(time_t date) {
    //int mod = date % 86400;
    int mod = date % 3600;
    date = date - mod + 3600;
    return date;
}

void setLastDate(){
    //struct tm maxDate;
    // TODAY
    //today = (time_t *) malloc(4);
    //last_date = (time_t *) malloc(4);
    time(&today);
    time(&last_date);
    today = improveDate(today);
    //printf ("Today is improved %s\n", ctime( & today ));
    //maxDate = *localtime(&today);
    //printf ("Today is a %s\n", asctime( & maxDate ));

    // AFTER TWO MONTHS
    //*last_date = *today + 5184000; // if pointer
    last_date = today + 5184000;
    //maxDate = *localtime(&last_date);
    //printf ("In two months will be a %s\n", ctime( & last_date ));
    return;
}

void readConfigurationFile(char file_name[30]) {
    FILE *fp;
    char line[80];
    char c;
    fp = fopen(file_name, "r");
        if (fp == NULL) {
        perror("// The file is not good\n");
        return;
    }
    // fgets make first line unread
    while (fgets(line, 200, fp) != NULL) {
        struct msgbuf object;
        // TYPE
        fscanf(fp, "%c", &c);
        if (c == 'p') object.typ = PATIENT_TYPE;
        else if (c == 'd') object.typ = DOCTOR_TYPE;
        else if (c == 'a') object.typ = APPOINTMENT_TYPE;
        //
        fscanf(fp, "%d", &object.index);
        //fscanf(fp, "%d", &object.pid);
        fscanf(fp, "%s", object.name);
        fscanf(fp, "%s", object.surname);
        char pes[11];
        fscanf(fp, "%s", pes);
        int i;
        for (i = 0; i < 11; i++) {
            object.pesel[i] = pes[i] - 48;
        }
        fscanf(fp, "%s", object.password);
        int date;
        fscanf(fp, "%d", &date);
        object.date_of_visit = (time_t)date;
        printf("%s", ctime( & object.date_of_visit));
        fscanf(fp, "%d", &object.time_of_visit);
        // ADDING TO ARRAYS
        int type_p = PATIENT_TYPE;
        int type_d = DOCTOR_TYPE;
        int type_a = APPOINTMENT_TYPE;
        if (object.typ == type_p) {
            patients_list_size++;
            patients_list[patients_list_size - 1] = object;
            //printf("// Dodałem pacjenta %d, pas: %s, pesel: %d\n", patients_list_size, object.password, object.pesel[10]);
        }
        else if (object.typ == type_d) {
            doctors_list_size++;
            doctors_list[doctors_list_size - 1] = object;
            //printf("// Dodałem lekarza %d, pas: %s\n", doctors_list_size, object.password);
        }
        else if (object.typ == type_a) {
            appointments_list_size++;
            appointments_list[appointments_list_size - 1] = object;
            //printf("// Dodałem spotkanie %d\n", appointments_list_size);
        }
    }
    fclose(fp);
    return;
}