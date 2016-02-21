#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include"inf122464_all.h"
#include"inf122464_rfun.h"

int main() {
    setLastDate();
    patients_list_size = 0;
    doctors_list_size = 0;
    appointments_list_size = 0;
    quit = false;
    readConfigurationFile("ConfigurationFile.bat");
    int msgrcv_size = 0;
    int msgid = createMessageQueue("REGISTRATION");

    // FOR CHILD:
    if (fork() == 0) {
        // INFO FOR PARENT REGISTRATION
        int child_pid = getpid();
        int parent_pid = getppid();
        struct msgbuf child_info;
        child_info.pid = child_pid;
        child_info.typ = parent_pid;
        msgsnd(msgid, &child_info, MSGBUF_SIZE, 0);
        //
        while (!quit) {
            // SEARCHING OUTDATED APPOINTMENTS
            searchForOutdatedAppointments();
            // CHECK IF PATIENT WANT TO CHANGE THE DATE OF VISIT
            answerForChangeDateOfVisit(msgid);
            // CHECK IF PATIENT'S DELETED AN APPOINTMENT
            deleteVisit(msgid, msgrcv_size);
            // CHECK IF DOCTOR HAS TAKEN A LEAVE
            answerForDoctorsLeave(msgid);
            // SEARCHING FOR NEW APPOINTMENT (FROM CHILDREN IN FORK())
            searchForNewAppointment(msgid, msgrcv_size);
            // ADDING NEW REGISTERED PATIENT
            addPatient(msgid, msgrcv_size);
            // LOGIN REQUESTS
            answerForDoctorLoginRequest(msgid, msgrcv_size);
            answerForLoginRequest(msgid, msgrcv_size);
            // LIST OF DOCTORS REQUEST
            answerForListOfDoctors(msgid, msgrcv_size);
            // LIST OF VISITS REQUEST
            answerForListOfVisits(msgid, msgrcv_size);
            answerForDoctorListOfVisits(msgid, msgrcv_size);
            // CHECK IF QUIT
            quit = checkExit(msgid);
        }
        msgctl(msgid, IPC_RMID, NULL);
        exit(child_pid);
    }
    // FOR PARENT:
    else {
        struct msgbuf child_info;
        int my_pid = getpid();
        msgrcv(msgid, &child_info, MSGBUF_SIZE, my_pid, 0);

        while (!quit) {
            char decision;
            printf("\tIf you want to quit registration system, enter 'q': ");
            scanf("%s", &decision);
            if (decision == 'q') {
                quit = true;
                child_info.typ = QUIT;
                msgsnd(msgid, &child_info, MSGBUF_SIZE, 0);
            }
        }
    }
    printf("-------------------------------\n");
    printf("\tBYE BYE!\n");
    printf("-------------------------------\n");
    return 0;
}  
