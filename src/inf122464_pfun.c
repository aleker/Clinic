#include <stdio.h>
#include <stdlib.h>
#include<unistd.h>
// #include<fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "inf122464_all.h"
#include "inf122464_pfun.h"


void chooseDate(struct msgbuf* patient) {
    setLastDate();
    time_t chosenTime;
    struct tm * chosenDate;
    int year, month ,day;
    bool appropriate = false;
    time(&chosenTime);
    chosenDate = localtime(&chosenTime);
    do {
        printf("\tChoose date between:\n\t%s", ctime(&today));
        printf("\t%s\n", ctime(&last_date));
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
        //printf("\t%s", ctime(&chosenTime));
        printf("%d.%d.%d\n", day, month, year );

        if (chosenTime >= today && chosenTime <= last_date) {
            if (chosenDate->tm_wday == 6 ) {
                printf("\t Sorry, we don't work on Saturdays.\n");
            }
            else if (chosenDate->tm_wday == 0 ) {
                printf("\t Sorry, we don't work on Sundays.\n");
            }
            else {
                appropriate = true;
                patient->date_of_visit = chosenTime;
            }
        }
        else printf ("\tYour date is not in range\n");
    } while(!appropriate);
    return;
}


void addAppointmentToCalendar(struct msgbuf* patient, struct msgbuf appointment) {
    printf("//addAppointment to Calendar\n");
    struct tm chosenDate;
    chosenDate = *localtime(&appointment.date_of_visit);
    // 0-23 -> 1-24
    int hour = chosenDate.tm_hour;
    if (hour < 9) hour = 0;
    else hour -= 9;
    int duration = appointment.time_of_visit;
    bool this_patient = true;
    int i;
    for (i = 0; i < 11; i++) {
        if (patient->pesel[i] != appointment.pesel[i]) {
            this_patient = false;
            break;
        }
    }
    int index_doctor = atoi(appointment.password);
    while (duration > 0) {
        if (appointment.typ == D_LEAVE) {
            day[hour][appointment.index] = 'X'; // holiday time
        }
        else if (this_patient) {
            //(*day)[hour][index_doctor] = 'y'; // from "yours"
            day[hour][index_doctor] = 'y'; // from "yours"
        }
        else  {
            //(*day)[hour][index_doctor] = 'o';
            day[hour][index_doctor] = 'o'; // from "occupied"
        } // from "occupied"
        duration--;
        hour++;
    }
    return;
}

void displayCalendar(char (*day)[13][5], int width, int height) {
    // TODO nazwiska zamiast id
    printf("\tLEGEND:\n");
    printf("\t'o' means 'occupied'\n\t'y' means 'yours appointment'\n\t'f' means 'free'\n");
    printf("\tCALENDAR:\n");
    int i;
    float hour = 9.00;
    printf("\t\t| DOCTORS' ID       |\n");
    printf("\ttime\t| 0 | 1 | 2 | 3 | 4 |\n");
    printf("\t________|___|___|___|___|___|\n");
    for (i = 0; i < height; i++) {
        int j;
        printf("\t%.2f\t|", hour);
        for (j = 0; j < width; j++) {
            printf (" %c |", (*day)[i][j]);
        }
        printf("\n");
        hour++;
    }
    return;
}

void requestForDoctorsList(int msgid, struct msgbuf* patient) {
    int pid_registration;
    int pid_patient = getpid();
    // CHOOSING DATE
    chooseDate(patient);
    printf("------------------------\n");
    printf("FREE DOCTORS AT: %s\n", ctime(&patient->date_of_visit));
    printf("------------------------\n");
    printf("\tThe list of free appointments:\n");
    patient->pid = pid_patient;
    patient->typ = DOCTORS_REQUEST;
    msgsnd(msgid, patient, MSGBUF_SIZE, 0);
    // CREATING DAY FROM CALENDAR
    clearCalendar(&day);
    // READING FREE APPOINTMENTS
    struct msgbuf request;
    // receiving appointments:
    do {
        printf("//dostanę?\n");
        msgrcv(msgid, &request, MSGBUF_SIZE, pid_patient, 0);
        printf("//dostałem app\n");
        pid_registration = request.pid;
        if (request.index == 1000) {
            break;
        }
        addAppointmentToCalendar(patient, request);
    } while (1);
    printf("//dostałem appointmenty\n");
    // receiving holidays:
    do {
        msgrcv(msgid, &request, MSGBUF_SIZE, pid_patient, 0);
        pid_registration = request.pid;
        if (request.index == 1000) {
            break;
        }
        request.typ = D_LEAVE;
        addAppointmentToCalendar(patient, request);
    } while (1);
    printf("//dostałem holidaysy\n");
    displayCalendar(&day, 5, 13);
    while (true) {
        printf("\n\tWrite 'a' to make an appointment or 'q' to go to main menu\n");
        char choice;
        scanf("%s", &choice);
        if (choice == 'q') {
            // QUIT
            displayInstruction();
            request.pid = pid_patient;
            request.typ = pid_registration;
            request.index = 1000;
            msgsnd(msgid, &request, MSGBUF_SIZE, 0);
            return;
        }
        else if (choice == 'a') {
            // MAKING AN APPOINTMENT
            makeAnAppointment(msgid, patient, pid_registration, false);
            displayInstruction();
            return;
        }
    }
}
// TODO ! umawianie wizyty ponad 2 miesiące -> przypomnienie 2 tygodnie przed
bool makeAnAppointment(int msgid, struct msgbuf* patient, int pid_registration, bool doctors_decision) {
    struct msgbuf request;
    request.pid = patient->pid;
    request.typ = pid_registration;
    printf("\tNow you can make an appointment.\n");
    bool correct = false;
    // ENTERING DATA FOR AN APPOINTMENT
    while(!correct) {
        correct = true;
        do {
            printf("\tEnter the id of doctor: ");
            scanf("%s", request.password);
            request.index = atoi(request.password);
        } while (request.index < 0 || request.index > 5);
        printf("\tEnter the duration of visit (in hours): ");
        scanf("%d", &request.time_of_visit);
        int hour;
        do {
            printf("\tEnter the hour: ");
            scanf("%d", &hour);
        } while(hour < 9 || hour > 21);
        time_t chosen_date = patient->date_of_visit;
        int mod = chosen_date % 86400;
        chosen_date = chosen_date - mod + 3600 * (hour-1);
        request.date_of_visit = chosen_date;
        if (chosen_date < today) {
            printf("\tYour chosen time has come!\n");
            correct = false;
        }
        int z;
        for (z = 0; z < request.time_of_visit; z++) {
            if (day[hour-9 + z][request.index] == 'o') {
                printf("\tThis time is occupied!\n");
                correct = false;
                break;
            }
        }
    }
    // pesel in appointment as patient's pesel
    int p;
    for (p = 0; p < 11; p++) {
        request.pesel[p] = patient->pesel[p];
    }
    // SENDING INFORMATION TO DOCTOR
    if (doctors_decision) {
        request.pid = patient->pid;
        sprintf(request.password, "%d", request.index); // (index will be changed, password is unnecessary)
        request.typ = request.index + 20;                       // ACCEPT_X
        msgsnd(msgid, &request, MSGBUF_SIZE, 0);
        printf("\tYou have to wait for doctor's acceptation.\n");
        msgrcv(msgid, &request, MSGBUF_SIZE , patient->pid, 0); // dałam pid!!!!!!!!!!!!!!!
        if (request.index == 1000) {
            printf("\tSorry. Doctor hasn't accepted your visit.\n");
            return false;
        }
        else printf("\tDoctor has accepted your visit.\n");
    }
    // SENDING MSG WITH NEW APPOINTMENT
    request.pid = patient->pid;
    sprintf(request.password, "%d", request.index); // (index will be changed, password is unnecessary)
    request.typ = pid_registration;
    msgsnd(msgid, &request, MSGBUF_SIZE, 0);
    printf("// wysłałem nowy appointment\n");
    return true;
}


struct msgbuf registerNewPatient(int msgid) {
    // TODO dozwolone znaki, ograniczenia wielkościowe, wyjście z trybu rejestrowania
    int pid_patient = getpid(); // pid of patient

    printf("-------\n");
    printf("SIGN UP\n");
    printf("-------\n");
    struct msgbuf patient;
    patient.typ = PATIENT_TYPE;
    patient.pid = pid_patient;
    printf("\tYou have to provide your personal information.\n");
    printf("\tConfirm every information by pressing 'Enter'.\n");
    printf("\tWRITE YOUR NAME: ");
    scanf("%s", patient.name);
    printf("\tWRITE YOUR SURNAME: ");
    scanf("%s", patient.surname);
    char pesel[11];
    int pesel_size;
    // TODO niepowtarzający się pesel
    do {
        printf("\tWRITE YOUR PESEL: ");
        scanf("%s", pesel);
        pesel_size = strlen(pesel);
        printf("// pesel_size = %d\n", pesel_size);
        if (pesel_size != 11) {
            printf("\tPESEL is incorrect!\n");
        }
    } while (pesel_size != 11);
    for (pesel_size =0; pesel_size < 11; pesel_size++) {
        patient.pesel[pesel_size] = pesel[pesel_size] - 48;
    }
    printf("\tWRITE PASSWORD (MAX. %d SIGNS): ", PASSWORD_SIZE);
    scanf("%s", patient.password);
    patient.time_of_visit = 0;
    msgsnd(msgid, &patient, MSGBUF_SIZE, 0); // blokująco
    printf("\tYou've registered successfully.\n");
    return patient;
}

void displayYourVisits(int msgid, struct msgbuf* patient, int my_pid, int *registration_pid, int *visit_counter, int receiver) {
    struct msgbuf visit;
    visit.typ = receiver;
    visit.pid = my_pid;
    int i;
    for (i = 0; i < 11; i++) {
        visit.pesel[i] = patient->pesel[i];
    }
    // DISPLAY ALL VISITS
    msgsnd(msgid, &visit, MSGBUF_SIZE, 0);
    printf("\n\tLIST OF YOUR VISITS:\n");
    while (true) {
        msgrcv(msgid, &visit, MSGBUF_SIZE , my_pid, 0);
        if (visit.index == 1000) {
            (*registration_pid) = visit.pid;
            break;
        }
        else {
            (*visit_counter) = *visit_counter + 1;
            printf("\t%d %s", *visit_counter, ctime( &visit.date_of_visit));
        }
    }
    printf("\n");
    return;
}

struct msgbuf displayStatusOfVisit(int msgid, struct msgbuf patient) {
    int my_pid = getpid();
    int registration_pid, visit_counter = 0;
    displayYourVisits(msgid, &patient, my_pid, &registration_pid, &visit_counter, VISITS_REQUEST);
    struct msgbuf visit;
    // DISPLAY MORE INFORMATION ABOUT CONCRETE VISIT OR CANCEL
    printf("\tIf you want to resign, enter 'q'.\n");
    printf("\tIf you want to continue, enter the number of visit wanted to display: ");
    char choice[3];
    scanf("%s", choice);
    if (choice[0] == 'q') {
        visit.typ = registration_pid;
        visit.index = 1000;
        msgsnd(msgid, &visit, MSGBUF_SIZE, 0);
        return visit;
    }
    else if (atoi(choice) >= 0 && atoi(choice) <= visit_counter) {
        //printf("//mieści sie w granicach\n");
        visit.typ = registration_pid;
        visit.pid = my_pid;
        visit.index = atoi(choice);
        int i;
        for (i = 0; i < 11; i++) {
            visit.pesel[i] = patient.pesel[i];
        }
        msgsnd(msgid, &visit, MSGBUF_SIZE, 0);
        // displaying:cancelVisit(int msgid, struct msgbuf* patient)
        msgrcv(msgid, &visit, MSGBUF_SIZE, my_pid, 0);
        displayVisit(&patient, &visit);
    }
    else {
        printf("\tThis is NOT a correct choice! Quitting.\n");
        visit.index = 1000;
        visit.typ = registration_pid;
        msgsnd(msgid, &visit, MSGBUF_SIZE, 0);
    }
    return visit;
}

void cancelVisit(int msgid, struct msgbuf *patient) {
    struct msgbuf del_appointment = displayStatusOfVisit(msgid, *patient);
    if (del_appointment.index == 1000) return;
    while (true) {
        printf("\tDo you want to cancel this visit? (Y/N): ");
        char choice;
        scanf("%s", &choice);
        if (choice == 'N') {
            return;
        }
        else if (choice == 'Y') {
            printf("\tThis visit has been canceled!\n");
            del_appointment.typ = DELETE_VISIT;
            msgsnd(msgid, &del_appointment, MSGBUF_SIZE, 0);
            return;
        }
    }
}


void changeDateOfVisit(int msgid, struct msgbuf* patient) {
    struct msgbuf del_appointment = displayStatusOfVisit(msgid, *patient);
    if (del_appointment.index == 1000) return;
    while (true) {
        printf("\tDo you want to change date of this visit? (Y/N): ");
        char choice;
        scanf("%s", &choice);
        if (choice == 'N') {
            return;
        }
        else if (choice == 'Y') {
            int my_pid = getpid();
            printf("\tThis visit will be changed!\n");
            printf("\tRegistation will choose the date for you.\n");
            bool agree = false;
            struct msgbuf new_appointment;
            while (!agree) {
                del_appointment.pid = my_pid;
                del_appointment.typ = CHANGE_VISIT;
                msgsnd(msgid, &del_appointment, MSGBUF_SIZE, 0);
                msgrcv(msgid, &new_appointment, MSGBUF_SIZE, my_pid, 0);
                printf("\n\tYOUR NEW DATE: %s", ctime( & new_appointment.date_of_visit));
                while (true) {
                    printf("\tDo you accept new date (a), want to generate new date (g), cancel changes (c)?: ");
                    char choice;
                    scanf("%s", &choice);
                    if (choice == 'a') {
                        agree = true;
                        break;
                    }
                    else if (choice == 'g') break;
                    else if (choice == 'c') {
                        return;
                    }
                }

            }
            // DELETING OLD VISIT
            printf("//usuwam starą wizytę\n");
            del_appointment.typ = DELETE_VISIT;
            msgsnd(msgid, &del_appointment, MSGBUF_SIZE, 0);
            // ADDING NEW VISIT
            new_appointment.typ = NEW_VISIT;
            msgsnd(msgid, &new_appointment, MSGBUF_SIZE, 0);
            printf("//wysłałem nową datę\n");
            return;
        }
    }
}

void displayInstruction() {
    printf("\tIf you want to logout, enter 'q'.\n");
    printf("\tIf you want to display list of free doctors for concrete date, enter 'd'.\n");
    printf("\tIf you want to display the status of your appointments, enter 's'.\n");
    printf("\tIf you want to cancel visit, enter 'c'.\n");
    printf("\tIf you want to change date of your visit, enter 'v': ");
}

// TODO ! wyswietlanie wolnych terminów do lekarza
void createMonth(char (*day), int width, int height, time_t date) {
//    struct tm chosenDate;
//    chosenDate = *localtime(&date);
//    // 0=sunday, 1=monday...
//    int day_type = chosenDate.tm_wday;
//    // 1=january...
//    int month_type = chosenDate.tm_mon + 1;
//    int day_amount;
//    if (month_type == 2) {
//        day_amount = 28;
//    }
//    else if (month_type == 1 || month_type == 3 || month_type == 5 || month_type == 7 ||
//             month_type == 8 || month_type == 10 || month_type == 12) {
//        day_amount = 31;
//    }
//    else day_amount = 30;
    return;
}