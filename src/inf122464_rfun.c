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

#include "inf122464_all.h"
#include "inf122464_rfun.h"

// TODO ! usuwanie przeterminowanych spotkań (nie działa) i przeterminowanych wakacji (index -> -1)
void searchForOutdatedAppointments() {
//    time_t now;
//    time(&now);
//    int i;
//    for (i = 0; i < APPOINTMENTS_LIST_SIZE; i++) {
//        if (appointments_list[i].time_of_visit == 0) continue; // free space
//        if ((appointments_list[i].date_of_visit)  < now) {
//            printf("//usuwam przeterminowany appointment");
//            appointments_list[i].time_of_visit = 0;
//            appointments_list_size--;
//        }
//    }
//    for (i = 0; i < 5; i++) {
//        if (vacation_list[i].index == i) {
//            if ((vacation_list[i].date_of_visit + (vacation_list[i].time_of_visit * 86400)) < now) {
//                printf("//usuwam przeterminowane wakacje\t");
//                vacation_list[i].index = -1;
//            }
//        }
//    }
    return;
}


void answerForChangeDateOfVisit(int msgid) {
    struct msgbuf new_date;
    int msgrcv_size = msgrcv(msgid, &new_date, MSGBUF_SIZE, CHANGE_VISIT, IPC_NOWAIT);
    if (msgrcv_size > 0) {
        printf("%s", ctime( & new_date.date_of_visit));
        if (fork() == 0) {
            printf("//forknąłem answerForChangeDateOfVisit! %d\n", getpid());
            // choose doctor
            int i;
            int meetings[5];
            for (i = 0; i < 5; i++) { meetings[i] = 0;}
            // if (appointments_list_size > 0)
            for (i = 0; i < APPOINTMENTS_LIST_SIZE; i++) {
                if (appointments_list[i].time_of_visit <= 0) continue;
                int index = atoi(appointments_list[i].password);
                printf("//index = %d\n", index);
                meetings[index]++;
            }
            int min_meetings_index = meetings[0];
            for (i = 1; i < 5; i++) {
                if (meetings[i] < meetings[i-1])
                    min_meetings_index = meetings[i];
            }
            // CHOOSING TIME
            // set tomorrow's date
            printf("// wybrany index = %d\n", min_meetings_index);
            struct tm tomorrow_date;
            const int one_day = 86400;
            time_t tomorrow = today;
            while (true) {                  // cannot be saturday or sunday
                tomorrow = tomorrow + one_day;
                tomorrow_date = *localtime(&tomorrow);
                if (tomorrow_date.tm_wday != 6 && tomorrow_date.tm_wday != 0) break;
            }
            tomorrow_date.tm_hour = 9;
            tomorrow_date.tm_min = 0;
            tomorrow_date.tm_sec = 0;
            tomorrow = mktime(&tomorrow_date);
            printf ("//jutro to %s\n", ctime( & tomorrow ));
            printf("//wybieramy czas\n");

            // TODO wziąć pod uwagę wakacje
            int time_difference = (int)difftime(last_date,tomorrow);
            bool founded_date;
            time_t rand_date;
            while (!founded_date) {
                founded_date = true;
                srand(time(NULL));
                int rand_add = rand() % time_difference + 1;
                rand_date = tomorrow + rand_add;
                tomorrow_date = *localtime(&rand_date);
                rand_add = rand() % 13 - new_date.time_of_visit;
                tomorrow_date.tm_hour = 9 + rand_add;
                tomorrow_date.tm_min = 0;
                tomorrow_date.tm_sec = 0;
                rand_date = mktime(&tomorrow_date);
                printf ("//rand_date to %s\n", ctime( & rand_date ));
                if (tomorrow_date.tm_wday != 6 && tomorrow_date.tm_wday != 0) {
                    bool the_same_pesel;
                    for(i = 0; i < APPOINTMENTS_LIST_SIZE; i++) {
                        the_same_pesel = true;
                        int j;
                        bool the_same_pesel = true;
                        for (j = 0; j < 11; j++) {
                            if (appointments_list[i].pesel[j] != new_date.pesel[j]) {
                                the_same_pesel = false;
                                break;
                            }
                        }
                        if (appointments_list[i].index == min_meetings_index || the_same_pesel) {
                            if (rand_date >= appointments_list[i].date_of_visit
                                    && rand_date <= appointments_list[i].date_of_visit + (86400*appointments_list[i].time_of_visit)) {
                                founded_date = false;
                                break;
                            }
                        }
                    }
                }
            }
            // DELETING OLD VISIT
            printf("//usuwam starą wizytę\n");
            new_date.typ = DELETE_VISIT;
            msgsnd(msgid, &new_date, MSGBUF_SIZE, 0);
            // SENDING INFO TO PATIENT
            new_date.date_of_visit = rand_date;
            sprintf(new_date.password,"%d", min_meetings_index);
            strcpy(new_date.name, doctors_list[min_meetings_index].name);
            strcpy(new_date.surname, doctors_list[min_meetings_index].surname);
            new_date.typ = new_date.pid;
            msgsnd(msgid, &new_date, MSGBUF_SIZE, 0);

            exit(getpid());
        }
    }
    return;
}

void deleteReservedVisits(int msgid, struct msgbuf leave) {
    int i;
    for (i = 0; i < APPOINTMENTS_LIST_SIZE; i++) {
        if (appointments_list[i].time_of_visit > 1) {
            time_t end = leave.date_of_visit + (leave.time_of_visit * 86400);
            if (appointments_list[i].date_of_visit >= leave.date_of_visit &&
                    appointments_list[i].date_of_visit <= end) {
                if (leave.index == atoi(appointments_list[i].password)) {   // if ids of doctor are the same
                    // setting time_of_visit=0 -> deleting appointment
                    printf("//usuwam wizytę\n");
                    appointments_list[i].time_of_visit = 0;
                    appointments_list_size--;
                }
            }
        }
    }
    return;
}

void answerForDoctorsLeave(int msgid) {
    struct msgbuf leave;
    int msgrcv_size = msgrcv(msgid, &leave, MSGBUF_SIZE , D_LEAVE, IPC_NOWAIT);
    if (msgrcv_size > 0) {
        printf("// dodałem do vacation_list\n");
        vacation_list[leave.index] = leave;
        deleteReservedVisits(msgid, leave);
    }
    return;
}

void deleteVisit(int msgid, int msgrcv_size) {
    struct msgbuf appointment;
    msgrcv_size = msgrcv(msgid, &appointment, MSGBUF_SIZE , DELETE_VISIT, IPC_NOWAIT);
    if (msgrcv_size > 0) {
        int i;
        for (i = 0; i < APPOINTMENTS_LIST_SIZE; i++) {
            if (appointment.index == appointments_list[i].index) {
                printf("//usuwam wizytę\n");
                appointments_list[i].time_of_visit = 0;
                appointments_list_size--;
            }
        }
    }
    return;
}

void addPatient(int msgid, int msgrcv_size) {
    struct msgbuf patient;
    msgrcv_size = msgrcv(msgid, &patient, MSGBUF_SIZE , PATIENT_TYPE, IPC_NOWAIT); // nieblokująco
    if (msgrcv_size > 0) {
        msgrcv_size = 0;
        patients_list_size++;
        patient.index = patients_list_size - 1;
        patients_list[patients_list_size - 1] = patient;
        printf("// New patient has been added.\n");
        printf("// patients_list_size = %d\n", patients_list_size);
    }
    return;
}

void sendDayContent(int msgid, int msgrcv_size, int pid_registration, int pid_patient, struct msgbuf appointment) {
    printf("//sendDayContent\n");
    struct tm appointmentDate = *localtime(&appointment.date_of_visit);
    time_t chosen_date = appointment.date_of_visit;
    printf("%s", asctime( & appointmentDate ));
    msgrcv_size = 0;
    int i;
    // SENDING REQUEST TO PARENT REGISTRATION
    appointment.typ = getppid();
    appointment.pid = pid_registration;
    msgsnd(msgid, &appointment, MSGBUF_SIZE, 0);
    appointments_list_size = 0;
    while (1) {
        //printf("dostałem\n");
        msgrcv(msgid, &appointment, MSGBUF_SIZE, pid_registration, 0);
        if (appointment.index == 1000) {
            break;
        }
        appointments_list[appointments_list_size] = appointment;
        printf("%d ", appointment.time_of_visit);
        if (appointment.time_of_visit > 0) appointments_list_size++;
    }
    printf("// appointments_list_size %d\n", appointments_list_size);

    // SENDING INFO ABOUT APPOINTMENTS THAT DAY
    if (appointments_list_size > 0) {
        for (i = 0; i < APPOINTMENTS_LIST_SIZE; i++) {
            struct tm currentDate;
            currentDate = *localtime(&appointments_list[i].date_of_visit);
            if ((currentDate.tm_year == appointmentDate.tm_year) &&
                (currentDate.tm_mon == appointmentDate.tm_mon)
                && (currentDate.tm_mday == appointmentDate.tm_mday)
                    && appointments_list[i].time_of_visit > 0) {
                appointment = appointments_list[i];
                appointment.pid = pid_registration; //APPOINTMENT_ANSWER;
                appointment.typ = pid_patient;
                msgsnd(msgid, &appointment, MSGBUF_SIZE, 0); // blokująco
                printf("// wysłałem appointment //APPOINTMENT_ANSWER\n");
            }
            //else printf("// This appointment is not in that day\n");
        }
    }
    appointment.pid = pid_registration;
    appointment.typ = pid_patient;
    appointment.index = 1000;
    msgsnd(msgid, &appointment, MSGBUF_SIZE, 0);
    printf("// wysłałem M_END (pseudo)\n");

    // SENDING INFO ABOUT VACATION THAT DAY
    for (i = 0; i < 5; i++) {
        if (vacation_list[i].index == i) {
            time_t start = vacation_list[i].date_of_visit;
            time_t end = vacation_list[i].date_of_visit + (vacation_list[i].time_of_visit * 86400);
            if (chosen_date > start && chosen_date < end) {
                appointment = vacation_list[i];
                appointment.time_of_visit = 13;
                appointment.pid = pid_registration;
                appointment.typ = pid_patient;
                msgsnd(msgid, &appointment, MSGBUF_SIZE, 0);
                printf("// wysłałem2 vacation (pseudo)\n");
            }
        }
    }
    appointment.pid = pid_registration;
    appointment.typ = pid_patient;
    appointment.index = 1000;
    msgsnd(msgid, &appointment, MSGBUF_SIZE, 0);
    printf("// wysłałem2 M_END (pseudo)\n");
    return;
}

void answerForListOfDoctors(int msgid, int msgrcv_size) {
    struct msgbuf appointment;
    msgrcv_size = msgrcv(msgid, &appointment, MSGBUF_SIZE , DOCTORS_REQUEST, IPC_NOWAIT);
    if (msgrcv_size > 0) {
        // CHILD REGISTRATION
        if (fork() == 0) {
            printf("//forknalem answerForListDoctors %d\n", getpid());
            int pid_registration = getpid();
            int pid_patient = appointment.pid; // pid of patient
            // to fill in the content of day[][] array:
            sendDayContent(msgid, msgrcv_size, pid_registration, pid_patient, appointment);
            // REGISTRATION TO DOCTOR:
            msgrcv(msgid, &appointment, MSGBUF_SIZE, pid_registration, 0);
            if (appointment.index == 1000) {
                printf("// pacjent zrezygnował z zapisu do lekarza\n");
                exit(pid_registration);
                return;
            }
            else {
                // SENDING APPOINTMENT TO PARENT REGISTRATION
                appointment.typ = getppid();
                msgsnd(msgid, &appointment, MSGBUF_SIZE, 0);
                printf("// otrzymałem i wysłałem do rodzica nowy appointment\n");
            }
            exit(pid_registration);
        }
            // PARENT REGISTRATION
        else {
            msgrcv(msgid, &appointment, MSGBUF_SIZE , getpid(), 0); // waiting
            int pid_parent = appointment.pid;
            int i;
            // sending list of appointments:
            for (i = 0; i < APPOINTMENTS_LIST_SIZE; i++) {
                appointment = appointments_list[i];
                appointment.typ = pid_parent;
                msgsnd(msgid, &appointment, MSGBUF_SIZE, 0);
            }
            appointment.index = 1000;
            msgsnd(msgid, &appointment, MSGBUF_SIZE, 0);
        }
    }
    return;
}

void answerForDoctorLoginRequest(int msgid, int msgrcv_size) {
    struct msgbuf doctor;
    msgrcv_size = msgrcv(msgid, &doctor, MSGBUF_SIZE , D_LOGIN_REQUEST, IPC_NOWAIT);
    if (msgrcv_size > 0) {
        if (fork() == 0) {
            printf("//forknalem answerForDoctorLoginRequest %d\n", getpid());
            int pid_registration = getpid();
            int pid_doctor = doctor.pid;

            printf("// patients_list_size = %d\n", doctors_list_size);
            doctor.typ = pid_doctor;
            doctor.pid = pid_registration;
            if (doctors_list_size > 0) {
                int i;
                for (i = 0; i < doctors_list_size; i++) {
                    bool appropriate = true;
                    int j;
                    // checking name
                    int name_size = strlen(doctors_list[i].name);
                    for (j = 0; j < name_size; j++) {
                        if (doctor.name[j] != doctors_list[i].name[j]) {
                            appropriate = false;
                            break;
                        }
                    }
                    if (!appropriate) continue;
                    // checking surname
                    int surname_size = strlen(doctors_list[i].surname);
                    for (j = 0; j < surname_size; j++) {
                        if (doctor.surname[j] != doctors_list[i].surname[j]) {
                            appropriate = false;
                            break;
                        }
                    }
                    if (!appropriate) continue;
                    // good name and surname:
                    int password_size = strlen(doctors_list[i].password);
                    for (j = 0; j < password_size; j++) {
                        if (doctor.password[j] != doctors_list[i].password[j]) {
                            // wrong password
                            printf("// złe hasło, brak logowania\n");
                            doctor.index = 1000;
                            msgsnd(msgid, &doctor, MSGBUF_SIZE, 0);
                            exit(pid_registration);
                            return;
                        }
                    }
                    // good name, surname and password
                    printf("// znalazłem takiego doktorka\n");
                    doctor = doctors_list[i];
                    doctor.test = 'a';
                    doctor.typ = pid_doctor;
                    msgsnd(msgid, &doctor, MSGBUF_SIZE, 0);
                    exit(pid_registration);
                    return;
                }
            }
            // wrong pesel
            doctor.index = 1000;
            msgsnd(msgid, &doctor, MSGBUF_SIZE, 0);
            printf("// wychodze exit(pid)\n");
            exit(pid_registration);
        }
    }
    return;
}

void answerForLoginRequest(int msgid, int msgrcv_size) {
    struct msgbuf patient;
    msgrcv_size = msgrcv(msgid, &patient, MSGBUF_SIZE , P_LOGIN_REQUEST, IPC_NOWAIT);
    if (msgrcv_size > 0) {
        if (fork() == 0) {
            printf("//forknalem answerForLoginRequest %d\n", getpid());
            int pid_registration = getpid();
            int pid_patient = patient.pid;

            printf("// patients_list_size = %d\n", patients_list_size);
            patient.typ = pid_patient;
            patient.pid = pid_registration;
            if (patients_list_size > 0) {
                int i;
                for (i = 0; i < patients_list_size; i++) {
                    bool appropriate = true;
                    int j;
                    for (j = 0; j < 11; j++) {
                        if (patient.pesel[j] != patients_list[i].pesel[j]) {
                            appropriate = false;
                            break;
                        }
                    }
                    if (!appropriate) continue;
                    // good pesel, wrong password
                    int password_size = strlen(patients_list[i].password);
                    for (j = 0; j < password_size; j++) {
                        if (patient.password[j] != patients_list[i].password[j]) {
                            printf("// złe hasło, brak logowania\n");
                            patient.index = 1000;
                            msgsnd(msgid, &patient, MSGBUF_SIZE, 0);
                            exit(pid_registration);
                            return;
                        }
                    }
                    // good pesel and password
                    printf("// znalazłem takiego ziomka\n");
                    patient = patients_list[i];
                    patient.test = 'a';
                    patient.typ = pid_patient;
                    msgsnd(msgid, &patient, MSGBUF_SIZE, 0);
                    exit(pid_registration);
                    return;
                }
            }
            // wrong pesel
            patient.index = 1000;
            msgsnd(msgid, &patient, MSGBUF_SIZE, 0);
            printf("// wychodze exit(pid)\n");
            exit(pid_registration);
        }
    }
    return;
}

void answerForListOfVisits(int msgid, int msgrcv_size) {
    struct msgbuf visit, appointment;
    msgrcv_size = msgrcv(msgid, &visit, MSGBUF_SIZE , VISITS_REQUEST, IPC_NOWAIT);
    if (msgrcv_size > 0) {
        if (fork() == 0) {
            printf("//forknalem answerForListOfVisits %d\n", getpid());
            int pid_registration = getpid();
            int pid_patient = visit.pid;

            // SENDING ALL APPOINTMENTS
            int i;
            bool appropriate = false;
            for (i = 0; i < APPOINTMENTS_LIST_SIZE; i++) {
                int j;
                for (j = 0; j < 11; j++) {
                    if (appointments_list[i].pesel[j] != visit.pesel[j]) {
                        appropriate = false;
                        break;
                    }
                    appropriate = true;
                }
                if (appropriate && appointments_list[i].time_of_visit > 0) {
                    printf("//wysyłam appointment\n");
                    appointment = appointments_list[i];
                    appointment.typ = pid_patient;
                    msgsnd(msgid, &appointment, MSGBUF_SIZE, 0);
                }
            }
            appointment.index = 1000;
            appointment.pid = pid_registration;
            appointment.typ = pid_patient;
            printf("//już nie ma więcej appointmentów\n");
            msgsnd(msgid, &appointment, MSGBUF_SIZE, 0);

            // INFORMATION ABOUT ONE CHOSEN APPOINTMENT
            msgrcv(msgid, &visit, MSGBUF_SIZE , pid_registration, 0);
            if (visit.index == 1000) {   // resignation
                printf("//zrezygnował z wyświetlania\n");
                exit(pid_registration);
                return;
            }
            // choosing visit
            int number = visit.index;   // number of visit
            int counter = 0;
            for (i = 0; i < appointments_list_size; i++) {
                appropriate = true;
                int j;
                for (j = 0; j < 11; j++) {
                    if (appointments_list[i].pesel[j] != visit.pesel[j]) {
                        appropriate = false;
                        break;
                    }
                }
                if (appropriate) counter++;
                if (appropriate && appointments_list[i].time_of_visit > 0 && counter == number) {
                    appointment = appointments_list[i];
                    appointment.typ = pid_patient;
                    appointment.test = 'a';
                    msgsnd(msgid, &appointment, MSGBUF_SIZE, 0);
                    exit(pid_registration);
                    return;
                }
            }
            exit(pid_registration);
        }
    }
    return;
}

void searchForNewAppointment(int msgid, int msgrcv_size) {
    struct msgbuf appointment;
    int pid_registration = getpid();
    msgrcv_size = msgrcv(msgid, &appointment, MSGBUF_SIZE, pid_registration, IPC_NOWAIT);
    // from doctor when he want to change the date of visit:
    if (msgrcv_size <= 0) msgrcv_size = msgrcv(msgid, &appointment, MSGBUF_SIZE, NEW_VISIT, IPC_NOWAIT);
    if (msgrcv_size > 0) {
        bool ready;
        int i = 0;
        while (!ready) {
            if (appointments_list[i].time_of_visit != 0) i++;
            else {
                appointment.index = i;
                appointments_list[i] = appointment;
                appointments_list_size++;
                ready = true;
                printf("// Dodałem spotkanie %d\n", appointments_list_size);
            }
        }
    }
    return;
}

void displayInstruction() {
    printf("\tIf you want to turn off the registration, write 'q'.\n");
}

bool checkExit(int msgid) {
    struct msgbuf quit;
    int msgrcv_size = msgrcv(msgid, &quit, MSGBUF_SIZE, QUIT, IPC_NOWAIT);
    if (msgrcv_size > 0) {
        return true;
    }
    return false;
}