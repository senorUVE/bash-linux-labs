#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>

pid_t child_pid; //хранение идентификатора дочернего процесса, который возвращается функцией fork()
int q = 0;
int delay = 5; // Задержка 

void kill_on_time_handler() {
    sleep(delay);
   
    if(q) return;

    printf("Program B closed on timout\n");
    kill(child_pid, SIGKILL); // завершаем,тк отослали сигнал
    
}
void kill_on_sucess_handler() {
    q = 1;
    kill(child_pid, SIGKILL);
    printf("Program B closed sucessfully\n");
}


int main() {
    signal(SIGUSR1, kill_on_time_handler);
    signal(SIGUSR2, kill_on_sucess_handler);
    child_pid = fork();

    if(child_pid == 0) {
        // Запуск подпрограммы Б
        execl("./B", "B", (char*) NULL); //Аргументы, передаваемые в функцию execl(), становятся аргументами командной строки нового процесса. Эти аргументы можно получить в новом процессе из параметров argc и argv функции main()
    } else {
        // Ждем завершения подпрограммы
        wait(NULL);
    }

    return 0;
}