#include "repo.h"

// 전역 변수 실제 정의
int fd_stag;
int fd_commit;
staging *stag_head = NULL;
commit *commit_head = NULL;

/*
 * 프로그램의 메인 진입점. 부모 프로세스는 사용자 입력을 받아 자식 프로세스를 생성하고,
 * 자식 프로세스는 입력된 명령어에 맞춰 분기하여 기능을 수행한다.
 * int argc: 실행 시 인자 개수
 * char *argv[]: 실행 인자 문자열 배열
 * return : int, 프로그램 종료 상태 코드
 */
int main(int argc, char *argv[]) {
    // 프로그램 시작 시 .repo 구조 및 로그 파일 초기화
    init_repo();

    if (argc == 1) { // Parent process: 프롬프트를 띄우고 사용자 입력을 대기
        char input[PATH_LENGTH];
        
        while (1) {
            printf("> ");
            // 예외 처리: 파일 끝(EOF) 도달 등 입력 실패 시 루프 탈출 후 종료
            if (fgets(input, sizeof(input), stdin) == NULL) {
                printf("\n");
                break;
            }
            
            // 입력된 문자열의 끝에 있는 개행 문자 제거
            input[strcspn(input, "\n")] = '\0';
            // 예외 처리: 엔터만 입력된 경우 무시하고 프롬프트 재출력
            if (strlen(input) == 0) continue;

            char *args[128];
            int i = 0;
            args[i++] = argv[0]; // 0번 인자는 실행 파일 자신

            char *token = strtok(input, " ");
            if (token == NULL) continue;

            // "exit" 명령어 입력 시 즉시 종료 루틴 수행
            if (strcmp(token, "exit") == 0) break;

            // 띄어쓰기 기준으로 인자 파싱 및 동적 할당 배열 구성
            while (token != NULL && i < 127) {
                args[i++] = strdup(token);
                token = strtok(NULL, " ");
            }
            args[i] = NULL; // exec 계열 함수를 위해 마지막은 NULL로 패딩

            // 프로세스 분기
            pid_t pid = fork();
            if (pid < 0) {
                // 예외 처리: fork 실패 시 에러 출력 후 종료
                perror("fork error");
                exit(1);
            } else if (pid == 0) { // Child process
                execvp(args[0], args);
                // 예외 처리: execvp 실패 (파일을 찾을 수 없는 경우 등)
                perror("execvp error");
                exit(1);
            } else { // Parent process
                wait(NULL); // 자식 프로세스의 작업이 끝날 때까지 대기
                for (int j = 1; j < i; j++) free(args[j]); // 동적 할당 해제
            }
        }
    } else { // Child executing command: 명령어 실제 수행 부분
        int command = parse_command(argv[1]);
        switch (command) {
            case 1: cmd_add(argc, argv); break;
            case 2: cmd_remove(argc, argv); break;
            case 3: cmd_status(argc, argv); break;
            case 4: cmd_commit(argc, argv); break;
            case 5: cmd_revert(argc, argv); break;
            case 6: cmd_log(argc, argv); break;
            case 7: print_usage_all(); break;
            default:
                // 예외 처리: 잘못된 명령어거나 알 수 없는 명령어가 입력된 경우 전체 도움말 출력
                if (command == -1) {
                    print_usage_all();
                } else {
                    printf("Working in Progress\n");
                }
                break;
        }
    }
    
    // 프로그램 종료 직전 열어둔 파일 디스크립터 정리
    close(fd_stag);
    close(fd_commit);
    exit(0);
}
