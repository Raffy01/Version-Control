#include "repo.h"

/*
 * 커밋 이름을 입력받아, 해당 버전으로 백업되었던 파일들을 현재 작업 경로(CWD)로 다시 덮어씌워 복원한다.
 * int argc: 입력된 인자의 개수
 * char *argv[]: 입력된 인자 배열 (명령어 및 복원할 커밋명)
 */
void cmd_revert(int argc, char *argv[]) {
    // 예외 처리: 커밋명 인자가 누락된 경우
    if (argc < 3) {
        fprintf(stderr, "ERROR: <COMMIT_NAME> is not include\n");
        print_usage(5, 1);
        return;
    }

    char cwd[PATH_LENGTH];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
        fprintf(stderr, "unable to get cwd\n");
    
    char commit_dir[PATH_LENGTH * 2];
    snprintf(commit_dir, sizeof(commit_dir), "%s/.repo/%s", cwd, argv[2]);

    // 예외 처리: 입력한 이름의 커밋(버전) 디렉토리가 존재하지 않는 경우
    if (access(commit_dir, F_OK) != 0) {
        fprintf(stderr, "ERROR: \"%s\" commit is not exist\n", argv[2]);
        return;
    }

    // BFS 탐색을 위한 큐 할당 (백업 디렉토리 내부 스캔)
    char **queue = malloc(10000 * sizeof(char*));
    int head = 0, tail = 0;
    queue[tail++] = strdup(commit_dir);

    while (head < tail) {
        char *current_dir = queue[head++];
        struct dirent **namelist;
        int n = scandir(current_dir, &namelist, NULL, alphasort);
        // scandir 에러 처리
        if (n < 0) { free(current_dir); continue; }

        for (int i = 0; i < n; i++) {
            // 자신 및 부모 디렉토리 제외
            if (strcmp(namelist[i]->d_name, ".") == 0 || strcmp(namelist[i]->d_name, "..") == 0) {
                free(namelist[i]); continue;
            }
            // 타겟 경로
            char full_path[PATH_LENGTH * 2];
            snprintf(full_path, sizeof(full_path), "%s/%s", current_dir, namelist[i]->d_name);

            struct stat st;
            if (lstat(full_path, &st) == 0) {
                if (S_ISDIR(st.st_mode)) {
                    // 디렉토리면 큐에 넣어서 하위 탐색
                    queue[tail++] = strdup(full_path);
                } else if (S_ISREG(st.st_mode)) {
                    // 일반 파일이면 현재 작업 경로(CWD)로 복원!
                    char dest_path[PATH_LENGTH];
                    // 백업 파일의 경로에서 commit_dir 부분만 cwd로 치환하여 원본 복구 경로를 계산
                    snprintf(dest_path, sizeof(dest_path), "%s%s", cwd, full_path + strlen(commit_dir));
                    backup_file(full_path, dest_path);
                }
            }
            free(namelist[i]);
        }
        free(namelist);
        free(current_dir);
    }
    free(queue);

    // 성공 메시지 출력
    printf("revert to \"%s\"\n", argv[2]);
}
