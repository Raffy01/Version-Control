#include "repo.h"

/*
 * 인자로 받은 경로의 파일 또는 디렉토리를 스테이징 구역에 추가한다.
 * int argc: 입력된 인자의 개수
 * char *argv[]: 입력된 인자 배열 (명령어 및 경로)
 */
void cmd_add(int argc, char *argv[]) {
    // 예외 처리: 경로(PATH) 인자가 누락된 경우 에러 및 Usage 출력
    if (argc < 3) {
        fprintf(stderr, "ERROR: <PATH> is not include\n");
        print_usage(1, 1);
        return;
    }
    // 예외 처리: 인자가 제한 개수를 초과하여 너무 많이 입력된 경우
    if (argc > 3) {
        fprintf(stderr, "ERROR: Too many arguments entered\n");
        print_usage(1, 1);
        return;
    }

    char path[PATH_LENGTH];
    // 예외 처리: 입력받은 경로를 절대경로로 변환하는 데 실패한 경우
    if (realpath(argv[2], path) == NULL) {
        fprintf(stderr, "ERROR: %s is wrong path\n", argv[2]);
        return;
    }

    struct stat statbuf;
    // 예외 처리: 파일 상태를 읽어올 수 없는 경우 (권한 부족, 파일 부재 등)
    if (lstat(path, &statbuf) < 0) {
        fprintf(stderr, "ERROR: %s is wrong path\n", argv[2]);
        return;
    }

    // 예외 처리: 입력된 경로가 일반 파일(Regular file)도 아니고 디렉토리도 아닌 경우
    if (!S_ISDIR(statbuf.st_mode) && !S_ISREG(statbuf.st_mode)) {
        fprintf(stderr, "ERROR: %s < Path is not Regular file or Directory\n", path);
        return;
    }

    char cwd[PATH_LENGTH];
    if (getcwd(cwd, sizeof(cwd)) == NULL){
        fprintf(stderr, "unable to get cwd\n");
    };
    
    char repo_dir[PATH_LENGTH * 2];
    snprintf(repo_dir, sizeof(repo_dir), "%s/.repo", cwd);

    // 예외 처리: 입력받은 경로가 현재 작업 디렉토리(CWD)를 벗어나는 경우
    if (strncmp(cwd, path, strlen(cwd)) != 0) {
        fprintf(stderr, "ERROR: %s is wrong path\n", argv[2]);
        return;
    }
    // 예외 처리: 입력받은 경로가 레포지토리 관리 구역(.repo) 하위인 경우 차단
    if (strncmp(repo_dir, path, strlen(repo_dir)) == 0) {
        fprintf(stderr, "ERROR: %s is wrong path\n", argv[2]);
        return;
    }

    // 현재 작업 경로를 기준으로 하는 원본 상대경로 계산
    char relative_path[PATH_LENGTH];
    if (strcmp(cwd, path) == 0) {
        strcpy(relative_path, ".");
    } else {
        snprintf(relative_path, sizeof(relative_path), ".%s", path + strlen(cwd));
    }

    // 예외 처리: 스테이징 리스트를 탐색하여 중복 혹은 하위 경로 추가 시도인지 검증
    staging *tmp = stag_head;
    while (tmp != NULL) {
        // 이미 스테이징된 경로(tmp->path)가 입력된 경로(path)의 접두사인 경우 (예: /dir 이 이미 있는데 /dir/file 을 추가하려는 경우)
        if (strncmp(path, tmp->path, strlen(tmp->path)) == 0) {
            int a = get_path_depth(path);
            int b = get_path_depth(tmp->path);
            
            // 뎁스가 다르면(하위 경로이면) 이미 포함된 것으로 간주
            if (a != b) {
                printf("\"%s\" already exist in staging area\n", relative_path);
                return;
            } else {
                // 뎁스가 같을 때, 우연히 이름 앞부분이 겹치는 경우 방지 (예: /a 와 /a.txt)
                char *filename1 = strrchr(path, '/') + 1;
                char *filename2 = strrchr(tmp->path, '/') + 1;
                if (strcmp(filename1, filename2) == 0) {
                    printf("\"%s\" already exist in staging area\n", relative_path);
                    return;
                }
            }
        }
        tmp = tmp->next;
    }

    // 검증을 통과하면 로그 파일에 기록하고 메모리 리스트에 추가한 뒤 결과 출력
    append_staging_log(argv[1], path);
    add_staging_node(argv[1], path);
    printf("%s \"%s\"\n", argv[1], relative_path);
}
