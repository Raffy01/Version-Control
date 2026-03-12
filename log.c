#include "repo.h"

/*
 * 지금까지 백업된 커밋 기록(.commit.log)을 읽어 화면에 출력한다.
 * int argc: 입력된 인자의 개수
 * char *argv[]: 입력된 인자 배열
 */
void cmd_log(int argc, char *argv[]) {
    // 예외 처리 : 인자가 너무 많음
    if (argc > 3) {
        fprintf(stderr, "ERROR: Too many arguments for log command\n");
        return;
    }  

    char cwd[PATH_LENGTH];
    if (getcwd(cwd, sizeof(cwd)) == NULL){
        fprintf(stderr, "unable to get cwd\n");
        exit(1);
    }

    if (argc == 3){
        if (strlen(argv[2]) > FILE_LENGTH) {
            fprintf(stderr, "ERROR: Commit name is too long\n");
            return;
        }
        // 리눅스 파일 시스템에서 디렉토리 이름에는 '/'가 들어갈 수 없으므로 차단
        if (strchr(argv[2], '/') != NULL) {
            fprintf(stderr, "ERROR: Invalid commit name\n");
            return;
        }
        // 커밋 버전이 없는 경우
        char commit_dir[PATH_LENGTH * 2];
        snprintf(commit_dir, sizeof(commit_dir), "%s/.repo/%s", cwd, argv[2]);
        if (access(commit_dir, F_OK) != 0) {
            fprintf(stderr, "ERROR: \"%s\" commit is not exist\n", argv[2]);
            return;
        }
    }


    char log_path[PATH_LENGTH + 32];
    snprintf(log_path, sizeof(log_path), "%s/.repo/.commit.log", cwd);
    
    struct stat st;
    // 파일 자체의 상태를 읽어올 수 없거나, 크기가 0(빈 파일)이면 기록이 없는 것
    if (stat(log_path, &st) < 0 || st.st_size == 0) {
        fprintf(stderr, "ERROR: Commit history not found\n");
        return;
    }
    
    // 로그 파일 열기
    FILE *f = fopen(log_path, "r");
    if (!f) {
        // stat으로 검사했기에 필요없는 부분이지만, 안전상 존치함
        return; 
    }

    char buf[PATH_LENGTH * 2];
    
    // 파일 내용을 한 줄씩 읽음
    while (fgets(buf, sizeof(buf), f)) {
        if (argc == 2) {
            // 인자가 생략된 경우: 전체 출력
            printf("%s", buf);
        } else if (argc == 3) {
            // 커밋명(NAME) 인자가 포함된 경우
            char log_name[FILE_LENGTH] = {0};
            
            // sscanf를 이용해 원본 버퍼를 훼손하지 않고 "커밋명"만 쏙 뽑아냄
            // 포맷: commit: "NAME" - STATUS: "PATH"
            if (sscanf(buf, "commit: \"%[^\"]\"", log_name) == 1) {
                // 추출한 커밋명이 사용자가 입력한 커밋명(argv[2])과 완벽히 일치할 때만 출력
                if (strcmp(log_name, argv[2]) == 0) {
                    printf("%s", buf);
                }
            }
        }
    }
    fclose(f);
}
