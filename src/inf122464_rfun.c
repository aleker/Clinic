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

#include "inf122464_all.h"
#include "inf122464_rfun.h"

void searchForOutdatedAppointments() {
    time_t now;
    time(&now);
    int i;
    for (i = 0; i < APPOINTMENTS_LIST_SIZE; i++) {
        if (appointments_list[i].time_of_visit == 0) continue; // free space
        if ((appointments_list[i].date_of_visit)  < now) {
            appointments_list[i].time_of_visit = 0;
            appointments_list_size--;
        }
    }
    for (i = 0; i < 5; i++) {
        if (vacation_list[i].index == i) {
            if ((vacation_list[i].date_of_visit + (vacation_list[i].time_of_visit * 86400)) < now) {
                vacation_list[i].index = -1;
            }
        }
    }
    return;
}


void answerForChangeDateOfVisit(int msgid) {
    struct msgbuf new_date;
    int msgrcv_size = msgrcv(msgid, &new_date, MSGBUF_SIZE, CHANGE_VISIT, IPC_NOWAIT);
    if (msgrcv_size > 0) {
        printf("%s", ctime( & new_date.date_of_visit));
        if (fork() == 0) {
            // choose doctor
            int i;
            int meetings[5];
            for (i = 0; i < 5; i++) { meetings[i] = 0;}
            // if (appointments_list_size > 0)
            for (i = 0; i < APPOINTMENTS_LIST_SIZE; i++) {
                if (appointments_list[i].time_of_visit <= 0) continue;
                int index = atoi(appointments_list[i].password);
                meetings[index] += appointments_list[i].time_of_visit;
            }
            int min_meetings_index = meetings[0];
            for (i = 1; i < 5; i++) {
                if (meetings[i] < meetings[i-1])
                    min_meetings_index = i;
            }
            // CHOOSING TIME
            // set tomorrow's date
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

            int time_difference = (int)difftime(last_date,tomorrow);
            bool founded_date;
            time_t rand_date;
            srand(time(NULL));
            while (!founded_date) {
                founded_date = true;
                int rand_add = rand() % time_difference + 1;
                rand_date = tomorrow + rand_add;
                tomorrow_date = *localtime(&rand_date);
                rand_add = rand() % 13 - new_date.time_of_visit;
                tomorrow_date.tm_hour = 9 + rand_add;
                tomorrow_date.tm_min = 0;
                tomorrow_date.tm_sec = 0;
                rand_date = mktime(&tomorrow_date);
                if (tomorrow_date.tm_wday != 6 && tomorrow_date.tm_wday != 0) {
                    for(i = 0; i < APPOINTMENTS_LIST_SIZE; i++) {
                        int j;
                        bool the_same_pesel = true;
                        for (j = 0; j < 11; j++) {
                            if (appointments_list[i].pesel[j] != new_date.pesel[j]) {
                                the_same_pesel = false;
                                break;
                            }
                        }
                        if (appointments_list[i].index == min_meetings_index || the_same_pesel) {
                            if (rand_date >= appointments_list[i].date_of_visit &&
                                rand_date <= appointments_list[i].date_of_visit +
                                                     (86400*appointments_list[i].time_of_visit)) {
                                founded_date = false;
                                break;
                            }
                            if (vacation_list[min_meetings_index].index == min_meetings_index &&
                                rand_date >= vacation_list[min_meetings_index].date_of_visit &&
                                rand_date <= vacation_list[min_meetings_index].date_of_visit +
                                                     (86400*vacation_list[min_meetings_index].time_of_visit)) {
                                founded_date = false;
                                break;
                            }
                        }
                    }
                }
            }
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
    }
    return;
}

void sendDayContent(int msgid, int msgrcv_size, int pid_registration, int pid_patient, struct msgbuf appointment) {
    struct tm appointmentDate = *localtime(&appointment.date_of_visit);
    time_t chosen_date = appointment.date_of_visit;
    //printf("%s", asctime( & appointmentDate ));
    msgrcv_size = 0;
    int i;
    // SENDING REQUEST TO PARENT REGISTRATION
    appointment.typ = getppid();
    appointment.pid = pid_registration;
    msgsnd(msgid, &appointment, MSGBUF_SIZE, 0);
    appointments_list_size = 0;
    while (1) {
        msgrcv(msgid, &appointment, MSGBUF_SIZE, pid_registration, 0);
        if (appointment.index == 1000) {
            break;
        }
        appointments_list[appointments_list_size] = appointment;
        if (appointment.time_of_visit > 0) appointments_list_size++;
    }

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
                msgsnd(msgid, &appointment, MSGBUF_SIZE, 0);
            }
        }
    }
    appointment.pid = pid_registration;
    appointment.typ = pid_patient;
    appointment.index = 1000;
    msgsnd(msgid, &appointment, MSGBUF_SIZE, 0);

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
            }
        }
    }
    appointment.pid = pid_registration;
    appointment.typ = pid_patient;
    appointment.index = 1000;
    msgsnd(msgid, &appointment, MSGBUF_SIZE, 0);
    return;
}

void answerForListOfDoctors(int msgid, int msgrcv_size) {
    struct msgbuf appointment;
    msgrcv_size = msgrcv(msgid, &appointment, MSGBUF_SIZE , DOCTORS_REQUEST, IPC_NOWAIT);
    if (msgrcv_size > 0) {
        // CHILD REGISTRATION
        if (fork() == 0) {
            int pid_registration = getpid();
            int pid_patient = appointment.pid; // pid of patient
            // to fill in the content of day[][] array:
            sendDayContent(msgid, msgrcv_size, pid_registration, pid_patient, appointment);
            // REGISTRATION TO DOCTOR:
            msgrcv(msgid, &appointment, MSGBUF_SIZE, pid_registration, 0);
            if (appointment.index == 1000) {
                exit(pid_registration);
                return;
            }
            else {
                // SENDING APPOINTMENT TO PARENT REGISTRATION
                appointment.typ = getppid();
                msgsnd(msgid, &appointment, MSGBUF_SIZE, 0);
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
            int pid_registration = getpid();
            int pid_doctor = doctor.pid;
            doctor.typ = pid_doctor;
            doctor.pid = pid_registration;
            if (doctors_list_size > 0) {
                int i;
                for (i = 0; i < doctors_list_size; i++) {
                    bool appropriate = true;
                    int j;
                    // checking name
                    int name_size = strlen(doctors_list[i].name);
                    int name_size2 = strlen(doctor.name);
                    if (name_size != name_size2) appropriate = false;
                    if (!appropriate) continue;
                    for (j = 0; j < name_size; j++) {
                        if (doctor.name[j] != doctors_list[i].name[j]) {
                            appropriate = false;
                            break;
                        }
                    }
                    if (!appropriate) continue;
                    // checking surname
                    int surname_size = strlen(doctors_list[i].surname);
                    int surname_size2 = strlen(doctor.surname);
                    if (surname_size != surname_size2) appropriate = false;
                    if (!appropriate) continue;
                    for (j = 0; j < surname_size; j++) {
                        if (doctor.surname[j] != doctors_list[i].surname[j]) {
                            appropriate = false;
                            break;
                        }
                    }
                    if (!appropriate) continue;
                    // good name and surname:
                    int password_size = strlen(doctors_list[i].password);
                    int password_size2 = strlen(doctor.password);
                    if (password_size != password_size2) appropriate = false;
                    if (!appropriate) continue;
                    for (j = 0; j < password_size; j++) {
                        if (doctor.password[j] != doctors_list[i].password[j]) {
                            // wrong password
                            doctor.index = 1000;
                            msgsnd(msgid, &doctor, MSGBUF_SIZE, 0);
                            exit(pid_registration);
                            return;
                        }
                    }
                    // good name, surname and password
                    doctor = doctors_list[i];
                    doctor.typ = pid_doctor;
                    msgsnd(msgid, &doctor, MSGBUF_SIZE, 0);
                    exit(pid_registration);
                    return;
                }
            }
            // wrong pesel
            doctor.index = 1000;
            msgsnd(msgid, &doctor, MSGBUF_SIZE, 0);
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
            int pid_registration = getpid();
            int pid_patient = patient.pid;

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
                    int password_size2 = strlen(patient.password);
                    if (password_size != password_size2) appropriate = false;
                    if (!appropriate) continue;
                    for (j = 0; j < password_size; j++) {
                        if (patient.password[j] != patients_list[i].password[j]) {
                            patient.index = 1000;
                            msgsnd(msgid, &patient, MSGBUF_SIZE, 0);
                            exit(pid_registration);
                            return;
                        }
                    }
                    // good pesel and password
                    patient = patients_list[i];
                    patient.typ = pid_patient;
                    msgsnd(msgid, &patient, MSGBUF_SIZE, 0);
                    exit(pid_registration);
                    return;
                }
            }
            // wrong pesel
            patient.index = 1000;
            msgsnd(msgid, &patient, MSGBUF_SIZE, 0);
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
                    appointment = appointments_list[i];
                    appointment.typ = pid_patient;
                    msgsnd(msgid, &appointment, MSGBUF_SIZE, 0);
                }
            }
            appointment.index = 1000;
            appointment.pid = pid_registration;
            appointment.typ = pid_patient;
            msgsnd(msgid, &appointment, MSGBUF_SIZE, 0);

            // INFORMATION ABOUT ONE CHOSEN APPOINTMENT
            msgrcv(msgid, &visit, MSGBUF_SIZE , pid_registration, 0);
            if (visit.index == 1000) {   // resignation
                exit(pid_registration);
                return;
            }
            // choosing visit
            int number = visit.index;   // number of visit
            int counter = 0;
            for (i = 0; i < APPOINTMENTS_LIST_SIZE; i++) {
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

void answerForDoctorListOfVisits(int msgid, int msgrcv_size) {
    struct msgbuf visit, appointment;
    msgrcv_size = msgrcv(msgid, &visit, MSGBUF_SIZE, D_VISITS_REQUEST, IPC_NOWAIT);
    if (msgrcv_size > 0) {
        if (fork() == 0) {
            int pid_registration = getpid();
            int pid_doctor = visit.pid;

            // SENDING ALL APPOINTMENTS
            int i;
            bool appropriate;
            for (i = 0; i < APPOINTMENTS_LIST_SIZE; i++) {
                appropriate = true;
                if (atoi(appointments_list[i].password) != visit.index) appropriate = false;
                if (appropriate && appointments_list[i].time_of_visit > 0) {
                    appointment = appointments_list[i];
                    // sending name and surname of patient:
                    int j;
                    bool founded;
                    for (j = 0; j < patients_list_size; j++) {
                        founded = true;
                        int i;
                        for (i = 0; i < 11; i++) {
                            if (appointment.pesel[i] != patients_list[j].pesel[i]) {
                                founded = false;
                                break;
                            }
                        }
                        if (founded) {
                            strcpy(appointment.name, patients_list[j].name);
                            strcpy(appointment.surname, patients_list[j].surname);
                            break;
                        }
                    }
                    appointment.typ = pid_doctor;
                    msgsnd(msgid, &appointment, MSGBUF_SIZE, 0);
                }
            }
            appointment.index = 1000;
            appointment.pid = pid_registration;
            appointment.typ = pid_doctor;
            msgsnd(msgid, &appointment, MSGBUF_SIZE, 0);

            exit(pid_registration);
        }
    }
    return;
}

void searchForNewAppointment(int msgid, int msgrcv_size) {
    struct msgbuf appointment;
    int pid_registration = getpid();
    msgrcv_size = msgrcv(msgid, &appointment, MSGBUF_SIZE, pid_registration, IPC_NOWAIT);
    // from doctor when he wants to change the date of visit:
    if (msgrcv_size <= 0) msgrcv_size = msgrcv(msgid, &appointment, MSGBUF_SIZE, NEW_VISIT, IPC_NOWAIT);
    if (msgrcv_size > 0) {
        // ADDING NAMES AND SURNAMES
        int index = atoi(appointment.password);
        strcpy(appointment.name, doctors_list[index].name);
        strcpy(appointment.surname, doctors_list[index].surname);
        //
        bool ready;
        int i = 0;
        while (!ready) {
            if (appointments_list[i].time_of_visit != 0) i++;
            else {
                appointment.index = i;
                appointments_list[i] = appointment;
                appointments_list_size++;
                ready = true;
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
