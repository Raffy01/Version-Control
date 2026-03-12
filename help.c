#include "repo.h"

/*
 * 입력된 명령어에 대한 올바른 사용법(Usage)을 출력한다.
 * int command: 출력할 명령어 번호 (1: add, 2: remove, ...)
 * int is_error: 에러 상황에서 호출되었는지 여부 (0: 기본 출력, 1: 에러 출력(Usage: 포함))
 */
void print_usage(int command, int is_error) {
    if (is_error)
        printf("Usage:\n");
    else
        printf("  >");

    switch (command) {
        case 1: // add
            printf(" add <PATH> : record path to staging area, path will tracking modification\n");
            break;
        case 2: // remove
            printf(" remove <PATH> : record path to staging area, path will not tracking modification\n");
            break;
        case 3: // status
            printf(" status : show staging area status\n");
            break;
        case 4: // commit
            printf(" commit <NAME> : backup staging area with commit name\n");
            break;
        case 5: // revert
            printf(" revert <NAME> : recover commit version with commit name\n");
            break;
        case 6: // log
            printf(" log : show commit log\n");
            break;
        case 7: // help
            printf(" help : show commands for program\n");
            break;
        case 8: // exit
            printf(" exit : exit program\n");
            break;
    }
}

/*
 * 프로그램에서 지원하는 모든 내장 명령어의 사용법을 전체 출력한다.
 */
void print_usage_all(void) {
    printf("Usage:\n");
    for (int i = 1; i <= 8; i++) {
        print_usage(i, 0);
    }
}
