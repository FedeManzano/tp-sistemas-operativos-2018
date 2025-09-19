#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include<sys/wait.h>

int main(char *args, char * argv[]){


  if(argv[1] != NULL && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "-help") || !strcmp(argv[1], "-?") )) {
      puts("Ejecucion" );
      puts("[Ruta del del programa]procesos" );
      puts("Compilacion" );
      puts("[Ruta del directorio del programa]make ./procesos" );
      puts("");
      return 3;
  }

  pid_t padre_todo = getpid(); // Padre de todo
  pid_t h1, h2;
  pid_t n1, n2, n3;
  pid_t b1, b2, b3, b4, b5;

  // Creacion del primer hijo

  printf("Soy el proceso con PID %d y pertenezco a la generación Nº %d\n", getpid(), 0);
  printf("Pid: %d   Pid padre: %d  Parentesco/Tipo: %s\n", getpid(), getppid(), "Padre");

  h1 = fork();


  if(h1 == 0){ // Si estoy en el proceso hijo 1
    printf("Soy el proceso con PID %d y pertenezco a la generación Nº %d\n", getpid(), 1);
    printf("Pid: %d   Pid padre: %d  Parentesco/Tipo: %s\n", getpid(), getppid(), "Hijo");
    n1 = fork(); // Creo al proceso nieto 1

    if(n1 == 0){
      printf("Soy el proceso con PID %d y pertenezco a la generación Nº %d\n", getpid(), 2);
      printf("Pid: %d   Pid padre: %d  Parentesco/Tipo: %s\n", getpid(), getppid(), "Nieto 1");
      b1 = fork();
      if(b1 == 0){
        printf("Soy el proceso con PID %d y pertenezco a la generación Nº %d\n", getpid(), 3);
        printf("Pid: %d   Pid padre: %d  Parentesco/Tipo: %s\n", getpid(), getppid(), "Bisnieto 1");
      }else{
        waitpid(b1, NULL, 0);
      }
      if(b1 > 0){
        b2 = fork();
        if(b2 == 0){
          printf("Soy el proceso con PID %d y pertenezco a la generación Nº %d\n", getpid(), 3);
          printf("Pid: %d   Pid padre: %d  Parentesco/Tipo: %s\n", getpid(), getppid(), "Bisnieto 2 Deamon");
          sleep(60);
        }else{
          sleep(10);
          exit(1);
        }
      }
    }
    if(n1 > 0){
        n2 = fork();
        if( n2 == 0 ){
          printf("Soy el proceso con PID %d y pertenezco a la generación Nº %d\n", getpid(), 2);
          printf("Pid: %d   Pid padre: %d  Parentesco/Tipo: %s\n", getpid(), getppid(), "Nieto 2 Zombie");
          b3 = fork();
          if(b3 == 0){
            printf("Soy el proceso con PID %d y pertenezco a la generación Nº %d\n", getpid(), 3);
            printf("Pid: %d   Pid padre: %d  Parentesco/Tipo: %s\n", getpid(), getppid(), "Bisnieto 3 Deamon");
            sleep(60);
          }

          if(b3 > 0){
            b4 = fork();
            if(b4 == 0){
              printf("Soy el proceso con PID %d y pertenezco a la generación Nº %d\n", getpid(), 3);
              printf("Pid: %d   Pid padre: %d  Parentesco/Tipo: %s\n", getpid(), getppid(), "Bisnieto 4");
            }else
              sleep(10);
              exit(1);
          }
        }
    }
  }
  /**
      Hijo 2
      nieto 3
      bisnieto 5
  */
  if(h1 > 0){
    // Creacion del segundo hijo
    pid_t h2 = fork();

    if(h2 == 0){
      printf("Soy el proceso con PID %d y pertenezco a la generación Nº %d\n", getpid(), 1);
      printf("Pid: %d   Pid padre: %d  Parentesco/Tipo: %s\n", getpid(), getppid(), "Hijo");

      n3 = fork();

      // Creacion del nieto 3
      if(n3 == 0){
        printf("Soy el proceso con PID %d y pertenezco a la generación Nº %d\n", getpid(), 2);
        printf("Pid: %d   Pid padre: %d  Parentesco/Tipo: %s\n", getpid(), getppid(), "Nieto 3 Zombie");

        b5 = fork();
        if(b5 == 0){ // Bisnieto 1 Demoño
          printf("Soy el proceso con PID %d y pertenezco a la generación Nº %d\n", getpid(), 3);
          printf("Pid: %d   Pid padre: %d  Parentesco/Tipo: %s\n", getpid(), getppid(), "Bisnieto 5 Deamon");
          sleep(60);
        }else{
          sleep(10);
          exit(1);
        }
      }
    }
  }

  if(getpid() == padre_todo){
    waitpid(h2, NULL, 0);
    waitpid(h1, NULL, 0);
    fflush(stdin);
    getchar();
  }

  sleep(60);
}
