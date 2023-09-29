#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int razdelenieString(char *str, char *commands[]) {
    int len = strlen(str);
    int i, j, k;
    int insideQuotes = 0;
    int commandCount = 0;

    commands[commandCount++] = str; //количество команд, которые нужно выполнить последовательно

    for (i = 0, j = 0; i < len; i++) {
        if (str[i] == '"') {
            insideQuotes = !insideQuotes; //dup2 просто дублирует дескриптор, который передали первым аргументом, под номером, который передали вторым аргументом.В нашем случае мы дублируем дескриптор, связанный с пайпом под номером одного из стандартных потоков
        }

        if (!insideQuotes && str[i] == '|') {
            str[i] = '\0';
            commands[commandCount++] = &str[i + 2];
        }
    }
    return commandCount; //pipe - для связи между процессами 
    //Вы можете использовать этот файловый дескриптор для записи данных, которые будут переданы в другой процесс.
}

void execute_commands(char* command) {
    char* commands[256];
    int commandCount = razdelenieString(command, commands);


    int fds[2]; //файловые дескрипторы, помогающие определить какой файл или устройство мы используем
    pid_t pid; //фунция dup2 берет 1 файловый декскриптор и клонирует в другой(если был открыт,то закрывается)
//если все ок то оба дескриптора указывают на 1 файл
//нужно это для перенаправления вывода программы из консоли в файл
    for(int j = 0; j < commandCount; j++) {
        pipe(fds); //В каждой итерации цикла создается канал с помощью функции pipe(fds). Канал представляет два файловых дескриптора fds[0] и fds[1], которые связываются с вводом и выводом между процессами.

        if((pid = fork()) < 0) {
            perror("fork");
            exit(1);
        } else if(pid == 0) { //создание дочернего процесса
            if(j < commandCount - 1) {           //выполняется код дочернего процесса. Если j меньше commandCount - 1, то выполняется перенаправление стандартного вывода (STDOUT_FILENO) в файловый дескриптор записи fds[1] с помощью dup2(). Затем закрывается файловый дескриптор чтения fds[0]. После этого выполняется команда с помощью system(commands[j]), и дочерний процесс завершается
                if(dup2(fds[1], STDOUT_FILENO) < 0) exit(-1); // открытие стандартного вывода
            }
            close(fds[0]); //fds[0] (Read End): Это файловый дескриптор, связанный с концом канала для чтения данных.
            //канал pipe имеет два конца - один для чтения из него и другой для записи в него.
            system(commands[j]); //дескрипторы могут быть переданы в различные системные вызовы, которые требуют доступа к файлам или другим ресурсам, таким как dup2(), pipe()
            exit(0); //Работа с стандартными потоками: Стандартные потоки ввода, вывода и ошибок (stdin, stdout, stderr) также представлены файловыми дескрипторами. Вы можете использовать их для чтения ввода с клавиатуры, вывода на экран или перенаправления ввода-вывода.
        } else {
            if(j < commandCount - 1) {
            if(dup2(fds[0], STDIN_FILENO) < 0) exit(-1); //открытие ввода
                close(fds[1]);
            }
            wait(NULL);
        }
    }
    kill(getppid(), SIGUSR2);
}//как пример - родительский записывает,дочерний считает
// один в начало обращается,другой в конец. Закрываем один 
int main() {
    char command[1024];
    printf("Enter command: \"\"\n""");
    fgets(command, sizeof(command), stdin);

    // Отправка сигнала процессу А
    kill(getppid(), SIGUSR1);

    // Выполнение команды
    execute_commands(command);

    return 0;
}