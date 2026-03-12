#include "repo.h"

/*
 * 인자로 받은 경로의 파일 또는 디렉토리를 스테이징 구역에서 제거한다.
 * int argc: 입력된 인자의 개수
 * char *argv[]: 입력된 인자 배열 (명령어 및 경로)
 */
void cmd_remove(int argc, char *argv[]) {
    // 예외 처리: 경로(PATH) 인자가 누락된 경우
    if (argc < 3) {
        fprintf(stderr, "ERROR: <PATH> is not include\n");
        print_usage(2, 1);
        return;
    }
    // 예외 처리: 인자가 초과 입력된 경우
    if (argc > 3) {
        fprintf(stderr, "ERROR: Too many arguments entered\n");
        print_usage(2, 1);
        return;
    }

    char path[PATH_LENGTH];
    // 예외 처리: 절대경로 변환 실패 시
    if (realpath(argv[2], path) == NULL) {
        fprintf(stderr, "ERROR: %s is wrong path\n", argv[2]);
        return;
    }

    struct stat statbuf;
    // 예외 처리: 파일 상태를 읽어올 수 없는 경우
    if (lstat(path, &statbuf) < 0) {
        fprintf(stderr, "ERROR: %s is wrong path\n", argv[2]);
        return;
    }

    // 예외 처리: 일반 파일이나 디렉토리가 아닌 경우
    if (!S_ISDIR(statbuf.st_mode) && !S_ISREG(statbuf.st_mode)) {
        fprintf(stderr, "ERROR: %s < Path is not Regular file or Directory\n", path);
        return;
    }

    char cwd[PATH_LENGTH];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
        fprintf(stderr, "unable to get cwd\n");
    
    char repo_dir[PATH_LENGTH * 2];
    snprintf(repo_dir, sizeof(repo_dir), "%s/.repo", cwd);

    // 예외 처리: 작업 디렉토리(CWD)를 벗어나거나 .repo 하위 경로인 경우 차단
    if (strncmp(cwd, path, strlen(cwd)) != 0) {
        fprintf(stderr, "ERROR: %s is wrong path\n", argv[2]);
        return;
    }
    if (strncmp(repo_dir, path, strlen(repo_dir)) == 0) {
        fprintf(stderr, "ERROR: %s is wrong path\n", argv[2]);
        return;
    }

    // 원본 상대경로 계산
    char relative_path[PATH_LENGTH];
    if (strcmp(cwd, path) == 0) {
        strcpy(relative_path, ".");
    } else {
        snprintf(relative_path, sizeof(relative_path), ".%s", path + strlen(cwd));
    }

    // 예외 처리: 이미 remove된 경로인지 검증
    if (get_staging_state(path) == 2) { // 2 : removed
        printf("\"%s\" already removed from staging area\n", relative_path);
        return;
    }

    // 로그 기록 및 리스트 업데이트
    append_staging_log(argv[1], path);
    update_staging_list(argv[1], path);
    printf("%s \"%s\"\n", argv[1], relative_path);
}
