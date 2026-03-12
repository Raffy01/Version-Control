#include "repo.h"

/*
 * 커밋 이름을 입력받아 해당 이름으로 버전 디렉토리를 생성하고, 스테이징 구역 파일들의 변경 내역을 백업한다.
 * int argc: 입력된 인자의 개수
 * char *argv[]: 입력된 인자 배열 (명령어 및 커밋명)
 */
void cmd_commit(int argc, char *argv[]) {
    // 예외 처리: 커밋명 인자가 누락된 경우 Usage 출력
    if (argc < 3) {
        fprintf(stderr, "ERROR: <NAME> is not include\n");
        print_usage(4, 1);
        return;
    }
    // 예외 처리: 커밋명이 시스템 제한(255 Byte)을 초과한 경우
    if (strlen(argv[2]) > FILE_LENGTH) {
        fprintf(stderr, "ERROR: Commit name is too long\n");
        return;
    }
    
    char cwd[PATH_LENGTH]; 
    if (getcwd(cwd, sizeof(cwd)) == NULL)
        fprintf(stderr, "unable to get cwd\n");
    char version_dir[PATH_LENGTH * 2];
    snprintf(version_dir, sizeof(version_dir), "%s/.repo/%s", cwd, argv[2]);

    // 예외 처리: 동일한 이름의 커밋 디렉토리가 이미 존재하는 경우 차단
    if (access(version_dir, F_OK) == 0) {
        printf("\"%s\" is already exist in repo\n", argv[2]);
        return;
    }

    StatusFile *staged, *untracked;
    int staged_cnt, untracked_cnt;
    get_status_arrays(&staged, &staged_cnt, &untracked, &untracked_cnt);

    // 예외 처리: 변경 사항이 없을 경우 백업 디렉토리를 생성하지 않고 종료
    if (staged_cnt == 0) {
        printf("Nothing to commit\n");
        free(staged); free(untracked);
        return;
    }

    mkdir(version_dir, 0777); // 버전 디렉토리 생성

    // 각 파일의 추가/삭제 라인 수(Diff) 누적 계산
    int total_ins = 0, total_del = 0;
    for (int i = 0; i < staged_cnt; i++) {
        int ins = 0, del = 0;
        char backup_path[PATH_LENGTH * 3] = {0};
        commit *last = get_last_commit(staged[i].abs_path);
        
        if (last) {
            snprintf(backup_path, sizeof(backup_path), "%s/.repo/%s%s", cwd, last->dir, staged[i].abs_path + strlen(cwd));
        }

        if (strcmp(staged[i].status_str, "new file") == 0) {
            calc_diff(NULL, staged[i].abs_path, &ins, &del);
        } else if (strcmp(staged[i].status_str, "removed") == 0) {
            calc_diff(backup_path, NULL, &ins, &del);
        } else { // modified
            calc_diff(backup_path, staged[i].abs_path, &ins, &del);
        }
        total_ins += ins; total_del += del;
    }

    // 결과 요약 출력
    printf("commit to \"%s\"\n", argv[2]);
    printf("%d files changed, %d insertions(+), %d deletions(-)\n", staged_cnt, total_ins, total_del);

    // 실제 백업 수행 및 로그 기록
    for (int i = 0; i < staged_cnt; i++) {
        printf("%s: \"%s\"\n", staged[i].status_str, staged[i].rel_path);

        // 삭제가 아닌 경우에만 실제 백업 파일 복사 진행
        if (strcmp(staged[i].status_str, "removed") != 0) {
            char new_backup_path[PATH_LENGTH * 2];
            snprintf(new_backup_path, sizeof(new_backup_path), "%s%s", version_dir, staged[i].abs_path + strlen(cwd));
            backup_file(staged[i].abs_path, new_backup_path);
        }

        // 로그 기록 및 리스트 업데이트
        append_commit_log(argv[2], staged[i].status_str, staged[i].abs_path);
        update_commit_list(argv[2], staged[i].status_str, staged[i].abs_path);
    }

    free(staged); free(untracked);
}
