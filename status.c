#include "repo.h"

/*
 * 스테이징 구역에 포함된 파일들에 대해 변경 내역을 추적하여 정보를 콘솔에 출력한다.
 * int argc: 입력된 인자의 개수
 * char *argv[]: 입력된 인자 배열 (명령어)
 */
void cmd_status(int argc, char *argv[]) {
    (void) argc;
    (void) argv;
    StatusFile *staged, *untracked;
    int staged_cnt, untracked_cnt;

    // 스테이징 중이거나 추적 되지 않은 파일들
    get_status_arrays(&staged, &staged_cnt, &untracked, &untracked_cnt);

    if (staged_cnt == 0 && untracked_cnt == 0) {
        printf("Nothing to commit\n");
    } else {
        if (staged_cnt > 0) {
            printf("Changes to be committed:\n");
            for (int i = 0; i < staged_cnt; i++) {
                printf("  %s: \"%s\"\n", staged[i].status_str, staged[i].rel_path);
            }
        }
        if (untracked_cnt > 0) {
            printf("Untracked files:\n");
            for (int i = 0; i < untracked_cnt; i++) {
                printf("  %s: \"%s\"\n", untracked[i].status_str, untracked[i].rel_path);
            }
        }
    }
    free(staged); 
    free(untracked);
}
